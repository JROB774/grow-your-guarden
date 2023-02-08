/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL void spawn_bullet_at_target(EntityID id, nkF32 x, nkF32 y, const Entity& target)
{
    nkU64 index = entity_spawn(id, x,y);
    Entity* b = get_entity(index);
    nkVec2 dir = nk_normalize(target.position - b->position);
    b->velocity = dir * NK_CAST(nkF32, b->speed);
}

/*////////////////////////////////////////////////////////////////////////////*/

//
// Plants
//

INTERNAL nkBool plant_is_fully_grown(Entity& e)
{
    if(e.type != EntityType_Plant) return NK_FALSE;

    const EntityDesc& desc = ENTITY_TABLE[e.id];
    nkS32 max_phases = 0;
    while(desc.phase_times[max_phases] > 0.0f)
        max_phases++;
    max_phases++;

    return (e.current_phase >= max_phases-1);
}

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
            if(m.type == EntityType_Monster && m.active)
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
            if(m && m->id != EntityType_None && m->active && m->state != EntityState_Dead && NK_CHECK_FLAGS(m->type, EntityType_Monster))
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

/*////////////////////////////////////////////////////////////////////////////*/

//
// Monsters
//

INTERNAL nkBool do_monster_bite(Entity& e, nkF32& attack_cooldown, const nkF32 ATTACK_COOLDOWN)
{
    nkU64 hit_index = check_entity_collision(e, EntityType_Plant|EntityType_Base);
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
    nkBool eating = do_monster_bite(e, bite_cooldown, BITE_COOLDOWN);
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
                if(p.type == EntityType_Plant && p.active)
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
                particle_spawn("tar_explosion", e.position.x + e.draw_offset.x, e.position.y + e.draw_offset.y);
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

/*////////////////////////////////////////////////////////////////////////////*/

//
// Items
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
            particle_spawn("sparkle", x,y,w,h, 3, 5);
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
                case EntityID_CoinCopper: particle_spawn("puff_small", e.position.x,e.position.y); break;
                case EntityID_CoinSilver: particle_spawn("puff_medium", e.position.x,e.position.y); break;
                case EntityID_CoinGold:   particle_spawn("puff_large", e.position.x,e.position.y); break;
            }
        }
    }
}

/*////////////////////////////////////////////////////////////////////////////*/

