/* Undertale: the cheat menu (EXP). The menu itself is common/np_cheat.h's; this file is what it changes. */
#include "battle.h"
#include "cheats.h"
#include "game.h"
#include "items.h"
#include "world.h"

int ch_hp, ch_god, ch_gold, ch_onehit, ch_crit, ch_noattack, ch_noenc, ch_noclip, ch_walk = 1;

#ifndef HOST
#include "../../common/np_cheat.h"

static int walk_i, speed_i = 1, step_on, item_i;
static const char *const WALK_NAMES[3] = {"X1", "X2", "X3"};
static const char *const SPEED_NAMES[4] = {"X0.5", "X1", "X2", "X3"};
static const uint16_t SPEED_PCT[4] = {50, 100, 200, 300};
static const char *item_names[ITEM_COUNT];

static struct {
  bool ok;
  Save sv;
  int16_t x, y;
  uint8_t dir;
} quick;

static void set_level(int lv) {
  sv.lv = (uint8_t)lv;
  sv.maxhp = (int16_t)lv_maxhp(lv), sv.hp = sv.maxhp;
  sv.at = (uint8_t)lv_at(lv), sv.df = (uint8_t)lv_df(lv);
  if ((int)sv.exp < lv_exp_for(lv)) sv.exp = (uint32_t)lv_exp_for(lv);
}
static const char *c_lv20(void) {
  set_level(20);
  return "NIVEAU 20";
}
static const char *c_lvup(void) {
  if (sv.lv >= 20) return "DEJA AU MAXIMUM";
  set_level(sv.lv + 1);
  return "NIVEAU +1";
}
static const char *c_gold(void) {
  sv.gold = (uint16_t)min_i(sv.gold + 1000, 9999);
  return "+1000 OR";
}
static const char *c_heal(void) {
  sv.hp = sv.maxhp;
  return "SOIGNE";
}
static const char *c_stats(void) {
  sv.at = (uint8_t)min_i(sv.at + 20, 255), sv.df = (uint8_t)min_i(sv.df + 20, 255);
  return "ATK ET DEF +20";
}
static const char *c_gear(void) {
  sv.weapon = ITEM_WORN_DAGGER, sv.armor = ITEM_HEART_LOCKET;
  return "ARME ET ARMURE MAX";
}
static const char *c_item(void) { return item_add(item_i + 1) ? "OBJET DONNE" : "INVENTAIRE PLEIN"; }
static const char *c_food(void) {
  for (int i = 0; i < NUM_ITEMS; i++) sv.items[i] = ITEM_LEGENDARY_HERO;
  return "8 LEGENDARY HERO";
}
static const char *c_win(void) {
  if (g_mode != MODE_BATTLE) return "PAS EN COMBAT";
  cheat_battle_win();
  return "COMBAT GAGNE";
}
static const char *c_save(void) {
  if (g_mode != MODE_WORLD) return "SEULEMENT DANS LE MONDE";
  quick.ok = true, quick.sv = sv, quick.x = pl.x, quick.y = pl.y, quick.dir = pl.dir;
  return "ETAT SAUVEGARDE";
}
static const char *c_load(void) {
  if (!quick.ok) return "RIEN A CHARGER";
  if (g_mode != MODE_WORLD) return "SEULEMENT DANS LE MONDE";
  sv = quick.sv;
  world_enter(sv.room, quick.x, quick.y, quick.dir);
  return "ETAT CHARGE";
}

static np_cheat_t CHEATS[] = {
    {"PV INFINIS", NPC_TOGGLE, &ch_hp, 0, 0, 0},
    {"INVINCIBLE", NPC_TOGGLE, &ch_god, 0, 0, 0},
    {"OR INFINI", NPC_TOGGLE, &ch_gold, 0, 0, 0},
    {"TUER EN UN COUP", NPC_TOGGLE, &ch_onehit, 0, 0, 0},
    {"BARRE PARFAITE", NPC_TOGGLE, &ch_crit, 0, 0, 0},
    {"PAS D'ATTAQUES", NPC_TOGGLE, &ch_noattack, 0, 0, 0},
    {"PAS DE RENCONTRES", NPC_TOGGLE, &ch_noenc, 0, 0, 0},
    {"NOCLIP", NPC_TOGGLE, &ch_noclip, 0, 0, 0},
    {"VITESSE MARCHE", NPC_CYCLE, &walk_i, 3, WALK_NAMES, 0},
    {"NIVEAU 20", NPC_ACTION, 0, 0, 0, c_lv20},
    {"NIVEAU +1", NPC_ACTION, 0, 0, 0, c_lvup},
    {"+1000 OR", NPC_ACTION, 0, 0, 0, c_gold},
    {"SOIGNER", NPC_ACTION, 0, 0, 0, c_heal},
    {"ATK ET DEF +20", NPC_ACTION, 0, 0, 0, c_stats},
    {"ARME ET ARMURE MAX", NPC_ACTION, 0, 0, 0, c_gear},
    {"DONNER OBJET", NPC_CYCLE, &item_i, ITEM_COUNT - 1, item_names, c_item},
    {"OBJETS DE SOIN", NPC_ACTION, 0, 0, 0, c_food},
    {"FINIR LE COMBAT", NPC_ACTION, 0, 0, 0, c_win},
    {"VITESSE DU JEU", NPC_CYCLE, &speed_i, 4, SPEED_NAMES, 0},
    {"IMAGE PAR IMAGE", NPC_TOGGLE, &step_on, 0, 0, 0},
    {"SAUVEGARDE RAPIDE", NPC_ACTION, 0, 0, 0, c_save},
    {"CHARGEMENT RAPIDE", NPC_ACTION, 0, 0, 0, c_load},
};
#define NCHEATS ((int)(sizeof CHEATS / sizeof CHEATS[0]))

bool cheats_menu(void) {
  for (int i = 1; i < ITEM_COUNT; i++) item_names[i - 1] = item_info(i)->short_name;
  bool home = np_cheat_menu("CHEATS UNDERTALE", CHEATS, NCHEATS);
  ch_walk = walk_i + 1;
  return home;
}
/* LN: one frame at a time (in the menu, "IMAGE PAR IMAGE") */
bool cheats_run_frame(void) {
  static bool held;
  if (!step_on) return true;
  bool down = eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_ln);
  bool go = down && !held;
  held = down;
  return go;
}
int cheats_frame_len(void) { return 100 * 100 / SPEED_PCT[speed_i]; }
#else
bool cheats_menu(void) { return false; }
bool cheats_run_frame(void) { return true; }
int cheats_frame_len(void) { return 100; }
#endif

void cheats_tick(void) {
  if (g_mode != MODE_WORLD && g_mode != MODE_BATTLE && g_mode != MODE_MENU) return;
  if (ch_hp && sv.hp > 0) sv.hp = sv.maxhp;
  if (ch_gold) sv.gold = 9999;
}
