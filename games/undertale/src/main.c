#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("Os")   /* not drawn every frame: smaller over faster */
#endif
/* Undertale for the NumWorks calculator.
 *
 * Inspired by Undertale by Toby Fox, not affiliated with him.
 * Made by Mason Chen as part of NumPlay. */
#include "game.h"
#include "gfx.h"
#include "plat.h"
#include "res.h"
#include "cheats.h"
#ifndef HOST
#include "../../common/np_cheat.h"
#endif

#ifndef HOST
const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Undertale";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;
extern const uint8_t ut_data[];

uint32_t perf_frames __attribute__((used));   /* read by tools/emu.py */
uint32_t perf_updates __attribute__((used));

int main(void) {
  uint16_t band[SCREEN_W * GFX_BAND];   /* (here: on the stack, out of the RAM apps get) */
  gfx_set_band_memory(band);
  plat_begin();
  ut_bin = ut_data;
  game_init();
  uint32_t next = plat_millis() * 3;   /* in thirds of a millisecond: 100 per frame, as the game's 30 a second */
  while (g_running) {
    /* the game's frames: drawn when caught up (at most 2 updates per drawing) */
    int n = 0;
    uint32_t now = plat_millis() * 3;
    /* the cheat menu (EXP): the game waits */
    if (np_cheat_key() && (g_mode == MODE_WORLD || g_mode == MODE_BATTLE)) {
      if (cheats_menu()) { /* Home in the menu: as Home in the game */
        game_quit_save();
        break;
      }
      next = plat_millis() * 3;
      now = next;
    }
    while ((int32_t)(now - next) >= 0 && n < 2 && g_running) {
      if (cheats_run_frame()) {
        game_frame(plat_keys());
        cheats_tick();
      }
      perf_updates++;
      next += cheats_frame_len();
      n++;
    }
    if (!g_running) break;
    if ((int32_t)(now - next) > 600) next = now;   /* far behind: do not rush */
    if (n) {
      game_draw();
      perf_frames++;
    } else
      plat_sleep(1);
  }
  return plat_end();
}
#endif
