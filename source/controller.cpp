/*////////////////////////////////////////////////////////////////////////////*/

struct Controller
{
    PlantID hotbar[8];
    PlantID selected;
    nkVec2  camera_pos;
    nkVec2  cursor_pos;
    nkBool  panning;
    nkBool  occluded;
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

GLOBAL void controller_init(void)
{
    // Center the camera on the map.
    g_controller.camera_pos.x = NK_CAST(nkF32, (g_world.width * TILE_WIDTH)) * 0.5f;
    g_controller.camera_pos.y = NK_CAST(nkF32, (g_world.height * TILE_HEIGHT)) * 0.5f;

    // For now we are just starting with the flower plant.
    g_controller.hotbar[0] = PlantID_Flower;
}

GLOBAL void controller_tick(nkF32 dt)
{
    // Convert the raw mouse position into screen position for the cursor.
    g_controller.cursor_pos = mouse_pos_to_screen(get_window_mouse_pos(), NK_TRUE);

    // Pan the camera around the world.
    g_controller.panning = is_mouse_button_down(MouseButton_Middle);
    if(g_controller.panning)
    {
        g_controller.camera_pos -= mouse_pos_to_screen(get_relative_mouse_pos(), NK_FALSE);
    }

    // Check if the cursor is occluded from the world by the HUD.
    Texture hotbar = asset_manager_load<Texture>("hotbar.png");
    nkF32 hw = NK_CAST(nkF32, get_texture_width(hotbar));
    nkF32 hh = NK_CAST(nkF32, get_texture_height(hotbar));
    g_controller.occluded = point_vs_rect(g_controller.cursor_pos, 0.0f,0.0f,hw,hh);

    // Select the hovered plant.
    if(g_controller.occluded && is_mouse_button_pressed(MouseButton_Left))
    {
        nkF32 ix = 8.0f;
        nkF32 iy = 8.0f;

        for(nkS32 i=0,n=NK_ARRAY_SIZE(g_controller.hotbar); i<n; ++i)
        {
            if(point_vs_rect(g_controller.cursor_pos, ix,iy,32.0f,32.0f))
            {
                g_controller.selected = g_controller.hotbar[i];
            }

            ix += 40.0f;
        }
    }
    // De-select current plant.
    if(g_controller.selected != PlantID_None && is_mouse_button_pressed(MouseButton_Right))
    {
        g_controller.selected = PlantID_None;
    }
    // Place current plant.
    if(g_controller.selected != PlantID_None && !g_controller.occluded && !g_controller.panning && is_mouse_button_pressed(MouseButton_Left))
    {
        nkF32 cx = g_controller.cursor_pos.x + (g_controller.camera_pos.x - (NK_CAST(nkF32, SCREEN_WIDTH) * 0.5f));
        nkF32 cy = g_controller.cursor_pos.y + (g_controller.camera_pos.y - (NK_CAST(nkF32, SCREEN_HEIGHT) * 0.5f));

        iPoint tile;

        tile.x = NK_CAST(nkS32, cx / TILE_WIDTH);
        tile.y = NK_CAST(nkS32, cy / TILE_HEIGHT);

        place_plant(g_controller.selected, tile.x, tile.y);
    }
}

GLOBAL void controller_draw(void)
{
    // Draw the highlighted tile if not panning.
    if(!g_controller.panning && !g_controller.occluded && g_controller.selected != PlantID_None)
    {
        Texture highlight = asset_manager_load<Texture>("highlight.png");

        nkF32 cx = g_controller.cursor_pos.x + (g_controller.camera_pos.x - (NK_CAST(nkF32, SCREEN_WIDTH) * 0.5f));
        nkF32 cy = g_controller.cursor_pos.y + (g_controller.camera_pos.y - (NK_CAST(nkF32, SCREEN_HEIGHT) * 0.5f));

        iPoint tile;

        tile.x = NK_CAST(nkS32, cx / TILE_WIDTH);
        tile.y = NK_CAST(nkS32, cy / TILE_HEIGHT);

        // Make sure we're in bounds.
        if(tile.x >= 0 && tile.x < g_world.width && tile.y >= 0 && tile.y < g_world.height)
        {
            nkF32 tx = NK_CAST(nkF32, tile.x * TILE_WIDTH) + (NK_CAST(nkF32,TILE_WIDTH) * 0.5f);
            nkF32 ty = NK_CAST(nkF32, tile.y * TILE_HEIGHT) + (NK_CAST(nkF32,TILE_HEIGHT) * 0.5f);

            imm_texture(highlight, tx,ty);
        }
    }

    // Unset the camera so that we render in screen-space for the HUD.
    imm_set_view(nk_m4_identity());

    // Draw the hotbar.
    Texture hotbar = asset_manager_load<Texture>("hotbar.png");
    nkF32 hx = roundf(NK_CAST(nkF32, get_texture_width(hotbar)) * 0.5f);
    nkF32 hy = roundf(NK_CAST(nkF32, get_texture_height(hotbar)) * 0.5f);
    imm_texture(hotbar, hx,hy);

    nkF32 ix = 24.0f;
    nkF32 iy = 24.0f;

    for(nkS32 i=0,n=NK_ARRAY_SIZE(g_controller.hotbar); i<n; ++i)
    {
        Texture icon = NULL;

        switch(g_controller.hotbar[i])
        {
            case PlantID_Flower: icon = asset_manager_load<Texture>("flower.png"); break;
        }

        if(icon)
        {
            imm_texture(icon, ix,iy);

            if(g_controller.selected == g_controller.hotbar[i])
            {
                Texture highlight = asset_manager_load<Texture>("highlight.png");
                imm_texture(highlight, ix,iy);
            }
        }

        ix += 40.0f;
    }

    // Draw the cursor.
    Texture cursor = asset_manager_load<Texture>("cursor.png");
    nkF32 cx = g_controller.cursor_pos.x + (NK_CAST(nkF32, get_texture_width(cursor) * 0.5f)) - 4;
    nkF32 cy = g_controller.cursor_pos.y + (NK_CAST(nkF32, get_texture_height(cursor) * 0.5f)) - 4;
    imm_texture(cursor, cx,cy);
}

GLOBAL void set_controller_camera(void)
{
    // Setup the view matrix using the controller camera.
    nkF32 cx = g_controller.camera_pos.x - (NK_CAST(nkF32, SCREEN_WIDTH) * 0.5f);
    nkF32 cy = g_controller.camera_pos.y - (NK_CAST(nkF32, SCREEN_HEIGHT) * 0.5f);

    nkMat4 camera_matrix = nk_translate(nk_m4_identity(), { -cx,-cy,0.0f });
    imm_set_view(camera_matrix);
}

/*////////////////////////////////////////////////////////////////////////////*/
