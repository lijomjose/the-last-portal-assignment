#ifndef PORTAL_DIAGNOSTICS_H
#define PORTAL_DIAGNOSTICS_H
#include <stdbool.h>
#include "structures.h"

/* Existing portal-controller interfaces. Diagnose behavior using the monitor
 * and maintenance notes; keep these signatures when repairing the code. */
bool select_finisher_power(const int *readings, int count, int *out);
bool shield_accepts(const AttackNode *enemy_top, const CommandQueue *counter_queue);

#endif
