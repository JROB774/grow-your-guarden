/*////////////////////////////////////////////////////////////////////////////*/

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
    nkS32 width, height;
    Tile* tilemap;
};

INTERNAL World g_world;

GLOBAL void world_init(void)
{
    g_world.width  = 9;
    g_world.height = 6;

    g_world.tilemap = NK_CALLOC_TYPES(Tile, g_world.width*g_world.height);

    TileID id = TileID_GrassLight;

    for(nkS32 y=0; y<g_world.height; ++y)
    {
        for(nkS32 x=0; x<g_world.width; ++x)
        {
            g_world.tilemap[y*g_world.width+x].id = id;

            switch(id)
            {
                case TileID_GrassLight: id = TileID_GrassDark; break;
                case TileID_GrassDark: id = TileID_GrassLight; break;
            }
        }

        // Ensure the alternating grid patten on even width maps.
        if(g_world.width % 2 == 0)
        {
            switch(id)
            {
                case TileID_GrassLight: id = TileID_GrassDark; break;
                case TileID_GrassDark: id = TileID_GrassLight; break;
            }
        }
    }
}

GLOBAL void world_quit(void)
{
    NK_FREE(g_world.tilemap);
}

GLOBAL void world_tick(nkF32 dt)
{
    // Nothing...
}

GLOBAL void world_draw(void)
{
    for(nkS32 y=0; y<g_world.height; ++y)
    {
        for(nkS32 x=0; x<g_world.width; ++x)
        {
            Texture texture = NULL;

            TileID id = g_world.tilemap[y*g_world.width+x].id;
            switch(id)
            {
                case TileID_GrassLight: texture = asset_manager_load<Texture>("grass_light.png"); break;
                case TileID_GrassDark: texture = asset_manager_load<Texture>("grass_dark.png"); break;
            }

            NK_ASSERT(texture);

            nkF32 tx = NK_CAST(nkF32, x * TILE_WIDTH) + (NK_CAST(nkF32,TILE_WIDTH) * 0.5f);
            nkF32 ty = NK_CAST(nkF32, y * TILE_HEIGHT) + (NK_CAST(nkF32,TILE_HEIGHT) * 0.5f);

            imm_texture(texture, tx,ty);
        }
    }
}

/*////////////////////////////////////////////////////////////////////////////*/
