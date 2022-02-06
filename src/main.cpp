#include <cstdlib>
#include "raylib.h"
#include <raymath.h>
#include <cstring>
#include <assert.h>
#include "sprite_stuff.hpp"
#include "enemies.hpp"
#include "keybinds.hpp"
#include "main.hpp"

#define SCREEN_WIDTH (1280)
#define SCREEN_HEIGHT (720)
#define WINDOW_TITLE "Monsters"
#define MAX_ENEMY_COUNT 2000
#define MAX_COLLISION_CHECK_DISTANCE 32.1f
#define FRICTION_COEFFICIENT 0.1f

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
    enemyEntities->velocity[enemyId] = {0, 0};
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
    destEnemyEntities->velocity[destIndex] = srcEnemyEntities->velocity[srcIndex];
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
    auto momentum = enemyEntities->velocity[a];
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
    enemyEntities->velocity[a] = enemyEntities->velocity[b];
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
    enemyEntities->velocity[b] = momentum;
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
    auto velocity = enemyEntitiesSorted->velocity;
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
    enemyEntitiesSorted->velocity = enemyEntities->velocity;
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
    enemyEntities->velocity = velocity;
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
    memset(enemyEntitiesSorted->velocity, 0, MAX_ENEMY_COUNT * sizeof(Vector2));
    memset(enemyEntitiesSorted->size, 0, MAX_ENEMY_COUNT * sizeof(float));
    memset(enemyEntitiesSorted->hp, 0, MAX_ENEMY_COUNT * sizeof(int));
    memset(enemyEntitiesSorted->attack, 0, MAX_ENEMY_COUNT * sizeof(int));
    memset(enemyEntitiesSorted->speed, 0, MAX_ENEMY_COUNT * sizeof(float));
    memset(enemyEntitiesSorted->animFrame, 0, MAX_ENEMY_COUNT * sizeof(int));
}

void updateCamera(Camera2D *camera, Vector2 playerPos, int width, int height)
{
    static Vector2 bbox = { 0.2f, 0.2f };

    Vector2 bboxWorldMin = GetScreenToWorld2D({ (1 - bbox.x)*0.5f*width, (1 - bbox.y)*0.5f*height }, *camera);
    Vector2 bboxWorldMax = GetScreenToWorld2D({ (1 + bbox.x)*0.5f*width, (1 + bbox.y)*0.5f*height }, *camera);
    camera->offset = { (1 - bbox.x)*0.5f * width, (1 - bbox.y)*0.5f*height };

    if (playerPos.x < bboxWorldMin.x) camera->target.x = playerPos.x;
    if (playerPos.y < bboxWorldMin.y) camera->target.y = playerPos.y;
    if (playerPos.x > bboxWorldMax.x) camera->target.x = bboxWorldMin.x + (playerPos.x - bboxWorldMax.x);
    if (playerPos.y > bboxWorldMax.y) camera->target.y = bboxWorldMin.y + (playerPos.y - bboxWorldMax.y);
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
    float playerTurnSpeed = 3.0f;
    float playerSize = 64.0f;
    int playerAnimationFrame = 0;

    Camera2D camera = { 0 };
    camera.target = playerPos;
    camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    bool manualStepping = false;
    double time = GetTime();
    int highlightedMonsterIndex = 0;
    bool playerMovedThisFrame = false;
    while (!WindowShouldClose())
    {
        bool playerMovedLastFrame = playerMovedThisFrame;
        playerMovedThisFrame = false;
        bool animDoNextFrame = false;

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
            highlightedMonsterIndex++;
            if (highlightedMonsterIndex > lastEnemyId) {
                highlightedMonsterIndex = 0;
            }

            goto draw;
        }

        // update animation frame
        if (time - animTime > animStep) {
            animTime = time;
            animDoNextFrame = true;
        }

        //
        // update enemies
        //

        // update velocity / apply friction / move
        for (int i = 0; i < lastEnemyId; i++) {
            Vector2 towardsPlayer = Vector2Normalize(Vector2Subtract(playerPos, enemyEntities.position[i]));
            Vector2 velocity = Vector2Add(enemyEntities.velocity[i], Vector2Scale(towardsPlayer, enemyEntities.speed[i] * dt));
            Vector2 frictionDirection = Vector2Negate(Vector2Normalize(velocity));
            float frictionMagnitude = Vector2Length(velocity);
            Vector2 frictionVector = Vector2Scale(frictionDirection, frictionMagnitude * frictionMagnitude);
            enemyEntities.velocity[i] = Vector2Add(velocity, Vector2Scale(Vector2Scale(frictionVector, dt), FRICTION_COEFFICIENT));
            enemyEntities.nextPosition[i] = Vector2Add(enemyEntities.position[i], Vector2Scale(velocity, dt));
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
            float size = enemyEntities.size[i];
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
                float otherSize = enemyEntities.size[j];
                if (abs(nextPos.y - otherNextPos.y) > MAX_COLLISION_CHECK_DISTANCE) break;

                float distance = Vector2Distance(otherNextPos, nextPos);
                float sumOfRadii = (size * 0.5) + (otherSize * 0.5);
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
                spawnEnemy(GetRandomValue(0,1), &enemyEntities, spawnLocation, size, (float)GetRandomValue(100, 400));
            }
        }

        // sort by Y value again
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
            swapAndClearEnemyEntities(&enemyEntities, &enemyEntitiesSorted);
            lastEnemyId = sortedIndex; // sorted index is naturally the number of alive entities after the ordering
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

        //
        // draw
        //
        draw:

        updateCamera(&camera, playerPos, SCREEN_WIDTH, SCREEN_HEIGHT);

        BeginDrawing();

        ClearBackground(GRAY);

        BeginMode2D(camera);

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
            auto color = WHITE;
            if (manualStepping && highlightedMonsterIndex == i) color = RED;
            DrawTexturePro(texture, textureSource, quad, {0.0f, 0.0f}, 0.0f, color);
        }

        // draw player
        {

            AnimationInfo anim = knight_m_idle_anim;
            if (playerMovedThisFrame) {
                anim = knight_m_run_anim;
                if (!playerMovedLastFrame) {
                    playerAnimationFrame = 0;
                }
            }

            Vector2 animRatio = Vector2Normalize({anim.width, anim.height});
            Vector2 playerSizedVector = Vector2Scale(animRatio, playerSize);
            Rectangle quad = { playerPos.x, playerPos.y, playerSizedVector.x, playerSizedVector.y };

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
        }

        EndMode2D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}