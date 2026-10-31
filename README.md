# The Last Portal

A Data Structures assignment in C: repair a small game and help Caitlyn find her way home.

**[Download the complete assignment pack for Windows and Linux](https://github.com/lijomjose/the-last-portal-assignment/releases/download/v1.5.1/The_Last_Portal_Assignment_Pack_v1.5.1.zip)** (about 19 MB).

Extract the entire ZIP, then open **START_HERE.txt**. You do not need a GitHub account or Git installed to download it.

**Mac users:** see [the macOS instructions](MACOS_README.md) and download the
**Mac add-on** from the [release page](https://github.com/lijomjose/the-last-portal-assignment/releases/tag/v1.5.1).
It adds native Intel/Apple Silicon builds and a bundled Mac library, without
replacing your four student C files. X11 and Homebrew are not required.
To play the finished game, download the separate **[Mac Reference ZIP](https://github.com/lijomjose/the-last-portal-assignment/releases/download/v1.5.1/The_Last_Portal_Reference_macOS_v1.5.1.zip)**.
Extract it and run `sh play-macos.sh` in its folder. No compiler is needed for the Reference.
If you already downloaded the earlier Puzzle Mac add-on, re-download and apply the corrected version.

## What you will do

The supplied game has six missing data-structure functions and two faulty controller routines. Your repairs restore undo, guide a courier through a remembered route, and help Caitlyn counter an alien blocking the portal home. The engine, graphics, stories and resources are supplied. Your job is to understand the failure, implement the repair, rebuild, and use the restored mechanic.

The aim is to gain experience diagnosing a running application and the confidence that comes from making your own code work. Expect to spend time reading, tracing and trying again. Get help understanding concepts and tools, but do the diagnosis, implementation and testing yourself. Do not ask an LLM to generate the repair implementations or solve the controllers for you.

## What is included

| Item | Purpose |
| --- | --- |
| `Puzzle/` | Editable starter source, build/check/debug scripts, libraries, resources and unfinished executables. This is your assignment. |
| `Reference/` | Completed Windows x64, Linux x86-64 and universal Mac executables, resources and launchers. The original full ZIP has Windows/Linux only; the separate Mac Reference ZIP supplies Mac play. Completed source code is not included. |
| [Assignment_Handout.pdf](Assignment_Handout.pdf) | Full 18-page assignment: game design, all eight repair contracts, build/debug instructions and submission requirements. |
| [Assignment_Handout.docx](Assignment_Handout.docx) | The same handout in editable Word format. |
| [START_HERE.txt](START_HERE.txt) | Package orientation and platform instructions. |

This public repository contains only the student material. There is no instructor solution source or instructor repository history here.

## Start with the completed reference

After extracting the full pack:

- **Windows:** open `Reference` and double-click `PLAY_WINDOWS.bat`.
- **Linux:** open a terminal in `Reference` and run `sh play-linux.sh`.
- **macOS 11+:** use the Mac Reference ZIP above, or run `sh play-macos.sh` in this repository's `Reference` folder. Both Intel and Apple Silicon are supported.

No compiler is needed to play the reference. Use the matching 64-bit operating system with a graphical desktop and working OpenGL support. Play the three rooms, then work on your own Puzzle copy. A reference playthrough or screenshot does not complete the assignment.

## Save, build and debug your Puzzle

Edit only the four C files under `Puzzle/puzzle/student/`. Save the code and close the running Puzzle game before rebuilding.

From the **outer extracted pack folder**:

- **Windows:** double-click `DEBUG_PUZZLE_WINDOWS.bat`.
- **Linux:** open a terminal there and run `sh debug-puzzle-linux.sh`.
- **macOS:** after adding the Mac files and installing Apple's Command Line Tools,
  run `sh debug-puzzle-macos.sh`. See [the Mac guide](MACOS_README.md).

These launchers build your current Puzzle source with debug information, then run it. Compiler and runtime logs are saved under `Puzzle/build/puzzle-debug/`. A failed build never launches an old executable.

A compiler is still required for your repairs: 64-bit MinGW-w64 GCC on Windows, or a C99 compiler and the required X11/OpenGL libraries on Linux. The scripts do not install compilers. See [the debugging guide](Puzzle/DEBUGGING.md) for build-only, optional GDB and Linux sanitizer modes. The normal build scripts are also supplied inside `Puzzle`.

Keep the Reference and Puzzle folders separate. The reference uses `Reference/completed.save`; Puzzle uses `Puzzle/puzzle.save`. These room checkpoints do not contain source repairs.

## Download options

The recommended download is the complete ZIP linked above, also listed on the [release page](https://github.com/lijomjose/the-last-portal-assignment/releases/tag/v1.5.1). If you use GitHub's **Code > Download ZIP**, extract it and start from the folder containing `START_HERE.txt`; the same student files and folders are present.

## Submission and course information

Follow the handout for the required source files, brief explanation and final screenshot of your repaired Puzzle. Use your course portal for the submission link, naming rules and deadline. No formal report or course certificate is required.

## Validation and credits

The packaged Linux reference passed a full playthrough. The Linux debug launcher was checked for compilation, source line information, paths containing spaces, failure handling and runtime logging. Windows executables are cross-compiled; the Windows builds and launchers have not been run on Windows in the authoring environment.

The Mac add-on bundles the official universal Mac library and uses Apple's
compiler and native frameworks. The normal Puzzle build, debug build and missing-repair
checker passed on both Apple Silicon and Intel macOS runners. The completed Reference passed
55,078 game-logic checks and 31,162 diagnostic checks natively and builds as a
locally signed universal executable. Graphics startup on both hosted CI runners
was blocked by their unavailable OpenGL pixel format; this is not a completed
interactive playthrough on student hardware. See [the Mac guide](MACOS_README.md).

The game reuses the instructor's generated Caitlyn course illustrations. raylib and DejaVu license notices are retained in the relevant folders. `SHA256SUMS.txt` lists repository files before editing or playing; each release ZIP has its own checksum. The Mac add-on also includes a separate file manifest.
