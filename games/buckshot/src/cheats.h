/* Cheat menu for Buckshot on the NumWorks: EXP opens it during a match. */
#ifndef BR_CHEATS_H
#define BR_CHEATS_H
#include "game.h"

#define CHEAT_KEY 18 /* the event of the EXP key */

enum { CH_GOD, CH_PEEK, CH_FORCE, CH_DBLANK, CH_SAW, CH_INFITEMS, CH_DSKIP, CH_ITEM, CH_HEAL, CH_DHP1, CH_DNOITEMS,
       CH_ORDER, CH_COUNT };

int cheat_on(int id);                       /* a toggle's state (CH_FORCE: 0 auto, 1 live, 2 blank) */
void cheats_menu(void);                     /* the menu: returns when it is closed */
int cheats_active(void);                    /* the menu is up */
void cheat_before_player_shot(void);        /* in the player's shot: the forced shell */
void cheat_before_dealer_shot(int at_player); /* in the Dealer's: a blank for the player */
const char *cheat_peek_hint(void);          /* "NEXT: LIVE ..." for the hint line */
#endif
