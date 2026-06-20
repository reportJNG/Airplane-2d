#ifndef ENEMY_H
#define ENEMY_H

#include <stdlib.h>
#include <raylib.h>
#include <stdio.h>

extern int width;
extern int height;
extern float width_scale;

typedef struct Enemy {
  struct Enemy *prev;
  float x;
  float y;
  int health;
  float bar_x;
  float bar_y;
  int counter_explosion;
  int is_destroyed;
  double lastShotTime;
  double lastDamagedTime;
  Rectangle hitbox;
  struct Enemy *next;
} Enemy;

typedef struct EnemyList {
  struct Enemy *head;
  struct Enemy *tail;
} EnemyList;

void add_enemy(EnemyList *enemy_list, int x, int y, double time);

void delete_enemy(EnemyList *list, Enemy *p, char status);

typedef struct EnemyBullet {
  struct EnemyBullet *prev;
  float x;
  float y;
  Rectangle hitbox;
  struct EnemyBullet *next;
} EnemyBullet;

typedef struct EnemyBulletList {
  struct EnemyBullet *head;
  struct EnemyBullet *tail;
} EnemyBulletList;

void add_enemy_bullet(EnemyBulletList *p, float enemy_x, float enemy_y);

void delete_enemy_bullet(EnemyBulletList *list, EnemyBullet *p);

#endif