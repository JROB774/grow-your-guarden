/*////////////////////////////////////////////////////////////////////////////*/

DEFINE_PRIVATE_TYPE(BitmapFont)
{
    nkF32   px_height;
    Texture atlas;
    ImmClip glyphs[128];
};

GLOBAL BitmapFont create_bitmap_font(void* data, nkU64 bytes)
{
    BitmapFont font = ALLOCATE_PRIVATE_TYPE(BitmapFont);
    if(!font) fatal_error("Failed to allocate bitmap font!");

    // Allocate a copy of the input data so that we can operate on it.
    nkU64 buffer_size = (bytes+1)*sizeof(nkChar);
    nkChar* buffer = NK_CALLOC_TYPES(nkChar, buffer_size);
    if(!buffer) fatal_error("Failed to allocate bitmap font buffer!");
    strncpy(buffer, NK_CAST(nkChar*,data), bytes);

    nkChar* ptr = buffer;

    // Load the font atlas texture.
    const nkChar* texture_name = str_get_line(&ptr);
    font->atlas = asset_manager_load<Texture>(texture_name);
    if(!font->atlas)
        fatal_error("Failed to load bitmap font atlas!");

    // Load the glyph dimensions.
    str_eat_space(&ptr);
    nkS32 gw = str_get_s32(&ptr, 16);
    nkS32 gh = str_get_s32(&ptr, 16);
    font->px_height = NK_CAST(nkF32, gh);

    // Build the glyph map.
    nkS32 x = 0;
    nkS32 y = 0;

    nkS32 tw = get_texture_width(font->atlas);
    for(nkS32 i=0; i<NK_ARRAY_SIZE(font->glyphs); ++i)
    {
        str_eat_space(&ptr);

        nkS32 w = str_get_s32(&ptr, 16);

        font->glyphs[i].x = NK_CAST(nkF32, x);
        font->glyphs[i].y = NK_CAST(nkF32, y);
        font->glyphs[i].w = NK_CAST(nkF32, w);
        font->glyphs[i].h = NK_CAST(nkF32,gh);

        x += gw;

        if(x >= tw)
        {
            x = 0;
            y += gh;
        }
    }

    NK_FREE(buffer);

    return font;
}

GLOBAL void free_bitmap_font(BitmapFont font)
{
    NK_ASSERT(font);
    NK_FREE(font);
}

GLOBAL nkVec2 get_bitmap_text_bounds(BitmapFont font, const nkChar* text)
{
    NK_ASSERT(font);

    nkF32 line_w = 0.0f;

    nkF32 w = 0.0f;
    nkF32 h = font->px_height;

    for(nkU64 i=0,n=strlen(text); i<n; ++i)
    {
        nkChar c = text[i];
        if(c != '\n')
            line_w += font->glyphs[NK_CAST(nkS32, c)].w;
        else
        {
            w = nk_max(w,line_w);
            line_w = 0.0f;
            h += font->px_height;
        }
    }

    w = nk_max(w,line_w);

    nkVec2 bounds;
    bounds.x = w;
    bounds.y = h;
    return bounds;
}

GLOBAL nkF32 get_bitmap_text_width(BitmapFont font, const nkChar* text)
{
    NK_ASSERT(font);
    return get_bitmap_text_bounds(font, text).x;
}

GLOBAL nkF32 get_bitmap_text_height(BitmapFont font, const nkChar* text)
{
    NK_ASSERT(font);
    return get_bitmap_text_bounds(font, text).y;
}

GLOBAL nkF32 get_bitmap_font_px_height(BitmapFont font)
{
    NK_ASSERT(font);
    return font->px_height;
}

GLOBAL void draw_bitmap_text(BitmapFont font, nkF32 x, nkF32 y, const nkChar* text, nkVec4 color)
{
    NK_ASSERT(font);
    NK_ASSERT(text);

    nkF32 start_x = x;

    if(strlen(text) == 0)
    {
        return;
    }

    imm_begin_texture_batch(font->atlas);
    while(*text)
    {
        if(*text == '\n')
        {
            x = start_x;
            y += font->px_height;
        }
        else
        {
            ImmClip* clip = &font->glyphs[NK_CAST(nkS32, *text)];
            x += clip->w * 0.5f;
            imm_texture_batched(x,y, clip, color);
            x += clip->w * 0.5f;
        }

        ++text;
    }
    imm_end_texture_batch();
}

GLOBAL void draw_bitmap_char(BitmapFont font, nkF32 x, nkF32 y, nkChar chr, nkVec4 color)
{
    nkChar buffer[2] = { chr, '\0' };
    draw_bitmap_text(font, x,y, buffer, color);
}

/*////////////////////////////////////////////////////////////////////////////*/
