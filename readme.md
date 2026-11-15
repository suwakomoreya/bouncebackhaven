# bouncebackhaven

C++ + [raylib](https://www.raylib.com) game project, built with `g++` on macOS.

## Build

```sh
cd "~/hackclub hcb/bouncebackhaven"
make run
```

`make` produces `build/bouncebackhaven`; `make clean` removes it.
raylib lives in `~/libs/raylib` (`include/`, `lib/libraylib.a`, version 6.2).

> Note: `g++` on macOS is Apple clang. MSYS2 (`msys g++`) is Windows-only and is
> not used or needed here.

## Layout

| Path | What |
|---|---|
| `src/main.cpp` | game entry point (raylib starter window) |
| `Makefile` | builds `build/bouncebackhaven`, statically links raylib |
| `.vscode/tasks.json` | `make` (default build), `make run`, `make clean` |
| `.vscode/launch.json` | debug with lldb (F5) |
| `.vscode/c_cpp_properties.json` | IntelliSense include paths for raylib |

## From VS Code

```sh
code .
```

- **Cmd+Shift+B** build
- **F5** build + debug (lldb)
- **Ctrl+Shift+B** → `make run`
