#include "tennis.h"
#include <bits/stdc++.h>

namespace {

const float kCourtL = 355.0f;
const float kCourtR = 925.0f;
const float kCourtT = 8.0f;
const float kCourtB = 718.0f;
const float kNetY   = 357.0f;

const float kPlayerTop = kNetY + 30.0f;   // the net stops you
const float kPlayerBot = kCourtB - 12.0f;

const float kRacketRX = 38.0f;
const float kRacketRY = 40.0f;
const float kBallR    = 11.0f;
//steered across court so the balls dont go out of range
const float kAimL = kCourtL + 18.0f;
const float kAimR = kCourtR - 18.0f;

const float kBaseSpeed  = 235.0f;  // pixels/second at combo 0
const float kHpMax      = 100.0f;
const float kMissDamage = 12.0f;
const float kComboSpeedStep = 0.025f;  // +2.5% ball speed per consecutive hit
const int   kComboSpeedCap = 100;      // same top speed, reached twice as slowly

const Color kBallYellow{223, 255, 79, 255};

struct Ball {
    Vector2 pos{};
    Vector2 vel{};
    float spin = 0.0f;
    bool dead = false;
    bool returned = false;
    std::deque<Vector2> trail;
};

struct Spark {
    Vector2 pos{};
    Vector2 vel{};
    float life = 0.0f;
    float maxLife = 1.0f;
};

struct Game {
    Vector2 racket{};
    std::vector<Ball> balls;
    std::vector<Spark> sparks;

    float hp = kHpMax;
    int score = 0;
    int combo = 0;
    int hits = 0;
    float spawnTimer = 1.2f;

    float hurtFlash = 0.0f;
    float comboPop = 0.0f;
    float time = 0.0f;

    bool over = false;
    bool wantsExit = false;
};

Game g;

int gBestScore = 0;
int gBestCombo = 0;

float randf(float lo, float hi) {
    return lo + (hi - lo) * ((float)GetRandomValue(0, 10000) / 10000.0f);
}

Vector2 vadd(Vector2 a, Vector2 b) { return Vector2{a.x + b.x, a.y + b.y}; }
Vector2 vscale(Vector2 a, float s) { return Vector2{a.x * s, a.y * s}; }
float vlen(Vector2 a) { return sqrtf(a.x * a.x + a.y * a.y); }
Vector2 vnorm(Vector2 a) {
    const float l = vlen(a);
    return l > 0.0001f ? Vector2{a.x / l, a.y / l} : Vector2{0.0f, 1.0f};
}

Vector2 clampToReach(Vector2 origin, Vector2 direction) {
    if (direction.y < 0.001f) direction.y = 0.001f;
    const float toBaseline = kPlayerBot - origin.y;
    const float xAtBaseline = origin.x + direction.x * (toBaseline / direction.y);
    float clampedX = xAtBaseline;
    if (clampedX < kAimL) clampedX = kAimL;
    if (clampedX > kAimR) clampedX = kAimR;
    if (clampedX == xAtBaseline) return direction;
    return vnorm(Vector2{clampedX - origin.x, toBaseline});
}
Vector2 vrot(Vector2 a, float radians) {
    const float c = cosf(radians), s = sinf(radians);
    return Vector2{a.x * c - a.y * s, a.x * s + a.y * c};
}

// --- audio ---------------------------------------------------------------
Sound gHit[3];
Music gTrack;
bool gAudioLoaded = false;
bool gTrackPlaying = false;

const char* kHitPaths[3] = {
    "assets/audio/freesound-community-tennis-ball-being-hit-72611_g91Q2h6q.mp3",
    "assets/audio/freesound-community-tennis-ball-being-hit-72611_puNda1ef.mp3",
    "assets/audio/freesound-community-tennis-ball-being-hit-72611_wRaTEFhb.mp3",
};
const char* kTrackPath =
    "assets/audio/Graffiti - KV Royalty Free Music No Copyright Background Music For Videos Hip Hop Uplifting.mp3";

void loadAudio() {
    if (gAudioLoaded || !IsAudioDeviceReady()) return;
    for (int i = 0; i < 3; ++i) {
        gHit[i] = LoadSound(kHitPaths[i]);
        SetSoundVolume(gHit[i], 0.75f);
    }
    gTrack = LoadMusicStream(kTrackPath);
    gTrack.looping = true;
    SetMusicVolume(gTrack, 0.35f);
    gAudioLoaded = true;
}
//randomize
void playHitSound() {
    if (!gAudioLoaded) return;
    const int index = GetRandomValue(0, 2);
    SetSoundPitch(gHit[index], randf(0.92f, 1.10f));
    PlaySound(gHit[index]);
}

void playMissSound() {
    if (!gAudioLoaded) return;
    SetSoundPitch(gHit[1], 0.62f);
    PlaySound(gHit[1]);
}

void startMusic() {
    if (!gAudioLoaded || gTrackPlaying) return;
    PlayMusicStream(gTrack);
    gTrackPlaying = true;
}

void stopMusic() {
    if (!gAudioLoaded || !gTrackPlaying) return;
    StopMusicStream(gTrack);
    gTrackPlaying = false;
}
//hide curosr
void setCursorHidden(bool hidden) {
    static bool hiddenNow = false;
    if (hidden == hiddenNow) return;
    if (hidden) HideCursor();
    else ShowCursor();
    hiddenNow = hidden;
}

float speedMultiplier() {
    const int combo = g.combo < kComboSpeedCap ? g.combo : kComboSpeedCap;
    return 1.0f + (float)combo * kComboSpeedStep;
}

float nextSpawnDelay() {
    const float delay = 0.85f - (float)g.combo * 0.0065f;
    return delay > 0.16f ? delay : 0.16f;
}

void addBall(Vector2 origin, Vector2 direction, float speed) {
    if (g.balls.size() > 120) return;
    Ball ball;
    ball.pos = origin;
    ball.vel = vscale(clampToReach(origin, direction), speed);
    ball.spin = randf(0.0f, 6.28f);
    ball.trail.push_back(origin);
    g.balls.push_back(ball);
}

void spawnSingle() {
    const Vector2 origin{randf(kCourtL + 40.0f, kCourtR - 40.0f), kCourtT - 10.0f};
    const Vector2 target{randf(kCourtL + 30.0f, kCourtR - 30.0f),
                         randf(kNetY + 50.0f, kPlayerBot - 10.0f)};
    addBall(origin, vnorm(Vector2{target.x - origin.x, target.y - origin.y}),
            kBaseSpeed * speedMultiplier() * randf(0.95f, 1.05f));
}

void spawnFan() {
    const Vector2 origin{randf(kCourtL + 150.0f, kCourtR - 150.0f), kCourtT - 10.0f};
    const Vector2 target{randf(kCourtL + 40.0f, kCourtR - 40.0f),
                         randf(kNetY + 70.0f, kPlayerBot)};
    const Vector2 base = vnorm(Vector2{target.x - origin.x, target.y - origin.y});
    const int count = GetRandomValue(3, 5);
    const float spread = 0.20f;
    for (int i = 0; i < count; ++i) {
        const float angle = ((float)i - (float)(count - 1) / 2.0f) * spread;
        addBall(origin, vrot(base, angle), kBaseSpeed * speedMultiplier() * randf(0.95f, 1.05f));
    }
}

// three balls in a row
void spawnStream() {
    const Vector2 origin{randf(kCourtL + 80.0f, kCourtR - 80.0f), kCourtT - 10.0f};
    const Vector2 target{randf(kCourtL + 40.0f, kCourtR - 40.0f),
                         randf(kNetY + 90.0f, kPlayerBot)};
    const Vector2 base = vnorm(Vector2{target.x - origin.x, target.y - origin.y});
    const float speed = kBaseSpeed * speedMultiplier();
    for (int i = 0; i < 3; ++i) {
        const Vector2 offset = vscale(base, -(float)i * 62.0f);
        addBall(vadd(origin, offset), base, speed);
    }
}

void spawnWave() {
    const int roll = GetRandomValue(0, 9);
    if (roll < 4) spawnSingle();
    else if (roll < 8) spawnFan();
    else spawnStream();
}

void spawnSparks(Vector2 at) {
    const int count = GetRandomValue(6, 10);
    for (int i = 0; i < count; ++i) {
        Spark spark;
        const float angle = randf(0.0f, 6.28f);
        const float speed = randf(60.0f, 260.0f);
        spark.pos = at;
        spark.vel = Vector2{cosf(angle) * speed, sinf(angle) * speed};
        spark.life = spark.maxLife = randf(0.18f, 0.38f);
        g.sparks.push_back(spark);
    }
}

void updateSparks(float dt) {
    for (size_t i = 0; i < g.sparks.size();) {
        Spark& spark = g.sparks[i];
        spark.life -= dt;
        if (spark.life <= 0.0f) {
            g.sparks.erase(g.sparks.begin() + (long)i);
            continue;
        }
        spark.pos = vadd(spark.pos, vscale(spark.vel, dt));
        spark.vel = vscale(spark.vel, 0.90f);
        ++i;
    }
}

void registerHit(Vector2 at) {
    ++g.combo;
    ++g.hits;
    playHitSound();
    if (g.combo > gBestCombo) gBestCombo = g.combo;

    const int multiplier = 1 + g.combo / 5;
    g.score += 100 * multiplier;
    if (g.score > gBestScore) gBestScore = g.score;

    g.comboPop = 1.0f;
    spawnSparks(at);
}

void registerMiss() {
    playMissSound();
    g.hp -= kMissDamage;
    g.combo = 0;
    g.hurtFlash = 0.35f;
    if (g.hp <= 0.0f) {
        g.hp = 0.0f;
        g.over = true;
        if (g.score > gBestScore) gBestScore = g.score;
    }
}

void drawBall(const Ball& ball, Texture2D sprite) {
    const int trailCount = (int)ball.trail.size();
    for (int i = 0; i < trailCount; ++i) {
        const float t = (float)(i + 1) / (float)trailCount;
        DrawCircleV(ball.trail[(size_t)i], kBallR * (0.30f + 0.45f * t),
                    Fade(kBallYellow, 0.06f + 0.18f * t));
    }

    if (sprite.id != 0) {
        const float diameter = kBallR * 2.0f;
        const float scale = diameter / (float)sprite.width;
        DrawTexturePro(sprite,
                       Rectangle{0.0f, 0.0f, (float)sprite.width, (float)sprite.height},
                       Rectangle{ball.pos.x - kBallR, ball.pos.y - kBallR,
                                 (float)sprite.width * scale, (float)sprite.height * scale},
                       Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        return;
    }

    DrawCircleV(Vector2{ball.pos.x + 2.0f, ball.pos.y + 3.0f}, kBallR, Fade(BLACK, 0.30f));
    DrawCircleV(ball.pos, kBallR, kBallYellow);

    const float deg = ball.spin * 57.29578f;
    DrawCircleSectorLines(ball.pos, kBallR - 1.0f, deg, deg + 130.0f, 14, Fade(WHITE, 0.85f));
    DrawCircleSectorLines(ball.pos, kBallR - 1.0f, deg + 180.0f, deg + 310.0f, 14, Fade(WHITE, 0.85f));
}

void drawRacket(Vector2 center, Texture2D racket) {
    if (racket.id != 0) {
        const float headX = 40.0f;
        const float headY = 43.0f;
        DrawTexturePro(racket,
                       Rectangle{0.0f, 0.0f, (float)racket.width, (float)racket.height},
                       Rectangle{center.x - headX, center.y - headY,
                                 (float)racket.width, (float)racket.height},
                       Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        return;
    }

    // Fallback: draw a racket in code if the sprite is missing.
    DrawEllipse((int)center.x + 3, (int)center.y + 4, kRacketRX, kRacketRY, Fade(BLACK, 0.35f));
    DrawEllipse((int)center.x, (int)center.y, kRacketRX, kRacketRY, Fade(Color{16, 20, 36, 255}, 0.80f));
    DrawEllipseLines((int)center.x, (int)center.y, kRacketRX, kRacketRY, RAYWHITE);

    // Strings, clipped to the ellipse by clamping each chord.
    for (int x = -28; x <= 28; x += 7) {
        const float t = (float)x / kRacketRX;
        if (fabsf(t) >= 0.98f) continue;
        const float half = kRacketRY * sqrtf(1.0f - t * t);
        DrawLineEx(Vector2{center.x + (float)x, center.y - half},
                   Vector2{center.x + (float)x, center.y + half}, 1.0f, Fade(RAYWHITE, 0.30f));
    }
    for (int y = -34; y <= 34; y += 7) {
        const float t = (float)y / kRacketRY;
        if (fabsf(t) >= 0.98f) continue;
        const float half = kRacketRX * sqrtf(1.0f - t * t);
        DrawLineEx(Vector2{center.x - half, center.y + (float)y},
                   Vector2{center.x + half, center.y + (float)y}, 1.0f, Fade(RAYWHITE, 0.30f));
    }
}

void drawHud() {
    DrawRectangleRounded(Rectangle{18.0f, 16.0f, 250.0f, 46.0f}, 0.45f, 8, Fade(BLACK, 0.45f));
    DrawText(TextFormat("SCORE  %d", g.score), 34, 28, 24, RAYWHITE);

    // Health
    const float barW = 250.0f, barH = 18.0f;
    const float ratio = g.hp / kHpMax;
    const Color fill = ratio > 0.55f ? Color{120, 220, 120, 255}
                     : ratio > 0.28f ? Color{240, 200, 90, 255}
                                     : Color{240, 90, 90, 255};
    DrawRectangleRounded(Rectangle{18.0f, 70.0f, barW, barH}, 0.6f, 8, Fade(BLACK, 0.45f));
    DrawRectangleRounded(Rectangle{21.0f, 73.0f, (barW - 6.0f) * ratio, barH - 6.0f}, 0.6f, 8, fill);
    DrawText("STAMINA", 18, 92, 16, Fade(RAYWHITE, 0.55f));

    // Rally, right aligned in the right-hand stands so it never hides the ball.
    if (g.combo > 0) {
        const int multiplier = 1 + g.combo / 5;
        const int size = 30 + (int)(10.0f * g.comboPop);
        const char* text = TextFormat("RALLY %d", g.combo);
        const int width = MeasureText(text, size);
        DrawText(text, 1258 - width, 24, size, RAYWHITE);
        const char* mult = TextFormat("x%d", multiplier);
        const int multWidth = MeasureText(mult, 22);
        DrawText(mult, 1258 - multWidth, 24 + size + 4, 22, Fade(kBallYellow, 0.9f));
    }

    if (g.hits == 0 && !g.over) {
        const char* hint = "Move the mouse: touching a ball returns it";
        const int width = MeasureText(hint, 22);
        DrawText(hint, (1280 - width) / 2, 690, 22, Fade(RAYWHITE, 0.75f));
    }
}

void drawGameOver() {
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.68f));

    const char* title = "GAME OVER";
    const int titleSize = 68;
    DrawText(title, (1280 - MeasureText(title, titleSize)) / 2, 210, titleSize, RAYWHITE);

    const char* score = TextFormat("Score  %d", g.score);
    DrawText(score, (1280 - MeasureText(score, 34)) / 2, 320, 34, kBallYellow);

    const char* rally = TextFormat("Best rally  %d", gBestCombo);
    DrawText(rally, (1280 - MeasureText(rally, 26)) / 2, 366, 26, Fade(RAYWHITE, 0.85f));

    const char* best = TextFormat("Best score  %d", gBestScore);
    DrawText(best, (1280 - MeasureText(best, 26)) / 2, 400, 26, Fade(RAYWHITE, 0.55f));

    const char* again = "R  play again        ESC  menu";
    DrawText(again, (1280 - MeasureText(again, 24)) / 2, 476, 24, Fade(RAYWHITE, 0.75f));
}

}  // namespace

namespace tennis {

void Reset() {
    g.balls.clear();
    g.sparks.clear();
    g.racket = Vector2{(kCourtL + kCourtR) / 2.0f, kPlayerBot - 60.0f};
    g.hp = kHpMax;
    g.score = 0;
    g.combo = 0;
    g.hits = 0;
    g.spawnTimer = 1.2f;
    g.hurtFlash = 0.0f;
    g.comboPop = 0.0f;
    g.time = 0.0f;
    g.over = false;
    g.wantsExit = false;

    loadAudio();
    startMusic();
}

bool WantsExit() {
    return g.wantsExit;
}

void Update() {
    float dt = GetFrameTime();
    if (dt > 0.05f) dt = 0.05f;  // don't let a stalled window teleport the ball
    g.time += dt;

    if (gTrackPlaying) UpdateMusicStream(gTrack);

    setCursorHidden(true);

    if (g.comboPop > 0.0f) g.comboPop -= dt * 4.0f;
    if (g.comboPop < 0.0f) g.comboPop = 0.0f;
    if (g.hurtFlash > 0.0f) g.hurtFlash -= dt;
    if (g.hurtFlash < 0.0f) g.hurtFlash = 0.0f;

    if (g.over) {
        updateSparks(dt);
        if (IsKeyPressed(KEY_R)) Reset();
        if (IsKeyPressed(KEY_ESCAPE)) {
            g.wantsExit = true;
            setCursorHidden(false);
            stopMusic();
        }
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        g.wantsExit = true;
        setCursorHidden(false);
        stopMusic();
        return;
    }

    // The racket is the mouse, confined to your half of the court.
    const Vector2 mouse = GetMousePosition();
    g.racket.x = mouse.x;
    g.racket.y = mouse.y;
    if (g.racket.x < kCourtL + kRacketRX) g.racket.x = kCourtL + kRacketRX;
    if (g.racket.x > kCourtR - kRacketRX) g.racket.x = kCourtR - kRacketRX;
    if (g.racket.y < kPlayerTop) g.racket.y = kPlayerTop;
    if (g.racket.y > kPlayerBot) g.racket.y = kPlayerBot;

    g.spawnTimer -= dt;
    if (g.spawnTimer <= 0.0f) {
        spawnWave();
        g.spawnTimer = nextSpawnDelay();
    }

    for (size_t i = 0; i < g.balls.size();) {
        Ball& ball = g.balls[i];
        ball.pos = vadd(ball.pos, vscale(ball.vel, dt));
        ball.spin += dt * 6.0f;

        ball.trail.push_front(ball.pos);
        while (ball.trail.size() > 7) ball.trail.pop_back();

        const float nx = (ball.pos.x - g.racket.x) / (kRacketRX + kBallR);
        const float ny = (ball.pos.y - g.racket.y) / (kRacketRY + kBallR);
        if (!ball.returned && nx * nx + ny * ny <= 1.0f) {
            registerHit(ball.pos);
            ball.returned = true;
            ball.vel.y = -fabsf(ball.vel.y);
            ball.vel.x += nx * 120.0f;
        } else if (!ball.returned && ball.pos.y - kBallR > kCourtB + 40.0f) {
            registerMiss();
            ball.dead = true;
        } else if (ball.returned && ball.pos.y + kBallR < kCourtT - 40.0f) {
            ball.dead = true;
        } else if (ball.pos.y > kCourtB + 260.0f || ball.pos.y < -260.0f || ball.pos.x < -200.0f || ball.pos.x > 1480.0f) {
            ball.dead = true;
        }

        if (ball.dead) {
            g.balls.erase(g.balls.begin() + (long)i);
            continue;
        }
        ++i;
    }

    updateSparks(dt);
}

void Draw() {
    static Texture2D court{};
    static Texture2D racket{};
    static Texture2D ballSprite{};
    static bool courtLoaded = false;
    if (!courtLoaded) {
        court = LoadTexture("assets/backgrounds/tennis_background.png");
        racket = LoadTexture("assets/game1/tennis_racket.png");
        SetTextureFilter(racket, TEXTURE_FILTER_POINT);  // keep the pixel art crisp
        ballSprite = LoadTexture("assets/game1/tennis_ball.png");
        courtLoaded = true;
    }

    if (court.id != 0) DrawTexture(court, 0, 0, WHITE);
    else DrawRectangle(0, 0, 1280, 720, Color{28, 34, 64, 255});

    for (const Ball& ball : g.balls) drawBall(ball, ballSprite);
    for (const Spark& spark : g.sparks) {
        const float t = spark.life / spark.maxLife;
        DrawCircleV(spark.pos, 2.0f + 3.0f * t, Fade(kBallYellow, t));
    }

    if (!g.over) drawRacket(g.racket, racket);

    if (g.hurtFlash > 0.0f) {
        DrawRectangle(0, 0, 1280, 720, Fade(Color{200, 40, 40, 255}, g.hurtFlash * 0.45f));
    }

    drawHud();
    if (g.over) drawGameOver();
}

void Shutdown() {
    stopMusic();
    if (!gAudioLoaded) return;
    for (int i = 0; i < 3; ++i) UnloadSound(gHit[i]);
    UnloadMusicStream(gTrack);
    gAudioLoaded = false;
}

}  // namespace tennis