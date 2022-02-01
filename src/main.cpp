#include <cstdlib>
#include "raylib.h"
#include <raymath.h>
#include <cstring>
#include "sprite_stuff.hpp"
#include "enemies.hpp"
#include "keybinds.hpp"

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "Monsters"

#define MAX_ENEMY_COUNT 999
#define COLLISION_DEBUG

struct EnemyEntities {
    int * enemyInfoId;
    bool * alive;
    Vector2 * position;
    Vector2 * nextPosition;
    float * size;
    int * hp;
    int * attack;
    float * speed;
    int * animFrame;
};

#ifdef COLLISION_DEBUG
struct Collision {
    float angle;
    Vector2 position1;
    Vector2 position2;
    Vector2 position3;
    float size1;
    float size2;
    float correctionDistance;
};
#endif

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
    int r = GetRandomValue(10, 3600);
    return ((float)r) / 10.0f;
}

void spawnEnemy(int enemyInfoId, EnemyEntities * enemyEntities, Vector2 spawnLocation, float size, float speed) {
    int enemyId = GetNewEnemyId();
    enemyEntities->enemyInfoId[enemyId] = enemyInfoId;
    enemyEntities->alive[enemyId] = true;
    enemyEntities->position[enemyId] = {spawnLocation.x, spawnLocation.y};
    enemyEntities->nextPosition[enemyId] = {spawnLocation.x, spawnLocation.y};
    enemyEntities->size[enemyId] = size;
    enemyEntities->hp[enemyId] = enemyInfos[enemyInfoId].hp;
    enemyEntities->attack[enemyId] = enemyInfos[enemyInfoId].attack;
    enemyEntities->speed[enemyId] = speed;
    enemyEntities->animFrame[enemyId] = 0;
}

void copyEnemy(int srcIndex, EnemyEntities * srcEnemyEntities, int destIndex, EnemyEntities * destEnemyEntities) {
    destEnemyEntities->enemyInfoId[destIndex] = srcEnemyEntities->enemyInfoId[srcIndex];
    destEnemyEntities->alive[destIndex] = srcEnemyEntities->alive[srcIndex];
    destEnemyEntities->position[destIndex] = srcEnemyEntities->position[srcIndex];
    destEnemyEntities->nextPosition[destIndex] = srcEnemyEntities->nextPosition[srcIndex];
    destEnemyEntities->size[destIndex] = srcEnemyEntities->size[srcIndex];
    destEnemyEntities->hp[destIndex] = srcEnemyEntities->hp[srcIndex];
    destEnemyEntities->attack[destIndex] = srcEnemyEntities->attack[srcIndex];
    destEnemyEntities->speed[destIndex] = srcEnemyEntities->speed[srcIndex];
    destEnemyEntities->animFrame[destIndex] = srcEnemyEntities->animFrame[srcIndex];
}

void swapEnemy(int a, int b, EnemyEntities * enemyEntities) {
    // copy a into stack
    auto enemyInfoId = enemyEntities->enemyInfoId[a];
    auto alive = enemyEntities->alive[a];
    auto position = enemyEntities->position[a];
    auto nextPosition = enemyEntities->nextPosition[a];
    auto size = enemyEntities->size[a];
    auto hp = enemyEntities->hp[a];
    auto attack = enemyEntities->attack[a];
    auto speed = enemyEntities->speed[a];
    auto animFrame = enemyEntities->animFrame[a];

    // b to a
    enemyEntities->enemyInfoId[a] = enemyEntities->enemyInfoId[b];
    enemyEntities->alive[a] = enemyEntities->alive[b];
    enemyEntities->position[a] = enemyEntities->position[b];
    enemyEntities->nextPosition[a] = enemyEntities->nextPosition[b];
    enemyEntities->size[a] = enemyEntities->size[b];
    enemyEntities->hp[a] = enemyEntities->hp[b];
    enemyEntities->attack[a] = enemyEntities->attack[b];
    enemyEntities->speed[a] = enemyEntities->speed[b];
    enemyEntities->animFrame[a] = enemyEntities->animFrame[b];

    // a to b
    enemyEntities->enemyInfoId[b] = enemyInfoId;
    enemyEntities->alive[b] = alive;
    enemyEntities->position[b] = position;
    enemyEntities->nextPosition[b] = nextPosition;
    enemyEntities->size[b] = size;
    enemyEntities->hp[b] = hp;
    enemyEntities->attack[b] = attack;
    enemyEntities->speed[b] = speed;
    enemyEntities->animFrame[b] = animFrame;
}

void swapAndClearEnemyEntities(EnemyEntities * enemyEntities, EnemyEntities * enemyEntitiesSorted) {
    // copy pointers onto stack
    auto enemyInfoId = enemyEntitiesSorted->enemyInfoId;
    auto alive = enemyEntitiesSorted->alive;
    auto position = enemyEntitiesSorted->position;
    auto nextPosition = enemyEntitiesSorted->nextPosition;
    auto size = enemyEntitiesSorted->size;
    auto hp = enemyEntitiesSorted->hp;
    auto attack = enemyEntitiesSorted->attack;
    auto speed = enemyEntitiesSorted->speed;
    auto animFrame = enemyEntitiesSorted->animFrame;

    // b to a
    enemyEntitiesSorted->enemyInfoId = enemyEntities->enemyInfoId;
    enemyEntitiesSorted->alive = enemyEntities->alive;
    enemyEntitiesSorted->position = enemyEntities->position;
    enemyEntitiesSorted->nextPosition = enemyEntities->nextPosition;
    enemyEntitiesSorted->size = enemyEntities->size;
    enemyEntitiesSorted->hp = enemyEntities->hp;
    enemyEntitiesSorted->attack = enemyEntities->attack;
    enemyEntitiesSorted->speed = enemyEntities->speed;
    enemyEntitiesSorted->animFrame = enemyEntities->animFrame;

    // a to b
    enemyEntities->enemyInfoId = enemyInfoId;
    enemyEntities->alive = alive;
    enemyEntities->position = position;
    enemyEntities->nextPosition = nextPosition;
    enemyEntities->size = size;
    enemyEntities->hp = hp;
    enemyEntities->attack = attack;
    enemyEntities->speed = speed;
    enemyEntities->animFrame = animFrame;

    // clear to 0
    memset(enemyEntitiesSorted->enemyInfoId, 0, MAX_ENEMY_COUNT * sizeof(int));
    memset(enemyEntitiesSorted->alive, 0, MAX_ENEMY_COUNT * sizeof(bool));
    memset(enemyEntitiesSorted->position, 0, MAX_ENEMY_COUNT * sizeof(Vector2));
    memset(enemyEntitiesSorted->nextPosition, 0, MAX_ENEMY_COUNT * sizeof(Vector2));
    memset(enemyEntitiesSorted->size, 0, MAX_ENEMY_COUNT * sizeof(float));
    memset(enemyEntitiesSorted->hp, 0, MAX_ENEMY_COUNT * sizeof(int));
    memset(enemyEntitiesSorted->attack, 0, MAX_ENEMY_COUNT * sizeof(int));
    memset(enemyEntitiesSorted->speed, 0, MAX_ENEMY_COUNT * sizeof(float));
    memset(enemyEntitiesSorted->animFrame, 0, MAX_ENEMY_COUNT * sizeof(int));
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    Texture2D texture = LoadTexture(ASSETS_PATH"0x72_DungeonTilesetII_v1.4.png");

    float animStep = 0.15f;
    double animTime = GetTime();

    double spawnTimer = GetTime();
    float spawnThreshold = 0.01f;

    EnemyEntities enemyEntities = {
            (int *)calloc(MAX_ENEMY_COUNT, sizeof(int)),
            (bool *)calloc(MAX_ENEMY_COUNT, sizeof(bool)),
            (Vector2 *)calloc(MAX_ENEMY_COUNT,sizeof(Vector2)),
            (Vector2 *)calloc(MAX_ENEMY_COUNT,sizeof(Vector2)),
            (float *)calloc(MAX_ENEMY_COUNT,sizeof(float)),
            (int *)calloc(MAX_ENEMY_COUNT,sizeof(int)),
            (int *)calloc(MAX_ENEMY_COUNT,sizeof(int)),
            (float *)calloc(MAX_ENEMY_COUNT,sizeof(float)),
            (int *)calloc(MAX_ENEMY_COUNT,sizeof(int)),
    };

    // for sorting
    EnemyEntities enemyEntitiesSorted = {
            (int *)calloc(MAX_ENEMY_COUNT, sizeof(int)),
            (bool *)calloc(MAX_ENEMY_COUNT, sizeof(bool)),
            (Vector2 *)calloc(MAX_ENEMY_COUNT,sizeof(Vector2)),
            (Vector2 *)calloc(MAX_ENEMY_COUNT,sizeof(Vector2)),
            (float *)calloc(MAX_ENEMY_COUNT,sizeof(float)),
            (int *)calloc(MAX_ENEMY_COUNT,sizeof(int)),
            (int *)calloc(MAX_ENEMY_COUNT,sizeof(int)),
            (float *)calloc(MAX_ENEMY_COUNT,sizeof(float)),
            (int *)calloc(MAX_ENEMY_COUNT,sizeof(int)),
    };

    Vector2 playerPos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

#ifdef COLLISION_DEBUG
    int collisionCounter = 0;
#define MAX_COLLISIONS 100
    Collision collisions[MAX_COLLISIONS] = {0};
#endif
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

        if (IsKeyPressed(KB_TOGGLE_MANUAL_STEPPING)) {
            manualStepping = !manualStepping;
        } else if (manualStepping && !IsKeyPressed(KB_MANUAL_STEP)) {
            goto draw;
        }

#ifdef COLLISION_DEBUG
        collisionCounter = 0;
        for (int i = 0; i < MAX_COLLISIONS; i++)
            collisions[i] = {0};
#endif
        // update animation frame
        bool animDoNextFrame = false;
        if (time - animTime > animStep) {
            animTime = time;
            animDoNextFrame = true;
        }

        //
        // update enemies
        //

        // set desired next position
        for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
            if (!enemyEntities.alive[i]) break; // no body died yet, so dead one = end of arrays
            enemyEntities.nextPosition[i] = Vector2MoveTowards(enemyEntities.position[i], playerPos, enemyEntities.speed[i]);
        }

        // check for collisions

        /*
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
                    sumOfRadii = (enemyEntities.size[j] / 3.0) + (size / 3.0);
                    if (distance < sumOfRadii) {
                        collision = true;
                        break;
                    }
                }

                if (collision) {
                    Vector2 vecDiff = Vector2Subtract(enemyEntities.position[j], newPos);
                    float angle = atan2f(vecDiff.y, vecDiff.x);
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
            } while (collision && collisionRuns < 5);
            if (collisionRuns < 5) {
                enemyEntities.position[i] = newPos;
            }
            if (Vector2Distance(newPos, playerPos) < size * 1.1) {
                // touch player get dizzy
                enemyEntities.alive[i] = false;
            }
        }
        */

        // actually do the move (via pointer swap)
        {
            auto tmp = enemyEntities.position;
            enemyEntities.position = enemyEntities.nextPosition;
            enemyEntities.nextPosition = tmp;
        }

        // spawn new enemies
        if (time - spawnTimer > spawnThreshold) {
            spawnTimer = time;
            int spawnDirectionDegrees = randomDegrees() + (((float)GetRandomValue(-200, 200)) / 100.0f);
            Vector2 spawnLocation = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            spawnLocation.x += cos(spawnDirectionDegrees * DEG2RAD) * 400;
            spawnLocation.y += sin(spawnDirectionDegrees * DEG2RAD) * 400;
            float size = 32.0;//(float)GetRandomValue(16, 64);
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
            if (!enemyEntities.alive[i]) continue;

            Vector2 pos = enemyEntities.position[i];
            float size = enemyEntities.size[i];
            EnemyInfo info = enemyInfos[enemyEntities.enemyInfoId[i]];
            Rectangle quad = { pos.x, pos.y, size, size };
            Rectangle textureSource = info.textureSource;
            int animFrame = enemyEntities.animFrame[i];
            if (animDoNextFrame) enemyEntities.animFrame[i] = ++animFrame;
            textureSource.x += textureSource.width * (animFrame % info.animationFrames);
            if (pos.x > playerPos.x) {
                textureSource.width = -textureSource.width;
            }
            DrawTexturePro(texture, textureSource, quad, {0.0f, 0.0f}, 0.0f, WHITE);
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

        // reorder entities by aliveness, distance to player
        float previousDistance = 0.0f;
        int sortedIndex = 0;
        for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
            if (!enemyEntities.alive[i]) continue;

            float distance = Vector2Distance(enemyEntities.position[i], playerPos);
            if (distance >= previousDistance) {
                previousDistance = distance;
                copyEnemy(i, &enemyEntities, sortedIndex, &enemyEntitiesSorted);
            } else {
                for (int j = sortedIndex; j >= 0; j--) {
                    swapEnemy(j, j+1, &enemyEntitiesSorted);
                    if (distance < Vector2Distance(enemyEntities.position[j], playerPos)) {
                        copyEnemy(i, &enemyEntities, j, &enemyEntitiesSorted);
                    }
                }
            }

            sortedIndex++;
        }
        swapAndClearEnemyEntities(&enemyEntities, &enemyEntitiesSorted);
    }

    CloseWindow();

    return 0;
}