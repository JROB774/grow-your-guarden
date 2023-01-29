/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void world_init(void)
{
    g_world.width  = 24;
    g_world.height = 18;

    g_world.tilemap = NK_CALLOC_TYPES(Tile, g_world.width*g_world.height);

    nk_array_reserve(&g_world.plants, g_world.width * g_world.height);

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
    nk_array_free(&g_world.plants);
    NK_FREE(g_world.tilemap);
}

GLOBAL void world_tick(nkF32 dt)
{
    // Nothing...
}

GLOBAL void world_draw(void)
{
    // Draw the world tiles.
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

    // Draw the plants.
    for(nkU64 i=0; i<g_world.plants.length; ++i)
    {
        Plant* p = &g_world.plants[i];

        Texture icon = NULL;
        switch(p->id)
        {
            case PlantID_Flower: icon = asset_manager_load<Texture>("flower.png"); break;
        }
        if(icon)
        {
            nkF32 px = NK_CAST(nkF32, p->x * TILE_WIDTH) + (NK_CAST(nkF32,TILE_WIDTH) * 0.5f);
            nkF32 py = NK_CAST(nkF32, p->y * TILE_HEIGHT) + (NK_CAST(nkF32,TILE_HEIGHT) * 0.5f);

            imm_texture(icon, px,py);
        }
    }
}

GLOBAL nkBool place_plant(PlantID id, nkS32 x, nkS32 y)
{
    // Is the spot in bounds.
    if(x < 0 || x >= g_world.width || y < 0 || y >= g_world.height)
    {
        return NK_FALSE;
    }

    // Check if the spot is already occupied.
    for(nkU64 i=0; i<g_world.plants.length; ++i)
    {
        Plant* p = &g_world.plants[i];
        if(p->x == x && p->y == y)
        {
            return NK_FALSE;
        }
    }

    Plant plant;
    plant.id = id;
    plant.x  = x;
    plant.y  = y;

    nk_array_append(&g_world.plants, plant);

    return NK_TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
