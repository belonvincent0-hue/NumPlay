/* Cheat menu for Buckshot on the NumWorks. EXP opens it during a match (the match waits);
 * Up/Down choose, Left/Right change a value, OK toggles or runs, Back or EXP closes it.
 * Text is the game's own small font: capitals, no accents. */
#include "cheats.h"
#include "scene.h"

typedef struct {
  const char *name;
  uint8_t type; /* 0 toggle, 1 cycle, 2 action, 3 info */
} Item;
enum { K_TOGGLE, K_CYCLE, K_ACTION, K_INFO };
static const Item items[CH_COUNT] = {
    [CH_GOD] = {"VIE INFINIE", K_TOGGLE},
    [CH_PEEK] = {"VOIR CARTOUCHE", K_TOGGLE},
    [CH_FORCE] = {"FORCER CARTOUCHE", K_CYCLE},
    [CH_DBLANK] = {"DEALER TIRE A BLANC", K_TOGGLE},
    [CH_SAW] = {"SCIE ACTIVE", K_TOGGLE},
    [CH_INFITEMS] = {"OBJETS INFINIS", K_TOGGLE},
    [CH_DSKIP] = {"DEALER SAUTE TOUR", K_TOGGLE},
    [CH_ITEM] = {"DONNER OBJET", K_CYCLE},
    [CH_HEAL] = {"SOIGNER", K_ACTION},
    [CH_DHP1] = {"DEALER A 1 CHARGE", K_ACTION},
    [CH_DNOITEMS] = {"VIDER OBJETS DEALER", K_ACTION},
    [CH_ORDER] = {"ORDRE CARTOUCHES", K_INFO},
};

static uint8_t val[CH_COUNT];
static int item_sel = IT_SAW; /* the item "DONNER OBJET" gives */
static int active, sel;
static const char *msg = "";

int cheat_on(int id) { return val[id]; }
int cheats_active(void) { return active; }

/* ------------------------------------------------------------ the hooks */
void cheat_before_player_shot(void) {
  if (val[CH_FORCE] && G.nshell) G.shell[0] = val[CH_FORCE] == 1;
}
void cheat_before_dealer_shot(int at_player) {
  if (val[CH_DBLANK] && at_player && G.nshell) G.shell[0] = 0;
}
const char *cheat_peek_hint(void) {
  return G.shell[0] ? "NEXT: LIVE   OK: USE   BACK: PAUSE" : "NEXT: BLANK   OK: USE   BACK: PAUSE";
}

/* --------------------------------------------------------- the actions */
static void order_text(char *out) { /* L live, B blank, the chamber first */
  int n = 0;
  for (int i = 0; i < G.nshell && i < 8; i++) {
    if (i) out[n++] = ' ';
    out[n++] = G.shell[i] ? 'L' : 'B';
  }
  if (!n) out[n++] = '-';
  out[n] = 0;
}

static void action(int id) {
  switch (id) {
    case CH_HEAL:
      G.hp[0] = (int8_t)G.maxhp;
      msg = "SOIGNE";
      break;
    case CH_DHP1:
      if (G.hp[1] > 1) G.hp[1] = 1;
      msg = "DEALER A 1 CHARGE";
      break;
    case CH_DNOITEMS:
      for (int g = 0; g < 8; g++) G.items[1][g] = IT_NONE;
      msg = "OBJETS DU DEALER VIDES";
      break;
    case CH_ITEM:
      for (int g = 0; g < 8; g++)
        if (G.items[0][g] < 0) {
          G.items[0][g] = (int8_t)item_sel;
          G.seq[0][g] = G.seqn++;
          msg = "OBJET DONNE";
          return;
        }
      msg = "PLUS DE PLACE";
      break;
  }
}

/* ------------------------------------------------------------ drawing */
#define ROW_Y 34
#define ROW_H 14
static void draw(void) {
  gfx_fill(10, 6, 300, 228, C_BLACK);
  gfx_brackets(10, 6, 300, 228, C_WHITE);
  gfx_text_c(&font_big, "CHEATS", 160, 12, C_WHITE);
  for (int i = 0; i < CH_COUNT; i++) {
    int y = ROW_Y + i * ROW_H;
    if (i == sel) gfx_fill(14, y - 2, 292, ROW_H, C_DARK);
    uint8_t c = i == sel ? C_WHITE : C_GREY;
    gfx_text(&font_small, items[i].name, 20, y, c);
    char buf[24];
    const char *v = buf;
    uint8_t vc = c;
    switch (i) {
      case CH_FORCE: v = val[i] == 0 ? "AUTO" : val[i] == 1 ? "LIVE" : "BLANK"; break;
      case CH_ITEM: v = ITEM_NAME[item_sel]; break;
      case CH_ORDER: order_text(buf); break;
      case CH_HEAL:
      case CH_DHP1:
      case CH_DNOITEMS: v = "OK"; break;
      default: v = val[i] ? "ON" : "OFF", vc = val[i] ? C_YELLOW : c; break;
    }
    gfx_text(&font_small, v, 176, y, vc);
  }
  gfx_text(&font_small, "OK:VALIDER  G/D:VALEUR  RETOUR:FERMER", 20, 208, C_GREY);
  gfx_text(&font_small, msg, 20, 220, C_YELLOW);
  gfx_present();
}

void cheats_menu(void) {
  active = 1;
  msg = "";
  gfx_darken(0, 0, GFX_W, GFX_H, 2);
  for (;;) {
    draw();
    int k = key_wait(-1);
    if (k == KEY_BACK || k == CHEAT_KEY) break;
    msg = "";
    if (k == KEY_UP) sel = sel ? sel - 1 : CH_COUNT - 1;
    else if (k == KEY_DOWN) sel = sel < CH_COUNT - 1 ? sel + 1 : 0;
    else if (k == KEY_LEFT || k == KEY_RIGHT) {
      int d = k == KEY_RIGHT ? 1 : -1;
      if (sel == CH_FORCE) val[sel] = (uint8_t)((val[sel] + d + 3) % 3);
      else if (sel == CH_ITEM) item_sel = (item_sel + d + IT_COUNT) % IT_COUNT;
      else if (items[sel].type == K_TOGGLE) val[sel] ^= 1;
    } else if (IS_OK(k)) {
      if (items[sel].type == K_TOGGLE) val[sel] ^= 1;
      else if (sel == CH_FORCE) val[sel] = (uint8_t)((val[sel] + 1) % 3);
      else if (items[sel].type != K_INFO) action(sel);
    }
  }
  active = 0;
  if (redraw) redraw();
  gfx_present();
}
