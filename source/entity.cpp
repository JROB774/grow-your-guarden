/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void entity_init(void)
{
    // Pre-load all of the entity sprite textures.
    for(nkU32 i=0; i<EntityID_TOTAL; ++i)
    {
        const EntityDesc& desc = ENTITY_TABLE[i];
        if(desc.texture)
            asset_manager_load<Texture>(desc.texture);
    }
}

GLOBAL void entity_quit(void)
{
    // Does nothing...
}

GLOBAL void entity_tick(nkF32 dt)
{
    for(auto& e: g_world.entities)
    {
        if(e.id != EntityID_None)
        {
            // Do the entity's custom update logic.
            const EntityDesc& desc = ENTITY_TABLE[e.id];
            if(desc.tick)
            {
                desc.tick(e, dt);
            }

            // @Incomplete: ...
            /*
            // Do animation logic.
            m.anim_timer += dt;
            if(m.anim_timer >= MONSTER_ANIM_SPEED)
            {
                nkS32 max_frames = get_texture_width(get_monster_id_texture(m.id)) / TILE_WIDTH;

                m.anim_timer -= MONSTER_ANIM_SPEED;
                m.anim_frame = ((m.anim_frame + 1) % max_frames);
            }
            */
        }
    }
}

GLOBAL void entity_draw(void)
{
    for(auto& e: g_world.entities)
    {
        if(e.id != EntityID_None)
        {
            // @Incomplete: Render...
        }
    }
}

GLOBAL void entity_spawn(EntityID id, nkF32 x, nkF32 y)
{
    // @Incomplete: ...
}

/*////////////////////////////////////////////////////////////////////////////*/
