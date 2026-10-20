#include "structures.h"
#include "repair.h"

/* WHAT THE QUEUE STORES
 * One Command per cell: CMD_UP / RIGHT / DOWN / LEFT for the courier;
 * CMD_BLOCK_ROUND / SQUARE / CURLY for the duel (meaning ) / ] / }).
 * front = oldest pending command; rear = last inserted; count = number waiting.
 * Empty: front = rear = -1 and count = 0. All COMMAND_CAPACITY (64) cells work.
 * After the random route disappears, record the WHOLE remembered plan.
 * The engine launches only when the complete route is queued, then moves the
 * bot after each dequeue. No single-step mode; one wrong arrow destroys it.
 * Replace the starter bodies, including repair_required(...) and unused
 * (void) lines. repair_required is a supplied marker, not your assignment.
 */

void queue_init(CommandQueue *queue)
{
    if (!queue) return;
    *queue = (CommandQueue){ .front = -1, .rear = -1, .count = 0 };
}

bool queue_enqueue(CommandQueue *queue, Command command)
{
    /* TASK 3: Add command at the rear, wrapping around the array; return true.
     * The first command occupies slot 0. RIGHT then UP must wait in that order.
     * NULL queue, an invalid Command or a full queue returns false unchanged. */
    (void)queue;
    (void)command;
    repair_required(REPAIR_QUEUE_ENQUEUE);
    return false;
}

bool queue_dequeue(CommandQueue *queue, Command *out)
{
    /* TASK 4: Write the oldest Command into *out, remove it, and return true.
     * Advance front circularly. Last removal restores front = rear = -1.
     * Empty/NULL input returns false without changing the queue or *out.
     * This function retrieves a Command; the engine moves or fires afterward. */
    (void)queue;
    (void)out;
    repair_required(REPAIR_QUEUE_DEQUEUE);
    return false;
}

bool queue_peek(const CommandQueue *queue, Command *out)
{
    if (!queue || !out || queue->count <= 0 ||
        queue->count > COMMAND_CAPACITY) return false;
    *out = queue->items[queue->front];
    return true;
}
