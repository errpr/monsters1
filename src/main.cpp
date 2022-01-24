#include <cstdio>
#include "raylib.h"
#include "sprite_stuff.hpp"

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "Window title"


int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    Texture2D texture = LoadTexture(ASSETS_PATH"0x72_DungeonTilesetII_v1.4.png");

    int animFrame = 0;
    float animStep = 0.15f;
    double animTime = GetTime();
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        double time = GetTime();
        if (time - animTime > animStep) {
            animFrame++;
            animTime = time;
        }
        BeginDrawing();

        ClearBackground(GRAY);

        Rectangle textureSource = {skelet_run_anim.x, skelet_run_anim.y, skelet_run_anim.width, skelet_run_anim.height};
        textureSource.x += textureSource.width * (animFrame % skelet_run_anim.frames);
        Rectangle quad = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 32, 32};
        DrawTexturePro(texture, textureSource, quad, {0.0f, 0.0f}, 0.0f, WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}