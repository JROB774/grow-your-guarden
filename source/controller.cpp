/*////////////////////////////////////////////////////////////////////////////*/

struct Controller
{
    nkVec2 camera_pos;
    nkVec2 cursor_pos;
};

INTERNAL Controller g_controller;

INTERNAL nkVec2 mouse_pos_to_screen(nkVec2 pos, nkBool apply_offset)
{
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkVec2 screen_mouse = pos;

    nkF32 scaledw = NK_CAST(nkF32, SCREEN_WIDTH);
    nkF32 scaledh = NK_CAST(nkF32, SCREEN_HEIGHT);

    while((scaledw+SCREEN_WIDTH <= ww) && (scaledh+SCREEN_HEIGHT <= wh))
    {
        scaledw += SCREEN_WIDTH;
        scaledh += SCREEN_HEIGHT;
    }

    nkF32 scaledx = (ww-scaledw)*0.5f;
    nkF32 scaledy = (wh-scaledh)*0.5f;

    nkF32 sx = ww / SCREEN_WIDTH;
    nkF32 sy = wh / SCREEN_HEIGHT;

    nkF32 s = nk_min(sx,sy);

    if(s < 1.0f) s = 1.0f; // Avoid scale of zero.

    if(apply_offset)
    {
        screen_mouse.x -= scaledx;
        screen_mouse.y -= scaledy;
    }

    screen_mouse.x /= floorf(s);
    screen_mouse.y /= floorf(s);

    return screen_mouse;
}

GLOBAL void controller_tick(nkF32 dt)
{
    // Convert the raw mouse position into screen position for the cursor.
    g_controller.cursor_pos = mouse_pos_to_screen(get_window_mouse_pos(), NK_TRUE);

    // Pan the camera around the world.
    if(is_mouse_button_down(MouseButton_Middle) || is_mouse_button_down(MouseButton_Right))
    {
        g_controller.camera_pos -= mouse_pos_to_screen(get_relative_mouse_pos(), NK_FALSE);
    }
}

GLOBAL void controller_draw(void)
{
    // Unset the camera.
    imm_set_view(nk_m4_identity());

    // Draw the cursor.
    Texture cursor = asset_manager_load<Texture>("cursor.png");

    nkF32 x = g_controller.cursor_pos.x;
    nkF32 y = g_controller.cursor_pos.y;

    imm_texture(cursor, x,y);
}

GLOBAL void set_controller_camera(void)
{
    // Setup the view matrix using the controller camera.
    nkF32 cx = g_controller.camera_pos.x;
    nkF32 cy = g_controller.camera_pos.y;

    nkMat4 camera_matrix = nk_translate(nk_m4_identity(), { -cx,-cy,0.0f });
    imm_set_view(camera_matrix);
}

/*////////////////////////////////////////////////////////////////////////////*/
