/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(CursorType, nkS32)
{
    CursorType_Pointer,
    CursorType_Arrow,
    CursorType_Custom,
    CursorType_TOTAL
};

GLOBAL void cursor_tick(nkF32 dt);
GLOBAL void cursor_draw(void);

// The other values after the type are only important if you are setting the cursor type to CursorType_Custom.
GLOBAL void       set_cursor(CursorType type, Texture texture = NULL, ImmClip clip = NK_ZERO_MEM, nkF32 anchor_x = 0.0f, nkF32 anchor_y = 0.0f);
GLOBAL CursorType get_cursor(void);

/*////////////////////////////////////////////////////////////////////////////*/
