#include "player.h"

void add_player_bullet(PlayerBulletList *player_bullet_list, int player_x, int player_y) {
  if (player_bullet_list->tail == NULL) {
    player_bullet_list->tail = malloc(sizeof(PlayerBullet));
    player_bullet_list->head = player_bullet_list->tail;
    player_bullet_list->tail->prev = NULL;
  } else {
    player_bullet_list->tail->next = malloc(sizeof(PlayerBullet));
    player_bullet_list->tail->next->prev = player_bullet_list->tail;
    player_bullet_list->tail = player_bullet_list->tail->next;
  }
  player_bullet_list->tail->x = player_x + 24 * width_scale;
  player_bullet_list->tail->y = player_y - 10 * width_scale;
  player_bullet_list->tail->hitbox = (Rectangle){player_bullet_list->tail->x, player_bullet_list->tail->y, 9 * width_scale, 22 * width_scale};
  player_bullet_list->tail->next = NULL;
}

void delete_player_bullet(PlayerBulletList *list, PlayerBullet *p) {
  if (p->prev != NULL) p->prev->next = p->next;
  else list->head = p->next;

  if (p->next != NULL) p->next->prev = p->prev;
  else list->tail = p->prev;
  
  free(p);
}

void move_player_right(float *player_x, float *player_bar_x, int speed, Rectangle *player_hitbox, Rectangle *player_hitbox2) {
  *player_x += speed;
  (*player_hitbox).x += speed;
  (*player_hitbox2).x = *player_x + 18 * width_scale;
  *player_bar_x += speed;
  if (*player_x + 56 * width_scale > width * width_scale) {
    *player_x = (width - 56) * width_scale;
    (*player_hitbox).x = (width - 56) * width_scale;
    *player_bar_x -= speed;
  }
}

void move_player_left(float *player_x, float *player_bar_x, int speed, Rectangle *player_hitbox, Rectangle *player_hitbox2) {
  *player_x -= speed;
  *player_bar_x -= speed;
  (*player_hitbox).x -= speed;
  (*player_hitbox2).x = *player_x + 18 * width_scale;
  if (*player_x < 0) {
    *player_x = 0;
    (*player_hitbox).x = 0;
    *player_bar_x += speed;
  }
}