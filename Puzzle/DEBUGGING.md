# Build and run your Puzzle for debugging

Save your edits and close the running Puzzle game first. A compiler must
already be installed: 64-bit MinGW-w64 GCC on Windows, or a C99 compiler and
the required X11/OpenGL libraries on Linux. These scripts do not install tools.

In the assignment pack, Windows students can double-click
`DEBUG_PUZZLE_WINDOWS.bat` in the outer pack folder. Linux students can open
a terminal there and run `sh debug-puzzle-linux.sh`. Each launcher switches
to the correct Puzzle folder, compiles your current source and then runs it.
Both work when the extracted folder name contains spaces.

You can also run the scripts directly inside `Puzzle`:

| Purpose | Windows PowerShell | Linux terminal |
| --- | --- | --- |
| Build and play | `.\debug-windows.bat` | `sh debug-linux.sh` |
| Build without opening a game | `.\debug-windows.bat build-only` | `sh debug-linux.sh --build-only` |
| Run under GDB | `.\debug-windows.bat gdb` | `sh debug-linux.sh --gdb` |
| Check invalid memory access and undefined behavior | Use GDB or the lab debugger | `sh debug-linux.sh --sanitize` |

The outer pack launchers accept the same optional mode arguments.
The Windows window waits for a key before closing, so messages remain visible.

The debug build uses `-g3 -O0 -fno-omit-frame-pointer`, alongside the normal
warnings. Compiler messages go to `Puzzle/build/puzzle-debug/build.log` and
are printed if compilation fails. A failed build never launches an old game.
The default run saves standard output and error output to
`Puzzle/build/puzzle-debug/run.log`; the script prints it after the game closes
or crashes. Each new build/run replaces the corresponding log.

The separate debug executable is `Puzzle/build/puzzle-debug/last_portal.exe`
on Windows or `Puzzle/build/puzzle-debug/last_portal` on Linux. It runs your
Puzzle source and uses its existing `puzzle.save` room checkpoint. It does not
change the completed Reference game or grant progress through missing code.
An unfinished function still shows its normal repair screen; that is expected.

GDB is optional and must be installed to use `gdb`/`--gdb`. It opens an
interactive prompt when the game stops or exits. After a crash, `bt` shows the
call stack; `frame 0`, `list` and `print variable_name` help inspect the fault.
Use `quit` to leave. A GDB session log is saved as `build/puzzle-debug/gdb.log`.
The normal `run.log` applies to default/sanitizer runs, not the GDB session.

Linux `--sanitize` additionally enables AddressSanitizer and
UndefinedBehaviorSanitizer; compiler/runtime support is required. Reports can
identify the source line of an invalid access or other detected fault. If your
compiler does not support these flags, use the default or GDB mode instead.
Neither a successful build nor a quiet sanitizer run proves that your data
structure satisfies all contracts: trace and test the specified boundary cases.

For every repair: save, close the game, run the launcher again, inspect the first
error if there is one, and try the restored mechanic. Do not copy the completed
Reference executable into Puzzle as a replacement for fixing the source.
