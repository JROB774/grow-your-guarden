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
    nkU64 index = 0;

    for(auto& e: g_world.entities)
    {
        if(e.id != EntityID_None && e.active)
        {
            const EntityDesc& desc = ENTITY_TABLE[e.id];

            // Do entity-type specific update logic.
            switch(e.type)
            {
                // Time the current phase and automatically advance to the next.
                case EntityType_Plant:
                {
                    nkS32 max_phases = 0;
                    while(desc.phase_times[max_phases] > 0.0f)
                        max_phases++;
                    max_phases++;
                    if(e.current_phase < max_phases-1)
                    {
                        nkF32 phase_duration = desc.phase_times[e.current_phase];
                        e.phase_timer += dt;
                        if(e.phase_timer >= phase_duration)
                        {
                            e.phase_timer -= phase_duration;
                            e.current_phase++;
                        }
                    }
                } break;

                // Check if we have reached our max range and if so die.
                case EntityType_Bullet:
                {
                    nkF32 distance = distance_between_points(e.position, e.spawn);
                    if(distance >= e.range)
                    {
                        entity_kill(index);
                    }
                } break;
            }

            // Do the entity's custom update logic.
            if(desc.tick)
            {
                desc.tick(e, dt);
            }

            // Apply velocity.
            e.position += e.velocity * dt;

            // Update animation logic.
            update_animation(&e.anim_state, dt);
        }

        ++index;
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

GLOBAL void entity_kill(nkU64 index)
{
    if(index >= g_world.entities.length) return;
    g_world.entities[index].active = NK_FALSE;
    nk_hashset_insert(&g_free_entity_slots, index);
}

GLOBAL nkU64 entity_spawn(EntityID id, nkF32 x, nkF32 y)
{
    NK_ASSERT(id < EntityID_TOTAL);

    const EntityDesc& desc = ENTITY_TABLE[id];

    Entity entity         = NK_ZERO_MEM;
    entity.type           = desc.type;
    entity.id             = id;
    entity.position       = { x,y };
    entity.spawn          = { x,y };
    entity.velocity       = { 0,0 };
    entity.health         = desc.health;
    entity.damage         = desc.damage;
    entity.speed          = desc.speed;
    entity.range          = desc.range;
    entity.bounds         = desc.bounds;
    entity.anim_state     = create_animation_state(desc.anim_file);
    entity.collision_mask = desc.collision_mask;
    entity.current_phase  = 0;
    entity.phase_timer    = 0.0f;
    entity.timer0         = 0.0f;
    entity.timer1         = 0.0f;
    entity.timer2         = 0.0f;
    entity.timer3         = 0.0f;
    entity.active         = NK_TRUE;

    set_animation(&entity.anim_state, desc.start_anim, NK_TRUE);

    // If there are free slots available then use them, otherwise append on the end (potentially grow memory).
    if(!nk_hashset_empty(&g_free_entity_slots))
    {
        nkU64 index = g_free_entity_slots.begin()->key;
        g_world.entities[index] = entity;
        nk_hashset_remove(&g_free_entity_slots, index);
        return index;
    }
    else
    {
        nk_array_append(&g_world.entities, entity);
        return (g_world.entities.length - 1);
    }
}

/*////////////////////////////////////////////////////////////////////////////*/
