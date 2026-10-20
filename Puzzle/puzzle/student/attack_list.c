#include "structures.h"
#include "repair.h"
#include <stdlib.h>

/* WHAT THE LIST STORES
 * AttackNode { id, opening, next }: a unique ID, one opening bracket ( [ {,
 * and a link to the next node. The final node points to NULL.
 * The duel reads an entire enemy sequence LEFT TO RIGHT. Each opening is
 * inserted at the HEAD (predecessor -1), making that list a linked stack.
 * Example: enemy ([ becomes head '[' -> '(' -> NULL. Head is stack TOP.
 * A successful counter removes the head by ID. General insertion/removal
 * must still work at head, middle and tail; IDs are not list positions.
 * Replace the starter bodies, including repair_required(...) and unused
 * (void) lines. repair_required is supplied, not your assignment.
 */

bool attack_contains(const AttackNode *head, int id)
{
    for (const AttackNode *node = head; node; node = node->next)
        if (node->id == id) return true;
    return false;
}

bool attack_insert_after(AttackNode **head, int predecessor, int id, char opening)
{
    /* TASK 5: Allocate a new node and insert it AFTER the node whose ID is
     * predecessor, preserving its old successor. -1 means before the head.
     * ID 0 -> ID 2; add ID 1 after ID 0 => 0 -> 1 -> 2.
     * Store both id and opening. Reject an opening other than ( [ {.
     * Return true on success. Duplicate ID, missing predecessor, NULL head
     * argument or allocation failure: return false with the list unchanged. */
    (void)head;
    (void)predecessor;
    (void)id; (void)opening;
    repair_required(REPAIR_LIST_INSERT);
    return false;
}

bool attack_remove(AttackNode **head, int id)
{
    /* TASK 6: Find id, unlink it, free exactly that node, and return true.
     * IDs 0 -> 1 -> 2; remove 1 => 0 -> 2.
     * Handle head, middle, tail and final-node removal. Missing ID or NULL
     * head argument: return false without changing the list. */
    (void)head;
    (void)id;
    repair_required(REPAIR_LIST_REMOVE);
    return false;
}

int attack_copy_ids(const AttackNode *head, int *out, int capacity)
{
    if (!out || capacity < 0) return -1;
    int count = 0;
    for (const AttackNode *node = head; node; node = node->next) {
        if (count >= capacity) return -1;
        out[count++] = node->id;
    }
    return count;
}

void attack_free(AttackNode **head)
{
    if (!head) return;
    while (*head) {
        AttackNode *next = (*head)->next;
        free(*head);
        *head = next;
    }
}
