/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkS32 STARTING_MONEY = 1200;

struct Controller
{
    PlantSpawn hotbar[8];
    nkU32      selected;

    nkS32 money;

    nkVec2 camera_pos;
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

INTERNAL void place_plant(nkS32 tile_x, nkS32 tile_y)
{
    // If the tile position is out of the world bounds then the spawn isn't possible.
    if(tile_x < 0 || tile_x >= g_world.width || tile_y < 0 || tile_y >= g_world.height) return;
    // If nothing is selected then there's nothing to place.
    if(g_controller.selected == NO_SELECTION) return;

    EntityID id = g_controller.hotbar[g_controller.selected].id;

    const EntityDesc& desc = ENTITY_TABLE[id];

    // Determine the bounds of the plant and check if the spot is occupied.
    nkF32 x = NK_CAST(nkF32, tile_x * TILE_WIDTH);
    nkF32 y = NK_CAST(nkF32, tile_y * TILE_HEIGHT);
    nkF32 w = desc.bounds.x;
    nkF32 h = desc.bounds.y;

    if(check_entity_collision(x,y,w,h, EntityType_Plant) != NK_U64_MAX)
    {
        return; // A plant is already at this position.
    }

    // If we get here we can place!

    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_controller.shovel_sfx)-1);
    play_sound(g_controller.shovel_sfx[sound_index]);

    x += (desc.bounds.x * 0.5f);
    y += (desc.bounds.y * 0.5f);

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
    nkU64 entity_index = check_entity_collision(x,y,1,1, EntityType_Plant);
    if(entity_index == NK_U64_MAX) return; // Nothing at the spot to remove.

    EntityID entity_id = g_world.entities[entity_index].id;

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
    g_controller.camera_pos.x = NK_CAST(nkF32, (g_world.width * TILE_WIDTH)) * 0.5f;
    g_controller.camera_pos.y = NK_CAST(nkF32, (g_world.height * TILE_HEIGHT)) * 0.5f;

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

    TrueTypeFontDesc font_desc;
    font_desc.px_sizes = { 10, 20 };

    g_controller.font = asset_manager_load<TrueTypeFont>("helsinki.ttf", &font_desc);
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
    nkF32 hw = NK_CAST(nkF32, get_texture_width(g_controller.hotbar_tex));
    nkF32 hh = NK_CAST(nkF32, get_texture_height(g_controller.hotbar_tex));

    g_controller.occluded = point_vs_rect(g_controller.cursor_pos, 0.0f,0.0f,hw,hh);

    // Select the hovered plant/tool.
    if(g_controller.occluded && is_mouse_button_pressed(MouseButton_Left))
    {
        nkF32 ix = 8.0f;
        nkF32 iy = 8.0f;

        for(nkS32 i=0,n=NK_ARRAY_SIZE(g_controller.hotbar); i<n; ++i)
        {
            if(point_vs_rect(g_controller.cursor_pos, ix,iy,32.0f,32.0f))
            {
                const PlantSpawn& spawn = g_controller.hotbar[i];
                if(g_controller.money >= spawn.cost)
                {
                    g_controller.selected = i;
                    g_controller.watering = NK_FALSE;
                    g_controller.removing = NK_FALSE;
                }
            }

            ix += 40.0f;
        }

        ix += 8.0f;
        if(point_vs_rect(g_controller.cursor_pos, ix,iy,32.0f,32.0f))
        {
            g_controller.selected = NO_SELECTION;
            g_controller.watering = !g_controller.watering;
            g_controller.removing = NK_FALSE;
        }

        ix += 40.0f;
        if(point_vs_rect(g_controller.cursor_pos, ix,iy,32.0f,32.0f))
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
        nkF32 cx = g_controller.cursor_pos.x + (g_controller.camera_pos.x - (NK_CAST(nkF32, SCREEN_WIDTH) * 0.5f));
        nkF32 cy = g_controller.cursor_pos.y + (g_controller.camera_pos.y - (NK_CAST(nkF32, SCREEN_HEIGHT) * 0.5f));

        if(g_controller.selected != NO_SELECTION)
        {
            const PlantSpawn& spawn = g_controller.hotbar[g_controller.selected];
            if(g_controller.money >= spawn.cost)
            {
                iPoint tile;

                tile.x = NK_CAST(nkS32, cx / TILE_WIDTH);
                tile.y = NK_CAST(nkS32, cy / TILE_HEIGHT);

                place_plant(tile.x, tile.y);
            }
        }
        if(g_controller.watering)
        {
            water_plant(cx, cy);
        }
        if(g_controller.removing)
        {
            remove_plant(cx, cy);
        }
    }
}

GLOBAL void controller_draw(void)
{
    // Draw the highlighted tile if not panning.
    if(!g_controller.panning && !g_controller.occluded && (g_controller.selected != NO_SELECTION || g_controller.watering || g_controller.removing))
    {
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

            imm_texture(g_controller.highlight_tex, tx,ty);
        }
    }

    // Unset the camera so that we render in screen-space for the HUD.
    imm_set_view(nk_m4_identity());

    // Draw the hotbar.
    nkF32 hx = roundf(NK_CAST(nkF32, get_texture_width(g_controller.hotbar_tex)) * 0.5f);
    nkF32 hy = roundf(NK_CAST(nkF32, get_texture_height(g_controller.hotbar_tex)) * 0.5f);

    imm_texture(g_controller.hotbar_tex, hx,hy);

    nkF32 ix = 24.0f;
    nkF32 iy = 24.0f;

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

            ImmClip clip = { NK_CAST(nkF32, i * 32), 0.0f, 32.0f, 32.0f };
            imm_texture(g_controller.icons_tex, ix,iy, &clip, color);

            // Draw the cost of the plant.
            set_truetype_font_size(g_controller.font, 10);
            nkString string;
            number_to_string_with_commas(&string, spawn.cost);
            nkF32 text_x = ix - 16.0f;
            nkF32 text_y = iy + 16.0f;
            draw_truetype_text(g_controller.font, text_x+2,text_y+2, string.cstr, NK_V4_BLACK * color);
            draw_truetype_text(g_controller.font, text_x,text_y, string.cstr, NK_V4_WHITE * color);
        }

        ix += 40.0f;
    }

    ix += 8.0f;
    imm_texture(g_controller.watercan_tex, ix,iy);
    ix += 40.0f;
    imm_texture(g_controller.shovel_tex, ix,iy);

    // Draw the money counter.
    set_truetype_font_size(g_controller.font, 20);
    nkString string = "$";
    number_to_string_with_commas(&string, g_controller.money);
    nkF32 text_x = 4.0f;
    nkF32 text_y = get_texture_height(g_controller.hotbar_tex) + get_truetype_line_height(g_controller.font);
    draw_truetype_text(g_controller.font, text_x+2,text_y+2, string.cstr, NK_V4_BLACK);
    draw_truetype_text(g_controller.font, text_x,text_y, string.cstr, NK_V4_WHITE);

    // Draw the cursor.
    if(g_controller.watering)
    {
        nkF32 cx = g_controller.cursor_pos.x;
        nkF32 cy = g_controller.cursor_pos.y;
        imm_texture(g_controller.watercan_tex, cx,cy);
    }
    else if(g_controller.removing)
    {
        nkF32 cx = g_controller.cursor_pos.x;
        nkF32 cy = g_controller.cursor_pos.y;
        imm_texture(g_controller.shovel_tex, cx,cy);
    }
    else if(g_controller.selected != NO_SELECTION)
    {
        ImmClip clip = { NK_CAST(nkF32, g_controller.selected * 32), 0.0f, 32.0f, 32.0f };
        nkF32 cx = g_controller.cursor_pos.x;
        nkF32 cy = g_controller.cursor_pos.y;
        imm_texture(g_controller.icons_tex, cx,cy, &clip);
    }
    else
    {
        nkF32 cx = g_controller.cursor_pos.x + (NK_CAST(nkF32, get_texture_width(g_controller.cursor_tex) * 0.5f)) - 4;
        nkF32 cy = g_controller.cursor_pos.y + (NK_CAST(nkF32, get_texture_height(g_controller.cursor_tex) * 0.5f)) - 4;
        imm_texture(g_controller.cursor_tex, cx,cy);
    }
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
