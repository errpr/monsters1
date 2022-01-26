#include <cstdio>
#include <cstdlib>
#include "raylib.h"
#include "sprite_stuff.hpp"
#include "enemies.hpp"

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "Monster survivor"

#define MAX_ENEMY_COUNT 512

struct EnemyEntities {
    int * enemyInfoId;
    bool * alive;
    Vector2 * position;
    Vector2 * size;
    int * hp;
    int * attack;
};

static int lastEnemyId = 0;

int GetNewEnemyId() {
    if (lastEnemyId >= MAX_ENEMY_COUNT) {
        lastEnemyId = 0;
    }
    return lastEnemyId++;
}

void spawnEnemy(int enemyInfoId, EnemyEntities * enemyEntities, Vector2 spawnLocation) {
    int enemyId = GetNewEnemyId();
    enemyEntities->enemyInfoId[enemyId] = enemyInfoId;
    enemyEntities->alive[enemyId] = true;
    enemyEntities->position[enemyId] = {spawnLocation.x, spawnLocation.y};
    enemyEntities->size[enemyId] = {16, 16};
    enemyEntities->hp[enemyId] = enemyInfos[enemyInfoId].hp;
    enemyEntities->attack[enemyId] = enemyInfos[enemyInfoId].attack;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    Texture2D texture = LoadTexture(ASSETS_PATH"0x72_DungeonTilesetII_v1.4.png");

    int animFrame = 0;
    float animStep = 0.15f;
    double animTime = GetTime();

    double spawnTimer = GetTime();
    float spawnThreshold = 1.0f;

    double sizeIncrementTimer = GetTime();
    float sizeIncrementThreshold = 1.0f;

    EnemyEntities enemyEntities = {
            (int *)malloc(sizeof(int) * MAX_ENEMY_COUNT),
            (bool *)malloc(sizeof(bool) * MAX_ENEMY_COUNT),
            (Vector2 *)malloc(sizeof(Vector2) * MAX_ENEMY_COUNT),
            (Vector2 *)malloc(sizeof(Vector2) * MAX_ENEMY_COUNT),
            (int *)malloc(sizeof(int) * MAX_ENEMY_COUNT),
            (int *)malloc(sizeof(int) * MAX_ENEMY_COUNT),
    };

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        double time = GetTime();

        // update animation frame
        if (time - animTime > animStep) {
            animTime = time;
            animFrame++;
        }

        // update enemies
        // size increment
        if (time - sizeIncrementTimer > sizeIncrementThreshold) {
            sizeIncrementTimer = time;
            for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
                enemyEntities.size[i].x++;
                enemyEntities.size[i].y++;
            }
        }

        //

        // spawn new enemies
        if (time - spawnTimer > spawnThreshold) {
            spawnTimer = time;
            int spawnDirectionDegrees = GetRandomValue(0, 360);
            Vector2 spawnLocation = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            spawnLocation.x += (float)sin((float)spawnDirectionDegrees) * 30;
            spawnLocation.y += (float)cos((float)spawnDirectionDegrees) * 30;
            spawnEnemy(0, &enemyEntities, spawnLocation);
        }
        BeginDrawing();

        ClearBackground(GRAY);

        // draw enemies
        for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
            if (enemyEntities.alive[i] == true) {
                Vector2 pos = enemyEntities.position[i];
                Vector2 size = enemyEntities.size[i];
                EnemyInfo info = enemyInfos[enemyEntities.enemyInfoId[i]];
                Rectangle quad = { pos.x, pos.y, size.x, size.y };
                Rectangle textureSource = info.textureSource;
                textureSource.x += textureSource.width * (animFrame % info.animationFrames);
                DrawTexturePro(texture, textureSource, quad, {0.0f, 0.0f}, 0.0f, WHITE);
            }
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}