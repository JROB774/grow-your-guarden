/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void decal_init(void);
GLOBAL void decal_quit(void);
GLOBAL void decal_tick(nkF32 dt);
GLOBAL void decal_draw(void);

// Use animations within the "decals.anm" group to define the different decal clips. Whatever animation name
// you pass in, the decal system will randomly select a frame from within that animation and that will be drawn.
GLOBAL void decal_spawn(nkF32 x, nkF32 y,                                         nkF32 life_min, nkF32 life_max, const nkChar* anim_name);
GLOBAL void decal_spawn(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkS32 min, nkS32 max, nkF32 life_min, nkF32 life_max, const nkChar* anim_name); // Spawn multiple in a box region.

/*////////////////////////////////////////////////////////////////////////////*/
