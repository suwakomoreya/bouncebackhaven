#include <bits/stdc++.h>
#include "raylib.h"
#include "game.h"
#include "ui.h"

int main(){
    InitWindow(screenw,screenh,"bounce back haven");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    //textures (must load after InitWindow)
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
        LoadTexture("assets/menu/play_button.png"),
    };
    Texture2D menuhover[] = {
        LoadTexture("assets/menu/exit_button_hover.png"),
        LoadTexture("assets/menu/play_button_hover.png"),
    };
    
    const char*items[] ={
        "Game 1",
        // "Game 2",
        "Exit"
    };
    const int count = 2;
    int index =0;


    screen current = screen::menu;
    while(!WindowShouldClose() && current != screen::exit){
        DrawTexture(bg,0,0,WHITE);
        if(current == screen::menu){
            if(IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)){
                index = (index+1)%count;
            }
            if(IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)){
                index = (index-1+count)%count;
            }
            if(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)){
            
                if(index == 0){
                    current = screen::modes;
                    
                }else if(index == 1){
                    current = screen::game1;
                }else if(index == 2){
                    current = screen::game2;
                }else if(index == 3){
                    current = screen::exit;
                }
            }
        }   

        // drawing
        BeginDrawing();
        ClearBackground(BLACK);

        
        if(current == screen::menu){
            DrawTexture(title,0,0,WHITE);
            for(int i=0;i < count;i++){
                DrawTexture(menupngs[i], 0, 0, WHITE);
                if(i==index){
                    DrawTexture(menuhover[i],0,0,WHITE);
                }
            }

        }
        if(current == screen::game1){

            
        }
        EndDrawing();
    }

    UnloadTexture(bg);
    for(int i=0;i<3;i++) UnloadTexture(menupngs[i]);
    CloseWindow();
    return 0;
}