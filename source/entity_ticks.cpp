/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL nkBool plant_is_fully_grown(Entity& e)
{
    if(e.type != EntityType_Plant) return NK_FALSE;

    const EntityDesc& desc = ENTITY_TABLE[e.id];
    nkS32 max_phases = 0;
    while(desc.phase_times[max_phases] > 0.0f)
        max_phases++;
    max_phases++;

    return (e.current_phase >= max_phases);
}

DEF_ETICK(daisy)
{
    const nkF32 COOLDOWN = 1.5f;

    nkF32& shot_cooldown = e.timer0;

    // If we are fully grown then try and shoot any enemies that are close enough.
    if(plant_is_fully_grown(e) && shot_cooldown <= 0.0f)
    {
        for(auto& m: g_world.entities)
        {
            if(m.type == EntityType_Monster)
            {
                nkF32 distance = distance_between_points(e.position, m.position);
                if(distance <= e.range)
                {
                    // Spawn a bullet and make it target the monster.
                    nkU64 index = entity_spawn(EntityID_Pollen, e.position.x,e.position.y);
                    Entity& b = g_world.entities[index];
                    nkVec2 dir = nk_normalize(m.position - e.position);
                    b.velocity = dir * NK_CAST(nkF32, b.speed);

                    shot_cooldown = COOLDOWN;

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

/*////////////////////////////////////////////////////////////////////////////*/
