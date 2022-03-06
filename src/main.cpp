
//#define COLLISION_DEBUG
//#define FLOOR_DEBUG
//#define SORTING_DEBUG

#define SCREEN_WIDTH (1280)
#define SCREEN_HEIGHT (720)
#define WINDOW_TITLE "Monsters"
#define MAX_ENEMY_COUNT 2000
#define MAX_DELTA_TIME 0.5f
#define SMALLEST_ENEMY_RADIUS 16.0f
#define MAX_COLLISION_CHECK_DISTANCE (SMALLEST_ENEMY_RADIUS * 2) + 0.1f
#define SPEED_LIMIT (SMALLEST_ENEMY_RADIUS * MAX_DELTA_TIME)
#define FRICTION_COEFFICIENT 0.1f

#include <cstdlib>
#include "raylib.h"
#include <raymath.h>
#include <cstring>
#include <assert.h>
#include "sprite_stuff.h"
#include "enemies.h"
#include "keybinds.hpp"
#include "main.h"
#include "TileMap.h"
#include "enemy_entities.h"
#include "render.h"

static int lastEnemyIndex = 0;

int GetNewEnemyId() {
    if (lastEnemyIndex >= MAX_ENEMY_COUNT) {
        lastEnemyIndex = 0;
    }
    return lastEnemyIndex++;
}

static float degrees = 230.0f;
float randomDegrees() {
//    return degrees;
    int r = GetRandomValue(10, 3600);
    return ((float)r) / 10.0f;
}

void spawnEnemy(int enemyInfoId, EnemyEntities * enemyEntities, Vector2 spawnLocation, float radius, float speed) {
    int enemyId = GetNewEnemyId();
    enemyEntities->enemyInfoId[enemyId] = enemyInfoId;
    enemyEntities->alive[enemyId] = true;
    enemyEntities->position[enemyId] = {spawnLocation.x, spawnLocation.y};
    enemyEntities->nextPosition[enemyId] = {spawnLocation.x, spawnLocation.y};
    enemyEntities->velocity[enemyId] = {0, 0};
    enemyEntities->radius[enemyId] = radius;
    enemyEntities->hp[enemyId] = enemyInfos[enemyInfoId].hp;
    enemyEntities->attack[enemyId] = enemyInfos[enemyInfoId].attack;
    enemyEntities->speed[enemyId] = speed;
    enemyEntities->animFrame[enemyId] = 0;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    Texture2D texture = LoadTexture(ASSETS_PATH"0x72_DungeonTilesetII_v1.4.png");
    Texture2D floorTexture;
    {
        Image floorImage = LoadImageFromTexture(texture);
        ImageCrop(&floorImage, {floor_1.x, floor_1.y, floor_1.width, floor_1.height});
        floorTexture = LoadTextureFromImage(floorImage);
    }

    TileMap *tileMap = new TileMap(ASSETS_PATH"tilemap.bin", 32.0f);

    float animStep = 0.15f;
    double nextAnimTime = GetTime() + animStep;

    float spawnThreshold = 0.01f;
    double nextSpawnTime = GetTime() + spawnThreshold;

    float swordSlashRate = 1.0f;
    double swordSlashTime = GetTime() + swordSlashRate;
    bool swordActive = false;
    bool swordRight = false;
    double swordLeftStartTime = 0.0f;
    double swordRightStartTime = 0.0f;
    float swordLingerTime = 0.25f;
    Vector2 swordTopLeftPos = {0};
    float swordWidth = 60.0f;
    float swordHeight = 20.0f;
    int swordDamage = 10;

    EnemyEntities enemyEntities = {
            (int *)calloc(MAX_ENEMY_COUNT, sizeof(int)),
            (bool *)calloc(MAX_ENEMY_COUNT, sizeof(bool)),
            (Vector2 *)calloc(MAX_ENEMY_COUNT,sizeof(Vector2)),
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
            (Vector2 *)calloc(MAX_ENEMY_COUNT,sizeof(Vector2)),
            (float *)calloc(MAX_ENEMY_COUNT,sizeof(float)),
            (int *)calloc(MAX_ENEMY_COUNT,sizeof(int)),
            (int *)calloc(MAX_ENEMY_COUNT,sizeof(int)),
            (float *)calloc(MAX_ENEMY_COUNT,sizeof(float)),
            (int *)calloc(MAX_ENEMY_COUNT,sizeof(int)),
    };

    Vector2 playerPos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    Vector2 playerFacing = { 1.0f, 1.0f };
    float playerSpeed = 128.0f;
    float playerTurnSpeed = 4.0f;
    float playerRadius = 16.0f;
    float playerHp = 100.0f;
    float maxPlayerHp = 100.0f;
    int playerAnimationFrame = 0;

#ifdef SORTING_DEBUG
    int highlightedMonsterIndex = 0;
#endif
#ifdef FLOOR_DEBUG
    float prevFloorX = 0.0f;
    float prevFloorY = 0.0f;
#endif

    bool manualStepping = false;
    double time = GetTime();
    bool playerMovedThisFrame = false;
    float speedScale = 1.0f;
    int targetFps = 60;

    Gamestate  * gamestate = (Gamestate *)calloc(1, sizeof(Gamestate));
    gamestate->enemyPosition = (Vector2 *)calloc(MAX_ENEMY_COUNT, sizeof(Vector2));
    gamestate->enemyRadius = (float *)calloc(MAX_ENEMY_COUNT, sizeof(float));
    gamestate->enemyInfoId = (int *)calloc(MAX_ENEMY_COUNT, sizeof(int));
    gamestate->animFrame = (int *)calloc(MAX_ENEMY_COUNT, sizeof(int));
    gamestate->camera = {
            { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f },
            playerPos,
            0.0f,
            1.0f
    };
    gamestate->screenHeight = SCREEN_HEIGHT;
    gamestate->screenWidth = SCREEN_WIDTH;


    while (!WindowShouldClose())
    {
        bool gameOver = false;
        bool playerMovedLastFrame = playerMovedThisFrame;
        playerMovedThisFrame = false;
        bool animDoNextFrame = false;
        bool damageTakenThisFrame = false;

        float dt = 0;
        if (manualStepping) {
            dt = 0.033;
            time = time + dt;
        } else {
            dt = GetFrameTime() * speedScale;
            dt = Clamp(dt, 0.0f, MAX_DELTA_TIME);
            time = GetTime();
        }

        if (IsKeyPressed(KB_INCREASE_SPEED_SCALING)) {
            speedScale *= 1.5f;
        }
        if (IsKeyPressed(KB_DECREASE_SPEED_SCALING)) {
            speedScale *= 0.5f;
        }
        if (speedScale > 0.9f && speedScale < 1.1f) {
            speedScale = 1.0f;
        }

        if (IsKeyPressed(KB_INCREASE_FRAME_RATE)) {
            targetFps += 10;
            SetTargetFPS(targetFps);
        }
        if (IsKeyPressed(KB_DECREASE_FRAME_RATE)) {
            targetFps -= 10;
            if (targetFps < 1) targetFps = 1;
            SetTargetFPS(targetFps);
        }

        if (IsKeyPressed(KB_TOGGLE_MANUAL_STEPPING)) {
            manualStepping = !manualStepping;
        } else if (manualStepping && !IsKeyPressed(KB_MANUAL_STEP)) {
#ifdef SORTING_DEBUG
            highlightedMonsterIndex++;
            if (highlightedMonsterIndex > lastEnemyId) {
                highlightedMonsterIndex = 0;
            }
#endif
            goto draw;
        }

        // update animation frame
        if (time > nextAnimTime) {
            nextAnimTime += animStep;
            animDoNextFrame = true;
        }

        //
        // update enemies
        //

        // update velocity / apply friction / move
        for (int i = 0; i < lastEnemyIndex; i++) {
            Vector2 towardsPlayer = Vector2Normalize(Vector2Subtract(playerPos, enemyEntities.position[i]));
            Vector2 prefrictionVelocity = Vector2Add(enemyEntities.velocity[i], Vector2Scale(towardsPlayer, enemyEntities.speed[i] * dt));
            Vector2 frictionDirection = Vector2Negate(Vector2Normalize(prefrictionVelocity));
            float frictionMagnitude = Vector2Length(prefrictionVelocity);
            Vector2 frictionVector = Vector2Scale(frictionDirection, frictionMagnitude * frictionMagnitude);
            Vector2 rawVelocity = Vector2Add(prefrictionVelocity, Vector2Scale(frictionVector, FRICTION_COEFFICIENT));
            rawVelocity.x = Clamp(rawVelocity.x, -SPEED_LIMIT, SPEED_LIMIT);
            rawVelocity.y = Clamp(rawVelocity.y, -SPEED_LIMIT, SPEED_LIMIT);
            enemyEntities.velocity[i] = rawVelocity;

            enemyEntities.nextPosition[i] = Vector2Add(enemyEntities.position[i], rawVelocity);
        }

        // sort by Y value of nextPos
        {
            float previousY = -INFINITY;
            int sortedIndex = 0;
            for (int i = 0; i < lastEnemyIndex; i++) {
                float y = enemyEntities.nextPosition[i].y;

                copyEnemy(i, &enemyEntities, sortedIndex, &enemyEntities);

                if (y >= previousY) {
                    previousY = y;
                } else {
                    for (int j = sortedIndex; j > 0; j--) {
                        if (enemyEntities.nextPosition[j - 1].y >= y) {
                            swapEnemy(j - 1, j, &enemyEntities);
                        } else {
                            break;
                        }
                    }
                }

                sortedIndex++;
            }
        }


        // check for collisions
        for (int i = 0; i < lastEnemyIndex; i++) {
            Vector2 nextPos = enemyEntities.nextPosition[i];
            float radius = enemyEntities.radius[i];
            int minIndex = 0;
            // since we sort by Y value and everything is a circle, we know once we find something too far in Y nothing more can collide.
            for (int j = i - 1; j >= 0; j--) {
                Vector2 otherNextPos = enemyEntities.nextPosition[j];
                if (abs(nextPos.y - otherNextPos.y) > MAX_COLLISION_CHECK_DISTANCE) break;
                minIndex = j;
            }
            for (int j = minIndex; j < MAX_ENEMY_COUNT; j++) {
                if (j == i) continue; // don't check against self
                Vector2 otherNextPos = enemyEntities.nextPosition[j];
                float otherRadius = enemyEntities.radius[j];
                if (abs(nextPos.y - otherNextPos.y) > MAX_COLLISION_CHECK_DISTANCE) break;

                float distance = Vector2Distance(otherNextPos, nextPos);
                float sumOfRadii = radius + otherRadius;
                if (distance < sumOfRadii) {
                    Vector2 vecDiff = Vector2Subtract(otherNextPos, nextPos);
//                    float collisionNormal = atan2f(vecDiff.y, vecDiff.x);
                    Vector2 collisionNormal = Vector2Normalize(vecDiff);
                    float collisionAmount = (distance - sumOfRadii);
                    Vector2 correction = Vector2Scale(collisionNormal, collisionAmount / 2);
                    enemyEntities.velocity[i] = Vector2Add(enemyEntities.velocity[i], correction);
                    enemyEntities.nextPosition[i] = Vector2Add(enemyEntities.nextPosition[i], correction);
                    Vector2 oppositeCorrection = Vector2Negate(correction);
                    enemyEntities.velocity[j] = Vector2Add(enemyEntities.velocity[j], oppositeCorrection);
                    enemyEntities.nextPosition[j] = Vector2Add(enemyEntities.nextPosition[j], oppositeCorrection);
                }
            }
        }

        // actually do the move (via pointer swap) Maybe we should zero out nextPostion after but I don't think it matters
        {
            auto tmp = enemyEntities.position;
            enemyEntities.position = enemyEntities.nextPosition;
            enemyEntities.nextPosition = tmp;
        }

        // update bullets / weapons (kill enemies)
        if (time > swordSlashTime) {
            swordSlashTime += swordSlashRate;
            swordActive = true;
            swordLeftStartTime = time;
            swordRightStartTime = time + swordLingerTime;
        }

        if (swordActive) {
            // end swording if past its time
            if (swordRightStartTime + swordLingerTime < time) {
                swordActive = false;
                swordRight = false;
            } else if (time > swordRightStartTime) { // swing right
                swordRight = true;
                swordTopLeftPos = Vector2Add(playerPos, {playerRadius, 0});
            } else { // swing left
                swordTopLeftPos = Vector2Add(playerPos, {-playerRadius - swordWidth, 0});
            }

            if (swordActive) {
                Vector2 swordBottomRight = Vector2Add(swordTopLeftPos, {swordWidth, swordHeight});
                for (int i = 0; i < lastEnemyIndex; i++) {
                    if (!enemyEntities.alive[i]) continue;

                    Vector2 pos = enemyEntities.position[i];
                    float radius = enemyEntities.radius[i];

                    if (pos.y < swordTopLeftPos.y - radius)
                        continue;
                    if (pos.y > swordBottomRight.y + radius)
                        continue;
                    if (pos.x < swordTopLeftPos.x - radius)
                        continue;
                    if (pos.x > swordBottomRight.x + radius)
                        continue;

                    enemyEntities.hp[i] -= swordDamage;
                    if (enemyEntities.hp[i] <= 0) {
                        enemyEntities.alive[i] = false;
                    }
                }
            }
        }

        // spawn new enemies
        if (time > nextSpawnTime) {
            nextSpawnTime += spawnThreshold;
            int spawnDirectionDegrees = randomDegrees() + (((float)GetRandomValue(-200, 200)) / 100.0f);
            Vector2 spawnLocation = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            spawnLocation.x += cos(spawnDirectionDegrees * DEG2RAD) * 400;
            spawnLocation.y += sin(spawnDirectionDegrees * DEG2RAD) * 400;
            float size = 16.0;//(float)GetRandomValue(16, 64);
            bool blocked = false;
            for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
                if (!enemyEntities.alive[i]) continue;
                float distance = Vector2Distance(enemyEntities.position[i], spawnLocation);
                float sumOfRadii = enemyEntities.radius[i] + size;
                if (distance < sumOfRadii) {
                    blocked = true;
                    break;
                }
            }
            if (!blocked) {
                spawnEnemy(GetRandomValue(0,1), &enemyEntities, spawnLocation, size, (float)GetRandomValue(playerSpeed / 2, playerSpeed));
            }
        }

        // sort enemies by Y value again
        {
            float previousY = -INFINITY;
            int sortedIndex = 0;
            for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
                if (!enemyEntities.alive[i]) continue;

                float y = enemyEntities.position[i].y;

                copyEnemy(i, &enemyEntities, sortedIndex, &enemyEntitiesSorted);

                if (y >= previousY) {
                    previousY = y;
                } else {
                    for (int j = sortedIndex; j > 0; j--) {
                        if (enemyEntitiesSorted.position[j - 1].y >= y) {
                            swapEnemy(j - 1, j, &enemyEntitiesSorted);
                        } else {
                            break;
                        }
                    }
                }

                sortedIndex++;
            }
            swapAndClearEnemyEntities(&enemyEntities, &enemyEntitiesSorted, MAX_ENEMY_COUNT);
            lastEnemyIndex = sortedIndex;
        }

        // update player
        if (IsKeyDown(KB_MOVE_RIGHT)) {
            playerPos.x += playerSpeed * dt;
            playerFacing.x = Clamp(playerFacing.x + (playerTurnSpeed * dt), -1.0f, 1.0f);
            playerMovedThisFrame = true;
        }
        if (IsKeyDown(KB_MOVE_LEFT)) {
            playerPos.x -= playerSpeed * dt;
            playerFacing.x = Clamp(playerFacing.x - (playerTurnSpeed * dt), -1.0f, 1.0f);
            playerMovedThisFrame = true;
        }
        if (IsKeyDown(KB_MOVE_DOWN)) {
            playerPos.y += playerSpeed * dt;
            playerFacing.y = Clamp(playerFacing.y + (playerTurnSpeed * dt), -1.0f, 1.0f);
            playerMovedThisFrame = true;
        }
        if (IsKeyDown(KB_MOVE_UP)) {
            playerPos.y -= playerSpeed * dt;
            playerFacing.y = Clamp(playerFacing.y - (playerTurnSpeed * dt), -1.0f, 1.0f);
            playerMovedThisFrame = true;
        }

        // collide with enemies (take damage)
        for (int i = 0; i < lastEnemyIndex; i++) {
            if (enemyEntities.position[i].y < playerPos.y - MAX_COLLISION_CHECK_DISTANCE) continue;
            if (enemyEntities.position[i].y > playerPos.y + MAX_COLLISION_CHECK_DISTANCE) continue;
            float distance = Vector2Distance(enemyEntities.position[i], playerPos);
            float sumOfRadii = enemyEntities.radius[i] + playerRadius;
            if (distance < sumOfRadii) {
                damageTakenThisFrame = true;
                playerHp -= enemyEntities.attack[i] * dt;
            }
        }

        // game over
        if (playerHp <= 0) {
            gameOver = true;
        }

        //
        // draw
        //
        draw:

        gamestate->animDoNextFrame = animDoNextFrame;
        gamestate->swordActive = swordActive;
        gamestate->swordTopLeftPos = swordTopLeftPos;
        gamestate->playerMovedThisFrame = playerMovedThisFrame;
        gamestate->playerMovedLastFrame = playerMovedLastFrame;
        gamestate->damageTakenThisFrame = damageTakenThisFrame;
        gamestate->playerAnimationFrame = playerAnimationFrame;
        gamestate->playerPos = playerPos;
        gamestate->playerFacing = playerFacing;
        gamestate->playerHp = playerHp;
        gamestate->playerHpMax = maxPlayerHp;
        gamestate->playerRadius = playerRadius;
        gamestate->lastEnemyIndex = lastEnemyIndex;
        memcpy(gamestate->enemyPosition, enemyEntities.position, MAX_ENEMY_COUNT);
        memcpy(gamestate->enemyRadius, enemyEntities.radius, MAX_ENEMY_COUNT);
        memcpy(gamestate->enemyInfoId, enemyEntities.enemyInfoId, MAX_ENEMY_COUNT);
        memcpy(gamestate->animFrame, enemyEntities.animFrame, MAX_ENEMY_COUNT);

        render(gamestate, tileMap, &texture);
    }

    CloseWindow();

    return 0;
}
