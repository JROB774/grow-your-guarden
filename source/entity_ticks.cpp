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

INTERNAL void spawn_bullet_at_target(EntityID id, nkF32 x, nkF32 y, const Entity& target)
{
    nkU64 index = entity_spawn(id, x,y);
    Entity* b = get_entity(index);
    nkVec2 dir = nk_normalize(target.position - b->position);
    b->velocity = dir * NK_CAST(nkF32, b->speed);
}

DEF_ETICK(daisy)
{
    // @Incomplete: Don't want to do this for every plant...
    if(e.current_phase == 0) set_animation(&e.anim_state, "phase0_idle");
    if(e.current_phase == 1) set_animation(&e.anim_state, "phase1_idle");
    if(e.current_phase == 2) set_animation(&e.anim_state, "phase2_idle");

    const nkF32 COOLDOWN = 3.0f;

    nkF32& shot_cooldown = e.timer0;

    // If we are fully grown then try and shoot any enemies that are close enough.
    if(plant_is_fully_grown(e) && shot_cooldown <= 0.0f)
    {
        for(auto& m: g_entity_manager.entities)
        {
            if(m.type == EntityType_Monster && m.active)
            {
                nkF32 distance = distance_between_points(e.position, m.position);
                if(distance <= e.range)
                {
                    shot_cooldown = COOLDOWN;
                    spawn_bullet_at_target(EntityID_Pollen, e.position.x,e.position.y, m);
                    break; // Exit early we don't need to keep looping.
                }
            }
        }
    }

    if(shot_cooldown > 0.0f)
    {
        shot_cooldown -= dt;
    }
}

DEF_ETICK(bramble)
{
    // @Incomplete: Don't want to do this for every plant...
    if(e.current_phase == 0) set_animation(&e.anim_state, "phase0_idle");
    if(e.current_phase == 1) set_animation(&e.anim_state, "phase1_idle");
    if(e.current_phase == 2) set_animation(&e.anim_state, "phase2_idle");
    if(e.current_phase == 3) set_animation(&e.anim_state, "phase3_idle");

    const nkF32 COOLDOWN = 3.0f;

    nkF32& attack_cooldown = e.timer0;

    if(attack_cooldown <= 0.0f)
    {
        nkU64 hit_entity = check_entity_collision(e, EntityType_Monster);
        if(hit_entity != NK_U64_MAX)
        {
            attack_cooldown = COOLDOWN;

            // The amount of damage dealth depends on the growth stage of the bramble.
            switch(e.current_phase)
            {
                case 0: entity_damage(hit_entity, 1); break;
                case 1: entity_damage(hit_entity, 1); break;
                case 2: entity_damage(hit_entity, 2); break;
                case 3: entity_damage(hit_entity, 3); break;
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

DEF_ETICK(walker)
{
    const nkF32 COOLDOWN = 1.8f;

    nkF32& attack_cooldown = e.timer0;

    e.velocity = NK_V2_ZERO;

    // If we collide with a plant or the base then stop to eat it, otherwise continue walking to the house.
    nkU64 hit_index = check_entity_collision(e, EntityType_Plant|EntityType_Base);
    if(hit_index != NK_U64_MAX)
    {
        if(attack_cooldown <= 0.0f)
        {
            attack_cooldown = COOLDOWN;
            entity_damage(hit_index, e.damage);
        }
    }
    else
    {
        // @Incomplete + @Speed: Don't hunt for the house each time, just store the target...
        Entity* target = get_first_entity_with_id(EntityID_House);
        if(target)
        {
            // Walk towards the house.
            nkVec2 dir = nk_normalize(target->position - e.position);

            e.velocity = dir * NK_CAST(nkF32, e.speed);
            e.flip = (dir.x < 0.0f) ? -1.0f : 1.0f; // Face the walking direction.
        }
    }

    if(attack_cooldown > 0.0f)
    {
        attack_cooldown -= dt;
    }
}

/*////////////////////////////////////////////////////////////////////////////*/
