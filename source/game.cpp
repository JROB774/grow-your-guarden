/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr const nkChar* PAUSE_TITLE_TEXT  = "PAUSED!";
INTERNAL constexpr const nkChar* PAUSE_RESUME_TEXT = "Resume";
INTERNAL constexpr const nkChar* PAUSE_MENU_TEXT   = "Menu";

INTERNAL constexpr nkS32 PAUSE_TITLE_SIZE  = 100;
INTERNAL constexpr nkS32 PAUSE_RESUME_SIZE =  40;
INTERNAL constexpr nkS32 PAUSE_MENU_SIZE   =  35;

INTERNAL constexpr nkF32 PAUSE_TITLE_YPOS  =  0.40f;
INTERNAL constexpr nkF32 PAUSE_RESUME_YPOS =  0.65f;
INTERNAL constexpr nkF32 PAUSE_MENU_YPOS   = -1.00f;

INTERNAL constexpr const nkChar* GAMEOVER_TITLE_TEXT = "GAME OVER!";
INTERNAL constexpr const nkChar* GAMEOVER_RETRY_TEXT = "Retry";
INTERNAL constexpr const nkChar* GAMEOVER_MENU_TEXT  = "Menu";

INTERNAL constexpr nkS32 GAMEOVER_TITLE_SIZE = 100;
INTERNAL constexpr nkS32 GAMEOVER_RETRY_SIZE =  40;
INTERNAL constexpr nkS32 GAMEOVER_MENU_SIZE  =  35;

INTERNAL constexpr nkF32 GAMEOVER_TITLE_YPOS =  0.40f;
INTERNAL constexpr nkF32 GAMEOVER_STAT_YPOS  =  0.52f;
INTERNAL constexpr nkF32 GAMEOVER_RETRY_YPOS =  0.70f;
INTERNAL constexpr nkF32 GAMEOVER_MENU_YPOS  = -1.00f;

struct GameState
{
    Sound  pause_sound;
    Sound  munch_sounds[12];
    Sound  splat_sounds[3];
    nkBool paused;
    nkBool game_over;
};

INTERNAL GameState g_game;

INTERNAL void draw_letterbox_background(void)
{
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkVec4 back_color = { 0.0f,0.0f,0.0f,0.7f };
    nkVec4 lbox_color = { 0.0f,0.0f,0.0f,1.0f };

    imm_rect_filled(0.0f,0.0f,    ww,wh,       back_color);
    imm_rect_filled(0.0f,0.0f,    ww,wh*0.15f, lbox_color);
    imm_rect_filled(0.0f,wh*0.85f,ww,wh*0.15f, lbox_color);
}

INTERNAL void pause_tick(nkF32 dt)
{
    // Toggle the pause state.
    if(is_key_pressed(KeyCode_Escape))
    {
        g_game.paused = !g_game.paused;
        if(g_game.paused) pause_music();
        else resume_music();
        play_sound(g_game.pause_sound);
    }

    if(!g_game.paused) return;

    if(tick_menu_text_button(PAUSE_RESUME_TEXT, PAUSE_RESUME_YPOS, PAUSE_RESUME_SIZE))
    {
        g_game.paused = NK_FALSE;
        resume_music();
        play_sound(g_game.pause_sound);
    }
    if(tick_menu_text_button(PAUSE_MENU_TEXT, PAUSE_MENU_YPOS, PAUSE_MENU_SIZE))
    {
        set_app_state(AppState_Menu);
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

    draw_letterbox_background();

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

INTERNAL void game_over_tick(nkF32 dt)
{
    if(tick_menu_text_button(GAMEOVER_RETRY_TEXT, GAMEOVER_RETRY_YPOS, GAMEOVER_RETRY_SIZE))
    {
        game_start();
        play_music(asset_manager_load<Music>("garden.ogg"));
    }
    if(tick_menu_text_button(GAMEOVER_MENU_TEXT, GAMEOVER_MENU_YPOS, GAMEOVER_MENU_SIZE))
    {
        set_app_state(AppState_Menu);
    }
}

INTERNAL void game_over_draw(void)
{
    if(!g_game.game_over) return;

    draw_letterbox_background();

    // Draw the text and buttons.
    draw_menu_text_button(GAMEOVER_TITLE_TEXT, GAMEOVER_TITLE_YPOS, GAMEOVER_TITLE_SIZE, NK_FALSE);
    draw_menu_text_button(GAMEOVER_RETRY_TEXT, GAMEOVER_RETRY_YPOS, GAMEOVER_RETRY_SIZE);
    draw_menu_text_button(GAMEOVER_MENU_TEXT, GAMEOVER_MENU_YPOS, GAMEOVER_MENU_SIZE);

    // Draw the stats.
    nkF32 img_scale = get_hud_scale() / 4.0f;
    nkF32 hud_scale = get_hud_scale();

    nkF32 icon_scale = img_scale * 0.8f;

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkF32 sx = ww * 0.5f;
    nkF32 sy = wh * GAMEOVER_STAT_YPOS;

    Texture hud = asset_manager_load<Texture>("hud.png");

    draw_hud_stat(hud, sx-(96.0f*hud_scale),sy, icon_scale, hud_scale, HUD_CLIP_FLAG,  get_wave_counter());
    draw_hud_stat(hud, sx+(96.0f*hud_scale),sy, icon_scale, hud_scale, HUD_CLIP_SKULL, get_kills());
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

    // Spawn the home tree.
    nkF32 hx = NK_CAST(nkF32, get_world_width() * TILE_WIDTH) * 0.5f;
    nkF32 hy = NK_CAST(nkF32, get_world_height() * TILE_HEIGHT) * 0.5f;
    entity_spawn(EntityID_HomeTree, hx,hy);

    // Make sure we aren't paused or game overed.
    g_game.paused = NK_FALSE;
    g_game.game_over = NK_FALSE;

    set_app_state(AppState_Game);
}

GLOBAL void game_init(void)
{
    // Pre-load some assets.
    asset_manager_load<Music>("battle_01.ogg");
    asset_manager_load<Music>("battle_02.ogg");
    asset_manager_load<Music>("garden.ogg");
    asset_manager_load<Sound>("coin_collect.wav");
    asset_manager_load<Sound>("coin_drop.wav");
    asset_manager_load<Sound>("fanfare_normal.wav");
    asset_manager_load<Sound>("fanfare_unlock.wav");
    asset_manager_load<Sound>("gulp.wav");
    asset_manager_load<Sound>("cough.wav");

    g_game.munch_sounds[ 0] = asset_manager_load<Sound>("munch_000.wav");
    g_game.munch_sounds[ 1] = asset_manager_load<Sound>("munch_001.wav");
    g_game.munch_sounds[ 2] = asset_manager_load<Sound>("munch_002.wav");
    g_game.munch_sounds[ 3] = asset_manager_load<Sound>("munch_003.wav");
    g_game.munch_sounds[ 4] = asset_manager_load<Sound>("munch_004.wav");
    g_game.munch_sounds[ 5] = asset_manager_load<Sound>("munch_005.wav");
    g_game.munch_sounds[ 6] = asset_manager_load<Sound>("munch_006.wav");
    g_game.munch_sounds[ 7] = asset_manager_load<Sound>("munch_007.wav");
    g_game.munch_sounds[ 8] = asset_manager_load<Sound>("munch_008.wav");
    g_game.munch_sounds[ 9] = asset_manager_load<Sound>("munch_009.wav");
    g_game.munch_sounds[10] = asset_manager_load<Sound>("munch_010.wav");
    g_game.munch_sounds[11] = asset_manager_load<Sound>("munch_011.wav");

    g_game.splat_sounds[0] = asset_manager_load<Sound>("splat_000.wav");
    g_game.splat_sounds[1] = asset_manager_load<Sound>("splat_001.wav");
    g_game.splat_sounds[2] = asset_manager_load<Sound>("splat_002.wav");

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
    if(!g_game.game_over)
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

        // If the tree is dead then its game over!
        if(get_health() <= 0)
        {
            g_game.game_over = NK_TRUE;
            stop_music();
        }
    }
    else
    {
        game_over_tick(dt);
    }
}

GLOBAL void game_draw(void)
{
    set_controller_camera();
    world_draw_below();
    decal_draw();
    entity_draw();
    particle_draw();
    world_draw_above();
    wave_manager_draw_world();
    controller_draw(); // This internally unsets the controller camera!
    wave_manager_draw_hud();
    pause_draw();
    game_over_draw();
}

GLOBAL nkBool is_game_paused(void)
{
    return g_game.paused;
}

GLOBAL nkBool is_game_over(void)
{
    return g_game.game_over;
}

GLOBAL Sound get_random_munch_sound(void)
{
    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_game.munch_sounds)-1);
    return g_game.munch_sounds[sound_index];
}

GLOBAL Sound get_random_splat_sound(void)
{
    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_game.splat_sounds)-1);
    return g_game.splat_sounds[sound_index];
}

/*////////////////////////////////////////////////////////////////////////////*/
