/*////////////////////////////////////////////////////////////////////////////*/

#define CHECK_TILE_EDGE(edge, flags) (((edge) & (flags)) == (flags))

INTERNAL nkU32 TILE_COLORS[] =
{
//    AABBGGRR
    0xFFFFFFFF, // TileID_None
    0xFF347571, // TileID_Dirt
    0xFF38605E, // TileID_DirtDark
    0xFF3AC431, // TileID_Grass
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(TILE_COLORS) == TileID_TOTAL, tile_color_size_mismatch);

INTERNAL World g_world;

INTERNAL TileID tile_id_from_tile_color(nkU32 color)
{
    for(nkU32 i=0; i<TileID_TOTAL; ++i)
        if(TILE_COLORS[i] == color)
            return i;
    NK_ASSERT(NK_FALSE);
    return TileID_None;
}

INTERNAL Texture get_tile_texture_from_id(TileID id)
{
    switch(id)
    {
        case TileID_None:     return NULL;
        case TileID_Dirt:     return asset_manager_load<Texture>("tiles/dirt.png");
        case TileID_DirtDark: return asset_manager_load<Texture>("tiles/dirt_dark.png");
        case TileID_Grass:    return asset_manager_load<Texture>("tiles/grass.png");
    }
    NK_ASSERT(NK_FALSE); // Texture case needs assing for the given tile ID!
    return NULL;
}

INTERNAL nkBool is_tile_texture_dynamic(Texture texture)
{
    // Dynamic tiles will have a texture size bigger than this to fit all the different variants.
    NK_ASSERT(texture);
    nkS32 tw = get_texture_width(texture);
    nkS32 th = get_texture_height(texture);
    return ((tw != (TILE_WIDTH*5)) ||
            (th != (TILE_HEIGHT)));
}

INTERNAL nkS32 randomize_tile_variant(nkS32 min, nkS32 max)
{
    if(rng_s32(0,100) <= 70)
    {
        return min;
    }
    else
    {
        return rng_s32(min+1,max);
    }
}

INTERNAL ImmClip get_tile_clip_from_edges(TileEdge edges)
{
    iPoint tile = { 4,1 }; // Default is no sides occupied by neighboring tiles of the same type.

    // This is pretty ridiculous but it works so oh well!
         if(CHECK_TILE_EDGE(edges, TileEdge_All                                                                                                      )) tile = { randomize_tile_variant(10,14),2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthEast|TileEdge_NorthWest|TileEdge_SouthWest)) tile = {  7, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthEast|TileEdge_NorthWest|TileEdge_SouthEast)) tile = {  8, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthWest|TileEdge_SouthEast|TileEdge_SouthWest)) tile = {  7, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthEast|TileEdge_SouthEast|TileEdge_SouthWest)) tile = {  8, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthEast|TileEdge_SouthWest                   )) tile = {  9, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthWest|TileEdge_SouthEast                   )) tile = { 10, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthEast|TileEdge_NorthWest                   )) tile = { 11, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthEast|TileEdge_SouthEast                   )) tile = { 12, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthWest|TileEdge_SouthWest                   )) tile = { 11, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_SouthEast|TileEdge_SouthWest                   )) tile = { 12, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthWest                                      )) tile = { 13, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_NorthEast                                      )) tile = { 14, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_SouthEast                                      )) tile = { 13, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_SouthWest                                      )) tile = { 14, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_South|TileEdge_East|TileEdge_West|TileEdge_SouthEast|TileEdge_SouthWest                                  )) tile = {  4, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_NorthEast|TileEdge_SouthEast                                 )) tile = {  3, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_West|TileEdge_NorthWest|TileEdge_SouthWest                                 )) tile = {  5, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_East|TileEdge_West|TileEdge_NorthEast|TileEdge_NorthWest                                  )) tile = {  4, 2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East|TileEdge_West                                                         )) tile = {  1, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_South|TileEdge_East|TileEdge_SouthEast                                                                   )) tile = {  3, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_South|TileEdge_West|TileEdge_SouthWest                                                                   )) tile = {  5, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_East|TileEdge_NorthEast                                                                   )) tile = {  3, 2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_West|TileEdge_NorthWest                                                                   )) tile = {  5, 2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_South|TileEdge_East|TileEdge_West                                                                        )) tile = {  1, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_East                                                                       )) tile = {  0, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South|TileEdge_West                                                                       )) tile = {  2, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_East|TileEdge_West                                                                        )) tile = {  1, 2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_South|TileEdge_East                                                                                      )) tile = {  0, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_South|TileEdge_West                                                                                      )) tile = {  2, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_East                                                                                      )) tile = {  0, 2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_West                                                                                      )) tile = {  2, 2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North|TileEdge_South                                                                                     )) tile = {  6, 1 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_East|TileEdge_West                                                                                       )) tile = {  8, 2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_South                                                                                                    )) tile = {  6, 0 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_North                                                                                                    )) tile = {  6, 2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_East                                                                                                     )) tile = {  7, 2 };
    else if(CHECK_TILE_EDGE(edges, TileEdge_West                                                                                                     )) tile = {  9, 2 };

    ImmClip clip;
    clip.x = NK_CAST(nkF32, tile.x) * TILE_WIDTH;
    clip.y = NK_CAST(nkF32, tile.y) * TILE_HEIGHT;
    clip.w = TILE_WIDTH;
    clip.h = TILE_HEIGHT;
    return clip;
}

GLOBAL void world_load(const nkChar* level_name, nkU32 seed)
{
    LevelBMP lvlbmp = asset_manager_load<LevelBMP>(level_name);

    // World's must be an odd number size!
    NK_ASSERT((lvlbmp.width % 2) != 0);
    NK_ASSERT((lvlbmp.height % 2) != 0);

    rng_seed(seed);

    g_world.border = asset_manager_load<Texture>("border.png");
    g_world.border_color = { 0.0157f, 0.2863f, 0.1412f, 1.0f }; // @Incomplete: Hard-coded to match the border image!

    g_world.width  = lvlbmp.width;
    g_world.height = lvlbmp.height;

    g_world.tilemap = NK_CALLOC_TYPES(Tile, g_world.width * g_world.height);

    for(nkS32 y=0; y<g_world.height; ++y)
    {
        for(nkS32 x=0; x<g_world.width; ++x)
        {
            nkS32 index = y * g_world.width + x;

            Tile& tile = g_world.tilemap[index];
            tile.id = tile_id_from_tile_color(lvlbmp.pixels[index]);
            tile.edges = TileEdge_None;

            // If the tile is dynamic then calculate the edges of the current tile.
            Texture texture = get_tile_texture_from_id(tile.id);
            if(is_tile_texture_dynamic(texture))
            {
                if((x == (               0)                           ) || (lvlbmp.pixels[(y  )*g_world.width+(x-1)] == TILE_COLORS[tile.id])) NK_SET_FLAGS(tile.edges, TileEdge_West     );
                if((y == (g_world.height-1)                           ) || (lvlbmp.pixels[(y+1)*g_world.width+(x  )] == TILE_COLORS[tile.id])) NK_SET_FLAGS(tile.edges, TileEdge_South    );
                if((x == (g_world.width -1)                           ) || (lvlbmp.pixels[(y  )*g_world.width+(x+1)] == TILE_COLORS[tile.id])) NK_SET_FLAGS(tile.edges, TileEdge_East     );
                if((y == (               0)                           ) || (lvlbmp.pixels[(y-1)*g_world.width+(x  )] == TILE_COLORS[tile.id])) NK_SET_FLAGS(tile.edges, TileEdge_North    );
                if((x == (               0) || y == (               0)) || (lvlbmp.pixels[(y-1)*g_world.width+(x-1)] == TILE_COLORS[tile.id])) NK_SET_FLAGS(tile.edges, TileEdge_NorthWest);
                if((y == (g_world.height-1) || x == (               0)) || (lvlbmp.pixels[(y+1)*g_world.width+(x-1)] == TILE_COLORS[tile.id])) NK_SET_FLAGS(tile.edges, TileEdge_SouthWest);
                if((x == (g_world.width -1) || y == (g_world.height-1)) || (lvlbmp.pixels[(y+1)*g_world.width+(x+1)] == TILE_COLORS[tile.id])) NK_SET_FLAGS(tile.edges, TileEdge_SouthEast);
                if((y == (               0) || x == (g_world.width -1)) || (lvlbmp.pixels[(y-1)*g_world.width+(x+1)] == TILE_COLORS[tile.id])) NK_SET_FLAGS(tile.edges, TileEdge_NorthEast);

                tile.clip = get_tile_clip_from_edges(tile.edges);
            }
            else
            {
                tile.clip.x = NK_CAST(nkF32, randomize_tile_variant(0,4)) * TILE_WIDTH;
                tile.clip.y = 0.0f;
                tile.clip.w = TILE_WIDTH;
                tile.clip.h = TILE_HEIGHT;
            }
        }
    }

    // After we've loaded set the random seed to be the current time. Otherwise stuff like entity
    // spawning patterns, etc. will all be the same every single time which isn't very good...
    rng_seed(NK_CAST(nkU32, time(NULL)));
}

GLOBAL void world_free(void)
{
    NK_FREE(g_world.tilemap);
}

GLOBAL void world_tick(nkF32 dt)
{
    // Does nothing...
}

GLOBAL void world_draw_below(void)
{
    clear_screen(g_world.border_color);

    for(TileID id=0; id<TileID_TOTAL; ++id)
    {
        Texture texture = get_tile_texture_from_id(id);
        if(texture)
        {
            imm_begin_texture_batch(texture);
            for(nkS32 y=0; y<g_world.height; ++y)
            {
                for(nkS32 x=0; x<g_world.width; ++x)
                {
                    Tile tile = g_world.tilemap[y*g_world.width+x];
                    if(tile.id == id)
                    {
                        nkF32 tx = NK_CAST(nkF32, x * TILE_WIDTH) + (NK_CAST(nkF32,TILE_WIDTH) * 0.5f);
                        nkF32 ty = NK_CAST(nkF32, y * TILE_HEIGHT) + (NK_CAST(nkF32,TILE_HEIGHT) * 0.5f);

                        imm_texture_batched(tx,ty, &tile.clip);
                    }
                }
            }
            imm_end_texture_batch();
        }
    }
}

GLOBAL void world_draw_above(void)
{
    imm_begin_texture_batch(g_world.border);
    // Top
    {
        nkF32 x = 0.0f;
        nkF32 y = 0.0f;
        while(x <= (g_world.width * TILE_WIDTH))
        {
            imm_texture_batched_ex(x,y, 1.0f,1.0f, nk_torad(180.0f), NULL);
            x += get_texture_width(g_world.border);
        }
    }
    // Right
    {
        nkF32 x = (g_world.width * TILE_WIDTH);
        nkF32 y = 0.0f;
        while(y <= (g_world.height * TILE_HEIGHT))
        {
            imm_texture_batched_ex(x,y, 1.0f,1.0f, nk_torad(270.0f), NULL);
            y += get_texture_height(g_world.border);
        }
    }
    // Bottom
    {
        nkF32 x = 0.0f;
        nkF32 y = (g_world.height * TILE_HEIGHT);
        while(x <= (g_world.width * TILE_WIDTH))
        {
            imm_texture_batched_ex(x,y, 1.0f,1.0f, 0.0f, NULL);
            x += get_texture_width(g_world.border);
        }
    }
    // Left
    {
        nkF32 x = 0.0f;
        nkF32 y = 0.0f;
        while(y <= (g_world.height * TILE_HEIGHT))
        {
            imm_texture_batched_ex(x,y, 1.0f,1.0f, nk_torad(90.0f), NULL);
            y += get_texture_height(g_world.border);
        }
    }
    imm_end_texture_batch();
}

GLOBAL nkS32 get_world_width(void)
{
    return g_world.width;
}

GLOBAL nkS32 get_world_height(void)
{
    return g_world.height;
}

/*////////////////////////////////////////////////////////////////////////////*/
