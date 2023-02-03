/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkS32 STARTING_MONEY = 20000;

INTERNAL constexpr nkF32 ICON_WIDTH  = 128.0f;
INTERNAL constexpr nkF32 ICON_HEIGHT = 160.0f;

INTERNAL constexpr nkF32 CAMERA_START_ZOOM       = 0.50f;
INTERNAL constexpr nkF32 CAMERA_MIN_ZOOM         = 0.25f;
INTERNAL constexpr nkF32 CAMERA_MAX_ZOOM         = 1.00f;
INTERNAL constexpr nkF32 CAMERA_ZOOM_SENSITIVITY = 0.15f;
INTERNAL constexpr nkF32 CAMERA_ZOOM_SPEED       = 12.0f;
INTERNAL constexpr nkF32 CAMERA_PAN_SPEED        = 12.0f;

struct Controller
{
    PlantSpawn hotbar[8];
    nkU32      selected;

    nkF32 hud_scale;

    nkS32 money;

    nkVec2 camera_current_pos;
    nkVec2 camera_target_pos;
    nkF32  camera_current_zoom;
    nkF32  camera_target_zoom;
    nkMat4 camera_proj;
    nkMat4 camera_view;

    nkVec2 cursor_pos;
    nkBool panning;
    nkBool occluded;
    nkBool watering;
    nkBool removing;

    // Assets
    Texture hotbar_tex;
    Texture highlight_tex;
    Texture icons_tex;
    Texture watercan_tex;
    Texture shovel_tex;
    Texture cursor_tex;

    Sound shovel_sfx[5];

    TrueTypeFont font;
};

INTERNAL Controller g_controller;

INTERNAL nkVec2 screen_to_world(nkVec2 screen)
{
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkMat4 proj = g_controller.camera_proj;
    nkMat4 view = g_controller.camera_view;

    nkVec4 coord = { screen.x, fabsf(screen.y - wh), 0.0f, 1.0f }; // We use inverted y-axis.

    nkMat4 inverse = nk_inverse(proj * view);

    coord.x = coord.x / ww;
    coord.y = coord.y / wh;
    coord.x = coord.x * 2.0f - 1.0f;
    coord.y = coord.y * 2.0f - 1.0f;

    nkVec4 world = inverse * coord;

    world /= world.w;

    return { world.x, world.y };
}

// https://stackoverflow.com/a/1449859
INTERNAL void number_to_string_with_commas(nkString* str, nkS32 number)
{
    NK_ASSERT(str);

    if(number < 0)
    {
        nk_string_append(str, '-');
        number_to_string_with_commas(str, -number);
    }
    else if(number < 1000)
    {
        nkString append = format_string("%d", number);
        nk_string_append(str, &append);
        return;
    }
    else
    {
        number_to_string_with_commas(str, number/1000);
        nkString append = format_string(",%03d", number % 1000);
        nk_string_append(str, &append);
    }
}

INTERNAL nkBool can_place_plant_at_position(nkS32 tile_x, nkS32 tile_y)
{
    // If the tile position is out of the world bounds then the spawn isn't possible.
    if(tile_x < 0 || tile_x >= get_world_width() || tile_y < 0 || tile_y >= get_world_height()) return NK_FALSE;
    // If nothing is selected then there's nothing to place.
    if(g_controller.selected == NO_SELECTION) return NK_FALSE;

    EntityID id = g_controller.hotbar[g_controller.selected].id;

    const EntityDesc& desc = ENTITY_TABLE[id];

    // Determine the bounds of the plant and check if the spot is occupied.
    nkF32 x = NK_CAST(nkF32, tile_x * TILE_WIDTH);
    nkF32 y = NK_CAST(nkF32, tile_y * TILE_HEIGHT);
    nkF32 w = desc.bounds.x * TILE_WIDTH;
    nkF32 h = desc.bounds.y * TILE_HEIGHT;

    if(check_entity_bounds(x,y,w,h, EntityType_Plant|EntityType_Base|EntityType_Object) != NK_U64_MAX)
    {
        return NK_FALSE; // A plant or object is already at this position.
    }

    return NK_TRUE;
}

INTERNAL void place_plant(nkS32 tile_x, nkS32 tile_y)
{
    if(!can_place_plant_at_position(tile_x, tile_y)) return;

    EntityID id = g_controller.hotbar[g_controller.selected].id;

    const EntityDesc& desc = ENTITY_TABLE[id];

    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_controller.shovel_sfx)-1);
    play_sound(g_controller.shovel_sfx[sound_index]);

    nkF32 x = NK_CAST(nkF32, tile_x * TILE_WIDTH) + ((desc.bounds.x * TILE_WIDTH) * 0.5f);
    nkF32 y = NK_CAST(nkF32, tile_y * TILE_HEIGHT) + ((desc.bounds.y * TILE_HEIGHT) * 0.5f);

    entity_spawn(id, x,y);

    // If we no longer have enough money to purchase another, just de-select it.
    nkS32 cost = g_controller.hotbar[g_controller.selected].cost;
    g_controller.money -= cost;
    if(g_controller.money < cost)
    {
        g_controller.selected = NO_SELECTION;
    }
}

INTERNAL void remove_plant(nkF32 x, nkF32 y)
{
    nkU64 entity_index = check_entity_bounds(x,y,1,1, EntityType_Plant);
    if(entity_index == NK_U64_MAX) return; // Nothing at the spot to remove.

    EntityID entity_id = get_entity(entity_index)->id;

    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_controller.shovel_sfx)-1);
    play_sound(g_controller.shovel_sfx[sound_index]);

    entity_kill(entity_index);

    // Get back some of the money spent on the entity.
    for(nkU64 i=0,n=NK_ARRAY_SIZE(g_controller.hotbar); i<n; ++i)
    {
        const PlantSpawn& spawn = g_controller.hotbar[i];
        if(spawn.id == entity_id)
        {
            g_controller.money += (spawn.cost / 2);
            break;
        }
    }
}

INTERNAL void water_plant(nkF32 x, nkF32 y)
{
    // @Incomplete: Does nothing currently...
}

GLOBAL void controller_init(void)
{
    // Center the camera on the map.
    nkF32 center_x = NK_CAST(nkF32, (get_world_width() * TILE_WIDTH)) * 0.5f;
    nkF32 center_y = NK_CAST(nkF32, (get_world_height() * TILE_HEIGHT)) * 0.5f;

    g_controller.camera_current_pos = { center_x, center_y };
    g_controller.camera_target_pos  = { center_x, center_y };

    g_controller.camera_current_zoom = CAMERA_START_ZOOM;
    g_controller.camera_target_zoom  = CAMERA_START_ZOOM;

    // @Incomplete: Hook up a way to change this (or do it dynamically)?
    // Allow the HUD to scale so that it is clearly visible on large displays.
    g_controller.hud_scale = 1.5f;

    g_controller.money = STARTING_MONEY;

    // @Incomplete: Just giving some plants for testing.
    g_controller.hotbar[0] = { EntityID_Daisy,  100 };
    g_controller.hotbar[1] = { EntityID_Bramble, 50 };

    g_controller.selected = NO_SELECTION;

    // Pre-load a bunch of assets.
    g_controller.hotbar_tex    = asset_manager_load<Texture>("hotbar.png");
    g_controller.icons_tex     = asset_manager_load<Texture>("hotbar_icons.png");
    g_controller.highlight_tex = asset_manager_load<Texture>("highlight.png");
    g_controller.watercan_tex  = asset_manager_load<Texture>("tool_water.png");
    g_controller.shovel_tex    = asset_manager_load<Texture>("tool_shovel.png");
    g_controller.cursor_tex    = asset_manager_load<Texture>("cursor.png");

    g_controller.shovel_sfx[0] = asset_manager_load<Sound>("shovel_000.wav");
    g_controller.shovel_sfx[1] = asset_manager_load<Sound>("shovel_001.wav");
    g_controller.shovel_sfx[2] = asset_manager_load<Sound>("shovel_002.wav");
    g_controller.shovel_sfx[3] = asset_manager_load<Sound>("shovel_003.wav");
    g_controller.shovel_sfx[4] = asset_manager_load<Sound>("shovel_004.wav");

    g_controller.font = asset_manager_load<TrueTypeFont>("helsinki.ttf");
}

GLOBAL void controller_tick(nkF32 dt)
{
    // Convert the raw mouse position into screen position for the cursor.
    g_controller.cursor_pos = get_window_mouse_pos();

    // Pan the camera around the world.
    g_controller.panning = is_mouse_button_down(MouseButton_Middle);
    if(g_controller.panning)
    {
        g_controller.camera_target_pos -= (get_relative_mouse_pos() / g_controller.camera_current_zoom);
        g_controller.camera_target_pos.x = nk_clamp(g_controller.camera_target_pos.x, 0.0f, get_world_width() * TILE_WIDTH);
        g_controller.camera_target_pos.y = nk_clamp(g_controller.camera_target_pos.y, 0.0f, get_world_height() * TILE_HEIGHT);

    }

    g_controller.camera_current_pos = nk_lerp(g_controller.camera_current_pos, g_controller.camera_target_pos, CAMERA_PAN_SPEED * dt);

    // Zoom the camera in and out.
    nkVec2 mouse_wheel = get_mouse_wheel();
    if(mouse_wheel.y != 0.0f)
    {
        g_controller.camera_target_zoom += (mouse_wheel.y * CAMERA_ZOOM_SENSITIVITY) * g_controller.camera_target_zoom; // Multiply by the current zoom for more even increments.
        g_controller.camera_target_zoom = nk_clamp(g_controller.camera_target_zoom, CAMERA_MIN_ZOOM, CAMERA_MAX_ZOOM);
    }

    g_controller.camera_current_zoom = nk_lerp(g_controller.camera_current_zoom, g_controller.camera_target_zoom, CAMERA_ZOOM_SPEED * dt);

    // Images need to be scaled down because they are actually at the max scale initially and scaled down for lower scales.
    // Other elements like text and general positioning need to still be scaled up, so we have these two variables to do it.
    nkF32 img_scale = g_controller.hud_scale / 4.0f;
    nkF32 hud_scale = g_controller.hud_scale;

    // Check if the cursor is occluded from the world by the HUD.
    nkF32 hw = NK_CAST(nkF32, get_texture_width(g_controller.hotbar_tex)) * img_scale;
    nkF32 hh = NK_CAST(nkF32, get_texture_height(g_controller.hotbar_tex)) * img_scale;

    g_controller.occluded = point_vs_rect(g_controller.cursor_pos, 0.0f,0.0f,hw,hh);

    // Select the hovered plant/tool.
    if(g_controller.occluded && is_mouse_button_pressed(MouseButton_Left))
    {
        nkF32 ix = 8.0f * hud_scale;
        nkF32 iy = 8.0f * hud_scale;

        for(nkS32 i=0,n=NK_ARRAY_SIZE(g_controller.hotbar); i<n; ++i)
        {
            if(point_vs_rect(g_controller.cursor_pos, ix,iy,32.0f*hud_scale,32.0f*hud_scale))
            {
                const PlantSpawn& spawn = g_controller.hotbar[i];
                if(g_controller.money >= spawn.cost)
                {
                    g_controller.selected = i;
                    g_controller.watering = NK_FALSE;
                    g_controller.removing = NK_FALSE;
                }
            }

            ix += 40.0f * hud_scale;
        }

        ix += 8.0f * hud_scale;
        if(point_vs_rect(g_controller.cursor_pos, ix,iy,32.0f*hud_scale,32.0f*hud_scale))
        {
            g_controller.selected = NO_SELECTION;
            g_controller.watering = !g_controller.watering;
            g_controller.removing = NK_FALSE;
        }

        ix += 40.0f * hud_scale;
        if(point_vs_rect(g_controller.cursor_pos, ix,iy,32.0f*hud_scale,32.0f*hud_scale))
        {
            g_controller.selected = NO_SELECTION;
            g_controller.watering = NK_FALSE;
            g_controller.removing = !g_controller.removing;
        }
    }

    // De-select current plant/tool.
    if(is_mouse_button_pressed(MouseButton_Right))
    {
        g_controller.selected = NO_SELECTION;
        g_controller.watering = NK_FALSE;
        g_controller.removing = NK_FALSE;
    }

    // Place current plant / perform the current tool action.
    if(!g_controller.occluded && !g_controller.panning && is_mouse_button_pressed(MouseButton_Left))
    {
        nkVec2 pos = screen_to_world(g_controller.cursor_pos);

        if(g_controller.selected != NO_SELECTION)
        {
            const PlantSpawn& spawn = g_controller.hotbar[g_controller.selected];
            if(g_controller.money >= spawn.cost)
            {
                iPoint tile;

                tile.x = NK_CAST(nkS32, floorf(pos.x / NK_CAST(nkF32, TILE_WIDTH)));
                tile.y = NK_CAST(nkS32, floorf(pos.y / NK_CAST(nkF32, TILE_HEIGHT)));

                place_plant(tile.x, tile.y);
            }
        }
        if(g_controller.watering)
        {
            water_plant(pos.x, pos.y);
        }
        if(g_controller.removing)
        {
            remove_plant(pos.x, pos.y);
        }
    }
}

GLOBAL void controller_draw(void)
{
    // Draw the highlighted tile if not panning.
    if(!g_controller.panning && !g_controller.occluded && (g_controller.selected != NO_SELECTION || g_controller.watering || g_controller.removing))
    {
        nkVec2 pos = screen_to_world(g_controller.cursor_pos);

        iPoint tile;

        tile.x = NK_CAST(nkS32, floorf(pos.x / (NK_CAST(nkF32, TILE_WIDTH))));
        tile.y = NK_CAST(nkS32, floorf(pos.y / (NK_CAST(nkF32, TILE_HEIGHT))));

        // Make sure we can place at the spot.
        if(can_place_plant_at_position(tile.x, tile.y))
        {
            nkF32 tx = NK_CAST(nkF32, tile.x * TILE_WIDTH) + (NK_CAST(nkF32,TILE_WIDTH) * 0.5f);
            nkF32 ty = NK_CAST(nkF32, tile.y * TILE_HEIGHT) + (NK_CAST(nkF32,TILE_HEIGHT) * 0.5f);

            imm_texture(g_controller.highlight_tex, tx,ty);
        }
    }

    // Unset the camera so that we render in screen-space for the HUD.
    unset_controller_camera();

    // Images need to be scaled down because they are actually at the max scale initially and scaled down for lower scales.
    // Other elements like text and general positioning need to still be scaled up, so we have these two variables to do it.
    nkF32 img_scale = g_controller.hud_scale / 4.0f;
    nkF32 hud_scale = g_controller.hud_scale;

    // Draw the hotbar.
    nkF32 hx = roundf((NK_CAST(nkF32, get_texture_width(g_controller.hotbar_tex)) * 0.5f) * img_scale);
    nkF32 hy = roundf((NK_CAST(nkF32, get_texture_height(g_controller.hotbar_tex)) * 0.5f) * img_scale);

    imm_texture_ex(g_controller.hotbar_tex, hx,hy, img_scale,img_scale, 0.0f, NULL);

    nkF32 ix = 24.0f * hud_scale;
    nkF32 iy = 24.0f * hud_scale;

    for(nkS32 i=0,n=NK_ARRAY_SIZE(g_controller.hotbar); i<n; ++i)
    {
        if(g_controller.hotbar[i].id != EntityID_None)
        {
            const PlantSpawn& spawn = g_controller.hotbar[i];

            nkVec4 color = NK_V4_WHITE;
            if(g_controller.money < spawn.cost)
            {
                color *= 0.5f;
            }

            ImmClip clip = { NK_CAST(nkF32, i) * ICON_WIDTH, 0.0f, ICON_WIDTH, ICON_HEIGHT };
            imm_texture_ex(g_controller.icons_tex, ix,iy, img_scale,img_scale, 0.0f, NULL, &clip, color);

            // Draw the cost of the plant.
            set_truetype_font_size(g_controller.font, NK_CAST(nkS32, 10 * hud_scale));
            nkString string;
            number_to_string_with_commas(&string, spawn.cost);
            nkF32 text_x = ix - (16.0f * hud_scale);
            nkF32 text_y = iy + (16.0f * hud_scale);
            draw_truetype_text(g_controller.font, text_x+(2*hud_scale),text_y+(2*hud_scale), string.cstr, NK_V4_BLACK * color);
            draw_truetype_text(g_controller.font, text_x,text_y, string.cstr, NK_V4_WHITE * color);
        }

        ix += 40.0f * hud_scale;
    }

    ix += 8.0f * hud_scale;
    imm_texture_ex(g_controller.watercan_tex, ix,iy, img_scale,img_scale, 0.0f, NULL);
    ix += 40.0f * hud_scale;
    imm_texture_ex(g_controller.shovel_tex, ix,iy, img_scale,img_scale, 0.0f, NULL);

    // Draw the money counter.
    set_truetype_font_size(g_controller.font, NK_CAST(nkS32, 20 * hud_scale));
    nkString string = "$";
    number_to_string_with_commas(&string, g_controller.money);
    nkF32 text_x = 4.0f * hud_scale;
    nkF32 text_y = (get_texture_height(g_controller.hotbar_tex) * img_scale) + get_truetype_line_height(g_controller.font);
    draw_truetype_text(g_controller.font, text_x+(2*hud_scale),text_y+(2*hud_scale), string.cstr, NK_V4_BLACK);
    draw_truetype_text(g_controller.font, text_x,text_y, string.cstr, NK_V4_WHITE);

    // Draw the cursor.
    if(g_controller.watering)
    {
        nkF32 cx = g_controller.cursor_pos.x;
        nkF32 cy = g_controller.cursor_pos.y;
        imm_texture_ex(g_controller.watercan_tex, cx,cy, img_scale,img_scale, 0.0f, NULL);
    }
    else if(g_controller.removing)
    {
        nkF32 cx = g_controller.cursor_pos.x;
        nkF32 cy = g_controller.cursor_pos.y;
        imm_texture_ex(g_controller.shovel_tex, cx,cy, img_scale,img_scale, 0.0f, NULL);
    }
    else if(g_controller.selected != NO_SELECTION)
    {
        ImmClip clip = { NK_CAST(nkF32, g_controller.selected) * ICON_WIDTH, 0.0f, ICON_WIDTH, ICON_HEIGHT };
        nkF32 cx = g_controller.cursor_pos.x;
        nkF32 cy = g_controller.cursor_pos.y;
        imm_texture_ex(g_controller.icons_tex, cx,cy, img_scale,img_scale, 0.0f, NULL, &clip);
    }
    else
    {
        nkF32 cx = g_controller.cursor_pos.x + (NK_CAST(nkF32, get_texture_width(g_controller.cursor_tex) * 0.5f) * img_scale) - (4 * hud_scale);
        nkF32 cy = g_controller.cursor_pos.y + (NK_CAST(nkF32, get_texture_height(g_controller.cursor_tex) * 0.5f) * img_scale) - (4 * hud_scale);
        imm_texture_ex(g_controller.cursor_tex, cx,cy, img_scale,img_scale, 0.0f, NULL);
    }
}

GLOBAL void set_controller_camera(void)
{
    // Setup the camera matrices using the controller camera.
    nkF32 l = -(NK_CAST(nkF32, get_window_width ()) * 0.5f);
    nkF32 r =  (NK_CAST(nkF32, get_window_width ()) * 0.5f);
    nkF32 b =  (NK_CAST(nkF32, get_window_height()) * 0.5f);
    nkF32 t = -(NK_CAST(nkF32, get_window_height()) * 0.5f);

    nkF32 cx = g_controller.camera_current_pos.x;
    nkF32 cy = g_controller.camera_current_pos.y;

    nkF32 cs = g_controller.camera_current_zoom;

    g_controller.camera_proj = nk_orthographic(l,r,b,t);
    g_controller.camera_view = nk_m4_identity();

    g_controller.camera_view = nk_scale    (g_controller.camera_view, {  cs, cs, 1.0f });
    g_controller.camera_view = nk_translate(g_controller.camera_view, { -cx,-cy, 0.0f });

    imm_set_projection(g_controller.camera_proj);
    imm_set_view(g_controller.camera_view);
}

GLOBAL void unset_controller_camera(void)
{
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    imm_set_projection(nk_orthographic(0.0f,ww,wh,0.0f));
    imm_set_view(nk_m4_identity());
}

/*////////////////////////////////////////////////////////////////////////////*/
