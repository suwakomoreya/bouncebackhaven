
#include <bits/stdc++.h>
#include "raylib.h"
#include "game.h"
#include "ui.h"
#include "tennis.h"
int main() {
    InitWindow(screenw, screenh, "bounce back haven");
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    Vector2 Cursor;

    Texture2D bg = LoadTexture("assets/backgrounds/menu_background.png");
    Texture2D title = LoadTexture("assets/menu/title.png");
    Texture2D sport_title = LoadTexture("assets/sports/sport_choice_title.png");

    Texture2D sportpngs[] = {
        LoadTexture("assets/sports/tennis_button.png"),
        LoadTexture("assets/sports/golf_button.png"),
        LoadTexture("assets/sports/back_button.png")
    };

    Texture2D sporthover[] = {
        LoadTexture("assets/sports/tennis_button_hover.png"),
        LoadTexture("assets/sports/golf_button_hover.png"),
        LoadTexture("assets/sports/back_button_hover.png")
    };

    Texture2D menupngs[] = {
        LoadTexture("assets/menu/exit_button.png"),
        LoadTexture("assets/menu/play_button.png")
    };

    Texture2D menuhover[] = {
        LoadTexture("assets/menu/exit_button_hover.png"),
        LoadTexture("assets/menu/play_button_hover.png")
    };

    screen current = screen::menu;
    bool tennisStarted = false;

    while(!WindowShouldClose() && current != screen::exit){
        Cursor = GetMousePosition();

        bool play = CheckCollisionPointRec(Cursor,{463,332,360,68});
        bool exit = CheckCollisionPointRec(Cursor,{460,444,360,68});

        bool back = CheckCollisionPointRec(Cursor,{463,476,360,68});
        bool game1 = CheckCollisionPointRec(Cursor,{463,300,360,68});
        bool game2 = CheckCollisionPointRec(Cursor,{463,388,360,68});

        //drawing
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexture(bg,0,0,WHITE);

        if(current == screen::menu){
            DrawTexture(title,0,0,WHITE);

            if(play){
                DrawTexture(menuhover[1],0,0,WHITE);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    current = screen::modes;
                }
            }
            else{
                DrawTexture(menupngs[1],0,0,WHITE);
            }

            if(exit){
                DrawTexture(menuhover[0],0,0,WHITE);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    current = screen::exit;
                }
            }
            else{
                DrawTexture(menupngs[0],0,0,WHITE);
            }
        }

        else if(current == screen::modes){
            DrawTexture(sport_title,0,0,WHITE);
            if(game1){
                DrawTexture(sporthover[0],0,0,WHITE);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    tennisStarted = false;
                    current = screen::game1;
                }
            }
            else{
                DrawTexture(sportpngs[0],0,0,WHITE);
            }

            //golf
            if(game2){
                DrawTexture(sporthover[1],0,0,WHITE);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    current = screen::game2;
                }
            }
            else{
                DrawTexture(sportpngs[1],0,0,WHITE);
            }

            //back
            if(back){
                DrawTexture(sporthover[2],0,0,WHITE);
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    current = screen::menu;
                }
            }
            else{
                DrawTexture(sportpngs[2],0,0,WHITE);
            }
        }

        else if(current == screen::game1){
            if(!tennisStarted){
                tennis::Reset();
                tennisStarted = true;
            }
            tennis::Update();
            tennis::Draw();
            if(tennis::WantsExit()){
                current = screen::modes;
            }
        }

        else if(current == screen::game2){
            DrawText("GOLF GAME",500,300,40,WHITE);

            if(IsKeyPressed(KEY_ESCAPE)){
                current = screen::modes;
            }
        }

        EndDrawing();
    }

    //unload textures
    UnloadTexture(bg);
    UnloadTexture(title);
    UnloadTexture(sport_title);

    for(int i=0;i<2;i++){
        UnloadTexture(menupngs[i]);
        UnloadTexture(menuhover[i]);
    }

    for(int i=0;i<3;i++){
        UnloadTexture(sportpngs[i]);
        UnloadTexture(sporthover[i]);
    }

    tennis::Shutdown();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
