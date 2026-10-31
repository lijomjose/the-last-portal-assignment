THE LAST PORTAL - COMPLETED REFERENCE GAME v1.5.1

This is the finished, playable game. All eight repairs are already working.
It contains executables, resources, licenses and launch instructions only;
there are no completed C source files, headers or build libraries here.

Windows 10/11 x64: double-click PLAY_WINDOWS.bat in this folder.
PowerShell in this folder: .\PLAY_WINDOWS.bat
Linux x86-64: open a terminal in this folder and run: sh play-linux.sh
macOS 11+ (Intel or Apple Silicon): run sh play-macos.sh or double-click
PLAY_MACOS.command. The original assignment ZIP requires the separate Mac
Reference download from the v1.5.1 release. See README_MACOS.md.
No compiler or rebuild is needed for this Reference edition.
Keep bin and assets together. Do not run from inside the ZIP.
A graphical desktop, OpenGL driver and normal system runtime are required.

Press Enter to begin; Enter/Esc skips a story. F1 explains the current room.
F2 replays its story; H gives a hint; R restarts the room; M toggles sound.
Play all three missions to observe what your repaired Puzzle should do:
undo the crate mistake, remember a route, and counter the alien to reach Home.
Only the memory preview is timed; use Start planning now to hide it early.
Random routes can differ between playthroughs and between editions.

The game saves room checkpoints in completed.save in this folder. To start
fresh, close this game and delete completed.save, if present.

Return to the pack's Puzzle folder to do the assignment. Edit its four
student C files and rebuild that edition. Do not copy this binary into
Puzzle or use a Reference screenshot as proof that your repairs work.
The assignment handout and START_HERE.txt are in the outer pack folder.

Validation: the Linux game was tested locally. The Windows executable is
cross-compiled and has not been run on Windows in this environment.
The universal Mac build and logic checks passed on native Intel and Apple
Silicon runners. Graphics could not be checked on those hosted machines:
their OpenGL pixel format was unavailable. See README_MACOS.md for details.
Third-party license texts are in LICENSES and assets/DejaVu-LICENSE.txt.
