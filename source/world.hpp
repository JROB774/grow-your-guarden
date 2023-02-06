/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL constexpr nkF32 TILE_WIDTH  = 256.0f;
GLOBAL constexpr nkF32 TILE_HEIGHT = 256.0f;

NK_ENUM(TileID, nkU32)
{
    TileID_None,
    TileID_Dirt,
    TileID_DirtDark,
    TileID_Grass,
    TileID_TOTAL
};

NK_ENUM(TileEdge, nkU32)
{
    TileEdge_None      = (   0),
    TileEdge_North     = (1<<0),
    TileEdge_NorthEast = (1<<1),
    TileEdge_East      = (1<<2),
    TileEdge_SouthEast = (1<<3),
    TileEdge_South     = (1<<4),
    TileEdge_SouthWest = (1<<5),
    TileEdge_West      = (1<<6),
    TileEdge_NorthWest = (1<<7),
    TileEdge_All       = (0xFF)
};

struct Tile
{
    TileID   id;
    TileEdge edges;
    ImmClip  clip;
};

struct World
{
    nkS32   width, height;
    Tile*   tilemap;
    Texture border;
    nkVec4  border_color;
};

GLOBAL void  world_load      (const nkChar* level_name, nkU32 seed = 573273); // Just some default seed value for consistent randomization of tiles.
GLOBAL void  world_free      (void);
GLOBAL void  world_tick      (nkF32 dt);
GLOBAL void  world_draw_below(void); // Draw the tiles below the entities.
GLOBAL void  world_draw_above(void); // Draw the border above the entities.
GLOBAL nkS32 get_world_width (void);
GLOBAL nkS32 get_world_height(void);

/*////////////////////////////////////////////////////////////////////////////*/

struct LevelBMP
{
    nkS32  width;
    nkS32  height;
    nkU32* pixels;
};

template<>
LevelBMP asset_load<LevelBMP>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    LevelBMP lvlbmp = NK_ZERO_MEM;
    nkS32 bpp;
    lvlbmp.pixels = NK_CAST(nkU32*, stbi_load_from_memory(NK_CAST(stbi_uc*, data), NK_CAST(int,size), &lvlbmp.width,&lvlbmp.height,&bpp, 4));
    return lvlbmp;
}
template<>
void asset_free<LevelBMP>(Asset<LevelBMP>& asset)
{
    if(asset.data.pixels)
        stbi_image_free(asset.data.pixels);
    asset.data = NK_ZERO_MEM;
}
template<>
const nkChar* asset_path<LevelBMP>(void)
{
    return "levels/";
}

/*////////////////////////////////////////////////////////////////////////////*/
