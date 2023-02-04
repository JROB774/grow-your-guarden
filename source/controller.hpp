/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void     controller_init        (void);
GLOBAL void     controller_tick        (nkF32 dt);
GLOBAL void     controller_draw        (void);
GLOBAL void     controller_reset       (void);
GLOBAL void     set_controller_camera  (void);
GLOBAL void     unset_controller_camera(void);
GLOBAL void     increment_kill_count   (void);
GLOBAL EntityID get_selected_plant     (void);

/*////////////////////////////////////////////////////////////////////////////*/
