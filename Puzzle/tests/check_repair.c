#include "repair.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int room=argc>1?atoi(argv[1]):2;
    if(room<0||room>2) return 2;
    if(repair_check_through(room)&&(room<2||repair_check_diagnostics())) {
        puts("PASS: every required repair through this room behaves correctly.");
        return 0;
    }
    const RepairReport *r=repair_report();
    printf("TASK %d/8 | %s | %s\n%s\nWHY: %s\nDATA: %s\nREQUIRED: %s\nEXAMPLE: %s\nCHECK: %s\n",
        r->task+1,r->diagnosis?r->title:r->function,r->diagnosis?"DIAGNOSIS":r->missing?"MISSING":"INCORRECT",r->file,r->purpose,r->data,r->contract,r->example,r->detail);
    if(!r->diagnosis) puts("repair_required(...) is a supplied marker. Replace the starter body including that call; do not implement the helper. (void)x only silences unused-parameter warnings. Return true on success, false on rejected operations.");
    puts("Close game, edit the source, save and rebuild: ./run-linux.sh puzzle (Linux), run-windows.bat puzzle (Windows).");
    return 10+(int)r->task;
}
