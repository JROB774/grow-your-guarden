/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void game_start(void)
{
    // NOTE: DO NOT TOUCH THE ORDER WITHOUT CONSIDERING WHAT IT MIGHT BREAK!
    world_load("level00.png");
    entity_init();
    controller_init();
    decal_init();

    // Spawn the house.
    nkF32 hx = NK_CAST(nkF32, get_world_width() * TILE_WIDTH) * 0.5f;
    nkF32 hy = NK_CAST(nkF32, get_world_height() * TILE_HEIGHT) * 0.5f;
    entity_spawn(EntityID_House, hx,hy);

    set_app_state(AppState_Game);
}

GLOBAL void game_quit(void)
{
    decal_quit();
    entity_quit();
    world_free();
}

GLOBAL void game_tick(nkF32 dt)
{
    controller_tick(dt);
    world_tick(dt);
    entity_tick(dt);
    decal_tick(dt);

    // @Incomplete + @Temporary: Just testing decals...
    nkF32 x = 0.0f;
    nkF32 y = 0.0f;
    nkF32 w = get_world_width() * TILE_WIDTH;
    nkF32 h = get_world_height() * TILE_HEIGHT;

    if(rng_s32(0,100) < 10)
    {
        decal_spawn(x,y,w,h, 2,20, 1.0f,5.0f, "test_splat");
    }
}

GLOBAL void game_draw(void)
{
    set_controller_camera();
    world_draw_below();
    decal_draw();
    entity_draw();
    world_draw_above();
    controller_draw(); // This internally unsets the controller camera!
}

/*////////////////////////////////////////////////////////////////////////////*/
