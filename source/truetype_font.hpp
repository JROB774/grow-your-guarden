/*////////////////////////////////////////////////////////////////////////////*/

DECLARE_PRIVATE_TYPE(TrueTypeFont);

NK_ENUM(TrueTypeFontFlags, nkU32)
{
    TrueTypeFontFlags_None       = (   0),
    TrueTypeFontFlags_HasKerning = (1<<0), // Whether the font has kerning data or not (automatically assigned on font creation).
};

struct GlyphInfo
{
    nkS32 offset_x;
    nkS32 offset_y;
    nkS32 width;
    nkS32 height;
    nkF32 advance;
};

struct GlyphBounds
{
    nkF32 x,y,w,h;
};

struct Glyph
{
    GlyphInfo   info;
    GlyphBounds bounds;
};

struct CharRange
{
    wchar_t start;
    wchar_t end;
};

struct TrueTypeMetrics
{
    nkF32 max_advance;
    nkF32 ascent;
    nkF32 descent;
    nkF32 line_space;
    nkF32 line_gap;
};

struct TrueTypeFontDesc
{
    TrueTypeFontFlags  flags     = TrueTypeFontFlags_None;
    void*              data      = NULL;
    nkU64              size      = NULL;
    nkBool             owns_data = NK_FALSE;
    nkArray<nkS32>     px_sizes  = { 12 };
    nkArray<CharRange> ranges    = { { L'\u0020', L'\u007E' } };
};

GLOBAL void            init_truetype_font_system(void);
GLOBAL void            quit_truetype_font_system(void);
GLOBAL TrueTypeFont    create_truetype_font     (const TrueTypeFontDesc& desc);
GLOBAL void            free_truetype_font       (TrueTypeFont font);
GLOBAL void            set_truetype_font_size   (TrueTypeFont font, nkS32 new_size);
GLOBAL nkS32           get_truetype_font_size   (TrueTypeFont font);
GLOBAL TrueTypeMetrics get_truetype_font_metrics(TrueTypeFont font);
GLOBAL nkBool          has_glyph                (TrueTypeFont font, wchar_t codepoint);
GLOBAL Glyph           get_glyph                (TrueTypeFont font, wchar_t codepoint);
GLOBAL nkF32           get_kerning              (TrueTypeFont font, wchar_t left, wchar_t right);
GLOBAL nkF32           get_truetype_text_width  (TrueTypeFont font, const wchar_t* text, nkU64 length = NK_U64_MAX); // Default length value means scan the whole string.
GLOBAL nkF32           get_truetype_text_height (TrueTypeFont font, const wchar_t* text, nkU64 length = NK_U64_MAX); // Default length value means scan the whole string.
GLOBAL nkF32           get_truetype_text_width  (TrueTypeFont font, const nkChar*  text, nkU64 length = NK_U64_MAX); // Default length value means scan the whole string.
GLOBAL nkF32           get_truetype_text_height (TrueTypeFont font, const nkChar*  text, nkU64 length = NK_U64_MAX); // Default length value means scan the whole string.
GLOBAL nkF32           get_truetype_line_height (TrueTypeFont font);
GLOBAL void            draw_truetype_text       (TrueTypeFont font, nkF32 x, nkF32 y, const wchar_t* text, nkVec4 color = NK_V4_WHITE);
GLOBAL void            draw_truetype_char       (TrueTypeFont font, nkF32 x, nkF32 y, wchar_t chr,         nkVec4 color = NK_V4_WHITE);
GLOBAL void            draw_truetype_text       (TrueTypeFont font, nkF32 x, nkF32 y, const nkChar*  text, nkVec4 color = NK_V4_WHITE);
GLOBAL void            draw_truetype_char       (TrueTypeFont font, nkF32 x, nkF32 y, nkChar  chr,         nkVec4 color = NK_V4_WHITE);

/*////////////////////////////////////////////////////////////////////////////*/

template<>
TrueTypeFont asset_load<TrueTypeFont>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    // If no desc has been passed in then use the default font descriptor.
    TrueTypeFontDesc desc = (userdata) ? *NK_CAST(TrueTypeFontDesc*, userdata) : TrueTypeFontDesc();
    desc.data      = data;
    desc.size      = size;
    desc.owns_data = !from_npak;
    return create_truetype_font(desc);
}
template<>
void asset_free<TrueTypeFont>(Asset<TrueTypeFont>& asset)
{
    free_truetype_font(asset.data);
}
template<>
const nkChar* asset_path<TrueTypeFont>(void)
{
    return "fonts_ttf/";
}

/*////////////////////////////////////////////////////////////////////////////*/
