/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr const nkChar* MENU_TITLE_TEXT = "GROW YOUR GUARDEN!";
INTERNAL constexpr const nkChar* MENU_PLAY_TEXT  = "Start Game";
INTERNAL constexpr const nkChar* MENU_EXIT_TEXT  = "Exit";

INTERNAL constexpr nkS32 MENU_TITLE_SIZE = 100;
INTERNAL constexpr nkS32 MENU_PLAY_SIZE  =  40;
INTERNAL constexpr nkS32 MENU_EXIT_SIZE  =  35;

INTERNAL constexpr nkF32 MENU_TITLE_YPOS = 0.40f;
INTERNAL constexpr nkF32 MENU_PLAY_YPOS  = 0.65f;
INTERNAL constexpr nkF32 MENU_EXIT_YPOS  = 0.70f;

INTERNAL nkBool tick_menu_button(const nkChar* text, nkF32 y, nkS32 size, nkBool interactive = NK_TRUE)
{
    TrueTypeFont font = asset_manager_load<TrueTypeFont>("helsinki.ttf");

    set_truetype_font_size(font, size);

    nkVec2 cursor_pos = get_window_mouse_pos();

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkF32 tw = get_truetype_text_width(font, text);
    nkF32 th = get_truetype_text_height(font, text);
    nkF32 tx = (ww - tw) * 0.5f;
    nkF32 ty = (wh * y);

    return (point_vs_rect(cursor_pos, tx,ty-th,tw,th) && is_mouse_button_pressed(MouseButton_Left) && interactive);
}

INTERNAL void draw_menu_button(const nkChar* text, nkF32 y, nkS32 size, nkBool interactive = NK_TRUE)
{
    TrueTypeFont font = asset_manager_load<TrueTypeFont>("helsinki.ttf");

    set_truetype_font_size(font, size);

    nkVec2 cursor_pos = get_window_mouse_pos();

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    nkF32 tw = get_truetype_text_width(font, text);
    nkF32 th = get_truetype_text_height(font, text);
    nkF32 tx = (ww - tw) * 0.5f;
    nkF32 ty = (wh * y);

    nkVec4 color = (point_vs_rect(cursor_pos, tx,ty-th,tw,th) && interactive) ? NK_V4_YELLOW : NK_V4_WHITE;

    draw_truetype_text(font, tx+5,ty+5, text, NK_V4_BLACK);
    draw_truetype_text(font, tx,ty, text, color);
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
    tick_menu_button(MENU_TITLE_TEXT, MENU_TITLE_YPOS, MENU_TITLE_SIZE, NK_FALSE);

    if(tick_menu_button(MENU_PLAY_TEXT, MENU_PLAY_YPOS, MENU_PLAY_SIZE))
    {
        game_start();
    }
    if(tick_menu_button(MENU_EXIT_TEXT, MENU_EXIT_YPOS, MENU_EXIT_SIZE))
    {
        terminate_app();
    }
}

GLOBAL void menu_draw(void)
{
    draw_menu_button(MENU_TITLE_TEXT, MENU_TITLE_YPOS, MENU_TITLE_SIZE, NK_FALSE);
    draw_menu_button(MENU_PLAY_TEXT, MENU_PLAY_YPOS, MENU_PLAY_SIZE);
    draw_menu_button(MENU_EXIT_TEXT, MENU_EXIT_YPOS, MENU_EXIT_SIZE);
}

/*////////////////////////////////////////////////////////////////////////////*/
