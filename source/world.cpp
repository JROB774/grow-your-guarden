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
    // Update the plants.
    plant_tick(g_world.plants.data, g_world.plants.length, dt);
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
    plant_draw(g_world.plants.data, g_world.plants.length);
}

GLOBAL nkBool place_plant(PlantID id, nkS32 x, nkS32 y)
{
    NK_ASSERT(id < PlantID_TOTAL);

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

    Sound sound = asset_manager_load<Sound>("shovel_000.wav"); // @Incomplete: Randomize!
    play_sound(sound);

    const PlantDesc& desc = PLANT_DESC_TABLE[id];

    Plant plant;

    plant.id         = id;
    plant.x          = x;
    plant.y          = y;
    plant.health     = desc.health;
    plant.phase      = 0;
    plant.width      = desc.width;
    plant.height     = desc.height;
    plant.anim_timer = rng_f32(0.0f, PLANT_ANIM_SPEED);
    plant.anim_frame = 0;

    nk_array_append(&g_world.plants, plant);

    return NK_TRUE;
}

GLOBAL nkBool remove_plant(nkS32 x, nkS32 y)
{
    // Is the spot in bounds.
    if(x < 0 || x >= g_world.width || y < 0 || y >= g_world.height)
    {
        return NK_FALSE;
    }

    // Check for a plant and if so remove it.
    for(nkU64 i=0; i<g_world.plants.length; ++i)
    {
        Plant* p = &g_world.plants[i];
        if(p->x == x && p->y == y)
        {
            Sound sound = asset_manager_load<Sound>("shovel_001.wav"); // @Incomplete: Randomize!
            play_sound(sound);
            nk_array_remove(&g_world.plants, i);
            return NK_TRUE;
        }
    }

    return NK_FALSE;
}

GLOBAL nkBool water_plant(nkS32 x, nkS32 y)
{
    // Is the spot in bounds.
    if(x < 0 || x >= g_world.width || y < 0 || y >= g_world.height)
    {
        return NK_FALSE;
    }

    // Check for a plant to water.
    for(nkU64 i=0; i<g_world.plants.length; ++i)
    {
        Plant* p = &g_world.plants[i];
        if(p->x == x && p->y == y)
        {
            Sound sound = asset_manager_load<Sound>("water_pour.wav");
            play_sound(sound);
            // @Incomplete: Do the watering logic...
            return NK_TRUE;
        }
    }

    return NK_FALSE;
}

/*////////////////////////////////////////////////////////////////////////////*/
