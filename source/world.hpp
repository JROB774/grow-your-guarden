/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL nkS32 TILE_WIDTH  = 32;
GLOBAL nkS32 TILE_HEIGHT = 32;

NK_ENUM(TileID, nkU32)
{
    TileID_GrassLight,
    TileID_GrassDark,
    TileID_TOTAL
};

struct Tile
{
    TileID id;
};

struct World
{
    nkS32            width, height;
    Tile*            tilemap;
    nkArray<Plant>   plants;
    nkArray<Monster> monsters;
};

GLOBAL World g_world;

GLOBAL void world_init(void);
GLOBAL void world_quit(void);
GLOBAL void world_tick(nkF32 dt);
GLOBAL void world_draw(void);

/*////////////////////////////////////////////////////////////////////////////*/
