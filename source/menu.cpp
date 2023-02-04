/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr const nkChar* MENU_TITLE_TEXT = "GROW YOUR GUARDEN!";
INTERNAL constexpr const nkChar* MENU_PLAY_TEXT  = "Start Game";
INTERNAL constexpr const nkChar* MENU_EXIT_TEXT  = "Exit";

INTERNAL constexpr nkS32 MENU_TITLE_SIZE = 100;
INTERNAL constexpr nkS32 MENU_PLAY_SIZE  =  40;
INTERNAL constexpr nkS32 MENU_EXIT_SIZE  =  35;

INTERNAL constexpr nkF32 MENU_TITLE_YPOS =  0.40f;
INTERNAL constexpr nkF32 MENU_PLAY_YPOS  =  0.65f;
INTERNAL constexpr nkF32 MENU_EXIT_YPOS  = -1.00f;

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
    // Does nothing...
}

GLOBAL void menu_quit(void)
{
    // Does nothing...
}

GLOBAL void menu_tick(nkF32 dt)
{
    if(tick_menu_button(MENU_PLAY_TEXT, MENU_PLAY_YPOS, MENU_PLAY_SIZE))
    {
        game_start();
    }

    // We don't need/want an exit button in the web build.
    #if !defined(BUILD_WEB)
    if(tick_menu_button(MENU_EXIT_TEXT, MENU_EXIT_YPOS, MENU_EXIT_SIZE))
    {
        terminate_app();
    }
    #endif // BUILD_WEB
}

GLOBAL void menu_draw(void)
{
    draw_menu_button(MENU_TITLE_TEXT, MENU_TITLE_YPOS, MENU_TITLE_SIZE, NK_FALSE);
    draw_menu_button(MENU_PLAY_TEXT, MENU_PLAY_YPOS, MENU_PLAY_SIZE);

    // We don't need/want an exit button in the web build.
    #if !defined(BUILD_WEB)
    draw_menu_button(MENU_EXIT_TEXT, MENU_EXIT_YPOS, MENU_EXIT_SIZE);
    #endif // BUILD_WEB
}

GLOBAL nkBool tick_menu_button(const nkChar* text, nkF32 y, nkS32 size, nkBool interactive)
{
    PERSISTENT nkF32 previous_y = 0.0f;

    TrueTypeFont font = get_font();

    set_truetype_font_size(font, size);

    fRect t = calculate_menu_button_bounds(font, text, previous_y, y);

    nkVec2 cursor_pos = get_window_mouse_pos();

    return (point_vs_rect(cursor_pos, t.x,t.y-t.h,t.w,t.h) && is_mouse_button_pressed(MouseButton_Left) && interactive);
}

GLOBAL void draw_menu_button(const nkChar* text, nkF32 y, nkS32 size, nkBool interactive)
{
    PERSISTENT nkF32 previous_y = 0.0f;

    TrueTypeFont font = get_font();

    set_truetype_font_size(font, size);

    fRect t = calculate_menu_button_bounds(font, text, previous_y, y);

    nkVec2 cursor_pos = get_window_mouse_pos();

    nkVec4 color = (point_vs_rect(cursor_pos, t.x,t.y-t.h,t.w,t.h) && interactive) ? NK_V4_YELLOW : NK_V4_WHITE;

    draw_truetype_text(font, t.x+5,t.y+5, text, NK_V4_BLACK);
    draw_truetype_text(font, t.x,t.y, text, color);

    previous_y = y;
}

/*////////////////////////////////////////////////////////////////////////////*/
