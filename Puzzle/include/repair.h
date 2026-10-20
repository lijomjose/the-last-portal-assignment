#ifndef PORTAL_REPAIR_H
#define PORTAL_REPAIR_H

#include <stdbool.h>

#ifndef PORTAL_PUZZLE
#define PORTAL_PUZZLE 0
#endif

#if PORTAL_PUZZLE
#define PORTAL_BUILD_LABEL "SOURCE REPAIR PUZZLE"
#define PORTAL_TITLE_NOTE "STUDENT EDITION  /  Six repairs. Two controller mysteries."
#define PORTAL_WINDOW_TITLE "The Last Portal | Source repair puzzle"
#define PORTAL_SAVE_FILE "puzzle.save"
#else
#define PORTAL_BUILD_LABEL "COMPLETE REFERENCE BUILD"
#define PORTAL_TITLE_NOTE "INSTRUCTOR REFERENCE  /  All C implementations included"
#define PORTAL_WINDOW_TITLE "The Last Portal | Complete reference game"
#define PORTAL_SAVE_FILE "completed.save"
#endif

typedef enum {
    REPAIR_STACK_PUSH, REPAIR_STACK_POP,
    REPAIR_QUEUE_ENQUEUE, REPAIR_QUEUE_DEQUEUE,
    REPAIR_LIST_INSERT, REPAIR_LIST_REMOVE,
    REPAIR_SHIELD_LOGIC, REPAIR_POWER_LOGIC,
    REPAIR_TASK_COUNT
} RepairTask;

typedef struct {
    bool active, missing, diagnosis, show_clue;
    RepairTask task;
    const char *module, *file, *function, *contract, *example;
    const char *purpose, *data;
    char detail[240];
    const char *title, *clue;
} RepairReport;

/* A stub reports a missing implementation explicitly. Ordinary false/NULL
 * results are still available for empty, full, or not-found conditions. */
void repair_required(RepairTask task);
const RepairReport *repair_report(void);
bool repair_is_blocked(void);
void repair_clear(void);
bool repair_check_through(int room);
bool repair_check_diagnostics(void);
bool repair_check_shield(void);
bool repair_check_finisher(void);
void repair_toggle_clue(void);

#endif
