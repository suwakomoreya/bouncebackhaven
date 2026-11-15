#include <bits/stdc++.h>
#include "raylib.h"
#include "game.h"
#include "ui.h"
#include "udp_receiver.h"

static constexpr int    UDP_PORT    = 7000;   // must match bridge.js --udp
static constexpr float  GAIN        = 90.0f;  // pixels/sec per m/s^2
static constexpr float  MAX_SPEED   = 900.0f; // pixels/sec
static constexpr float  SMOOTHING   = 10.0f;  // higher = snappier
static constexpr float  PLAYER_WIDTH  = 56.0f;
static constexpr float  PLAYER_HEIGHT = 100.0f;

// Flip these if tilt feels inverted on your phone.
static constexpr float  AX_SIGN = -1.0f;
static constexpr float  AY_SIGN = 1.0f;      // screen Y grows downward

// Temporary 9:16 portrait tennis court, centred in the window.
static constexpr float  COURT_HEIGHT = 680.0f;
static constexpr float  COURT_WIDTH  = COURT_HEIGHT * 7.0f / 15.19f;

struct SensorData
{
    float  ax = 0.0f;
    float  ay = 0.0f;
    float  az = 0.0f;
    double lastPacketTime = -1000.0;

    bool Connected(double now) const { return now - lastPacketTime < 0.5; }

    void Apply(const SensorSample& s, double now)
    {
        ax = s.ax;
        ay = s.ay;
        az = s.az;
        lastPacketTime = now;
    }
};
static void DrawTennisCourt(Rectangle court)
{
    const float midY = court.y + court.height * 0.5f;
    const float centerX = court.x + court.width * 0.5f;
    const float alley = court.width * 0.125f;                  // doubles alley
    const float serviceInset = court.height * 0.5f * 0.5385f;  // net -> service line

    const float topServiceY = midY - serviceInset;
    const float bottomServiceY = midY + serviceInset;

    DrawRectangleRec(
        Rectangle{ court.x - 6, court.y - 6, court.width + 12, court.height + 12 },
        Color{ 12, 13, 18, 255 });
    DrawRectangleRec(court, Color{ 28, 62, 96, 255 });
    DrawRectangleRec(
        Rectangle{ court.x, court.y, court.width, court.height * 0.5f },
        Color{ 12, 13, 18, 255 });

    // Outer (doubles) boundary and singles sidelines.
    DrawRectangleLinesEx(court, 3.0f, RAYWHITE);
    DrawLineEx(Vector2{ court.x + alley, court.y },
               Vector2{ court.x + alley, court.y + court.height }, 2.0f, RAYWHITE);
    DrawLineEx(Vector2{ court.x + court.width - alley, court.y },
               Vector2{ court.x + court.width - alley, court.y + court.height }, 2.0f, RAYWHITE);

    // Service lines and centre service line.
    DrawLineEx(Vector2{ court.x + alley, topServiceY },
               Vector2{ court.x + court.width - alley, topServiceY }, 2.0f, RAYWHITE);
    DrawLineEx(Vector2{ court.x + alley, bottomServiceY },
               Vector2{ court.x + court.width - alley, bottomServiceY }, 2.0f, RAYWHITE);
    DrawLineEx(Vector2{ centerX, topServiceY },
               Vector2{ centerX, bottomServiceY }, 2.0f, RAYWHITE);

    // Net and posts.
    DrawLineEx(Vector2{ court.x, midY },
               Vector2{ court.x + court.width, midY }, 4.0f, Color{ 235, 236, 245, 255 });
    DrawCircleV(Vector2{ court.x, midY }, 4.0f, RAYWHITE);
    DrawCircleV(Vector2{ court.x + court.width, midY }, 4.0f, RAYWHITE);

    // Centre marks on the baselines.
    DrawLineEx(Vector2{ centerX, court.y },
               Vector2{ centerX, court.y + 10 }, 2.0f, RAYWHITE);
    DrawLineEx(Vector2{ centerX, court.y + court.height - 10 },
               Vector2{ centerX, court.y + court.height }, 2.0f, RAYWHITE);

}

int main(){
    InitWindow(screenw,screenh,"bounce back haven");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    //init
    int mousex = GetMouseX();
    int mousey = GetMouseY();
    UdpReceiver receiver;
    const bool udpReady = receiver.Start(UDP_PORT);
    if (!udpReady) TraceLog(LOG_WARNING, "Could not bind UDP port %d", UDP_PORT);

    SensorData sensor;
    SensorSample sample;

    const Rectangle court{
        (screenw - COURT_WIDTH) * 0.5f,
        (screenh - COURT_HEIGHT) * 0.5f,
        COURT_WIDTH,
        COURT_HEIGHT
    };

    const float halfWidth  = PLAYER_WIDTH * 0.5f;
    const float halfHeight = PLAYER_HEIGHT * 0.5f;

    // The player may only occupy the bottom half of the court.
    const float playLeft   = court.x + halfWidth;
    const float playRight  = court.x + court.width - halfWidth;
    const float playTop    = court.y + court.height * 0.5f + halfHeight;
    const float playBottom = court.y + court.height - halfHeight;

    Vector2 square{ court.x + court.width * 0.5f, court.y + court.height * 0.75f };
    Vector2 velocity{ 0.0f, 0.0f };
    std::vector<Vector2> trail;

    //textures (must load after InitWindow)
    Texture2D bg = LoadTexture("assets/backgrounds/menu_background.png");
    Texture2D title = LoadTexture("assets/menu/title.png");
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
        const double now = GetTime();
        const float  dt  = std::min(GetFrameTime(), 0.05f);

        if (receiver.Poll(sample)) sensor.Apply(sample, now);

        const bool live = sensor.Connected(now);
        const float ax = live ? sensor.ax : 0.0f;
        const float ay = live ? sensor.ay : 0.0f;

        // Keyboard fallback so the demo is usable without a phone.
        const float kx = (float)(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) -
                         (float)(IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A));
        const float ky = (float)(IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) -
                         (float)(IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W));

        const float targetX = std::clamp(ax * AX_SIGN * GAIN + kx * 520.0f, -MAX_SPEED, MAX_SPEED);
        const float targetY = std::clamp(ay * AY_SIGN * GAIN + ky * 520.0f, -MAX_SPEED, MAX_SPEED);

        const float blend = 1.0f - std::exp(-SMOOTHING * dt);
        velocity.x += (targetX - velocity.x) * blend;
        velocity.y += (targetY - velocity.y) * blend;

        square.x += velocity.x * dt;
        square.y += velocity.y * dt;

        // Stay inside the bottom half of the court.
        if (square.x < playLeft)   { square.x = playLeft;   velocity.x = 0.0f; }
        if (square.x > playRight)  { square.x = playRight;  velocity.x = 0.0f; }
        if (square.y < playTop)    { square.y = playTop;    velocity.y = 0.0f; }
        if (square.y > playBottom) { square.y = playBottom; velocity.y = 0.0f; }

        trail.push_back(square);
        if (trail.size() > 18) trail.erase(trail.begin());

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
            DrawTexture(title,0,0,WHITE);
            for(int i=0;i < count;i++){
                DrawTexture(menupngs[i], 0, 0, WHITE);
                if(i==index){
                    DrawTexture(menuhover[i],0,0,WHITE);
                }
            }

        }
        if(current == screen::game1){
            DrawTennisCourt(court);

        for (size_t i = 0; i < trail.size(); ++i)
        {
            const float alpha = (float)(i + 1) / (float)trail.size() * 0.35f;
            const Color c = Fade(Color{ 80, 210, 255, 255 }, alpha);
            DrawRectangleRounded(
                Rectangle{ trail[i].x - halfWidth, trail[i].y - halfHeight, PLAYER_WIDTH, PLAYER_HEIGHT },
                0.22f, 6, c);
        }

        const Color squareColor = live ? Color{ 80, 210, 255, 255 } : Color{ 110, 112, 130, 255 };
        DrawRectangleRounded(
            Rectangle{ square.x - halfWidth, square.y - halfHeight, PLAYER_WIDTH, PLAYER_HEIGHT },
            0.22f, 8, squareColor);
        DrawCircleV(square, 7.0f, Color{ 14, 14, 20, 255 });

        // ---- HUD -----------------------------------------------------------
        DrawText("h sport demo", 20, 16, 28, RAYWHITE);

        const char* status = !udpReady ? "UDP port 7000 unavailable"
                           : live       ? "LIVE"
                                        : "waiting for phyphox...";
        DrawText(status, 20, 50, 18,
                 !udpReady ? Color{ 235, 110, 110, 255 }
                 : live    ? Color{ 80, 230, 140, 255 }
                           : Color{ 230, 190, 90, 255 });

        if (!live)
        {
            const char* hint = "Tilt your phone - or use the arrow keys";
            DrawText(hint, screenw / 2 - MeasureText(hint, 20) / 2, 620, 20,
                     Color{ 150, 155, 175, 255 });
        }

        DrawText("bridge:  node tools/phyphox-bridge/bridge.js <phone-ip>",
                 20, 672, 16, Color{ 110, 113, 130, 255 });

        EndDrawing();

            
        }
        EndDrawing();
    }

    UnloadTexture(bg);
    for(int i=0;i<3;i++) UnloadTexture(menupngs[i]);
    CloseWindow();
    return 0;
}