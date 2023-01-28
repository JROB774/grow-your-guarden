/*////////////////////////////////////////////////////////////////////////////*/

#include <ft2build.h>
#include FT_FREETYPE_H

// Converts from FreeType's fixed-point to floating-point.
#define FT_CEIL(x) ((nkF32)(((x + 63) & -64) / 64))

INTERNAL constexpr nkS32 FONT_ATLAS_SIZE    = 512;
INTERNAL constexpr nkS32 FONT_ATLAS_PADDING = 4;

// Glyphs are indexed into a hashmap using this structure because just the codepoint is not enough information. We
// can cache multiple sizes of the same glyph in one atlas so we need to also know the size for correct lookup.
struct GlyphID
{
    nkS32   px_size;
    wchar_t codepoint;
};

DEFINE_PRIVATE_TYPE(TrueTypeFont)
{
    // @Improve: Instead of mapping wchar_t's directly to glyphs we should map wchar_t's to indicies and indicies to
    // glyphs. That way we don't end up with multiple tofus or other redundant glyphs being rasterized in our atlas.

    nkHashMap<nkS32,TrueTypeMetrics> metrics; // Store metrics per-size of the font cached.
    nkHashMap<GlyphID,Glyph>         glyphs;
    nkArray<CharRange>               ranges;
    TrueTypeFontFlags                flags;
    Texture                          atlas_texture;
    nkU8*                            atlas_pixels;
    nkVec2                           atlas_cursor;
    nkF32                            atlas_row_max_height;
    nkS32                            current_size;
    FT_Face                          font_face;
    nkU8*                            data_buffer;
    nkU64                            data_size;
    nkBool                           owns_data;
};

struct TrueTypeFontSystem
{
    FT_Library freetype;
    Shader     font_shader;
};

INTERNAL TrueTypeFontSystem g_truetype;

INTERNAL void bake_font_glyph(TrueTypeFont font, nkS32 size, wchar_t codepoint)
{
    NK_ASSERT(font);

    FT_Error error;

    FT_UInt index = FT_Get_Char_Index(font->font_face, codepoint);
    error = FT_Load_Glyph(font->font_face, index, FT_LOAD_RENDER);
    if(error != 0) fatal_error("Failed to load font glyph! (%d)", error);

    FT_GlyphSlot slot = font->font_face->glyph;
    FT_Bitmap* bitmap = &font->font_face->glyph->bitmap;

    Glyph glyph = NK_ZERO_MEM;

    glyph.info.offset_x =  font->font_face->glyph->bitmap_left;
    glyph.info.offset_y = -font->font_face->glyph->bitmap_top;
    glyph.info.width    =  bitmap->width;
    glyph.info.height   =  bitmap->rows;
    glyph.info.advance  =  FT_CEIL(slot->advance.x);

    // @Improve: Pull these out so the caller can control how much padding...
    nkF32 x_padding = NK_CAST(nkF32, FONT_ATLAS_PADDING);
    nkF32 y_padding = NK_CAST(nkF32, FONT_ATLAS_PADDING);

    if((font->atlas_cursor.x + glyph.info.width) >= FONT_ATLAS_SIZE)
    {
        font->atlas_cursor.y += font->atlas_row_max_height + y_padding;
        font->atlas_cursor.x = 0.0f;
        font->atlas_row_max_height = 0.0f;
    }

    // @Improve: Handle no more space to pack glyphs better!
    NK_ASSERT((font->atlas_cursor.y + glyph.info.height) < FONT_ATLAS_SIZE);

    glyph.bounds.x = font->atlas_cursor.x;
    glyph.bounds.y = font->atlas_cursor.y;
    glyph.bounds.w = NK_CAST(nkF32, glyph.info.width);
    glyph.bounds.h = NK_CAST(nkF32, glyph.info.height);

    for(FT_UInt y=0; y<glyph.bounds.h; ++y)
    {
        void* dst = font->atlas_pixels + NK_CAST(nkS32, (((glyph.bounds.y+y) * FONT_ATLAS_SIZE + glyph.bounds.x)));
        void* src = bitmap->buffer + (y * bitmap->pitch);
        memcpy(dst, src, bitmap->pitch);
    }

    font->atlas_cursor.x += glyph.bounds.w + x_padding;
    font->atlas_row_max_height = nk_max(font->atlas_row_max_height, glyph.bounds.h);

    GlyphID glyph_id = { size, codepoint };
    nk_hashmap_insert(&font->glyphs, glyph_id, glyph);
}

INTERNAL void bake_font_at_size(TrueTypeFont font, nkS32 size)
{
    NK_ASSERT(font);

    if(nk_hashmap_contains(&font->metrics, size))
    {
        printf("[Font]: Attempting to bake font size %d that already exists!\n", size);
        return;
    }

    FT_Error error = FT_Set_Pixel_Sizes(font->font_face, 0, size);
    if(error != 0)
        fatal_error("Failed to set font pixel size to %d! (%d)", size, error);

    // Add the metric information for the size.
    FT_Size_Metrics ft_metrics = font->font_face->size->metrics;

    TrueTypeMetrics metrics;
    metrics.ascent       = FT_CEIL(ft_metrics.ascender);
    metrics.descent      = FT_CEIL(ft_metrics.descender);
    metrics.line_space   = FT_CEIL(ft_metrics.height);
    metrics.line_gap     = FT_CEIL(ft_metrics.height - ft_metrics.ascender + ft_metrics.descender);
    metrics.max_advance  = FT_CEIL(ft_metrics.max_advance);
    nk_hashmap_insert(&font->metrics, size, metrics);

    // Bake the glyph ranges at the size.
    for(auto& range: font->ranges)
    {
        for(wchar_t codepoint=range.start; codepoint<=range.end; ++codepoint)
        {
            bake_font_glyph(font, size, codepoint);
        }
    }
}

GLOBAL void init_truetype_font_system(void)
{
    #if defined(USE_RENDERER_ADVANCED)
    g_truetype.font_shader = asset_manager_load<Shader>("advanced_text.shader");
    #endif // USE_RENDERER_ADVANCED
    #if defined(USE_RENDERER_SIMPLE)
    g_truetype.font_shader = asset_manager_load<Shader>("simple_text.shader");
    #endif // USE_RENDERER_SIMPLE

    FT_Init_FreeType(&g_truetype.freetype);
    if(!g_truetype.freetype)
    {
        fatal_error("Failed to initialize FreeType!");
    }
}

GLOBAL void quit_truetype_font_system(void)
{
    FT_Done_FreeType(g_truetype.freetype);
}

GLOBAL TrueTypeFont create_truetype_font(const TrueTypeFontDesc& desc)
{
    TrueTypeFont font = ALLOCATE_PRIVATE_TYPE(TrueTypeFont);
    if(!font) fatal_error("Failed to allocate truetype font!");

    font->flags = desc.flags;

    font->data_buffer = NK_CAST(nkU8*, desc.data);
    font->data_size = desc.size;
    font->owns_data = desc.owns_data;

    FT_Error error;

    error = FT_New_Memory_Face(g_truetype.freetype, font->data_buffer, NK_CAST(FT_Long, font->data_size), 0, &font->font_face);
    if(error != 0) fatal_error("Failed to create new font face! (%d)", error);
    error = FT_Select_Charmap(font->font_face, FT_ENCODING_UNICODE);
    if(error != 0) fatal_error("Failed to select font character map! (%d)", error);

    font->atlas_pixels = NK_CALLOC_TYPES(nkU8, FONT_ATLAS_SIZE*FONT_ATLAS_SIZE);
    if(!font->atlas_pixels) fatal_error("Failed to allocate font atlas!");

    font->ranges = desc.ranges;

    if(FT_HAS_KERNING(font->font_face))
    {
        NK_SET_FLAGS(font->flags, TrueTypeFontFlags_HasKerning);
    }

    // @Improve: For now we just use a very simple packing algorithm where we place glyphs linearly from left-to-right
    // and top-to-bottom. We also add a small amount of padding between each glyph. In the future we will want to pack
    // these glyphs more tightly using a better algorithm so that we can fit more glyphs into a single atlas.

    // Bake the specified character ranges at all of the specified sizes.
    font->atlas_row_max_height = 0.0f;
    font->atlas_cursor = NK_V2_ZERO;

    nk_hashmap_init(&font->glyphs);
    for(nkU64 i=0; i<desc.px_sizes.length; ++i)
    {
        bake_font_at_size(font, desc.px_sizes[i]);
    }

    #if defined(USE_RENDERER_ADVANCED)
    TextureDesc texture_desc;
    texture_desc.format = TextureFormat_R;
    texture_desc.width  = FONT_ATLAS_SIZE;
    texture_desc.height = FONT_ATLAS_SIZE;
    texture_desc.data   = font->atlas_pixels;
    font->atlas_texture = create_texture(texture_desc);
    #endif // USE_RENDERER_ADVANCED

    #if defined(USE_RENDERER_SIMPLE)
    font->atlas_texture = create_texture(FONT_ATLAS_SIZE,FONT_ATLAS_SIZE, 1, font->atlas_pixels, SamplerFilter_Linear, SamplerWrap_Clamp);
    #endif // USE_RENDERER_SIMPLE

    font->current_size = desc.px_sizes[0];

    return font;
}

GLOBAL void free_truetype_font(TrueTypeFont font)
{
    NK_ASSERT(font);

    free_texture(font->atlas_texture);
    NK_FREE(font->atlas_pixels);

    nk_array_free(&font->ranges);

    nk_hashmap_free(&font->glyphs);
    nk_hashmap_free(&font->metrics);

    if(font->owns_data) NK_FREE(font->data_buffer);

    FT_Done_Face(font->font_face);

    NK_FREE(font);
}

GLOBAL void set_truetype_font_size(TrueTypeFont font, nkS32 new_size)
{
    NK_ASSERT(font);

    if(font->current_size == new_size) return;

    // Set the new size, if the font does not currently have that size cached then we do it now.
    // We might want to change this in the future as rasterizing the font for a whole new size
    // could be quite slow. For now though this is fine and gives us a bit of flexibility.
    font->current_size = new_size;
    if(!nk_hashmap_contains(&font->metrics, font->current_size))
    {
        printf("[Font]: Attempting to set font to unbaked size (%d), baking now...\n", new_size);
        bake_font_at_size(font, font->current_size);

        // @Improve: Add a way of updating a texture's pixels without fully recrating it...

        // We need to update the texture with the new font data.
        free_texture(font->atlas_texture);

        #if defined(USE_RENDERER_ADVANCED)
        TextureDesc texture_desc;
        texture_desc.format = TextureFormat_R;
        texture_desc.width  = FONT_ATLAS_SIZE;
        texture_desc.height = FONT_ATLAS_SIZE;
        texture_desc.data   = font->atlas_pixels;
        font->atlas_texture = create_texture(texture_desc);
        #endif // USE_RENDERER_ADVANCED

        #if defined(USE_RENDERER_SIMPLE)
        font->atlas_texture = create_texture(FONT_ATLAS_SIZE,FONT_ATLAS_SIZE, 1, font->atlas_pixels, SamplerFilter_Linear, SamplerWrap_Clamp);
        #endif // USE_RENDERER_SIMPLE
    }
}

GLOBAL nkS32 get_truetype_font_size(TrueTypeFont font)
{
    NK_ASSERT(font);
    return font->current_size;
}

GLOBAL TrueTypeMetrics get_truetype_font_metrics(TrueTypeFont font)
{
    NK_ASSERT(font);
    return nk_hashmap_getref(&font->metrics, font->current_size);
}

GLOBAL nkBool has_glyph(TrueTypeFont font, wchar_t codepoint)
{
    NK_ASSERT(font);

    GlyphID glyph_id = { font->current_size, codepoint };
    return nk_hashmap_contains(&font->glyphs, glyph_id);
}

GLOBAL Glyph get_glyph(TrueTypeFont font, wchar_t codepoint)
{
    NK_ASSERT(font);

    GlyphID glyph_id = { font->current_size, codepoint };
    if(nk_hashmap_contains(&font->glyphs, glyph_id))
    {
        return nk_hashmap_getref(&font->glyphs, glyph_id);
    }
    else
    {
        // If we don't have a glyph that has been requested then just return an empty glyph...
        // Maybe we want to do something else here? Return a tofu glyph or something instead?
        Glyph glyph = NK_ZERO_MEM;
        return glyph;
    }
}

GLOBAL nkF32 get_kerning(TrueTypeFont font, wchar_t left, wchar_t right)
{
    NK_ASSERT(font);

    if(!NK_CHECK_FLAGS(font->flags, TrueTypeFontFlags_HasKerning))
    {
        return 0.0f;
    }
    else
    {
        FT_UInt l = FT_Get_Char_Index(font->font_face, left);
        FT_UInt r = FT_Get_Char_Index(font->font_face, right);
        FT_Vector vector;
        FT_Get_Kerning(font->font_face, l, r, FT_KERNING_DEFAULT, &vector); // @Improve: Handle error???
        return FT_CEIL(vector.x);
    }
}

GLOBAL nkF32 get_truetype_text_width(TrueTypeFont font, const wchar_t* text, nkU64 length)
{
    NK_ASSERT(font);

    if(!text) return 0.0f;

    nkF32 line_width = 0.0f;
    nkF32 width = 0.0f;

    if(length == NK_U64_MAX)
    {
        length = wcslen(text);
    }

    for(nkU64 i=0; i<length; ++i)
    {
        if(text[i] == L'\n')
        {
            width = nk_max(width, line_width);
            line_width = 0.0f;
        }
        else
        {
            Glyph glyph = get_glyph(font, text[i]);
            nkF32 advance = glyph.info.advance;
            nkF32 kerning = get_kerning(font, text[i], text[i+1]);
            line_width += advance + kerning;
        }
    }

    return nk_max(width, line_width);
}

GLOBAL nkF32 get_truetype_text_height(TrueTypeFont font, const wchar_t* text, nkU64 length)
{
    NK_ASSERT(font);

    if(!text) return 0.0f;

    TrueTypeMetrics metrics = get_truetype_font_metrics(font);

    nkF32 line_height = metrics.ascent - metrics.descent;
    nkF32 height = 0.0f;

    if(length == NK_U64_MAX)
    {
        length = wcslen(text);
    }

    if(text && (length > 0))
    {
        height = line_height;
        for(nkU64 i=0; i<length; ++i)
        {
            if(text[i] == L'\n')
            {
                height += line_height;
            }
        }
    }

    return height;
}

GLOBAL nkF32 get_truetype_text_width(TrueTypeFont font, const nkChar* text, nkU64 length)
{
    NK_ASSERT(font);

    if(!text) return 0.0f;

    wchar_t* wtext = convert_string_to_wide(text);
    NK_DEFER(NK_FREE(wtext));
    return get_truetype_text_width(font, wtext, length);
}

GLOBAL nkF32 get_truetype_text_height(TrueTypeFont font, const nkChar* text, nkU64 length)
{
    NK_ASSERT(font);

    if(!text) return 0.0f;

    wchar_t* wtext = convert_string_to_wide(text);
    NK_DEFER(NK_FREE(wtext));
    return get_truetype_text_height(font, wtext, length);
}

GLOBAL nkF32 get_truetype_line_height(TrueTypeFont font)
{
    NK_ASSERT(font);
    TrueTypeMetrics metrics = get_truetype_font_metrics(font);
    return (metrics.ascent - metrics.descent);
}

GLOBAL void draw_truetype_text(TrueTypeFont font, nkF32 x, nkF32 y, const wchar_t* text, nkVec4 color)
{
    #if defined(USE_RENDERER_ADVANCED)

    NK_ASSERT(font);

    if(!text) return;

    if(wcslen(text) == 0)
    {
        return;
    }

    TrueTypeMetrics metrics = get_truetype_font_metrics(font);

    nkF32 start_x = x;
    nkF32 start_y = y;
    nkF32 tx = start_x;
    nkF32 ty = start_y;

    Texture old_texture = imm_get_texture();
    Shader old_shader = imm_get_shader();

    imm_set_texture(font->atlas_texture);
    imm_set_shader(g_truetype.font_shader);

    imm_begin(DrawMode_Triangles);

    for(nkU64 i=0,n=wcslen(text); i<n; ++i)
    {
        wchar_t current_char = text[i];
        if(current_char == L'\n')
        {
            tx = start_x;
            ty += roundf(metrics.ascent - metrics.descent);
        }
        else
        {
            Glyph glyph = get_glyph(font, current_char);
            nkF32 advance = glyph.info.advance + get_kerning(font, current_char, text[i+1]);

            nkF32 x1 = roundf(tx) + glyph.info.offset_x;
            nkF32 y1 = roundf(ty) + glyph.info.offset_y;
            nkF32 x2 = x1         + glyph.info.width;
            nkF32 y2 = y1         + glyph.info.height;

            nkF32 s1 =      (glyph.bounds.x / get_texture_width(font->atlas_texture));
            nkF32 t1 =      (glyph.bounds.y / get_texture_height(font->atlas_texture));
            nkF32 s2 = s1 + (glyph.bounds.w / get_texture_width(font->atlas_texture));
            nkF32 t2 = t1 + (glyph.bounds.h / get_texture_height(font->atlas_texture));

            imm_position(x1,y2); imm_color(color.r,color.g,color.b,color.a); imm_texcoord(s1,t2);
            imm_position(x1,y1); imm_color(color.r,color.g,color.b,color.a); imm_texcoord(s1,t1);
            imm_position(x2,y2); imm_color(color.r,color.g,color.b,color.a); imm_texcoord(s2,t2);
            imm_position(x2,y2); imm_color(color.r,color.g,color.b,color.a); imm_texcoord(s2,t2);
            imm_position(x1,y1); imm_color(color.r,color.g,color.b,color.a); imm_texcoord(s1,t1);
            imm_position(x2,y1); imm_color(color.r,color.g,color.b,color.a); imm_texcoord(s2,t1);

            tx += roundf(advance);
        }
    }

    imm_end();

    imm_set_shader(old_shader);
    imm_set_texture(old_texture);

    #endif // USE_RENDERER_ADVANCED

    #if defined(USE_RENDERER_SIMPLE)

    NK_ASSERT(font);

    if(!text) return;

    if(wcslen(text) == 0)
    {
        return;
    }

    TrueTypeMetrics metrics = get_truetype_font_metrics(font);

    nkF32 start_x = x;
    nkF32 start_y = y;
    nkF32 tx = start_x;
    nkF32 ty = start_y;

    imm_begin(DrawMode_Triangles, font->atlas_texture, g_truetype.font_shader);

    for(nkU64 i=0,n=wcslen(text); i<n; ++i)
    {
        wchar_t current_char = text[i];
        if(current_char == L'\n')
        {
            tx = start_x;
            ty += roundf(metrics.ascent - metrics.descent);
        }
        else
        {
            Glyph glyph = get_glyph(font, current_char);
            nkF32 advance = glyph.info.advance + get_kerning(font, current_char, text[i+1]);

            nkF32 x1 = roundf(tx) + glyph.info.offset_x;
            nkF32 y1 = roundf(ty) + glyph.info.offset_y;
            nkF32 x2 = x1         + glyph.info.width;
            nkF32 y2 = y1         + glyph.info.height;

            nkF32 s1 =      (glyph.bounds.x / get_texture_width(font->atlas_texture));
            nkF32 t1 =      (glyph.bounds.y / get_texture_height(font->atlas_texture));
            nkF32 s2 = s1 + (glyph.bounds.w / get_texture_width(font->atlas_texture));
            nkF32 t2 = t1 + (glyph.bounds.h / get_texture_height(font->atlas_texture));

            imm_vertex({ { x1,y2 }, { s1,t2 }, color });
            imm_vertex({ { x1,y1 }, { s1,t1 }, color });
            imm_vertex({ { x2,y2 }, { s2,t2 }, color });
            imm_vertex({ { x2,y2 }, { s2,t2 }, color });
            imm_vertex({ { x1,y1 }, { s1,t1 }, color });
            imm_vertex({ { x2,y1 }, { s2,t1 }, color });

            tx += roundf(advance);
        }
    }

    imm_end();

    #endif // USE_RENDERER_SIMPLE
}

GLOBAL void draw_truetype_char(TrueTypeFont font, nkF32 x, nkF32 y, wchar_t chr, nkVec4 color)
{
    wchar_t buffer[2] = { chr, L'\0' };
    draw_truetype_text(font, x,y, buffer, color);
}

GLOBAL void draw_truetype_text(TrueTypeFont font, nkF32 x, nkF32 y, const nkChar* text, nkVec4 color)
{
    NK_ASSERT(font);

    if(!text) return;

    wchar_t* wtext = convert_string_to_wide(text);
    NK_DEFER(NK_FREE(wtext));
    draw_truetype_text(font, x,y, wtext, color);
}

GLOBAL void draw_truetype_char(TrueTypeFont font, nkF32 x, nkF32 y, nkChar chr, nkVec4 color)
{
    nkChar buffer[2] = { chr, '\0' };
    draw_truetype_text(font, x,y, buffer, color);
}

/*////////////////////////////////////////////////////////////////////////////*/
