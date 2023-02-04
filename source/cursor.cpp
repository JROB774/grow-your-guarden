/*////////////////////////////////////////////////////////////////////////////*/

struct Cursor
{
    CursorType type;
    Texture    texture;
    ImmClip    clip;
    nkF32      anchor_x;
    nkF32      anchor_y;
};

INTERNAL Cursor g_cursor;

GLOBAL void cursor_tick(nkF32 dt)
{
    // Currently does nothing...
}

GLOBAL void cursor_draw(void)
{
    // Images need to be scaled down because they are actually at the max scale initially and scaled down for lower scales.
    // Other elements like text and general positioning need to still be scaled up, so we have these two variables to do it.
    nkF32 img_scale = get_hud_scale() / 4.0f;
    nkF32 hud_scale = get_hud_scale();

    nkVec2 cursor_pos = get_window_mouse_pos();

    Texture texture = (g_cursor.texture) ? g_cursor.texture : asset_manager_load<Texture>("hud.png");

    switch(g_cursor.type)
    {
        case CursorType_Pointer:
        {
            nkF32 cx = cursor_pos.x + ((HUD_CLIP_POINTER.w * 0.5f) * img_scale) - (4 * hud_scale);
            nkF32 cy = cursor_pos.y + ((HUD_CLIP_POINTER.h * 0.5f) * img_scale) - (4 * hud_scale);
            imm_texture_ex(texture, cx,cy, img_scale,img_scale, 0.0f, NULL, &HUD_CLIP_POINTER);
        } break;
        case CursorType_Arrow:
        {
            nkF32 cx = cursor_pos.x + ((HUD_CLIP_ARROW.w * 0.5f) * img_scale) - (4 * hud_scale);
            nkF32 cy = cursor_pos.y + ((HUD_CLIP_ARROW.h * 0.5f) * img_scale) - (4 * hud_scale);
            imm_texture_ex(texture, cx,cy, img_scale,img_scale, 0.0f, NULL, &HUD_CLIP_ARROW);
        } break;

        case CursorType_Custom:
        {
            // Draw the custom cursor.
            ImmClip clip = g_cursor.clip;
            if(clip.w == 0.0f) clip.w = NK_CAST(nkF32, get_texture_width(texture));
            if(clip.h == 0.0f) clip.h = NK_CAST(nkF32, get_texture_height(texture));
            nkF32 cx = cursor_pos.x + (g_cursor.anchor_x * clip.w);
            nkF32 cy = cursor_pos.y + (g_cursor.anchor_y * clip.h);
            imm_texture_ex(texture, cx,cy, img_scale,img_scale, 0.0f, NULL, &clip);
        } break;
    }
}

GLOBAL void set_cursor(CursorType type, Texture texture, ImmClip clip, nkF32 anchor_x, nkF32 anchor_y)
{
    g_cursor.type     = type;
    g_cursor.texture  = texture;
    g_cursor.clip     = clip;
    g_cursor.anchor_x = anchor_x;
    g_cursor.anchor_y = anchor_y;
}

/*////////////////////////////////////////////////////////////////////////////*/
