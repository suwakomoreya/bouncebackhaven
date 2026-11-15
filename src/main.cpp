
#include <bits/stdc++.h>
#include "raylib.h"
#include "game.h"
#include "ui.h"

int main() {
    InitWindow(screenw, screenh, "bounce back haven");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    Vector2 Cursor;

    // Textures (must load after InitWindow)
    Texture2D bg = LoadTexture("assets/backgrounds/menu_background.png");
    Texture2D title = LoadTexture("assets/menu/title.png");
    Texture2D sport_title = LoadTexture("assets/sports/sport_choice_title.png");

    // Sports buttons: 0 = Tennis, 1 = Back
    Texture2D sportpngs[] = {
        LoadTexture("assets/sports/tennis_button.png"),
        LoadTexture("assets/sports/back_button.png")
    };

    Texture2D sporthover[] = {
        LoadTexture("assets/sports/tennis_button_hover.png"),
        LoadTexture("assets/sports/back_button_hover.png")
    };

    // Menu buttons: 0 = Exit, 1 = Play
    Texture2D menupngs[] = {
        LoadTexture("assets/menu/exit_button.png"),
        LoadTexture("assets/menu/play_button.png")
    };

    Texture2D menuhover[] = {
        LoadTexture("assets/menu/exit_button_hover.png"),
        LoadTexture("assets/menu/play_button_hover.png")
    };

    Rectangle topButton = {463, 332, 360, 68};
    Rectangle bottomButton = {460, 444, 360, 68};

    screen current = screen::menu;

    while (!WindowShouldClose() && current != screen::exit) {
        Cursor = GetMousePosition();

        bool topHover = CheckCollisionPointRec(Cursor, topButton);
        bool bottomHover = CheckCollisionPointRec(Cursor, bottomButton);
        bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexture(bg, 0, 0, WHITE);

        if (current == screen::menu) {
            DrawTexture(title, 0, 0, WHITE);

            // Play button (top)
            if (topHover) {
                DrawTexture(menuhover[1], 0, 0, WHITE);

                if (clicked) {
                    current = screen::modes;
                }
            } else {
                DrawTexture(menupngs[1], 0, 0, WHITE);
            }

            // Exit button (bottom)
            if (bottomHover) {
                DrawTexture(menuhover[0], 0, 0, WHITE);

                if (clicked) {
                    current = screen::exit;
                }
            } else {
                DrawTexture(menupngs[0], 0, 0, WHITE);
            }
        }
        else if (current == screen::modes) {
            DrawTexture(sport_title, 0, 0, WHITE);

            // Tennis button (top)
            if (topHover) {
                DrawTexture(sporthover[0], 0, 0, WHITE);

                if (clicked) {
                    current = screen::game1;
                }
            } else {
                DrawTexture(sportpngs[0], 0, 0, WHITE);
            }

            // Back button (bottom)
            if (bottomHover) {
                DrawTexture(sporthover[1], 0, 0, WHITE);

                if (clicked) {
                    current = screen::menu;
                }
            } else {
                DrawTexture(sportpngs[1], 0, 0, WHITE);
            }
        }
        else if (current == screen::game1) {
            DrawText("TENNIS GAME", 500, 300, 40, WHITE);

            if (IsKeyPressed(KEY_ESCAPE)) {
                current = screen::modes;
            }
        }

        EndDrawing();
    }

    // Unload all textures
    UnloadTexture(bg);
    UnloadTexture(title);
    UnloadTexture(sport_title);

    for (int i = 0; i < 2; i++) {
        UnloadTexture(menupngs[i]);
        UnloadTexture(menuhover[i]);
        UnloadTexture(sportpngs[i]);
        UnloadTexture(sporthover[i]);
    }

    CloseWindow();
    return 0;
}
