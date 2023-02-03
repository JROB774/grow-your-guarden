/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void cursor_tick(nkF32 dt);
GLOBAL void cursor_draw(void);

GLOBAL void set_custom_cursor (Texture texture, ImmClip clip = NK_ZERO_MEM, nkF32 anchor_x = 0.0f, nkF32 anchor_y = 0.0f);
GLOBAL void set_default_cursor(void);

/*////////////////////////////////////////////////////////////////////////////*/
