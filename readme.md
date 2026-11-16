C++ and raylib project with g++ on macOs.

to compile on macos, run this:
cd "$HOME/hackclub hcb/bouncebackhaven" && mkdir -p build && g++ -std=c++17 -O2 -Wall -I. -I"$HOME/libs/raylib/include" src/*.cpp -o build/bouncebackhaven "$HOME/libs/raylib/lib/libraylib.a" -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreAudio -framework QuartzCore && ./build/bouncebackhaven

to compile on windows, run this:
mkdir -p build
g++ -std=c++17 -O2 -Wall -Wextra -I. src/*.cpp -o build/bouncebackhaven.exe -lraylib -lglfw3 -lopengl32 -lgdi32 -lwinmm

you need to have raylibs installed + msys g++ or clang.

