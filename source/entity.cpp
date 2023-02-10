/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr const nkChar* ENTITY_ANIM_NAMES[] = { NULL, "idle", "move", "hurt", "attack", "dead" };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(ENTITY_ANIM_NAMES) == EntityState_TOTAL, entity_anim_size_mismatch);

struct EntityManager
{
    nkArray<Entity>  entities;
    nkHashSet<nkU64> free_entity_slots; // Track entity slots that were in use but no longer are as they can be filled without growing the array.
    Texture          shadow_texture;
    nkBool           draw_colliders;
};

INTERNAL EntityManager g_entity_manager;

INTERNAL nkS32 entity_sort_op(const void* a, const void* b)
{
    Entity* aa = *NK_CAST(Entity**, a);
    Entity* bb = *NK_CAST(Entity**, b);

    if(NK_CHECK_FLAGS(aa->flags, EntityFlag_DrawFirst) && !NK_CHECK_FLAGS(bb->flags, EntityFlag_DrawFirst))
    {
        return -1;
    }
    if(!NK_CHECK_FLAGS(aa->flags, EntityFlag_DrawFirst) && NK_CHECK_FLAGS(bb->flags, EntityFlag_DrawFirst))
    {
        return +1;
    }

    if((aa->position.y + aa->draw_offset.y + (aa->bounds.y * 0.5f)) < (bb->position.y + bb->draw_offset.y + (bb->bounds.y * 0.5f))) return -1;
    if((aa->position.y + aa->draw_offset.y + (aa->bounds.y * 0.5f)) > (bb->position.y + bb->draw_offset.y + (bb->bounds.y * 0.5f))) return +1;
    if((aa->position.x + aa->draw_offset.x + (aa->bounds.x * 0.5f)) < (bb->position.x + bb->draw_offset.x + (bb->bounds.x * 0.5f))) return -1;
    if((aa->position.x + aa->draw_offset.x + (aa->bounds.x * 0.5f)) > (bb->position.x + bb->draw_offset.x + (bb->bounds.x * 0.5f))) return +1;

    return 0;
}

INTERNAL nkString get_current_entity_anim_name(Entity& e)
{
    nkString name;
    if(e.type == EntityType_Plant)
        name = format_string("phase%d_", e.current_phase);
    nk_string_append(&name, ENTITY_ANIM_NAMES[e.state]);
    return name;
}

INTERNAL void fully_kill_entity(Entity& e, nkU64 index)
{
    // This is what happens once an entities death animation completes (if it has one).
    e.active = NK_FALSE;
    if(!nk_hashset_contains(&g_entity_manager.free_entity_slots, index)) // Just do a check here as we were running into issues with attempting to store the same value twice.
    {
        nk_hashset_insert(&g_entity_manager.free_entity_slots, index);
    }

    // Go through and unset any references to this entity as they are no longer valid.
    // Handling this automatically here simplifies things a lot for the entity logic.
    for(auto& entity: g_entity_manager.entities)
    {
        if(entity.id != EntityID_None && entity.active && entity.target != NO_TARGET)
        {
            if(!g_entity_manager.entities[entity.target].active)
            {
                entity.target = NO_TARGET;
            }
        }
    }
}

GLOBAL void entity_init(void)
{
    // Pre-load all of the entity assets.
    g_entity_manager.shadow_texture = asset_manager_load<Texture>("shadow.png");

    for(nkU32 i=0; i<EntityID_TOTAL; ++i)
    {
        const EntityDesc& desc = ENTITY_TABLE[i];

        if(desc.texture)
        {
            asset_manager_load<Texture>(desc.texture);
        }
        if(desc.animation)
        {
            asset_manager_load<AnimGroup*>(desc.animation);
        }
        for(nkS32 j=0,n=NK_ARRAY_SIZE(desc.death_sounds); j<n; ++j)
        {
            if(desc.death_sounds[j])
            {
                asset_manager_load<Sound>(desc.death_sounds[j]);
            }
        }
    }
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

                            // Update the animation for the new phase.
                            set_animation(&e.anim_state, get_current_entity_anim_name(e).cstr);
                        }
                    }

                    // Update the bouncing effect.
                    const nkF32 BOUNCE_SPEED = 0.5f;

                    e.bounce_timer += (dt * BOUNCE_SPEED);
                    if(e.bounce_timer > 1.0f)
                    {
                        e.bounce_timer = 1.0f;
                    }

                    // If a plant is fertilized then do some extra effects.
                    if(e.fertilized_timer > 0.0f)
                    {
                        const nkF32 PULSE_SPEED = 5.0f;

                        e.color.r = nk_sin_range(1.0f, 2.0f, e.fertilized_timer * PULSE_SPEED);
                        e.color.g = nk_sin_range(1.0f, 2.0f, e.fertilized_timer * PULSE_SPEED);
                        e.color.b = nk_sin_range(1.0f, 1.4f, e.fertilized_timer * PULSE_SPEED);

                        if(rng_s32(0,100) < 20)
                        {
                            nkF32 x = e.position.x - (e.radius * 1.5f);
                            nkF32 y = e.position.y - (e.radius * 1.5f);
                            nkF32 w = (e.radius * 1.5f) * 2.0f;
                            nkF32 h = (e.radius * 1.5f) * 2.0f;

                            particle_spawn("sparkle", x,y,e.z_depth,w,h, 1,1);
                        }
                    }
                    else
                    {
                        e.color = NK_V4_WHITE;
                    }
                } break;

                // Check if we have reached our max range and if so drop off, once we hit the floor die.
                case EntityType_Bullet:
                {
                    if(e.state != EntityState_Dead && e.id != EntityID_Rocket) // Rockets are an exception...
                    {
                        const nkF32 BULLET_DROPOFF_SPEED = 300.0f;
                        nkF32 distance = distance_between_points(e.position, e.spawn);
                        if(distance >= e.range)
                        {
                            e.z_depth -= BULLET_DROPOFF_SPEED * dt;
                            if(e.z_depth <= 0.0f)
                            {
                                entity_kill(index);
                                e.velocity = NK_V2_ZERO;
                            }
                        }
                    }
                } break;
            }

            // Check if we have collided with any bullets.
            if(e.state != EntityState_Dead && !NK_CHECK_FLAGS(e.flags, EntityFlag_NotShootable))
            {
                nkU64 sub_index = 0;
                for(auto& b: g_entity_manager.entities)
                {
                    if(b.id != EntityID_Rocket) // Rockets are an exception...
                    {
                        if(b.type == EntityType_Bullet && b.state != EntityState_Dead && b.active && NK_CHECK_FLAGS(b.collision_mask, e.type))
                        {
                            // If the entity or bullet is aerial do a 3D collision check otherwise just do a normal one.
                            nkBool collided = NK_FALSE;
                            if(NK_CHECK_FLAGS(e.flags, EntityFlag_Aerial) || NK_CHECK_FLAGS(b.flags, EntityFlag_Aerial))
                            {
                                collided = check_entity_collision_3d(e, b);
                            }
                            else
                            {
                                collided = check_entity_collision(e, b);
                            }
                            if(collided)
                            {
                                entity_damage(index, b.damage);
                                entity_kill(sub_index);
                                b.velocity = NK_V2_ZERO;
                            }
                        }
                    }
                    ++sub_index;
                }
            }

            // Do the entity's custom update logic.
            if(desc.tick)
            {
                desc.tick(e, index, dt);
            }

            // Apply velocity.
            e.position += e.velocity * dt;
            e.z_depth += e.thrust * dt;

            if(e.z_depth < 0.0f)
            {
                e.z_depth = 0.0f;
            }

            // Update state and animation logic.
            update_animation(&e.anim_state, dt);
            if(is_animation_done(&e.anim_state) && e.state != desc.default_state)
            {
                // Special case for death.
                if(e.state != EntityState_Dead)
                    change_entity_state(e, desc.default_state);
                else
                {
                    fully_kill_entity(e, index);
                }
            }

            // Update damage timer.
            if(e.damage_timer > 0.0f)
            {
                e.damage_timer -= dt;
            }

            // Update fertilized timer.
            if(e.fertilized_timer > 0.0f)
            {
                e.fertilized_timer -= dt;
            }

            // Kill if dead.
            if(e.health <= 0.0f)
            {
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
        if(e.id != EntityID_None && e.active && !NK_CHECK_FLAGS(e.flags, EntityFlag_Hidden))
        {
            nk_array_append(&entity_draw_list, &e);
        }
    }

    qsort(entity_draw_list.data, entity_draw_list.length, sizeof(Entity*), entity_sort_op);

    // Draw all the entity shadows first.
    imm_begin_texture_batch(g_entity_manager.shadow_texture);
    for(Entity* e: entity_draw_list)
    {
        // Only bullets, monsters, and items have shadows.
        if((e->type == EntityType_Monster || e->type == EntityType_Bullet || e->type == EntityType_Item) && (e->state != EntityState_Dead))
        {
            nkVec4 shadow_color = { 1.0f,1.0f,1.0f,0.25f };

            nkF32 pos_x = e->position.x + e->draw_offset.x;
            nkF32 pos_y = e->position.y + e->draw_offset.y + (e->bounds.y * 0.4f);

            nkF32 scale = e->bounds.x / NK_CAST(nkF32, get_texture_width(g_entity_manager.shadow_texture));
            if(e->type == EntityType_Bullet)
            {
                scale *= 0.65f; // Make bullet shadows even smaller because it looks nice.
            }

            // Make z-depth change the size of the shadows.
            if(e->z_depth > 0.0f)
            {
                nkF32 depth_in_tiles = (e->z_depth / TILE_HEIGHT) * 0.75f;
                if(depth_in_tiles >= 1.0f)
                {
                    scale /= depth_in_tiles;
                    if(scale < 0.05f) scale = 0.05f;
                }
            }

            imm_texture_batched_ex(pos_x,pos_y, scale,scale*0.75f, 0.0f, NULL, NULL, shadow_color);
        }
    }
    imm_end_texture_batch();

    // Draw the sorted entities.
    for(Entity* e: entity_draw_list)
    {
        Texture texture = asset_manager_load<Texture>(ENTITY_TABLE[e->id].texture);

        AnimFrame frame = get_current_animation_frame(&e->anim_state);
        ImmClip clip = { frame.x,frame.y,frame.w,frame.h };

        nkF32 ex = e->position.x + e->draw_offset.x;
        nkF32 ey = e->position.y + e->draw_offset.y;

        nkF32 sx = e->flip_x;
        nkF32 sy = e->flip_y;

        nkVec4 color = (e->damage_timer > 0.0f) ? NK_V4_RED : e->color;

        if(e->bounce_timer < 1.0f)
        {
            nkF32 ease = nk_max(0.75f, ease_out_elastic(e->bounce_timer));

            sx = ease * e->flip_x;
            sy = ease * e->flip_y;
        }

        imm_texture_ex(texture, ex,ey - e->z_depth, sx,sy, e->angle, NULL, &clip, color);

        // A special case is made for the home tree. We draw different mouths on top of the base image.
        PERSISTENT const ImmClip TREE_FACE_HAPPY = { 1280.0f,   0.0f, 420.0f, 128.0f };
        PERSISTENT const ImmClip TREE_FACE_SAD   = { 1280.0f, 128.0f, 420.0f, 128.0f };
        PERSISTENT const ImmClip TREE_FACE_HURT  = { 1280.0f, 256.0f, 420.0f, 128.0f };

        if(e->id == EntityID_HomeTree)
        {
            ImmClip mouth_clip = (e->health > 50.0f) ? TREE_FACE_HAPPY : TREE_FACE_SAD;
            if(e->state == EntityState_Hurt || e->state == EntityState_Dead)
            {
                mouth_clip = TREE_FACE_HURT;
            }

            nkF32 mx = ex - (1280 * 0.5f) +  652.0f;
            nkF32 my = ey - (1280 * 0.5f) + 1006.0f;

            imm_texture_ex(texture, mx,my - e->z_depth, e->flip_x,e->flip_y, 0.0f, NULL, &mouth_clip, color);
        }
    }

    // Draw debug colliders on top.
    #if defined(BUILD_DEBUG)
    if(g_entity_manager.draw_colliders)
    {
        for(Entity* e: entity_draw_list)
        {
            nkVec4 collider_color = { 1.0f,1.0f,1.0f,0.5f };
            switch(e->type)
            {
                case EntityType_Plant:   collider_color = { 0.0f,1.0f,0.0f,0.5f }; break;
                case EntityType_Monster: collider_color = { 1.0f,0.0f,0.0f,0.5f }; break;
                case EntityType_Base:    collider_color = { 0.0f,0.0f,1.0f,0.5f }; break;
                case EntityType_Bullet:  collider_color = { 1.0f,1.0f,0.0f,0.5f }; break;
                case EntityType_Item:    collider_color = { 0.0f,1.0f,1.0f,0.5f }; break;
            }

            nkF32 pos_x = e->position.x;
            nkF32 pos_y = e->position.y;

            imm_circle_filled(pos_x,pos_y, e->radius, 48, collider_color);
        }
    }
    #endif // BUILD_DEBUG
}

GLOBAL void entity_reset(void)
{
    nk_array_clear(&g_entity_manager.entities);
    nk_hashset_clear(&g_entity_manager.free_entity_slots);
}

GLOBAL void entity_damage(nkU64 index, nkF32 damage)
{
    if(index >= g_entity_manager.entities.length) return;

    Entity* e = get_entity(index);
    if(!e || e->state == EntityState_Dead) return;

    // Special case to handle functionality of the Hedge Wall plant.
    if(e->id == EntityID_HedgeWall)
    {
        if(plant_is_fully_grown(*e))
        {
            damage *= 0.5f; // Damage is halved when fully grown (essentially doubling the plant's health).
        }
        else if(e->current_phase == 0)
        {
            damage *= 4.0f; // We take more damage as a seed to simulate less health.
        }
        if(e->fertilized_timer > 0.0f)
        {
            damage *= 0.5f; // All damage is halved when fertilized.
        }
    }

    e->health -= damage;
    e->damage_timer = 0.1f;

    change_entity_state(index, EntityState_Hurt);
    if(e->health <= 0.0f)
    {
        change_entity_state(index, EntityState_Dead);
    }
}

GLOBAL void entity_kill(nkU64 index)
{
    if(index >= g_entity_manager.entities.length) return;
    change_entity_state(index, EntityState_Dead);

    Entity* e = get_entity(index);
    if(e)
    {
        // If the entity doesn't have a death animation then just handle the death logic straight away!
        if(!has_animation(&e->anim_state, get_current_entity_anim_name(*e).cstr))
        {
            fully_kill_entity(*e, index);
        }

        const EntityDesc& desc = ENTITY_TABLE[e->id];

        // If the entity has death sounds then pick one and play it.
        nkS32 max_sounds = 0;
        while(desc.death_sounds[max_sounds])
            max_sounds++;
        if(max_sounds > 0)
        {
            nkS32 sound_index = rng_s32(0,max_sounds-1);
            play_sound(asset_manager_load<Sound>(desc.death_sounds[sound_index]));
        }

        // If the entity has death particles then spawn them.
        if(desc.death_effect)
        {
            nkF32 x = e->position.x + e->draw_offset.x;
            nkF32 y = e->position.y + e->draw_offset.y;

            // Making a special case for the goliath to spawn more of them...
            if(e->id == EntityID_Goliath)
            {
                nkF32 w = e->radius * 2.0f;
                nkF32 h = e->radius * 2.0f;

                particle_spawn(desc.death_effect, x,y,e->z_depth, w,h, 20,30);
            }
            else
            {
                particle_spawn(desc.death_effect, x,y, e->z_depth);
            }
        }

        if(desc.death_particle)
        {
            nkF32 x = e->position.x - e->radius;
            nkF32 y = e->position.y - e->radius;
            nkF32 w = e->radius * 2.0f;
            nkF32 h = e->radius * 2.0f;

            particle_spawn(desc.death_particle, x,y,e->z_depth,w,h, desc.death_particle_min, desc.death_particle_max);
        }

        // If the entity has death decals then spawn them.
        if(desc.death_decal)
        {
            nkF32 x = e->position.x - (e->radius * 1.5f);
            nkF32 y = e->position.y - (e->radius * 1.5f);
            nkF32 w = (e->radius * 1.5f) * 2.0f;
            nkF32 h = (e->radius * 1.5f) * 2.0f;

            decal_spawn(desc.death_decal, x,y,w,h, desc.death_decal_min, desc.death_decal_max, 15.0f,20.0f);
        }

        // Increment the kill count if it's a monster.
        if(e->type == EntityType_Monster)
        {
            increment_kill_count();
        }

        // Potentially spawn some coins!
        if(rng_s32(1,100) <= desc.coin_chance)
        {
            nkS32 count = rng_s32(desc.coin_min, desc.coin_max);
            for(nkS32 i=0; i<count; ++i)
            {
                EntityID id = EntityID_None;

                nkF32 x = rng_f32(e->position.x - e->radius, e->position.x + e->radius);
                nkF32 y = rng_f32(e->position.y - e->radius, e->position.y + e->radius);
                nkF32 z = rng_f32(e->z_depth+50 - e->radius, e->z_depth+50 + e->radius);

                if(z < 0.0f) z = 0.5f; // Prevent coins spawning in the floor and not flying through the air first.

                nkS32 type = rng_s32(0,100);

                if(type >=  0) id = EntityID_CoinCopper;
                if(type >= 65) id = EntityID_CoinSilver;
                if(type >= 90) id = EntityID_CoinGold;

                nkU64 coin_index = entity_spawn(id, x,y,z);
                Entity* coin = get_entity(coin_index);

                // Apply some force to the coin to make it fly out of the monster.
                coin->velocity = nk_normalize(nk_rotate(NK_V2_UNIT_X, rng_f32(0.0f, NK_TAU_F32))) * rng_f32(150,400);
                coin->thrust   = rng_f32(1200.0f,1400.0f);
            }
        }
    }
}

GLOBAL nkU64 entity_spawn(EntityID id, nkF32 x, nkF32 y, nkF32 z)
{
    NK_ASSERT(id < EntityID_TOTAL);

    const EntityDesc& desc = ENTITY_TABLE[id];

    Entity entity           = NK_ZERO_MEM;
    entity.type             = desc.type;
    entity.id               = id;
    entity.state            = EntityState_None; // Properly set further down!
    entity.flags            = desc.flags;
    entity.target           = NO_TARGET;
    entity.position         = { x,y };
    entity.spawn            = { x,y };
    entity.velocity         = { 0,0 };
    entity.health           = desc.health;
    entity.damage           = desc.damage;
    entity.speed            = desc.speed   * TILE_WIDTH;
    entity.range            = desc.range   * TILE_HEIGHT;
    entity.radius           = desc.radius  * TILE_WIDTH;
    entity.thrust           = 0.0f;
    entity.z_depth          = desc.z_depth * TILE_HEIGHT;
    entity.flip_x           = 1.0f;
    entity.flip_y           = 1.0f;
    entity.angle            = 0.0f;
    entity.color            = NK_V4_WHITE;
    entity.anim_state       = create_animation_state(desc.animation);
    entity.collision_mask   = desc.collision_mask;
    entity.draw_offset.x    = desc.draw_offset.x * TILE_WIDTH;
    entity.draw_offset.y    = desc.draw_offset.y * TILE_HEIGHT;
    entity.bounds.x         = desc.bounds.x      * TILE_WIDTH;
    entity.bounds.y         = desc.bounds.y      * TILE_HEIGHT;
    entity.current_phase    = 0;
    entity.phase_timer      = 0.0f;
    entity.fertilized_timer = 0.0f;
    entity.bounce_timer     = 1.0f;
    entity.timer0           = 0.0f;
    entity.timer1           = 0.0f;
    entity.timer2           = 0.0f;
    entity.timer3           = 0.0f;
    entity.active           = NK_TRUE;

    change_entity_state(entity, desc.default_state);

    // If a custom Z-depth has been supplied use it.
    if(z != 0.0f)
    {
        entity.z_depth = z;
    }

    // Plants can be flipped for more visual variance.
    if(entity.type == EntityType_Plant && rng_s32(0,100) < 50)
    {
        entity.flip_x = -1.0f;
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

GLOBAL void change_entity_state(nkU64 index, EntityState state)
{
    NK_ASSERT(index < g_entity_manager.entities.length);
    Entity* e = get_entity(index);
    if(e) change_entity_state(*e, state);
}

GLOBAL void change_entity_state(Entity& e, EntityState state)
{
    if(e.state == state) return;

    EntityState old_state = e.state;
    e.state = state;

    nkString anim_name = get_current_entity_anim_name(e);

    if(!has_animation(&e.anim_state, anim_name.cstr) && state != EntityState_Dead)
    {
        e.state = old_state; // If we don't have an animation for that state and it's not the death state just set us back.
    }
    else
    {
        set_animation(&e.anim_state, anim_name.cstr);
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

GLOBAL nkU64 check_entity_bounds_vs_radius_collision(const Entity& e, EntityType collision_mask)
{
    if(e.active && e.state != EntityState_Dead)
    {
        nkU64 index = 0;

        for(auto& o: g_entity_manager.entities)
        {
            if(o.id != EntityType_None && o.active && o.state != EntityState_Dead && NK_CHECK_FLAGS(collision_mask, o.type))
            {
                if(check_entity_bounds_vs_radius_collision(e, o))
                {
                    return index;
                }
            }

            ++index;
        }
    }

    return NK_U64_MAX;
}

GLOBAL nkBool check_entity_bounds_vs_radius_collision(const Entity& a, const Entity& b)
{
    if(!a.active || !b.active) return NK_FALSE;

    if(a.state == EntityState_Dead) return NK_FALSE;
    if(b.state == EntityState_Dead) return NK_FALSE;

    nkF32 ax = a.position.x - (a.bounds.x * 0.5f);
    nkF32 ay = a.position.y - (a.bounds.y * 0.5f);
    nkF32 aw = a.bounds.x;
    nkF32 ah = a.bounds.y;

    nkF32 bx = b.position.x;
    nkF32 by = b.position.y;
    nkF32 br = b.radius;

    return rect_vs_circle({ ax,ay,aw,ah }, bx,by,br);
}

GLOBAL nkU64 check_entity_collision(const Entity& e, EntityType collision_mask)
{
    if(e.active && e.state != EntityState_Dead)
    {
        nkU64 index = 0;

        for(auto& o: g_entity_manager.entities)
        {
            if(o.id != EntityType_None && o.active && o.state != EntityState_Dead && NK_CHECK_FLAGS(collision_mask, o.type))
            {
                if(check_entity_collision(e, o))
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

    if(a.state == EntityState_Dead) return NK_FALSE;
    if(b.state == EntityState_Dead) return NK_FALSE;

    nkF32 ax = a.position.x;
    nkF32 ay = a.position.y;
    nkF32 ar = a.radius;

    nkF32 bx = b.position.x;
    nkF32 by = b.position.y;
    nkF32 br = b.radius;

    return circle_vs_circle(ax,ay,ar, bx,by,br);
}

GLOBAL nkBool check_entity_collision_3d(const Entity& a, const Entity& b)
{
    if(!a.active || !b.active) return NK_FALSE;

    if(a.state == EntityState_Dead) return NK_FALSE;
    if(b.state == EntityState_Dead) return NK_FALSE;

    nkF32 ax = a.position.x;
    nkF32 ay = a.position.y;
    nkF32 az = a.z_depth;
    nkF32 ar = a.radius;

    nkF32 bx = b.position.x;
    nkF32 by = b.position.y;
    nkF32 bz = b.z_depth;
    nkF32 br = b.radius;

    return nk_length(nkVec3{ ax-bx, ay-by, az-bz }) < (ar + br);
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

GLOBAL nkBool is_entity_dead(nkU64 index)
{
    Entity* e = get_entity(index);
    if(!e) return NK_FALSE;
    return (e->state == EntityState_Dead);
}

GLOBAL nkBool any_entities_of_type_alive(EntityType type)
{
    for(auto& e: g_entity_manager.entities)
    {
        if(e.active && e.state != EntityState_Dead && NK_CHECK_FLAGS(type, e.type))
            return NK_TRUE;
    }
    return NK_FALSE;
}

GLOBAL nkBool plant_is_fully_grown(Entity& e)
{
    if(e.type != EntityType_Plant) return NK_FALSE;

    const EntityDesc& desc = ENTITY_TABLE[e.id];
    nkS32 max_phases = 0;
    while(desc.phase_times[max_phases] > 0.0f)
        max_phases++;
    max_phases++;

    return (e.current_phase >= max_phases-1);
}

/*////////////////////////////////////////////////////////////////////////////*/
