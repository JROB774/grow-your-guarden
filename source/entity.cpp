/*////////////////////////////////////////////////////////////////////////////*/

// Track entity slots that were in use but no longer are as they can be filled without growing the array.
INTERNAL nkHashSet<nkU64> g_free_entity_slots;
INTERNAL Sound            g_splat_sfx[3];
INTERNAL Sound            g_monster_die_sfx;

INTERNAL nkS32 entity_sort_op(const void* a, const void* b)
{
    Entity* aa = *NK_CAST(Entity**, a);
    Entity* bb = *NK_CAST(Entity**, b);
    if((aa->position.y + (aa->bounds.y * 0.5f)) < (bb->position.y + (bb->bounds.y * 0.5f))) return -1;
    if((aa->position.y + (aa->bounds.y * 0.5f)) > (bb->position.y + (bb->bounds.y * 0.5f))) return +1;
    return 0;
}

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

    // @Incomplete: CREDIT: https://freesound.org/people/duckduckpony/sounds/204027/
    g_splat_sfx[0] = asset_manager_load<Sound>("splat_000.wav");
    g_splat_sfx[1] = asset_manager_load<Sound>("splat_001.wav");
    g_splat_sfx[2] = asset_manager_load<Sound>("splat_002.wav");

    // @IncompletE: CREDIT: https://freesound.org/people/Michel88/sounds/76962/
    g_monster_die_sfx = asset_manager_load<Sound>("monster_die.wav");
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

                // Check if we have collided with any plant bullets.
                case EntityType_Monster:
                {
                    nkU64 sub_index = 0;

                    for(auto& b: g_world.entities)
                    {
                        if(b.type == EntityType_Bullet && b.active && NK_CHECK_FLAGS(b.collision_mask, EntityType_Monster))
                        {
                            nkF32 mw = e.bounds.x;
                            nkF32 mh = e.bounds.y;
                            nkF32 mx = e.position.x - (mw * 0.5f);
                            nkF32 my = e.position.y - (mh * 0.5f);

                            nkF32 bw = b.bounds.x;
                            nkF32 bh = b.bounds.y;
                            nkF32 bx = b.position.x - (bw * 0.5f);
                            nkF32 by = b.position.y - (bh * 0.5f);

                            if(rect_vs_rect({ mx,my,mw,my }, { bx,by,bw,bh }))
                            {
                                entity_damage(index, b.damage);
                                entity_kill(sub_index);
                            }
                        }

                        ++sub_index;
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

            // Update damage timer.
            if(e.damage_timer > 0.0f)
            {
                e.damage_timer -= dt;
            }

            // Kill if dead.
            if(e.health < 0)
            {
                if(e.type == EntityType_Monster)
                    play_sound(g_monster_die_sfx);
                entity_kill(index);
            }
        }

        ++index;
    }
}

GLOBAL void entity_draw(void)
{
    // Sort the entities based on their Y position so that they draw in the correct order.
    nkArray<Entity*> entity_draw_list;
    nk_array_reserve(&entity_draw_list, g_world.entities.length);

    for(auto& e: g_world.entities)
    {
        if(e.id != EntityID_None && e.active)
        {
            nk_array_append(&entity_draw_list, &e);
        }
    }

    qsort(entity_draw_list.data, entity_draw_list.length, sizeof(Entity*), entity_sort_op);

    // Draw the sorted entities.
    for(Entity* e: entity_draw_list)
    {
        Texture texture = asset_manager_load<Texture>(ENTITY_TABLE[e->id].texture_file);

        AnimFrame frame = get_current_animation_frame(&e->anim_state);
        ImmClip clip = { frame.x,frame.y,frame.w,frame.h };

        nkF32 ex = e->position.x;
        nkF32 ey = e->position.y;

        nkVec4 color = (e->damage_timer > 0.0f) ? NK_V4_RED : NK_V4_WHITE;

        imm_texture(texture, ex,ey, &clip, color);
    }
}

GLOBAL void entity_damage(nkU64 index, nkF32 damage)
{
    if(index >= g_world.entities.length) return;

    // @Incomplete: Different sound effects!
    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_splat_sfx)-1);
    play_sound(g_splat_sfx[sound_index]);

    g_world.entities[index].health -= damage;
    g_world.entities[index].damage_timer = 0.1f;
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
    entity.speed          = desc.speed    * TILE_WIDTH;
    entity.range          = desc.range    * TILE_HEIGHT;
    entity.bounds.x       = desc.bounds.x * TILE_WIDTH;
    entity.bounds.y       = desc.bounds.y * TILE_HEIGHT;
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

GLOBAL nkU64 check_entity_collision(nkF32 x, nkF32 y, nkF32 w, nkF32 h, EntityType collision_mask)
{
    nkU64 index = 0;

    for(auto& e: g_world.entities)
    {
        if(e.id != EntityType_None && e.active && NK_CHECK_FLAGS(collision_mask, e.type))
        {
            nkF32 ex = e.position.x - (e.bounds.x * 0.5f);
            nkF32 ey = e.position.y - (e.bounds.y * 0.5f);
            nkF32 ew = e.bounds.x;
            nkF32 eh = e.bounds.y;

            if(rect_vs_rect({ x,y,w,h }, { ex,ey,ew,eh }))
            {
                return index;
            }
        }

        ++index;
    }

    return NK_U64_MAX;
}

GLOBAL nkU64 check_entity_collision(const Entity& e, EntityType collision_mask)
{
    nkF32 x = e.position.x - (e.bounds.x * 0.5f);
    nkF32 y = e.position.y - (e.bounds.y * 0.5f);
    nkF32 w = e.bounds.x;
    nkF32 h = e.bounds.y;

    return check_entity_collision(x,y,w,h, collision_mask);
}

GLOBAL nkU64 get_first_entity_with_id(EntityID id)
{
    nkU64 index = 0;
    for(auto& e: g_world.entities)
    {
        if(e.id == id && e.active)
            return index;
        ++index;
    }
    return NK_U64_MAX;
}

/*////////////////////////////////////////////////////////////////////////////*/
