#include "boss.h"

// distance between 2 bullets is 60px

void move_boss(Boss *boss) {
  boss->x += boss->speed;
  boss->hitbox.x += boss->speed;
  boss->bar_x += boss->speed;
  if(boss->x >= (600 - 129) * width_scale){
    boss->speed *= -1;
    boss->x = (600 - 128) * width_scale;
    boss->hitbox.x = (600 - 128) * width_scale;
  }
  else if(boss->x <= 0){
    boss->speed *= -1;
    boss->x = (1) * width_scale;
    boss->hitbox.x += (1) * width_scale;
  }
}

void add_boss_bullets(BossBulletList *p, float boss_x, float boss_y){
  if(p->tail == NULL){
    p->tail = malloc(sizeof(BossBullet));
    p->head = p->tail;
    p->tail->prev = NULL;
    p->tail->x = boss_x + 24.5 * width_scale;
    p->tail->y = boss_y + 113 * width_scale;
    p->tail->hitbox = (Rectangle){p->tail->x, p->tail->y, 10 * width_scale, 29 * width_scale};
    p->tail->next = malloc(sizeof(BossBullet));
  }
  else{
    p->tail->next = malloc(sizeof(BossBullet));
    p->tail->next->prev = p->tail;
    p->tail = p->tail->next;
    p->tail->x = boss_x + 24.5 * width_scale;
    p->tail->y = boss_y + 113 * width_scale;
    p->tail->hitbox = (Rectangle){p->tail->x, p->tail->y, 10 * width_scale, 29 * width_scale};
    p->tail->next = malloc(sizeof(BossBullet));
  }

  p->tail->next->prev = p->tail;
  p->tail = p->tail->next;
  p->tail->x = p->tail->prev->x + 70 * width_scale;
  p->tail->y = p->tail->prev->y;
  p->tail->hitbox = (Rectangle){p->tail->x, p->tail->y, 10 * width_scale, 29 * width_scale};
  p->tail->next = NULL;
}

void delete_boss_bullet(BossBulletList *list, BossBullet *p){
  if(p->prev != NULL) p->prev->next = p->next;
  else list->head = p->next;

  if(p->next != NULL) p->next->prev = p->prev;
  else list->tail = p->prev;

  free(p);
}

void add_boss_rocket(BossRocketList *p, float boss_x, float boss_y){
  if(p->tail == NULL){
    p->tail = malloc(sizeof(BossRocket));
    p->head = p->tail;
    p->tail->prev = NULL;
  }
  else{
    p->tail->next = malloc(sizeof(BossRocket));
    p->tail->next->prev = p->tail;
    p->tail = p->tail->next;
  }
  
  p->tail->x = boss_x + 53.5 * width_scale;
  p->tail->y = boss_y + 113 * width_scale;
  p->tail->hitbox = (Rectangle){p->tail->x, p->tail->y, 22 * width_scale, 40 * width_scale};
  p->tail->next = NULL;
}

void delete_boss_rocket(BossRocketList *list, BossRocket *p){
  if(p->prev != NULL) p->prev->next = p->next;
  else list->head = p->next;

  if(p->next != NULL) p->next->prev = p->prev;
  else list->tail = p->prev;

  free(p);
}