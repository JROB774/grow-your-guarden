/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL nkF32 TILE_WIDTH  = 256.0f;
GLOBAL nkF32 TILE_HEIGHT = 256.0f;

NK_ENUM(TileID, nkU32)
{
    TileID_Grass0,
    TileID_Grass1,
    TileID_Grass2,
    TileID_Grass3,
    TileID_Grass4,
    TileID_Grass5,
    TileID_Grass6,
    TileID_Grass7,
    TileID_TOTAL
};

struct Tile
{
    TileID id;
};

struct World
{
    nkS32           width, height;
    Texture         tileset;
    Tile*           tilemap;
    nkArray<Entity> entities;
};

GLOBAL World g_world;

GLOBAL void world_init(void);
GLOBAL void world_quit(void);
GLOBAL void world_tick(nkF32 dt);
GLOBAL void world_draw(void);

/*////////////////////////////////////////////////////////////////////////////*/
