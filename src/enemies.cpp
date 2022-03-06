//
// Created by steven on 3/6/2022.
//


#include "enemies.h"

#define ANIMRECT(animationName) \
        {animationName.x, animationName.y, animationName.width, animationName.height}, \
        animationName.frames,\

EnemyInfo enemy_skeleton = {
        ANIMRECT(skelet_run_anim)
        1,
        1,
        1
};

EnemyInfo enemy_orc {
        ANIMRECT(orc_warrior_run_anim)
        2,
        2,
        2
};

EnemyInfo enemyInfos[] = {
        enemy_skeleton,
        enemy_orc
};