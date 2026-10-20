#ifndef PORTAL_GAME_H
#define PORTAL_GAME_H

#include "structures.h"
#include "repair.h"
#include "diagnostics.h"

#define MAP_W 12
#define MAP_H 8

typedef enum { EVENT_NONE, EVENT_MOVE, EVENT_PUSH, EVENT_UNDO, EVENT_BUMP,
               EVENT_COMMAND, EVENT_CRASH, EVENT_ATTACK, EVENT_BLOCK, EVENT_FINISH,
               EVENT_POWER, EVENT_COMPLETE } GameEvent;

typedef struct {
    int room, unlocked, moves;
    Cell hero, crate, bot;
    CommandQueue queue, counter_queue;
    History history;
    AttackNode *attacks;
    DuelState duel;
    CourierState courier;
    uint32_t random_state;
    bool delivered, interacting, running, room_complete, won, hint;
    float command_clock;
    char message[180];
    GameEvent event;
    unsigned event_serial;
} Game;

void game_init(Game *game, int room);
void game_init_seeded(Game *game, int room, uint32_t seed);
bool game_memory_reveal(Game *game);
bool game_memory_hide(Game *game);
void game_backspace_command(Game *game);
void game_destroy(Game *game);
bool game_load_room(Game *game, int room);
bool game_move(Game *game, int dx, int dy);
bool game_undo(Game *game);
bool game_interact(Game *game);
bool game_enqueue(Game *game, Command command);
bool game_step_bot(Game *game);
void game_run_bot(Game *game);
void game_clear_commands(Game *game);
void game_tick(Game *game, float dt);
bool game_duel_start(Game *game);
bool game_queue_counter(Game *game, char closing);
bool game_submit_counters(Game *game);
void game_clear_counters(Game *game);
void game_backspace_counter(Game *game);
bool game_finisher(Game *game);
char game_attack_opener(int id);
char game_expected_counter(const Game *game);
const char *game_wave_pattern(int wave);
bool game_next_room(Game *game);
bool game_ready(const Game *game);
bool game_reserve_reading(int *out);
bool game_regulator_ok(void);
bool game_seal_ok(void);
extern const int portal_probe_readings[5];
char game_tile(int room, int x, int y);
Cell game_exit(int room);
const char *command_name(Command command);
void game_message(Game *game, GameEvent event, const char *message);

#endif
