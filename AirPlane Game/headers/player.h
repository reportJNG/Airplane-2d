#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include <stdlib.h>

extern int width;
extern float width_scale;

typedef struct Player{
    float x;
    float y;
    int health;
    double last_dammaged;
    double last_healthed_time;
    int is_destroyed;
    int explosion_counter;
    int speed;
    int bullet_speed;
    Rectangle hitbox;
    Rectangle hitbox2;
    float bar_x;
    float bar_y;
} Player;

typedef struct PlayerBullet {
  struct PlayerBullet *prev;
  float x;
  float y;
  Rectangle hitbox;
  struct PlayerBullet *next;
} PlayerBullet;

typedef struct PlayerBulletList {
  PlayerBullet *head;
  PlayerBullet *tail;
} PlayerBulletList;

void add_player_bullet(PlayerBulletList *player_bullet_list, int player_x, int player_y);

void delete_player_bullet(PlayerBulletList *list, PlayerBullet *p);

void move_player_right(float *player_x, float *player_bar_x, int speed, Rectangle *player_hitbox, Rectangle *player_hitbox2);

void move_player_left(float *player_x, float *player_bar_x, int speed, Rectangle *player_hitbox, Rectangle *player_hitbox2);

#endif