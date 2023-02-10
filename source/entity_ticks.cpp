/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL Entity* spawn_bullet_at_target(EntityID id, nkF32 x, nkF32 y, const Entity& target)
{
    nkU64 index = entity_spawn(id, x,y);
    Entity* b = get_entity(index);
    nkVec2 dir = nk_normalize(target.position - b->position);
    b->velocity = dir * NK_CAST(nkF32, b->speed);
    return b;
}

/*////////////////////////////////////////////////////////////////////////////*/

//
// Plants
//

DEF_ETICK(daisy)
{
    const nkF32 ATTACK_COOLDOWN = 2.75f;

    nkF32& attack_cooldown = e.timer0;

    // If we are fully grown then try and shoot any enemies that are close enough (pick the closest one).
    if(plant_is_fully_grown(e) && attack_cooldown <= 0.0f)
    {
        nkF32 shortest_distance = FLT_MAX;
        nkF32 distance = 0.0f;

        Entity* target = NULL;

        for(auto& m: g_entity_manager.entities)
        {
            if(m.type == EntityType_Monster && m.active && !NK_CHECK_FLAGS(m.flags, EntityFlag_Aerial))
            {
                distance = distance_between_points(e.position, m.position);
                if(distance <= e.range && distance < shortest_distance)
                {
                    shortest_distance = distance;
                    target = &m;
                }
            }
        }
        if(target)
        {
            change_entity_state(e, EntityState_Attack);
            attack_cooldown = ATTACK_COOLDOWN;
            spawn_bullet_at_target(EntityID_Pollen, e.position.x,e.position.y, *target);

            // Shoot faster if fertilized.
            if(e.fertilized_timer > 0.0f)
            {
                attack_cooldown *= 0.33f;
            }
        }
    }

    if(attack_cooldown > 0.0f)
    {
        attack_cooldown -= dt;
    }
}

DEF_ETICK(bramble)
{
    const nkF32 ATTACK_COOLDOWN = 2.0f;

    nkF32& attack_cooldown = e.timer0;

    if(attack_cooldown <= 0.0f)
    {
        // Brambles will hit all enemies standing on them.
        nkF32 multiplier = (e.fertilized_timer > 0.0f) ? 2.0f : 1.0f; // Deal more damage if fertilized.

        for(nkU64 i=0; i<get_entity_count(); ++i)
        {
            Entity* m = get_entity(i);
            if(m && m->id != EntityType_None && m->active && m->state != EntityState_Dead && NK_CHECK_FLAGS(m->type, EntityType_Monster) && !NK_CHECK_FLAGS(m->flags, EntityFlag_Aerial))
            {
                if(check_entity_bounds_vs_radius_collision(e, *m))
                {
                    attack_cooldown = ATTACK_COOLDOWN;

                    play_sound(get_random_splat_sound());

                    // The amount of damage dealth depends on the growth stage of the bramble.
                    switch(e.current_phase)
                    {
                        case 0: entity_damage(i, 0.25f * multiplier); break;
                        case 1: entity_damage(i, 0.50f * multiplier); break;
                        case 2: entity_damage(i, 1.00f * multiplier); break;
                        case 3: entity_damage(i, 2.00f * multiplier); break;
                    }
                }
            }
        }
    }

    if(attack_cooldown > 0.0f)
    {
        attack_cooldown -= dt;
    }
}

DEF_ETICK(hedge_wall)
{
    // This doesn't actually do anything, fertilized logic, etc. actually weirdly happens inside entity_damage.
    // It's quite hacky but it works better than attempting to handle the logic inside of here instead.
}

DEF_ETICK(bell_plant)
{
    const nkF32 ATTACK_COOLDOWN = 1.5f;

    nkF32& attack_cooldown = e.timer0;

    // If we are fully grown then try and shoot any enemies that are close enough (pick the closest one).
    if(plant_is_fully_grown(e) && attack_cooldown <= 0.0f)
    {
        nkF32 shortest_distance = FLT_MAX;
        nkF32 distance = 0.0f;

        Entity* target = NULL;

        nkU64 target_index = 0;
        nkU64 entity_index = 0;

        for(auto& m: g_entity_manager.entities)
        {
            if(m.type == EntityType_Monster && m.active && NK_CHECK_FLAGS(m.flags, EntityFlag_Aerial))
            {
                distance = distance_between_points(e.position, m.position);
                if(distance <= e.range && distance < shortest_distance)
                {
                    shortest_distance = distance;
                    target = &m;
                    target_index = entity_index;
                }
            }

            ++entity_index;
        }
        if(target)
        {
            change_entity_state(e, EntityState_Attack);
            Entity* b = spawn_bullet_at_target(EntityID_BellMissile, e.position.x,e.position.y, *target);
            if(b)
            {
                b->target = target_index;

                // Bullets are faster when fertilized.
                if(e.fertilized_timer > 0.0f)
                {
                    b->speed *= 2.0f;
                }
            }

            attack_cooldown = ATTACK_COOLDOWN;
            // We shoot faster if fertilized.
            if(e.fertilized_timer > 0.0f)
            {
                attack_cooldown *= 0.33f;
            }
        }
    }

    if(attack_cooldown > 0.0f)
    {
        attack_cooldown -= dt;
    }
}

DEF_ETICK(rocket_plant)
{
    // @Incomplete: Fertilized...

    // Once we are fully grown we check for any nearby targets. If we have one then we launch
    // our rocket (which is acutally ourselves). Afterwards, we set our growth phase back to
    // the start to grow again and repeat the process.
    if(!plant_is_fully_grown(e)) return;

    nkF32 shortest_distance = FLT_MAX;
    nkF32 distance = 0.0f;

    Entity* target = NULL;

    for(auto& m: g_entity_manager.entities)
    {
        if(m.type == EntityType_Monster && m.active && !NK_CHECK_FLAGS(m.flags, EntityFlag_Aerial))
        {
            distance = distance_between_points(e.position, m.position);
            if(distance <= e.range && distance < shortest_distance)
            {
                shortest_distance = distance;
                target = &m;
            }
        }
    }
    if(target)
    {
        play_sound(asset_manager_load<Sound>("rocket.wav"));

        spawn_bullet_at_target(EntityID_Rocket, e.position.x,e.position.y, *target);

        e.current_phase = 0;
        e.phase_timer = 0.0f;

        set_animation(&e.anim_state, get_current_entity_anim_name(e).cstr);
    }
}

/*////////////////////////////////////////////////////////////////////////////*/

//
// Monsters
//

INTERNAL nkBool do_monster_bite(Entity& e, nkF32& attack_cooldown, const nkF32 ATTACK_COOLDOWN, EntityType collision_mask = EntityType_Plant|EntityType_Base)
{
    nkU64 hit_index = check_entity_collision(e, collision_mask);
    if(hit_index != NK_U64_MAX)
    {
        Entity* target = get_entity(hit_index);
        if(target && !NK_CHECK_FLAGS(target->flags, EntityFlag_NotEdible))
        {
            if(attack_cooldown <= 0.0f)
            {
                attack_cooldown = ATTACK_COOLDOWN;
                play_sound(get_random_munch_sound());
                entity_damage(hit_index, e.damage);
                if(is_entity_dead(hit_index))
                {
                    play_sound(asset_manager_load<Sound>("gulp.wav"));
                }
            }

            return NK_TRUE;
        }
    }

    return NK_FALSE;
}

DEF_ETICK(walker)
{
    nkF32 BITE_COOLDOWN = 1.25f;
    nkF32 SHOT_COOLDOWN = 2.50f;

    // Barbarians attack twice as fast as the other walker types.
    if(e.id == EntityID_Barbarian)
    {
        BITE_COOLDOWN *= 0.5f;
    }

    nkF32& bite_cooldown = e.timer0;
    nkF32& shot_cooldown = e.timer1;

    e.velocity = NK_V2_ZERO;

    // If we collide with a plant or the base then stop to eat it, otherwise continue walking to the tree.
    nkBool eating = eating = do_monster_bite(e, bite_cooldown, BITE_COOLDOWN);
    if(!eating)
    {
        // Hunt for the home tree if we aren't already.
        if(e.state == EntityState_Move)
        {
            if(e.target == NO_TARGET)
            {
                e.target = get_first_entity_index_with_id(EntityID_HomeTree);
            }
            if(e.target != NO_TARGET)
            {
                // Walk towards the tree if we found it.
                Entity* target = get_entity(e.target);
                nkVec2 dir = nk_normalize(target->position - e.position);
                e.velocity = dir * NK_CAST(nkF32, e.speed);
                e.flip = (dir.x < 0.0f) ? -1.0f : 1.0f; // Face the walking direction.
            }
        }

        // Soldiers can also fire bullets at plants.
        if(e.id == EntityID_Soldier && shot_cooldown <= 0.0f)
        {
            nkF32 shortest_distance = FLT_MAX;
            nkF32 distance = 0.0f;

            Entity* target = NULL;

            for(auto& p: g_entity_manager.entities)
            {
                if((p.type == EntityType_Plant || p.type == EntityType_Base) && p.active)
                {
                    distance = distance_between_points(e.position, p.position);
                    if(distance <= e.range && distance < shortest_distance)
                    {
                        shortest_distance = distance;
                        target = &p;
                    }
                }
            }
            if(target)
            {
                change_entity_state(e, EntityState_Attack);
                shot_cooldown = SHOT_COOLDOWN;
                spawn_bullet_at_target(EntityID_Tarball, e.position.x,e.position.y, *target);
                e.velocity = NK_V2_ZERO; // Stop moving whilst shooting.
                play_sound(asset_manager_load<Sound>("cough.wav"));
                particle_spawn("tar_explosion", e.position.x + e.draw_offset.x, e.position.y + e.draw_offset.y, 0.0f);
            }
        }
    }

    // Cooldown our attacks.
    if(bite_cooldown > 0.0f)
    {
        bite_cooldown -= dt;
    }
    if(shot_cooldown > 0.0f)
    {
        shot_cooldown -= dt;
    }

    // Randomly spawn splats under us.
    if(rng_s32(0,100) < 5)
    {
        nkF32 x = e.position.x - (e.radius * 0.75f);
        nkF32 y = e.position.y - (e.radius * 0.75f);
        nkF32 w = (e.radius * 0.75f) * 2.0f;
        nkF32 h = (e.radius * 0.75f) * 2.0f;

        decal_spawn("tar_splat_small", x,y,w,h, 1,5, 8.0f,10.0f);
    }
}

DEF_ETICK(dripper)
{
    const nkF32 MAX_HEIGHT =  2.00f * TILE_HEIGHT;
    const nkF32 MIN_BOUNCE = -0.25f * TILE_HEIGHT;
    const nkF32 MAX_BOUNCE =  0.25f * TILE_HEIGHT;

    const nkF32 HEIGHT_SPEED = 0.15f;
    const nkF32 BOUNCE_SPEED = 1.50f;

    const nkF32 BITE_COOLDOWN = 1.25f;

    nkF32& bite_cooldown = e.timer0;

    // Calculate our current Z value using these two timers.
    nkF32& height_timer = e.timer1;
    nkF32& bounce_timer = e.timer2;

    height_timer += (dt * HEIGHT_SPEED);
    bounce_timer += (dt * BOUNCE_SPEED);

    height_timer = nk_clamp(height_timer, 0.0f, 1.0f);

    e.z_depth = nk_lerp(0.0f, MAX_HEIGHT, height_timer) + (nk_sin_range(MIN_BOUNCE, MAX_BOUNCE, bounce_timer));

    // Reset velocity.
    e.velocity = NK_V2_ZERO;

    // If we collide with the base then stop to eat it, otherwise continue floating to the tree.
    nkBool eating = eating = do_monster_bite(e, bite_cooldown, BITE_COOLDOWN, EntityType_Base);
    if(!eating)
    {
        // Float towards the tree if we found it.
        if(e.target == NO_TARGET)
        {
            e.target = get_first_entity_index_with_id(EntityID_HomeTree);
        }
        if(e.target != NO_TARGET)
        {
            Entity* target = get_entity(e.target);
            nkVec2 dir = nk_normalize(target->position - e.position);
            e.velocity = dir * NK_CAST(nkF32, e.speed);
            e.flip = (dir.x < 0.0f) ? -1.0f : 1.0f; // Face the floating direction.
        }
    }

    // Cooldown our attack.
    if(bite_cooldown > 0.0f)
    {
        bite_cooldown -= dt;
    }

    // Randomly spawn drips.
    if(rng_s32(0,100) < 10)
    {
        nkF32 x = e.position.x - (e.radius * 0.5f);
        nkF32 y = e.position.y - (e.radius * 0.5f);
        nkF32 w = (e.radius * 0.5f) * 2.0f;
        nkF32 h = (e.radius * 0.5f) * 2.0f;

        particle_spawn("tar_drips", x,y,e.z_depth,w,h, 1,3);
    }

    // Randomly spawn splats under us.
    if(rng_s32(0,100) < 8)
    {
        nkF32 x = e.position.x - (e.radius * 0.75f);
        nkF32 y = e.position.y - (e.radius * 0.75f);
        nkF32 w = (e.radius * 0.75f) * 2.0f;
        nkF32 h = (e.radius * 0.75f) * 2.0f;

        decal_spawn("tar_splat_small", x,y,w,h, 1,5, 8.0f,10.0f);
    }
}

DEF_ETICK(goliath)
{
    nkF32 BITE_COOLDOWN = 2.0f;

    nkF32& bite_cooldown = e.timer0;

    // Reset velocity.
    e.velocity = NK_V2_ZERO;

    // If we collide with any plants we just instantly kill them, don't even bother biting.
    nkU64 hit_index = check_entity_collision(e, EntityType_Plant);
    if(hit_index != NK_U64_MAX)
    {
        Entity* target = get_entity(hit_index);
        if(target)
        {
            play_sound(asset_manager_load<Sound>("squish.wav"));
            entity_kill(hit_index);
        }
    }

    // If we collide with the tree then stop to eat it, otherwise continue walking to the tree.
    nkBool eating = eating = do_monster_bite(e, bite_cooldown, BITE_COOLDOWN, EntityType_Base);
    if(!eating)
    {
        // Float towards the tree if we found it.
        if(e.target == NO_TARGET)
        {
            e.target = get_first_entity_index_with_id(EntityID_HomeTree);
        }
        if(e.target != NO_TARGET)
        {
            Entity* target = get_entity(e.target);
            nkVec2 dir = nk_normalize(target->position - e.position);
            e.velocity = dir * NK_CAST(nkF32, e.speed);
            e.flip = (dir.x < 0.0f) ? -1.0f : 1.0f; // Face the walking direction.
        }
    }

    // Cooldown our attack.
    if(bite_cooldown > 0.0f)
    {
        bite_cooldown -= dt;
    }

    // Randomly spawn splats under us.
    if(rng_s32(0,100) < 10)
    {
        nkF32 x = e.position.x - (e.radius * 0.9f);
        nkF32 y = e.position.y - (e.radius * 0.9f);
        nkF32 w = (e.radius * 0.9f) * 2.0f;
        nkF32 h = (e.radius * 0.9f) * 2.0f;

        decal_spawn("tar_splat_large", x,y,w,h, 1,5, 14.0f,20.0f);
    }
}

/*////////////////////////////////////////////////////////////////////////////*/

//
// Bullets
//

DEF_ETICK(bell_missile)
{
    const nkF32 HEIGHT_ADVANCE_SPEED = 1.5f;
    const nkF32 HOMING_SPEED = 5.5f;

    const nkF32 MAX_HOMING_DISTANCE = 7.1f * TILE_WIDTH;

    Entity* target = get_entity(e.target);
    if(!target) return;

    // Move towards the target's z-depth and also home in.
    e.z_depth = nk_lerp(e.z_depth, target->z_depth, HEIGHT_ADVANCE_SPEED * dt);

    nkVec2 target_dir = nk_normalize(target->position - e.position);
    nkVec2 curr_dir = nk_normalize(e.velocity);

    nkVec2 new_velocity = nk_normalize(nk_lerp(curr_dir, target_dir, HOMING_SPEED * dt)) * e.speed;
    nkVec2 new_dir = nk_normalize(new_velocity);

    e.velocity = new_velocity;
    e.angle = atan2f(new_dir.y, new_dir.x);

    // If we are too far away from our target, stop tracking.
    nkF32 distance = distance_between_points(e.position, target->position);
    if(distance >= MAX_HOMING_DISTANCE)
    {
        e.target = NK_U64_MAX;
    }
}

DEF_ETICK(rocket)
{
    // @Incomplete: Spawn smoke particles behind us...

    // Face our direction.
    nkVec2 dir = nk_normalize(e.velocity);
    e.angle = atan2f(dir.y, dir.x);

    // Collide with enemies like normal. On collision we also spawn an explosion.
    // This explosion will calculate and handle splash damage to surrounding enemies.
    nkU64 hit_index = check_entity_collision(e, EntityType_Monster);
    if(hit_index != NK_U64_MAX)
    {
        Entity* target = get_entity(hit_index);
        if(target)
        {
            Entity* explosion = get_entity(entity_spawn(EntityID_Explosion, e.position.x,e.position.y));
            if(explosion)
                explosion->z_depth = e.z_depth;
            entity_damage(hit_index, e.damage);
            entity_kill(index);
        }
    }

    // If we go outside our range, we just de-spawn.
    nkF32 distance = distance_between_points(e.position, e.spawn);
    if(distance >= e.range)
    {
        entity_kill(index);
    }

    // Spawn smoke puffs behind us as we go.
    if(rng_s32(0, 100) < 50)
    {
        nkF32 x = e.position.x + rng_f32(-(e.radius * 0.5f), (e.radius * 0.5f));
        nkF32 y = e.position.y + rng_f32(-(e.radius * 0.5f), (e.radius * 0.5f));

        nkVec2 backwards = nk_rotate(nk_normalize(e.velocity), NK_PI_F32);

        x += (backwards.x * e.radius);
        y += (backwards.y * e.radius);

        particle_spawn("smoke_small", x,y, e.z_depth);
    }
}

/*////////////////////////////////////////////////////////////////////////////*/

//
// Other
//

DEF_ETICK(coin)
{
    const nkF32 WARNING_TIME = 25.0f;
    const nkF32 DANGER_TIME = 29.0f;
    const nkF32 DESPAWN_TIME = 30.0f;

    const nkF32 WEIGHT = 3200.0f;

    if(e.state == EntityState_Dead) return;

    // Fall down and once we hit the floor stop moving outwards.
    if(e.z_depth <= 0.0f)
    {
        e.velocity = NK_V2_ZERO;
        if(e.thrust != 0.0f)
        {
            play_sound(asset_manager_load<Sound>("coin_drop.wav"));
            e.thrust = 0.0f;
        }
    }
    else
    {
        e.thrust -= WEIGHT * dt;
    }

    // Check if we have been dragged over on and if so die and give the player money.
    if(!is_something_selected() && is_mouse_button_down(MouseButton_Left))
    {
        nkVec2 cursor = get_cursor_world_pos();
        if(point_vs_circle(cursor, e.position.x, e.position.y - e.z_depth, e.radius))
        {
            switch(e.id)
            {
                case EntityID_CoinCopper: add_money(10); break;
                case EntityID_CoinSilver: add_money(15); break;
                case EntityID_CoinGold:   add_money(25); break;
            }

            // The sound and particle are not implemented in the entity desc because we don't want them to happen on de-spawn.
            nkF32 x = e.position.x - e.radius;
            nkF32 y = e.position.y - e.radius;
            nkF32 w = e.radius * 2.0f;
            nkF32 h = e.radius * 2.0f;

            play_sound(asset_manager_load<Sound>("coin_collect.wav"));
            particle_spawn("sparkle", x,y,0.0f, w,h, 3, 5);
            entity_kill(index);
        }
    }

    // De-spawn after a while
    e.timer0 += dt;
    if(e.timer0 >= WARNING_TIME)
    {
        if(e.timer0 >= DANGER_TIME)
        {
            if((get_elapsed_ticks() % 2) == 0)
            {
                NK_TOGGLE_FLAGS(e.flags, EntityFlag_Hidden);
            }
        }
        else
        {
            if((get_elapsed_ticks() % 4) == 0)
            {
                NK_TOGGLE_FLAGS(e.flags, EntityFlag_Hidden);
            }
        }

        if(e.timer0 >= DESPAWN_TIME)
        {
            entity_kill(index);

            // Spawn a little animation on de-spawn.
            switch(e.id)
            {
                case EntityID_CoinCopper: particle_spawn("puff_small", e.position.x,e.position.y,0.0f); break;
                case EntityID_CoinSilver: particle_spawn("puff_medium", e.position.x,e.position.y,0.0f); break;
                case EntityID_CoinGold:   particle_spawn("puff_large", e.position.x,e.position.y,0.0f); break;
            }
        }
    }
}

DEF_ETICK(explosion)
{
    if(e.state == EntityState_Dead) return;

    play_sound(asset_manager_load<Sound>("explosion.wav"));

    // Loop through all monsters, determine if they are in range to receive blast damage.
    // Then calculate how much blast damage they should receive based on their distance.
    nkU64 sub_index = 0;
    for(auto& m: g_entity_manager.entities)
    {
        if(m.active && m.type == EntityType_Monster)
        {
            nkF32 distance = distance_between_points(e.position, m.position);
            if(distance <= e.radius)
            {
                nkF32 blast_damage = e.damage * (1.0f - (distance / e.radius));
                entity_damage(sub_index, blast_damage);
                printf("BLAST DAMAGE: %f\n", blast_damage);
            }
        }
        ++sub_index;
    }

    entity_kill(index); // When we are done we die instantly so this only happens on one tick.
}

/*////////////////////////////////////////////////////////////////////////////*/

