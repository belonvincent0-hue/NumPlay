/* NumDash: the cheat menu (EXP). The menu is common/np_cheat.h's; this file is what it changes. */
#include "cheats.h"
#include "app.h"

int ch_noclip, ch_force, ch_hold;

#if PLATFORM_DEVICE
#include "../../common/np_cheat.h"

static int speed_i = 1, mode_i, started;
static const char *const SPEED_NAMES[4] = {"X0.5", "X1", "X1.5", "X2"};
static const uint16_t SPEED_PCT[4] = {50, 100, 150, 200};
static const char *const MODE_NAMES[8] = {"CUBE", "SHIP", "BALL", "UFO", "WAVE", "ROBOT", "SPIDER", "SWING"};

static bool in_level(void) { return app.screen == SCR_PLAY && app.has_level && !app.g.dead && !app.g.complete; }

static const char *c_mode(void) {
  if (!in_level()) return "PAS DANS UN NIVEAU";
  app.g.p.mode = (uint8_t)mode_i;
  return "MODE CHANGE";
}
static const char *c_mini(void) {
  if (!in_level()) return "PAS DANS UN NIVEAU";
  app.g.p.mini = !app.g.p.mini;
  return app.g.p.mini ? "MINI ACTIVE" : "MINI DESACTIVE";
}
static const char *c_ahead(void) {
  if (!in_level()) return "PAS DANS UN NIVEAU";
  app.g.p.x += 600.0f;
  ch_force = 240; /* a second of noclip to land safely */
  return "AVANCE DE 600";
}
static const char *c_finish(void) {
  if (!in_level()) return "PAS DANS UN NIVEAU";
  float x = (float)app.g.L->wall_x - 90.0f;
  if (app.g.p.x < x) app.g.p.x = x;
  ch_force = 960;
  return "FIN DU NIVEAU";
}
static const char *c_full(void) {
  for (int i = 0; i < LEVEL_SLOTS; i++) {
    progress.lv[i].normal = 100, progress.lv[i].practice = 100, progress.lv[i].coins = 7;
  }
  app_save_progress();
  return "TOUT A 100%";
}

static np_cheat_t CHEATS[] = {
    {"NOCLIP", NPC_TOGGLE, &ch_noclip, 0, 0, 0},
    {"TOUJOURS APPUYE", NPC_TOGGLE, &ch_hold, 0, 0, 0},
    {"VITESSE DU JEU", NPC_CYCLE, &speed_i, 4, SPEED_NAMES, 0},
    {"MODE DU JOUEUR", NPC_CYCLE, &mode_i, 8, MODE_NAMES, c_mode},
    {"MINI ON/OFF", NPC_ACTION, 0, 0, 0, c_mini},
    {"AVANCER", NPC_ACTION, 0, 0, 0, c_ahead},
    {"FINIR LE NIVEAU", NPC_ACTION, 0, 0, 0, c_finish},
    {"TOUT A 100%", NPC_ACTION, 0, 0, 0, c_full},
};

bool cheats_menu(void) {
  started = 1;
  return np_cheat_menu("CHEATS NUMDASH", CHEATS, (int)(sizeof CHEATS / sizeof CHEATS[0])) != 0;
}
uint32_t cheats_speed_pct(void) { return started ? SPEED_PCT[speed_i] : 100; }
#else
bool cheats_menu(void) { return false; }
uint32_t cheats_speed_pct(void) { return 100; }
#endif

void cheats_tick(void) {
  if (ch_force > 0) ch_force--;
}
