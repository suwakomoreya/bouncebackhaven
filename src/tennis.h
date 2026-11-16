#pragma once
#include "raylib.h"

// Bullet-hell tennis: barrages of balls cross the net, your racket is the only
// thing between them and your baseline. Contact is the hit.
namespace tennis {

void Reset();       // start (or restart) a run
void Update();      // input + simulation, call once per frame
void Draw();        // court, balls, racket, effects, HUD
bool WantsExit();   // player asked to go back to the menu

}
