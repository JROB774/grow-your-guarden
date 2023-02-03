/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void cursor_tick(nkF32 dt);
GLOBAL void cursor_draw(void);

// Pass in a NULL texture to set the cursor back to its default graphic.
GLOBAL void set_custom_cursor(Texture texture, ImmClip clip = NK_ZERO_MEM, nkF32 anchor_x = 0.0f, nkF32 anchor_y = 0.0f);

/*////////////////////////////////////////////////////////////////////////////*/
