#include "view.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Review captures use the same entry and submit actions as a player. */
static void enter_matching_sequence(Game *g)
{
    for(const AttackNode *node=g->attacks;node;node=node->next)
        game_queue_counter(g,node->opening=='('?')':node->opening=='['?']':'}');
    game_submit_counters(g);
}

static int read_checkpoint(const char *path)
{
    FILE *file=fopen(path,"r");
    if(!file) return 0;
    int room=0;
    if(fscanf(file,"LAST_PORTAL_1 %d",&room)!=1 || room<0 || room>2) room=0;
    fclose(file); return room;
}

static bool write_checkpoint(const char *path, int room)
{
    FILE *file=fopen(path,"w");
    if(!file) return false;
    bool written=fprintf(file,"LAST_PORTAL_1 %d\n",room)>0;
    if(fclose(file)!=0) written=false;
    return written;
}

#if !PORTAL_PUZZLE
/* Instructor screenshot driver; absent from the student executable. */
static void solve_for_review(Game *g)
{
    if(g->room==0) {
        game_undo(g);
        const Cell route[]={ {-1,0},{0,-1},{0,-1},{1,0},{0,1},{0,1},
                            {1,0},{0,1},{-1,0},{-1,0},{-1,0} };
        for(unsigned i=0;i<sizeof(route)/sizeof(route[0]);i++) game_move(g,route[i].x,route[i].y);
    } else if(g->room==1) {
        game_memory_reveal(g); game_tick(g,MEMORY_REVEAL_SECONDS+.01f);
        for(int i=0;i<g->courier.length;i++) {
            Cell a=g->courier.path[i], b=g->courier.path[i+1];
            game_enqueue(g,b.x>a.x?CMD_RIGHT:b.x<a.x?CMD_LEFT:b.y>a.y?CMD_DOWN:CMD_UP);
        }
        game_run_bot(g);
        for(int i=0;i<g->courier.length;i++) game_tick(g,.36f);
    } else {
        game_duel_start(g);
        for(int guard=0;guard<200&&g->duel.phase!=DUEL_READY;guard++) {
            if(g->duel.phase==DUEL_BLOCK) enter_matching_sequence(g);
            else game_tick(g,.4f);
        }
    }
}
#endif

static void apply_action(UiAction action, Game *g, View *v, Screen *screen, int *checkpoint, bool *quit)
{
    switch(action.kind) {
        case UI_CONTINUE: view_start_story(v,g->room); *screen=SCREEN_STORY; break;
        case UI_NEW:
            if(game_load_room(g,0)) { g->unlocked=0; *checkpoint=0; view_start_story(v,0); *screen=SCREEN_STORY; }
            break;
        case UI_QUIT: *quit=true; break;
        case UI_RESUME: *screen=SCREEN_PLAY; break;
        case UI_PAUSE: *screen=SCREEN_PAUSE; break;
        case UI_NEXT:
            if(game_next_room(g)) { view_start_story(v,g->room); *screen=SCREEN_STORY; }
            break;
        case UI_UNDO: game_undo(g); break;
        case UI_RESET: game_load_room(g,g->room); *screen=SCREEN_PLAY; break;
        case UI_CONSOLE: game_interact(g); break;
        case UI_RUN: game_run_bot(g); break;
        case UI_BACKSPACE: game_backspace_command(g); break;
        case UI_CLEAR: game_clear_commands(g); break;
        case UI_COMMAND: game_enqueue(g,(Command)action.arg); break;
        case UI_DUEL_START: game_duel_start(g); break;
        case UI_BLOCK:
            game_queue_counter(g,(char)action.arg);
            break;
        case UI_SUBMIT:
            if(g->duel.phase==DUEL_BLOCK&&repair_check_shield()) game_submit_counters(g);
            break;
        case UI_COUNTER_CLEAR: game_clear_counters(g); break;
        case UI_COUNTER_BACKSPACE: game_backspace_counter(g); break;
        case UI_FINISH:
            if(g->duel.phase!=DUEL_READY||repair_check_finisher()) game_finisher(g);
            break;
        case UI_HINT: g->hint=!g->hint; break;
        case UI_MUTE: v->muted=!v->muted; break;
        case UI_DIAGNOSIS_HINT: repair_toggle_clue(); break;
        case UI_GUIDE: v->guide=!v->guide; break;
        case UI_STORY: view_start_story(v,g->room); *screen=SCREEN_STORY; break;
        case UI_STORY_BEGIN: *screen=SCREEN_PLAY; break;
        case UI_STORY_PAUSE: v->story_paused=!v->story_paused; break;
        case UI_STORY_RESTART: view_start_story(v,g->room); break;
        case UI_NONE: break;
    }
}

static bool pressed(int key) { return IsKeyPressed(key)||IsKeyPressedRepeat(key); }

static void keyboard(Game *g, View *v, Screen *screen, int *checkpoint, bool *quit)
{
    if(IsKeyPressed(KEY_F1)) { v->guide=!v->guide; return; }
    if(IsKeyPressed(KEY_F2)&&*screen!=SCREEN_TITLE&&!g->room_complete) {
        v->guide=false;
        view_start_story(v,g->room); *screen=SCREEN_STORY; return;
    }
    if(v->guide) {
        if(IsKeyPressed(KEY_ESCAPE)) v->guide=false;
        return;
    }
    if(*screen==SCREEN_STORY) {
        if(IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_ESCAPE)) *screen=SCREEN_PLAY;
        else if(IsKeyPressed(KEY_SPACE)) v->story_paused=!v->story_paused;
        else if(IsKeyPressed(KEY_R)) view_start_story(v,g->room);
        if(IsKeyPressed(KEY_Q)) *quit=true;
        return;
    }
    if(repair_is_blocked()) {
        if(IsKeyPressed(KEY_H)) repair_toggle_clue();
        if(IsKeyPressed(KEY_Q)||IsKeyPressed(KEY_ESCAPE)) *quit=true;
        return;
    }
    if(IsKeyPressed(KEY_M)) v->muted=!v->muted;
    if(*screen==SCREEN_TITLE) {
        if(IsKeyPressed(KEY_ENTER)) apply_action((UiAction){UI_CONTINUE,0},g,v,screen,checkpoint,quit);
        if(IsKeyPressed(KEY_N)) apply_action((UiAction){UI_NEW,0},g,v,screen,checkpoint,quit);
        if(IsKeyPressed(KEY_Q)) *quit=true;
        return;
    }
    if(IsKeyPressed(KEY_ESCAPE)) { *screen=*screen==SCREEN_PAUSE?SCREEN_PLAY:SCREEN_PAUSE; return; }
    if(*screen==SCREEN_PAUSE) {
        if(IsKeyPressed(KEY_Q)) *quit=true;
        if(IsKeyPressed(KEY_R)) apply_action((UiAction){UI_RESET,0},g,v,screen,checkpoint,quit);
        return;
    }
    if(g->room_complete) {
        if(IsKeyPressed(KEY_ENTER)) apply_action((UiAction){g->won?UI_NEW:UI_NEXT,0},g,v,screen,checkpoint,quit);
        return;
    }
    if(IsKeyPressed(KEY_H)) g->hint=!g->hint;
    if(IsKeyPressed(KEY_Z)) { game_undo(g); return; }
    if(IsKeyPressed(KEY_R)) { game_load_room(g,g->room); return; }
    if(IsKeyPressed(KEY_E)) { game_interact(g); return; }
    if(g->room==1&&g->interacting) {
        if(IsKeyPressed(KEY_SPACE)) game_run_bot(g);
        if(IsKeyPressed(KEY_ENTER)) game_run_bot(g);
        if(IsKeyPressed(KEY_BACKSPACE)) game_backspace_command(g);
        if(IsKeyPressed(KEY_C)) game_clear_commands(g);
        if(pressed(KEY_UP)||pressed(KEY_W)) game_enqueue(g,CMD_UP);
        else if(pressed(KEY_RIGHT)||pressed(KEY_D)) game_enqueue(g,CMD_RIGHT);
        else if(pressed(KEY_DOWN)||pressed(KEY_S)) game_enqueue(g,CMD_DOWN);
        else if(pressed(KEY_LEFT)||pressed(KEY_A)) game_enqueue(g,CMD_LEFT);
    } else if(g->room==2) {
        if(IsKeyPressed(KEY_SPACE)) game_duel_start(g);
        if(IsKeyPressed(KEY_F)) apply_action((UiAction){UI_FINISH,0},g,v,screen,checkpoint,quit);
        if(IsKeyPressed(KEY_ENTER)) apply_action((UiAction){UI_SUBMIT,0},g,v,screen,checkpoint,quit);
        if(IsKeyPressed(KEY_C)) game_clear_counters(g);
        if(IsKeyPressed(KEY_BACKSPACE)) game_backspace_counter(g);
        int typed=0, value;
        while((value=GetCharPressed())>0) if(value==')'||value==']'||value=='}') { if(!typed) typed=value; }
        if(!typed) {
            if(IsKeyPressed(KEY_ONE)) typed=')';
            else if(IsKeyPressed(KEY_TWO)) typed=']';
            else if(IsKeyPressed(KEY_THREE)) typed='}';
        }
        if(typed) apply_action((UiAction){UI_BLOCK,typed},g,v,screen,checkpoint,quit);
        if(pressed(KEY_RIGHT)||pressed(KEY_D)) game_move(g,1,0);
        else if(pressed(KEY_LEFT)||pressed(KEY_A)) game_move(g,-1,0);
    } else {
        if(pressed(KEY_UP)||pressed(KEY_W)) game_move(g,0,-1);
        else if(pressed(KEY_RIGHT)||pressed(KEY_D)) game_move(g,1,0);
        else if(pressed(KEY_DOWN)||pressed(KEY_S)) game_move(g,0,1);
        else if(pressed(KEY_LEFT)||pressed(KEY_A)) game_move(g,-1,0);
    }
}

int main(int argc, char **argv)
{
    const char *capture=NULL, *save=PORTAL_SAVE_FILE;
    bool title=true, solved=false, ending=false, audio=true, hidden=false, guide=false, clue_preview=false, story=false;
    float story_at=0;
    const char *duel_phase=NULL, *diagnose=NULL, *memory_phase=NULL;
    uint32_t seed=0; bool seeded=false;
    int requested_room=-1;
    for(int i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--capture")&&i+1<argc) { capture=argv[++i]; title=false; audio=false; }
        else if(!strcmp(argv[i],"--room")&&i+1<argc) requested_room=atoi(argv[++i]);
        else if(!strcmp(argv[i],"--save")&&i+1<argc) save=argv[++i];
        else if(!strcmp(argv[i],"--title")) title=true;
#if !PORTAL_PUZZLE
        else if(!strcmp(argv[i],"--solved")) solved=true;
        else if(!strcmp(argv[i],"--ending")) { requested_room=2; solved=true; ending=true; }
#endif
        else if(!strcmp(argv[i],"--silent")) audio=false;
        else if(!strcmp(argv[i],"--hidden")) hidden=true;
        else if(!strcmp(argv[i],"--seed")&&i+1<argc) {
            char *end=NULL; unsigned long long value=strtoull(argv[++i],&end,10);
            if(*end||value>UINT32_MAX||!value) { fputs("Seed must be an integer from 1 to 4294967295.\n",stderr); return 2; }
            seed=(uint32_t)value; seeded=true;
        }
        else if(!strcmp(argv[i],"--memory-phase")&&i+1<argc) { memory_phase=argv[++i]; requested_room=1; }
        else if(!strcmp(argv[i],"--duel-phase")&&i+1<argc) { duel_phase=argv[++i]; requested_room=2; }
        else if(!strcmp(argv[i],"--diagnose")&&i+1<argc) diagnose=argv[++i];
        else if(!strcmp(argv[i],"--guide")) guide=true;
        else if(!strcmp(argv[i],"--clue")) clue_preview=true;
        else if(!strcmp(argv[i],"--story")) story=true;
        else if(!strcmp(argv[i],"--story-at")&&i+1<argc) {
            char *end=NULL; story_at=strtof(argv[++i],&end); story=true;
            if(*end||!isfinite(story_at)||story_at<0||story_at>120) {
                fputs("Story capture time must be between 0 and 120 seconds.\n",stderr); return 2;
            }
        }
        else if(!strcmp(argv[i],"--help")) {
            puts("The Last Portal: Enter begins/skips the story; F1 explains the mission, F2 replays its story. WASD moves; Z rewinds; E interacts.");
            puts("Review: --capture PATH [--room 0|1|2] [--title]. Puzzle progress requires working source code.");
            return 0;
        } else { fprintf(stderr,"Unknown or incomplete option: %s\n",argv[i]); return 2; }
    }
    if(requested_room < -1 || requested_room > 2) { fputs("Room must be 0, 1, or 2.\n",stderr); return 2; }
    if((duel_phase||diagnose||memory_phase)&&!capture) { fputs("Phase previews require --capture.\n",stderr); return 2; }
    if(clue_preview&&!capture) { fputs("--clue is only available for screenshot review.\n",stderr); return 2; }
    if(story_at>0&&!capture) { fputs("--story-at is only available for screenshot review.\n",stderr); return 2; }
    /* Scripts start in the project root. This also supports launching build/ directly. */
    if(!DirectoryExists("assets")) {
        ChangeDirectory(GetApplicationDirectory());
        for(int i=0;i<4&&!DirectoryExists("assets");i++) ChangeDirectory("..");
    }
    int checkpoint=capture?0:read_checkpoint(save);
    Game *g=calloc(1,sizeof(*g));
    View *v=calloc(1,sizeof(*v));
    if(!g||!v) { free(g); free(v); return 1; }
    if(seeded) game_init_seeded(g,requested_room>=0?requested_room:checkpoint,seed);
    else game_init(g,requested_room>=0?requested_room:checkpoint);
    bool preview_checked=false;
    if(memory_phase&&repair_check_through(1)) {
        bool crashed=!strcmp(memory_phase,"crash")||!strcmp(memory_phase,"wreck");
        preview_checked=true; game_memory_reveal(g);
        if(!strcmp(memory_phase,"reveal")) game_tick(g,1.5f);
        else {
            game_tick(g,MEMORY_REVEAL_SECONDS+.01f);
            if(strcmp(memory_phase,"plan")) {
                for(int i=0;i<g->courier.length;i++) {
                    Cell a=g->courier.path[i], b=g->courier.path[i+1];
                    game_enqueue(g,crashed&&i==0?CMD_LEFT:b.x>a.x?CMD_RIGHT:b.x<a.x?CMD_LEFT:b.y>a.y?CMD_DOWN:CMD_UP);
                }
                if(strcmp(memory_phase,"queued")) {
                    game_run_bot(g);
                    if(crashed) { game_step_bot(g); game_tick(g,!strcmp(memory_phase,"wreck")?1.1f:.05f); }
                    else if(!strcmp(memory_phase,"run")) game_tick(g,.7f);
                    else for(int i=0;i<g->courier.length;i++) game_tick(g,.36f);
                }
            }
        }
    }
    if(duel_phase&&repair_check_through(2)) {
        preview_checked=true;
        game_duel_start(g);
        if(!strcmp(duel_phase,"incoming")) game_tick(g,.4f);
        else {
            for(int guard=0;guard<200;guard++) {
                if(g->duel.phase==DUEL_BLOCK&&(!strcmp(duel_phase,"block")||!strcmp(duel_phase,"queued")||!strcmp(duel_phase,"counter"))) break;
                if(g->duel.phase==DUEL_READY) break;
                if(g->duel.phase==DUEL_BLOCK) enter_matching_sequence(g);
                else game_tick(g,.4f);
            }
            if(!strcmp(duel_phase,"queued"))
                for(const AttackNode *node=g->attacks;node;node=node->next)
                    game_queue_counter(g,node->opening=='('?')':node->opening=='['?']':'}');
            if(!strcmp(duel_phase,"counter")) { enter_matching_sequence(g); game_tick(g,.25f); }
            if(!strcmp(duel_phase,"finish")||!strcmp(duel_phase,"cleared")) {
                game_finisher(g); game_tick(g,!strcmp(duel_phase,"finish")?.65f:1.4f);
            }
        }
        if(diagnose) {
            if(!strcmp(diagnose,"shield")) repair_check_shield();
            else if(!strcmp(diagnose,"finisher")) repair_check_finisher();
        }
    }
#if !PORTAL_PUZZLE
    if(solved) solve_for_review(g);
    if(ending) {
        game_finisher(g); game_tick(g,1.4f);
        for(int i=0;i<8;i++) game_move(g,1,0);
    }
#else
    (void)solved; (void)ending;
#endif
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|((capture||hidden)?FLAG_WINDOW_HIDDEN:FLAG_VSYNC_HINT));
    InitWindow(VIEW_W,VIEW_H,PORTAL_WINDOW_TITLE);
    if(!IsWindowReady()) { game_destroy(g); free(g); free(v); return 1; }
    SetWindowMinSize(960,615); SetExitKey(KEY_NULL); SetTargetFPS(capture?120:60);
    view_init(v,audio);
    v->guide=guide;
    RenderTexture2D target=LoadRenderTexture(VIEW_W,VIEW_H);
    SetTextureFilter(target.texture,TEXTURE_FILTER_BILINEAR);
    Screen screen=title?SCREEN_TITLE:SCREEN_PLAY;
    if(story) {
        view_start_story(v,g->room); v->story_elapsed=fminf(story_at,v->story_duration);
        screen=SCREEN_STORY;
    }
    bool quit=false; int frames=0, last_saved=checkpoint;
    int checked_room=preview_checked?g->room:-1;
    while(!WindowShouldClose()&&!quit) {
        float dt=capture?1.0f/60.0f:GetFrameTime();
        if(dt>.1f) dt=.1f;
        float scale=fminf((float)GetScreenWidth()/VIEW_W,(float)GetScreenHeight()/VIEW_H);
        Rectangle destination={ (GetScreenWidth()-VIEW_W*scale)/2,(GetScreenHeight()-VIEW_H*scale)/2,VIEW_W*scale,VIEW_H*scale };
        Vector2 mouse=GetMousePosition();
        v->mouse=(Vector2){(mouse.x-destination.x)/scale,(mouse.y-destination.y)/scale};
        if(screen==SCREEN_PLAY&&checked_room!=g->room) {
            repair_check_through(g->room); checked_room=g->room;
        }
        if(!capture) keyboard(g,v,&screen,&checkpoint,&quit);
        if(screen==SCREEN_PLAY&&checked_room!=g->room) {
            repair_check_through(g->room); checked_room=g->room;
        }
        if(screen==SCREEN_PLAY&&!g->room_complete&&!repair_is_blocked()&&!v->guide) game_tick(g,dt);
        if(capture&&clue_preview&&repair_is_blocked()&&!repair_report()->show_clue) repair_toggle_clue();
        view_update(v,g,screen,dt);
        if(!capture) {
            if(g->unlocked>checkpoint) checkpoint=g->unlocked;
            if(checkpoint!=last_saved) {
                if(!write_checkpoint(save,checkpoint))
                    game_message(g,EVENT_BUMP,"The checkpoint could not be saved. This session still works.");
                last_saved=checkpoint;
            }
        }
        BeginTextureMode(target);
        UiAction action=view_draw(v,g,screen,checkpoint);
        EndTextureMode();
        BeginDrawing(); ClearBackground((Color){8,13,23,255});
        DrawTexturePro(target.texture,(Rectangle){0,0,VIEW_W,-VIEW_H},destination,(Vector2){0,0},0,WHITE);
        EndDrawing();
        if(!capture) apply_action(action,g,v,&screen,&checkpoint,&quit);
        if(capture&&++frames>=12) {
            Image image=LoadImageFromTexture(target.texture);
            ImageFlipVertical(&image);
            bool result=ExportImage(image,capture);
            UnloadImage(image);
            if(!result) fprintf(stderr,"Could not save capture: %s\n",capture);
            quit=true;
        }
    }
    UnloadRenderTexture(target); view_destroy(v); CloseWindow();
    game_destroy(g); free(g); free(v);
    return 0;
}
