#include <bits/stdc++.h>
#include "raylib.h"
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
#include "game.h"
#include "ui.h"
#include "tennis.h"

namespace {

Texture2D bg{};
Texture2D title{};
Texture2D sportTitle{};

Texture2D sportPngs[3]{};
Texture2D sportHover[3]{};
Texture2D menuPngs[2]{};
Texture2D menuHover[2]{};

bool assetsLoaded = false;
bool tennisStarted = false;

void LoadAssets() {
    bg = LoadTexture("assets/backgrounds/menu_background.png");
    title = LoadTexture("assets/menu/title.png");
    sportTitle = LoadTexture("assets/sports/sport_choice_title.png");

    sportPngs[0] = LoadTexture("assets/sports/tennis_button.png");
    sportPngs[1] = LoadTexture("assets/sports/golf_button.png");
    sportPngs[2] = LoadTexture("assets/sports/back_button.png");

    sportHover[0] = LoadTexture("assets/sports/tennis_button_hover.png");
    sportHover[1] = LoadTexture("assets/sports/golf_button_hover.png");
    sportHover[2] = LoadTexture("assets/sports/back_button_hover.png");

    menuPngs[0] = LoadTexture("assets/menu/exit_button.png");
    menuPngs[1] = LoadTexture("assets/menu/play_button.png");

    menuHover[0] = LoadTexture("assets/menu/exit_button_hover.png");
    menuHover[1] = LoadTexture("assets/menu/play_button_hover.png");

    assetsLoaded = true;
}

[[maybe_unused]] void UnloadAssets() {
    if (!assetsLoaded) return;

    UnloadTexture(bg);
    UnloadTexture(title);
    UnloadTexture(sportTitle);

    for (int i = 0; i < 2; i++) {
        UnloadTexture(menuPngs[i]);
        UnloadTexture(menuHover[i]);
    }

    for (int i = 0; i < 3; i++) {
        UnloadTexture(sportPngs[i]);
        UnloadTexture(sportHover[i]);
    }

    assetsLoaded = false;
}

// One frame of the game. Kept separate from main() so the browser build can
// drive it from Emscripten's requestAnimationFrame loop.
void UpdateDrawFrame() {
    if (WindowShouldClose() || current == screen::exit) {
#if defined(__EMSCRIPTEN__)
        emscripten_cancel_main_loop();
#endif
        return;
    }

    const Vector2 cursor = GetMousePosition();

    const bool play = CheckCollisionPointRec(cursor, {463, 332, 360, 68});
    const bool exitButton = CheckCollisionPointRec(cursor, {460, 444, 360, 68});

    const bool back = CheckCollisionPointRec(cursor, {463, 476, 360, 68});
    const bool game1 = CheckCollisionPointRec(cursor, {463, 300, 360, 68});
    const bool game2 = CheckCollisionPointRec(cursor, {463, 388, 360, 68});

    // drawing
    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexture(bg, 0, 0, WHITE);

    if (current == screen::menu) {
        DrawTexture(title, 0, 0, WHITE);

        if (play) {
            DrawTexture(menuHover[1], 0, 0, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                current = screen::modes;
            }
        } else {
            DrawTexture(menuPngs[1], 0, 0, WHITE);
        }

        if (exitButton) {
            DrawTexture(menuHover[0], 0, 0, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
#if defined(__EMSCRIPTEN__)
                // There is no window to close in a browser, so treat the quit
                // button as "back to the title screen" instead of killing the
                // page (which would leave Emscripten's exit screen behind).
                current = screen::menu;
#else
                current = screen::exit;
#endif
            }
        } else {
            DrawTexture(menuPngs[0], 0, 0, WHITE);
        }
    }

    else if (current == screen::modes) {
        DrawTexture(sportTitle, 0, 0, WHITE);
        if (game1) {
            DrawTexture(sportHover[0], 0, 0, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                tennisStarted = false;
                current = screen::game1;
            }
        } else {
            DrawTexture(sportPngs[0], 0, 0, WHITE);
        }

        // golf
        if (game2) {
            DrawTexture(sportHover[1], 0, 0, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                current = screen::game2;
            }
        } else {
            DrawTexture(sportPngs[1], 0, 0, WHITE);
        }

        // back
        if (back) {
            DrawTexture(sportHover[2], 0, 0, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                current = screen::menu;
            }
        } else {
            DrawTexture(sportPngs[2], 0, 0, WHITE);
        }
    }

    else if (current == screen::game1) {
        if (!tennisStarted) {
            tennis::Reset();
            tennisStarted = true;
        }
        tennis::Update();
        tennis::Draw();
        if (tennis::WantsExit()) {
            current = screen::modes;
        }
    }

    else if (current == screen::game2) {
        DrawText("GOLF GAME", 500, 300, 40, WHITE);

        if (IsKeyPressed(KEY_ESCAPE)) {
            current = screen::modes;
        }
    }

    EndDrawing();
}

}  // namespace

int main() {
    InitWindow(screenw, screenh, "bounce back haven");
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    current = screen::menu;
    LoadAssets();

#if defined(__EMSCRIPTEN__)
    // 0 == use the browser's requestAnimationFrame rate. The last argument
    // keeps main() from returning while the browser loop is running.
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose() && current != screen::exit) {
        UpdateDrawFrame();
    }

    UnloadAssets();
    tennis::Shutdown();
    CloseAudioDevice();
    CloseWindow();
#endif

    return 0;
}
