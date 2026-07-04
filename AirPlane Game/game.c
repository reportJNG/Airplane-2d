#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define GAME_SIZE 600
#define MAX_PLAYER_BULLETS 96
#define MAX_ENEMIES 32
#define MAX_ENEMY_BULLETS 96
#define MAX_BOSS_BULLETS 48
#define MAX_BOSS_ROCKETS 24

int width = GAME_SIZE;
int height = GAME_SIZE;
float width_scale = 1.0f;

typedef struct MovingSprite {
  bool active;
  float x;
  float y;
  float speed;
  Rectangle hitbox;
} MovingSprite;

typedef struct EnemyState {
  bool active;
  bool exploding;
  float x;
  float y;
  int health;
  int explosion_tick;
  double last_shot_time;
  double last_damaged_time;
  Rectangle hitbox;
} EnemyState;

typedef struct BossState {
  bool active;
  bool entering;
  bool exploding;
  float x;
  float y;
  float speed;
  int health;
  int explosion_tick;
  double last_shot_time;
  double last_rocket_time;
  double last_damaged_time;
  Rectangle hitbox;
} BossState;

typedef struct PlayerState {
  float x;
  float y;
  int health;
  bool destroyed;
  int explosion_tick;
  double last_shot_time;
  double last_damaged_time;
  double last_healed_time;
  Rectangle body_hitbox;
  Rectangle wing_hitbox;
} PlayerState;

static Texture2D background;
static Texture2D loading_background;
static Texture2D player_texture;
static Texture2D player_bullet_texture;
static Texture2D player_health_textures[5];
static Texture2D health_texture;
static Texture2D enemy_texture;
static Texture2D enemy_bullet_texture;
static Texture2D enemy_health_textures[3];
static Texture2D boss_texture;
static Texture2D boss_bullet_texture;
static Texture2D boss_rocket_texture;
static Texture2D boss_health_textures[7];
static Texture2D explosion_textures[6];

#if defined(PLATFORM_WEB)
typedef enum GameSound {
  GAME_SOUND_PLAYER_SHOOT = 0,
  GAME_SOUND_PLAYER_HEALED,
  GAME_SOUND_ENEMY_SHOOT,
  GAME_SOUND_HIT,
  GAME_SOUND_PLAYER_EXPLOSION,
  GAME_SOUND_ENEMY_EXPLOSION
} GameSound;

static const GameSound player_shoot_sound = GAME_SOUND_PLAYER_SHOOT;
static const GameSound player_healed_sound = GAME_SOUND_PLAYER_HEALED;
static const GameSound enemy_shoot_sound = GAME_SOUND_ENEMY_SHOOT;
static const GameSound hit_sound = GAME_SOUND_HIT;
static const GameSound player_explosion_sound = GAME_SOUND_PLAYER_EXPLOSION;
static const GameSound enemy_explosion_sound = GAME_SOUND_ENEMY_EXPLOSION;

EM_JS(void, PlayBrowserSound, (int sound_id), {
  if (typeof window !== "undefined" && window.airplanePlaySound) {
    window.airplanePlaySound(sound_id);
  }
});

static void PlayGameSound(GameSound sound) {
  PlayBrowserSound((int)sound);
}
#else
static Sound player_shoot_sound;
static Sound player_healed_sound;
static Sound enemy_shoot_sound;
static Sound hit_sound;
static Sound player_explosion_sound;
static Sound enemy_explosion_sound;

static void PlayGameSound(Sound sound) { PlaySound(sound); }
#endif

static PlayerState player;
static EnemyState enemies[MAX_ENEMIES];
static MovingSprite player_bullets[MAX_PLAYER_BULLETS];
static MovingSprite enemy_bullets[MAX_ENEMY_BULLETS];
static MovingSprite boss_bullets[MAX_BOSS_BULLETS];
static MovingSprite boss_rockets[MAX_BOSS_ROCKETS];
static MovingSprite health_pickup;
static BossState boss;

static int score = 0;
static int best_score = 0;
static bool game_over = false;
static bool paused = false;
static bool virtual_left = false;
static bool virtual_right = false;
static bool virtual_shoot = false;
static bool virtual_replay = false;
static bool virtual_escape = false;
static bool previous_replay = false;
static bool previous_escape = false;
static double last_enemy_spawn_time = -10.0;
static double last_health_spawn_time = -31.0;
static double boss_chapter_start_time = 0.0;
static int next_boss_score = 50;

static Texture2D LoadGameTexture(const char *path) {
  Texture2D texture = LoadTexture(path);
  SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
  return texture;
}

static Rectangle RectFor(float x, float y, Texture2D texture) {
  return (Rectangle){x, y, (float)texture.width, (float)texture.height};
}

static void UpdatePlayerHitboxes(void) {
  player.body_hitbox = (Rectangle){player.x, player.y + 20.0f, 56.0f, 20.0f};
  player.wing_hitbox = (Rectangle){player.x + 18.0f, player.y, 20.0f, 56.0f};
}

static void ResetMovingSprites(MovingSprite *items, int count) {
  for (int i = 0; i < count; i++) items[i].active = false;
}

static void ResetRound(void) {
  score = 0;
  game_over = false;
  paused = false;
  last_enemy_spawn_time = GetTime();
  last_health_spawn_time = GetTime() - 26.0;
  boss_chapter_start_time = 0.0;
  next_boss_score = 50;

  player = (PlayerState){
      .x = GAME_SIZE / 2.0f - 28.0f,
      .y = 503.0f,
      .health = 100,
      .destroyed = false,
      .explosion_tick = 0,
      .last_shot_time = -1.0,
      .last_damaged_time = -1.0,
      .last_healed_time = -1.0,
  };
  UpdatePlayerHitboxes();

  for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
  ResetMovingSprites(player_bullets, MAX_PLAYER_BULLETS);
  ResetMovingSprites(enemy_bullets, MAX_ENEMY_BULLETS);
  ResetMovingSprites(boss_bullets, MAX_BOSS_BULLETS);
  ResetMovingSprites(boss_rockets, MAX_BOSS_ROCKETS);

  health_pickup.active = false;
  health_pickup.x = (float)GetRandomValue(0, GAME_SIZE - 31);
  health_pickup.y = -44.0f;
  health_pickup.speed = 2.0f;
  health_pickup.hitbox = (Rectangle){health_pickup.x, health_pickup.y, 31.0f, 44.0f};

  boss = (BossState){
      .active = false,
      .entering = false,
      .exploding = false,
      .x = (float)GetRandomValue(0, 471),
      .y = -113.0f,
      .speed = 3.0f,
      .health = 150,
      .explosion_tick = 0,
      .last_shot_time = 0.0,
      .last_rocket_time = 0.0,
      .last_damaged_time = -1.0,
  };
  boss.hitbox = RectFor(boss.x, boss.y, boss_texture);
}

static MovingSprite *FindInactiveSprite(MovingSprite *items, int count) {
  for (int i = 0; i < count; i++) {
    if (!items[i].active) return &items[i];
  }
  return NULL;
}

static EnemyState *FindInactiveEnemy(void) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) return &enemies[i];
  }
  return NULL;
}

static void SpawnPlayerBullet(void) {
  MovingSprite *bullet = FindInactiveSprite(player_bullets, MAX_PLAYER_BULLETS);
  if (bullet == NULL) return;
  bullet->active = true;
  bullet->x = player.x + 24.0f;
  bullet->y = player.y - 10.0f;
  bullet->speed = 10.0f;
  bullet->hitbox = (Rectangle){bullet->x, bullet->y, 9.0f, 22.0f};
  PlayGameSound(player_shoot_sound);
}

static void SpawnEnemy(void) {
  EnemyState *enemy = FindInactiveEnemy();
  if (enemy == NULL) return;
  enemy->active = true;
  enemy->exploding = false;
  enemy->x = (float)GetRandomValue(8, 533);
  enemy->y = -28.0f;
  enemy->health = 100;
  enemy->explosion_tick = 0;
  enemy->last_shot_time = GetTime();
  enemy->last_damaged_time = -1.0;
  enemy->hitbox = (Rectangle){enemy->x, enemy->y, 54.0f, 47.0f};
}

static void SpawnEnemyBullet(float x, float y) {
  MovingSprite *bullet = FindInactiveSprite(enemy_bullets, MAX_ENEMY_BULLETS);
  if (bullet == NULL) return;
  bullet->active = true;
  bullet->x = x + 17.0f;
  bullet->y = y + 47.0f;
  bullet->speed = 5.0f;
  bullet->hitbox = (Rectangle){bullet->x, bullet->y, 20.0f, 26.0f};
  PlayGameSound(enemy_shoot_sound);
}

static void SpawnBoss(void) {
  boss.active = true;
  boss.entering = true;
  boss.exploding = false;
  boss.x = (float)GetRandomValue(0, 471);
  boss.y = -113.0f;
  boss.speed = 3.0f;
  boss.health = 150;
  boss.explosion_tick = 0;
  boss.last_shot_time = GetTime();
  boss.last_rocket_time = GetTime();
  boss.last_damaged_time = -1.0;
  boss.hitbox = RectFor(boss.x, boss.y, boss_texture);
}

static void SpawnBossBullets(void) {
  MovingSprite *left = FindInactiveSprite(boss_bullets, MAX_BOSS_BULLETS);
  MovingSprite *right = FindInactiveSprite(boss_bullets, MAX_BOSS_BULLETS);
  if (left == NULL || right == NULL || left == right) return;

  left->active = true;
  left->x = boss.x + 24.5f;
  left->y = boss.y + 113.0f;
  left->speed = 5.0f;
  left->hitbox = (Rectangle){left->x, left->y, 10.0f, 29.0f};

  right->active = true;
  right->x = boss.x + 94.5f;
  right->y = boss.y + 113.0f;
  right->speed = 5.0f;
  right->hitbox = (Rectangle){right->x, right->y, 10.0f, 29.0f};

  PlayGameSound(enemy_shoot_sound);
}

static void SpawnBossRocket(void) {
  MovingSprite *rocket = FindInactiveSprite(boss_rockets, MAX_BOSS_ROCKETS);
  if (rocket == NULL) return;
  rocket->active = true;
  rocket->x = boss.x + 53.5f;
  rocket->y = boss.y + 113.0f;
  rocket->speed = 8.0f;
  rocket->hitbox = (Rectangle){rocket->x, rocket->y, 22.0f, 40.0f};
  PlayGameSound(enemy_shoot_sound);
}

static void DamagePlayer(int amount) {
  if (player.destroyed || game_over) return;
  player.health -= amount;
  player.last_damaged_time = GetTime();
  if (player.health <= 0) {
    player.health = 0;
    player.destroyed = true;
    player.explosion_tick = 0;
    PlayGameSound(player_explosion_sound);
  } else {
    PlayGameSound(hit_sound);
  }
}

static void StartGameOver(void) {
  if (best_score < score) best_score = score;
  game_over = true;
}

static void CheckBossUnlock(void) {
  if (!boss.active && score >= next_boss_score) {
    boss_chapter_start_time = GetTime();
    next_boss_score += 50;
    SpawnBoss();
  }
}

static void UpdatePlayer(void) {
  if (player.destroyed) return;

  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) || virtual_right) player.x += 5.0f;
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) || virtual_left) player.x -= 5.0f;

  if (player.x < 0.0f) player.x = 0.0f;
  if (player.x > GAME_SIZE - 56.0f) player.x = GAME_SIZE - 56.0f;
  UpdatePlayerHitboxes();

  if ((IsKeyDown(KEY_SPACE) || virtual_shoot) && GetTime() - player.last_shot_time >= 0.3) {
    SpawnPlayerBullet();
    player.last_shot_time = GetTime();
  }
}

static void UpdatePlayerBullets(void) {
  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
    MovingSprite *bullet = &player_bullets[i];
    if (!bullet->active) continue;

    bullet->y -= bullet->speed;
    bullet->hitbox.y = bullet->y;
    if (bullet->y < -30.0f) {
      bullet->active = false;
      continue;
    }

    for (int j = 0; j < MAX_ENEMIES; j++) {
      EnemyState *enemy = &enemies[j];
      if (!enemy->active || enemy->exploding) continue;
      if (CheckCollisionRecs(bullet->hitbox, enemy->hitbox)) {
        bullet->active = false;
        enemy->health -= 50;
        enemy->last_damaged_time = GetTime();
        PlayGameSound(hit_sound);
        if (enemy->health <= 0) {
          enemy->health = 0;
          enemy->exploding = true;
          enemy->explosion_tick = 0;
          score += 5;
          PlayGameSound(enemy_explosion_sound);
          CheckBossUnlock();
        }
        break;
      }
    }

    if (bullet->active && boss.active && !boss.entering && !boss.exploding &&
        CheckCollisionRecs(bullet->hitbox, boss.hitbox)) {
      bullet->active = false;
      boss.health -= 25;
      boss.last_damaged_time = GetTime();
      PlayGameSound(hit_sound);
      if (boss.health <= 0) {
        boss.health = 0;
        boss.exploding = true;
        boss.explosion_tick = 0;
        score += 10;
        PlayGameSound(enemy_explosion_sound);
      }
    }
  }
}

static void UpdateEnemies(void) {
  if (!boss.active && GetTime() - last_enemy_spawn_time >= 1.2) {
    SpawnEnemy();
    last_enemy_spawn_time = GetTime();
  }

  for (int i = 0; i < MAX_ENEMIES; i++) {
    EnemyState *enemy = &enemies[i];
    if (!enemy->active) continue;

    if (enemy->exploding) {
      enemy->explosion_tick++;
      if (enemy->explosion_tick >= 18) enemy->active = false;
      continue;
    }

    enemy->y += 3.0f;
    enemy->hitbox.y = enemy->y;
    if (enemy->y > GAME_SIZE) {
      enemy->active = false;
      continue;
    }

    if (GetTime() - enemy->last_shot_time >= 1.1) {
      SpawnEnemyBullet(enemy->x, enemy->y);
      enemy->last_shot_time = GetTime();
    }
  }
}

static void UpdateEnemyProjectiles(MovingSprite *items, int count, int damage) {
  for (int i = 0; i < count; i++) {
    MovingSprite *item = &items[i];
    if (!item->active) continue;
    item->y += item->speed;
    item->hitbox.y = item->y;

    if (item->y > GAME_SIZE) {
      item->active = false;
      continue;
    }

    if (CheckCollisionRecs(item->hitbox, player.body_hitbox) ||
        CheckCollisionRecs(item->hitbox, player.wing_hitbox)) {
      item->active = false;
      DamagePlayer(damage);
    }
  }
}

static void UpdateHealthPickup(void) {
  if (!health_pickup.active && GetTime() - last_health_spawn_time >= 30.0) {
    health_pickup.active = true;
    health_pickup.x = (float)GetRandomValue(0, GAME_SIZE - 31);
    health_pickup.y = -44.0f;
    health_pickup.hitbox = (Rectangle){health_pickup.x, health_pickup.y, 31.0f, 44.0f};
  }

  if (!health_pickup.active) return;

  health_pickup.y += health_pickup.speed;
  health_pickup.hitbox.y = health_pickup.y;

  if (health_pickup.y > GAME_SIZE) {
    health_pickup.active = false;
    last_health_spawn_time = GetTime();
    return;
  }

  if (CheckCollisionRecs(health_pickup.hitbox, player.body_hitbox) ||
      CheckCollisionRecs(health_pickup.hitbox, player.wing_hitbox)) {
    player.health += 50;
    if (player.health > 100) player.health = 100;
    player.last_healed_time = GetTime();
    health_pickup.active = false;
    last_health_spawn_time = GetTime();
    PlayGameSound(player_healed_sound);
  }
}

static void UpdateBoss(void) {
  if (!boss.active) return;

  if (boss.entering) {
    if (GetTime() - boss_chapter_start_time < 1.0) return;
    boss.y += boss.speed;
    boss.hitbox.y = boss.y;
    if (boss.y >= 40.0f) {
      boss.entering = false;
      boss.last_shot_time = GetTime();
      boss.last_rocket_time = GetTime();
    }
    return;
  }

  if (boss.exploding) {
    boss.explosion_tick++;
    if (boss.explosion_tick >= 60) {
      boss.active = false;
      last_enemy_spawn_time = GetTime();
      ResetMovingSprites(boss_bullets, MAX_BOSS_BULLETS);
      ResetMovingSprites(boss_rockets, MAX_BOSS_ROCKETS);
    }
    return;
  }

  boss.x += boss.speed;
  if (boss.x <= 0.0f || boss.x >= GAME_SIZE - (float)boss_texture.width) {
    boss.speed *= -1.0f;
    if (boss.x < 0.0f) boss.x = 0.0f;
    if (boss.x > GAME_SIZE - (float)boss_texture.width) boss.x = GAME_SIZE - (float)boss_texture.width;
  }
  boss.hitbox.x = boss.x;

  if (GetTime() - boss.last_shot_time >= 0.8) {
    SpawnBossBullets();
    boss.last_shot_time = GetTime();
  }
  if (GetTime() - boss.last_rocket_time >= 1.3) {
    SpawnBossRocket();
    boss.last_rocket_time = GetTime();
  }
}

static void UpdateGame(void) {
  bool replay_pressed = IsKeyPressed(KEY_ENTER) || (virtual_replay && !previous_replay);
  bool escape_pressed = IsKeyPressed(KEY_ESCAPE) || (virtual_escape && !previous_escape);

  previous_replay = virtual_replay;
  previous_escape = virtual_escape;

  if (replay_pressed && game_over) ResetRound();

  if (escape_pressed) {
#if defined(PLATFORM_WEB)
    paused = !paused;
#else
    if (!player.destroyed) CloseWindow();
#endif
  }

  if (game_over || paused) return;

  if (player.destroyed) {
    player.explosion_tick++;
    if (player.explosion_tick >= 35) StartGameOver();
    return;
  }

  UpdatePlayer();
  UpdatePlayerBullets();
  UpdateEnemies();
  UpdateEnemyProjectiles(enemy_bullets, MAX_ENEMY_BULLETS, 25);
  UpdateBoss();
  UpdateEnemyProjectiles(boss_bullets, MAX_BOSS_BULLETS, 25);
  UpdateEnemyProjectiles(boss_rockets, MAX_BOSS_ROCKETS, 50);
  UpdateHealthPickup();
}

static void DrawCenteredText(const char *text, int y, int size, Color color) {
  DrawText(text, (GAME_SIZE - MeasureText(text, size)) / 2, y, size, color);
}

static void DrawHealthBar(Texture2D *textures, int health, int max_health, float x, float y) {
  int max_index = max_health / 25;
  int index = health / 25;
  if (index < 0) index = 0;
  if (index > max_index) index = max_index;
  DrawTexture(textures[index], (int)x, (int)y, WHITE);
}

static void DrawMovingSprite(MovingSprite item, Texture2D texture) {
  if (item.active) DrawTexture(texture, (int)item.x, (int)item.y, WHITE);
}

static void DrawGame(void) {
  BeginDrawing();
  DrawTexture(background, 0, 0, WHITE);

  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) DrawMovingSprite(player_bullets[i], player_bullet_texture);
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) DrawMovingSprite(enemy_bullets[i], enemy_bullet_texture);
  for (int i = 0; i < MAX_BOSS_BULLETS; i++) DrawMovingSprite(boss_bullets[i], boss_bullet_texture);
  for (int i = 0; i < MAX_BOSS_ROCKETS; i++) DrawMovingSprite(boss_rockets[i], boss_rocket_texture);

  if (health_pickup.active) DrawTexture(health_texture, (int)health_pickup.x, (int)health_pickup.y, WHITE);

  for (int i = 0; i < MAX_ENEMIES; i++) {
    EnemyState enemy = enemies[i];
    if (!enemy.active) continue;
    if (enemy.exploding) {
      int frame = enemy.explosion_tick / 3;
      if (frame > 5) frame = 5;
      DrawTexturePro(explosion_textures[frame], (Rectangle){0, 0, (float)explosion_textures[frame].width, (float)explosion_textures[frame].height},
                     (Rectangle){enemy.x + 27.0f, enemy.y + 23.5f, (float)explosion_textures[frame].width, (float)explosion_textures[frame].height},
                     (Vector2){explosion_textures[frame].width / 2.0f, explosion_textures[frame].height / 2.0f}, 0.0f, WHITE);
      continue;
    }

    Color tint = GetTime() - enemy.last_damaged_time < 0.2 ? RED : WHITE;
    DrawTexture(enemy_texture, (int)enemy.x, (int)enemy.y, tint);
    DrawHealthBar(enemy_health_textures, enemy.health / 2, 50, enemy.x - 7.0f, enemy.y - 3.0f);
  }

  if (boss.active) {
    if (boss.exploding) {
      int frame = boss.explosion_tick / 10;
      if (frame > 5) frame = 5;
      DrawTexturePro(explosion_textures[frame], (Rectangle){0, 0, (float)explosion_textures[frame].width, (float)explosion_textures[frame].height},
                     (Rectangle){boss.x + boss_texture.width / 2.0f, boss.y + boss_texture.height / 2.0f,
                                 (float)explosion_textures[frame].width * 2.0f, (float)explosion_textures[frame].height * 2.0f},
                     (Vector2){(float)explosion_textures[frame].width, (float)explosion_textures[frame].height}, 0.0f, WHITE);
    } else {
      Color tint = GetTime() - boss.last_damaged_time < 0.4 ? RED : WHITE;
      DrawTexture(boss_texture, (int)boss.x, (int)boss.y, tint);
      int boss_index = boss.health / 25;
      if (boss_index < 0) boss_index = 0;
      if (boss_index > 6) boss_index = 6;
      DrawTexture(boss_health_textures[boss_index], (int)(boss.x - 23.5f), (int)(boss.y - 19.0f), WHITE);
    }
  }

  if (player.destroyed || game_over) {
    int frame = player.explosion_tick / 6;
    if (frame > 5) frame = 5;
    DrawTexturePro(explosion_textures[frame], (Rectangle){0, 0, (float)explosion_textures[frame].width, (float)explosion_textures[frame].height},
                   (Rectangle){player.x + 28.0f, player.y + 28.0f, (float)explosion_textures[frame].width, (float)explosion_textures[frame].height},
                   (Vector2){explosion_textures[frame].width / 2.0f, explosion_textures[frame].height / 2.0f}, 0.0f, WHITE);
  } else {
    Color tint = WHITE;
    if (GetTime() - player.last_damaged_time < 0.4) tint = RED;
    if (GetTime() - player.last_healed_time < 0.3) tint = GREEN;
    DrawTexture(player_texture, (int)player.x, (int)player.y, tint);
  }
  DrawTexture(player_health_textures[player.health / 25], (int)(player.x - 35.0f), 560, WHITE);

  DrawFPS(10, 10);
  DrawText(TextFormat("SCORE : %d", score), 10, 40, 19, WHITE);

  if (paused) {
    DrawRectangle(0, 0, GAME_SIZE, GAME_SIZE, (Color){0, 0, 0, 170});
    DrawCenteredText("PAUSED", 255, 48, WHITE);
    DrawCenteredText("PRESS ESC TO RESUME", 320, 20, LIGHTGRAY);
  }

  if (game_over) {
    DrawRectangle(0, 0, GAME_SIZE, GAME_SIZE, (Color){0, 0, 0, 190});
    DrawCenteredText("GAME OVER", 170, 50, RED);
    DrawCenteredText(TextFormat("SCORE : %d", score), 260, 42, WHITE);
    DrawCenteredText(TextFormat("BEST SCORE : %d", best_score), 330, 36, WHITE);
    DrawCenteredText("PRESS ENTER TO REPLAY", 455, 20, LIGHTGRAY);
  }

  EndDrawing();
}

static void UpdateDrawFrame(void) {
  UpdateGame();
  DrawGame();
}

#if defined(PLATFORM_WEB)
EMSCRIPTEN_KEEPALIVE
void SetBrowserKey(int key, int is_down) {
  bool down = is_down != 0;

  switch (key) {
    case KEY_LEFT:
    case KEY_A:
      virtual_left = down;
      break;
    case KEY_RIGHT:
    case KEY_D:
      virtual_right = down;
      break;
    case KEY_SPACE:
      virtual_shoot = down;
      break;
    case KEY_ENTER:
      virtual_replay = down;
      break;
    case KEY_ESCAPE:
      virtual_escape = down;
      break;
    default:
      break;
  }
}
#endif

static void LoadAssets(void) {
  loading_background = LoadGameTexture("images/loading-bg.png");
  background = LoadGameTexture("images/bg.png");
  player_texture = LoadGameTexture("images/player/airplane.png");
  player_bullet_texture = LoadGameTexture("images/player/bullet.png");
  player_health_textures[0] = LoadGameTexture("images/player/health/0-4.png");
  player_health_textures[1] = LoadGameTexture("images/player/health/1-4.png");
  player_health_textures[2] = LoadGameTexture("images/player/health/2-4.png");
  player_health_textures[3] = LoadGameTexture("images/player/health/3-4.png");
  player_health_textures[4] = LoadGameTexture("images/player/health/4-4.png");
  health_texture = LoadGameTexture("images/health.png");
  enemy_texture = LoadGameTexture("images/enemy/airplane.png");
  enemy_bullet_texture = LoadGameTexture("images/enemy/bullet.png");
  enemy_health_textures[0] = LoadGameTexture("images/enemy/health/0-2.png");
  enemy_health_textures[1] = LoadGameTexture("images/enemy/health/1-2.png");
  enemy_health_textures[2] = LoadGameTexture("images/enemy/health/2-2.png");
  boss_texture = LoadGameTexture("images/boss/airplane.png");
  boss_bullet_texture = LoadGameTexture("images/boss/bullet.png");
  boss_rocket_texture = LoadGameTexture("images/boss/rocket.png");
  boss_health_textures[0] = LoadGameTexture("images/boss/health/0-6.png");
  boss_health_textures[1] = LoadGameTexture("images/boss/health/1-6.png");
  boss_health_textures[2] = LoadGameTexture("images/boss/health/2-6.png");
  boss_health_textures[3] = LoadGameTexture("images/boss/health/3-6.png");
  boss_health_textures[4] = LoadGameTexture("images/boss/health/4-6.png");
  boss_health_textures[5] = LoadGameTexture("images/boss/health/5-6.png");
  boss_health_textures[6] = LoadGameTexture("images/boss/health/6-6.png");
  explosion_textures[0] = LoadGameTexture("images/explosion/1.png");
  explosion_textures[1] = LoadGameTexture("images/explosion/2.png");
  explosion_textures[2] = LoadGameTexture("images/explosion/3.png");
  explosion_textures[3] = LoadGameTexture("images/explosion/4.png");
  explosion_textures[4] = LoadGameTexture("images/explosion/5.png");
  explosion_textures[5] = LoadGameTexture("images/explosion/6.png");

#if !defined(PLATFORM_WEB)
  player_shoot_sound = LoadSound("sounds/player-shoot.mp3");
  player_healed_sound = LoadSound("sounds/player-healthed.mp3");
  enemy_shoot_sound = LoadSound("sounds/enemy-shoot.mp3");
  hit_sound = LoadSound("sounds/hit.mp3");
  player_explosion_sound = LoadSound("sounds/player-explosion.mp3");
  enemy_explosion_sound = LoadSound("sounds/enemy-explosion.mp3");
#endif
}

#if !defined(PLATFORM_WEB)
static void UnloadAssets(void) {
  UnloadTexture(loading_background);
  UnloadTexture(background);
  UnloadTexture(player_texture);
  UnloadTexture(player_bullet_texture);
  for (int i = 0; i < 5; i++) UnloadTexture(player_health_textures[i]);
  UnloadTexture(health_texture);
  UnloadTexture(enemy_texture);
  UnloadTexture(enemy_bullet_texture);
  for (int i = 0; i < 3; i++) UnloadTexture(enemy_health_textures[i]);
  UnloadTexture(boss_texture);
  UnloadTexture(boss_bullet_texture);
  UnloadTexture(boss_rocket_texture);
  for (int i = 0; i < 7; i++) UnloadTexture(boss_health_textures[i]);
  for (int i = 0; i < 6; i++) UnloadTexture(explosion_textures[i]);

  UnloadSound(player_shoot_sound);
  UnloadSound(player_healed_sound);
  UnloadSound(enemy_shoot_sound);
  UnloadSound(hit_sound);
  UnloadSound(player_explosion_sound);
  UnloadSound(enemy_explosion_sound);
}
#endif

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(GAME_SIZE, GAME_SIZE, "Airplane Game");
  InitAudioDevice();
  SetTargetFPS(60);
  srand((unsigned int)time(NULL));

  Image icon = LoadImage("icon/icon.png");
  SetWindowIcon(icon);
  UnloadImage(icon);

  LoadAssets();
  BeginDrawing();
  DrawTexture(loading_background, 0, 0, WHITE);
  EndDrawing();
  ResetRound();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
  while (!WindowShouldClose()) UpdateDrawFrame();
  UnloadAssets();
  CloseAudioDevice();
  CloseWindow();
#endif

  return 0;
}
