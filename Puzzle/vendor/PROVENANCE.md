# Bundled dependencies

## macOS compatibility add-on

The Mac add-on includes only the header, universal static library (x86_64 and
arm64), and license from this official upstream archive:

https://github.com/raysan5/raylib/releases/download/6.2/raylib-6.2_macos.tar.gz

Locally computed archive SHA-256:
`100437734a44d56ed9d2a1c4b249dbfc6d6cc3477a45e306f9f56fe5f1e2b4ef`.

This records the archive retrieved for the add-on, rather than claiming an
independently supplied upstream checksum. Mac builds use native Apple frameworks;
the bundled Linux/Windows libraries are not reused on macOS.

raylib 6.2 is pinned to the official release:
https://github.com/raysan5/raylib/releases/tag/6.2

Only the public header, static library, and license are included in the distributable package for each platform.

| Original download | Locally computed SHA-256 |
| --- | --- |
| `raylib-6.2_linux_amd64.tar.gz` | `586bbc32f3596d439382b1c5dbc6c1c573eb7e1d00fe92be7d4deb116883720d` |
| `raylib-6.2_win64_mingw-w64.zip` | `48ba378d100fe7e13fc2c35d708764b855b815596f3b0a31746e37259283961f` |

Download URLs:

- https://github.com/raysan5/raylib/releases/download/6.2/raylib-6.2_linux_amd64.tar.gz
- https://github.com/raysan5/raylib/releases/download/6.2/raylib-6.2_win64_mingw-w64.zip

These checksums record the files retrieved for this build; they are not a claim of independent upstream checksum verification.

The Linux executable uses the host C compiler. The Windows executable is built from the same C sources against the bundled MinGW-w64 raylib library using a temporary Zig C cross-compiler. The cross-compiler is not included and is not needed to play the executable. Windows runtime behavior still requires validation on Windows.
