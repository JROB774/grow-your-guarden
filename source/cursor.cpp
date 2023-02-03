/*////////////////////////////////////////////////////////////////////////////*/

struct Cursor
{
    Texture texture;
    ImmClip clip;
    nkF32   anchor_x;
    nkF32   anchor_y;
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

    if(!g_cursor.texture)
    {
        // Draw the default cursor.
        Texture texture = asset_manager_load<Texture>("hud.png");
        nkF32 cx = cursor_pos.x + ((HUD_CLIP_CURSOR.w * 0.5f) * img_scale) - (4 * hud_scale);
        nkF32 cy = cursor_pos.y + ((HUD_CLIP_CURSOR.h * 0.5f) * img_scale) - (4 * hud_scale);
        imm_texture_ex(texture, cx,cy, img_scale,img_scale, 0.0f, NULL, &HUD_CLIP_CURSOR);
    }
    else
    {
        // Draw the custom cursor.
        Texture texture = g_cursor.texture;
        ImmClip clip = g_cursor.clip;
        if(clip.w == 0.0f) clip.w = NK_CAST(nkF32, get_texture_width(texture));
        if(clip.h == 0.0f) clip.h = NK_CAST(nkF32, get_texture_height(texture));
        nkF32 cx = cursor_pos.x + (g_cursor.anchor_x * clip.w);
        nkF32 cy = cursor_pos.y + (g_cursor.anchor_y * clip.h);
        imm_texture_ex(texture, cx,cy, img_scale,img_scale, 0.0f, NULL, &clip);
    }
}

GLOBAL void set_custom_cursor(Texture texture, ImmClip clip, nkF32 anchor_x, nkF32 anchor_y)
{
    g_cursor.texture = texture;
    g_cursor.clip = clip;
    g_cursor.anchor_x = anchor_x;
    g_cursor.anchor_y = anchor_y;
}

/*////////////////////////////////////////////////////////////////////////////*/
