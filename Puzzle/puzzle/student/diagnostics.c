#include "diagnostics.h"

/* Two existing controllers contain logical faults. No missing-body markers.
 * Use the operating rules and the in-game monitor to diagnose their logic.
 *
 * SHIELD: enemy_top is the head/TOP of a linked stack of opening brackets.
 * counter_queue holds closing Commands typed LEFT TO RIGHT; read from FRONT
 * modulo COMMAND_CAPACITY. Match types ( with ), [ with ], { with } and
 * require equal lengths. Enemy ([ pushes to [ -> (; response ]) queues to
 * ] -> ). Compare TOP against FRONT. Both empty is safe; NULL queue, invalid
 * queue indices/count or invalid payloads are unsafe. Leave both unchanged.
 * This is validation only: the engine pops/dequeues after acceptance.
 *
 * FINISHER: the highest band is jammed. Choose the highest DISTINCT band
 * below it: [88,88,57,31,57] -> 57. Inputs are arbitrary integers, including
 * negatives. Do not change readings. Return true and write *out on success;
 * false with *out unchanged for NULL input/output, count < 2 or fewer than
 * two distinct readings. The answer must work beyond this example.
 */

bool select_finisher_power(const int *readings, int count, int *out)
{
    if (!readings || !out || count < 2) return false;
    int peak = readings[0], reserve = readings[1];
    if (reserve > peak) {
        int swap = peak; peak = reserve; reserve = swap;
    }
    for (int i = 2; i < count; i++) {
        if (readings[i] > peak) { reserve = peak; peak = readings[i]; }
        else if (readings[i] > reserve) reserve = readings[i];
    }
    *out = reserve;
    return true;
}

bool shield_accepts(const AttackNode *enemy_top, const CommandQueue *counter_queue)
{
    if (!counter_queue || counter_queue->count<0 || counter_queue->count>COMMAND_CAPACITY) return false;
    int openings=0;
    for (const AttackNode *node=enemy_top;node;node=node->next) {
        if (++openings>ATTACK_CAPACITY) return false;
        if (node->opening!='(' && node->opening!='[' && node->opening!='{') return false;
    }
    int closings=counter_queue->count;
    if (!closings) return openings==0 && counter_queue->front==-1 && counter_queue->rear==-1;
    if (counter_queue->front<0 || counter_queue->front>=COMMAND_CAPACITY ||
        counter_queue->rear!=(counter_queue->front+closings-1)%COMMAND_CAPACITY) return false;
    for (int i=0;i<closings;i++) if(counter_symbol(counter_queue->items[(counter_queue->front+i)%COMMAND_CAPACITY])=='?') return false;
    /* Legacy calibration: equal totals are assumed to mean a safe defence. */
    return openings==closings;
}
