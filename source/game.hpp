/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void   game_init     (void);
GLOBAL void   game_quit     (void);
GLOBAL void   game_start    (void);
GLOBAL void   game_tick     (nkF32 dt);
GLOBAL void   game_draw     (void);
GLOBAL void   pause_game    (void);
GLOBAL void   unpause_game  (void);
GLOBAL nkBool is_game_paused(void);
GLOBAL nkBool is_game_over  (void);

// Difficulty related multipliers.
GLOBAL void  set_spawn_multiplier(nkF32 multiplier);
GLOBAL void  set_coin_multiplier (nkF32 multiplier);
GLOBAL void  set_bonus_multiplier(nkF32 multiplier);
GLOBAL void  set_prep_multiplier (nkF32 multiplier);
GLOBAL nkF32 get_spawn_multiplier(void);
GLOBAL nkF32 get_coin_multiplier (void);
GLOBAL nkF32 get_bonus_multiplier(void);
GLOBAL nkF32 get_prep_multiplier (void);

// This is a weird place for these to go but whatever...
GLOBAL Sound get_random_munch_sound(void);
GLOBAL Sound get_random_splat_sound(void);

/*////////////////////////////////////////////////////////////////////////////*/
