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
    Entity& b = g_world.entities[index];
    nkVec2 dir = nk_normalize(target.position - b.position);
    b.velocity = dir * NK_CAST(nkF32, b.speed);
}

DEF_ETICK(daisy)
{
    // @Incomplete: Don't want to do this for every plant...
    if(e.current_phase == 0) set_animation(&e.anim_state, "phase0_idle");
    if(e.current_phase == 1) set_animation(&e.anim_state, "phase1_idle");
    if(e.current_phase == 2) set_animation(&e.anim_state, "phase2_idle");

    const nkF32 COOLDOWN = 1.5f;

    nkF32& shot_cooldown = e.timer0;

    // If we are fully grown then try and shoot any enemies that are close enough.
    if(plant_is_fully_grown(e) && shot_cooldown <= 0.0f)
    {
        for(auto& m: g_world.entities)
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
}

/*////////////////////////////////////////////////////////////////////////////*/

//
// Monsters
//

DEF_ETICK(walker)
{
    // @Incomplete + @Speed: Don't hunt for the house each time, just store the target...
    nkU64 target_index = get_first_entity_with_id(EntityID_House);
    if(target_index != NK_U64_MAX)
    {
        // Walk towards the house.
        Entity& target = g_world.entities[target_index];
        nkVec2 dir = nk_normalize(target.position - e.position);
        e.velocity = dir * NK_CAST(nkF32, e.speed);
    }
}

/*////////////////////////////////////////////////////////////////////////////*/
