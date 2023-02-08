/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(WaveState, nkS32)
{
    WaveState_Prepare,
    WaveState_Fight,
    WaveState_TOTAL
};

GLOBAL void      wave_manager_tick      (nkF32 dt);
GLOBAL void      wave_manager_draw_world(void);
GLOBAL void      wave_manager_draw_hud  (void);
GLOBAL void      wave_manager_reset     (void);
GLOBAL void      begin_next_wave_now    (void);
GLOBAL nkU32     get_wave_counter       (void);
GLOBAL nkU32     get_waves_beaten       (void);
GLOBAL WaveState get_wave_state         (void);

/*////////////////////////////////////////////////////////////////////////////*/
