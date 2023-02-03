/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void cursor_tick(nkF32 dt)
{
    // Currently does nothing...
}

GLOBAL void cursor_draw(void)
{
    nkVec2 cursor_pos = get_window_mouse_pos();

    // Images need to be scaled down because they are actually at the max scale initially and scaled down for lower scales.
    // Other elements like text and general positioning need to still be scaled up, so we have these two variables to do it.
    nkF32 img_scale = get_hud_scale() / 4.0f;
    nkF32 hud_scale = get_hud_scale();

    // @Incomplete: This is using a bunch of stuff from the controller internals, not very good...
    if(g_controller.watering && !is_game_paused())
    {
        nkF32 cx = cursor_pos.x;
        nkF32 cy = cursor_pos.y;
        imm_texture_ex(g_controller.watercan_tex, cx,cy, img_scale,img_scale, 0.0f, NULL);
    }
    else if(g_controller.removing && !is_game_paused())
    {
        nkF32 cx = cursor_pos.x;
        nkF32 cy = cursor_pos.y;
        imm_texture_ex(g_controller.shovel_tex, cx,cy, img_scale,img_scale, 0.0f, NULL);
    }
    else if(g_controller.selected != NO_SELECTION && !is_game_paused())
    {
        ImmClip clip = { NK_CAST(nkF32, g_controller.selected) * ICON_WIDTH, 0.0f, ICON_WIDTH, ICON_HEIGHT };
        nkF32 cx = cursor_pos.x;
        nkF32 cy = cursor_pos.y;
        imm_texture_ex(g_controller.icons_tex, cx,cy, img_scale,img_scale, 0.0f, NULL, &clip);
    }
    else
    {
        nkF32 cx = cursor_pos.x + (NK_CAST(nkF32, get_texture_width(g_controller.cursor_tex) * 0.5f) * img_scale) - (4 * hud_scale);
        nkF32 cy = cursor_pos.y + (NK_CAST(nkF32, get_texture_height(g_controller.cursor_tex) * 0.5f) * img_scale) - (4 * hud_scale);
        imm_texture_ex(g_controller.cursor_tex, cx,cy, img_scale,img_scale, 0.0f, NULL);
    }
}

/*////////////////////////////////////////////////////////////////////////////*/
