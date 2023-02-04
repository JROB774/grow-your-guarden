/*////////////////////////////////////////////////////////////////////////////*/

// @Incomplete: The window size will effect how close together the buttons are placed, this is not very good...

INTERNAL constexpr const nkChar* PAUSE_TITLE_TEXT  = "PAUSED!";
INTERNAL constexpr const nkChar* PAUSE_RESUME_TEXT = "Resume";
INTERNAL constexpr const nkChar* PAUSE_MENU_TEXT   = "Menu";

INTERNAL constexpr nkS32 PAUSE_TITLE_SIZE  = 100;
INTERNAL constexpr nkS32 PAUSE_RESUME_SIZE =  40;
INTERNAL constexpr nkS32 PAUSE_MENU_SIZE   =  35;

INTERNAL constexpr nkF32 PAUSE_TITLE_YPOS  =  0.40f;
INTERNAL constexpr nkF32 PAUSE_RESUME_YPOS =  0.65f;
INTERNAL constexpr nkF32 PAUSE_MENU_YPOS   = -1.00f;

struct GameState
{
    Sound  pause_sound;
    nkBool paused;
};

INTERNAL GameState g_game;

INTERNAL void pause_tick(nkF32 dt)
{
    // Toggle the pause state.
    if(is_key_pressed(KeyCode_Escape))
    {
        g_game.paused = !g_game.paused;
        // play_sound(g_game.pause_sound);
    }

    if(!g_game.paused) return;

    if(tick_menu_text_button(PAUSE_RESUME_TEXT, PAUSE_RESUME_YPOS, PAUSE_RESUME_SIZE))
    {
        g_game.paused = NK_FALSE;
        // play_sound(g_game.pause_sound);
    }
    if(tick_menu_text_button(PAUSE_MENU_TEXT, PAUSE_MENU_YPOS, PAUSE_MENU_SIZE))
    {
        set_app_state(AppState_Menu);
        // play_sound(g_game.pause_sound);
    }

    // Do the options buttons.
    nkF32 img_scale = get_hud_scale() / 4.0f;

    nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
    nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

    nkF32 bx = NK_CAST(nkF32, get_window_width()) - (btn_width * 0.5f);
    nkF32 by = (NK_CAST(nkF32, get_window_height()) * 0.85f) - (btn_height * 0.6f);

    if(tick_menu_toggle_button(HUD_CLIP_MUSIC,      bx-(btn_width*0.0f),by, is_music_on  ())) set_music_volume((is_music_on()) ? 0.0f : 0.7f);
    if(tick_menu_toggle_button(HUD_CLIP_SOUND,      bx-(btn_width*1.1f),by, is_sound_on  ())) set_sound_volume((is_sound_on()) ? 0.0f : 0.8f);
    if(tick_menu_toggle_button(HUD_CLIP_FULLSCREEN, bx-(btn_width*2.2f),by, is_fullscreen())) set_fullscreen(!is_fullscreen());
}

INTERNAL void pause_draw(void)
{
    if(!g_game.paused) return;

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    // Draw the background.
    nkVec4 background_color = { 0.0f,0.0f,0.0f,0.7f };
    imm_rect_filled(0.0f,0.0f,ww,wh, background_color);

    // Draw the letterbox.
    nkVec4 letterbox_color = { 0.0f,0.0f,0.0f,1.0f };

    imm_rect_filled(0.0f,0.0f,    ww,wh*0.15f, letterbox_color);
    imm_rect_filled(0.0f,wh*0.85f,ww,wh*0.15f, letterbox_color);

    // Draw the text and buttons.
    draw_menu_text_button(PAUSE_TITLE_TEXT, PAUSE_TITLE_YPOS, PAUSE_TITLE_SIZE, NK_FALSE);
    draw_menu_text_button(PAUSE_RESUME_TEXT, PAUSE_RESUME_YPOS, PAUSE_RESUME_SIZE);
    draw_menu_text_button(PAUSE_MENU_TEXT, PAUSE_MENU_YPOS, PAUSE_MENU_SIZE);

    // Draw the options buttons.
    nkF32 img_scale = (get_hud_scale() / 4.0f);

    nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
    nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

    nkF32 bx = (NK_CAST(nkF32, get_window_width())) - (btn_width * 0.6f);
    nkF32 by = (NK_CAST(nkF32, get_window_height()) * 0.85f) - (btn_height * 0.6f);

    draw_menu_toggle_button(HUD_CLIP_MUSIC,      bx-(btn_width*0.0f),by, is_music_on  ());
    draw_menu_toggle_button(HUD_CLIP_SOUND,      bx-(btn_width*1.1f),by, is_sound_on  ());
    draw_menu_toggle_button(HUD_CLIP_FULLSCREEN, bx-(btn_width*2.2f),by, is_fullscreen());
}

GLOBAL void game_start(void)
{
    world_load("level00.png");

    // Reset the systems.
    wave_manager_reset();
    entity_reset();
    controller_reset();
    particle_reset();
    decal_reset();

    // Spawn the house.
    nkF32 hx = NK_CAST(nkF32, get_world_width() * TILE_WIDTH) * 0.5f;
    nkF32 hy = NK_CAST(nkF32, get_world_height() * TILE_HEIGHT) * 0.5f;
    entity_spawn(EntityID_House, hx,hy);

    // Make sure we aren't paused.
    g_game.paused = NK_FALSE;

    set_app_state(AppState_Game);
}

GLOBAL void game_init(void)
{
    g_game.pause_sound = asset_manager_load<Sound>("pause.wav");

    entity_init();
    controller_init();
    particle_init();
    decal_init();
}

GLOBAL void game_quit(void)
{
    decal_quit();
    particle_quit();
    entity_quit();
    world_free();
}

GLOBAL void game_tick(nkF32 dt)
{
    controller_tick(dt);

    // If we're paused we don't want to update the game world.
    if(!g_game.paused)
    {
        wave_manager_tick(dt);
        world_tick(dt);
        entity_tick(dt);
        particle_tick(dt);
        decal_tick(dt);
    }

    pause_tick(dt); // Make sure this happens at the end so clicks don't bleed through!
}

GLOBAL void game_draw(void)
{
    set_controller_camera();
    world_draw_below();
    decal_draw();
    entity_draw();
    particle_draw();
    world_draw_above();
    wave_manager_draw();
    controller_draw(); // This internally unsets the controller camera!
    pause_draw();
}

GLOBAL nkBool is_game_paused(void)
{
    return g_game.paused;
}

/*////////////////////////////////////////////////////////////////////////////*/
