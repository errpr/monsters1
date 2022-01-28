#include <cstdio>
#include <cstdlib>
#include "raylib.h"
#include <raymath.h>
#include "sprite_stuff.hpp"
#include "enemies.hpp"

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "Monsters"

#define MAX_ENEMY_COUNT 999
#define COLLISION_DEBUG

struct EnemyEntities {
    int * enemyInfoId;
    bool * alive;
    Vector2 * position;
    float * size;
    int * hp;
    int * attack;
    float * speed;
};

struct Collision {
    float angle;
    Vector2 position1;
    Vector2 position2;
    Vector2 position3;
    float size1;
    float size2;
    float correctionDistance;
};

static int lastEnemyId = 0;

int GetNewEnemyId() {
    if (lastEnemyId >= MAX_ENEMY_COUNT) {
        lastEnemyId = 0;
    }
    return lastEnemyId++;
}

static float degrees = 230.0f;
float randomDegrees() {
//    return degrees;
    int r = GetRandomValue(0, 36000);
    return ((float)r) / 100.0f;
}

void spawnEnemy(int enemyInfoId, EnemyEntities * enemyEntities, Vector2 spawnLocation, float size, float speed) {
    int enemyId = GetNewEnemyId();
    enemyEntities->enemyInfoId[enemyId] = enemyInfoId;
    enemyEntities->alive[enemyId] = true;
    enemyEntities->position[enemyId] = {spawnLocation.x, spawnLocation.y};
    enemyEntities->size[enemyId] = size;
    enemyEntities->hp[enemyId] = enemyInfos[enemyInfoId].hp;
    enemyEntities->attack[enemyId] = enemyInfos[enemyInfoId].attack;
    enemyEntities->speed[enemyId] = speed;
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
    float spawnThreshold = 0.01f;

    EnemyEntities enemyEntities = {
            (int *)malloc(sizeof(int) * MAX_ENEMY_COUNT),
            (bool *)calloc(MAX_ENEMY_COUNT, sizeof(bool)),
            (Vector2 *)malloc(sizeof(Vector2) * MAX_ENEMY_COUNT),
            (float *)malloc(sizeof(float) * MAX_ENEMY_COUNT),
            (int *)malloc(sizeof(int) * MAX_ENEMY_COUNT),
            (int *)malloc(sizeof(int) * MAX_ENEMY_COUNT),
            (float *)malloc(sizeof(float) * MAX_ENEMY_COUNT),
    };

    Vector2 playerPos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

    int collisionCounter = 0;
#define MAX_COLLISIONS 100
    Collision collisions[MAX_COLLISIONS] = {0};
    bool manualStepping = false;
    double time = GetTime();

    while (!WindowShouldClose())
    {
        float dt = 0;
        if (manualStepping) {
            dt = 0.033;
            time = time + dt;
        } else {
            dt = GetFrameTime();
            time = GetTime();
        }

        if (IsKeyPressed(KEY_F7)) {
            manualStepping = !manualStepping;
        } else if (manualStepping && !IsKeyPressed(KEY_F1)) {
            goto draw;
        }

        degrees += 2.0 * dt;
        collisionCounter = 0;
        for (int i = 0; i < MAX_COLLISIONS; i++)
            collisions[i] = {0};
        // update animation frame
        if (time - animTime > animStep) {
            animTime = time;
            animFrame++;
        }

        //
        // update enemies
        //

        // move & collide
        for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
            if (!enemyEntities.alive[i]) continue;
            Vector2 newPos = Vector2MoveTowards(enemyEntities.position[i], playerPos, enemyEntities.speed[i]);
            float size = enemyEntities.size[i];

            bool collision = false;
            int collisionRuns = 0;
            do {
                collision = false;
                collisionRuns++;
                float distance = 0;
                float sumOfRadii = 0;
                int j = 0;
                for (; j < MAX_ENEMY_COUNT; j++) {
                    if (j == i) continue;
                    if (!enemyEntities.alive[j]) continue;
                    distance = Vector2Distance(enemyEntities.position[j], newPos);
                    sumOfRadii = (enemyEntities.size[j] / 2) + (size / 2);
                    if (distance < sumOfRadii) {
                        collision = true;
                        break;
                    }
                }

                if (collision) {
                    Vector2 vecDiff = Vector2Subtract(enemyEntities.position[j], newPos);
                    float angle = atan2f(vecDiff.y, vecDiff.x);
                    float originalMoveAngle = Vector2Angle(enemyEntities.position[i], playerPos);
                    float correctionMoveAmount = (distance - sumOfRadii) * (((float)GetRandomValue(1001, 1100) / 1000));

#ifdef COLLISION_DEBUG
                    Collision c = Collision{};
                    c.angle = angle;
                    c.position1 = {enemyEntities.position[j].x, enemyEntities.position[j].y};
                    c.position2 = {newPos.x, newPos.y};
                    c.size1 = enemyEntities.size[j];
                    c.size2 = size;
                    c.correctionDistance = correctionMoveAmount;
#endif

                    newPos.x += cosf(angle) * correctionMoveAmount;
                    newPos.y += sinf(angle) * correctionMoveAmount;

#ifdef COLLISION_DEBUG
                    c.position3 = {newPos.x, newPos.y};
                    collisions[collisionCounter++] = c;
                    if (collisionCounter >= MAX_COLLISIONS)
                        collisionCounter = 0;
#endif
                }
            } while (collision && collisionRuns < 20);
            if (collisionRuns < 20) {
                enemyEntities.position[i] = newPos;
            }
            if (Vector2Distance(newPos, playerPos) < size * 2) {
                // touch player get dizzy
                enemyEntities.alive[i] = false;
            }
        }

        // spawn new enemies
        if (time - spawnTimer > spawnThreshold) {
            spawnTimer = time;
            int spawnDirectionDegrees = randomDegrees() + (((float)GetRandomValue(-200, 200)) / 100.0f);
            Vector2 spawnLocation = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            spawnLocation.x += cos(spawnDirectionDegrees * DEG2RAD) * 300;
            spawnLocation.y += sin(spawnDirectionDegrees * DEG2RAD) * 300;
            float size = 32.0f; //(float)GetRandomValue(16, 64);
            bool blocked = false;
            for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
                if (!enemyEntities.alive[i]) continue;
                float distance = Vector2Distance(enemyEntities.position[i], spawnLocation);
                float sumOfRadii = (enemyEntities.size[i] / 2) + (size / 2);
                if (distance < sumOfRadii) {
                    blocked = true;
                    break;
                }
            }
            if (!blocked) {
                spawnEnemy(GetRandomValue(0,1), &enemyEntities, spawnLocation, size, (float)GetRandomValue(100, 400) / 100.0f);
            }
        }

        //
        // draw
        //
        draw:

        BeginDrawing();

        ClearBackground(GRAY);

        // draw enemies
        for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
            if (enemyEntities.alive[i] == true) {
                Vector2 pos = enemyEntities.position[i];
                float size = enemyEntities.size[i];
                EnemyInfo info = enemyInfos[enemyEntities.enemyInfoId[i]];
                Rectangle quad = { pos.x, pos.y, size, size };
                Rectangle textureSource = info.textureSource;
                textureSource.x += textureSource.width * (animFrame % info.animationFrames);
                DrawTexturePro(texture, textureSource, quad, {0.0f, 0.0f}, 0.0f, WHITE);
            }
        }

        // draw collision debug data
#ifdef COLLISION_DEBUG
        for (int i = 0; i < MAX_COLLISIONS; i++) {
            Collision c = collisions[i];
            if (c.size1 == NULL) continue;
//            DrawCircle(c.position1.x, c.position1.y, c.size1 / 2, BLUE);
            DrawCircle(c.position2.x, c.position2.y, c.size2 / 2, RED);
            DrawCircle(c.position3.x, c.position3.y, c.size2 / 2, GREEN);
            DrawLine(c.position2.x, c.position2.y, c.position3.x, c.position3.y, PURPLE);
        }
#endif

//        char strbuf[128] = {};
//        sprintf(strbuf, "Degrees: %0.2f", degrees);
//        DrawText(strbuf, 0, 0, 16, RED);
//        sprintf(strbuf, "Heading X: %0.2f Heading Y: %0.2f", cos(deg2rad(degrees)), sin(deg2rad(degrees)));
//        DrawText(strbuf, 0, 26, 16, RED);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}