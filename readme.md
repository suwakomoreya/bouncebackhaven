C++ and raylib project with g++ on macOs.

to compile on macos, run this:
cd "$HOME/hackclub hcb/bouncebackhaven" && mkdir -p build && g++ -std=c++17 -O2 -Wall -I. -I"$HOME/libs/raylib/include" src/*.cpp -o build/bouncebackhaven "$HOME/libs/raylib/lib/libraylib.a" -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreAudio -framework QuartzCore && ./build/bouncebackhaven

to compile on windows, run this:
mkdir -p build
g++ -std=c++17 -O2 -Wall -Wextra -I. src/*.cpp -o build/bouncebackhaven.exe -lraylib -lglfw3 -lopengl32 -lgdi32 -lwinmm
OR  cmd /c build-win.bat
you need to have raylibs installed + msys g++ or clang.

## Windows .exe build

From PowerShell in the project folder, compile and launch:

```powershell
.\build-win.bat run
```

Or compile only:

```powershell
.\build-win.bat
```

The script finds your MSYS2/W64DevKit g++, builds `build\bouncebackhaven.exe`,
copies the runtime DLLs next to it, and (with `run`) starts the game.

## itch.io / browser build

From the project folder, run:

```
powershell -ExecutionPolicy Bypass -File .\build-web.ps1
```

The first run installs Emscripten to `%USERPROFILE%\emsdk` and builds raylib for
WebAssembly into `.deps\raylib` (this takes a few minutes). Later runs reuse both
and are much faster.

The build writes:

- `dist/web/` - `index.html`, `index.js`, `index.wasm`, `index.data`
- `dist/bouncebackhaven-web.zip` - upload this file to itch.io

On itch.io, create an HTML project, upload the zip, tick **"This file will be
played in the browser"**, and set the embed size to **1280x720** (or choose
Fullscreen). The canvas scales to fit the iframe while keeping its 16:9 shape.

Web notes:

- Audio starts after the first click/keypress, as required by browser autoplay
  rules.
- The menu's Exit button returns to the title screen in the browser (there is no
  window to close). On desktop it still quits normally.


