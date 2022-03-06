//
// Created by steven on 3/6/2022.
//

#include "render.h"
#include "sprite_stuff.h"
#include "enemies.h"
#include <cstdlib>

Rectangle circleToSquare(Vector2 circlePos, float radius, float scale) {
    Rectangle result = {};
    result.x = circlePos.x - (radius * scale);
    result.y = circlePos.y - (radius * scale);
    result.width = (radius * scale) * 2;
    result.height = (radius * scale) * 2;
    return result;
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

void render(Gamestate* gamestate, TileMap* tileMap, Texture2D* texture) {

    updateCamera(&gamestate->camera, gamestate->playerPos, gamestate->screenWidth, gamestate->screenHeight);

    BeginDrawing();

    ClearBackground(GRAY);

    BeginMode2D(gamestate->camera);

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

    Vector2 beginPosition = GetScreenToWorld2D({0.0f - floorTileWidth, 0.0f - floorTileHeight}, gamestate->camera);
    Vector2 endPosition = GetScreenToWorld2D({gamestate->screenWidth + (float)floorTileWidth, gamestate->screenHeight + (float)floorTileHeight}, gamestate->camera);
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
                DrawTexturePro(*texture, floorSource[tileInfo.tileType - 1], quad, {0.0f, 0.0f}, 0.0f, WHITE);
            }
        }
    }

    // draw enemies
    for (int i = 0; i < gamestate->lastEnemyIndex; i++) {
        Vector2 pos = gamestate->enemyPosition[i];
        float radius = gamestate->enemyRadius[i];
        EnemyInfo info = enemyInfos[gamestate->enemyInfoId[i]];
        Rectangle quad = circleToSquare(pos, radius, 1.25);
        Rectangle textureSource = info.textureSource;
        int animFrame = gamestate->animFrame[i];
        if (gamestate->animDoNextFrame) gamestate->animFrame[i] = ++animFrame;
        textureSource.x += textureSource.width * (animFrame % info.animationFrames);
        if (pos.x > gamestate->playerPos.x) {
            textureSource.width = -textureSource.width;
        }
        auto color = WHITE;
#ifdef SORTING_DEBUG
        if (manualStepping && highlightedMonsterIndex == i) color = RED;
#endif
        DrawTexturePro(*texture, textureSource, quad, {0.0f, 0.0f}, 0.0f, color);

#ifdef COLLISION_DEBUG
        Color c = RED;
            c.a = 100;
            DrawCircle(pos.x, pos.y, radius, c);
#endif
    }

    // draw player
    {
        AnimationInfo anim = knight_m_idle_anim;
        if (gamestate->playerMovedThisFrame)
            anim = knight_m_run_anim;

        if (gamestate->playerMovedThisFrame && !gamestate->playerMovedLastFrame)
            gamestate->playerAnimationFrame = 0;

        if (!gamestate->playerMovedThisFrame && gamestate->playerMovedLastFrame)
            gamestate->playerAnimationFrame = 0;

        if (gamestate->damageTakenThisFrame) {
            anim = knight_m_hit_anim;
            gamestate->playerAnimationFrame = 0;
        }


        Rectangle quad = {0};
        float widthScale = gamestate->playerRadius / anim.width;
        float heightScale = gamestate->playerRadius / anim.height;
        // the knight has a bunch of extra pixels for height to fit his stupid hat
        float scale = fmax(widthScale, heightScale) * 1.75;
        quad.x = gamestate->playerPos.x - ((anim.width * scale) * 0.5);
        quad.y = gamestate->playerPos.y - ((anim.height * scale) * 0.75);
        quad.width = anim.width * scale;
        quad.height = anim.height * scale;

        Rectangle textureSource = { 0 };
        if (gamestate->animDoNextFrame) gamestate->playerAnimationFrame += 1;
        textureSource.x = anim.x + (anim.width * (gamestate->playerAnimationFrame % anim.frames));
        textureSource.y = anim.y;
        textureSource.width = anim.width;
        textureSource.height = anim.height;

        if (gamestate->playerFacing.x < 0) {
            textureSource.width = -textureSource.width;
        }

        DrawTexturePro(*texture, textureSource, quad, {0.0f, 0.0f}, 0.0f, WHITE);

        Rectangle hpBar =  {quad.x, quad.y + quad.height, quad.width * (gamestate->playerHp / gamestate->playerHpMax), 3.0f};
        DrawRectangleRec(hpBar, RED);

#ifdef COLLISION_DEBUG
        Color c = BLUE;
            c.a = 100;
            DrawCircle(playerPos.x, playerPos.y, playerRadius, c);
#endif
    }

    // draw bullets / weapons
    {
        if (gamestate->swordActive) {
            Rectangle swordRect = {0};
            swordRect.x = gamestate->swordTopLeftPos.x;
            swordRect.y = gamestate->swordTopLeftPos.y;
            swordRect.width = gamestate->swordWidth;
            swordRect.height = gamestate->swordHeight;
            DrawRectangleRec(swordRect, WHITE);
        }
    }

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
