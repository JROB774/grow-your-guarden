/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr const nkChar* MENU_TITLE_TEXT = "GROW YOUR GUARDEN!";
INTERNAL constexpr const nkChar* MENU_PLAY_TEXT  = "Play";
INTERNAL constexpr const nkChar* MENU_HOWTO_TEXT  = "How to Play";
INTERNAL constexpr const nkChar* MENU_EXIT_TEXT  = "Exit";

INTERNAL constexpr nkS32 MENU_TITLE_SIZE = 100;
INTERNAL constexpr nkS32 MENU_PLAY_SIZE  =  50;
INTERNAL constexpr nkS32 MENU_HOWTO_SIZE =  35;
INTERNAL constexpr nkS32 MENU_EXIT_SIZE  =  35;

INTERNAL constexpr nkF32 MENU_TITLE_YPOS =  0.40f;
INTERNAL constexpr nkF32 MENU_PLAY_YPOS  =  0.65f;
INTERNAL constexpr nkF32 MENU_HOWTO_YPOS = -1.00f;
INTERNAL constexpr nkF32 MENU_EXIT_YPOS  = -1.00f;

INTERNAL constexpr nkF32 MENU_SLIDE_SPEED = 2.0f;

INTERNAL constexpr nkS32 MENU_LAYERS = 3;

INTERNAL constexpr nkF32 MENU_WIDTH = 1280.0f;
INTERNAL constexpr nkF32 MENU_HEIGHT = 720.0f;

NK_ENUM(MenuStage, nkS32)
{
    MenuStage_Animation,
    MenuStage_PressAnyButton,
    MenuStage_Interactive,
    MenuStage_TOTAL
};

struct MenuState
{
    MenuStage stage;
    nkF32     xoffset[MENU_LAYERS];
    nkF32     timer;
};

INTERNAL MenuState g_menu;

INTERNAL fRect calculate_menu_button_bounds(TrueTypeFont font, const nkChar* text, nkF32& prev_y, nkF32 curr_y)
{
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    // We multiply by a percentage because it gives just a bit of a gap between the buttons which is nice.
    if(curr_y <= -1.0f) curr_y = ((wh * prev_y) + (get_truetype_line_height(font) * 1.3f)) / wh; // Increment from the previous y-position.

    fRect bounds;

    bounds.w = get_truetype_text_width(font, text);
    bounds.h = get_truetype_text_height(font, text);
    bounds.x = (ww - bounds.w) * 0.5f;
    bounds.y = (wh * curr_y);

    prev_y = curr_y;

    return bounds;
}

GLOBAL void menu_init(void)
{
    asset_manager_load<Texture>("title.png");

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
    // Update the sliding animation.
    if(g_menu.timer < 1.0f)
    {
        if(g_menu.timer > 0.0f)
        {
            g_menu.xoffset[1] = nk_lerp(-(MENU_WIDTH), 0.0f, g_menu.timer);
            g_menu.xoffset[2] = nk_lerp(+(MENU_WIDTH), 0.0f, g_menu.timer);

            if(g_menu.xoffset[1] > 0.0f) g_menu.xoffset[1] = 0.0f;
            if(g_menu.xoffset[2] < 0.0f) g_menu.xoffset[2] = 0.0f;
        }

        g_menu.timer += (dt * MENU_SLIDE_SPEED);
        if(g_menu.timer >= 1.0f)
        {
            g_menu.timer = 1.0f;
            g_menu.xoffset[1] = 0.0f;
            g_menu.xoffset[2] = 0.0f;
            g_menu.stage++;
        }
    }

    // Do state-specific update.
    if(g_menu.stage == MenuStage_Interactive)
    {
        if(tick_menu_text_button(MENU_PLAY_TEXT, MENU_PLAY_YPOS, MENU_PLAY_SIZE))
        {
            game_start();
        }
        if(tick_menu_text_button(MENU_HOWTO_TEXT, MENU_HOWTO_YPOS, MENU_HOWTO_SIZE))
        {
            // @Incomplete: How to play pages...
        }

        // We don't need/want an exit button in the web build.
        #if !defined(BUILD_WEB)
        if(tick_menu_text_button(MENU_EXIT_TEXT, MENU_EXIT_YPOS, MENU_EXIT_SIZE))
        {
            terminate_app();
        }
        #endif // BUILD_WEB

        // Do the options buttons.
        nkF32 img_scale = get_hud_scale() / 4.0f;

        nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
        nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

        nkF32 bx = NK_CAST(nkF32, get_window_width()) - (btn_width * 0.5f);
        nkF32 by = NK_CAST(nkF32, get_window_height()) - (btn_height * 0.5f);

        if(tick_menu_toggle_button(HUD_CLIP_MUSIC,      bx-(btn_width*0.0f),by, is_music_on  ())) set_music_volume((is_music_on()) ? 0.0f : 0.7f);
        if(tick_menu_toggle_button(HUD_CLIP_SOUND,      bx-(btn_width*1.1f),by, is_sound_on  ())) set_sound_volume((is_sound_on()) ? 0.0f : 0.8f);
        if(tick_menu_toggle_button(HUD_CLIP_FULLSCREEN, bx-(btn_width*2.2f),by, is_fullscreen())) set_fullscreen(!is_fullscreen());
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
        }
    }
}

GLOBAL void menu_draw(void)
{
    // Draw the title screen background.
    Texture background = asset_manager_load<Texture>("title.png");

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());
    nkF32 tw = MENU_WIDTH;
    nkF32 th = MENU_HEIGHT;

    nkF32 sx = ww / tw;
    nkF32 sy = wh / th;
    nkF32 sw = tw;
    nkF32 sh = th;

    if(fabsf(sx) < fabsf(sy)) sh = roundf((th/tw)*sw);
    if(fabsf(sx) > fabsf(sy)) sw = roundf((tw/th)*sh);

    nkF32 scale = nk_min(ww / sw, wh / sh);

    begin_scissor((ww-(MENU_WIDTH*scale))*0.5f, (wh-(MENU_HEIGHT*scale))*0.5f, (MENU_WIDTH*scale), (MENU_HEIGHT*scale));
    for(nkS32 i=0; i<MENU_LAYERS; ++i) // There are multiple layers.
    {
        ImmClip clip = { 0.0f, MENU_HEIGHT * i, MENU_WIDTH, MENU_HEIGHT };
        imm_texture_ex(background, (ww*0.5f) + (g_menu.xoffset[i]*scale), wh*0.5f, scale,scale, 0.0f, NULL, &clip);
    }
    end_scissor();

    // If interactive draw the rest of the menu background.
    if(g_menu.stage == MenuStage_Interactive)
    {
        imm_rect_filled(0.0f,0.0f,ww,wh, { 0.0f,0.0f,0.0f,0.7f });

        // Do the title and main buttons.
        draw_menu_text_button(MENU_TITLE_TEXT, MENU_TITLE_YPOS, MENU_TITLE_SIZE, NK_FALSE);
        draw_menu_text_button(MENU_PLAY_TEXT, MENU_PLAY_YPOS, MENU_PLAY_SIZE);
        draw_menu_text_button(MENU_HOWTO_TEXT, MENU_HOWTO_YPOS, MENU_HOWTO_SIZE);
        // We don't need/want an exit button in the web build.
        #if !defined(BUILD_WEB)
        draw_menu_text_button(MENU_EXIT_TEXT, MENU_EXIT_YPOS, MENU_EXIT_SIZE);
        #endif // BUILD_WEB

        // Do the options buttons.
        nkF32 img_scale = (get_hud_scale() / 4.0f);

        nkF32 btn_width = HUD_ICON_WIDTH * img_scale;
        nkF32 btn_height = HUD_ICON_HEIGHT * img_scale;

        nkF32 bx = NK_CAST(nkF32, get_window_width()) - (btn_width * 0.6f);
        nkF32 by = NK_CAST(nkF32, get_window_height()) - (btn_height * 0.6f);

        draw_menu_toggle_button(HUD_CLIP_MUSIC,      bx-(btn_width*0.0f),by, is_music_on  ());
        draw_menu_toggle_button(HUD_CLIP_SOUND,      bx-(btn_width*1.1f),by, is_sound_on  ());
        draw_menu_toggle_button(HUD_CLIP_FULLSCREEN, bx-(btn_width*2.2f),by, is_fullscreen());
    }
}

GLOBAL nkBool tick_menu_text_button(const nkChar* text, nkF32 y, nkS32 size, nkBool interactive)
{
    PERSISTENT nkF32 previous_y = 0.0f;

    TrueTypeFont font = get_font();

    set_truetype_font_size(font, size);

    fRect t = calculate_menu_button_bounds(font, text, previous_y, y);

    nkVec2 cursor_pos = get_window_mouse_pos();

    return (point_vs_rect(cursor_pos, t.x,t.y-t.h,t.w,t.h) && is_mouse_button_pressed(MouseButton_Left) && interactive);
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
