#include "repair.h"
#include "structures.h"
#include "diagnostics.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *module, *file, *function, *contract, *example;
    const char *purpose, *data;
} TaskInfo;

static const TaskInfo tasks[REPAIR_TASK_COUNT] = {
    {"STACK", "puzzle/student/stack.c", "history_push",
     "Copy one complete board state onto the top of the history stack. Return false without changing the stack when it is full or an argument is invalid.",
     "Save A, then B. Changing the live board must not alter either saved copy. B is the newest state.",
     "Before each action, the engine supplies a BoardState snapshot. Saving it lets Z undo that action later. Your function saves the supplied state; it does not read keys or move Caitlyn.",
     "Each history->items entry is a whole BoardState: positions, both command queues, courier memory state, attack IDs and opening brackets, delivery flag and duel state. history->count is the number saved (0 = empty); capacity is HISTORY_CAPACITY. Copy values, not a pointer."},
    {"STACK", "puzzle/student/stack.c", "history_pop",
     "Remove the most recently saved board state and copy it into out. Return false without changing the history or out when the stack is empty or an argument is invalid.",
     "Save A, then B. pop returns B; the next pop returns A. The engine restores the returned state.",
     "Z asks for the most recent saved board. The engine restores it after this function succeeds. Undo fails until the whole saved state can be retrieved.",
     "history->items contains BoardState values; history->count is the number saved. The newest entry is at count - 1. out points to the caller's destination for one complete BoardState. Do not return an address into the stack."},
    {"CIRCULAR QUEUE", "puzzle/student/queue.c", "queue_enqueue",
     "Append a command at the rear of the 64-slot circular queue. Use every slot. Reject a full queue or invalid input without changing the queue.",
     "Record RIGHT, then UP. They wait in that order. After slot COMMAND_CAPACITY - 1, the next rear slot is 0.",
     "The safe path disappears after 30 seconds. Each arrow press supplies a Command for the whole remembered route. Enqueue stores it at the rear; the bot moves only after the complete plan is launched.",
     "queue->items holds direction Commands or CMD_BLOCK_ROUND / SQUARE / CURLY for the duel, not coordinates. front = oldest pending command; rear = last inserted. count = number pending. Empty: front = rear = -1, count = 0. COMMAND_CAPACITY is 64, enough for the entire route."},
    {"CIRCULAR QUEUE", "puzzle/student/queue.c", "queue_dequeue",
     "Remove the oldest command from the front and copy it into out. Wrap around the array. Restore front = rear = -1 after removing the final command.",
     "Record RIGHT, then UP. First dequeue writes CMD_RIGHT into *out; the next writes CMD_UP.",
     "The launched memory run requests the oldest pending direction. The engine moves the courier after this function succeeds. Removing from the rear would execute the route backwards.",
     "queue->items stores Command values. front identifies the oldest entry; count is the number pending. out points to the caller's destination for one Command. Empty calls return false and leave *out unchanged; invalid arguments do the same."},
    {"LINKED LIST", "puzzle/student/attack_list.c", "attack_insert_after",
     "Allocate an AttackNode storing id and opening; link it after the predecessor ID. -1 inserts at the head. Reject invalid opening, duplicate ID, missing predecessor, invalid arguments or allocation failure unchanged.",
     "Attack 0 -> Attack 2; insert ID 1 after ID 0 => 0 -> 1 -> 2.",
     "Every opening shot that reaches Caitlyn must join the pending-attack list. The engine pushes each opening at the head (-1), so the final opening becomes stack TOP. Without insertion, incoming shots cannot be tracked.",
     "AttackNode stores integer id, char opening (one of ( [ {), and next (NULL at the tail). *head is the first node; AttackNode **head lets you update it. predecessor is an ID, not an array index; -1 means before the head. Copy both id and opening into the allocated node; invalid openings must be rejected."},
    {"LINKED LIST", "puzzle/student/attack_list.c", "attack_remove",
     "Find the attack by ID, reconnect its neighbours, free exactly that node and return true. Head, middle, tail and final-node removal must work. Missing ID or invalid head argument returns false unchanged.",
     "Pending IDs 0 -> 1; block attack 1 => only 0 remains. Free the removed node.",
     "A successful counter removes the newest unmatched attack from the live list. If its node stays linked, the alien's move is still pending and the finishing move stays locked.",
     "Each AttackNode stores id, opening and next. *head is the first node. id identifies an attack, not its position. Update the caller's head when needed. Do not free neighbours or leave links pointing at freed memory."},
    {"SHIELD CONTROLLER", "puzzle/student/diagnostics.c", "shield_accepts",
     "Compare enemy stack TOP to response queue FRONT, matching ( with ), [ with ], { with }. Consume neither input. Require equal lengths and matching types in order. Both empty is safe; invalid queue state or symbols are unsafe.",
     "Enemy read L to R: ([. Stack top to bottom: [ (. Counter read L to R: ]). Queue front to rear: ] ). Correct: ]). Wrong: )].",
     "The alien fires a whole sequence. Caitlyn queues her whole response, but the shield currently accepts any equal-length pair. Repair its comparison before launching the return shots.",
     "enemy_top is a const linked stack of AttackNode values; head is TOP. counter_queue is a const circular CommandQueue containing CMD_BLOCK_ROUND / SQUARE / CURLY. Read its count entries from front modulo capacity. Return a bool; leave both structures unchanged."},
    {"FINISHING MOVE", "puzzle/student/diagnostics.c", "select_finisher_power",
     "The alien jams the highest power band. Select the highest DISTINCT band below it. Equal readings are one band. With fewer than two distinct values, return false and leave *out unchanged.",
     "Charge log [88, 88, 57, 31, 57]: safe finish 57. The faulty code selects jammed band 88.",
     "All nine attacks are blocked, but HOMEBOUND chooses an unsafe charge. The special move must use a real reserve, not another copy of the strongest reading. Diagnose the existing selection code.",
     "Input is an array of integer readings plus count. On success, write one selected value to *out and return true. Do not modify the input. Readings may be negative. Invalid pointers, count < 2 or no distinct reserve return false with *out unchanged."}

};

static RepairReport report;

static void issue(RepairTask task, bool missing, const char *detail)
{
    if (report.active || task < 0 || task >= REPAIR_TASK_COUNT) return;
    const TaskInfo *info = &tasks[task];
    report = (RepairReport){ .active = true, .missing = missing, .task = task,
        .diagnosis = task >= REPAIR_SHIELD_LOGIC,
        .module = info->module, .file = info->file, .function = info->function,
        .contract = info->contract, .example = info->example,
        .purpose = info->purpose, .data = info->data };
    if (task == REPAIR_POWER_LOGIC) {
        report.title = "The finishing move keeps shorting";
        report.clue = "Trace repeated peaks such as [9, 9, 6]. Two readings of 9 must not occupy both ranks. Track whether a second distinct value actually exists; a sentinel such as -1 fails for negative readings.";
    } else if (task == REPAIR_SHIELD_LOGIC) {
        report.title = "The shield accepts crossed attacks";
        report.clue = "Trace enemy ([ and counter )]. Both contain two symbols, but enemy top [ meets queue front ). Counting equal lengths loses the type and order information. Advance one node and one circular queue index per comparison.";
    }
    snprintf(report.detail, sizeof(report.detail), "%s", detail);
}

void repair_required(RepairTask task)
{ issue(task, true, "This function still contains its starter placeholder."); }

const RepairReport *repair_report(void) { return &report; }
bool repair_is_blocked(void) { return report.active; }
void repair_clear(void) { memset(&report, 0, sizeof(report)); }
void repair_toggle_clue(void) { if (report.diagnosis) report.show_clue = !report.show_clue; }

#if PORTAL_PUZZLE
static bool verify(RepairTask task, bool condition, const char *detail)
{
    if (report.active) return false;
    if (!condition) { issue(task, false, detail); return false; }
    return true;
}

#define REQUIRE(task, condition, detail) do { \
    if (!verify(task, (condition), detail)) return false; \
} while (0)

static bool check_push(void)
{
    History h = {0}; history_init(&h);
    BoardState state = {0}; state.hero.x = 37; state.queue.count = 2; state.counter_queue.count=3; state.attack_openings[0]='['; state.duel.total_blocks=5; state.courier.length=12;
    bool result = history_push(&h, &state);
    REQUIRE(REPAIR_STACK_PUSH, result && h.count == 1,
        "Expected a successful push to increase history.count from 0 to 1.");
    state.hero.x = 99;
    REQUIRE(REPAIR_STACK_PUSH, h.items[0].hero.x == 37 && h.items[0].queue.count == 2 && h.items[0].counter_queue.count==3 && h.items[0].attack_openings[0]=='[' && h.items[0].duel.total_blocks==5 && h.items[0].courier.length==12,
        "The stack must copy the whole state; later changes to the caller's state must not change the saved copy.");
    for (int i = 1; i < HISTORY_CAPACITY; i++) {
        state.hero.x = i;
        result = history_push(&h, &state);
        REQUIRE(REPAIR_STACK_PUSH, result && h.count == i + 1 && h.items[i].hero.x == i,
            "Each push must save the state at the current top and increase the count exactly once.");
    }
    result = history_push(&h, &state);
    REQUIRE(REPAIR_STACK_PUSH, !result && h.count == HISTORY_CAPACITY,
        "A full stack must reject another push without changing its count.");
    REQUIRE(REPAIR_STACK_PUSH, !history_push(NULL, &state) && !history_push(&h, NULL),
        "Invalid arguments must return false.");
    return true;
}

static bool check_pop(void)
{
    History h; history_init(&h);
    BoardState a = {0}, b = {0}, out = {0};
    a.hero.x = 7; b.hero.x = 19; b.crate.y = 4; b.counter_queue.count=2; b.attack_openings[1]='{'; b.duel.total_blocks=6; b.courier.step=7;
    h.items[0] = a; h.items[1] = b; h.count = 2;
    bool result = history_pop(&h, &out);
    REQUIRE(REPAIR_STACK_POP, result && h.count == 1 && out.hero.x == 19 && out.crate.y == 4 && out.counter_queue.count==2 && out.attack_openings[1]=='{' && out.duel.total_blocks==6 && out.courier.step==7,
        "Expected the newest saved state (B), with all its fields copied, and count reduced to 1.");
    result = history_pop(&h, &out);
    REQUIRE(REPAIR_STACK_POP, result && h.count == 0 && out.hero.x == 7,
        "The next pop must return the older state (A) and empty the stack.");
    out.hero.x = 123;
    result = history_pop(&h, &out);
    REQUIRE(REPAIR_STACK_POP, !result && h.count == 0 && out.hero.x == 123,
        "An empty stack must return false and leave out unchanged.");
    REQUIRE(REPAIR_STACK_POP, !history_pop(NULL, &out) && !history_pop(&h, NULL),
        "Invalid arguments must return false.");
    return true;
}

static bool check_enqueue(void)
{
    CommandQueue q; queue_init(&q);
    bool result = queue_enqueue(&q, CMD_RIGHT);
    REQUIRE(REPAIR_QUEUE_ENQUEUE, result && q.count == 1 && q.front == 0 && q.rear == 0 && q.items[0] == CMD_RIGHT,
        "The first command must occupy slot 0: front = rear = 0 and count = 1.");
    queue_init(&q); q.front=COMMAND_CAPACITY-2; q.rear=COMMAND_CAPACITY-1; q.count=2;
    q.items[q.front]=CMD_UP; q.items[q.rear]=CMD_RIGHT;
    result=queue_enqueue(&q,CMD_DOWN);
    REQUIRE(REPAIR_QUEUE_ENQUEUE,result&&q.front==COMMAND_CAPACITY-2&&q.rear==0&&q.count==3&&q.items[0]==CMD_DOWN,
        "The rear must wrap from COMMAND_CAPACITY - 1 to slot 0 while the front stays fixed.");
    for(int i=3;i<COMMAND_CAPACITY;i++) {
        result=queue_enqueue(&q,CMD_LEFT);
        REQUIRE(REPAIR_QUEUE_ENQUEUE,result&&q.count==i+1,"Every array cell must be usable. Do not hard-code a four-command limit.");
    }
    CommandQueue full=q; result=queue_enqueue(&q,CMD_UP);
    REQUIRE(REPAIR_QUEUE_ENQUEUE,!result&&!memcmp(&q,&full,sizeof(q)),"A full queue rejects another command without changing any field.");
    queue_init(&q);
    REQUIRE(REPAIR_QUEUE_ENQUEUE, !queue_enqueue(NULL, CMD_UP) && !queue_enqueue(&q, (Command)99) && q.count == 0,
        "Invalid queue arguments and invalid commands must be rejected.");
    for(Command c=CMD_BLOCK_ROUND;c<=CMD_BLOCK_CURLY;c++) {
        queue_init(&q);
        REQUIRE(REPAIR_QUEUE_ENQUEUE, queue_enqueue(&q,c)&&q.items[0]==c,
            "The same queue also stores closing-bracket Commands in the duel. Accept all documented Command values.");
    }
    return true;
}

static bool check_dequeue(void)
{
    CommandQueue q; queue_init(&q);
    q.front = COMMAND_CAPACITY-2; q.rear = 1; q.count = 4;
    q.items[COMMAND_CAPACITY-2] = CMD_RIGHT; q.items[COMMAND_CAPACITY-1] = CMD_UP;
    q.items[0] = CMD_LEFT; q.items[1] = CMD_DOWN;
    const Command expected[] = {CMD_RIGHT, CMD_UP, CMD_LEFT, CMD_DOWN};
    Command out = CMD_DOWN;
    for (int i = 0; i < 4; i++) {
        bool result = queue_dequeue(&q, &out);
        REQUIRE(REPAIR_QUEUE_DEQUEUE, result && out == expected[i] && q.count == 3 - i,
            "Expected FIFO order RIGHT, UP, LEFT, DOWN across the wrapped front index.");
        REQUIRE(REPAIR_QUEUE_DEQUEUE, i == 3 ? (q.front == -1 && q.rear == -1) : q.front == (COMMAND_CAPACITY-1+i)%COMMAND_CAPACITY,
            "Advance the front circularly; after the last command, restore the empty -1/-1 indices.");
    }
    out = CMD_LEFT;
    bool result = queue_dequeue(&q, &out);
    REQUIRE(REPAIR_QUEUE_DEQUEUE, !result && q.count == 0 && out == CMD_LEFT,
        "An empty queue must return false without overwriting out.");
    REQUIRE(REPAIR_QUEUE_DEQUEUE, !queue_dequeue(NULL, &out) && !queue_dequeue(&q, NULL),
        "Invalid arguments must return false.");
    return true;
}

#define LIST_REQUIRE(task, condition, detail) do { \
    if (!verify(task, (condition), detail)) { passed = false; goto cleanup; } \
} while (0)

static bool check_insert(void)
{
    AttackNode *head = NULL;
    bool passed = true;
    bool result = attack_insert_after(&head, -1, 10, '(');
    LIST_REQUIRE(REPAIR_LIST_INSERT, result && head && head->id == 10 && !head->next,
        "Predecessor -1 must allocate the first node and update the head pointer.");
    result = attack_insert_after(&head, 10, 30, '(');
    LIST_REQUIRE(REPAIR_LIST_INSERT, result && head->next && head->next->id == 30,
        "Insert a new tail after the specified predecessor.");
    result = attack_insert_after(&head, 10, 20, '{');
    LIST_REQUIRE(REPAIR_LIST_INSERT, result && head->next && head->next->id == 20 && head->next->opening=='{' &&
        head->next->next && head->next->next->id == 30 && !head->next->next->next,
        "Inserting in the middle must preserve the old successor: 10 -> 20 -> 30.");
    result = attack_insert_after(&head, 10, 20, '(');
    LIST_REQUIRE(REPAIR_LIST_INSERT, !result,
        "Duplicate attack IDs must be rejected.");
    result = attack_insert_after(&head, 99, 40, '(');
    LIST_REQUIRE(REPAIR_LIST_INSERT, !result && !attack_contains(head, 40),
        "A missing predecessor must not create or append a node.");
    result = attack_insert_after(&head, -1, 5, '(');
    LIST_REQUIRE(REPAIR_LIST_INSERT, result && head && head->id == 5 && head->next && head->next->id == 10,
        "Predecessor -1 must also insert before an existing head.");
    LIST_REQUIRE(REPAIR_LIST_INSERT, !attack_insert_after(&head,-1,60,')') && !attack_contains(head,60),
        "Only opening brackets ( [ { are valid node payloads.");
    LIST_REQUIRE(REPAIR_LIST_INSERT, !attack_insert_after(NULL, -1, 60, '('),
        "A NULL head argument must return false.");
cleanup:
    attack_free(&head);
    return passed;
}

static bool check_remove(void)
{
    AttackNode *head = NULL;
    bool passed = true;
    for (int i = 3; i >= 1; i--) {
        AttackNode *node = malloc(sizeof(*node));
        if (!node) { issue(REPAIR_LIST_REMOVE, false, "Not enough memory to prepare the behavior check."); passed = false; goto cleanup; }
        *node = (AttackNode){.id=i, .opening='(', .next=head}; head = node;
    }
    bool result = attack_remove(&head, 2);
    LIST_REQUIRE(REPAIR_LIST_REMOVE, result && head && head->id == 1 && head->next && head->next->id == 3,
        "Removing a middle attack must reconnect its two neighbours: 1 -> 3.");
    result = attack_remove(&head, 1);
    LIST_REQUIRE(REPAIR_LIST_REMOVE, result && head && head->id == 3 && !head->next,
        "Removing the head must update the caller's head pointer.");
    result = attack_remove(&head, 3);
    LIST_REQUIRE(REPAIR_LIST_REMOVE, result && !head,
        "Removing the final attack must leave the head NULL.");
    LIST_REQUIRE(REPAIR_LIST_REMOVE, !attack_remove(&head, 99) && !attack_remove(NULL, 1),
        "A missing ID or an invalid head argument must return false.");
cleanup:
    attack_free(&head);
    return passed;
}
static bool check_power_logic(void)
{
    const int live[] = {88, 88, 57, 31, 57};
    int out = -999;
    bool available = select_finisher_power(live, 5, &out);
    char detail[240];
    snprintf(detail, sizeof(detail), "Live monitor selected %s%d from [88, 88, 57, 31, 57]. The reserve safety interlock is still open.", available ? "" : "no band; output ", out);
    REQUIRE(REPAIR_POWER_LOGIC, available && out == 57, detail);
    const int probes[][5] = {{4,9,1,6,9}, {-8,-3,-3,-6,-9},
        {0,INT_MIN,0,INT_MIN,0}, {INT_MAX,INT_MAX,2,2,1}};
    const int expected[] = {6,-6,INT_MIN,2};
    for (int i=0;i<4;i++) {
        int copy[5]; memcpy(copy,probes[i],sizeof(copy));
        out=-999; available=select_finisher_power(copy,5,&out);
        snprintf(detail,sizeof(detail),"Calibration replay [%d, %d, %d, %d, %d] still selects an unsafe reserve. Diagnose the rule, rather than one recorded example.",copy[0],copy[1],copy[2],copy[3],copy[4]);
        REQUIRE(REPAIR_POWER_LOGIC, available && out==expected[i],detail);
        REQUIRE(REPAIR_POWER_LOGIC, memcmp(copy,probes[i],sizeof(copy))==0,
            "The controller altered the probe log. Selection must leave its input readings unchanged.");
    }
    const int flat[]={7,7,7}; out=123;
    available=select_finisher_power(flat,3,&out);
    REQUIRE(REPAIR_POWER_LOGIC,!available&&out==123,
        "When every probe reports one band, the controller still invents a reserve. A missing reserve must leave the output unchanged.");
    REQUIRE(REPAIR_POWER_LOGIC,!select_finisher_power(NULL,3,&out)&&
        !select_finisher_power(flat,1,&out)&&!select_finisher_power(flat,0,&out)&&
        !select_finisher_power(flat,3,NULL),"Invalid or insufficient probe data must not produce a reserve.");
    return true;
}

static bool check_shield_logic(void)
{
    const char *patterns[]={"([", "{([", "([{("};
    for(int wave=0;wave<3;wave++) {
        AttackNode nodes[4]; const AttackNode *top=NULL;
        int n=(int)strlen(patterns[wave]);
        for(int i=0;i<n;i++) { nodes[i]=(AttackNode){.id=i,.opening=patterns[wave][i],.next=(AttackNode *)top}; top=&nodes[i]; }
        CommandQueue q; queue_init(&q); q.front=COMMAND_CAPACITY-1; q.rear=(q.front+n-1)%COMMAND_CAPACITY; q.count=n;
        for(int i=0;i<n;i++) q.items[(q.front+i)%COMMAND_CAPACITY]=counter_command(patterns[wave][n-1-i]=='('?')':patterns[wave][n-1-i]=='['?']':'}');
        CommandQueue before=q; AttackNode copy[4]; memcpy(copy,nodes,sizeof(nodes));
        REQUIRE(REPAIR_SHIELD_LOGIC,shield_accepts(top,&q),"The shield rejected a correctly reversed and type-matched response. Compare enemy TOP with response FRONT across circular wrap.");
        REQUIRE(REPAIR_SHIELD_LOGIC,!memcmp(&q,&before,sizeof(q))&&!memcmp(copy,nodes,sizeof(nodes)),"Validation must leave the linked stack and counter queue unchanged.");
        q.items[q.front]=q.items[q.front]==CMD_BLOCK_ROUND?CMD_BLOCK_SQUARE:CMD_BLOCK_ROUND;
        REQUIRE(REPAIR_SHIELD_LOGIC,!shield_accepts(top,&q),"The shield still accepts the wrong type at queue FRONT. Equal lengths alone cannot block an attack.");
    }
    CommandQueue empty; queue_init(&empty);
    AttackNode one={.id=0,.opening='(', .next=NULL};
    REQUIRE(REPAIR_SHIELD_LOGIC,shield_accepts(NULL,&empty)&&!shield_accepts(&one,&empty)&&!shield_accepts(NULL,NULL),"Both empty is safe; missing counters or an invalid queue pointer must be rejected.");
    empty.count=1; empty.front=0; empty.rear=0; empty.items[0]=CMD_UP;
    REQUIRE(REPAIR_SHIELD_LOGIC,!shield_accepts(&one,&empty),"A movement Command is not a closing-bracket counter.");
    return true;
}

#endif

bool repair_check_through(int room)
{
    repair_clear();
#if PORTAL_PUZZLE
    if (!check_push() || !check_pop()) return false;
    if (room >= 1 && (!check_enqueue() || !check_dequeue())) return false;
    if (room >= 2 && (!check_insert() || !check_remove())) return false;
#else
    (void)room;
#endif
    return true;
}

bool repair_check_diagnostics(void)
{
    if (report.active) return false;
#if PORTAL_PUZZLE
    return check_shield_logic() && check_power_logic();
#else
    return true;
#endif
}

bool repair_check_shield(void)
{
#if PORTAL_PUZZLE
    return !report.active && check_shield_logic();
#else
    return true;
#endif
}

bool repair_check_finisher(void)
{
#if PORTAL_PUZZLE
    return !report.active && check_power_logic();
#else
    return true;
#endif
}
