#include "enemy.h"

void add_enemy(EnemyList *enemy_list, int x, int y, double time) {
    if (enemy_list->tail == NULL) {
        enemy_list->tail = malloc(sizeof(Enemy));
        enemy_list->head = enemy_list->tail;
        enemy_list->tail->prev = NULL;
    } else {
        enemy_list->tail->next = malloc(sizeof(Enemy));
        enemy_list->tail->next->prev = enemy_list->tail;
        enemy_list->tail = enemy_list->tail->next;
    }
    enemy_list->tail->next = NULL;
    enemy_list->tail->x = (float)x;
    enemy_list->tail->y = (float)y;
    enemy_list->tail->health = 100;
    enemy_list->tail->bar_x = (float)x - 7 * width_scale;
    enemy_list->tail->bar_y = (float)y - 3 * width_scale;
    enemy_list->tail->counter_explosion = 0;
    enemy_list->tail->is_destroyed = 0;
    enemy_list->tail->lastShotTime = time;
    enemy_list->tail->hitbox = (Rectangle){x, y, 54 * width_scale, 47 * width_scale};
}

void delete_enemy(EnemyList *list, Enemy *p, char status){
  
  if(status == 'D'){
    p->is_destroyed = 1;
    return;
  }

  if(p->prev != NULL) p->prev->next = p->next;
  else list->head = p->next;

  if(p->next != NULL) p->next->prev = p->prev;
  else list->tail = p->prev;

  free(p);
}

void add_enemy_bullet(EnemyBulletList *p, float enemy_x, float enemy_y){
  if(p->tail == NULL){
    p->tail = malloc(sizeof(EnemyBullet));
    p->head = p->tail;
    p->tail->prev = NULL;
  }
  else{
    p->tail->next = malloc(sizeof(EnemyBullet));
    p->tail->next->prev = p->tail;
    p->tail = p->tail->next;
  }
  
  p->tail->x = enemy_x + 17 * width_scale;
  p->tail->y = enemy_y + 47 * width_scale;
  p->tail->hitbox = (Rectangle){p->tail->x, p->tail->y, 20 * width_scale, 26 * width_scale};
  p->tail->next = NULL;
}

void delete_enemy_bullet(EnemyBulletList *list, EnemyBullet *p){
  if(p->prev != NULL) p->prev->next = p->next;
  else list->head = p->next;

  if(p->next != NULL) p->next->prev = p->prev;
  else list->tail = p->prev;

  free(p);
}