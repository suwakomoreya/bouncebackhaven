#include <bits/stdc++.h>;
#include "raylibs.h"

int screenw = GetScreenWidth();
int screenh = GetScreenHeight();

int main(){
    InitWindow(screenw,screenh,"bounce back haven");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    //init
    enum class screen{
        menu,
        game1,
        exit
    };
    screen current = screen::menu;
    while(!WindowShouldClose() || current != screen::exit){
        
    }
}