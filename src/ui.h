#pragma once
#include <bits/stdc++.h>
#include "raylib.h"
#include "game.h"

const int screenw = 1280;
const int screenh = 720;
//textures
Texture2D gamebg[3];
screen current;
int screencount;
void drawbg(){
    if(current == screen::menu){
        DrawTexture(gamebg[0],0,0,WHITE);
    }else if(current == screen::game1){
        DrawTexture(gamebg[1],0,0,WHITE);
    }else if(current == screen::exit){ 
        for(int i=0;i<screencount;i++){
            UnloadTexture(gamebg[i]);
        }
    }


}