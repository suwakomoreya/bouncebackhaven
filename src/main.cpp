#include <bits/stdc++.h>
#include "raylib.h"
#include "game.h"
#include "ui.h"


int main(){
    InitWindow(screenw,screenh,"bounce back haven");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    //init
    int mousex = GetMouseX();
    int mousey = GetMouseY();
    //textures (must load after InitWindow)
    Texture2D bg = LoadTexture("assets/backgrounds/menu_background.png");
    Texture2D menupngs[3] = {
        LoadTexture("assets/menu/exit_button.png"),
        LoadTexture("assets/menu/play_button.png"),
        LoadTexture("assets/menu/title.png")
    };
    
    
    
    

    const char*items[] ={
        "Game 1",
        "Game 2",
        "Exit"
    };
    const int count = 3;
    int index =0;


    screen current = screen::menu;
    while(!WindowShouldClose() && current != screen::exit){

        if(current == screen::menu){
            if(IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)){
                index = (index+1)%count;
            }
            if(IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)){
                index = (index-1+count)%count;
            }
            if(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)){
                if(index == 0){
                    current = screen::game1;
                }else if(index == 1){
                    current = screen::game2;
                }else if(index == 2){
                    current = screen::exit;
                }
            }
        }   

        // drawing
        BeginDrawing();
        ClearBackground(BLACK);

        
        if(current == screen::menu){
            DrawTexture(menupngs[2], 0, 0, WHITE);
            for(int i=0;i < count;i++){
                DrawTexture(menupngs[i], 0, 0, WHITE);
                if(i==index){
                    
                    DrawTextureEx(menupngs[index], (Vector2){-63.0,-30.0}, 0.0f,1.1f, WHITE);
                }
            }

        }
        EndDrawing();
    }

    UnloadTexture(bg);
    for(int i=0;i<3;i++) UnloadTexture(menupngs[i]);
    CloseWindow();
    return 0;
}