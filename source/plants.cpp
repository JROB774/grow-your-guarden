/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL void ptick__flower(Plant* p, nkF32 dt)
{
    // @Incomplete: ...
}

INTERNAL void ptick__bramble(Plant* p, nkF32 dt)
{
    // @Incomplete: ...
}

/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr PlantDesc PLANT_TABLE[] =
{
{ NULL,           NULL,            0, 0, 0,0, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // None
{ ptick__flower,  "flower.png",  100, 3, 1,1, { 10,10, 0, 0, 0, 0, 0, 0 } }, // Flower
{ ptick__bramble, "bramble.png",  50, 3, 1,1, {  2, 2, 2, 0, 0, 0, 0, 0 } }, // Bramble
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(PLANT_TABLE) == PlantID_TOTAL, plant_table_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkF32 PLANT_ANIM_SPEED = 0.3f;

INTERNAL Sound g_shovel_sfx[5];
INTERNAL Sound g_water_sfx;

GLOBAL void plant_init(void)
{
    // Pre-load a bunch of assets.

    g_shovel_sfx[0] = asset_manager_load<Sound>("shovel_000.wav");
    g_shovel_sfx[1] = asset_manager_load<Sound>("shovel_001.wav");
    g_shovel_sfx[2] = asset_manager_load<Sound>("shovel_002.wav");
    g_shovel_sfx[3] = asset_manager_load<Sound>("shovel_003.wav");
    g_shovel_sfx[4] = asset_manager_load<Sound>("shovel_004.wav");

    g_water_sfx = asset_manager_load<Sound>("water_pour.wav");

    for(nkU32 i=0; i<PlantID_TOTAL; ++i)
    {
        const PlantDesc& desc = PLANT_TABLE[i];
        if(desc.texture)
        {
            asset_manager_load<Texture>(desc.texture);
        }
    }
}

GLOBAL void plant_tick(nkF32 dt)
{
    for(nkU64 i=0; i<g_world.plants.length; ++i)
    {
        Plant* p = &g_world.plants[i];
        if(p->id != PlantID_None)
        {
            // Do the plant's custom update logic.
            const PlantDesc& desc = PLANT_TABLE[p->id];
            if(desc.tick)
            {
                desc.tick(p, dt);
            }

            // Do phase logic.
            nkS32 max_phases = get_texture_height(get_plant_id_texture(p->id)) / TILE_HEIGHT;

            if(p->phase < max_phases-1)
            {
                nkF32 phase_duration = NK_CAST(nkF32, desc.phase_times[p->phase]);
                p->phase_timer += dt;
                if(p->phase_timer >= phase_duration)
                {
                    p->phase_timer -= phase_duration;
                    p->phase++;
                }
            }

            // Do animation logic.
            p->anim_timer += dt;
            if(p->anim_timer >= PLANT_ANIM_SPEED)
            {
                nkS32 max_frames = get_texture_width(get_plant_id_texture(p->id)) / TILE_WIDTH;

                p->anim_timer -= PLANT_ANIM_SPEED;
                p->anim_frame = ((p->anim_frame + 1) % max_frames);
            }
        }
    }
}

GLOBAL void plant_draw(void)
{
    for(nkU64 i=0; i<g_world.plants.length; ++i)
    {
        Plant* p = &g_world.plants[i];
        if(p->id != PlantID_None)
        {
            Texture texture = get_plant_id_texture(p->id);
            ImmClip clip = get_plant_clip(p);

            nkF32 px = NK_CAST(nkF32, p->x * TILE_WIDTH) + (NK_CAST(nkF32,TILE_WIDTH) * 0.5f);
            nkF32 py = NK_CAST(nkF32, p->y * TILE_HEIGHT) + (NK_CAST(nkF32,TILE_HEIGHT) * 0.5f);

            imm_texture(texture, px,py, &clip);
        }
    }
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

    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_shovel_sfx)-1);
    play_sound(g_shovel_sfx[sound_index]);

    const PlantDesc& desc = PLANT_TABLE[id];

    Plant plant = NK_ZERO_MEM;

    plant.id          = id;
    plant.x           = x;
    plant.y           = y;
    plant.w           = desc.width;
    plant.h           = desc.height;
    plant.health      = desc.health;
    plant.phase_timer = 0.0f;
    plant.phase       = 0;
    plant.anim_timer  = rng_f32(0.0f, PLANT_ANIM_SPEED);
    plant.anim_frame  = 0;

    nk_array_append(&g_world.plants, plant);

    return NK_TRUE;
}

GLOBAL PlantID remove_plant(nkS32 x, nkS32 y)
{
    // Is the spot in bounds.
    if(x < 0 || x >= g_world.width || y < 0 || y >= g_world.height)
    {
        return PlantID_None;
    }

    // Check for a plant and if so remove it.
    for(nkU64 i=0; i<g_world.plants.length; ++i)
    {
        Plant* p = &g_world.plants[i];
        if(p->x == x && p->y == y)
        {
            nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_shovel_sfx)-1);
            play_sound(g_shovel_sfx[sound_index]);
            PlantID id = p->id;
            nk_array_remove(&g_world.plants, i);
            return id;
        }
    }

    return PlantID_None;
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
            play_sound(g_water_sfx);
            // @Incomplete: Do the watering logic...
            return NK_TRUE;
        }
    }

    return NK_FALSE;
}

GLOBAL ImmClip get_plant_id_icon_clip(PlantID id)
{
    ImmClip clip;
    clip.x = NK_CAST(nkF32, id * TILE_WIDTH);
    clip.y = 0.0f;
    clip.w = NK_CAST(nkF32, TILE_WIDTH);
    clip.h = NK_CAST(nkF32, TILE_HEIGHT);
    return clip;
}

GLOBAL Texture get_plant_id_texture(PlantID id)
{
    NK_ASSERT(id < PlantID_TOTAL);
    return asset_manager_load<Texture>(PLANT_TABLE[id].texture);
}

GLOBAL ImmClip get_plant_clip(Plant* plant)
{
    NK_ASSERT(plant);
    ImmClip clip;
    clip.x = NK_CAST(nkF32, plant->anim_frame * TILE_WIDTH);
    clip.y = NK_CAST(nkF32, plant->phase * TILE_HEIGHT);
    clip.w = NK_CAST(nkF32, TILE_WIDTH);
    clip.h = NK_CAST(nkF32, TILE_HEIGHT);
    return clip;
}

GLOBAL const PlantDesc& get_plant_desc(PlantID id)
{
    NK_ASSERT(id < PlantID_TOTAL);
    return PLANT_TABLE[id];
}

/*////////////////////////////////////////////////////////////////////////////*/
