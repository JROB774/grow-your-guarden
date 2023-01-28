/*////////////////////////////////////////////////////////////////////////////*/

DECLARE_PRIVATE_TYPE(BitmapFont);

GLOBAL BitmapFont create_bitmap_font       (void* data, nkU64 bytes);
GLOBAL void       free_bitmap_font         (BitmapFont font);
GLOBAL nkVec2     get_bitmap_text_bounds   (BitmapFont font, const nkChar* text);
GLOBAL nkF32      get_bitmap_text_width    (BitmapFont font, const nkChar* text);
GLOBAL nkF32      get_bitmap_text_height   (BitmapFont font, const nkChar* text);
GLOBAL nkF32      get_bitmap_font_px_height(BitmapFont font);
GLOBAL void       draw_bitmap_text         (BitmapFont font, nkF32 x, nkF32 y, const nkChar* text, nkVec4 color = NK_V4_WHITE);
GLOBAL void       draw_bitmap_char         (BitmapFont font, nkF32 x, nkF32 y, nkChar chr,         nkVec4 color = NK_V4_WHITE);

/*////////////////////////////////////////////////////////////////////////////*/

template<>
BitmapFont asset_load<BitmapFont>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_bitmap_font(data, size);
}
template<>
void asset_free<BitmapFont>(Asset<BitmapFont>& asset)
{
    free_bitmap_font(asset.data);
}
template<>
const nkChar* asset_path<BitmapFont>(void)
{
    return "fonts_bmp/";
}

/*////////////////////////////////////////////////////////////////////////////*/
