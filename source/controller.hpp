/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void     controller_init            (void);
GLOBAL void     controller_tick            (nkF32 dt);
GLOBAL void     controller_draw            (void);
GLOBAL void     controller_reset           (void);
GLOBAL void     set_controller_camera      (void);
GLOBAL void     unset_controller_camera    (void);
GLOBAL void     increment_kill_count       (void);
GLOBAL EntityID get_selected_plant         (void);
GLOBAL nkVec2   get_cursor_world_pos       (void);
GLOBAL void     add_money                  (nkS32 money);
GLOBAL void     sub_money                  (nkS32 money);
GLOBAL nkBool   is_something_selected      (void);
GLOBAL nkS32    get_health                 (void);
GLOBAL nkS32    get_kills                  (void);
GLOBAL nkBool   anything_unlocked_this_wave(void);

GLOBAL void draw_hud_stat(Texture texture, nkF32 x, nkF32 y, nkF32 icon_scale, nkF32 hud_scale, ImmClip clip, nkS32 stat);

/*////////////////////////////////////////////////////////////////////////////*/
