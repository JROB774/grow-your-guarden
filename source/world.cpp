/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void world_init(void)
{
    g_world.width  = 25;
    g_world.height = 19;

    // World's must be an odd number size!
    NK_ASSERT((g_world.width % 2) != 0);
    NK_ASSERT((g_world.height % 2) != 0);

    g_world.tilemap = NK_CALLOC_TYPES(Tile, g_world.width*g_world.height);

    nk_array_reserve(&g_world.entities, 512);

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
    }

    entity_spawn(EntityID_Walker, 128.0f,128.0f); // @Temporary!

    g_world.tileset = asset_manager_load<Texture>("tileset.png");
}

GLOBAL void world_quit(void)
{
    nk_array_free(&g_world.entities);
    NK_FREE(g_world.tilemap);
}

GLOBAL void world_tick(nkF32 dt)
{
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
            ImmClip clip = { 0.0f, 0.0f, NK_CAST(nkF32, TILE_WIDTH), NK_CAST(nkF32, TILE_HEIGHT) };
            switch(id)
            {
                case TileID_GrassLight: clip.x = 0.0f; break;
                case TileID_GrassDark: clip.x = 32.0f; break;
            }

            nkF32 tx = NK_CAST(nkF32, x * TILE_WIDTH) + (NK_CAST(nkF32,TILE_WIDTH) * 0.5f);
            nkF32 ty = NK_CAST(nkF32, y * TILE_HEIGHT) + (NK_CAST(nkF32,TILE_HEIGHT) * 0.5f);

            imm_texture_batched(tx,ty, &clip);
        }
    }
    imm_end_texture_batch();

    entity_draw();
}

/*////////////////////////////////////////////////////////////////////////////*/
