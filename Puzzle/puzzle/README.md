# The Last Portal — Puzzle edition

Six basic C functions are missing. After repairing them, you must diagnose two faults in controller code that already runs. Your repairs restore rewind, the courier, the attack stack, and the final portal.

Extract this entire folder. Windows 10/11 x64: open `build/puzzle/last_portal.exe`. Linux x86-64: run `./build/puzzle/last_portal` from this folder. Press Enter to see the first chapter story; Enter again begins the room and its first repair.

Each room has a gold perspective story crawl using the existing Caitlyn presentation illustrations. The mission stays visible in a fixed card. Enter or Esc skips straight to the room; Space pauses/resumes the scroll, R restarts it, and F1 opens the room guide. F2 replays the introduction during play or from a repair screen, preserving the current board. The story never starts gameplay automatically. Ordinary room restart (R during play) does not repeat the introduction.

Read [the assignment](ASSIGNMENT.md). Edit only:

- `puzzle/student/stack.c`: `history_push`, `history_pop`.
- `puzzle/student/queue.c`: `queue_enqueue`, `queue_dequeue`.
- `puzzle/student/attack_list.c`: `attack_insert_after`, `attack_remove`.
- `puzzle/student/diagnostics.c`: investigate the two controller malfunctions during the final duel.

Press **F1 — What to do** for the current goal, controls and stored data. The crate room shows the actual history count and capacity of **128 full snapshots**. Each snapshot saves the whole board before an action.

**Level two is a memory game.** Space reveals a random 10–14-move path for 30 seconds. Click **Start planning now** (or Space/Enter again) to hide it early. After it disappears, queue EVERY direction with arrows/WASD; the 64-cell buffer fits the full plan. Backspace edits, C clears, Space/Enter launches. One wrong move triggers the laser and destroys the bot. R generates a different path. No single-step probing, mid-run changes or rewind; H gives strategy, never the answer. A perfect delivery lets Caitlyn walk right to EXIT.

The final room is **Clash of Queue and Stack**. Space begins. Enemy openings are read left to right and pushed at the linked-list head (TOP). Type a whole closing response left to right into your circular queue (FRONT first): 1 = `)`, 2 = `]`, 3 = `}`. Enter launches; C clears; Backspace edits. Enemy `([` requires `])` (keys 2, 1, Enter). Matching sequences trigger automatic return shots. A mismatch changes neither structure. Three sequences/nine pairs unlock F for HOMEBOUND; then hold Right / D to walk Home. There is no reaction timer.

Foundation panels identify the missing function and its data/contract. The history stack holds complete `BoardState` copies; the queue holds direction or closing-bracket `Command` values; each attack node stores id, opening and next. Diagnose the shield before launching a response and the charge selector when attempting the finishing move.

`repair_required(...)` is an already implemented engine helper that displays a missing-function task. **Do not implement it.** Replace the starter body, including that call. `(void)x` only suppresses an unused-parameter warning. Return true on a successful operation and false for the specified rejection cases. Keep function signatures and shared types.

The later diagnosis screens show the complete operating rule and expected/observed examples; those routines already contain code. Find the faulty logic. H opens an optional extra coding clue; no required behavior is hidden in the clue.

Close the game before rebuilding. Linux, with a C99 compiler and X11/OpenGL desktop libraries: `./run-linux.sh puzzle`. Windows, from a terminal with a 64-bit MinGW-w64 GCC compiler available: `run-windows.bat puzzle`. For example, use [w64devkit](https://github.com/skeeto/w64devkit). The needed raylib headers/libraries are included; no Python or CMake is required.

If compilation fails, correct the compiler errors first. Build & Run stops without launching an old binary. After a successful build, the game checks the functions again and unlocks the appropriate room. Later rooms can remain unfinished while you play the earlier ones.

Optional text-only check: `./check-linux.sh` or `check-windows.bat`. A nonzero exit status is expected while a repair remains.

WASD/arrows move. Z undoes, R restarts, H hints. The memory run uses Space to reveal/launch, arrows to plan, Backspace to edit and C to clear. Z cannot revive a destroyed bot. Duel controls are above and on screen. Wait for a flying shot or beam to finish before undoing; R can restart immediately.

Checkpoints are saved per room in `puzzle.save`. Rebuilding rechecks earlier repairs. Only the 30-second memory preview is timed; planning and duel responses have no deadline. The stack has room for 128 undo states.

This Puzzle project contains no completed solutions. The assignment pack also includes a separate Reference folder with the finished game as executables and resources only. Play it to observe the intended behavior; make your repairs here and rebuild this Puzzle project. Read the pack START_HERE.txt and Assignment_Handout.pdf for the full workflow.

The Windows executable is cross-compiled and has not been run on Windows. Supplied checks cover representative cases; invalid student pointers or non-terminating code can still crash or hang a build. Linux and OpenGL support are required for the native Linux executable. Audio is optional.

raylib and DejaVu retain their included licenses; see `vendor/PROVENANCE.md` and `assets/DejaVu-LICENSE.txt`.
