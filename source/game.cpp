/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void game_tick(nkF32 dt)
{
    controller_tick(dt);
    world_tick(dt);
}

GLOBAL void game_draw(void)
{
    set_controller_camera();
    world_draw();
    controller_draw(); // This internally unsets the controller camera!
}

/*////////////////////////////////////////////////////////////////////////////*/
