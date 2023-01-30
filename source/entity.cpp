/*////////////////////////////////////////////////////////////////////////////*/

// Track entity slots that were in use but no longer are as they can be filled without growing the array.
INTERNAL nkHashSet<nkU64> g_free_entity_slots;

GLOBAL void entity_init(void)
{
    // Pre-load all of the entity sprite textures and animation.
    for(nkU32 i=0; i<EntityID_TOTAL; ++i)
    {
        const EntityDesc& desc = ENTITY_TABLE[i];

        if(desc.texture_file)
            asset_manager_load<Texture>(desc.texture_file);
        if(desc.anim_file)
            asset_manager_load<AnimGroup*>(desc.anim_file, NULL, "textures/");
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
        if(e.id != EntityID_None && e.active)
        {
            // Do the entity's custom update logic.
            const EntityDesc& desc = ENTITY_TABLE[e.id];
            if(desc.tick)
            {
                desc.tick(e, dt);
            }

            // Apply velocity.
            e.position += e.velocity * dt;

            // Update animation logic.
            update_animation(&e.anim_state, dt);
        }
    }
}

GLOBAL void entity_draw(void)
{
    for(auto& e: g_world.entities)
    {
        if(e.id != EntityID_None && e.active)
        {
            Texture texture = asset_manager_load<Texture>(ENTITY_TABLE[e.id].texture_file);

            AnimFrame frame = get_current_animation_frame(&e.anim_state);
            ImmClip clip = { frame.x,frame.y,frame.w,frame.h };

            nkF32 ex = e.position.x;
            nkF32 ey = e.position.y;

            imm_texture(texture, ex,ey, &clip);
        }
    }
}

GLOBAL void entity_spawn(EntityID id, nkF32 x, nkF32 y)
{
    NK_ASSERT(id < EntityID_TOTAL);

    const EntityDesc& desc = ENTITY_TABLE[id];

    Entity entity     = NK_ZERO_MEM;
    entity.type       = desc.type;
    entity.id         = id;
    entity.position   = { x,y };
    entity.velocity   = { 0,0 };
    entity.health     = desc.health;
    entity.bounds     = desc.bounds;
    entity.anim_state = create_animation_state(desc.anim_file);
    entity.active     = NK_TRUE;

    set_animation(&entity.anim_state, desc.start_anim, NK_TRUE);

    // If there are free slots available then use them, otherwise append on the end (potentially grow memory).
    if(!nk_hashset_empty(&g_free_entity_slots))
    {
        auto slot = g_free_entity_slots.begin();
        g_world.entities[slot->key] = entity;
        nk_hashset_remove(&g_free_entity_slots, slot->key);
    }
    else
    {
        nk_array_append(&g_world.entities, entity);
    }
}

/*////////////////////////////////////////////////////////////////////////////*/
