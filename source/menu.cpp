/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr const nkChar* MENU_PLAY_TEXT    = "Play";
INTERNAL constexpr const nkChar* MENU_HOWTO_TEXT   = "How to Play";
INTERNAL constexpr const nkChar* MENU_CREDITS_TEXT = "Credits";
INTERNAL constexpr const nkChar* MENU_EXIT_TEXT    = "Exit";

INTERNAL constexpr nkS32 MENU_PLAY_SIZE    = 50;
INTERNAL constexpr nkS32 MENU_HOWTO_SIZE   = 35;
INTERNAL constexpr nkS32 MENU_CREDITS_SIZE = 35;
INTERNAL constexpr nkS32 MENU_EXIT_SIZE    = 35;

INTERNAL constexpr nkF32 MENU_PLAY_YPOS    =  0.75f;
INTERNAL constexpr nkF32 MENU_HOWTO_YPOS   = -1.00f;
INTERNAL constexpr nkF32 MENU_CREDITS_YPOS = -1.00f;
INTERNAL constexpr nkF32 MENU_EXIT_YPOS    = -1.00f;

INTERNAL constexpr const nkChar* MENU_DIFFICULTY_INFO_TEXT   = "Select Your Difficulty";
INTERNAL constexpr const nkChar* MENU_DIFFICULTY_EASY_TEXT   = "Easy";
INTERNAL constexpr const nkChar* MENU_DIFFICULTY_NORMAL_TEXT = "Normal";
INTERNAL constexpr const nkChar* MENU_DIFFICULTY_HARD_TEXT   = "Hard";

INTERNAL constexpr nkS32 MENU_DIFFICULTY_INFO_SIZE   = 100;
INTERNAL constexpr nkS32 MENU_DIFFICULTY_EASY_SIZE   =  35;
INTERNAL constexpr nkS32 MENU_DIFFICULTY_NORMAL_SIZE =  35;
INTERNAL constexpr nkS32 MENU_DIFFICULTY_HARD_SIZE   =  35;

INTERNAL constexpr nkF32 MENU_DIFFICULTY_INFO_YPOS   =  0.40f;
INTERNAL constexpr nkF32 MENU_DIFFICULTY_EASY_YPOS   =  0.75f;
INTERNAL constexpr nkF32 MENU_DIFFICULTY_NORMAL_YPOS = -1.00f;
INTERNAL constexpr nkF32 MENU_DIFFICULTY_HARD_YPOS   = -1.00f;

INTERNAL constexpr nkF32 MENU_SLIDE_SPEED = 2.0f;

INTERNAL constexpr nkS32 MENU_LAYERS = 4;

INTERNAL constexpr nkF32 MENU_WIDTH = 1280.0f;
INTERNAL constexpr nkF32 MENU_HEIGHT = 720.0f;

INTERNAL constexpr nkS32 MENU_TUTORIAL_PAGE_COUNT = 9;

// Calculated as a percentage of the viewport size.
INTERNAL constexpr nkF32 STEAM_BUTTON_WIDTH  = 0.35f;
INTERNAL constexpr nkF32 STEAM_BUTTON_HEIGHT = 0.38f;

INTERNAL constexpr nkS32 VERSION_SIZE = 20;

NK_ENUM(MenuScreen, nkS32)
{
    MenuScreen_Main,
    MenuScreen_Tutorial,
    MenuScreen_Credits,
    MenuScreen_Difficulty
};

NK_ENUM(MenuStage, nkS32)
{
    MenuStage_Animation,
    MenuStage_PressAnyButton,
    MenuStage_Interactive,
    MenuStage_TOTAL
};

struct MenuState
{
    MenuScreen screen;
    MenuStage  stage;
    nkF32      xoffset[MENU_LAYERS];
    nkF32      timer;
    fRect      viewport;
    nkF32      scale;
    nkS32      current_tutorial_page;
    Sound      page_flip_sound[11];
    nkBool     steam_area_hovered;
};

INTERNAL MenuState g_menu;

INTERNAL fRect calculate_menu_button_bounds(TrueTypeFont font, const nkChar* text, nkF32& prev_y, nkF32 curr_y)
{
    // We multiply by a percentage because it gives just a bit of a gap between the buttons which is nice.
    if(curr_y <= -1.0f) curr_y = ((g_menu.viewport.h * prev_y) + (get_truetype_line_height(font) * 1.3f)) / g_menu.viewport.h; // Increment from the previous y-position.

    fRect bounds;

    bounds.w = get_truetype_text_width(font, text);
    bounds.h = get_truetype_text_height(font, text);
    bounds.x = g_menu.viewport.x + ((g_menu.viewport.w - bounds.w) * 0.5f);
    bounds.y = g_menu.viewport.y + (g_menu.viewport.h * curr_y);

    prev_y = curr_y;

    return bounds;
}

INTERNAL void tick_back_button(void)
{
    nkF32 img_scale = (get_hud_scale() / 4.0f);

    nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
    nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

    nkF32 bx = (g_menu.viewport.x) + (btn_width * 0.6f);
    nkF32 by = (g_menu.viewport.y) + (btn_height * 0.6f);

    if(tick_menu_image_button(HUD_CLIP_BACK, bx,by) || is_key_pressed(KeyCode_Escape))
    {
        g_menu.screen = MenuScreen_Main;
    }
}

INTERNAL void draw_back_button(void)
{
    nkF32 img_scale = (get_hud_scale() / 4.0f);

    nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
    nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

    nkF32 bx = (g_menu.viewport.x) + (btn_width * 0.6f);
    nkF32 by = (g_menu.viewport.y) + (btn_height * 0.6f);

    draw_menu_image_button(HUD_CLIP_BACK, bx,by);
}

INTERNAL void menu_tick_main(nkF32 dt)
{
    // Update the sliding animation and sound effects.
    PERSISTENT nkF32 prev_timer = -1.0f;

    if(g_menu.timer < 1.0f)
    {
        if(g_menu.timer > 0.0f)
        {
            g_menu.xoffset[1] = nk_lerp(-(MENU_WIDTH), 0.0f, g_menu.timer);
            g_menu.xoffset[2] = nk_lerp(+(MENU_WIDTH), 0.0f, g_menu.timer);

            if(g_menu.xoffset[1] > 0.0f) g_menu.xoffset[1] = 0.0f;
            if(g_menu.xoffset[2] < 0.0f) g_menu.xoffset[2] = 0.0f;
        }

        prev_timer = g_menu.timer;
        g_menu.timer += (dt * MENU_SLIDE_SPEED);

        if(g_menu.timer >= 1.0f)
        {
            g_menu.timer = 1.0f;
            g_menu.xoffset[1] = 0.0f;
            g_menu.xoffset[2] = 0.0f;
            g_menu.stage++;
        }
    }

    nkBool sound_played = NK_FALSE;

    if(prev_timer <= 0.0f && g_menu.timer > 0.0f)
    {
        play_sound(asset_manager_load<Sound>("whoosh.wav"));
        sound_played = NK_TRUE;
    }
    if(prev_timer < 1.0f && g_menu.timer >= 1.0f)
    {
        play_sound(asset_manager_load<Sound>("thud.wav"));
        sound_played = NK_TRUE;
    }

    if(sound_played)
    {
        prev_timer = g_menu.timer;
    }

    // Do stage-specific update.
    if(g_menu.stage == MenuStage_Interactive)
    {
        if(tick_menu_text_button(MENU_PLAY_TEXT, MENU_PLAY_YPOS, MENU_PLAY_SIZE))
        {
            g_menu.screen = MenuScreen_Difficulty;
        }
        if(tick_menu_text_button(MENU_HOWTO_TEXT, MENU_HOWTO_YPOS, MENU_HOWTO_SIZE))
        {
            g_menu.screen = MenuScreen_Tutorial;
            g_menu.current_tutorial_page = 0;
        }
        if(tick_menu_text_button(MENU_CREDITS_TEXT, MENU_CREDITS_YPOS, MENU_CREDITS_SIZE))
        {
            g_menu.screen = MenuScreen_Credits;
        }

        // We don't need/want an exit button in the web build.
        #if !defined(BUILD_WEB)
        if(tick_menu_text_button(MENU_EXIT_TEXT, MENU_EXIT_YPOS, MENU_EXIT_SIZE) || is_key_pressed(KeyCode_Escape))
        {
            terminate_app();
        }
        #endif // BUILD_WEB

        // Do the options buttons.
        nkF32 img_scale = get_hud_scale() / 4.0f;

        nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
        nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

        nkF32 bx = (g_menu.viewport.x + g_menu.viewport.w) - (btn_width * 0.6f);
        nkF32 by = (g_menu.viewport.y) + (btn_height * 0.6f);

        if(tick_menu_toggle_button(HUD_CLIP_MUSIC,      bx-(btn_width*0.0f),by, is_music_on  ())) set_music_volume((is_music_on()) ? 0.0f : 0.7f);
        if(tick_menu_toggle_button(HUD_CLIP_SOUND,      bx-(btn_width*1.1f),by, is_sound_on  ())) set_sound_volume((is_sound_on()) ? 0.0f : 0.8f);
        if(tick_menu_toggle_button(HUD_CLIP_FULLSCREEN, bx-(btn_width*2.2f),by, is_fullscreen())) set_fullscreen(!is_fullscreen());

        // Is the mouse clicked in the Steam area then open the URL to the store page.
        nkF32 w = STEAM_BUTTON_WIDTH  * g_menu.viewport.w;
        nkF32 h = STEAM_BUTTON_HEIGHT * g_menu.viewport.h;
        nkF32 x = g_menu.viewport.x + (g_menu.viewport.w - w);
        nkF32 y = g_menu.viewport.y + (g_menu.viewport.h - h);

        g_menu.steam_area_hovered = NK_FALSE;

        if(point_vs_rect(get_window_mouse_pos(), x,y,w,h))
        {
            g_menu.steam_area_hovered = NK_TRUE;

            if(is_mouse_button_pressed(MouseButton_Left))
            {
                open_url("https://store.steampowered.com/app/2570080/Grow_Your_Guarden");
                play_sound(asset_manager_load<Sound>("click.wav"));
            }
        }
    }
    else
    {
        // If the mouse is clicked during either of these stages we skip to the next.
        if(is_any_mouse_button_pressed() || is_any_key_pressed())
        {
            g_menu.timer = 1.0f;
            g_menu.xoffset[1] = 0.0f;
            g_menu.xoffset[2] = 0.0f;
            g_menu.stage++;

            if(g_menu.stage == MenuStage_Interactive)
            {
                play_music(asset_manager_load<Music>("menu.ogg"));
                play_sound(asset_manager_load<Sound>("click.wav"));
            }
        }
    }
}

INTERNAL void menu_draw_main(void)
{
    // If interactive draw the rest of the main menu.
    if(g_menu.stage != MenuStage_Interactive) return;

    Texture background = asset_manager_load<Texture>("title.png");
    Texture steam_overlay = asset_manager_load<Texture>("steam.png");

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    // Do the title.
    ImmClip clip = { 0.0f, MENU_HEIGHT * 3, MENU_WIDTH, MENU_HEIGHT };
    imm_texture_ex(background, ww*0.5f, wh*0.5f, g_menu.scale,g_menu.scale, 0.0f, NULL, &clip);

    // Do the version.
    nkString version = format_string("v%d.%d.%d", VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH);

    TrueTypeFont font = get_font();
    set_truetype_font_size(font, VERSION_SIZE);

    nkF32 vx = g_menu.viewport.x + 10.0f;
    nkF32 vy = g_menu.viewport.y + get_truetype_line_height(font);

    draw_truetype_text(font, vx+5,vy+5, version.cstr, NK_V4_BLACK);
    draw_truetype_text(font, vx,vy, version.cstr, { 0.5f,0.5f,0.5f,1.0f });

    // Do the main buttons.
    draw_menu_text_button(MENU_PLAY_TEXT, MENU_PLAY_YPOS, MENU_PLAY_SIZE);
    draw_menu_text_button(MENU_HOWTO_TEXT, MENU_HOWTO_YPOS, MENU_HOWTO_SIZE);
    draw_menu_text_button(MENU_CREDITS_TEXT, MENU_CREDITS_YPOS, MENU_CREDITS_SIZE);
    // We don't need/want an exit button in the web build.
    #if !defined(BUILD_WEB)
    draw_menu_text_button(MENU_EXIT_TEXT, MENU_EXIT_YPOS, MENU_EXIT_SIZE);
    #endif // BUILD_WEB

    // Do the options buttons.
    nkF32 img_scale = (get_hud_scale() / 4.0f);

    nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
    nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

    nkF32 bx = (g_menu.viewport.x + g_menu.viewport.w) - (btn_width * 0.6f);
    nkF32 by = (g_menu.viewport.y) + (btn_height * 0.6f);

    draw_menu_toggle_button(HUD_CLIP_MUSIC,      bx-(btn_width*0.0f),by, is_music_on  ());
    draw_menu_toggle_button(HUD_CLIP_SOUND,      bx-(btn_width*1.1f),by, is_sound_on  ());
    draw_menu_toggle_button(HUD_CLIP_FULLSCREEN, bx-(btn_width*2.2f),by, is_fullscreen());

    // Do the Steam overlay.
    nkVec4 highlight_color = (g_menu.steam_area_hovered) ? nkVec4 { 2.2f,2.2f,2.2f,1.0f } : NK_V4_WHITE;
    imm_texture_ex(steam_overlay, ww*0.5f, wh*0.5f, g_menu.scale,g_menu.scale, 0.0f, NULL, NULL, highlight_color);
}

INTERNAL void menu_tick_tutorial(nkF32 dt)
{
    // Handle the increment and decrement page buttons.
    nkF32 img_scale = (get_hud_scale() / 4.0f);

    nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
    nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

    nkF32 bx = (g_menu.viewport.x + (g_menu.viewport.w * 0.5f));
    nkF32 by = (g_menu.viewport.y + g_menu.viewport.h) - (btn_height * 0.6f);

    if(g_menu.current_tutorial_page > 0)
    {
        if(tick_menu_image_button(HUD_CLIP_PAGE_L, bx-(btn_width*1.0f),by))
        {
            play_sound(g_menu.page_flip_sound[rng_s32(0,NK_ARRAY_SIZE(g_menu.page_flip_sound)-1)]);
            g_menu.current_tutorial_page--;
        }
    }
    if(g_menu.current_tutorial_page < (MENU_TUTORIAL_PAGE_COUNT-1))
    {
        if(tick_menu_image_button(HUD_CLIP_PAGE_R, bx+(btn_width*1.0f),by))
        {
            play_sound(g_menu.page_flip_sound[rng_s32(0,NK_ARRAY_SIZE(g_menu.page_flip_sound)-1)]);
            g_menu.current_tutorial_page++;
        }
    }

    // Handle the back button.
    tick_back_button();
}

INTERNAL void menu_draw_tutorial(void)
{
    // Draw the current page of the tutorial.
    nkString page_number = format_string("%02d", g_menu.current_tutorial_page);
    nkString texture_file;
    nk_string_assign(&texture_file, "tutorial/");
    nk_string_append(&texture_file, &page_number);
    nk_string_append(&texture_file, ".png");

    Texture texture = asset_manager_load<Texture>(texture_file.cstr);
    if(texture)
    {
        nkF32 ww = NK_CAST(nkF32, get_window_width());
        nkF32 wh = NK_CAST(nkF32, get_window_height());

        imm_texture_ex(texture, ww*0.5f, wh*0.5f, g_menu.scale,g_menu.scale, 0.0f, NULL);
    }

    // Draw the current page number.
    TrueTypeFont font = get_font();

    set_truetype_font_size(font, 40);

    nkF32 img_scale = (get_hud_scale() / 4.0f);

    nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
    nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

    nkF32 bx = (g_menu.viewport.x + (g_menu.viewport.w * 0.5f));
    nkF32 by = (g_menu.viewport.y + g_menu.viewport.h) - (btn_height * 0.6f);

    nkF32 tx = bx - (get_truetype_text_width(font, page_number.cstr) * 0.5f);
    nkF32 ty = by + (get_truetype_text_height(font, page_number.cstr) * 0.25f);

    draw_truetype_text(font, tx+4,ty+4, page_number.cstr, NK_V4_BLACK);
    draw_truetype_text(font, tx,ty, page_number.cstr, NK_V4_WHITE);

    // Draw the increment and decrement page buttons.
    if(g_menu.current_tutorial_page > 0)
    {
        draw_menu_image_button(HUD_CLIP_PAGE_L, bx-(btn_width*1.0f),by);
    }
    if(g_menu.current_tutorial_page < (MENU_TUTORIAL_PAGE_COUNT-1))
    {
        draw_menu_image_button(HUD_CLIP_PAGE_R, bx+(btn_width*1.0f),by);
    }

    // Draw the back button.
    draw_back_button();
}

INTERNAL void menu_tick_credits(nkF32 dt)
{
    tick_back_button();
}

INTERNAL void menu_draw_credits(void)
{
    Texture credits = asset_manager_load<Texture>("credits.png");

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    imm_texture_ex(credits, ww*0.5f, wh*0.5f, g_menu.scale,g_menu.scale, 0.0f, NULL);

    draw_back_button();
}

INTERNAL void menu_tick_difficulty(nkF32 dt)
{
    tick_menu_text_button(MENU_DIFFICULTY_INFO_TEXT, MENU_DIFFICULTY_INFO_YPOS, MENU_DIFFICULTY_INFO_SIZE, NK_FALSE);

    // Handle the difficulty selection buttons.
    if(tick_menu_text_button(MENU_DIFFICULTY_EASY_TEXT, MENU_DIFFICULTY_EASY_YPOS, MENU_DIFFICULTY_EASY_SIZE))
    {
        set_spawn_multiplier(0.5f);
        set_coin_multiplier(1.5f);
        set_bonus_multiplier(2.0f);
        set_prep_multiplier(1.5f);
        game_start();
        g_menu.screen = MenuScreen_Main;
    }
    if(tick_menu_text_button(MENU_DIFFICULTY_NORMAL_TEXT, MENU_DIFFICULTY_NORMAL_YPOS, MENU_DIFFICULTY_NORMAL_SIZE))
    {
        set_spawn_multiplier(1.0f);
        set_coin_multiplier(1.0f);
        set_bonus_multiplier(1.0f);
        set_prep_multiplier(1.0f);
        game_start();
        g_menu.screen = MenuScreen_Main;
    }
    if(tick_menu_text_button(MENU_DIFFICULTY_HARD_TEXT, MENU_DIFFICULTY_HARD_YPOS, MENU_DIFFICULTY_HARD_SIZE))
    {
        set_spawn_multiplier(2.0f);
        set_coin_multiplier(1.5f);
        set_bonus_multiplier(2.0f);
        set_prep_multiplier(0.5f);
        game_start();
        g_menu.screen = MenuScreen_Main;
    }

    // Handle the back button.
    tick_back_button();
}

INTERNAL void menu_draw_difficulty(void)
{
    draw_menu_text_button(MENU_DIFFICULTY_INFO_TEXT, MENU_DIFFICULTY_INFO_YPOS, MENU_DIFFICULTY_INFO_SIZE, NK_FALSE);

    // Draw the difficulty selection buttons.
    draw_menu_text_button(MENU_DIFFICULTY_EASY_TEXT, MENU_DIFFICULTY_EASY_YPOS, MENU_DIFFICULTY_EASY_SIZE);
    draw_menu_text_button(MENU_DIFFICULTY_NORMAL_TEXT, MENU_DIFFICULTY_NORMAL_YPOS, MENU_DIFFICULTY_NORMAL_SIZE);
    draw_menu_text_button(MENU_DIFFICULTY_HARD_TEXT, MENU_DIFFICULTY_HARD_YPOS, MENU_DIFFICULTY_HARD_SIZE);

    // Draw the back button.
    draw_back_button();
}

GLOBAL void menu_init(void)
{
    asset_manager_load<Sound>("click.wav");
    asset_manager_load<Sound>("whoosh.wav");
    asset_manager_load<Sound>("thud.wav");
    asset_manager_load<Texture>("title.png");
    asset_manager_load<Texture>("credits.png");
    asset_manager_load<Music>("menu.ogg");

    g_menu.page_flip_sound[ 0] = asset_manager_load<Sound>("page_flip_000.wav");
    g_menu.page_flip_sound[ 1] = asset_manager_load<Sound>("page_flip_001.wav");
    g_menu.page_flip_sound[ 2] = asset_manager_load<Sound>("page_flip_002.wav");
    g_menu.page_flip_sound[ 3] = asset_manager_load<Sound>("page_flip_003.wav");
    g_menu.page_flip_sound[ 4] = asset_manager_load<Sound>("page_flip_004.wav");
    g_menu.page_flip_sound[ 5] = asset_manager_load<Sound>("page_flip_005.wav");
    g_menu.page_flip_sound[ 6] = asset_manager_load<Sound>("page_flip_006.wav");
    g_menu.page_flip_sound[ 7] = asset_manager_load<Sound>("page_flip_007.wav");
    g_menu.page_flip_sound[ 8] = asset_manager_load<Sound>("page_flip_008.wav");
    g_menu.page_flip_sound[ 9] = asset_manager_load<Sound>("page_flip_009.wav");
    g_menu.page_flip_sound[10] = asset_manager_load<Sound>("page_flip_010.wav");

    g_menu.screen     = MenuScreen_Main;
    g_menu.stage      = MenuStage_Animation;
    g_menu.xoffset[1] = -(MENU_WIDTH);
    g_menu.xoffset[2] = +(MENU_WIDTH);
    g_menu.timer      = -1.0f;
}

GLOBAL void menu_quit(void)
{
    // Does nothing...
}

GLOBAL void menu_tick(nkF32 dt)
{
    // Calculate the viewport region within the window.
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());
    nkF32 vw = MENU_WIDTH;
    nkF32 vh = MENU_HEIGHT;

    nkF32 sx = ww / vw;
    nkF32 sy = wh / vh;
    nkF32 sw = vw;
    nkF32 sh = vh;

    if(fabsf(sx) < fabsf(sy)) sh = roundf((vh/vw)*sw);
    if(fabsf(sx) > fabsf(sy)) sw = roundf((vw/vh)*sh);

    g_menu.scale = nk_min(ww / sw, wh / sh);

    g_menu.viewport.w = (sw * g_menu.scale);
    g_menu.viewport.h = (sh * g_menu.scale);
    g_menu.viewport.x = (ww - g_menu.viewport.w) * 0.5f;
    g_menu.viewport.y = (wh - g_menu.viewport.h) * 0.5f;

    // Do state-specific menu update.
    switch(g_menu.screen)
    {
        case MenuScreen_Main: menu_tick_main(dt); break;
        case MenuScreen_Tutorial: menu_tick_tutorial(dt); break;
        case MenuScreen_Credits: menu_tick_credits(dt); break;
        case MenuScreen_Difficulty: menu_tick_difficulty(dt); break;
    }
}

GLOBAL void menu_draw(void)
{
    // Draw the title screen background.
    Texture background = asset_manager_load<Texture>("title.png");

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    begin_scissor((ww-(MENU_WIDTH*g_menu.scale))*0.5f, (wh-(MENU_HEIGHT*g_menu.scale))*0.5f, (MENU_WIDTH*g_menu.scale), (MENU_HEIGHT*g_menu.scale));
    for(nkS32 i=0; i<(MENU_LAYERS-1); ++i) // There are multiple layers (we don't include the title at this stage).
    {
        ImmClip clip = { 0.0f, MENU_HEIGHT * i, MENU_WIDTH, MENU_HEIGHT };
        imm_texture_ex(background, (ww*0.5f) + (g_menu.xoffset[i]*g_menu.scale), wh*0.5f, g_menu.scale,g_menu.scale, 0.0f, NULL, &clip);
    }
    end_scissor();

    if(g_menu.stage == MenuStage_Interactive)
    {
        imm_rect_filled(0.0f,0.0f,ww,wh, { 0.0f,0.0f,0.0f,0.6f });
    }

    // Do state-specific menu draw.
    switch(g_menu.screen)
    {
        case MenuScreen_Main: menu_draw_main(); break;
        case MenuScreen_Tutorial: menu_draw_tutorial(); break;
        case MenuScreen_Credits: menu_draw_credits(); break;
        case MenuScreen_Difficulty: menu_draw_difficulty(); break;
    }
}

GLOBAL nkBool tick_menu_text_button(const nkChar* text, nkF32 y, nkS32 size, nkBool interactive)
{
    PERSISTENT nkF32 previous_y = 0.0f;

    TrueTypeFont font = get_font();

    set_truetype_font_size(font, size);

    fRect t = calculate_menu_button_bounds(font, text, previous_y, y);

    nkVec2 cursor_pos = get_window_mouse_pos();

    nkBool clicked = (point_vs_rect(cursor_pos, t.x,t.y-t.h,t.w,t.h) && is_mouse_button_pressed(MouseButton_Left) && interactive);
    if(clicked)
        play_sound(asset_manager_load<Sound>("click.wav"));
    return clicked;
}

GLOBAL void draw_menu_text_button(const nkChar* text, nkF32 y, nkS32 size, nkBool interactive)
{
    PERSISTENT nkF32 previous_y = 0.0f;

    TrueTypeFont font = get_font();

    set_truetype_font_size(font, size);

    fRect t = calculate_menu_button_bounds(font, text, previous_y, y);

    nkVec2 cursor_pos = get_window_mouse_pos();

    nkVec4 color = (point_vs_rect(cursor_pos, t.x,t.y-t.h,t.w,t.h) && interactive) ? NK_V4_YELLOW : NK_V4_WHITE;

    draw_truetype_text(font, t.x+5,t.y+5, text, NK_V4_BLACK);
    draw_truetype_text(font, t.x,t.y, text, color);
}

INTERNAL nkBool tick_menu_image_button(ImmClip clip, nkF32 x, nkF32 y)
{
    nkF32 img_scale = (get_hud_scale() / 4.0f);

    nkF32 bx = x - ((HUD_ICON_WIDTH * img_scale) * 0.5f);
    nkF32 by = y - ((HUD_ICON_HEIGHT * img_scale) * 0.5f);
    nkF32 bw = (HUD_ICON_WIDTH * img_scale);
    nkF32 bh = (HUD_ICON_HEIGHT * img_scale);

    nkVec2 cursor_pos = get_window_mouse_pos();

    nkBool clicked = (point_vs_rect(cursor_pos, bx,by,bw,bh) && is_mouse_button_pressed(MouseButton_Left));
    if(clicked)
        play_sound(asset_manager_load<Sound>("click.wav"));
    return clicked;
}

INTERNAL void draw_menu_image_button(ImmClip clip, nkF32 x, nkF32 y)
{
    nkF32 img_scale = (get_hud_scale() / 4.0f);

    nkF32 bx = x - ((HUD_ICON_WIDTH * img_scale) * 0.5f);
    nkF32 by = y - ((HUD_ICON_HEIGHT * img_scale) * 0.5f);
    nkF32 bw = (HUD_ICON_WIDTH * img_scale);
    nkF32 bh = (HUD_ICON_HEIGHT * img_scale);

    nkVec2 cursor_pos = get_window_mouse_pos();

    if(point_vs_rect(cursor_pos, bx,by,bw,bh)) clip.y += HUD_ICON_HEIGHT;

    Texture texture = asset_manager_load<Texture>("hud.png");
    imm_texture_ex(texture, x,y, img_scale,img_scale, 0.0f, NULL, &clip);
}

INTERNAL nkBool tick_menu_toggle_button(ImmClip clip, nkF32 x, nkF32 y, nkBool toggle)
{
    nkF32 img_scale = (get_hud_scale() / 4.0f);

    nkF32 bx = x - ((HUD_ICON_WIDTH * img_scale) * 0.5f);
    nkF32 by = y - ((HUD_ICON_HEIGHT * img_scale) * 0.5f);
    nkF32 bw = (HUD_ICON_WIDTH * img_scale);
    nkF32 bh = (HUD_ICON_HEIGHT * img_scale);

    nkVec2 cursor_pos = get_window_mouse_pos();

    return (point_vs_rect(cursor_pos, bx,by,bw,bh) && is_mouse_button_pressed(MouseButton_Left));
}

INTERNAL void draw_menu_toggle_button(ImmClip clip, nkF32 x, nkF32 y, nkBool toggle)
{
    if(!toggle) clip.y += HUD_ICON_HEIGHT;
    Texture texture = asset_manager_load<Texture>("hud.png");
    nkF32 img_scale = (get_hud_scale() / 4.0f);
    imm_texture_ex(texture, x,y, img_scale,img_scale, 0.0f, NULL, &clip);
}

/*////////////////////////////////////////////////////////////////////////////*/
