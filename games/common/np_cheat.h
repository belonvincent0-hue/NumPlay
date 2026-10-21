/* A cheat menu any NumPlay game can open, the way Ship of Harkinian's is opened: EXP.
 *
 * It needs nothing of the game but <eadk.h>: it draws straight to the screen with its own 5x7 font
 * (capitals, no accents), reads the keys itself, and returns when it is closed. The game then has to
 * redraw its whole screen and forget the keys it thought were down. Include it in one file per app.
 *
 *   static int god;
 *   static const char *give(void) { lives += 3; return "+3 VIES"; }
 *   static np_cheat_t cheats[] = {
 *       {"INVINCIBLE", NPC_TOGGLE, &god, 0, 0, 0},
 *       {"VITESSE", NPC_CYCLE, &speed, 3, speed_names, 0},
 *       {"+3 VIES", NPC_ACTION, 0, 0, 0, give},
 *   };
 *   if (np_cheat_key()) { if (np_cheat_menu("CHEATS", cheats, 3)) quit(); redraw_everything(); }
 *
 * Up/Down choose, Left/Right change a value, OK or EXE toggles or runs, Back or EXP closes it. */
#ifndef NP_CHEAT_H
#define NP_CHEAT_H
#include <eadk.h>
#include <stdbool.h>
#include <stdint.h>

enum { NPC_TOGGLE, NPC_CYCLE, NPC_ACTION };
typedef struct {
  const char *name;
  int kind;
  int *value;                 /* toggle: 0 or 1; cycle: 0 .. count - 1 */
  int count;                  /* cycle: how many values */
  const char *const *labels;  /* cycle: the label of each value (or 0: the number) */
  const char *(*run)(void);   /* action: what it does; returns a line to show */
} np_cheat_t;

#define NPC_RGB(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))
#define NPC_BG NPC_RGB(20, 20, 40)
#define NPC_SEL NPC_RGB(70, 70, 130)
#define NPC_TXT NPC_RGB(255, 255, 255)
#define NPC_ON NPC_RGB(90, 230, 90)
#define NPC_DIM NPC_RGB(160, 160, 170)
#define NPC_ROWS 10

/* the classic 5x7 font, ASCII 32 .. 90: a glyph's columns, the top row the lowest bit */
static const uint8_t npc_font[59][5] = {
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

static void npc_fill(int x, int y, int w, int h, uint16_t c) {
  eadk_rect_t r = {(uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h};
  eadk_display_push_rect_uniform(r, c);
}
static void npc_glyph(int x, int y, char ch, uint16_t fg, uint16_t bg) {
  if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 32);
  if (ch < 32 || ch > 90) ch = '?';
  const uint8_t *g = npc_font[ch - 32];
  uint16_t buf[12 * 16]; /* a cell is 6 x 8, drawn twice as big */
  for (int py = 0; py < 16; py++)
    for (int px = 0; px < 12; px++) {
      int cx = px / 2, cy = py / 2;
      buf[py * 12 + px] = (cx < 5 && cy < 7 && (g[cx] >> cy & 1)) ? fg : bg;
    }
  eadk_rect_t r = {(uint16_t)x, (uint16_t)y, 12, 16};
  eadk_display_push_rect(r, buf);
}
static void npc_text(int x, int y, const char *s, uint16_t fg, uint16_t bg) {
  for (; *s; s++, x += 12) npc_glyph(x, y, *s, fg, bg);
}
static void npc_num(char *out, int v) {
  char t[12];
  int n = 0, k = 0;
  unsigned u = v < 0 ? (unsigned)-v : (unsigned)v;
  if (v < 0) out[k++] = '-';
  do t[n++] = (char)('0' + u % 10), u /= 10;
  while (u);
  while (n) out[k++] = t[--n];
  out[k] = 0;
}

/* EXP pressed anew (call it once per frame) */
static bool np_cheat_key(void) {
  static bool held;
  bool down = eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_exp);
  bool edge = down && !held;
  held = down;
  return edge;
}

static void npc_draw(const char *title, const np_cheat_t *list, int n, int sel, int top, const char *msg) {
  npc_fill(10, 6, 300, 228, NPC_BG);
  npc_fill(10, 6, 300, 1, NPC_TXT), npc_fill(10, 233, 300, 1, NPC_TXT);
  npc_fill(10, 6, 1, 228, NPC_TXT), npc_fill(309, 6, 1, 228, NPC_TXT);
  npc_text(22, 14, title, NPC_TXT, NPC_BG);
  for (int r = 0; r < NPC_ROWS && top + r < n; r++) {
    int i = top + r, y = 38 + r * 18;
    const np_cheat_t *c = &list[i];
    uint16_t bg = i == sel ? NPC_SEL : NPC_BG, fg = NPC_TXT;
    npc_fill(14, y - 1, 292, 18, bg);
    npc_text(20, y, c->name, fg, bg);
    char num[14];
    const char *v = "OK";
    uint16_t vc = NPC_DIM;
    if (c->kind == NPC_TOGGLE) v = *c->value ? "ON" : "OFF", vc = *c->value ? NPC_ON : NPC_DIM;
    else if (c->kind == NPC_CYCLE) {
      int k = *c->value;
      if (c->labels) v = c->labels[k];
      else npc_num(num, k), v = num;
      vc = NPC_TXT;
    }
    int w = 0;
    for (const char *p = v; *p; p++) w += 12;
    npc_text(300 - w, y, v, vc, bg);
  }
  npc_fill(14, 38 + NPC_ROWS * 18 + 2, 292, 16, NPC_BG);
  npc_text(20, 38 + NPC_ROWS * 18 + 2, msg, NPC_ON, NPC_BG);
  npc_text(20, 214, "OK:VALIDER RETOUR:FERMER", NPC_DIM, NPC_BG);
}

static bool npc_down(uint64_t s, eadk_key_t k) { return eadk_keyboard_key_down(s, k); }
/* wait for the keys to be let go (so that the game does not read them as presses) */
static void npc_release(void) {
  uint64_t end = eadk_timing_millis() + 700;
  while (eadk_keyboard_scan() != 0 && (int64_t)(end - eadk_timing_millis()) > 0) eadk_timing_msleep(10);
}

/* the menu: returns 1 if Home or On/Off was pressed (the game should quit), else 0 once it is closed */
static int np_cheat_menu(const char *title, const np_cheat_t *list, int n) {
  static const eadk_key_t K[8] = {eadk_key_up, eadk_key_down, eadk_key_left, eadk_key_right, eadk_key_ok, eadk_key_exe, eadk_key_back, eadk_key_exp};
  int sel = 0, top = 0;
  const char *msg = "";
  uint32_t prev = 0xFF; /* (nothing counts as a press until it has been let go) */
  uint64_t rep = 0;
  npc_release();
  npc_draw(title, list, n, sel, top, msg);
  for (;;) {
    uint64_t s = eadk_keyboard_scan();
    if (npc_down(s, eadk_key_home) || npc_down(s, eadk_key_on_off)) return 1;
    uint32_t cur = 0;
    for (int i = 0; i < 8; i++)
      if (npc_down(s, K[i])) cur |= 1u << i;
    uint32_t press = cur & ~prev;
    uint64_t now = eadk_timing_millis();
    if (cur & 0xF) { /* arrows repeat while held */
      if (press & 0xF) rep = now + 350;
      else if ((int64_t)(now - rep) >= 0) press |= cur & 0xF, rep = now + 90;
    }
    prev = cur;
    if (press & 0xC0) break; /* Back, EXP */
    if (press) {
      msg = "";
      const np_cheat_t *c = &list[sel];
      if (press & 1) sel = sel ? sel - 1 : n - 1;
      if (press & 2) sel = sel < n - 1 ? sel + 1 : 0;
      if (press & 12) { /* Left, Right */
        int d = (press & 8) ? 1 : -1;
        if (c->kind == NPC_TOGGLE) *c->value ^= 1;
        else if (c->kind == NPC_CYCLE) *c->value = (*c->value + d + c->count) % c->count;
      }
      if (press & 0x30) { /* OK, EXE */
        if (c->kind == NPC_TOGGLE) *c->value ^= 1;
        else if (c->kind == NPC_CYCLE) *c->value = (*c->value + 1) % c->count;
        else if (c->run) {
          const char *m = c->run();
          msg = m ? m : "";
        }
      }
      if (sel < top) top = sel;
      if (sel >= top + NPC_ROWS) top = sel - NPC_ROWS + 1;
      npc_draw(title, list, n, sel, top, msg);
    }
    eadk_timing_msleep(20);
  }
  npc_release();
  return 0;
}
#endif
