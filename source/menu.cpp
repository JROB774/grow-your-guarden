/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void menu_tick(nkF32 dt)
{
    if(is_mouse_button_pressed(MouseButton_Left))
    {
        game_start();
    }
}

GLOBAL void menu_draw(void)
{
    TrueTypeFont font = asset_manager_load<TrueTypeFont>("helsinki.ttf");

    // Draw the title.
    set_truetype_font_size(font, 80);

    const nkChar* title_text = "Grow Your Guard-en!";

    nkF32 tx = (NK_CAST(nkF32, get_window_width()) - get_truetype_text_width(font, title_text)) * 0.5f;
    nkF32 ty = (NK_CAST(nkF32, get_window_height()) * 0.25f);

    draw_truetype_text(font, tx+5,ty+5, title_text, NK_V4_BLACK);
    draw_truetype_text(font, tx,ty, title_text, NK_V4_WHITE);

    // Draw the prompt.
    set_truetype_font_size(font, 20);

    const nkChar* prompt_text = "Left-Click to Start";

    nkF32 px = (NK_CAST(nkF32, get_window_width()) - get_truetype_text_width(font, prompt_text)) * 0.5f;
    nkF32 py = (NK_CAST(nkF32, get_window_height()) * 0.85f);

    draw_truetype_text(font, px+2,py+2, prompt_text, NK_V4_BLACK);
    draw_truetype_text(font, px,py, prompt_text, NK_V4_WHITE);
}

/*////////////////////////////////////////////////////////////////////////////*/
