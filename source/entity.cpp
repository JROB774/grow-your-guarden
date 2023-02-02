/*////////////////////////////////////////////////////////////////////////////*/

struct EntityManager
{
    nkArray<Entity>  entities;
    nkHashSet<nkU64> free_entity_slots; // Track entity slots that were in use but no longer are as they can be filled without growing the array.
    Sound            splat_sfx[3];
    Sound            monster_die_sfx;
    nkBool           draw_colliders;
};

INTERNAL EntityManager g_entity_manager;

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
    g_entity_manager.splat_sfx[0] = asset_manager_load<Sound>("splat_000.wav");
    g_entity_manager.splat_sfx[1] = asset_manager_load<Sound>("splat_001.wav");
    g_entity_manager.splat_sfx[2] = asset_manager_load<Sound>("splat_002.wav");

    // @IncompletE: CREDIT: https://freesound.org/people/Michel88/sounds/76962/
    g_entity_manager.monster_die_sfx = asset_manager_load<Sound>("monster_die.wav");
}

GLOBAL void entity_quit(void)
{
    nk_array_free(&g_entity_manager.entities);
    nk_hashset_free(&g_entity_manager.free_entity_slots);
}

GLOBAL void entity_tick(nkF32 dt)
{
    #if defined(BUILD_DEBUG)
    if(is_key_pressed(KeyCode_F1))
        g_entity_manager.draw_colliders = !g_entity_manager.draw_colliders;
    #endif // BUILD_DEBUG

    nkU64 index = 0;

    for(auto& e: g_entity_manager.entities)
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

                // Check if we have reached our max range and if so drop off, once we hit the floor die.
                case EntityType_Bullet:
                {
                    const nkF32 BULLET_DROPOFF_SPEED = 300.0f;
                    nkF32 distance = distance_between_points(e.position, e.spawn);
                    if(distance >= e.range)
                    {
                        e.z_depth -= BULLET_DROPOFF_SPEED * dt;
                        if(e.z_depth <= 0.0f)
                        {
                            entity_kill(index);
                        }
                    }
                } break;

                // Check if we have collided with any plant bullets.
                case EntityType_Monster:
                {
                    nkU64 sub_index = 0;

                    for(auto& b: g_entity_manager.entities)
                    {
                        if(b.type == EntityType_Bullet && b.active && NK_CHECK_FLAGS(b.collision_mask, EntityType_Monster))
                        {
                            if(check_entity_collision(e, b))
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
            if(e.health <= 0.0f)
            {
                if(e.type == EntityType_Monster)
                    play_sound(g_entity_manager.monster_die_sfx);
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
    nk_array_reserve(&entity_draw_list, g_entity_manager.entities.length);

    for(auto& e: g_entity_manager.entities)
    {
        if(e.id != EntityID_None && e.active)
        {
            nk_array_append(&entity_draw_list, &e);
        }
    }

    qsort(entity_draw_list.data, entity_draw_list.length, sizeof(Entity*), entity_sort_op);

    Texture shadow = asset_manager_load<Texture>("shadow.png");

    // Draw the sorted entities.
    for(Entity* e: entity_draw_list)
    {
        Texture texture = asset_manager_load<Texture>(ENTITY_TABLE[e->id].texture_file);

        AnimFrame frame = get_current_animation_frame(&e->anim_state);
        ImmClip clip = { frame.x,frame.y,frame.w,frame.h };

        nkF32 ex = e->position.x + e->draw_offset.x;
        nkF32 ey = e->position.y + e->draw_offset.y;

        nkVec4 color = (e->damage_timer > 0.0f) ? NK_V4_RED : NK_V4_WHITE;

        // Bullets and monsters have shadows.
        if(e->type == EntityType_Monster || e->type == EntityType_Bullet)
        {
            nkVec4 shadow_color = { 1.0f,1.0f,1.0f,0.25f };

            nkF32 pos_x = e->position.x + e->draw_offset.x;
            nkF32 pos_y = e->position.y + e->draw_offset.y + (e->bounds.y * 0.4f);

            nkF32 scale = e->bounds.x / NK_CAST(nkF32, get_texture_width(shadow));

            imm_texture_ex(shadow, pos_x,pos_y, scale,scale*0.75f, 0.0f, NULL, NULL, shadow_color);
        }

        imm_texture_ex(texture, ex,ey - e->z_depth, e->flip, 1.0f, 0.0f, NULL, &clip, color);

        #if defined(BUILD_DEBUG)
        if(g_entity_manager.draw_colliders)
        {
            nkVec4 collider_color = { 1.0f,1.0f,1.0f,0.5f };
            switch(e->type)
            {
                case EntityType_Plant:   collider_color = { 0.0f,1.0f,0.0f,0.5f }; break;
                case EntityType_Monster: collider_color = { 1.0f,0.0f,0.0f,0.5f }; break;
                case EntityType_Base:    collider_color = { 0.0f,0.0f,1.0f,0.5f }; break;
                case EntityType_Bullet:  collider_color = { 1.0f,1.0f,0.0f,0.5f }; break;
                case EntityType_Object:  collider_color = { 0.0f,1.0f,1.0f,0.5f }; break;
            }

            nkF32 pos_x = e->position.x;
            nkF32 pos_y = e->position.y;

            imm_circle_filled(pos_x,pos_y, e->radius, 48, collider_color);
        }
        #endif // BUILD_DEBUG
    }
}

GLOBAL void entity_damage(nkU64 index, nkF32 damage)
{
    if(index >= g_entity_manager.entities.length) return;

    // @Incomplete: Different sound effects!
    nkS32 sound_index = rng_s32(0,NK_ARRAY_SIZE(g_entity_manager.splat_sfx)-1);
    play_sound(g_entity_manager.splat_sfx[sound_index]);

    g_entity_manager.entities[index].health -= damage;
    g_entity_manager.entities[index].damage_timer = 0.1f;
}

GLOBAL void entity_kill(nkU64 index)
{
    if(index >= g_entity_manager.entities.length) return;
    g_entity_manager.entities[index].active = NK_FALSE;
    nk_hashset_insert(&g_entity_manager.free_entity_slots, index);
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
    entity.speed          = desc.speed   * TILE_WIDTH;
    entity.range          = desc.range   * TILE_HEIGHT;
    entity.radius         = desc.radius  * TILE_WIDTH;
    entity.z_depth        = desc.z_depth * TILE_HEIGHT;
    entity.flip           = 1.0f;
    entity.anim_state     = create_animation_state(desc.anim_file);
    entity.collision_mask = desc.collision_mask;
    entity.draw_offset.x  = desc.draw_offset.x * TILE_WIDTH;
    entity.draw_offset.y  = desc.draw_offset.y * TILE_HEIGHT;
    entity.bounds.x       = desc.bounds.x      * TILE_WIDTH;
    entity.bounds.y       = desc.bounds.y      * TILE_HEIGHT;
    entity.current_phase  = 0;
    entity.phase_timer    = 0.0f;
    entity.timer0         = 0.0f;
    entity.timer1         = 0.0f;
    entity.timer2         = 0.0f;
    entity.timer3         = 0.0f;
    entity.active         = NK_TRUE;

    set_animation(&entity.anim_state, desc.start_anim, NK_TRUE);

    // Plants can be flipped for more visual variance.
    if(entity.type == EntityType_Plant && rng_s32(0,100) < 50)
    {
        entity.flip = -1.0f;
    }

    // If there are free slots available then use them, otherwise append on the end (potentially grow memory).
    if(!nk_hashset_empty(&g_entity_manager.free_entity_slots))
    {
        nkU64 index = g_entity_manager.free_entity_slots.begin()->key;
        g_entity_manager.entities[index] = entity;
        nk_hashset_remove(&g_entity_manager.free_entity_slots, index);
        return index;
    }
    else
    {
        nk_array_append(&g_entity_manager.entities, entity);
        return (g_entity_manager.entities.length - 1);
    }
}


GLOBAL nkU64 check_entity_bounds(nkF32 x, nkF32 y, nkF32 w, nkF32 h, EntityType collision_mask)
{
    nkU64 index = 0;

    for(auto& e: g_entity_manager.entities)
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
    if(e.active)
    {
        nkU64 index = 0;

        for(auto& o: g_entity_manager.entities)
        {
            if(o.id != EntityType_None && o.active && NK_CHECK_FLAGS(collision_mask, o.type))
            {
                nkF32 ax = e.position.x;
                nkF32 ay = e.position.y;
                nkF32 ar = e.radius;

                nkF32 bx = o.position.x;
                nkF32 by = o.position.y;
                nkF32 br = o.radius;

                if(circle_vs_circle(ax,ay,ar, bx,by,br))
                {
                    return index;
                }
            }

            ++index;
        }
    }

    return NK_U64_MAX;
}

GLOBAL nkBool check_entity_collision(const Entity& a, const Entity& b)
{
    if(!a.active || !b.active) return NK_FALSE;

    nkF32 ax = a.position.x;
    nkF32 ay = a.position.y;
    nkF32 ar = a.radius;

    nkF32 bx = b.position.x;
    nkF32 by = b.position.y;
    nkF32 br = b.radius;

    return circle_vs_circle(ax,ay,ar, bx,by,br);
}

GLOBAL nkU64 get_entity_count(void)
{
    return g_entity_manager.entities.length;
}

GLOBAL Entity* get_entity(nkU64 index)
{
    if(index >= g_entity_manager.entities.length) return NULL;
    Entity* e = &g_entity_manager.entities[index];
    return (e->active) ? e : NULL;
}

GLOBAL Entity* get_first_entity_with_id(EntityID id)
{
    return get_entity(get_first_entity_index_with_id(id));
}

GLOBAL nkU64 get_first_entity_index_with_id(EntityID id)
{
    nkU64 index = 0;
    for(auto& e: g_entity_manager.entities)
    {
        if(e.id == id && e.active)
            return index;
        ++index;
    }
    return NK_U64_MAX;
}

/*////////////////////////////////////////////////////////////////////////////*/
