#ifndef MONSTERS_ENEMIES_H
#define MONSTERS_ENEMIES_H

#include <raylib.h>
#include "sprite_stuff.h"

#define ANIMRECT(animationName) \
        {animationName.x, animationName.y, animationName.width, animationName.height}, \
        animationName.frames,\

struct EnemyInfo {
    Rectangle textureSource;
    int animationFrames;
    int hp;
    int attack;
    int lootValue;
};

extern EnemyInfo enemy_skeleton;
extern EnemyInfo enemy_orc;
extern EnemyInfo enemyInfos[];

#endif //MONSTERS_ENEMIES_H