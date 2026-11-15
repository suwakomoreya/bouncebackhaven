#include <bits/stdc++.h>;
#include "raylibs.h"
#include "ui.h";
#include "game.h";

int screenw = GetScreenWidth();
int screenh = GetScreenHeight();

//textures
Texture2D bg;


int main(){
    InitWindow(screenw,screenh,"bounce back haven");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    //init
    

    const char*items[] ={
        "Game 1",
        "Game 2",
        "Exit"
    };
    const int count = sizeof(items)/sizeof(items[0]);
    int index =0;


    screen current = screen::menu;
    while(!WindowShouldClose() || current != screen::exit){
        DrawTexture(bg,0,0,WHITE);
        

        if(current == screen::menu){
            if(IsKeyPressed(KEY_S || KEY_DOWN)){
                index = (index+1)%count;
            }
            if(IsKeyPressed(KEY_W || KEY_UP)){
                index = (index-1+count)%count;
            }
            if(IsKeyPressed(KEY_ENTER || KEY_SPACE)){
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
            for(int i=0;i<count;i++){
                Color c = (i==index) ? WHITE : GRAY;
                DrawText(items[i],screenw/2-MeasureText(items[i],20)/2,screenh/2-50+i*30,20,c);

                if(i==index){
                    DrawText(">",screenw/2-MeasureText(items[i],20)/2-50,screenh/2-50+i*30,20,WHITE);
                }
            }
            
        }
}