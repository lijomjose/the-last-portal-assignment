# Bring Caitlyn home

The game compiles, but six foundation functions are unfinished and two controller routines contain logical faults. Repair the foundations, use the restored systems, and then diagnose Caitlyn's shield and finishing move.

Press **F1 — What to do** at any time, including on a repair screen. The guide explains the current room's goal, controls, and connection to your C code. Opening it pauses play.

## Start

Open the puzzle executable. Press Enter for the first chapter's illustrated story crawl. The fixed mission card states the goal and controls. Press Enter (or Esc) whenever you are ready to begin the room; this can skip the animation immediately. Space pauses/resumes it, R restarts it, and F1 opens the room guide. F2 replays the current chapter later without resetting your board. A story appears before each new room and when continuing from a saved room; R during ordinary play just restarts the room.

After the introduction, the repair screen explains why play is blocked, what data the function receives, its required behavior, an example, and the source file to edit. Skipping the story never skips the source checks.

Close the game, open the named C file, and replace the placeholder body. Remove the `repair_required(...)` call as part of that replacement. Keep the same function signature and data structures. Save and use Build & Run:

- Linux: `./run-linux.sh puzzle`
- Windows, in a terminal with 64-bit MinGW-w64 GCC: `run-windows.bat puzzle`

If the compiler reports an error, fix it before trying to run again. If the game reports a failed behavior check, use the displayed example and failure detail to debug your implementation. There is no in-game button that implements the function for you.

### What are the starter lines?

| Starter line | Meaning | What you do |
| --- | --- | --- |
| `repair_required(REPAIR_STACK_PUSH);` (or another task ID) | An already implemented engine helper displays the missing-function screen and blocks play. | Remove this call when replacing that function's body. Do **not** implement `repair_required`, edit `src/repair.c`, or disable the checks. |
| `(void)history;` or `(void)state;` | Silences a compiler warning because the placeholder does not yet use that parameter. It does not operate on the structure. | Remove it when your code uses that parameter. |
| `return false;` | The placeholder always reports failure. These functions return `bool`: `true` means the operation succeeded; `false` means a valid rejection such as empty/full/invalid input. | Replace it with the appropriate success and rejection paths. Do not return `NULL` from a boolean function. |

Keep the provided headers, types, function names and parameters. The helpers such as `queue_init`, `queue_peek`, `attack_contains` and `attack_free` already work. The engine handles input, drawing, level setup and restoring the board; your functions only manipulate the supplied data.

**Saving source does not update an executable.** Close the game, save your C file, run the build-and-run command from the extracted folder, and fix any compiler errors. Opening the previous executable without rebuilding runs the old code.

## Mission 1: restore memory

Edit `puzzle/student/stack.c`:

1. `history_push`: store a complete value copy of the supplied board state at the top, increase the count, and reject invalid arguments or a full stack without changing the stack.
2. `history_pop`: copy the newest state into `out`, remove it, and reject an empty stack or invalid arguments without changing the state or `out`.

The fixed-capacity stack holds `HISTORY_CAPACITY` (128) complete snapshots. The screen shows the actual number saved and this capacity. It is not limited to four moves. `history.count` is the number stored, so the top occupied position is one less than the count. Function declarations and types are in `include/structures.h`.

**What goes into the stack:** one complete `BoardState` value per entry. The engine captures it **before an action** and passes its address as `state`. It includes `hero`, `crate`, and `bot` positions; both command queues; the full `CourierState`; `attack_ids`, `attack_openings`, `attack_count`, `delivered`, and the whole `DuelState`. Save a copy of all those fields, not a pointer, an arrow command, or just Caitlyn's position. The attack snapshot uses IDs and opening brackets instead of live node pointers so later list changes cannot damage an old saved board.

`history` points to the engine's stack. `state` points to the snapshot to save. In `history_pop`, `out` points to the caller's destination for a complete snapshot; write the returned state there. The engine restores the board after a successful pop. You do not handle a key press, capture a snapshot, or draw anything in these functions.

For example, a snapshot before a push has Caitlyn at `(8,3)` and the crate at `(8,2)`. After that push, the live positions are `(8,2)` and `(8,1)`. The saved copy must still hold the earlier positions. Popping it lets Z restore the earlier board. With snapshots A then B saved, two pops return B then A.

**Play:** press Z to undo the trapped push. Walk beside the crate and push it onto the green plate; it cannot be pulled. H shows a route, Z undoes a mistake, and R restarts. When the plate powers the door, walk into EXIT at the lower right and press Enter.

## Mission 2: restore the courier

Edit `puzzle/student/queue.c`:

3. `queue_enqueue`: add a valid command at the rear, wrapping around the array. Every cell can be used. Reject a full queue or invalid arguments without changing the queue.
4. `queue_dequeue`: remove the oldest command at the front, copy it into `out`, and advance circularly. After removing the final command, restore `front = rear = -1`. An empty queue returns false and leaves `out` unchanged.

The convention is **front = first pending command, rear = last inserted command**. Empty is `-1/-1` with `count == 0`. `COMMAND_CAPACITY` is **64**, enough for the complete 10–14-move route. Use the constant rather than hard-coding four.

**What goes into the queue:** one `Command` value (courier: `CMD_UP`, `CMD_RIGHT`, `CMD_DOWN`, `CMD_LEFT`; duel: `CMD_BLOCK_ROUND`, `CMD_BLOCK_SQUARE`, `CMD_BLOCK_CURLY`), not a bot position or keyboard character. `queue` points to the engine's pending-command queue. `command` is the new direction to record; `out` is where dequeue writes the oldest direction. The engine moves the courier after retrieving it. These functions do not move the bot themselves.

**Play — A route to remember:**

1. Space reveals a **random** safe path for **30 seconds**. Follow the arrows from S (start) to D (dock). Step numbers show the order. Click **Start planning now**, or press Space/Enter again, to hide the route early when ready. The bot waits while you plan.
2. When the path disappears, enter **all 10–14 directions** from memory. Your whole queued plan is visible in the side panel. Backspace removes the last arrow; C clears the plan.
3. Space or Enter launches only after the entire plan is entered. The bot dequeues one Command at a time and follows your exact sequence.
4. **One wrong step triggers the security laser and destroys the bot.** No single-step probing, mid-run changes, live appends or rewind. Even stepping onto a previously safe tile fails if it is the wrong next step.
5. R starts a new attempt with a different random path and fresh preview. H gives strategy, never the answer. F1/Esc pause the countdown and cover the route.
6. A perfect run delivers the cell. Caitlyn is released automatically: walk right into EXIT. There is no planning deadline after the preview.

Route generation, timing, editing controls and laser animation are supplied. Your queue functions still only store/retrieve Commands. They do not decide the route, test whether it is safe, or move/draw the bot.

## Mission 3: Clash of Queue and Stack

Edit `puzzle/student/attack_list.c`:

5. `attack_insert_after`: allocate a node storing both `id` and `opening`, and link it after the predecessor ID. -1 means insert at the head. Preserve the old successor. IDs must be unique and the opening must be `(`, `[` or `{`. Reject invalid arguments, missing predecessor or allocation failure without changing the list.
6. `attack_remove`: find an ID, reconnect neighbours, free exactly that node and return true. Handle head, middle, tail and final node. Return false unchanged for a missing ID or invalid head argument.

Each `AttackNode` stores `id`, `opening` and `next`. The tail points to `NULL`. `AttackNode **head` allows updating the caller's head pointer. IDs identify attacks, not their positions. General insertion must work after any existing predecessor, even though the duel specifically uses head insertion to implement a linked stack. Finding, copying and freeing helpers already work.

The alley has one path, with the Hollow Warden between Caitlyn and Home. Space begins the duel. Each attack is a **whole sequence** of opening brackets, read from left to right. The engine pushes each opening at the head. The last opening becomes stack TOP.

You type a **whole sequence** of closing brackets, also left to right. These enter a circular queue. Press 1 for `)`, 2 for `]`, 3 for `}`; clicking the buttons or typing closing brackets also works. C clears the response; Backspace removes the last typed symbol. **Enter submits the complete sequence.**

Example:

| Step | State |
| --- | --- |
| Enemy sends `([` left to right | Push `(`, then `[`; stack TOP is `[` → `(` |
| Player types `])` left to right | Enqueue `]`, then `)`; queue FRONT is `]` → `)` |
| First pair | Enemy TOP `[` matches queue FRONT `]` |
| Second pair | Enemy TOP `(` matches queue FRONT `)` |

A correct response triggers return shots automatically. Each consumes one head node and one front Command. A wrong response leaves both structures unchanged so you can edit and retry. There is no response deadline. Clear three sequences of lengths 2, 3 and 4, then press **F** for HOMEBOUND. The beam defeats the alien and opens the portal. Hold Right / D to walk along the only path Home.

## Two controller diagnoses

These are existing routines in `puzzle/student/diagnostics.c`, with no missing-function markers. The screen states the required rule and source-file lead; you identify the faulty code and choose the repair. H supplies an optional reasoning clue.

**The shield accepts crossed attacks.** This appears when first launching a response. Compare enemy **stack TOP** against player **queue FRONT**. Match types and order; require equal lengths. Enemy `([` needs `])`, not `)]`. The linked stack already exists: traverse its nodes and read the counter queue circularly from `front`. Return a bool and leave both inputs unchanged. The engine removes/dequeues only after acceptance. Both empty is safe. A NULL queue, invalid queue state, movement Commands in the counter queue or invalid opening payloads are unsafe.

**The finishing move keeps shorting.** This appears after all three sequences, when F attempts HOMEBOUND. The highest band is jammed; select the highest **distinct** band below it. `[88,88,57,31,57]` requires 57, not 88. Handle repeated and negative values and integer extremes. Do not alter the input. Invalid pointers, fewer than two readings or fewer than two distinct values return false with `out` unchanged. A hard-coded answer will not repair other readings.

Close, save, rebuild and relaunch after every source edit. Checkpoints save the room, not individual moves; when rebuilding in room three you replay the duel. Earlier operations are rechecked.

## Controls and checking

F1 explains the current mission and pauses play. F2 replays its story without resetting the board. WASD/arrows move, Z undoes, R restarts, H gives a hint, Esc pauses. In the memory run Space reveals/launches, Backspace edits, C clears; R retries with a fresh random route. Z cannot undo a run or revive a destroyed bot. In the duel Space begins, 1/2/3 types, Enter launches, C clears, Backspace edits and F finishes. Z waits until a shot/beam animation ends; R can restart immediately.

The optional text-only check is `./check-linux.sh` or `check-windows.bat`. It reports the first remaining repair, including the controller diagnoses. Passing supplied checks does not replace finishing the game or checking your own edge cases. Do not edit the engine checks or shared headers to bypass a task.
