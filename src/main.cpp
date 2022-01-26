#include <cstdio>
#include <cstdlib>
#include "raylib.h"
#include "sprite_stuff.hpp"
#include "enemies.hpp"

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "Monster survivor"

#define MAX_ENEMY_COUNT 100

struct EnemyEntities {
    int * enemyInfoId;
    bool * alive;
    Vector2 * position;
    Vector2 * size;
    int * hp;
    int * attack;
    float * headingDegrees;
};

static int lastEnemyId = 0;

double deg2rad( double degrees )
{
    return degrees * PI / 180;
}

int GetNewEnemyId() {
    if (lastEnemyId >= MAX_ENEMY_COUNT) {
        lastEnemyId = 0;
    }
    return lastEnemyId++;
}
static double degrees = 0.0f;
double randomDegrees() {
    return degrees;
    int r = GetRandomValue(0, 36000);
    return ((double)r) / 100.0f;
}

void spawnEnemy(int enemyInfoId, EnemyEntities * enemyEntities, Vector2 spawnLocation) {
    int enemyId = GetNewEnemyId();
    enemyEntities->enemyInfoId[enemyId] = enemyInfoId;
    enemyEntities->alive[enemyId] = true;
    enemyEntities->position[enemyId] = {spawnLocation.x, spawnLocation.y};
    enemyEntities->size[enemyId] = {16, 16};
    enemyEntities->hp[enemyId] = enemyInfos[enemyInfoId].hp;
    enemyEntities->attack[enemyId] = enemyInfos[enemyInfoId].attack;
    enemyEntities->headingDegrees[enemyId] = randomDegrees();
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
    float spawnThreshold = 0.1f;

    double sizeIncrementTimer = GetTime();
    float sizeIncrementThreshold = 1.0f;

    double wanderTimer = GetTime();
    float wanderThreshold = 2.0f;

    EnemyEntities enemyEntities = {
            (int *)malloc(sizeof(int) * MAX_ENEMY_COUNT),
            (bool *)calloc(MAX_ENEMY_COUNT, sizeof(bool)),
            (Vector2 *)malloc(sizeof(Vector2) * MAX_ENEMY_COUNT),
            (Vector2 *)malloc(sizeof(Vector2) * MAX_ENEMY_COUNT),
            (int *)malloc(sizeof(int) * MAX_ENEMY_COUNT),
            (int *)malloc(sizeof(int) * MAX_ENEMY_COUNT),
            (float *)malloc(sizeof(float) * MAX_ENEMY_COUNT),
    };

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        degrees += 1.0f * dt;
        if (degrees > 360.0f) {
            degrees = 0.0f;
        }

        double time = GetTime();

        // update animation frame
        if (time - animTime > animStep) {
            animTime = time;
            animFrame++;
        }

        //
        // update enemies
        //

        // size increment
        if (time - sizeIncrementTimer > sizeIncrementThreshold) {
            sizeIncrementTimer = time;
            for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
                enemyEntities.size[i].x++;
                enemyEntities.size[i].y++;
            }
        }

        // change directions
        if (time - wanderTimer > wanderThreshold) {
            wanderTimer = time;
            for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
                enemyEntities.headingDegrees[i] = randomDegrees();
            }
        }

        // move
        static float enemySpeed = 2.0f;
        for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
            if (!enemyEntities.alive[i]) continue;
            float heading = enemyEntities.headingDegrees[i];
            Vector2 position = enemyEntities.position[i];
            float newX = position.x + (cos(deg2rad(heading)) * enemySpeed);
            float newY = position.y + (sin(deg2rad(heading)) * enemySpeed);
            bool bounce = false;
            if (newX > SCREEN_WIDTH - 16) {
                newX = SCREEN_WIDTH - 16;
                bounce = true;
            } else if (newX < 0) {
                newX = 0;
                bounce = true;
            } else if (newY > SCREEN_HEIGHT - 16) {
                newY = SCREEN_HEIGHT - 16;
                bounce = true;
            } else if (newY < 0) {
                newY = 0;
                bounce = true;
            }
            if (bounce) {
                enemyEntities.headingDegrees[i] = fmod(heading + 180, 360);
            }
            enemyEntities.position[i].x = newX;
            enemyEntities.position[i].y = newY;
        }

        // spawn new enemies
        if (time - spawnTimer > spawnThreshold) {
            spawnTimer = time;
            int spawnDirectionDegrees = randomDegrees();
            Vector2 spawnLocation = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            spawnLocation.x += cos(deg2rad(spawnDirectionDegrees)) * 30;
            spawnLocation.y += sin(deg2rad(spawnDirectionDegrees)) * 30;
            spawnEnemy(GetRandomValue(0,1), &enemyEntities, spawnLocation);
        }

        //
        // draw
        //

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

        char strbuf[128] = {};
        sprintf(strbuf, "Degrees: %0.2f", degrees);
        DrawText(strbuf, 0, 0, 16, RED);
        sprintf(strbuf, "Heading X: %0.2f Heading Y: %0.2f", cos(deg2rad(degrees)), sin(deg2rad(degrees)));
        DrawText(strbuf, 0, 26, 16, RED);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}