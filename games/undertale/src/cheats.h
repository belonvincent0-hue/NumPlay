/* Undertale: the cheat menu (EXP opens it in the overworld and in battles, like Ship of Harkinian's). */
#ifndef UT_CHEATS_H
#define UT_CHEATS_H
#include <stdbool.h>
#include <stdint.h>

extern int ch_hp;       /* never below 1 HP, and full HP every frame */
extern int ch_god;      /* the SOUL takes no damage */
extern int ch_gold;     /* 9999 gold */
extern int ch_onehit;   /* every hit kills */
extern int ch_crit;     /* the FIGHT bar always counts as dead centre */
extern int ch_noattack; /* the monsters never attack */
extern int ch_noenc;    /* no random encounters */
extern int ch_noclip;   /* walls do not stop Frisk */
extern int ch_walk;     /* 1, 2 or 3: Frisk's steps per frame */

bool cheats_menu(void);       /* the menu: true if Home was pressed in it (the game saves and quits) */
void cheats_tick(void);       /* after each game frame */
bool cheats_run_frame(void);  /* false while "frame by frame" waits for LN */
int cheats_frame_len(void);   /* a game frame, in thirds of a millisecond: 100 at normal speed */
void cheat_battle_win(void);  /* battle.c: ends the battle as won */
#endif
