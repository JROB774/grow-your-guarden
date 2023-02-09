/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkU32 NO_SELECTION = NK_U32_MAX;

INTERNAL constexpr nkS32 STARTING_MONEY = 750;

INTERNAL constexpr nkF32 TOOLTIP_PADDING = 5.0f;

INTERNAL constexpr nkF32 PANNING_SPEED = 10.0f * TILE_WIDTH;

INTERNAL constexpr nkF32 CAMERA_START_ZOOM       = 0.50f;
INTERNAL constexpr nkF32 CAMERA_MIN_ZOOM         = 0.25f;
INTERNAL constexpr nkF32 CAMERA_MAX_ZOOM         = 1.00f;
INTERNAL constexpr nkF32 CAMERA_ZOOM_SENSITIVITY = 0.15f;
INTERNAL constexpr nkF32 CAMERA_ZOOM_SPEED       = 12.0f;
INTERNAL constexpr nkF32 CAMERA_PAN_SPEED        = 12.0f;

INTERNAL constexpr nkF32 FERTILIZE_TIME = 60.0f * 2.0f;

NK_ENUM(HotbarID, nkU32)
{
    HotbarID_Daisy,
    HotbarID_Bramble,
    HotbarID_HedgeWall,
    HotbarID_BellPlant,
    HotbarID_Fertilizer,
    HotbarID_Shovel,
    HotbarID_Bell,
    HotbarID_TOTAL
};

struct HotbarSlot
{
    const nkChar* name;
    const nkChar* description;
    nkS32         cost;
    nkU32         unlock;   // How many waves need to be beaten for the slot to become unlocked.
    EntityID      spawn_id; // Some slots will not care about this (such as the tool/item slots).
};

struct Controller
{
    HotbarSlot hotbar[HotbarID_TOTAL];

    nkU32 hovered;
    nkU32 selected;

    nkS32  money_counter; // This is used as the visual for money (for smooth interpolation).
    nkVec4 money_color;

    nkS32 money;
    nkS32 health;
    nkS32 kills;

    nkVec2 camera_current_pos;
    nkVec2 camera_target_pos;
    nkF32  camera_current_zoom;
    nkF32  camera_target_zoom;
    nkMat4 camera_proj;
    nkMat4 camera_view;

    nkBool panning;

    Sound shovel_sfx[5];
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
    // If the tile is not grass then we cannot place a plant on it.
    if(get_tile_id(tile_x, tile_y) != TileID_Grass) return NK_FALSE;

    EntityID id = g_controller.hotbar[g_controller.selected].spawn_id;
    if(id == EntityID_None) return NK_FALSE;

    const EntityDesc& desc = ENTITY_TABLE[id];

    // Determine the bounds of the plant and check if the spot is occupied.
    nkF32 x = NK_CAST(nkF32, tile_x * TILE_WIDTH);
    nkF32 y = NK_CAST(nkF32, tile_y * TILE_HEIGHT);
    nkF32 w = desc.bounds.x * TILE_WIDTH;
    nkF32 h = desc.bounds.y * TILE_HEIGHT;

    if(check_entity_bounds(x,y,w,h, EntityType_Plant|EntityType_Base) != NK_U64_MAX)
    {
        return NK_FALSE; // A plant or the base is already at this position.
    }

    return NK_TRUE;
}

INTERNAL nkBool can_remove_plant_at_position(nkF32 x, nkF32 y)
{
    return (g_controller.selected == HotbarID_Shovel && (check_entity_bounds(x,y,1,1, EntityType_Plant) != NK_U64_MAX));
}

INTERNAL nkBool can_fertilize_plant_at_position(nkF32 x, nkF32 y)
{
    if(g_controller.selected != HotbarID_Fertilizer) return NK_FALSE;

    nkU64 entity_index = check_entity_bounds(x,y,1,1, EntityType_Plant);
    if(entity_index == NK_U64_MAX) return NK_FALSE; // Nothing at the spot to fertilize.

    // Special cases for some plants that can be fertilized when not fully grown.
    Entity* entity = get_entity(entity_index);
    if(!entity) return NK_FALSE;

    switch(entity->id)
    {
        case EntityID_Bramble: return NK_TRUE;
        case EntityID_HedgeWall: return NK_TRUE;
    }

    return plant_is_fully_grown(*entity);
}

INTERNAL nkBool place_plant(nkS32 tile_x, nkS32 tile_y)
{
    if(!can_place_plant_at_position(tile_x, tile_y)) return NK_FALSE;

    EntityID id = g_controller.hotbar[g_controller.selected].spawn_id;
    if(id == EntityID_None) return NK_FALSE;

    const EntityDesc& desc = ENTITY_TABLE[id];

    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_controller.shovel_sfx)-1);
    play_sound(g_controller.shovel_sfx[sound_index]);

    nkF32 x = NK_CAST(nkF32, tile_x * TILE_WIDTH) + ((desc.bounds.x * TILE_WIDTH) * 0.5f);
    nkF32 y = NK_CAST(nkF32, tile_y * TILE_HEIGHT) + ((desc.bounds.y * TILE_HEIGHT) * 0.5f);

    entity_spawn(id, x,y);

    return NK_TRUE;
}

INTERNAL nkBool remove_plant(nkF32 x, nkF32 y)
{
    nkU64 entity_index = check_entity_bounds(x,y,1,1, EntityType_Plant);
    if(entity_index == NK_U64_MAX) return NK_FALSE; // Nothing at the spot to remove.

    EntityID entity_id = get_entity(entity_index)->id;

    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_controller.shovel_sfx)-1);
    play_sound(g_controller.shovel_sfx[sound_index]);

    entity_kill(entity_index);

    // Get back some of the money spent on the entity.
    for(nkU64 i=0,n=NK_ARRAY_SIZE(g_controller.hotbar); i<n; ++i)
    {
        const HotbarSlot& slot = g_controller.hotbar[i];
        if(slot.spawn_id == entity_id)
        {
            add_money(slot.cost / 2);
            break;
        }
    }

    return NK_TRUE;
}

INTERNAL nkBool fertilize_plant(nkF32 x, nkF32 y)
{
    if(!can_fertilize_plant_at_position(x,y)) return NK_FALSE;

    nkU64 entity_index = check_entity_bounds(x,y,1,1, EntityType_Plant);
    Entity* entity = get_entity(entity_index);

    entity->fertilized_timer = FERTILIZE_TIME;
    entity->bounce_timer = 0.1f;

    play_sound(asset_manager_load<Sound>("fertilized.wav"));

    return NK_TRUE;
}

GLOBAL void controller_init(void)
{
    // Pre-load a bunch of assets.
    g_controller.shovel_sfx[0] = asset_manager_load<Sound>("shovel_000.wav");
    g_controller.shovel_sfx[1] = asset_manager_load<Sound>("shovel_001.wav");
    g_controller.shovel_sfx[2] = asset_manager_load<Sound>("shovel_002.wav");
    g_controller.shovel_sfx[3] = asset_manager_load<Sound>("shovel_003.wav");
    g_controller.shovel_sfx[4] = asset_manager_load<Sound>("shovel_004.wav");

    asset_manager_load<Sound>("fertilized.wav");
    asset_manager_load<Sound>("bell.wav");

    asset_manager_load<Texture>("hud.png");

    // Fill out the hotbar slot information.
    g_controller.hotbar[HotbarID_Daisy     ].name        = "DAISY";
    g_controller.hotbar[HotbarID_Daisy     ].description = "Fires shots at nearby enemies.";
    g_controller.hotbar[HotbarID_Daisy     ].cost        = 100;
    g_controller.hotbar[HotbarID_Daisy     ].unlock      = 0;
    g_controller.hotbar[HotbarID_Daisy     ].spawn_id    = EntityID_Daisy;

    g_controller.hotbar[HotbarID_Bramble   ].name        = "BRAMBLE";
    g_controller.hotbar[HotbarID_Bramble   ].description = "Damages enemies that walk over it.\nDamage increases as it grows.";
    g_controller.hotbar[HotbarID_Bramble   ].cost        = 75;
    g_controller.hotbar[HotbarID_Bramble   ].unlock      = 0; // @Incomplete!
    g_controller.hotbar[HotbarID_Bramble   ].spawn_id    = EntityID_Bramble;

    g_controller.hotbar[HotbarID_HedgeWall ].name        = "HEDGE WALL";
    g_controller.hotbar[HotbarID_HedgeWall ].description = "A defensive wall-like plant.\nDoes not attack.\nGets stronger as it grows.";
    g_controller.hotbar[HotbarID_HedgeWall ].cost        = 200;
    g_controller.hotbar[HotbarID_HedgeWall ].unlock      = 0; // @Incomplete!
    g_controller.hotbar[HotbarID_HedgeWall ].spawn_id    = EntityID_HedgeWall;

    g_controller.hotbar[HotbarID_BellPlant ].name        = "MISSILE BELL";
    g_controller.hotbar[HotbarID_BellPlant ].description = "Fires anti-air homing projectiles at flying enemies.\nCannot attack ground enemies.";
    g_controller.hotbar[HotbarID_BellPlant ].cost        = 150;
    g_controller.hotbar[HotbarID_BellPlant ].unlock      = 0; // @Incomplete!
    g_controller.hotbar[HotbarID_BellPlant ].spawn_id    = EntityID_BellPlant;

    g_controller.hotbar[HotbarID_Fertilizer].name        = "FERTILIZER";
    g_controller.hotbar[HotbarID_Fertilizer].description = "Increase a plant's stats for a limited time.";
    g_controller.hotbar[HotbarID_Fertilizer].cost        = 450;
    g_controller.hotbar[HotbarID_Fertilizer].unlock      = 0; // @Incomplete!
    g_controller.hotbar[HotbarID_Fertilizer].spawn_id    = EntityID_None;

    g_controller.hotbar[HotbarID_Shovel    ].name        = "Shovel";
    g_controller.hotbar[HotbarID_Shovel    ].description = "Dig up a plant and get some of your money back.";
    g_controller.hotbar[HotbarID_Shovel    ].cost        = 0;
    g_controller.hotbar[HotbarID_Shovel    ].unlock      = 0;
    g_controller.hotbar[HotbarID_Shovel    ].spawn_id    = EntityID_None;

    g_controller.hotbar[HotbarID_Bell      ].name        = "Summoning Bell";
    g_controller.hotbar[HotbarID_Bell      ].description = "Spawn the forces of evil straight away.";
    g_controller.hotbar[HotbarID_Bell      ].cost        = 0;
    g_controller.hotbar[HotbarID_Bell      ].unlock      = 0;
    g_controller.hotbar[HotbarID_Bell      ].spawn_id    = EntityID_None;
}

GLOBAL void controller_tick(nkF32 dt)
{
    // We always want to reset this even whilst paused in case we become unhovered.
    g_controller.hovered = NO_SELECTION;

    if(is_game_paused()) return;

    nkVec2 cursor_pos = get_window_mouse_pos();

    // Pan the camera around the world (via mouse or keyboard).
    g_controller.panning = NK_FALSE;

    if(is_mouse_button_down(MouseButton_Middle))
    {
        g_controller.camera_target_pos -= (get_relative_mouse_pos() / g_controller.camera_current_zoom);
        g_controller.panning = NK_TRUE;
    }

    if(is_key_down(KeyCode_W) || is_key_down(KeyCode_Up))
    {
        g_controller.camera_target_pos.y -= (PANNING_SPEED / g_controller.camera_current_zoom) * dt;
        g_controller.panning = NK_TRUE;
    }
    if(is_key_down(KeyCode_D) || is_key_down(KeyCode_Right))
    {
        g_controller.camera_target_pos.x += (PANNING_SPEED / g_controller.camera_current_zoom) * dt;
        g_controller.panning = NK_TRUE;
    }
    if(is_key_down(KeyCode_S) || is_key_down(KeyCode_Down))
    {
        g_controller.camera_target_pos.y += (PANNING_SPEED / g_controller.camera_current_zoom) * dt;
        g_controller.panning = NK_TRUE;
    }
    if(is_key_down(KeyCode_A) || is_key_down(KeyCode_Left))
    {
        g_controller.camera_target_pos.x -= (PANNING_SPEED / g_controller.camera_current_zoom) * dt;
        g_controller.panning = NK_TRUE;
    }

    if(g_controller.panning)
    {
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
    nkF32 img_scale = get_hud_scale() / 4.0f;
    nkF32 hud_scale = get_hud_scale();

    NK_UNUSED(hud_scale);

    // Check hovering and handle interaction with the hotbar slots.
    nkF32 x = ((HUD_CLIP_SLOT.w * 0.80f) * 0.5f) * img_scale;
    nkF32 y = ((HUD_CLIP_SLOT.h * 0.80f) * 0.5f) * img_scale;

    for(HotbarID i=0; i<HotbarID_TOTAL; ++i)
    {
        nkF32 rx = x - ((HUD_ICON_WIDTH * 0.5f) * img_scale);
        nkF32 ry = y - ((HUD_ICON_HEIGHT * 0.5f) * img_scale);
        nkF32 rw = (HUD_ICON_WIDTH * img_scale);
        nkF32 rh = (HUD_ICON_HEIGHT * img_scale);

        if(point_vs_rect(cursor_pos, rx,ry,rw,rh))
        {
            g_controller.hovered = i;
        }

        if((g_controller.hovered == i && is_mouse_button_pressed(MouseButton_Left)) || is_key_pressed(KeyCode_1+i))
        {
            const HotbarSlot& slot = g_controller.hotbar[i];
            if(g_controller.money >= slot.cost && get_waves_beaten() >= slot.unlock)
            {
                // Special case for the bell where it can only be used during preparation phase.
                if(i != HotbarID_Bell || (i == HotbarID_Bell && get_wave_state() == WaveState_Prepare))
                {
                    // Toggle the selection depending on if we are already selected or not.
                    g_controller.selected = (g_controller.selected == i) ? NO_SELECTION : i;
                    play_sound(asset_manager_load<Sound>("click.wav"));
                }
            }
        }

        x += ((HUD_CLIP_SLOT.w * 0.65f) * img_scale);
    }

    // Place current plant / perform the current tool action.
    if(g_controller.hovered == NO_SELECTION && is_mouse_button_pressed(MouseButton_Left))
    {
        nkVec2 pos = screen_to_world(cursor_pos);

        if(g_controller.selected != NO_SELECTION)
        {
            const HotbarSlot& slot = g_controller.hotbar[g_controller.selected];
            if(g_controller.money >= slot.cost)
            {
                nkBool success = NK_FALSE;

                if(g_controller.selected == HotbarID_Fertilizer)
                {
                    success = fertilize_plant(pos.x, pos.y);
                }
                else if(g_controller.selected == HotbarID_Shovel)
                {
                    success = remove_plant(pos.x, pos.y);
                }
                else if(g_controller.selected == HotbarID_Bell)
                {
                    play_sound(asset_manager_load<Sound>("bell.wav"));
                    begin_next_wave_now();
                }
                else
                {
                    iPoint tile;

                    tile.x = NK_CAST(nkS32, floorf(pos.x / NK_CAST(nkF32, TILE_WIDTH)));
                    tile.y = NK_CAST(nkS32, floorf(pos.y / NK_CAST(nkF32, TILE_HEIGHT)));

                    success = place_plant(tile.x, tile.y);
                }

                // If the action was successful then subtract the appropiate funds.
                if(success)
                {
                    sub_money(slot.cost);
                    if(g_controller.money < slot.cost)
                    {
                        // If we no longer have enough money to purchase another of what we have selected, just de-select it.
                        g_controller.selected = NO_SELECTION;
                    }
                }
            }
        }
    }

    // De-select current plant/tool.
    if(is_mouse_button_pressed(MouseButton_Right))
    {
        g_controller.selected = NO_SELECTION;
    }

    // Update the cursor graphic.
    if(g_controller.selected != NO_SELECTION)
    {
        if(g_controller.selected == HotbarID_Fertilizer || g_controller.selected == HotbarID_Shovel || g_controller.selected == HotbarID_Bell)
        {
            ImmClip clip = HUD_CLIP_ICON;
            clip.x += NK_CAST(nkF32, g_controller.selected) * HUD_ICON_WIDTH;
            set_cursor(CursorType_Custom, NULL, clip);
        }
        else
        {
            set_cursor(CursorType_Arrow);
        }
    }

    // Update the health stat.
    Entity* house = get_first_entity_with_id(EntityID_HomeTree);
    if(!house)
        g_controller.health = 0;
    else
    {
        g_controller.health = NK_CAST(nkS32, ceilf(house->health));
    }

    // Interpolate the money counter toward the actual money. We only do this when money goes up as it feels better
    // to have it tick upwards and be set immediately when going down from purchasing stuff.
    const nkS32 MONEY_INCREMENT = 10;

    if(g_controller.money_counter > g_controller.money)
    {
        g_controller.money_counter = g_controller.money;
        g_controller.money_color = NK_V4_RED;
    }
    if(g_controller.money_counter < g_controller.money)
    {
        g_controller.money_counter += MONEY_INCREMENT;
        g_controller.money_color = NK_V4_GREEN;

        if(g_controller.money_counter > g_controller.money)
        {
            g_controller.money_counter = g_controller.money;
        }
    }

    g_controller.money_color = nk_lerp(g_controller.money_color, NK_V4_WHITE, dt * 1.0f);

    // Check if we are holding the bell when the fighting begins, if so deselect.
    if(g_controller.selected == HotbarID_Bell && get_wave_state() == WaveState_Fight)
    {
        g_controller.selected = NO_SELECTION;
    }
}

GLOBAL void controller_draw(void)
{
    // We don't draw the HUD if the game is paused because it looks better that way.
    // We still want to unset the controller camera transform though, so do that here.
    if(is_game_paused() || is_game_over())
    {
        unset_controller_camera();
        return;
    }

    Texture texture = asset_manager_load<Texture>("hud.png");

    TrueTypeFont font = get_font();

    nkVec2 cursor_pos = get_window_mouse_pos();

    // Draw the highlighted tile.
    if(g_controller.hovered == NO_SELECTION && g_controller.selected != NO_SELECTION && g_controller.selected != HotbarID_Bell)
    {
        nkVec2 pos = screen_to_world(cursor_pos);

        iPoint tile;

        tile.x = NK_CAST(nkS32, floorf(pos.x / (NK_CAST(nkF32, TILE_WIDTH))));
        tile.y = NK_CAST(nkS32, floorf(pos.y / (NK_CAST(nkF32, TILE_HEIGHT))));

        // Make sure we can place at the spot.
        nkVec4 color = NK_V4_RED;
        if(can_place_plant_at_position(tile.x, tile.y) || can_remove_plant_at_position(pos.x, pos.y) || can_fertilize_plant_at_position(pos.x, pos.y))
        {
            color = NK_V4_YELLOW;
        }

        nkF32 tx = NK_CAST(nkF32, tile.x * TILE_WIDTH) + (NK_CAST(nkF32,TILE_WIDTH) * 0.5f);
        nkF32 ty = NK_CAST(nkF32, tile.y * TILE_HEIGHT) + (NK_CAST(nkF32,TILE_HEIGHT) * 0.5f);

        imm_texture(texture, tx,ty, &HUD_CLIP_CURSOR, color);
    }

    // Unset the camera so that we render in screen-space for the HUD.
    unset_controller_camera();

    // Images need to be scaled down because they are actually at the max scale initially and scaled down for lower scales.
    // Other elements like text and general positioning need to still be scaled up, so we have these two variables to do it.
    nkF32 img_scale = get_hud_scale() / 4.0f;
    nkF32 hud_scale = get_hud_scale();

    // Draw the hotbar.
    nkF32 x = ((HUD_CLIP_SLOT.w * 0.80f) * 0.5f) * img_scale;
    nkF32 y = ((HUD_CLIP_SLOT.h * 0.80f) * 0.5f) * img_scale;

    nkS32 angle = 0;

    for(HotbarID i=0; i<HotbarID_TOTAL; ++i)
    {
        nkF32 rx = x - ((HUD_ICON_WIDTH * 0.5f) * img_scale);
        nkF32 ry = y - ((HUD_ICON_HEIGHT * 0.5f) * img_scale);
        nkF32 rw = (HUD_ICON_WIDTH * img_scale);
        nkF32 rh = (HUD_ICON_HEIGHT * img_scale);

        imm_rect_filled(rx,ry,rw,rh, { 0.0f,0.0f,0.0f,0.5f });

        const HotbarSlot& slot = g_controller.hotbar[i];

        nkVec4 frame_color = (g_controller.selected == i) ? NK_V4_YELLOW : NK_V4_BLACK;

        nkBool cannot_afford = (g_controller.money < slot.cost);
        nkBool is_locked = (get_waves_beaten() < slot.unlock);

        ImmClip clip = HUD_CLIP_ICON;
        clip.x += NK_CAST(nkF32, i) * HUD_ICON_WIDTH;
        if(cannot_afford || (i == HotbarID_Bell && get_wave_state() == WaveState_Fight)) // Special case for the bell.
            clip.y += HUD_ICON_HEIGHT;
        if(is_locked)
            clip = HUD_CLIP_PADLOCK;

        imm_texture_ex(texture, x,y, img_scale,img_scale, 0.0f, NULL, &clip, NK_V4_WHITE);

        imm_texture_ex(texture, x,y, img_scale,img_scale, nk_torad(NK_CAST(nkF32, angle)), NULL, &HUD_CLIP_SLOT, frame_color);

        // Draw the price of the slot if it has one.
        if(slot.cost > 0 && !is_locked)
        {
            set_truetype_font_size(font, NK_CAST(nkS32, 10 * hud_scale));

            nkString string;
            number_to_string_with_commas(&string, slot.cost);

            nkVec4 text_color = (cannot_afford || is_locked) ? nkVec4 { 0.5f,0.5f,0.5f,1.0f } : nkVec4 { 1.0f,1.0f,1.0f,1.0f };

            nkF32 text_x = x - (16.0f * hud_scale);
            nkF32 text_y = y + (18.0f * hud_scale);

            draw_truetype_text(font, text_x+(2*hud_scale),text_y+(1*hud_scale), string.cstr, NK_V4_BLACK);
            draw_truetype_text(font, text_x+(2*hud_scale),text_y-(1*hud_scale), string.cstr, NK_V4_BLACK);
            draw_truetype_text(font, text_x-(1*hud_scale),text_y+(1*hud_scale), string.cstr, NK_V4_BLACK);
            draw_truetype_text(font, text_x-(1*hud_scale),text_y-(1*hud_scale), string.cstr, NK_V4_BLACK);

            draw_truetype_text(font, text_x,text_y, string.cstr, NK_V4_WHITE * text_color);
        }

        angle = (angle + 90) % 360;

        x += ((HUD_CLIP_SLOT.w * 0.65f) * img_scale);
    }

    // Draw the money counter.
    set_truetype_font_size(font, NK_CAST(nkS32, 15 * hud_scale));

    nkString string = "$";
    number_to_string_with_commas(&string, g_controller.money_counter);

    nkF32 text_x = (8.0f * hud_scale);
    nkF32 text_y = ((HUD_ICON_HEIGHT * 1.4f) * img_scale) + get_truetype_line_height(font);

    draw_truetype_text(font, text_x+(2*hud_scale),text_y+(2*hud_scale), string.cstr, NK_V4_BLACK);
    draw_truetype_text(font, text_x,              text_y,               string.cstr, g_controller.money_color);

    // Draw the stats/icons.
    nkF32 icon_scale = img_scale * 0.8f;

    x = (((HUD_CLIP_SLOT.w * 0.80f) * 0.5f) * icon_scale);
    y = (((HUD_CLIP_SLOT.h * 1.20f) * 0.5f) * icon_scale) + text_y;

    draw_hud_stat(texture, x,y, icon_scale, hud_scale, HUD_CLIP_HEART, g_controller.health);
    y += ((HUD_ICON_HEIGHT) * icon_scale);
    draw_hud_stat(texture, x,y, icon_scale, hud_scale, HUD_CLIP_FLAG, get_wave_counter());
    y += ((HUD_ICON_HEIGHT) * icon_scale);
    draw_hud_stat(texture, x,y, icon_scale, hud_scale, HUD_CLIP_SKULL, g_controller.kills);
    y += ((HUD_ICON_HEIGHT) * icon_scale);

    // If hovered over an icon then display its tooltip.
    if(g_controller.hovered != NO_SELECTION && g_controller.selected == NO_SELECTION)
    {
        set_truetype_font_size(font, NK_CAST(nkS32, 10 * hud_scale));

        const HotbarSlot& slot = g_controller.hotbar[g_controller.hovered];

        nkF32 padding = (TOOLTIP_PADDING * hud_scale);

        nkString cost_string = "Cost: $";
        number_to_string_with_commas(&cost_string, slot.cost);

        nkString unlock_string = "Unlocks After Wave ";
        number_to_string_with_commas(&unlock_string, slot.unlock);

        nkF32 tx = cursor_pos.x;
        nkF32 ty = cursor_pos.y;
        nkF32 tw = 0.0f;
        nkF32 th = 0.0f;

        tx += ((HUD_CLIP_POINTER.w * 0.5f) * img_scale);
        ty += ((HUD_CLIP_POINTER.h * 0.5f) * img_scale);

        // Calculate the size of the tooltip box.
        if(get_waves_beaten() >= slot.unlock)
        {
            tw  = nk_max(tw, get_truetype_text_width(font, slot.name));
            tw  = nk_max(tw, get_truetype_text_width(font, cost_string.cstr));
            tw  = nk_max(tw, get_truetype_text_width(font, slot.description));
            tw += padding * 2.0f;

            th += (get_truetype_text_height(font, slot.name) + padding);
            th += (get_truetype_text_height(font, cost_string.cstr) + padding) * (slot.cost != 0);
            th += get_truetype_text_height(font, slot.description);
            th += padding * 2.0f;
        }
        else
        {
            tw  = nk_max(tw, get_truetype_text_width(font, "??????"));
            tw  = nk_max(tw, get_truetype_text_width(font, unlock_string.cstr));
            tw += padding * 2.0f;

            th += (get_truetype_text_height(font, "??????") + padding);
            th += (get_truetype_text_height(font, unlock_string.cstr));
            th += padding * 2.0f;
        }

        // Draw the background.
        nkF32 bg_scale = img_scale * 0.5f;

        nkVec4 bg_color = { 0.0f,0.0f,0.0f,0.8f };

        nkF32 iw = HUD_ICON_WIDTH * bg_scale;
        nkF32 ih = HUD_ICON_HEIGHT * bg_scale;

        tw -= iw * 2.0f;
        th -= ih * 2.0f;

        if(tw < 0.0f) tw = 0.0f;
        if(th < 0.0f) th = 0.0f;

        nkF32 x1 = tx + (iw * 0.5f);
        nkF32 y1 = ty + (ih * 0.5f);
        nkF32 x2 = x1 + (iw * 0.5f) + (tw * 0.5f);
        nkF32 y2 = y1 + (ih * 0.5f) + (th * 0.5f);
        nkF32 x3 = x1 + iw + tw;
        nkF32 y3 = y1 + ih + th;

        nkF32 w_scale = (tw / bg_scale) / (HUD_ICON_WIDTH) * bg_scale;
        nkF32 h_scale = (th / bg_scale) / (HUD_ICON_HEIGHT) * bg_scale;

        imm_texture_ex(texture, x1,y1, bg_scale,bg_scale, 0.0f, NULL, &HUD_CLIP_TOOLTIP_TL, bg_color);
        imm_texture_ex(texture, x2,y1, w_scale, bg_scale, 0.0f, NULL, &HUD_CLIP_TOOLTIP_T,  bg_color);
        imm_texture_ex(texture, x3,y1, bg_scale,bg_scale, 0.0f, NULL, &HUD_CLIP_TOOLTIP_TR, bg_color);
        imm_texture_ex(texture, x1,y2, bg_scale,h_scale,  0.0f, NULL, &HUD_CLIP_TOOLTIP_L,  bg_color);
        imm_texture_ex(texture, x3,y2, bg_scale,h_scale,  0.0f, NULL, &HUD_CLIP_TOOLTIP_R,  bg_color);
        imm_texture_ex(texture, x1,y3, bg_scale,bg_scale, 0.0f, NULL, &HUD_CLIP_TOOLTIP_BL, bg_color);
        imm_texture_ex(texture, x2,y3, w_scale, bg_scale, 0.0f, NULL, &HUD_CLIP_TOOLTIP_B,  bg_color);
        imm_texture_ex(texture, x3,y3, bg_scale,bg_scale, 0.0f, NULL, &HUD_CLIP_TOOLTIP_BR, bg_color);

        if(tw > 0.0f && th > 0.0f)
        {
            imm_rect_filled(tx+iw,ty+ih,tw,th, bg_color);
        }

        // Draw the text.
        text_x = tx + padding;
        text_y = ty + (padding * 0.5f);

        text_y += get_truetype_line_height(font);

        if(get_waves_beaten() >= slot.unlock)
        {
            draw_truetype_text(font, text_x+(2*hud_scale),text_y+(2*hud_scale), slot.name, NK_V4_BLACK);
            draw_truetype_text(font, text_x,text_y, slot.name, NK_V4_WHITE);

            if(slot.cost > 0)
            {
                nkVec4 cost_color = (g_controller.money >= slot.cost) ? NK_V4_YELLOW : NK_V4_RED;

                text_y += get_truetype_line_height(font) + padding;

                draw_truetype_text(font, text_x+(2*hud_scale),text_y+(2*hud_scale), cost_string.cstr, NK_V4_BLACK);
                draw_truetype_text(font, text_x, text_y, cost_string.cstr, cost_color);
            }

            text_y += get_truetype_line_height(font) + padding;

            draw_truetype_text(font, text_x+(2*hud_scale),text_y+(2*hud_scale), slot.description, NK_V4_BLACK);
            draw_truetype_text(font, text_x, text_y, slot.description, { 0.7f,0.7f,0.7f,1.0f });
        }
        else
        {
            draw_truetype_text(font, text_x+(2*hud_scale),text_y+(2*hud_scale), "??????", NK_V4_BLACK);
            draw_truetype_text(font, text_x,text_y, "??????", NK_V4_WHITE);

            text_y += get_truetype_line_height(font) + padding;

            draw_truetype_text(font, text_x+(2*hud_scale),text_y+(2*hud_scale), unlock_string.cstr, NK_V4_BLACK);
            draw_truetype_text(font, text_x,text_y, unlock_string.cstr, { 0.7f,0.7f,0.7f,1.0f });
        }
    }
}

GLOBAL void controller_reset(void)
{
    // Center the camera on the map.
    nkF32 center_x = NK_CAST(nkF32, (get_world_width() * TILE_WIDTH)) * 0.5f;
    nkF32 center_y = NK_CAST(nkF32, (get_world_height() * TILE_HEIGHT)) * 0.5f;

    g_controller.camera_current_pos = { center_x, center_y };
    g_controller.camera_target_pos  = { center_x, center_y };

    g_controller.camera_current_zoom = CAMERA_START_ZOOM;
    g_controller.camera_target_zoom  = CAMERA_START_ZOOM;

    g_controller.money_color   = NK_V4_WHITE;
    g_controller.money_counter = STARTING_MONEY;
    g_controller.money         = STARTING_MONEY;
    g_controller.kills         = 0;
    g_controller.health        = NK_CAST(nkS32, ENTITY_TABLE[EntityID_HomeTree].health);

    g_controller.selected = NO_SELECTION;
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

GLOBAL void increment_kill_count(void)
{
    g_controller.kills++;
}

GLOBAL EntityID get_selected_plant(void)
{
    if(g_controller.selected == NO_SELECTION) return EntityID_None;

    nkVec2 pos = screen_to_world(get_window_mouse_pos());

    iPoint tile;

    tile.x = NK_CAST(nkS32, floorf(pos.x / (NK_CAST(nkF32, TILE_WIDTH))));
    tile.y = NK_CAST(nkS32, floorf(pos.y / (NK_CAST(nkF32, TILE_HEIGHT))));

    if(!can_place_plant_at_position(tile.x, tile.y)) return EntityID_None;

    return g_controller.hotbar[g_controller.selected].spawn_id;
}

GLOBAL nkVec2 get_cursor_world_pos(void)
{
    return screen_to_world(get_window_mouse_pos());
}

GLOBAL void add_money(nkS32 money)
{
    g_controller.money += money;
}

GLOBAL void sub_money(nkS32 money)
{
    g_controller.money -= money;
}

GLOBAL nkBool is_something_selected(void)
{
    return (g_controller.selected != NO_SELECTION);
}

GLOBAL nkS32 get_health(void)
{
    return g_controller.health;
}

GLOBAL nkS32 get_kills(void)
{
    return g_controller.kills;
}

GLOBAL void draw_hud_stat(Texture texture, nkF32 x, nkF32 y, nkF32 icon_scale, nkF32 hud_scale, ImmClip clip, nkS32 stat)
{
    TrueTypeFont font = get_font();

    set_truetype_font_size(font, NK_CAST(nkS32, 15 * hud_scale));

    imm_texture_ex(texture, x+(2*hud_scale),y+(2*hud_scale), icon_scale,icon_scale, 0.0f, NULL, &clip, { 0.0f,0.0f,0.0f,0.35f });
    imm_texture_ex(texture, x,y, icon_scale,icon_scale, 0.0f, NULL, &clip);

    nkF32 text_x = x + ((HUD_ICON_WIDTH * 0.65f) * icon_scale);
    nkF32 text_y = y + (get_truetype_line_height(font) * 0.25f);

    nkString string;
    number_to_string_with_commas(&string, stat);

    draw_truetype_text(font, text_x+(2*hud_scale),text_y+(2*hud_scale), string.cstr, NK_V4_BLACK);
    draw_truetype_text(font, text_x,text_y, string.cstr, NK_V4_WHITE);
}

GLOBAL nkBool anything_unlocked_this_wave(void)
{
    for(nkS32 i=0; i<HotbarID_TOTAL; ++i)
    {
        if(g_controller.hotbar[i].unlock == get_waves_beaten())
            return NK_TRUE;
    }
    return NK_FALSE;
}

/*////////////////////////////////////////////////////////////////////////////*/
