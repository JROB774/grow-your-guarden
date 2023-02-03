/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkF32 DECAL_FADE_SPEED = 1.0f;

INTERNAL constexpr nkU32 MAX_DECALS = 8192; // Hard limit on decals, if we hit this we don't spawn more until new slots are available...

struct Decal
{
    nkVec2  position;
    ImmClip clip;
    nkF32   lifetime;
    nkF32   scale;
    nkF32   angle;
    nkF32   alpha;
};

struct DecalManager
{
    nkArray<Decal>   decals;
    nkHashSet<nkU64> free_slots; // Track decal slots that were in use but no longer are as they can be filled without growing the array.
    Texture          texture;
    AnimGroup*       animations;
};

INTERNAL DecalManager g_decal_manager;

GLOBAL void decal_init(void)
{
    g_decal_manager.texture = asset_manager_load<Texture>("decals.png");
    g_decal_manager.animations = asset_manager_load<AnimGroup*>("decals.anm");

    nk_array_reserve(&g_decal_manager.decals, 256);
    nk_hashset_clear(&g_decal_manager.free_slots);
}

GLOBAL void decal_quit(void)
{
    nk_hashset_free(&g_decal_manager.free_slots);
    nk_array_free(&g_decal_manager.decals);
}

GLOBAL void decal_tick(nkF32 dt)
{
    nkU64 index = 0;
    for(auto& decal: g_decal_manager.decals)
    {
        if(decal.alpha > 0.0f)
        {
            decal.lifetime -= dt;
            if(decal.lifetime <= 0.0f)
            {
                // Fade the decal out.
                decal.alpha -= DECAL_FADE_SPEED * dt;

                // Once fully faded mark the slot as available for overwriting.
                if(decal.alpha <= 0.0f)
                {
                    if(!nk_hashset_contains(&g_decal_manager.free_slots, index))
                    {
                        nk_hashset_insert(&g_decal_manager.free_slots, index);
                    }
                }
            }
        }
        ++index;
    }
}

GLOBAL void decal_draw(void)
{
    imm_begin_texture_batch(g_decal_manager.texture);
    for(auto& decal: g_decal_manager.decals)
    {
        if(decal.alpha > 0.0f)
        {
            nkVec4 color = { 1.0f,1.0f,1.0f,decal.alpha };
            nkF32 x = decal.position.x;
            nkF32 y = decal.position.y;
            nkF32 s = decal.scale;
            nkF32 a = decal.angle;
            imm_texture_batched_ex(x,y, s,s, a, NULL, &decal.clip, color);
        }
    }
    imm_end_texture_batch();
}

GLOBAL void decal_spawn(nkF32 x, nkF32 y, nkF32 life_min, nkF32 life_max, const nkChar* anim_name)
{
    nkString anim_name_str = anim_name;

    if(!nk_hashmap_contains(&g_decal_manager.animations->anims, anim_name_str)) return;

    // Pick a random graphic for the decal to use.
    const Anim& anim = nk_hashmap_getref(&g_decal_manager.animations->anims, anim_name_str);
    AnimFrame frame = anim.frames[rng_s32(0,NK_CAST(nkS32, anim.frames.length)-1)];

    Decal decal = NK_ZERO_MEM;
    decal.position = { x,y };
    decal.clip     = { frame.x,frame.y,frame.w,frame.h };
    decal.lifetime = rng_f32(life_min, life_max);
    decal.scale    = rng_f32(0.5f, 1.0f); // @Incomplete: Currently all decals get a random scale?
    decal.angle    = 0.0f; // rng_f32(0.0f, NK_TAU_F32); // @Incomplete: Currently all decals get a random rotation?
    decal.alpha    = 1.0f; // rng_f32(0.4f, 0.9f); // @Incomplete: Currently all decals get a random alpha?

    // If there are free slots available then use them, otherwise append on the end (potentially grow memory).
    if(!nk_hashset_empty(&g_decal_manager.free_slots))
    {
        nkU64 index = g_decal_manager.free_slots.begin()->key;
        g_decal_manager.decals[index] = decal;
        nk_hashset_remove(&g_decal_manager.free_slots, index);
    }
    else
    {
        if(g_decal_manager.decals.length < MAX_DECALS)
        {
            nk_array_append(&g_decal_manager.decals, decal);
        }
    }
}

GLOBAL void decal_spawn(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkS32 min, nkS32 max, nkF32 life_min, nkF32 life_max, const nkChar* anim_name)
{
    nkS32 count = rng_s32(min,max);
    for(nkS32 i=0; i<count; ++i)
    {
        nkF32 dx = rng_f32(x, x+w);
        nkF32 dy = rng_f32(y, y+h);
        decal_spawn(dx,dy, life_min, life_max, anim_name);
    }
}

/*////////////////////////////////////////////////////////////////////////////*/
