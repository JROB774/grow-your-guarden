/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void wave_manager_tick(nkF32 dt)
{
    // @Temporary: Spawn random monsters!
    if(rng_s32(0,100) < 2)
    {
        nkS32 tw = NK_CAST(nkS32, TILE_WIDTH);
        nkS32 th = NK_CAST(nkS32, TILE_HEIGHT);

        nkS32 pos = rng_s32(0,100);
        if(pos < 25)
        {
            entity_spawn(EntityID_Walker, 0, NK_CAST(nkF32, rng_s32(0, get_world_height() * th)));
        }
        else if(pos < 50)
        {
            entity_spawn(EntityID_Walker, NK_CAST(nkF32, get_world_width() * tw), NK_CAST(nkF32, rng_s32(0, get_world_height() * th)));
        }
        else if(pos < 75)
        {
            entity_spawn(EntityID_Walker, NK_CAST(nkF32, rng_s32(0, get_world_width() * tw)), 0);
        }
        else
        {
            entity_spawn(EntityID_Walker, NK_CAST(nkF32, rng_s32(0, get_world_width() * tw)), NK_CAST(nkF32, get_world_height() * th));
        }
    }
}

GLOBAL void wave_manager_draw(void)
{
    // @Incomplete: ...
}

GLOBAL void wave_manager_reset(void)
{
    // @Incomplete: ...
}

/*////////////////////////////////////////////////////////////////////////////*/
