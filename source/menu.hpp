/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void menu_init(void);
GLOBAL void menu_quit(void);
GLOBAL void menu_tick(nkF32 dt);
GLOBAL void menu_draw(void);

// A value of -1 passed into y means to use the previous y position + the new button's line height.
INTERNAL nkBool tick_menu_text_button(const nkChar* text, nkF32 y, nkS32 size, nkBool interactive = NK_TRUE);
INTERNAL void   draw_menu_text_button(const nkChar* text, nkF32 y, nkS32 size, nkBool interactive = NK_TRUE);

INTERNAL nkBool tick_menu_image_button(ImmClip clip, nkF32 x, nkF32 y);
INTERNAL void   draw_menu_image_button(ImmClip clip, nkF32 x, nkF32 y);

INTERNAL nkBool tick_menu_toggle_button(ImmClip clip, nkF32 x, nkF32 y, nkBool toggle);
INTERNAL void   draw_menu_toggle_button(ImmClip clip, nkF32 x, nkF32 y, nkBool toggle);

/*////////////////////////////////////////////////////////////////////////////*/
