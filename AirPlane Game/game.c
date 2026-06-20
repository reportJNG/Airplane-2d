#include "headers/enemy.h"
#include "headers/player.h"
#include "headers/boss.h"
#include <raylib.h>

int width = 600;
int height = 600;
float width_scale;

void clean_player_bullet_list(PlayerBulletList *player_bullet_list, EnemyList *enemy_list, Boss *boss, int *score, Sound *hit, Sound *enemy_explosion,int *boss_chapter, double *start_boss_chapter_time, int *boss_start_fight) {
  if (player_bullet_list->head == NULL)
    return;

  PlayerBullet *bullet = player_bullet_list->head;
  while (bullet != NULL) {
    PlayerBullet *next_bullet = bullet->next;
    if (bullet->y <= -22 * width_scale){
      delete_player_bullet(player_bullet_list, bullet);
      bullet = next_bullet;
      continue;
    }

    // For Enemy
    Enemy *current_enemy = enemy_list->head;
    while(current_enemy != NULL){
      if(CheckCollisionRecs(bullet->hitbox, current_enemy->hitbox)){
        PlaySound(*hit);
        delete_player_bullet(player_bullet_list, bullet);
        current_enemy->health -= 50;
        if(current_enemy->health == 0){
          PlaySound(*enemy_explosion);
          delete_enemy(enemy_list, current_enemy, 'D');
          *score += 5;
          if(*score % 50 == 0){
            *boss_chapter = 1;
            *start_boss_chapter_time = GetTime();
          }
          current_enemy = current_enemy->next;
          continue;
        }
        current_enemy->lastDamagedTime = GetTime();
      }
      current_enemy = current_enemy->next;
    }

    // For Boss
    if(*boss_start_fight && CheckCollisionRecs(bullet->hitbox, boss->hitbox)) {
      PlaySound(*hit);
      delete_player_bullet(player_bullet_list, bullet);
      boss->health -= 25;
      boss->lastDamagedTime = GetTime();
      if(boss->health == 0){
        PlaySound(*enemy_explosion);
        // *boss_chapter = 0;
        *score += 10;
        boss->is_destroyed = 1;
        *boss_start_fight = 0;
      }
    }
    bullet = next_bullet;
  }
}

int main() {
  int result_screen = 0;
  double lastShotTime = -1;
  double lastSpawnEnemy = -10;
  int best_score = 0;
  int score = 0;
  int boss_chapter = 0;
  int boss_start_fight = 0;
  double start_boss_chapter_time = 0;
  int show_boss = 0;
  int opacity_press_enter = 255;
  int speed_change_opacity = -3;

  PlayerBulletList *player_bullet_list = malloc(sizeof(PlayerBulletList));
  player_bullet_list->head = NULL;
  player_bullet_list->tail = NULL;

  EnemyList *enemy_list = malloc(sizeof(EnemyList));
  enemy_list->head = NULL;
  enemy_list->tail = NULL;

  EnemyBulletList *enemy_bullet_list = malloc(sizeof(EnemyBulletList));
  enemy_bullet_list->head = NULL;
  enemy_bullet_list->tail = NULL;

  printf("Width = %d\n", GetMonitorWidth(0));
  printf("Height = %d\n", GetMonitorHeight(0));

  InitWindow(width, height, "Airplane Game");
  InitAudioDevice();

  width_scale = (float)GetMonitorWidth(0) / 1280.0;

  // printf("Scale Width = %f\n", width_scale);
  // printf("Scale Height = %f\n", width_scale);

  SetWindowSize(600 * width_scale, 600 * width_scale);

  int window_x = (int)((float)GetMonitorWidth(0) / 2 - 300 * width_scale);
  int window_y = (int)((float)GetMonitorHeight(0) / 2 - 300 * width_scale);
  SetWindowPosition(window_x, window_y);

  Texture2D loading_background = LoadTexture("images/loading-bg.png");
  loading_background.width *= width_scale;
  loading_background.height *= width_scale;

  BeginDrawing();
  DrawTexture(loading_background, 0, 0, WHITE);
  EndDrawing();
  
  double LastHealthDropedTime = -31;

  // For Textures
  Texture2D background = LoadTexture("images/bg.png");
  background.width *= width_scale;
  background.height *= width_scale;
  Texture2D player_texture = LoadTexture("images/player/airplane.png");
  player_texture.width *= width_scale;
  player_texture.height *= width_scale;
  Texture2D player_bullet_texture = LoadTexture("images/player/bullet.png");
  player_bullet_texture.width *= width_scale;
  player_bullet_texture.height *= width_scale;
  Texture2D player_zero_health_texture = LoadTexture("images/player/health/0-4.png");
  player_zero_health_texture.width *= width_scale;
  player_zero_health_texture.height *= width_scale;
  Texture2D player_first_health_texture = LoadTexture("images/player/health/1-4.png");
  player_first_health_texture.width *= width_scale;
  player_first_health_texture.height *= width_scale;
  Texture2D player_second_health_texture = LoadTexture("images/player/health/2-4.png");
  player_second_health_texture.width *= width_scale;
  player_second_health_texture.height *= width_scale;
  Texture2D player_third_health_texture = LoadTexture("images/player/health/3-4.png");
  player_third_health_texture.width *= width_scale;
  player_third_health_texture.height *= width_scale;
  Texture2D player_full_health_texture = LoadTexture("images/player/health/4-4.png");
  player_full_health_texture.width *= width_scale;
  player_full_health_texture.height *= width_scale;
  Texture2D health_texture = LoadTexture("images/health.png");
  health_texture.width *= width_scale;
  health_texture.height *= width_scale;
  Texture2D enemy_texture = LoadTexture("images/enemy/airplane.png");
  enemy_texture.width *= width_scale;
  enemy_texture.height *= width_scale;
  Texture2D enemy_zero_health_texture = LoadTexture("images/enemy/health/0-2.png");
  enemy_zero_health_texture.width *= width_scale;
  enemy_zero_health_texture.height *= width_scale;
  Texture2D enemy_half_health_texture = LoadTexture("images/enemy/health/1-2.png");
  enemy_half_health_texture.width *= width_scale;
  enemy_half_health_texture.height *= width_scale;
  Texture2D enemy_full_health_texture = LoadTexture("images/enemy/health/2-2.png");
  enemy_full_health_texture.width *= width_scale;
  enemy_full_health_texture.height *= width_scale; 
  Texture2D enemy_bullet_texture = LoadTexture("images/enemy/bullet.png");
  enemy_bullet_texture.width *= width_scale;
  enemy_bullet_texture.height *= width_scale;
  Texture2D boss_texture = LoadTexture("images/boss/airplane.png");
  boss_texture.width *= width_scale;
  boss_texture.height *= width_scale;
  Texture2D boss_bullet_texture = LoadTexture("images/boss/bullet.png");
  boss_bullet_texture.width *= width_scale;
  boss_bullet_texture.height *= width_scale;
  Texture2D boss_rocket_texture = LoadTexture("images/boss/rocket.png");
  boss_rocket_texture.width *= width_scale;
  boss_rocket_texture.height *= width_scale;
  Texture2D boss_first_health_texture = LoadTexture("images/boss/health/1-6.png");
  boss_first_health_texture.width *= width_scale;
  boss_first_health_texture.height *= width_scale;
  Texture2D boss_second_health_texture = LoadTexture("images/boss/health/2-6.png");
  boss_second_health_texture.width *= width_scale;
  boss_second_health_texture.height *= width_scale;
  Texture2D boss_third_health_texture = LoadTexture("images/boss/health/3-6.png");
  boss_third_health_texture.width *= width_scale;
  boss_third_health_texture.height *= width_scale;
  Texture2D boss_fourth_health_texture = LoadTexture("images/boss/health/4-6.png");
  boss_fourth_health_texture.width *= width_scale;
  boss_fourth_health_texture.height *= width_scale;
  Texture2D boss_fifth_health_texture = LoadTexture("images/boss/health/5-6.png");
  boss_fifth_health_texture.width *= width_scale;
  boss_fifth_health_texture.height *= width_scale;
  Texture2D boss_full_health_texture = LoadTexture("images/boss/health/6-6.png");
  boss_full_health_texture.width *= width_scale;
  boss_full_health_texture.height *= width_scale;

  Texture2D explosion[] = {LoadTexture("images/explosion/1.png"), LoadTexture("images/explosion/2.png"), LoadTexture("images/explosion/3.png"), LoadTexture("images/explosion/4.png"), LoadTexture("images/explosion/5.png"), LoadTexture("images/explosion/6.png")};
  for(int i=0; i<6; i++){
    explosion[i].width *= width_scale;
    explosion[i].height *= width_scale;
  }

  Player player = {
    .x = ((float)width / 2 - 28) * width_scale,
    .y = 503 * width_scale,
    .health = 100,
    .last_dammaged = -1,
    .last_healthed_time = -1,
    .is_destroyed = 0,
    .explosion_counter = 0,
    .speed = 5 * width_scale,
    .bullet_speed = 10 * width_scale,
    .hitbox = {player.x, player.y + 20 * width_scale, 56 * width_scale, 20 * width_scale},
    .hitbox2 = {player.x + 18 * width_scale, player.y, 20 * width_scale, 56 * width_scale},
    .bar_x = player.x - 35 * width_scale,
    .bar_y = (560 + (float)player_full_health_texture.height / 2) * width_scale
  };

  Boss boss = {
    .x = (rand() % (471 - 0 + 1) + 0) * width_scale,
    .y = -113 * width_scale,
    .health = 150,
    .speed = 3 * width_scale,
    .bullet_speed = 5 * width_scale,
    .rocket_speed = 8 * width_scale,
    .bar_x = boss.x - 23.5 * width_scale,
    .bar_y = boss.y - 19 * width_scale,
    .explosion_counter = 0,
    .is_destroyed = 0,
    .lastDamagedTime = -1,
    .last_bullets_shoot_time = 5,
    .last_rocket_shoot_time = 5,
    .hitbox = {boss.x, boss.y, boss_texture.width, boss_texture.height},
    .bullets_list = malloc(sizeof(BossBulletList)),
    .rocket_list = malloc(sizeof(BossRocketList)),
  };
  boss.bullets_list->head = NULL;
  boss.bullets_list->tail = NULL;
  boss.rocket_list->head = NULL;
  boss.rocket_list->tail = NULL;

  // For Sounds
  Sound player_shoot_sound = LoadSound("sounds/player-shoot.mp3");
  Sound player_healthed_sound = LoadSound("sounds/player-healthed.mp3");
  Sound enemy_shoot_sound = LoadSound("sounds/enemy-shoot.mp3");
  Sound hit_sound = LoadSound("sounds/hit.mp3");
  Sound player_explosion_sound = LoadSound("sounds/player-explosion.mp3");
  Sound enemy_explosion_sound = LoadSound("sounds/enemy-explosion.mp3");
  // For Icon
  Image icon = LoadImage("icon/icon.png");
  SetWindowIcon(icon);

  float health_x = (rand() % (569 - 0 + 1) + 0) * width_scale;
  float health_y = -44 * width_scale;
  Rectangle health_hitbox = {health_x, -44 * width_scale, 31 * width_scale, 44 * width_scale};
  int health_is_dropped = 0;
  // For Speeds
  int enemy_speed = 3 * width_scale;
  int enemy_bullet_speed = 5 * width_scale;
  int health_speed = 2 * width_scale;

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    SetTargetFPS(60);
    if (IsKeyDown(KEY_ESCAPE) && !player.is_destroyed) break;

    if ((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) && !player.is_destroyed) {
      move_player_right(&player.x, &player.bar_x, player.speed, &player.hitbox, &player.hitbox2);
    }
    if ((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) && !player.is_destroyed) {
      move_player_left(&player.x, &player.bar_x, player.speed, &player.hitbox, &player.hitbox2);
    }

    // create new player bullet to shoot
    if (IsKeyDown(KEY_SPACE) && !player.is_destroyed) {
      if (GetTime() - lastShotTime >= 0.3) {
        PlaySound(player_shoot_sound);
        add_player_bullet(player_bullet_list, player.x, player.y);
        lastShotTime = GetTime();
      }
    }

    if (GetTime() - lastSpawnEnemy >= 1.2 && !player.is_destroyed && !boss_chapter) {
      add_enemy(enemy_list, rand() % (int)(533 * width_scale - 8 * width_scale + 1) + 8 * width_scale, -28  * width_scale, GetTime());

      lastSpawnEnemy = GetTime();
    }

    if(!player.is_destroyed) clean_player_bullet_list(player_bullet_list, enemy_list, &boss, &score, &hit_sound, &enemy_explosion_sound, &boss_chapter, &start_boss_chapter_time, &boss_start_fight);

    // -------------- For Drawing --------------

    BeginDrawing();

    DrawTexture(background, 0, 0, WHITE);

    // move player bullets
    PlayerBullet *current = player_bullet_list->head;
    while (current != NULL) {
      DrawTexture(player_bullet_texture, current->x , current->y, WHITE);
      // DrawRectangleLines(current->hitbox.x, current->hitbox.y, current->hitbox.width, current->hitbox.height, WHITE);
      if(!player.is_destroyed){
        current->y -= player.bullet_speed;
        current->hitbox.y -= player.bullet_speed;
      }
      current = current->next;
    }

    // Draw Player with his health bar
    if(player.is_destroyed && !result_screen){
      Texture2D current_explosion = explosion[player.explosion_counter/6];
      DrawTexturePro(
        current_explosion,
        (Rectangle){0, 0, current_explosion.width, current_explosion.height},
        (Rectangle){player.x + 28 * width_scale, player.y + 28 * width_scale, current_explosion.width, current_explosion.height},
        (Vector2){(float)current_explosion.width/2, (float)current_explosion.height/2},
        0,
        WHITE
      );
      player.explosion_counter++;

      if(player.explosion_counter == 35){
        if(best_score < score) best_score = score;
        result_screen = 1;
      }
    }
    else{
      if(result_screen) DrawTexturePro(
        explosion[5],
        (Rectangle){0, 0, explosion[5].width, explosion[5].height},
        (Rectangle){player.x + 28 * width_scale, player.y + 28 * width_scale, explosion[5].width, explosion[5].height},
        (Vector2){(float)explosion[5].width/2, (float)explosion[5].height/2},
        0,
        WHITE
      );
      else if(GetTime() - player.last_dammaged < 0.4) DrawTexture(player_texture, player.x, player.y, RED);
      else if(GetTime() - player.last_healthed_time < 0.3) DrawTexture(player_texture, player.x, player.y, GREEN);
      else DrawTexture(player_texture, player.x, player.y, WHITE);
    }

    // DrawRectangleLines(player.hitbox.x, player.hitbox.y, player.hitbox.width, player.hitbox.height, GREEN);
    // DrawRectangleLines(player.hitbox2.x, player.hitbox2.y, player.hitbox2.width, player.hitbox2.height, GREEN);
    if(player.health == 100) DrawTexture(player_full_health_texture, player.bar_x, player.bar_y, WHITE);
    if(player.health == 75) DrawTexture(player_third_health_texture, player.bar_x, player.bar_y, WHITE);
    if(player.health == 50) DrawTexture(player_second_health_texture, player.bar_x, player.bar_y, WHITE);
    if(player.health == 25) DrawTexture(player_first_health_texture, player.bar_x, player.bar_y, WHITE);
    if(player.health == 0) DrawTexture(player_zero_health_texture, player.bar_x, player.bar_y, WHITE);

    // Draw Boss
    if(boss.is_destroyed){
      Texture2D current_explosion = explosion[boss.explosion_counter/10];
      DrawTexturePro(
        current_explosion,
        (Rectangle){0, 0, current_explosion.width * 2, current_explosion.height * 2},
        (Rectangle){boss.x + (float)boss_texture.width / 2, boss.y + (float)boss_texture.height / 2, current_explosion.width * 2, current_explosion.height * 2},
        (Vector2){(float)current_explosion.width, (float)current_explosion.height},
        0,
        WHITE
      );
      boss.explosion_counter++;

      if(boss.explosion_counter == 60){
        PlaySound(player_explosion_sound);
        boss_chapter = 0;
        show_boss = 0;
        lastSpawnEnemy = GetTime();
        float new_x = (rand() % (471 - 0 + 1) + 0) * width_scale;
        float new_y = -113 * width_scale;
        boss = (Boss){
          .x = new_x,
          .y = new_y,
          .health = 150,
          .speed = 3 * width_scale,
          .bullet_speed = 5 * width_scale,
          .rocket_speed = 8 * width_scale,
          .bar_x = new_x - 23.5 * width_scale,
          .bar_y = new_y - 19 * width_scale,
          .explosion_counter = 0,
          .is_destroyed = 0,
          .lastDamagedTime = -1,
          .last_bullets_shoot_time = 5,
          .last_rocket_shoot_time = 5,
          .hitbox = {new_x, new_y, boss_texture.width, boss_texture.height},
          .bullets_list = boss.bullets_list,
          .rocket_list = boss.rocket_list,
        };
      }
    }

    else if(boss_chapter){
      if(boss_start_fight){
        if(GetTime() - boss.lastDamagedTime <= 0.4) DrawTexture(boss_texture, boss.x, boss.y, RED);
        else DrawTexture(boss_texture, boss.x, boss.y, WHITE);
        // DrawRectangleLines(boss.hitbox.x, boss.hitbox.y, boss_texture.width, boss_texture.height, RED);
        if(boss.health == 150) DrawTexture(boss_full_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 125) DrawTexture(boss_fifth_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 100) DrawTexture(boss_fourth_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 75) DrawTexture(boss_third_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 50) DrawTexture(boss_second_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 25) DrawTexture(boss_first_health_texture, boss.bar_x, boss.bar_y, WHITE);
        if(!player.is_destroyed) move_boss(&boss);
        // For Boss Bullets Time
        if(GetTime() - boss.last_bullets_shoot_time >= 0.8 && !player.is_destroyed){
          PlaySound(enemy_shoot_sound);
          add_boss_bullets(boss.bullets_list, boss.x, boss.y);
          boss.last_bullets_shoot_time = GetTime();
        }

        // For Boss Rocket Time
        if(GetTime() - boss.last_rocket_shoot_time >= 1.3 && !player.is_destroyed){
          PlaySound(enemy_shoot_sound);
          add_boss_rocket(boss.rocket_list, boss.x, boss.y);
          boss.last_rocket_shoot_time = GetTime();
        }

      }
      else if(show_boss){
        DrawTexture(boss_texture, boss.x, boss.y, WHITE);
        // DrawRectangleLines(boss.hitbox.x, boss.hitbox.y, boss_texture.width, boss_texture.height, RED);
        if(boss.health == 150) DrawTexture(boss_full_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 125) DrawTexture(boss_fifth_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 100) DrawTexture(boss_fourth_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 75) DrawTexture(boss_third_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 50) DrawTexture(boss_second_health_texture, boss.bar_x, boss.bar_y, WHITE);
        else if(boss.health == 25) DrawTexture(boss_first_health_texture, boss.bar_x, boss.bar_y, WHITE);
        if(!player.is_destroyed){
          boss.y += boss.speed;
          boss.hitbox.y += boss.speed;
          boss.bar_y += boss.speed;
          if(boss.y >= 40 * width_scale){
            boss_start_fight = 1;
            boss.last_bullets_shoot_time = GetTime();
            boss.last_rocket_shoot_time = GetTime();
          }
        }
      }

      else if(GetTime() - start_boss_chapter_time >= 1){
        show_boss = 1;
      }
    }

    // Draw Boss Bullets
    BossBullet *current_boss_bullet = boss.bullets_list->head;
    while(current_boss_bullet != NULL){
      BossBullet *next_bullet = current_boss_bullet->next;
      if(current_boss_bullet->y >= 600 *width_scale) delete_boss_bullet(boss.bullets_list, current_boss_bullet);
      else if(CheckCollisionRecs(player.hitbox, current_boss_bullet->hitbox) || CheckCollisionRecs(player.hitbox2, current_boss_bullet->hitbox)){
        PlaySound(hit_sound);
        delete_boss_bullet(boss.bullets_list, current_boss_bullet);
        player.health -= 25;
        if(player.health == 0){
          PlaySound(player_explosion_sound);
          player.is_destroyed = 1;
        }
        player.last_dammaged = GetTime();
      }
      else{
        DrawTexture(boss_bullet_texture, current_boss_bullet->x, current_boss_bullet->y, WHITE);
        // DrawRectangleLines(current_boss_bullet->x, current_boss_bullet->y, boss_bullet_texture.width, boss_bullet_texture.height, WHITE);
        if(!player.is_destroyed){
          current_boss_bullet->y += boss.bullet_speed;
          current_boss_bullet->hitbox.y += boss.bullet_speed;
        }
      }
      current_boss_bullet = next_bullet;
    }

    // Draw Boss Rockets
    BossRocket *current_boss_rocket = boss.rocket_list->head;
    while(current_boss_rocket != NULL){
      BossRocket *next_rocket = current_boss_rocket->next;
      if(current_boss_rocket->y >= 600 *width_scale) delete_boss_rocket(boss.rocket_list, current_boss_rocket);
      else if(CheckCollisionRecs(player.hitbox, current_boss_rocket->hitbox) || CheckCollisionRecs(player.hitbox2, current_boss_rocket->hitbox)){
        PlaySound(hit_sound);
        delete_boss_rocket(boss.rocket_list, current_boss_rocket);
        player.health -= 50;
        if(player.health <= 0){
          PlaySound(player_explosion_sound);
          player.health = 0;
          player.is_destroyed = 1;
        }
        player.last_dammaged = GetTime();
      }
      else{
        DrawTexture(boss_rocket_texture, current_boss_rocket->x, current_boss_rocket->y, WHITE);
        // DrawRectangleLines(current_boss_rocket->x, current_boss_rocket->y, boss_rocket_texture.width, boss_rocket_texture.height, WHITE);
        if(!player.is_destroyed){
          current_boss_rocket->y += boss.rocket_speed;
          current_boss_rocket->hitbox.y += boss.rocket_speed;
        }
      }
      current_boss_rocket = next_rocket;
    }

    // Draw Enemies with their health bars
    Enemy *current_enemy = enemy_list->head;
    while (current_enemy != NULL) {
        Enemy *next_enemy = current_enemy->next;

        if(current_enemy->is_destroyed){
          Texture2D current_explosion = explosion[(int)current_enemy->counter_explosion/2];
          DrawTexturePro(
              current_explosion,
              (Rectangle){0, 0, current_explosion.width, current_explosion.height},
              (Rectangle){current_enemy->x + 27 * width_scale, current_enemy->y + 23.5 * width_scale, current_explosion.width, current_explosion.height},
              (Vector2){(float)current_explosion.width/2, (float)current_explosion.height/2},
              0,
              WHITE
          );
          current_enemy->counter_explosion++;
          if(current_enemy->counter_explosion == 11){
            delete_enemy(enemy_list, current_enemy, 'N');
          }
          current_enemy = next_enemy;
          continue;
        }


        else if(current_enemy->y >= 600 * width_scale){
          delete_enemy(enemy_list, current_enemy, 'N');
          current_enemy = next_enemy;
          continue;
        }
        if(!player.is_destroyed){
          current_enemy->y += enemy_speed;
          current_enemy->bar_y += enemy_speed;
          current_enemy->hitbox.y += enemy_speed;
        }
        if(GetTime() - current_enemy->lastShotTime >= 1.1 && !player.is_destroyed){
            PlaySound(enemy_shoot_sound);
            add_enemy_bullet(enemy_bullet_list, current_enemy->x, current_enemy->y);
            current_enemy->lastShotTime = GetTime();
        }

        // Draw Enemy
        if(GetTime() - current_enemy->lastDamagedTime < 0.2){
          DrawTexture(enemy_texture, current_enemy->x, current_enemy->y, RED);
        }
        else DrawTexture(enemy_texture, current_enemy->x, current_enemy->y, WHITE);

        if(current_enemy->health == 100){
            DrawTexture(enemy_full_health_texture, current_enemy->bar_x, current_enemy->bar_y, WHITE);
        }
        else if(current_enemy->health == 50){
            DrawTexture(enemy_half_health_texture, current_enemy->bar_x, current_enemy->bar_y, WHITE);
        }

        else if(current_enemy->health == 0){
            DrawTexture(enemy_zero_health_texture, current_enemy->bar_x, current_enemy->bar_y, WHITE);
        }

        // DrawRectangleLines(current_enemy->hitbox.x, current_enemy->hitbox.y, current_enemy->hitbox.width, current_enemy->hitbox.height, RED);
        current_enemy = current_enemy->next;
    }

    // Draw Enemies Bullets
    EnemyBullet *current_bullet = enemy_bullet_list->head;
    while(current_bullet != NULL){
      EnemyBullet *next_bullet = current_bullet->next;
      if(current_bullet->y >= 600 * width_scale){
        delete_enemy_bullet(enemy_bullet_list, current_bullet);
        current_bullet = next_bullet;
        continue;
      }

      if(CheckCollisionRecs(current_bullet->hitbox, player.hitbox) || CheckCollisionRecs(current_bullet->hitbox, player.hitbox2)){
        player.health -= 25;
        player.last_dammaged = GetTime();
        delete_enemy_bullet(enemy_bullet_list, current_bullet);
        if(player.health == 0){
          player.is_destroyed = 1;
          PlaySound(player_explosion_sound);
        }
        else PlaySound(hit_sound);

        current_bullet = next_bullet;
        continue;
      }

      DrawTexture(enemy_bullet_texture, current_bullet->x, current_bullet->y, WHITE);
      // DrawRectangleLines(current_bullet->hitbox.x, current_bullet->hitbox.y, current_bullet->hitbox.width, current_bullet->hitbox.height, RED);
      if(!player.is_destroyed){
        current_bullet->y += enemy_bullet_speed;
        current_bullet->hitbox.y += enemy_bullet_speed;
      }
      
      current_bullet = current_bullet->next;
    }

    // For Health
    if(GetTime() - LastHealthDropedTime >= 30){
      health_is_dropped = 1;
    }
    if(health_is_dropped) {
      DrawTexture(health_texture, health_x, health_y, WHITE);
      // DrawRectangleLines(health_hitbox.x, health_hitbox.y, health_hitbox.width, health_hitbox.height, GREEN);
      if(!player.is_destroyed){
        health_y += health_speed;
        health_hitbox.y += health_speed;
      }
      if(health_y >= 600 * width_scale) {
        health_is_dropped = 0;
        health_x = (rand() % (569 - 0 + 1) + 0) * width_scale;
        health_y = -44 * width_scale;
        health_hitbox = (Rectangle){health_x, -44 * width_scale, 31 * width_scale, 44 * width_scale};
        health_is_dropped = 0;
        LastHealthDropedTime = GetTime();
      }
      else if(CheckCollisionRecs(health_hitbox, player.hitbox) || CheckCollisionRecs(health_hitbox, player.hitbox2)) {
        PlaySound(player_healthed_sound);
        player.last_healthed_time = GetTime();
        player.health += 50;
        if(player.health > 100) player.health = 100;
        health_is_dropped = 0;
        health_x = (rand() % (569 - 0 + 1) + 0) * width_scale;
        health_y = -44 * width_scale;
        health_hitbox = (Rectangle){health_x, -44 * width_scale, 31 * width_scale, 44 * width_scale};
        health_is_dropped = 0;
        LastHealthDropedTime = GetTime();
      }
    }

    DrawFPS(10 * width_scale, 10 * width_scale);

    DrawText(TextFormat("SCORE : %d", score), 10 * width_scale, 40 * width_scale, 19 * width_scale, WHITE);

    if(result_screen){
      if(IsKeyDown(KEY_ENTER)){
        // For Reset Player
        float new_x = ((float)width / 2 - 28) * width_scale;
        float new_y = 503 * width_scale;
        player = (Player){
          .x = new_x,
          .y = new_y,
          .health = 100,
          .last_dammaged = -1,
          .last_healthed_time = -1,
          .is_destroyed = 0,
          .explosion_counter = 0,
          .speed = 5 * width_scale,
          .bullet_speed = 10 * width_scale,
          .hitbox = {new_x, new_y + 20 * width_scale, 56 * width_scale, 20 * width_scale},
          .hitbox2 = {new_x + 18 * width_scale, new_y, 20 * width_scale, 56 * width_scale},
          .bar_x = new_x - 35 * width_scale,
          .bar_y = (560 + (float)player_full_health_texture.height / 2) * width_scale
        };
        // For Reset Player Bullets
        PlayerBullet *current_player_bullet = player_bullet_list->head;
        while(current_player_bullet != NULL){
          PlayerBullet *next_bullet = current_player_bullet->next;
          delete_player_bullet(player_bullet_list, current_player_bullet);
          current_player_bullet = next_bullet;
        }
        // For Reset Enemy List 
        Enemy *current_enemy = enemy_list->head;
        while(current_enemy != NULL){
          Enemy *next_enemy = current_enemy->next;
          delete_enemy(enemy_list, current_enemy, 'N');
          current_enemy = next_enemy;
        }
        // For Reset Enemy Bullet List
        EnemyBullet *current_enemy_bullet = enemy_bullet_list->head;
        while(current_enemy_bullet != NULL){
          EnemyBullet *next_enemy_bullet = current_enemy_bullet->next;
          delete_enemy_bullet(enemy_bullet_list, current_enemy_bullet);
          current_enemy_bullet = next_enemy_bullet;
        }
        // For Reset Boss Bullets List
        BossBullet *current_boss_bullet = boss.bullets_list->head;
        while(current_boss_bullet != NULL){
          BossBullet *next_boss_bullet = current_boss_bullet->next;
          delete_boss_bullet(boss.bullets_list, current_boss_bullet);
          current_boss_bullet = next_boss_bullet;
        }
        // For Reset Boss Rockets List
        BossRocket *current_boss_rocket = boss.rocket_list->head;
        while(current_boss_rocket != NULL){
          BossRocket *next_boss_rocket = current_boss_rocket->next;
          delete_boss_rocket(boss.rocket_list, current_boss_rocket);
          current_boss_rocket = next_boss_rocket;
        }
        // For Reset Boss
        new_x = (rand() % (471 - 0 + 1) + 0) * width_scale;
        new_y = -113 * width_scale;
        boss = (Boss){
          .x = new_x,
          .y = new_y,
          .health = 150,
          .speed = 3 * width_scale,
          .bullet_speed = 5 * width_scale,
          .rocket_speed = 8 * width_scale,
          .bar_x = new_x - 23.5 * width_scale,
          .bar_y = new_y - 19 * width_scale,
          .explosion_counter = 0,
          .is_destroyed = 0,
          .lastDamagedTime = -1,
          .last_bullets_shoot_time = 5,
          .last_rocket_shoot_time = 5,
          .hitbox = {new_x, new_y, boss_texture.width, boss_texture.height},
          .bullets_list = boss.bullets_list,
          .rocket_list = boss.rocket_list,
        };
        // For Reset Game
        boss_chapter = 0;
        boss_start_fight = 0;
        result_screen = 0;
        score = 0;
        lastSpawnEnemy = GetTime();
        LastHealthDropedTime = -100;
        health_x = (rand() % (569 - 0 + 1) + 0) * width_scale;
        health_y = -44 * width_scale;
        health_hitbox = (Rectangle){health_x, health_y, 31 * width_scale, 44 * width_scale};
        opacity_press_enter = 255;
        speed_change_opacity = -3;
        continue;
      }

      DrawRectangle(0, 0, 600 * width_scale, 600 * width_scale, (Color){0, 0, 0, 180});

      DrawText(
          "GAME OVER",
          (600 * width_scale - MeasureText("GAME OVER", 50 * width_scale)) / 2,
          (600 * width_scale - 50 * width_scale) / 2 - 100 * width_scale,
          50 * width_scale,
          (Color){255, 0, 0, 255}
      );

      DrawText(
          TextFormat("SCORE : %d", score),
          (600 * width_scale - MeasureText(TextFormat("SCORE : %d", score), 50 * width_scale)) / 2,
          (600 * width_scale - 50 * width_scale) / 2,
          50 * width_scale,
          WHITE
      );

      DrawText(
          TextFormat("BEST SCORE : %d", best_score),
          (600 * width_scale - MeasureText(TextFormat("BEST SCORE : %d", best_score), 50 * width_scale)) / 2,
          (600 * width_scale - 50 * width_scale) / 2 + 100 * width_scale,
          50 * width_scale,
          WHITE
      );

      DrawText(
        "PRESS ENTER TO REPLAY...",
        (600 * width_scale - MeasureText("PRESS ENTER TO REPLAY...", 20 * width_scale)) / 2,
        (600 * width_scale - 20 * width_scale) / 2 + 200 * width_scale,
        20 * width_scale,
        (Color){255, 255, 255, opacity_press_enter}
      );
      opacity_press_enter += speed_change_opacity;
      if(opacity_press_enter < 0){
        opacity_press_enter = 0;
        speed_change_opacity *= -1;
      }
      else if(opacity_press_enter > 100){
        opacity_press_enter = 100;
        speed_change_opacity *= -1;
      }
    }

    EndDrawing();
  }

  CloseAudioDevice();
  CloseWindow();

  return 0;
}