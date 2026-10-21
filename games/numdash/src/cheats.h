/* NumDash: the cheat menu (EXP, in any screen; most of it works in a level). */
#ifndef NUMDASH_CHEATS_H
#define NUMDASH_CHEATS_H
#include <stdbool.h>
#include <stdint.h>

extern int ch_noclip;   /* nothing kills the player */
extern int ch_force;    /* ticks left of noclip a cheat grants (a jump forward, the end of the level) */
extern int ch_hold;     /* the jump button counts as held */

bool cheats_menu(void);         /* true if Home was pressed in it (the game quits) */
uint32_t cheats_speed_pct(void); /* the game's speed: 100 is normal */
void cheats_tick(void);         /* in a level, before each physics step */
#endif
