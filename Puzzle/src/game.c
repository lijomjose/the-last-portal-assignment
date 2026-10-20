#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *maps[3][MAP_H] = {
    { "############", "#..........#", "#..##......#", "#..#.......#",
      "#..........#", "#..........#", "#..........#", "############" },
    { "############", "#..........#", "#..........#", "#..........#",
      "#..........#", "#..........#", "#..........#", "############" },
    { "############", "############", "############", "############",
      "#..........#", "############", "############", "############" }
};

static const int dxs[] = { 0, 1, 0, -1 };
static const int dys[] = { -1, 0, 1, 0 };
const int portal_probe_readings[5] = {88,88,57,31,57};
static bool same(Cell a, Cell b) { return a.x == b.x && a.y == b.y; }

char game_tile(int room, int x, int y)
{
    if (room < 0 || room > 2 || x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return '#';
    return maps[room][y][x];
}

Cell game_exit(int room) { return room == 2 ? (Cell){10, 4} : (Cell){10, 6}; }

const char *command_name(Command command)
{
    static const char *names[] = { "UP", "RIGHT", "DOWN", "LEFT" };
    return command >= CMD_UP && command <= CMD_LEFT ? names[command] : "?";
}

void game_message(Game *game, GameEvent event, const char *message)
{
    snprintf(game->message, sizeof(game->message), "%s", message);
    game->event = event;
    game->event_serial++;
}

static BoardState snapshot(const Game *game)
{
    BoardState state = { .hero = game->hero, .crate = game->crate,
        .bot = game->bot, .queue = game->queue, .counter_queue = game->counter_queue, .delivered = game->delivered, .duel = game->duel, .courier = game->courier };
    state.attack_count = attack_copy_ids(game->attacks, state.attack_ids, ATTACK_CAPACITY);
    int i=0;
    for(const AttackNode *n=game->attacks;n&&i<ATTACK_CAPACITY;n=n->next) state.attack_openings[i++]=n->opening;
    return state;
}

static bool remember(Game *game)
{
    BoardState state = snapshot(game);
    if (state.attack_count < 0 || !history_push(&game->history, &state)) {
        game_message(game, EVENT_BUMP, "Memory full. Undo a few steps, or restart this room with R.");
        return false;
    }
    return true;
}

static bool restore(Game *game, const BoardState *state)
{
    AttackNode *replacement = NULL;
    int before = -1;
    for (int i = 0; i < state->attack_count; i++) {
        if (!attack_insert_after(&replacement, before, state->attack_ids[i], state->attack_openings[i])) {
            attack_free(&replacement);
            return false;
        }
        before = state->attack_ids[i];
    }
    attack_free(&game->attacks);
    game->attacks = replacement;
    game->hero = state->hero; game->crate = state->crate; game->bot = state->bot;
    game->queue = state->queue; game->counter_queue = state->counter_queue; game->delivered = state->delivered;
    game->running = false; game->command_clock = 0; game->duel = state->duel; game->courier = state->courier;
    return true;
}

/* Small deterministic PRNG keeps random-route tests reproducible on both OSes. */
static uint32_t next_random(Game *game)
{
    uint32_t value=game->random_state;
    value^=value<<13; value^=value>>17; value^=value<<5;
    return game->random_state=value;
}

static void generate_memory_path(Game *game)
{
    CourierState *c=&game->courier;
    for(int trial=0;trial<512;trial++) {
        bool visited[6][11]={{false}};
        c->length=10+(int)(next_random(game)%5);
        c->path[0]=(Cell){1,1}; visited[1][1]=true;
        int turns=0, previous=-1, i;
        for(i=0;i<c->length;i++) {
            int options[4], count=0;
            for(int d=0;d<4;d++) {
                int x=c->path[i].x+dxs[d], y=c->path[i].y+dys[d];
                if(x>=1&&x<=10&&y>=1&&y<=5&&!visited[y][x]) options[count++]=d;
            }
            if(!count) break;
            int d=options[next_random(game)%(uint32_t)count];
            c->path[i+1]=(Cell){c->path[i].x+dxs[d],c->path[i].y+dys[d]};
            visited[c->path[i+1].y][c->path[i+1].x]=true;
            if(previous>=0&&previous!=d) turns++;
            previous=d;
        }
        if(i==c->length&&turns>=3&&c->path[c->length].x>=6) return;
    }
    /* Bounded generation always terminates, even for an unlucky seed. */
    const Command fallback[]={CMD_RIGHT,CMD_RIGHT,CMD_DOWN,CMD_DOWN,
        CMD_RIGHT,CMD_RIGHT,CMD_UP,CMD_UP,CMD_RIGHT,CMD_RIGHT,CMD_DOWN,CMD_DOWN};
    c->length=12; c->path[0]=(Cell){1,1};
    for(int i=0;i<c->length;i++) c->path[i+1]=(Cell){c->path[i].x+dxs[fallback[i]],c->path[i].y+dys[fallback[i]]};
}

bool game_load_room(Game *game, int room)
{
    if (room < 0 || room > 2) return false;
    CourierState previous=game->courier;
    int old_room=game->room;
    int unlocked = game->unlocked;
    uint32_t random_state=game->random_state?game->random_state:1;
    int attempt=room==1&&game->room==1?game->courier.attempt+1:1;
    attack_free(&game->attacks);
    memset(game, 0, sizeof(*game));
    game->random_state=random_state;
    game->room = room; game->unlocked = unlocked > room ? unlocked : room;
    game->crate = (Cell){ -1, -1 }; game->bot = (Cell){ 1, 1 };
    queue_init(&game->queue); queue_init(&game->counter_queue); history_init(&game->history);
    if (room == 0) {
        game->hero = (Cell){ 8, 3 }; game->crate = (Cell){ 8, 2 };
        game->history.items[0] = snapshot(game);
        game->history.count = 1;
        game->hero = (Cell){ 8, 2 }; game->crate = (Cell){ 8, 1 };
        game_message(game, EVENT_NONE, "That last push trapped the crate. Press Z to take it back.");
    } else if (room == 1) {
        game->hero = (Cell){ 2, 6 };
        game->courier.attempt=attempt; game->courier.phase=COURIER_WAITING;
        generate_memory_path(game);
        if(old_room==1&&previous.length==game->courier.length&&
           !memcmp(previous.path,game->courier.path,(size_t)(previous.length+1)*sizeof(Cell)))
            for(int i=0;i<=game->courier.length;i++) game->courier.path[i].y=6-game->courier.path[i].y;
        game->bot=game->courier.path[0]; game->interacting=true;
        game_message(game, EVENT_NONE, "Memory run: Space reveals a random route for 30 seconds. Then queue EVERY move and launch. One wrong step destroys the bot.");
    } else {
        game->hero = (Cell){ 2, 4 };
        game->duel.phase = DUEL_WAITING;
        game_message(game, EVENT_NONE, "An alien guards the only path Home. Space begins. Type the whole countersequence; Enter launches. F1 explains.");
    }
    return true;
}

void game_init_seeded(Game *game, int room, uint32_t seed)
{
    memset(game,0,sizeof(*game)); game->random_state=seed?seed:1;
    if(!game_load_room(game,room)) game_message(game,EVENT_BUMP,"Could not prepare this room. Please restart the game.");
}

void game_init(Game *game, int room)
{ game_init_seeded(game,room,(uint32_t)time(NULL)^(uint32_t)clock()); }

void game_destroy(Game *game) { attack_free(&game->attacks); }

bool game_reserve_reading(int *out)
{ return select_finisher_power(portal_probe_readings,5,out); }

bool game_regulator_ok(void)
{
    int reserve=0;
    return game_reserve_reading(&reserve) && reserve==57;
}

bool game_seal_ok(void)
{
    AttackNode older={.id=0,.opening='(', .next=NULL};
    AttackNode top={.id=1,.opening='[',.next=&older};
    CommandQueue response={.items={CMD_BLOCK_SQUARE,CMD_BLOCK_ROUND},.front=0,.rear=1,.count=2};
    bool good=shield_accepts(&top,&response);
    response.items[0]=CMD_BLOCK_ROUND; response.items[1]=CMD_BLOCK_SQUARE;
    return good && !shield_accepts(&top,&response);
}

bool game_ready(const Game *game)
{
    if (game->room == 0) return same(game->crate, (Cell){5,4});
    if (game->room == 1) return game->delivered;
    return game->duel.defeated && game->duel.total_blocks == DUEL_ATTACK_TOTAL &&
           !game->attacks && game_regulator_ok() && game_seal_ok();
}

static void complete_room(Game *game)
{
    game->room_complete = true;
    game->running = false; game->interacting = false;
    if (game->room == 2) game->won = true;
    else if (game->unlocked < game->room + 1) game->unlocked = game->room + 1;
    game_message(game, EVENT_COMPLETE, game->won ? "Every attack answered. The guardian defeated. You brought Caitlyn Home." :
                 "Another part of the station is alive. Press Enter to continue.");
}

bool game_move(Game *game, int dx, int dy)
{
    if (game->room_complete || game->interacting || abs(dx) + abs(dy) != 1) return false;
    if (game->room == 2 && !game_ready(game)) {
        game_message(game, EVENT_BUMP, "The alien blocks this alley. Block every attack, then use F for the finishing move."); return false;
    }
    Cell target = { game->hero.x + dx, game->hero.y + dy };
    if (game_tile(game->room, target.x, target.y) != '.') {
        game_message(game, EVENT_BUMP, game->room == 1 ? "Only the courier can travel on that track." : "A wall. Try another way.");
        return false;
    }
    if (same(target, game_exit(game->room)) && !game_ready(game)) {
        game_message(game, EVENT_BUMP, "The exit is still dark. Restore this room first.");
        return false;
    }
    bool pushing = game->room == 0 && same(target, game->crate);
    Cell crate_target = { game->crate.x + dx, game->crate.y + dy };
    if (pushing && game_tile(game->room, crate_target.x, crate_target.y) != '.') {
        game_message(game, EVENT_BUMP, "The crate cannot move there. Z can undo a push.");
        return false;
    }
    bool was_ready = game_ready(game);
    if (!remember(game)) return false;
    game->hero = target;
    if (pushing) game->crate = crate_target;
    game->moves++;
    if (!was_ready && game_ready(game)) game_message(game, EVENT_POWER, "The pressure plate is down. The doorway has power!");
    else if (pushing) game_message(game, EVENT_PUSH, "A push remembered. Z can take it back.");
    else game_message(game, EVENT_MOVE, game->room == 0 ? "Guide the crate onto the glowing plate." : "Follow the light to the exit.");
    if (same(game->hero, game_exit(game->room))) complete_room(game);
    return true;
}

bool game_undo(Game *game)
{
    if (game->room_complete) return false;
    if(game->room==1&&!game->delivered) {
        game_message(game,EVENT_BUMP,"Memory run: edit before launch. You cannot rewind a run or a destroyed bot. R gives a fresh route."); return false;
    }
    if(game->room==2&&(game->duel.phase==DUEL_INCOMING||game->duel.phase==DUEL_COUNTER||game->duel.phase==DUEL_FINISH)) {
        game_message(game,EVENT_BUMP,"Let this animation finish before rewinding, or R restarts the duel."); return false;
    }
    BoardState state;
    if (!history_pop(&game->history, &state)) {
        game_message(game, EVENT_BUMP, "No earlier state to return to.");
        return false;
    }
    if (!restore(game, &state)) {
        history_push(&game->history, &state);
        game_message(game, EVENT_BUMP, "Could not restore the earlier state. Nothing was changed.");
        return false;
    }
    game_message(game, EVENT_UNDO, "Rewound one step. The most recent state comes back first.");
    return true;
}

bool game_interact(Game *game)
{
    if(game->room_complete) return game_next_room(game);
    if(game->room==2&&!game->duel.defeated) return game_duel_start(game);
    if(game->room==1) {
        if(game->courier.phase==COURIER_WAITING) return game_memory_reveal(game);
        game_message(game,EVENT_NONE,game->delivered?"Delivery complete. Walk Caitlyn right into EXIT.":
            game->courier.phase==COURIER_CRASH?"The bot is destroyed. R starts a new attempt with a different route.":
            game->courier.phase==COURIER_REVEAL?"Memorize the glowing arrows before the countdown ends.":
            game->courier.phase==COURIER_RUN?"The bot is following the committed plan. A wrong step ends the attempt.":
            "Queue the whole remembered route; Space launches. R retries with a different path.");
        return false;
    }

    Cell gate = game_exit(game->room);
    if (abs(game->hero.x - gate.x) + abs(game->hero.y - gate.y) <= 1 && game_ready(game)) {
        complete_room(game); return true;
    }
    game_message(game, EVENT_NONE, game->room == 0 ? "Z rewinds. The glowing tile is a pressure plate." : "Walk to the console marked E.");
    return false;
}

bool game_memory_reveal(Game *game)
{
    if(game->room!=1||game->courier.phase!=COURIER_WAITING||game->room_complete) return false;
    game->courier.phase=COURIER_REVEAL; game->courier.clock=0; game->hint=false;
    game_message(game,EVENT_COMMAND,"MEMORIZE! You have 30 seconds. Press Space or Start planning now to hide the route early when ready.");
    return true;
}

bool game_memory_hide(Game *game)
{
    if(game->room!=1||game->courier.phase!=COURIER_REVEAL||game->room_complete) return false;
    game->courier.phase=COURIER_PLAN; game->courier.clock=0;
    game_message(game,EVENT_NONE,"ROUTE HIDDEN. Queue the complete path from memory. Space launches; C clears; Backspace edits.");
    return true;
}

bool game_enqueue(Game *game, Command command)
{
    if(game->room!=1||game->courier.phase!=COURIER_PLAN||game->room_complete||command<CMD_UP||command>CMD_LEFT) return false;
    if(game->queue.count>=game->courier.length) {
        game_message(game,EVENT_BUMP,"The whole route is queued. Space launches; Backspace edits the last arrow; C clears."); return false;
    }
    if(!queue_enqueue(&game->queue,command)) {
        game_message(game,EVENT_BUMP,"The command could not be enqueued. Check your queue implementation."); return false;
    }
    game_message(game,EVENT_COMMAND,"Arrow added at the REAR. Enter the entire remembered route, then Space launches the bot.");
    return true;
}

bool game_step_bot(Game *game)
{
    if(game->room!=1||game->courier.phase!=COURIER_RUN||game->room_complete) return false;
    Command command;
    if(!queue_dequeue(&game->queue,&command)) {
        game->running=false; game->courier.phase=COURIER_CRASH; game->courier.clock=0;
        game_message(game,EVENT_CRASH,"The command device failed mid-run. The bot was hit! Check dequeue, then R for a fresh route."); return false;
    }
    Cell target=game->bot;
    if(command>=CMD_UP&&command<=CMD_LEFT) {
        target.x+=dxs[command]; target.y+=dys[command];
    }
    CourierState *c=&game->courier;
    if(command<CMD_UP||command>CMD_LEFT||!same(target,c->path[c->step+1])) {
        game->bot=target; game->running=false; c->phase=COURIER_CRASH; c->clock=0;
        game_message(game,EVENT_CRASH,"WRONG STEP! Security fired on the bot. This attempt is over. R generates a new route."); return false;
    }
    game->bot=target; c->step++;
    if(c->step==c->length) {
        c->phase=COURIER_DELIVERED; game->delivered=true; game->running=false; game->interacting=false;
        queue_init(&game->queue);
        game_message(game,EVENT_POWER,"PERFECT RUN! The bot delivered the power cell. Walk Caitlyn right into EXIT.");
    } else game_message(game,EVENT_COMMAND,"Oldest arrow dequeued. The bot follows your remembered route...");
    return true;
}

void game_run_bot(Game *game)
{
    if(game->room!=1||game->room_complete) return;
    if(game->courier.phase==COURIER_WAITING) { game_memory_reveal(game); return; }
    if(game->courier.phase==COURIER_REVEAL) { game_memory_hide(game); return; }
    if(game->courier.phase!=COURIER_PLAN) return;
    if(game->queue.count!=game->courier.length) {
        game_message(game,EVENT_BUMP,"Queue EVERY move before launching. The panel shows how many arrows remain."); return;
    }
    game->courier.phase=COURIER_RUN; game->courier.clock=0;
    game->running=true; game->command_clock=0; game->hint=false;
    game_message(game,EVENT_COMMAND,"BOT LAUNCHED! Your queue runs from FRONT to REAR. One wrong step triggers the security laser.");
}

void game_clear_commands(Game *game)
{
    if(game->room!=1||game->courier.phase!=COURIER_PLAN||!game->queue.count) return;
    queue_init(&game->queue);
    game_message(game,EVENT_NONE,"Plan cleared. The hidden route stays hidden. R starts a new attempt with a fresh preview.");
}

void game_backspace_command(Game *game)
{
    if(game->room!=1||game->courier.phase!=COURIER_PLAN||!game->queue.count) return;
    CommandQueue replacement; queue_init(&replacement);
    for(int i=0;i<game->queue.count-1;i++)
        if(!queue_enqueue(&replacement,game->queue.items[(game->queue.front+i)%COMMAND_CAPACITY])) return;
    game->queue=replacement;
    game_message(game,EVENT_NONE,"Last planned arrow removed. Earlier entries keep their FIFO order.");
}

static const char *patterns[DUEL_WAVES] = {"([", "{([", "([{("};

const char *game_wave_pattern(int wave)
{ return wave >= 0 && wave < DUEL_WAVES ? patterns[wave] : ""; }

char game_attack_opener(int id)
{
    int wave = id / 16, at = id % 16;
    if (id < 0 || wave >= DUEL_WAVES || at >= (int)strlen(patterns[wave])) return '?';
    return patterns[wave][at];
}

static char closes(char opening)
{ return opening == '(' ? ')' : opening == '[' ? ']' : opening == '{' ? '}' : '?'; }

char game_expected_counter(const Game *game)
{
    return game->attacks ? closes(game->attacks->opening) : '?';
}

bool game_duel_start(Game *game)
{
    if (game->room != 2 || game->room_complete || game->duel.phase != DUEL_WAITING) return false;
    if (!remember(game)) return false;
    game->duel.phase = DUEL_INCOMING;
    game->duel.clock = 0;
    game->duel.last_attack_id = game->duel.wave*16;
    game_message(game, EVENT_ATTACK, "Watch the alien's whole sequence. Openings push onto its stack; type your whole response into the queue.");
    return true;
}

bool game_queue_counter(Game *game, char closing)
{
    if(game->room!=2||game->duel.phase!=DUEL_BLOCK||counter_symbol(counter_command(closing))=='?') return false;
    int needed=attack_copy_ids(game->attacks,(int[ATTACK_CAPACITY]){0},ATTACK_CAPACITY);
    if(game->counter_queue.count>=needed) {
        game_message(game,EVENT_BUMP,"The response is full. Backspace edits the last symbol; C clears it. Enter launches the whole sequence."); return false;
    }
    if(!remember(game)) return false;
    if(!queue_enqueue(&game->counter_queue,counter_command(closing))) { game->history.count--; return false; }
    game_message(game,EVENT_COMMAND,"Counter enqueued left to right. Fill the sequence, then press Enter to launch it.");
    return true;
}

void game_clear_counters(Game *game)
{
    if(game->room!=2||game->duel.phase!=DUEL_BLOCK||!game->counter_queue.count) return;
    if(!remember(game)) return;
    queue_init(&game->counter_queue);
    game_message(game,EVENT_NONE,"Counter queue cleared. The enemy stack is unchanged.");
}

void game_backspace_counter(Game *game)
{
    if(game->room!=2||game->duel.phase!=DUEL_BLOCK||!game->counter_queue.count) return;
    if(!remember(game)) return;
    CommandQueue replacement; queue_init(&replacement);
    for(int i=0;i<game->counter_queue.count-1;i++) {
        Command c=game->counter_queue.items[(game->counter_queue.front+i)%COMMAND_CAPACITY];
        if(!queue_enqueue(&replacement,c)) { game->history.count--; return; }
    }
    game->counter_queue=replacement;
    game_message(game,EVENT_NONE,"Last typed counter removed. Earlier symbols keep their left-to-right order.");
}

static bool return_next_counter(Game *game)
{
    if(!game->attacks) return false;
    CommandQueue before=game->counter_queue; Command command;
    if(!queue_dequeue(&game->counter_queue,&command)) return false;
    int id=game->attacks->id;
    if(counter_symbol(command)=='?'||!attack_remove(&game->attacks,id)) {
        game->counter_queue=before; return false;
    }
    game->duel.last_attack_id=id; game->duel.last_counter=counter_symbol(command);
    game->duel.last_block_ok=true; game->duel.wave_blocks++; game->duel.total_blocks++;
    game->duel.phase=DUEL_COUNTER; game->duel.clock=0;
    game_message(game,EVENT_BLOCK,"Clash! Pop the enemy stack; dequeue Caitlyn's oldest response. The matching shot returns to the alien.");
    return true;
}

bool game_submit_counters(Game *game)
{
    if(game->room!=2||game->duel.phase!=DUEL_BLOCK) return false;
    int count=attack_copy_ids(game->attacks,(int[ATTACK_CAPACITY]){0},ATTACK_CAPACITY);
    if(count<=0||game->counter_queue.count!=count) {
        game_message(game,EVENT_BUMP,"Enter one closing symbol for EACH opening, then launch with Enter."); return false;
    }
    if(!shield_accepts(game->attacks,&game->counter_queue)) {
        game->duel.mistakes++; game->duel.clock=0; game->duel.last_block_ok=false;
        game_message(game,EVENT_BUMP,"Sequence mismatch. Read your QUEUE from the front against the enemy STACK from the top. C clears; Backspace edits.");
        return false;
    }
    if(!remember(game)) return false;
    if(!return_next_counter(game)) { game->history.count--; return false; }
    return true;
}

bool game_finisher(Game *game)
{
    if (game->room != 2 || game->room_complete || game->duel.phase != DUEL_READY ||
        game->duel.total_blocks != DUEL_ATTACK_TOTAL || game->attacks ||
        game->duel.wave != DUEL_WAVES-1) {
        game_message(game,EVENT_BUMP,"The finishing move needs ALL nine attacks blocked first."); return false;
    }
    if (!game_regulator_ok() || !game_seal_ok()) {
        game_message(game,EVENT_BUMP,"The finishing controller is unsafe. Repair the source logic before firing."); return false;
    }
    if (!remember(game)) return false;
    game->duel.phase=DUEL_FINISH; game->duel.clock=0;
    game_message(game,EVENT_FINISH,"Special move: HOMEBOUND! Caitlyn channels the safe reserve into one final beam.");
    return true;
}

void game_tick(Game *game, float dt)
{
    if (!(dt>0) || game->room_complete) return;
    if(game->room==1) {
        CourierState *c=&game->courier;
        c->clock+=dt;
        if(c->phase==COURIER_REVEAL&&c->clock>=MEMORY_REVEAL_SECONDS) {
            game_memory_hide(game);
        } else if(c->phase==COURIER_RUN) {
            game->command_clock+=dt;
            if(game->command_clock>=.35f) { game->command_clock-=.35f; game_step_bot(game); }
        }
        return;
    }
    if (game->room == 2) {
        DuelState *d=&game->duel;
        d->clock+=dt;
        if (d->phase==DUEL_INCOMING && d->clock>=1.0f) {
            int count=(int)strlen(patterns[d->wave]);
            /* Read the enemy sequence LEFT TO RIGHT, pushing each opening at
             * the linked stack's head. The final opening becomes the top. */
            for(int i=0;i<count;i++) if(!attack_insert_after(&game->attacks,-1,d->wave*16+i,patterns[d->wave][i])) {
                attack_free(&game->attacks); d->phase=DUEL_WAITING; d->clock=0;
                game_message(game,EVENT_BUMP,"The incoming sequence could not be pushed onto the stack. Repair insertion or restart."); return;
            }
            d->sent=count; d->phase=DUEL_BLOCK; d->clock=0;
            queue_init(&game->counter_queue);
            game_message(game,EVENT_ATTACK,"Type the entire counter sequence left to right: 1 = ), 2 = ], 3 = }. Enter launches; C clears.");
        } else if (d->phase==DUEL_COUNTER && d->clock>=.55f) {
            d->clock=0;
            if (game->attacks) {
                if(!return_next_counter(game)) {
                    d->phase=DUEL_BLOCK;
                    game_message(game,EVENT_BUMP,"The next stack/queue pair could not be consumed. Check your list removal and dequeue code.");
                }
            } else if (d->wave<DUEL_WAVES-1) {
                d->phase=DUEL_GAP;
                game_message(game,EVENT_BLOCK,"Sequence blocked! The alien charges a longer nested attack.");
            } else {
                d->phase=DUEL_READY;
                game_message(game,EVENT_POWER,"All three sequences blocked: nine matched pairs. Press F for HOMEBOUND.");
            }
        } else if (d->phase==DUEL_GAP && d->clock>=.9f) {
            d->wave++; d->sent=0; d->wave_blocks=0; d->clock=0;
            d->phase=DUEL_INCOMING; d->last_attack_id=d->wave*16;
            game_message(game,EVENT_ATTACK,"The next burst begins. Watch the opening types and their order.");
        } else if (d->phase==DUEL_FINISH && d->clock>=1.3f) {
            d->defeated=true; d->phase=DUEL_CLEARED; d->clock=0;
            game_message(game,EVENT_POWER,"The alien falls. The alley is clear! Hold Right / D to walk into HOME.");
        }
        return;
    }

}

bool game_next_room(Game *game)
{
    if (!game->room_complete || game->room == 2) return false;
    return game_load_room(game, game->room + 1);
}
