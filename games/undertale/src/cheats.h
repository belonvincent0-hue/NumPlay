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
extern int ch_walk;     /* Frisk's steps per frame: 1, 2, 3 or 5 */
extern int ch_spare;    /* every monster can always be spared */
extern int ch_chip;     /* a hit takes 1 HP at most */
extern int ch_slowbul;  /* the bullets move at half speed */
extern int ch_infitems; /* food and bandages are not used up */
extern int ch_gain;     /* EXP and gold won: 1, 2 or 5 times */
extern int ch_healwin;  /* full HP after a battle won */
extern int ch_killall;  /* a battle starts and ends: all dead */

bool cheats_menu(void);       /* the menu: true if Home was pressed in it (the game saves and quits) */
void cheats_tick(void);       /* after each game frame */
bool cheats_run_frame(void);  /* false while "frame by frame" waits for LN */
int cheats_frame_len(void);   /* a game frame, in thirds of a millisecond: 100 at normal speed */
void cheat_battle_win(void);  /* battle.c: ends the battle as won */
#endif
