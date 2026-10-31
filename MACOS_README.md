# The Last Portal on macOS

The original v1.5.1 download was built for Windows and Linux. Its Linux scripts
link X11 and Linux libraries, and its prebuilt Linux executable is not a Mac
application. Installing XQuartz does not make that executable or library native.

The Mac add-on builds your **own Puzzle source** with Apple Clang, the supplied
universal macOS raylib library, and Apple's native frameworks. It supports both
Apple Silicon and Intel builds without Homebrew or X11.

## Already started the assignment

Keep your existing extracted assignment folder and your edited files. Download
the Mac add-on from the [release page](https://github.com/lijomjose/the-last-portal-assignment/releases/tag/v1.5.1).
Extract the add-on, then open Terminal in its extracted folder. Type
`sh install-macos-addon.sh ` followed by a space, drag your **existing outer
assignment folder** from Finder into Terminal, and press Return. For example:

```sh
sh install-macos-addon.sh "/Users/your-name/Documents/Your Assignment Folder"
```

Use your actual existing folder in place of that example. It must already contain
`Puzzle/puzzle/student`. The installer copies only a fixed list of platform files;
it never replaces the whole Puzzle folder or any of your four student C files.
The add-on contains no replacement implementations.

Avoid dragging the whole new Puzzle folder onto your old one and choosing
Finder's Replace option, which could remove your other project files. Use the
supplied installer to merge the required files individually.

After merging, these files should exist in your original assignment folder:

```text
DEBUG_PUZZLE_MACOS.command
debug-puzzle-macos.sh
MACOS_README.md
Puzzle/build-macos.sh
Puzzle/run-macos.sh
Puzzle/debug-macos.sh
Puzzle/check-macos.sh
Puzzle/macos-common.sh
Puzzle/vendor/raylib/macos/include/raylib.h
Puzzle/vendor/raylib/macos/lib/libraylib.a
```

If you have not started, download the repository using **Code > Download ZIP**
for a fresh copy containing the Mac files, then extract it. Do not overwrite an
existing copy containing your repairs with a fresh starter project.

## One-time setup

Open Terminal and run:

```sh
xcode-select --install
```

Finish Apple's installation dialog before continuing. If it says the tools are
already installed, proceed. The full Xcode application is not required. Check:

```sh
xcrun --find clang
```

This should print a compiler path. That is the only additional tool installation
needed with this add-on; the compatible raylib library is included.

## Build and run after each source edit

Save your code and close the running Puzzle game. In Terminal, type `cd` followed
by a space, drag the **outer assignment folder** from Finder into Terminal, and
press Return. Then run:

```sh
sh debug-puzzle-macos.sh
```

The script switches into `Puzzle`, builds your current source with debug
information, and opens the game only if compilation succeeds. Missing-function
panels are expected until your repairs work. You still need to edit, save and
rebuild after each repair; starting an old executable does not apply source edits.

The debug executable is `Puzzle/build/macos-puzzle-debug/last_portal`. Compiler
and runtime messages are saved beside it as `build.log` and `run.log`. The runtime
log is printed after you close the game or it exits with an error. It uses your
existing `Puzzle/puzzle.save` room checkpoint.

After setup, you can also double-click `DEBUG_PUZZLE_MACOS.command`. If Finder
reports a permission problem, use the Terminal command above; it does not depend
on executable permissions surviving ZIP extraction.

## Other commands

Run these from inside `Puzzle`:

| Task | Command |
| --- | --- |
| Normal build and play | `sh run-macos.sh` |
| Normal build only | `sh build-macos.sh` |
| Debug build only | `sh debug-macos.sh --build-only` |
| Run under Apple's LLDB debugger | `sh debug-macos.sh --lldb` |
| Run with memory/undefined-behavior checks | `sh debug-macos.sh --sanitize` |
| Text-only check of all required repairs | `sh check-macos.sh` |
| Text-only check through the first room | `sh check-macos.sh 0` |

In LLDB, type `bt` after a crash for a call stack and `quit` to leave. Sanitizer
reports go to `build/macos-puzzle-debug/run.log`. A failed text-only check is
normal while a required implementation is still missing or incorrect.

The supplied Linux launchers now route to the Mac scripts when executed on
macOS. Prefer the explicit Mac commands above when following the older handout.

## Common problems

| Message or symptom | What to do |
| --- | --- |
| `library not found for -lX11`, `-lrt`, or a Linux archive format error | You are using the old Linux build path. Merge the full add-on, then use `sh debug-puzzle-macos.sh`. Do not add X11 as a workaround. |
| `Bad CPU type in executable` or `cannot execute binary file` | Do not open the bundled Linux binary or Windows `.exe`. Build a Mac executable with the new launcher. |
| `invalid active developer path` / compiler or SDK missing | Complete `xcode-select --install`. After a macOS upgrade, check Apple's developer-tools updates. |
| The bundled Mac library is missing | Check the path `Puzzle/vendor/raylib/macos/lib/libraylib.a`. You may have copied the add-on to the wrong level or replaced instead of merging folders. |
| `No such file or directory` for the launcher | Use the outer assignment folder in Terminal, where `Puzzle` and `debug-puzzle-macos.sh` are together. |
| The game opens with blank art or missing fonts | Keep `Puzzle/assets` with the project and launch through the supplied scripts, which set the correct folder. |
| A repair reappears | Save the correct student file, close the game, and rebuild. Read the first compiler error in the log if building failed. |

If it still fails, send your instructor the exact error, your macOS version,
`uname -m` output, and `Puzzle/build/macos-puzzle-debug/build.log` if it exists.
Do not delete your edited student files or reinstall unrelated X11 packages.

## Completed reference and submission

There are two separate Mac downloads:

- **[Completed Mac Reference](https://github.com/lijomjose/the-last-portal-assignment/releases/download/v1.5.1/The_Last_Portal_Reference_macOS_v1.5.1.zip):** extract the whole ZIP, open Terminal in its extracted folder, and run `sh play-macos.sh`. No compiler or build is needed. This contains a universal executable and resources, with no completed source. See [the Reference guide](Reference/README_MACOS.md).
- **[Puzzle Mac add-on](https://github.com/lijomjose/the-last-portal-assignment/releases/download/v1.5.1/The_Last_Portal_macOS_Addon_v1.5.1.zip):** use the installer described above to build your own repaired code. This download does not contain the completed Reference.

If you downloaded the earlier Mac add-on, download it again and reapply it with
`install-macos-addon.sh`. The corrected scripts select the Mac SDK explicitly,
check the library architectures correctly, and link the required Apple frameworks.
Your four edited student C files are preserved.

The old Reference Linux executable cannot run on a Mac by changing `sh` to `bash`.
Use the native Mac Reference download instead. Both Mac downloads support Intel
and Apple Silicon; the Reference requires macOS 11 or newer.

Your submission rules, deadline and marks are the ones in the instructor's
email. This platform fix does not change the assignment or supply its solutions.

## Dependency source and checks

The universal x86_64/arm64 static library and header are from the official
[raylib 6.2 release](https://github.com/raysan5/raylib/releases/tag/6.2), with its
license retained under `Puzzle/vendor/raylib/macos/LICENSE`. The original archive
SHA-256 is recorded in `Puzzle/vendor/PROVENANCE.md`.

[Apple's Command Line Tools instructions](https://developer.apple.com/documentation/xcode/installing-the-command-line-tools)
and [raylib's macOS guide](https://github.com/raysan5/raylib/wiki/Working-on-macOS)
describe the native toolchain and frameworks. The bundled archive has both Intel
and Apple Silicon slices. Native build and runtime verification details are
recorded in the repository README. Intel support is built into the universal
Reference executable; Intel runtime testing is separate from Apple Silicon testing.
