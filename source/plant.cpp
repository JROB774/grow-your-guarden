/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL void plant_tick__flower(Plant* p, nkF32 dt)
{
    // @Incomplete: ...
}

/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL constexpr PlantDesc PLANT_DESC_TABLE[] =
{
{ NULL,               NULL,         0, 0,0 }, // None
{ plant_tick__flower, "flower.png", 3, 1,1 }, // Flower
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(PLANT_DESC_TABLE) == PlantID_TOTAL, plant_desc_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkF32 PLANT_ANIM_SPEED = 0.3f;

GLOBAL void plant_tick(Plant* plants, nkU64 count, nkF32 dt)
{
    NK_ASSERT(plants);

    for(nkU64 i=0; i<count; ++i)
    {
        Plant* p = &plants[i];
        if(p->id != PlantID_None)
        {
            // Do the plant's custom update logic.
            const PlantDesc& desc = PLANT_DESC_TABLE[p->id];
            if(desc.tick)
            {
                desc.tick(p, dt);
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

GLOBAL void plant_draw(Plant* plants, nkU64 count)
{
    NK_ASSERT(plants);

    for(nkU64 i=0; i<count; ++i)
    {
        Plant* p = &plants[i];
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
    return asset_manager_load<Texture>(PLANT_DESC_TABLE[id].texture);
}

GLOBAL ImmClip get_plant_clip(Plant* plant)
{
    ImmClip clip;
    clip.x = NK_CAST(nkF32, plant->anim_frame * TILE_WIDTH);
    clip.y = NK_CAST(nkF32, plant->phase * TILE_HEIGHT);
    clip.w = NK_CAST(nkF32, TILE_WIDTH);
    clip.h = NK_CAST(nkF32, TILE_HEIGHT);
    return clip;
}

/*////////////////////////////////////////////////////////////////////////////*/
