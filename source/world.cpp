/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void world_init(void)
{
    rng_seed(NK_CAST(nkU32, time(NULL)));

    g_world.width  = 25;
    g_world.height = 19;

    // World's must be an odd number size!
    NK_ASSERT((g_world.width % 2) != 0);
    NK_ASSERT((g_world.height % 2) != 0);

    g_world.tilemap = NK_CALLOC_TYPES(Tile, g_world.width*g_world.height);

    nk_array_reserve(&g_world.entities, 512);

    for(nkS32 y=0; y<g_world.height; ++y)
    {
        for(nkS32 x=0; x<g_world.width; ++x)
        {
            g_world.tilemap[y*g_world.width+x].id = rng_s32(0, TileID_TOTAL-1);
        }
    }

    nkF32 hx = NK_CAST(nkF32, g_world.width * TILE_WIDTH) * 0.5f;
    nkF32 hy = NK_CAST(nkF32, g_world.height * TILE_HEIGHT) * 0.5f;

    entity_spawn(EntityID_House, hx,hy);

    g_world.tileset = asset_manager_load<Texture>("tileset0.png");
}

GLOBAL void world_quit(void)
{
    nk_array_free(&g_world.entities);
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

    entity_tick(dt);
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

    entity_draw();
}

/*////////////////////////////////////////////////////////////////////////////*/
