/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL constexpr nkU32 NO_SELECTION = NK_U32_MAX;

struct PlantSpawn
{
    EntityID id;
    nkS32    cost;
};

GLOBAL void controller_init      (void);
GLOBAL void controller_tick      (nkF32 dt);
GLOBAL void controller_draw      (void);
GLOBAL void set_controller_camera(void);

/*////////////////////////////////////////////////////////////////////////////*/
