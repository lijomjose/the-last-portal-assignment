#include "structures.h"
#include "repair.h"

/* WHAT THE STACK STORES
 * The engine passes a BoardState containing hero/crate/bot positions, both queues,
 * attack IDs/openings/count, CourierState, delivered flag and DuelState BEFORE an action. Save that whole value,
 * not the pointer, a key press, a direction, or just the hero's coordinates.
 * History.items is a fixed array; count is the number of occupied entries.
 * No keyboard, drawing, or board-restoration code belongs in these functions.
 *
 * STARTER LINES
 * repair_required(...) is a supplied engine helper that displays the task.
 * Do not implement it. Replace each starter body, including that helper call.
 * (void)x just silences unused-parameter warnings. Remove it when x is used.
 * true means success; false means a rejected operation, not a NULL pointer.
 */

void history_init(History *history)
{
    if (history) history->count = 0;
}

bool history_push(History *history, const BoardState *state)
{
    /* TASK 1: Save a complete value copy of *state at the top, increasing
     * count exactly once; return true. NULL arguments or a full history return
     * false without changing it. Capacity: HISTORY_CAPACITY (128).
     * Saving A then B must preserve both even if the live board later changes. */
    (void)history;
    (void)state;
    repair_required(REPAIR_STACK_PUSH);
    return false;
}

bool history_pop(History *history, BoardState *out)
{
    /* TASK 2: Copy the newest complete saved state into *out, remove that
     * entry from the occupied stack, and return true. Saving A then B means
     * pops return B, then A. The engine restores the board from *out.
     * NULL arguments or count == 0 return false, leaving history/out unchanged. */
    (void)history;
    (void)out;
    repair_required(REPAIR_STACK_POP);
    return false;
}
