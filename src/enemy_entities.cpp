#include <cstring>
#include "enemy_entities.h"

void copyEnemy(int srcIndex, EnemyEntities * srcEnemyEntities, int destIndex, EnemyEntities * destEnemyEntities) {
      destEnemyEntities->enemyInfoId[destIndex] = srcEnemyEntities->enemyInfoId[srcIndex];
      destEnemyEntities->alive[destIndex] = srcEnemyEntities->alive[srcIndex];
      destEnemyEntities->position[destIndex] = srcEnemyEntities->position[srcIndex];
      destEnemyEntities->nextPosition[destIndex] = srcEnemyEntities->nextPosition[srcIndex];
      destEnemyEntities->velocity[destIndex] = srcEnemyEntities->velocity[srcIndex];
      destEnemyEntities->radius[destIndex] = srcEnemyEntities->radius[srcIndex];
      destEnemyEntities->hp[destIndex] = srcEnemyEntities->hp[srcIndex];
      destEnemyEntities->attack[destIndex] = srcEnemyEntities->attack[srcIndex];
      destEnemyEntities->speed[destIndex] = srcEnemyEntities->speed[srcIndex];
      destEnemyEntities->animFrame[destIndex] = srcEnemyEntities->animFrame[srcIndex];
  }

void swapEnemy(int a, int b, EnemyEntities * enemyEntities) {
      auto enemyInfoId = enemyEntities->enemyInfoId[a];
      auto alive = enemyEntities->alive[a];
      auto position = enemyEntities->position[a];
      auto nextPosition = enemyEntities->nextPosition[a];
      auto velocity = enemyEntities->velocity[a];
      auto radius = enemyEntities->radius[a];
      auto hp = enemyEntities->hp[a];
      auto attack = enemyEntities->attack[a];
      auto speed = enemyEntities->speed[a];
      auto animFrame = enemyEntities->animFrame[a];
  
      enemyEntities->enemyInfoId[a] = enemyEntities->enemyInfoId[b];
      enemyEntities->alive[a] = enemyEntities->alive[b];
      enemyEntities->position[a] = enemyEntities->position[b];
      enemyEntities->nextPosition[a] = enemyEntities->nextPosition[b];
      enemyEntities->velocity[a] = enemyEntities->velocity[b];
      enemyEntities->radius[a] = enemyEntities->radius[b];
      enemyEntities->hp[a] = enemyEntities->hp[b];
      enemyEntities->attack[a] = enemyEntities->attack[b];
      enemyEntities->speed[a] = enemyEntities->speed[b];
      enemyEntities->animFrame[a] = enemyEntities->animFrame[b];
  
      enemyEntities->enemyInfoId[b] = enemyInfoId;
      enemyEntities->alive[b] = alive;
      enemyEntities->position[b] = position;
      enemyEntities->nextPosition[b] = nextPosition;
      enemyEntities->velocity[b] = velocity;
      enemyEntities->radius[b] = radius;
      enemyEntities->hp[b] = hp;
      enemyEntities->attack[b] = attack;
      enemyEntities->speed[b] = speed;
      enemyEntities->animFrame[b] = animFrame;
  }

void swapAndClearEnemyEntities(EnemyEntities * enemyEntitiesA, EnemyEntities * enemyEntitiesB, int count) {
      auto enemyInfoId = enemyEntitiesB->enemyInfoId;
      auto alive = enemyEntitiesB->alive;
      auto position = enemyEntitiesB->position;
      auto nextPosition = enemyEntitiesB->nextPosition;
      auto velocity = enemyEntitiesB->velocity;
      auto radius = enemyEntitiesB->radius;
      auto hp = enemyEntitiesB->hp;
      auto attack = enemyEntitiesB->attack;
      auto speed = enemyEntitiesB->speed;
      auto animFrame = enemyEntitiesB->animFrame;
  
      enemyEntitiesB->enemyInfoId = enemyEntitiesA->enemyInfoId;
      enemyEntitiesB->alive = enemyEntitiesA->alive;
      enemyEntitiesB->position = enemyEntitiesA->position;
      enemyEntitiesB->nextPosition = enemyEntitiesA->nextPosition;
      enemyEntitiesB->velocity = enemyEntitiesA->velocity;
      enemyEntitiesB->radius = enemyEntitiesA->radius;
      enemyEntitiesB->hp = enemyEntitiesA->hp;
      enemyEntitiesB->attack = enemyEntitiesA->attack;
      enemyEntitiesB->speed = enemyEntitiesA->speed;
      enemyEntitiesB->animFrame = enemyEntitiesA->animFrame;
  
      enemyEntitiesA->enemyInfoId = enemyInfoId;
      enemyEntitiesA->alive = alive;
      enemyEntitiesA->position = position;
      enemyEntitiesA->nextPosition = nextPosition;
      enemyEntitiesA->velocity = velocity;
      enemyEntitiesA->radius = radius;
      enemyEntitiesA->hp = hp;
      enemyEntitiesA->attack = attack;
      enemyEntitiesA->speed = speed;
      enemyEntitiesA->animFrame = animFrame;
  
      memset(enemyEntitiesB->enemyInfoId, 0, count * sizeof(int));
      memset(enemyEntitiesB->alive, 0, count * sizeof(bool));
      memset(enemyEntitiesB->position, 0, count * sizeof(Vector2));
      memset(enemyEntitiesB->nextPosition, 0, count * sizeof(Vector2));
      memset(enemyEntitiesB->velocity, 0, count * sizeof(Vector2));
      memset(enemyEntitiesB->radius, 0, count * sizeof(float));
      memset(enemyEntitiesB->hp, 0, count * sizeof(int));
      memset(enemyEntitiesB->attack, 0, count * sizeof(int));
      memset(enemyEntitiesB->speed, 0, count * sizeof(float));
      memset(enemyEntitiesB->animFrame, 0, count * sizeof(int));
  }