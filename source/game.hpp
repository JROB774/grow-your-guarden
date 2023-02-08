/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void   game_init     (void);
GLOBAL void   game_quit     (void);
GLOBAL void   game_start    (void);
GLOBAL void   game_tick     (nkF32 dt);
GLOBAL void   game_draw     (void);
GLOBAL nkBool is_game_paused(void);
GLOBAL nkBool is_game_over  (void);

// This is a weird place for these to go but whatever...
GLOBAL Sound get_random_munch_sound(void);
GLOBAL Sound get_random_splat_sound(void);

/*////////////////////////////////////////////////////////////////////////////*/
