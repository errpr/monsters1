#include "raylib.h"

struct EnemyEntities {
      int * enemyInfoId;
      bool * alive;
      Vector2 * position;
      Vector2 * nextPosition;
      Vector2 * velocity;
      float * radius;
      int * hp;
      int * attack;
      float * speed;
      int * animFrame;
  };

void copyEnemy(int srcIndex, EnemyEntities * srcEnemyEntities, int destIndex, EnemyEntities * destEnemyEntities);
void swapEnemy(int a, int b, EnemyEntities * enemyEntities);
void swapAndClearEnemyEntities(EnemyEntities * enemyEntitiesA, EnemyEntities * enemyEntitiesB, int count);