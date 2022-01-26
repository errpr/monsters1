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