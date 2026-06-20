#ifndef BOSS_H
#define BOSS_H

#include <stdlib.h>
#include <raylib.h>
#include <stdio.h>

extern int width;
extern int height;
extern float width_scale;

typedef struct BossBullet {
  struct BossBullet *prev;
  float x;
  float y;
  Rectangle hitbox;
  struct BossBullet *next;
} BossBullet;

typedef struct BossBulletList {
  struct BossBullet *head;
  struct BossBullet *tail;
} BossBulletList;

typedef struct BossRocket {
  struct BossRocket *prev;
  float x;
  float y;
  Rectangle hitbox;
  struct BossRocket *next;
} BossRocket;

typedef struct BossRocketList {
  struct BossRocket *head;
  struct BossRocket *tail;
} BossRocketList;

typedef struct Boss {
  float x;
  float y;
  int health;
  float bar_x;
  float bar_y;
  int explosion_counter;
  int is_destroyed;
  double last_bullets_shoot_time;
  double last_rocket_shoot_time;
  double lastDamagedTime;
  int speed;
  int bullet_speed;
  int rocket_speed;
  Rectangle hitbox;
  BossBulletList *bullets_list;
  BossRocketList *rocket_list;
} Boss;

void move_boss(Boss *boss);

void add_boss_bullets(BossBulletList *p, float boss_x, float boss_y);

void delete_boss_bullet(BossBulletList *list, BossBullet *p);

void add_boss_rocket(BossRocketList *p, float boss_x, float boss_y);

void delete_boss_rocket(BossRocketList *list, BossRocket *p);

#endif