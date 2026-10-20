#ifndef PORTAL_STRUCTURES_H
#define PORTAL_STRUCTURES_H

#include <stdbool.h>
#include <stdint.h>

#define COMMAND_CAPACITY 64
#define MEMORY_PATH_MAX 14
#define MEMORY_REVEAL_SECONDS 30.0f
#define DUEL_SEQUENCE_MAX 4
#define HISTORY_CAPACITY 128
#define ATTACK_CAPACITY 8
#define DUEL_WAVES 3
#define DUEL_ATTACK_TOTAL 9

typedef enum { DUEL_WAITING, DUEL_INCOMING, DUEL_BLOCK, DUEL_COUNTER,
    DUEL_GAP, DUEL_READY, DUEL_FINISH, DUEL_CLEARED } DuelPhase;
typedef struct {
    DuelPhase phase;
    int wave, sent, wave_blocks, total_blocks, mistakes, last_attack_id;
    char last_counter;
    float clock;
    bool defeated, last_block_ok;
} DuelState;

typedef struct { int x, y; } Cell;
typedef enum { COURIER_WAITING, COURIER_REVEAL, COURIER_PLAN, COURIER_RUN,
    COURIER_CRASH, COURIER_DELIVERED } CourierPhase;
typedef struct {
    Cell path[MEMORY_PATH_MAX+1];
    int length, step, attempt;
    CourierPhase phase;
    float clock;
} CourierState;
typedef enum { CMD_UP, CMD_RIGHT, CMD_DOWN, CMD_LEFT,
    CMD_BLOCK_ROUND, CMD_BLOCK_SQUARE, CMD_BLOCK_CURLY } Command;

static inline char counter_symbol(Command command)
{ return command==CMD_BLOCK_ROUND?')':command==CMD_BLOCK_SQUARE?']':command==CMD_BLOCK_CURLY?'}':'?'; }
static inline Command counter_command(char symbol)
{ return symbol==')'?CMD_BLOCK_ROUND:symbol==']'?CMD_BLOCK_SQUARE:symbol=='}'?CMD_BLOCK_CURLY:(Command)-1; }

/* front is the first command; rear is the last inserted command.
 * Empty is -1/-1. All COMMAND_CAPACITY cells are usable. */
typedef struct {
    Command items[COMMAND_CAPACITY];
    int front, rear, count;
} CommandQueue;

void queue_init(CommandQueue *queue);
bool queue_enqueue(CommandQueue *queue, Command command);
bool queue_dequeue(CommandQueue *queue, Command *out);
bool queue_peek(const CommandQueue *queue, Command *out);

/* The engine supplies the complete board BEFORE an action. One snapshot is
 * one stack entry, not a direction, a key press, or just the hero position.
 * Snapshots own values, never pointers into a live attack list. */
typedef struct {
    Cell hero, crate, bot;
    CommandQueue queue, counter_queue;
    int attack_ids[ATTACK_CAPACITY], attack_count;
    char attack_openings[ATTACK_CAPACITY];
    bool delivered;
    DuelState duel;
    CourierState courier;
} BoardState;

typedef struct {
    BoardState items[HISTORY_CAPACITY];
    int count; /* Number saved; 0 is empty. Newest occupied index: count - 1. */
} History;

void history_init(History *history);
bool history_push(History *history, const BoardState *state);
bool history_pop(History *history, BoardState *out);

typedef struct AttackNode {
    int id; /* Unique attack identity, not a list position. */
    char opening; /* One of ( [ {. Head is the stack TOP. */
    struct AttackNode *next; /* NULL at the tail. */
} AttackNode;

/* predecessor -1 inserts at the head. IDs must be unique. */
bool attack_insert_after(AttackNode **head, int predecessor, int id, char opening);
bool attack_remove(AttackNode **head, int id);
bool attack_contains(const AttackNode *head, int id);
int attack_copy_ids(const AttackNode *head, int *out, int capacity);
void attack_free(AttackNode **head);

#endif
