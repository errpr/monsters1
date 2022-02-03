
struct EnemyEntities {
    int * enemyInfoId;
    bool * alive;
    Vector2 * position;
    Vector2 * nextPosition;
    Vector2 * velocity;
    float * size;
    int * hp;
    int * attack;
    float * speed;
    int * animFrame;
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