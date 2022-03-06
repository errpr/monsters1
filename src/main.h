#ifndef MONSTERS_MAIN_H
#define MONSTERS_MAIN_H

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


struct Gamestate {
    // timing
    bool animDoNextFrame;

    // sword
    bool swordActive;
    Vector2 swordTopLeftPos;
    float swordWidth;
    float swordHeight;

    // player
    bool playerMovedThisFrame;
    bool playerMovedLastFrame;
    bool damageTakenThisFrame;
    int playerAnimationFrame;
    Vector2 playerPos;
    Vector2 playerFacing;
    float playerHp;
    float playerHpMax;
    float playerRadius;

    // enemies
    int lastEnemyIndex;
    Vector2 * enemyPosition;
    float * enemyRadius;
    int * enemyInfoId;
    int * animFrame;

    // mostly static
    Camera2D camera;
    int screenWidth;
    int screenHeight;
};

#endif