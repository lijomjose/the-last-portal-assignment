# Play the completed Reference game on a Mac

This is the finished game for observing how all three missions should work. All
eight repairs are already implemented in the executable. No completed C source,
headers, object files or build libraries are included in the Mac Reference ZIP.

## Download and play

Download **The_Last_Portal_Reference_macOS_v1.5.1.zip** from the
[assignment release](https://github.com/lijomjose/the-last-portal-assignment/releases/tag/v1.5.1).
Double-click the ZIP to extract the entire folder. Keep `bin`, `assets` and the
launchers together. This is a separate playable reference; it does not replace
your Puzzle project or your edited C files.

Open Terminal, type `cd`, press Space, drag the extracted Mac Reference folder
from Finder into Terminal, and press Return. Then run:

```sh
sh play-macos.sh
```

You can also double-click `PLAY_MACOS.command`. If extraction removed its
executable permission, the Terminal command above still works.

The executable supports **Apple Silicon and Intel Macs with macOS 11 or newer**.
It includes raylib statically; no X11, Homebrew, compiler or build step is needed
to play this completed reference. An ordinary Mac desktop session is required.

The original `play-linux.sh` launches a Linux ELF executable. Running it with
`bash` does not turn that binary into a Mac application. Use the new Mac package
and launcher. In updated copies of the public repository, the Linux launcher
routes Mac users to the Mac executable automatically.

## Playing and checkpoints

Enter begins the game; Enter/Esc skips the story; F1 explains the current room.
F2 replays the story, H gives a hint, R restarts a room, and M toggles sound.
The memory route has a 30-second preview and can be hidden early with Start
planning now. Planning and the duel have no response deadline.

Room progress is saved as `completed.save` beside the launcher. To start this
reference fresh, close it and remove that file if present. Your Puzzle's own
checkpoint and edited source are separate.

After observing the mechanics, return to your assignment folder and repair and
rebuild **your Puzzle**. A Reference screenshot does not demonstrate your repairs.

## If it does not open

- Extract the whole ZIP first; do not run inside the archive.
- Keep the `assets` folder and `bin/last_portal-macos` with the launcher.
- Use the Mac launcher rather than the original Linux binary or Windows `.exe`.
- If the Terminal reports an error, send that message and `macos-reference.log`
  from this folder to your instructor.
- If macOS explicitly blocks this course download because the developer cannot
  be verified, follow [Apple's instructions for opening a trusted downloaded app](https://support.apple.com/en-us/102445).
  The reference has a local signature, not an Apple Developer ID notarization.
  Keep the normal system-wide security protections enabled.

The raylib and font license notices are included under `LICENSES` and
`assets/DejaVu-LICENSE.txt`.

## Validation scope

This universal executable was compiled and its command-line startup checked on
native Intel and Apple Silicon Mac runners. The completed game's logic passed
55,078 game checks and 31,162 diagnostic checks on each architecture. Both hosted
runners lack a suitable OpenGL graphics context, so graphical startup and a full
interactive playthrough on a physical Mac remain unverified. Send any desktop
startup error and `macos-reference.log` to your instructor.
