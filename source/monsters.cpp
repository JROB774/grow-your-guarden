/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL void mtick__walker(Monster* m, nkF32 dt)
{
    // @Incomplete: ...
}

/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr MonsterDesc MONSTER_TABLE[] =
{
{ NULL,          NULL,           0, {   0.0f,   0.0f,  0.0f,  0.0f } }, // None
{ mtick__walker, "monster.png", 10, { -16.0f, -16.0f, 32.0f, 32.0f } }, // Walker
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(MONSTER_TABLE) == MonsterID_TOTAL, monster_table_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkF32 MONSTER_ANIM_SPEED = 0.3f;

GLOBAL void monster_init(void)
{
    // Pre-load a bunch of assets.
    for(nkU32 i=0; i<MonsterID_TOTAL; ++i)
    {
        const MonsterDesc& desc = MONSTER_TABLE[i];
        if(desc.texture)
        {
            asset_manager_load<Texture>(desc.texture);
        }
    }
}

GLOBAL void monster_tick(nkF32 dt)
{
    for(auto& m: g_world.monsters)
    {
        if(m.id != MonsterID_None)
        {
            // Do the monster's custom update logic.
            const MonsterDesc& desc = MONSTER_TABLE[m.id];
            if(desc.tick)
            {
                desc.tick(&m, dt);
            }

            // Do animation logic.
            m.anim_timer += dt;
            if(m.anim_timer >= MONSTER_ANIM_SPEED)
            {
                nkS32 max_frames = get_texture_width(get_monster_id_texture(m.id)) / TILE_WIDTH;

                m.anim_timer -= MONSTER_ANIM_SPEED;
                m.anim_frame = ((m.anim_frame + 1) % max_frames);
            }
        }
    }
}

GLOBAL void monster_draw(void)
{
    for(auto& m: g_world.monsters)
    {
        if(m.id != MonsterID_None)
        {
            Texture texture = get_monster_id_texture(m.id);
            ImmClip clip = get_monster_clip(&m);

            nkF32 mx = m.position.x;
            nkF32 my = m.position.y;

            imm_texture(texture, mx,my, &clip);
        }
    }
}

GLOBAL void monster_spawn(MonsterID id, nkF32 x, nkF32 y)
{
    const MonsterDesc& desc = MONSTER_TABLE[id];

    Monster monster = NK_ZERO_MEM;

    monster.id          = id;
    monster.position    = { x,y };
    monster.bounds      = desc.bounds;
    monster.health      = desc.health;
    monster.anim_timer  = rng_f32(0.0f, MONSTER_ANIM_SPEED);
    monster.anim_frame  = 0;

    nk_array_append(&g_world.monsters, monster);
}

GLOBAL Texture get_monster_id_texture(MonsterID id)
{
    NK_ASSERT(id < MonsterID_TOTAL);
    return asset_manager_load<Texture>(MONSTER_TABLE[id].texture);
}

GLOBAL ImmClip get_monster_clip(Monster* monster)
{
    NK_ASSERT(monster);
    ImmClip clip;
    clip.x = NK_CAST(nkF32, monster->anim_frame * TILE_WIDTH);
    clip.y = 0.0f;
    clip.w = NK_CAST(nkF32, TILE_WIDTH);
    clip.h = NK_CAST(nkF32, TILE_HEIGHT);
    return clip;
}

/*////////////////////////////////////////////////////////////////////////////*/
