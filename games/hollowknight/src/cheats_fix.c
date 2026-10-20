/* Cheat menu for Hollow Knight on the NumWorks, in the spirit of Ship of Harkinian's.
 * EXP opens it in game (the game stops); Up/Down choose, OK toggles or runs, Left/Right change a value,
 * Back or EXP closes it. LN steps one frame when "IMAGE PAR IMAGE" is on. Text is drawn by its own 5x7 font
 * straight to the screen (no accents), so that nothing of the game's menus is needed. */
#include "cheats.h"

enum {
  C_INF_HEALTH, C_INF_SOUL, C_INF_GEO, C_INVINCIBLE, C_NAIL, C_MOON, C_NOCLIP, C_NOCOOL, C_ABILITIES, C_SPELLS,
  C_CHARMS, C_NOTCHES, C_MAP, C_HEAL, C_TELEPORT, C_SPEED, C_STEP, C_HITBOX, C_SAVE, C_LOAD, C_GEO500, C_MAXHP
};
enum { T_TOGGLE, T_CYCLE, T_ACTION, T_ROOM };
typedef struct {
  const char *name;
  uint8_t type;
} Item;
static const Item items[NUM_CHEATS] = {
    [C_INF_HEALTH] = {"VIE INFINIE", T_TOGGLE},      [C_INF_SOUL] = {"AME INFINIE", T_TOGGLE},
    [C_INF_GEO] = {"GEO INFINI", T_TOGGLE},          [C_INVINCIBLE] = {"INVINCIBLE", T_TOGGLE},
    [C_NAIL] = {"DEGATS CLOU", T_CYCLE},             [C_MOON] = {"SAUT LUNAIRE", T_TOGGLE},
    [C_NOCLIP] = {"NOCLIP", T_TOGGLE},               [C_NOCOOL] = {"DASH SANS DELAI", T_TOGGLE},
    [C_ABILITIES] = {"DEBLOQUER DASH", T_ACTION},    [C_SPELLS] = {"DEBLOQUER SORTS", T_ACTION},
    [C_CHARMS] = {"TOUS LES CHARMES", T_ACTION},     [C_NOTCHES] = {"ENCOCHES INFINIES", T_TOGGLE},
    [C_MAP] = {"CARTE COMPLETE", T_ACTION},          [C_HEAL] = {"SOIGNER", T_ACTION},
    [C_TELEPORT] = {"TELEPORTATION", T_ROOM},        [C_SPEED] = {"VITESSE JEU", T_CYCLE},
    [C_STEP] = {"IMAGE PAR IMAGE", T_TOGGLE},        [C_HITBOX] = {"HITBOX", T_TOGGLE},
    [C_SAVE] = {"SAUVEGARDE RAPIDE", T_ACTION},      [C_LOAD] = {"CHARGEMENT RAPIDE", T_ACTION},
    [C_GEO500] = {"+500 GEO", T_ACTION},             [C_MAXHP] = {"9 MASQUES", T_ACTION},
};

static uint8_t val[NUM_CHEATS] = {[C_SPEED] = 1};   /* toggles (0/1), cycles' index */
static int room_sel;
static const char *const nail_names[] = {"AUTO", "10", "25", "99"};
static const int8_t nail_vals[] = {0, 10, 25, 99};
static const char *const speed_names[] = {"X0.5", "X1", "X2", "X3"};
static const uint8_t speed_ms[] = {40, 20, 10, 7};

static bool g_open, g_dirty, g_step, g_wait_release;
static uint32_t g_prev, g_hold;
static int sel, top;
static const char *g_msg = "";

/* ---------------------------------------------------------------- what the hooks ask */
bool cheat_invincible(void) { return val[C_INVINCIBLE]; }
bool cheat_moonjump(void) { return val[C_MOON]; }
bool cheat_noclip(void) { return val[C_NOCLIP]; }
void cheat_noclip_move(Body *b, uint32_t keys, float dt) {
  float sp = (keys & K_DASH) ? 40.0f : 14.0f;
  float dx = (float)((keys & K_RIGHT) != 0) - (float)((keys & K_LEFT) != 0);
  float dy = (float)((keys & K_UP) != 0) - (float)((keys & K_DOWN) != 0);
  b->vx = dx * sp, b->vy = dy * sp;
  b->x += b->vx * dt, b->y += b->vy * dt;
  b->nevents = 0, b->ncontacts = 0;
}
uint32_t cheats_tick_ms(void) { return speed_ms[val[C_SPEED]]; }
bool cheats_run_tick(void) {
  if (!val[C_STEP]) return true;
  bool s = g_step;
  g_step = false;
  return s;
}

/* ---------------------------------------------------------------- the cheats themselves */
void cheats_tick(void) {
  if (!menu_in_game()) return;
  PlayerData *p = &g_pd;
  Hero *h = &g_hero;
  if (val[C_INF_HEALTH] && !h->cs.dead) p->health = p->max_health;
  if (val[C_INF_SOUL]) p->mp = p->max_mp, p->mp_reserve = p->mp_reserve_max;
  if (val[C_INF_GEO] && p->geo < 99999) p->geo = 99999;
  if (nail_vals[val[C_NAIL]]) p->nail_damage = nail_vals[val[C_NAIL]];
  if (val[C_NOCOOL]) h->dash_cooldown_timer = 0, h->attack_cooldown = 0, h->air_dashed = false;
  if (val[C_NOTCHES]) p->charm_slots_filled = 0, pd_set_flag(PDF_OVERCHARMED, false);
}

static struct {
  bool ok;
  int room;
  float x, y;
  bool right;
  PlayerData pd;
} ss;

static void place_hero(float x, float y) {
  g_hero.body.x = x, g_hero.body.y = y, g_hero.body.vx = g_hero.body.vy = 0;
  cam_snap_to_hero();
}

static bool do_action(int id) {   /* -> the menu closes */
  switch (id) {
    case C_ABILITIES:
      g_pd.has_dash = g_pd.can_dash = true;
      pd_set_flag(PDF_HAS_DASH, true);
      g_msg = "DASH DEBLOQUE";
      break;
    case C_SPELLS:
      g_pd.has_spell = true;
      g_pd.fireball_level = 2;
      g_msg = "SORTS DEBLOQUES";
      break;
    case C_CHARMS:
      pd_set_flag(PDF_HAS_CHARM, true);
      for (int i = 0; i < 40; i++) pd_set_flag(PDF_GOT_CHARM_1 + i, true);
      g_pd.charms_owned = 40;
      g_pd.charm_slots = 11;
      g_msg = "40 CHARMES, 11 ENCOCHES";
      break;
    case C_MAP:
      pd_set_flag(PDF_HAS_MAP, true), pd_set_flag(PDF_HAS_QUILL, true);
      pd_set_flag(PDF_MAP_CROSSROADS, true), pd_set_flag(PDF_MAP_GREENPATH, true);
      memset(g_pd.rooms_visited, 0xFF, sizeof g_pd.rooms_visited);
      memset(g_pd.rooms_mapped, 0xFF, sizeof g_pd.rooms_mapped);
      g_msg = "CARTE COMPLETE";
      break;
    case C_HEAL:
      hero_max_health();
      g_msg = "SOIGNE";
      break;
    case C_GEO500:
      g_pd.geo += 500;
      g_msg = "+500 GEO";
      break;
    case C_MAXHP:
      g_pd.max_health = 9, g_pd.health = 9;
      g_msg = "9 MASQUES";
      break;
    case C_TELEPORT: {
      if (!game_enter(room_sel, 20.0f, 10.0f, true)) {
        g_msg = "ECHEC";
        break;
      }
      /* (the room's respawn point, else its first gate) */
      int n;
      const Ent *es = room_ents(&n);
      int at = -1;
      for (int i = 0; i < n && at < 0; i++)
        if (es[i].type == ENT_RESPAWN) at = i;
      for (int i = 0; i < n && at < 0; i++)
        if (es[i].type == ENT_GATE) at = i;
      if (at >= 0) {
        float x = es[at].type == ENT_GATE ? (es[at].x0 + es[at].x1) / 2 : es[at].x0;
        float y = es[at].type == ENT_GATE ? (es[at].y0 + es[at].y1) / 2 : es[at].y0;
        place_hero(x, hero_ground_y(x, y + 1.0f));
      }
      g_msg = "TELEPORTE";
      return true;
    }
    case C_SAVE:
      if (!menu_in_game()) break;
      ss.ok = true, ss.room = g_room.id, ss.x = g_hero.body.x, ss.y = g_hero.body.y;
      ss.right = g_hero.cs.facing_right, ss.pd = g_pd;
      g_msg = "ETAT SAUVEGARDE";
      break;
    case C_LOAD:
      if (!ss.ok) {
        g_msg = "RIEN A CHARGER";
        break;
      }
      g_pd = ss.pd;
      g_pd.disable_pause = false;
      if (!game_enter(ss.room, ss.x, ss.y, ss.right)) {
        g_msg = "ECHEC";
        break;
      }
      place_hero(ss.x, ss.y);
      g_msg = "ETAT CHARGE";
      return true;
  }
  return false;
}

/* ---------------------------------------------------------------- drawing (screen pixels, straight to the display) */
#define RGB(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))
#define COL_BG RGB(20, 20, 40)
#define COL_SEL RGB(70, 70, 130)
#define COL_TXT RGB(255, 255, 255)
#define COL_ON RGB(90, 230, 90)
#define COL_OFF RGB(230, 90, 90)
#define COL_DIM RGB(160, 160, 170)

/* the classic 5x7 font, ASCII 32 .. 90: a glyph's columns, the top row the lowest bit */
static const uint8_t font5x7[59][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x5F, 0x00, 0x00}, {0x00, 0x07, 0x00, 0x07, 0x00},
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, {0x24, 0x2A, 0x7F, 0x2A, 0x12}, {0x23, 0x13, 0x08, 0x64, 0x62},
    {0x36, 0x49, 0x55, 0x22, 0x50}, {0x00, 0x05, 0x03, 0x00, 0x00}, {0x00, 0x1C, 0x22, 0x41, 0x00},
    {0x00, 0x41, 0x22, 0x1C, 0x00}, {0x14, 0x08, 0x3E, 0x08, 0x14}, {0x08, 0x08, 0x3E, 0x08, 0x08},
    {0x00, 0x50, 0x30, 0x00, 0x00}, {0x08, 0x08, 0x08, 0x08, 0x08}, {0x00, 0x60, 0x60, 0x00, 0x00},
    {0x20, 0x10, 0x08, 0x04, 0x02}, {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31}, {0x18, 0x14, 0x12, 0x7F, 0x10},
    {0x27, 0x45, 0x45, 0x45, 0x39}, {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E}, {0x00, 0x36, 0x36, 0x00, 0x00},
    {0x00, 0x56, 0x36, 0x00, 0x00}, {0x08, 0x14, 0x22, 0x41, 0x00}, {0x14, 0x14, 0x14, 0x14, 0x14},
    {0x00, 0x41, 0x22, 0x14, 0x08}, {0x02, 0x01, 0x51, 0x09, 0x06}, {0x32, 0x49, 0x79, 0x41, 0x3E},
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36}, {0x3E, 0x41, 0x41, 0x41, 0x22},
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x09, 0x01},
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F}, {0x00, 0x41, 0x7F, 0x41, 0x00},
    {0x20, 0x40, 0x41, 0x3F, 0x01}, {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F}, {0x3E, 0x41, 0x41, 0x41, 0x3E},
    {0x7F, 0x09, 0x09, 0x09, 0x06}, {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7F, 0x01, 0x01}, {0x3F, 0x40, 0x40, 0x40, 0x3F},
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, {0x3F, 0x40, 0x38, 0x40, 0x3F}, {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07}, {0x61, 0x51, 0x49, 0x45, 0x43},
};

static void glyph(int x, int y, int s, char c, uint16_t fg, uint16_t bg) {
  if (c >= 'a' && c <= 'z') c = (char)(c - 32);
  if (c < 32 || c > 90) c = '?';
  const uint8_t *g = font5x7[c - 32];
  uint16_t buf[12 * 16];   /* (a glyph's cell: 6 x 8 at most 2 times) */
  int w = 6 * s, h = 8 * s;
  for (int py = 0; py < h; py++)
    for (int px = 0; px < w; px++) {
      int cx = px / s, cy = py / s;
      buf[py * w + px] = (cx < 5 && cy < 7 && (g[cx] >> cy & 1)) ? fg : bg;
    }
  plat_push(x, y, w, h, buf);
}
static void text(int x, int y, int s, const char *t, uint16_t fg, uint16_t bg) {
  for (; *t; t++, x += 6 * s) glyph(x, y, s, *t, fg, bg);
}

#define ROWS 9
#define ROW_H 18
#define ROW_Y 36
static void value_text(int id, char *out, uint16_t *col) {
  *col = COL_TXT;
  out[0] = 0;
  switch (items[id].type) {
    case T_TOGGLE: strcpy(out, val[id] ? "ON" : "OFF"), *col = val[id] ? COL_ON : COL_OFF; break;
    case T_CYCLE: strcpy(out, id == C_NAIL ? nail_names[val[id]] : speed_names[val[id]]); break;
    case T_ROOM: {
      int n = room_sel, k = 0;
      char t[4];
      int c = 0;
      do t[c++] = (char)('0' + n % 10), n /= 10;
      while (n && c < 3);
      out[k++] = '<';
      while (c) out[k++] = t[--c];
      out[k++] = '>', out[k] = 0;
      break;
    }
    default: strcpy(out, "OK"); *col = COL_DIM; break;
  }
}
static void draw_row(int row) {
  int id = top + row, y = ROW_Y + row * ROW_H;
  uint16_t bg = id == sel ? COL_SEL : COL_BG, col;
  plat_fill(14, y, 292, ROW_H, bg);
  char v[8];
  value_text(id, v, &col);
  text(18, y + 1, 2, items[id].name, COL_TXT, bg);
  text(236, y + 1, 2, v, col, bg);
}
static void draw_menu(void) {
  if (sel < top) top = sel;
  if (sel >= top + ROWS) top = sel - ROWS + 1;
  plat_fill(10, 8, 300, 224, COL_BG);
  text(18, 14, 2, "CHEATS", COL_TXT, COL_BG);
  char c[8];
  int n = sel + 1;
  c[0] = n >= 10 ? (char)('0' + n / 10) : ' ', c[1] = (char)('0' + n % 10), c[2] = '/', c[3] = '2', c[4] = '2', c[5] = 0;
  text(250, 18, 1, c, COL_DIM, COL_BG);
  for (int r = 0; r < ROWS; r++) draw_row(r);
  text(16, 202, 1, "HAUT/BAS:CHOISIR  OK:VALIDER  GAUCHE/DROITE:VALEUR", COL_DIM, COL_BG);
  const char *info = g_msg;
  if (items[sel].type == T_ROOM && room_name(room_sel)) info = room_name(room_sel);
  plat_fill(16, 214, 288, 8, COL_BG);
  text(16, 214, 1, info, COL_ON, COL_BG);
  g_dirty = false;
}

/* the Knight's hitbox: a green outline (the gameplay plane's scale at the view's center) */
static void outline(int x0, int y0, int x1, int y1, uint16_t c) {
  int lo_y = VIEW_Y, hi_y = VIEW_Y + VIEW_H - 1;
  if (x1 < 0 || x0 >= SCREEN_W || y1 < lo_y || y0 > hi_y) return;
  int cx0 = x0 < 0 ? 0 : x0, cx1 = x1 >= SCREEN_W ? SCREEN_W - 1 : x1;
  int cy0 = y0 < lo_y ? lo_y : y0, cy1 = y1 > hi_y ? hi_y : y1;
  if (y0 >= lo_y) plat_fill(cx0, y0, cx1 - cx0 + 1, 1, c);
  if (y1 <= hi_y) plat_fill(cx0, y1, cx1 - cx0 + 1, 1, c);
  if (x0 >= 0) plat_fill(x0, cy0, 1, cy1 - cy0 + 1, c);
  if (x1 < SCREEN_W) plat_fill(x1, cy0, 1, cy1 - cy0 + 1, c);
}
void cheats_overlay(void) {
  if (!val[C_HITBOX] || !menu_in_game() || g_hero.hidden) return;
  const Body *b = &g_hero.body;
  float s = FOCAL / -CAM_Z, cx = b->x + b->ox, cy = b->y + b->oy;
  int x0 = (int)(SCREEN_W / 2 + (cx - b->hx - g_cam_x) * s), x1 = (int)(SCREEN_W / 2 + (cx + b->hx - g_cam_x) * s);
  int y0 = (int)(VIEW_Y + VIEW_H / 2 - (cy + b->hy - g_cam_y) * s), y1 = (int)(VIEW_Y + VIEW_H / 2 - (cy - b->hy - g_cam_y) * s);
  outline(x0, y0, x1, y1, RGB(0, 255, 0));
}

/* ---------------------------------------------------------------- the menu */
/* the conditions for the menu to open: bit i set when condition i holds */
#define N_COND 10
static uint32_t conditions(void) {
  uint32_t c = 0;
  if (menu_in_game()) c |= 1u << 0;
  c |= 1u << 1;   /* (disable_pause is not asked: the game keeps it on in some scenes, and the menu is no pause menu) */
  if (!game_changing_room()) c |= 1u << 2;
  if (!inv_open()) c |= 1u << 3;
  if (!shop_open()) c |= 1u << 4;
  if (!stag_busy()) c |= 1u << 5;
  if (!msg_shown()) c |= 1u << 6;
  if (!dialogue_box_shown()) c |= 1u << 7;
  if (!collect_active()) c |= 1u << 8;
  if (!g_hero.cs.dead) c |= 1u << 9;
  return c;
}
static bool can_open(void) { return conditions() == (1u << N_COND) - 1; }

/* a test: the key was heard (blue square), then one square a condition: green holds, red blocks; kept 1.5 s */
static void debug_flash(void) {
  uint32_t c = conditions();
  plat_fill(4, 4, 12, 12, RGB(0, 0, 255));
  for (int i = 0; i < N_COND; i++) plat_fill(24 + i * 18, 4, 12, 12, (c >> i & 1) ? RGB(0, 255, 0) : RGB(255, 0, 0));
  plat_sleep(1500);
}

uint32_t cheats_keys(uint32_t keys) {
  if (g_wait_release) {
    if (keys & ~(uint32_t)K_ANY) return 0;
    g_wait_release = false;
  }
  return keys;
}

bool cheats_menu(uint32_t keys) {
  uint32_t pressed = keys & ~g_prev;
  g_prev = keys;
  if (pressed & K_STEP) g_step = true;
  if (!g_open) {
    if (!(pressed & K_CHEAT)) return false;
    if (!can_open()) {
      debug_flash();
      return false;
    }
    g_open = true, g_msg = "", g_dirty = true;
    pressed = 0;   /* (this press opened the menu: it is not one to close it too) */
  }
  const uint32_t dirs = K_UP | K_DOWN | K_LEFT | K_RIGHT;
  if (keys & dirs) {
    g_hold++;
    if (g_hold > 12 && g_hold % 3 == 0) pressed |= keys & dirs;   /* (held: repeats) */
  } else
    g_hold = 0;
  if (pressed & (K_CHEAT | K_BACK)) {
    g_open = false, g_wait_release = true;
    return true;
  }
  if (pressed & K_UP) sel = sel ? sel - 1 : NUM_CHEATS - 1, g_msg = "", g_dirty = true;
  if (pressed & K_DOWN) sel = sel < NUM_CHEATS - 1 ? sel + 1 : 0, g_msg = "", g_dirty = true;
  const Item *it = &items[sel];
  if (pressed & (K_LEFT | K_RIGHT)) {
    int d = (pressed & K_RIGHT) ? 1 : -1;
    if (it->type == T_CYCLE) {
      const int n = 4;
      val[sel] = (uint8_t)((val[sel] + d + n) % n), g_dirty = true;
    } else if (it->type == T_ROOM) {
      room_sel = (room_sel + d + NUM_ROOMS) % NUM_ROOMS, g_dirty = true;
    } else if (it->type == T_TOGGLE)
      val[sel] ^= 1, g_dirty = true;
  }
  if (pressed & K_OK) {
    if (it->type == T_TOGGLE) val[sel] ^= 1;
    else if (it->type == T_CYCLE) val[sel] = (uint8_t)((val[sel] + 1) % 4);
    else if (do_action(sel)) {
      g_open = false, g_wait_release = true;
      return true;
    }
    g_dirty = true;
  }
  if (g_dirty) draw_menu();
  return true;
}
