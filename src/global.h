#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#ifdef __cplusplus
extern "C" {
#endif

# define TI99_RENDER_NORMAL      0
# define TI99_RENDER_FIT_HEIGHT  1
# define TI99_RENDER_FIT         2
# define TI99_LAST_RENDER        2

# define MAX_PATH            256
# define TI99_MAX_SAVE_STATE    5

# define TI99_LOAD_K7_MODE      0
# define TI99_LOAD_DISK_MODE    1
# define TI99_MAX_LOAD_MODE     1

# define TI99_SCREEN_W    256
# define TI99_SCREEN_H    192
# define TI99_HEIGHT      TI99_SCREEN_H
# define TI99_WIDTH       TI99_SCREEN_W
# define SNAP_WIDTH   (TI99_SCREEN_W/2)
# define SNAP_HEIGHT  (TI99_SCREEN_H/2)

# include <psptypes.h>
#include <SDL.h>

  typedef struct TI99_save_t {

    SDL_Surface    *surface;
    char            used;
    char            thumb;
    ScePspDateTime  date;

  } TI99_save_t;

  typedef struct TI99_t {
 
    TI99_save_t ti99_save_state[TI99_MAX_SAVE_STATE];
    char ti99_save_name[MAX_PATH];
    char ti99_home_dir[MAX_PATH];
    int  psp_screenshot_id;
    int  psp_cpu_clock;
    int  psp_reverse_analog;
    int  psp_display_lr;
    int  ti99_view_fps;
    int  ti99_current_fps;
    int  ti99_snd_enable;
    int  ti99_render_mode;
    int  ti99_vsync;
    int  psp_skip_max_frame;
    int  psp_skip_cur_frame;
    int  ti99_speed_limiter;
    int  ti99_auto_fire;
    int  ti99_auto_fire_pressed;
    int  ti99_auto_fire_period;

  } TI99_t;

  extern int ti99_in_menu;

  extern TI99_t TI99;

  extern void ti99_global_init();

//END_LUDO:
#ifdef __cplusplus
}
#endif

# endif
