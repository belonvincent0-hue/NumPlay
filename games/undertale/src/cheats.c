/* Undertale: the cheat menu (EXP). The menu itself is common/np_cheat.h's; this file is what it changes. */
#include "battle.h"
#include "cheats.h"
#include "game.h"
#include "items.h"
#include "world.h"

int ch_hp, ch_god, ch_gold, ch_onehit, ch_crit, ch_noattack, ch_noenc, ch_noclip, ch_walk = 1;
int ch_spare, ch_chip, ch_slowbul, ch_infitems, ch_gain = 1, ch_healwin, ch_killall;

#ifndef HOST
#include "../../common/np_cheat.h"
#include "flags.h"
#include "menu.h"
#include "save.h"

#define PBX 3 /* Frisk's feet box, as world.c has it */
#define PBY 20
#define PBW 14
#define PBH 9

static int walk_i, gain_i, speed_i = 1, step_on, item_i, tp_i = ROOM_RUINS1, end_i;
static const char *const WALK_NAMES[4] = {"X1", "X2", "X3", "X5"};
static const uint8_t WALK_VALS[4] = {1, 2, 3, 5};
static const char *const GAIN_NAMES[3] = {"X1", "X2", "X5"};
static const uint8_t GAIN_VALS[3] = {1, 2, 5};
static const char *const SPEED_NAMES[4] = {"X0.5", "X1", "X2", "X3"};
static const uint16_t SPEED_PCT[4] = {50, 100, 200, 300};
static const char *const END_NAMES[3] = {"PACIFISTE", "NEUTRE", "GENOCIDE"};
static const char *const END_TEXTS[3] = {
    "THE END|Nobody was hurt. Everyone\nwas spared and went home.|(A pacifist ending.)",
    "THE END|You went home. The monsters\nstayed underground.|(A neutral ending.)",
    "THE END|The Underground fell silent.\nNo one was left to meet.|(A genocide ending.)",
};
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
static const char *c_exp(void) {
  gain_exp(500);
  return "EXP +500";
}
static const char *c_maxhp(void) {
  sv.maxhp = (int16_t)min_i(sv.maxhp + 20, 99), sv.hp = sv.maxhp;
  return "VIE MAX +20 (99 AU PLUS)";
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
static const char *c_killall(void) {
  if (g_mode != MODE_BATTLE) return "PAS EN COMBAT";
  cheat_battle_win();
  return "TOUS LES MONSTRES SONT MORTS";
}
static const char *c_kills0(void) {
  sv.kills = 0, set_flag(F_KILLS_RUINS, 0);
  return "MEURTRES EFFACES";
}
static const char *c_time0(void) {
  sv.time = 0;
  return "CHRONO A 0";
}
static const char *c_savehere(void) {
  if (g_mode != MODE_WORLD) return "SEULEMENT DANS LE MONDE";
  sv.room = (int16_t)g_room, sv.x = pl.x, sv.y = pl.y;
  return save_write() ? "SAUVEGARDE ICI" : "ECHEC DE LA SAUVEGARDE";
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

/* teleport: the walkable spot of the room nearest to its middle (all the cells round it walk too) */
static int walkable(const RoomInfo *r, int cx, int cy) { return r->coll[cy * ((r->cw + 7) >> 3) + (cx >> 3)] >> (cx & 7) & 1; }
static const char *c_tp(void) {
  if (g_mode != MODE_WORLD) return "SEULEMENT DANS LE MONDE";
  RoomInfo ri;
  room_info(tp_i, &ri);
  if (!ri.coll || ri.cw < 3 || ri.ch < 3) return "SALLE INDISPONIBLE";
  int bx = -1, by = -1, bd = 0x7fffffff;
  for (int cy = 1; cy < ri.ch - 1; cy++)
    for (int cx = 1; cx < ri.cw - 1; cx++) {
      int ok = 1;
      for (int dy = -1; dy <= 1 && ok; dy++)
        for (int dx = -1; dx <= 1; dx++)
          if (!walkable(&ri, cx + dx, cy + dy)) {
            ok = 0;
            break;
          }
      if (!ok) continue;
      int d = (cx - ri.cw / 2) * (cx - ri.cw / 2) + (cy - ri.ch / 2) * (cy - ri.ch / 2);
      if (d < bd) bd = d, bx = cx, by = cy;
    }
  if (bx < 0) return "PAS DE SOL ICI";
  world_enter(tp_i, ri.x + bx * 4 - PBX - PBW / 2, ri.y + by * 4 - PBY - PBH / 2, DIR_DOWN);
  return "TELEPORTE";
}
static const char *tp_hint(void) {
  RoomInfo ri;
  room_info(tp_i, &ri);
  return ri.name ? str(ri.name) : "";
}

/* the ending: this port has one end screen (the neutral one, at the end of what there is); the others are these */
static const char *c_end(void) {
  menu_end_open(END_TEXTS[end_i]);
  return "FIN";
}

static np_cheat_t CHEATS[] = {
    /* the player */
    {"PV INFINIS", NPC_TOGGLE, &ch_hp, 0, 0, 0, 0},
    {"INVINCIBLE", NPC_TOGGLE, &ch_god, 0, 0, 0, 0},
    {"DEGATS RECUS = 1", NPC_TOGGLE, &ch_chip, 0, 0, 0, 0},
    {"OR INFINI", NPC_TOGGLE, &ch_gold, 0, 0, 0, 0},
    {"OBJETS INFINIS", NPC_TOGGLE, &ch_infitems, 0, 0, 0, 0},
    {"SOIN APRES COMBAT", NPC_TOGGLE, &ch_healwin, 0, 0, 0, 0},
    /* battles */
    {"TUER TOUS D'UN COUP", NPC_ACTION, 0, 0, 0, c_killall, 0},
    {"TUER TOUS (AUTO)", NPC_TOGGLE, &ch_killall, 0, 0, 0, 0},
    {"TUER EN UN COUP", NPC_TOGGLE, &ch_onehit, 0, 0, 0, 0},
    {"BARRE PARFAITE", NPC_TOGGLE, &ch_crit, 0, 0, 0, 0},
    {"PAS D'ATTAQUES", NPC_TOGGLE, &ch_noattack, 0, 0, 0, 0},
    {"TOUJOURS EPARGNABLE", NPC_TOGGLE, &ch_spare, 0, 0, 0, 0},
    {"BALLES LENTES", NPC_TOGGLE, &ch_slowbul, 0, 0, 0, 0},
    /* the world */
    {"TELEPORTATION", NPC_CYCLE, &tp_i, NUM_ROOMS, 0, c_tp, tp_hint},
    {"PAS DE RENCONTRES", NPC_TOGGLE, &ch_noenc, 0, 0, 0, 0},
    {"NOCLIP", NPC_TOGGLE, &ch_noclip, 0, 0, 0, 0},
    {"VITESSE MARCHE", NPC_CYCLE, &walk_i, 4, WALK_NAMES, 0, 0},
    {"SAUVEGARDER ICI", NPC_ACTION, 0, 0, 0, c_savehere, 0},
    {"SAUVEGARDE RAPIDE", NPC_ACTION, 0, 0, 0, c_save, 0},
    {"CHARGEMENT RAPIDE", NPC_ACTION, 0, 0, 0, c_load, 0},
    /* progress */
    {"NIVEAU 20", NPC_ACTION, 0, 0, 0, c_lv20, 0},
    {"NIVEAU +1", NPC_ACTION, 0, 0, 0, c_lvup, 0},
    {"EXP +500", NPC_ACTION, 0, 0, 0, c_exp, 0},
    {"GAINS EXP ET OR", NPC_CYCLE, &gain_i, 3, GAIN_NAMES, 0, 0},
    {"VIE MAX +20", NPC_ACTION, 0, 0, 0, c_maxhp, 0},
    {"+1000 OR", NPC_ACTION, 0, 0, 0, c_gold, 0},
    {"SOIGNER", NPC_ACTION, 0, 0, 0, c_heal, 0},
    {"ATK ET DEF +20", NPC_ACTION, 0, 0, 0, c_stats, 0},
    {"ARME ET ARMURE MAX", NPC_ACTION, 0, 0, 0, c_gear, 0},
    {"DONNER OBJET", NPC_CYCLE, &item_i, ITEM_COUNT - 1, item_names, c_item, 0},
    {"OBJETS DE SOIN", NPC_ACTION, 0, 0, 0, c_food, 0},
    {"EFFACER MEURTRES", NPC_ACTION, 0, 0, 0, c_kills0, 0},
    {"CHRONO A 0", NPC_ACTION, 0, 0, 0, c_time0, 0},
    /* the end */
    {"CHOISIR LA FIN", NPC_CYCLE, &end_i, 3, END_NAMES, c_end, 0},
    /* the game */
    {"VITESSE DU JEU", NPC_CYCLE, &speed_i, 4, SPEED_NAMES, 0, 0},
    {"IMAGE PAR IMAGE", NPC_TOGGLE, &step_on, 0, 0, 0, 0},
};
#define NCHEATS ((int)(sizeof CHEATS / sizeof CHEATS[0]))

bool cheats_menu(void) {
  for (int i = 1; i < ITEM_COUNT; i++) item_names[i - 1] = item_info(i)->short_name;
  bool home = np_cheat_menu("CHEATS UNDERTALE", CHEATS, NCHEATS);
  ch_walk = WALK_VALS[walk_i], ch_gain = GAIN_VALS[gain_i];
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
  if (ch_spare && g_mode == MODE_BATTLE) /* (every monster that is still there can be spared) */
    for (int i = 0; i < bt.nmon; i++)
      if (bt.mon[i].alive && !bt.mon[i].gone) bt.mon[i].spare = true;
}
