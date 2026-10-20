/* Cheat menu (inspired by Ship of Harkinian's): open it in game with the EXP key. */
#pragma once
#include "game.h"

#define NUM_CHEATS 22

/* main loop: -> true while the menu is up (the game is then not ticked nor drawn) */
bool cheats_menu(uint32_t keys);
uint32_t cheats_keys(uint32_t keys);   /* (the keys the game gets: none until the menu's are released) */
uint32_t cheats_tick_ms(void);         /* a game step's length: the game speed */
bool cheats_run_tick(void);            /* false while "frame by frame" is on and no step is asked */
void cheats_tick(void);                /* after each game step: the infinite this and that */
void cheats_overlay(void);             /* after gfx_frame: the hitbox */

/* hooks in hero.c */
bool cheat_invincible(void);
bool cheat_moonjump(void);
bool cheat_noclip(void);
void cheat_noclip_move(Body *b, uint32_t keys, float dt);
