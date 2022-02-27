
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
#include "sprite_stuff.hpp"
#include "enemies.hpp"
#include "keybinds.hpp"
#include "main.hpp"
#include "TileMap.h"
#include "enemy_entities.h"

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

void updateCamera(Camera2D *camera, Vector2 playerPos, int width, int height)
{
    static Vector2 bbox = { 0.15f, 0.15f };

    Vector2 bboxWorldMin = GetScreenToWorld2D({ (1 - bbox.x)*0.5f*width, (1 - bbox.y)*0.5f*height }, *camera);
    Vector2 bboxWorldMax = GetScreenToWorld2D({ (1 + bbox.x)*0.5f*width, (1 + bbox.y)*0.5f*height }, *camera);
    camera->offset = { (1 - bbox.x)*0.5f * width, (1 - bbox.y)*0.5f*height };

    if (playerPos.x < bboxWorldMin.x) camera->target.x = playerPos.x;
    if (playerPos.y < bboxWorldMin.y) camera->target.y = playerPos.y;
    if (playerPos.x > bboxWorldMax.x) camera->target.x = bboxWorldMin.x + (playerPos.x - bboxWorldMax.x);
    if (playerPos.y > bboxWorldMax.y) camera->target.y = bboxWorldMin.y + (playerPos.y - bboxWorldMax.y);
}

Rectangle circleToSquare(Vector2 circlePos, float radius, float scale) {
    Rectangle result = {};
    result.x = circlePos.x - (radius * scale);
    result.y = circlePos.y - (radius * scale);
    result.width = (radius * scale) * 2;
    result.height = (radius * scale) * 2;
    return result;
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

    Camera2D camera = { 0 };
    camera.target = playerPos;
    camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

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
        for (int i = 0; i < lastEnemyId; i++) {
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
            for (int i = 0; i < lastEnemyId; i++) {
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
        for (int i = 0; i < lastEnemyId; i++) {
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

        // update bullets / weapons (kill enemies)


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
            lastEnemyId = sortedIndex;
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
        for (int i = 0; i < lastEnemyId; i++) {
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

        updateCamera(&camera, playerPos, SCREEN_WIDTH, SCREEN_HEIGHT);

        BeginDrawing();

        ClearBackground(GRAY);

        BeginMode2D(camera);

        // draw floor
        Rectangle floorSource[] = {
                {floor_1.x, floor_1.y, floor_1.width, floor_1.height},
                {floor_2.x, floor_2.y, floor_2.width, floor_2.height},
                {floor_3.x, floor_3.y, floor_3.width, floor_3.height},
                {floor_4.x, floor_4.y, floor_4.width, floor_4.height},
                {floor_5.x, floor_5.y, floor_5.width, floor_5.height},
                {floor_6.x, floor_6.y, floor_6.width, floor_6.height},
        };
        auto floorTileHeight = tileMap->tileHeight;
        auto floorTileWidth = tileMap->tileWidth;

        Vector2 beginPosition = GetScreenToWorld2D({0.0f - floorTileWidth, 0.0f - floorTileHeight}, camera);
        Vector2 endPosition = GetScreenToWorld2D({SCREEN_WIDTH + (float)floorTileWidth, SCREEN_HEIGHT + (float)floorTileHeight}, camera);
        for (float y = beginPosition.y; y < endPosition.y; y += floorTileHeight) {
            for (float x = beginPosition.x; x < endPosition.x; x += floorTileWidth) {
                Rectangle quad = {};
                TileInfo tileInfo = tileMap->getTileAtWorldCoords(x, y);
                quad.x = tileInfo.x;
                quad.y = tileInfo.y;
                quad.width = floorTileWidth;
                quad.height = floorTileHeight;
                if (tileInfo.tileType == 0) {
                    DrawRectangleRec(quad, BLACK);
                } else {
                    DrawTexturePro(texture, floorSource[tileInfo.tileType - 1], quad, {0.0f, 0.0f}, 0.0f, WHITE);
                }
            }
        }

        // draw enemies
        for (int i = 0; i < lastEnemyId; i++) {
            assert(enemyEntities.alive[i]);
            Vector2 pos = enemyEntities.position[i];
            float radius = enemyEntities.radius[i];
            EnemyInfo info = enemyInfos[enemyEntities.enemyInfoId[i]];
            Rectangle quad = circleToSquare(pos, radius, 1.25);
            Rectangle textureSource = info.textureSource;
            int animFrame = enemyEntities.animFrame[i];
            if (animDoNextFrame) enemyEntities.animFrame[i] = ++animFrame;
            textureSource.x += textureSource.width * (animFrame % info.animationFrames);
            if (pos.x > playerPos.x) {
                textureSource.width = -textureSource.width;
            }
            auto color = WHITE;
#ifdef SORTING_DEBUG
            if (manualStepping && highlightedMonsterIndex == i) color = RED;
#endif
            DrawTexturePro(texture, textureSource, quad, {0.0f, 0.0f}, 0.0f, color);

#ifdef COLLISION_DEBUG
            Color c = RED;
            c.a = 100;
            DrawCircle(pos.x, pos.y, radius, c);
#endif
        }

        // draw player
        {
            AnimationInfo anim = knight_m_idle_anim;
            if (playerMovedThisFrame)
                anim = knight_m_run_anim;

            if (playerMovedThisFrame && !playerMovedLastFrame)
                playerAnimationFrame = 0;

            if (!playerMovedThisFrame && playerMovedLastFrame)
                playerAnimationFrame = 0;

            if (damageTakenThisFrame) {
                anim = knight_m_hit_anim;
                playerAnimationFrame = 0;
            }


            Rectangle quad = {0};
            float widthScale = playerRadius / anim.width;
            float heightScale = playerRadius / anim.height;
            // the knight has a bunch of extra pixels for height to fit his stupid hat
            float scale = fmax(widthScale, heightScale) * 1.75;
            quad.x = playerPos.x - ((anim.width * scale) * 0.5);
            quad.y = playerPos.y - ((anim.height * scale) * 0.75);
            quad.width = anim.width * scale;
            quad.height = anim.height * scale;

            Rectangle textureSource = { 0 };
            if (animDoNextFrame) ++playerAnimationFrame;
            textureSource.x = anim.x + (anim.width * (playerAnimationFrame % anim.frames));
            textureSource.y = anim.y;
            textureSource.width = anim.width;
            textureSource.height = anim.height;

            if (playerFacing.x < 0) {
                textureSource.width = -textureSource.width;
            }

            DrawTexturePro(texture, textureSource, quad, {0.0f, 0.0f}, 0.0f, WHITE);

            Rectangle hpBar =  {quad.x, quad.y + quad.height, quad.width * (playerHp / maxPlayerHp), 3.0f};
            DrawRectangleRec(hpBar, RED);

#ifdef COLLISION_DEBUG
            Color c = BLUE;
            c.a = 100;
            DrawCircle(playerPos.x, playerPos.y, playerRadius, c);
#endif
        }

        // draw bullets / weapons

        EndMode2D();

#ifdef FLOOR_DEBUG
        DrawText(TextFormat("Camera X %f Camera Y %f", camera.target.x, camera.target.y), 5, 5, 12, BLUE);
        DrawText(TextFormat("Previous Floor X %f Previous Floor Y %f", prevFloorX, prevFloorY), 5, 20, 12, BLUE);
        DrawText(TextFormat("New Floor X %f New Floor Y %f", floorQuad.x, floorQuad.y), 5, 35, 12, BLUE);
        DrawText(TextFormat("Floor X dff %f Floor Y diff %f", floorQuad.x - prevFloorX, floorQuad.y - prevFloorY), 5, 50, 12, BLUE);
        prevFloorX = floorQuad.x;
        prevFloorY = floorQuad.y;
#endif
        EndDrawing();
    }

    CloseWindow();

    return 0;
}