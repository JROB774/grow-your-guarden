/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL World g_world;

GLOBAL void world_init(nkS32 width, nkS32 height, nkU32 seed)
{
    // World's must be an odd number size!
    NK_ASSERT((width % 2) != 0);
    NK_ASSERT((height % 2) != 0);

    rng_seed((seed) ? seed : NK_CAST(nkU32, time(NULL)));

    g_world.width  = width;
    g_world.height = height;

    g_world.tilemap = NK_CALLOC_TYPES(Tile, g_world.width * g_world.height);

    for(nkS32 y=0; y<g_world.height; ++y)
    {
        for(nkS32 x=0; x<g_world.width; ++x)
        {
            nkU32 index = y * g_world.width + x;
            g_world.tilemap[index].id = rng_s32(0, TileID_TOTAL-1);
        }
    }

    g_world.tileset = asset_manager_load<Texture>("tileset0.png");
}

GLOBAL void world_quit(void)
{
    NK_FREE(g_world.tilemap);
}

GLOBAL void world_tick(nkF32 dt)
{
    // @Temporary: Spawn random monsters!
    if(rng_s32(0,100) < 2)
    {
        nkS32 tw = NK_CAST(nkS32, TILE_WIDTH);
        nkS32 th = NK_CAST(nkS32, TILE_HEIGHT);

        nkS32 pos = rng_s32(0,100);
        if(pos < 25)
        {
            entity_spawn(EntityID_Walker, 0, NK_CAST(nkF32, rng_s32(0, g_world.height * th)));
        }
        else if(pos < 50)
        {
            entity_spawn(EntityID_Walker, NK_CAST(nkF32, g_world.width * tw), NK_CAST(nkF32, rng_s32(0, g_world.height * th)));
        }
        else if(pos < 75)
        {
            entity_spawn(EntityID_Walker, NK_CAST(nkF32, rng_s32(0, g_world.width * tw)), 0);
        }
        else
        {
            entity_spawn(EntityID_Walker, NK_CAST(nkF32, rng_s32(0, g_world.width * tw)), NK_CAST(nkF32, g_world.height * th));
        }
    }
}

GLOBAL void world_draw(void)
{
    imm_begin_texture_batch(g_world.tileset);

    for(nkS32 y=0; y<g_world.height; ++y)
    {
        for(nkS32 x=0; x<g_world.width; ++x)
        {
            TileID id = g_world.tilemap[y*g_world.width+x].id;

            ImmClip clip = { NK_CAST(nkF32, id) * TILE_WIDTH, 0.0f, TILE_WIDTH, TILE_HEIGHT };

            nkF32 tx = NK_CAST(nkF32, x * TILE_WIDTH) + (NK_CAST(nkF32,TILE_WIDTH) * 0.5f);
            nkF32 ty = NK_CAST(nkF32, y * TILE_HEIGHT) + (NK_CAST(nkF32,TILE_HEIGHT) * 0.5f);

            imm_texture_batched(tx,ty, &clip);
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
