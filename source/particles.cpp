/*////////////////////////////////////////////////////////////////////////////*/

#define DEF_PTICK( name) INTERNAL void ptick__##name(Particle& p, nkF32 dt)
#define DEF_PSPAWN(name) INTERNAL void pspawn__##name(Particle& p)

// @Incomplete: Particle ticks and spawns go here...

/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkU32 MAX_PARTICLES = 8192; // Hard limit on particles, if we hit this we don't spawn more until new slots are available...

struct ParticleManager
{
    nkArray<Particle>                 particles;
    nkHashSet<nkU64>                  free_slots; // Track particle slots that were in use but no longer are as they can be filled without growing the array.
    nkHashMap<nkString,ParticleTick>  ticks;
    nkHashMap<nkString,ParticleSpawn> spawns;
    Texture                           texture;
    AnimGroup*                        animations;
};

INTERNAL ParticleManager g_particle_manager;

GLOBAL void particle_init(void)
{
    g_particle_manager.texture = asset_manager_load<Texture>("particles.png");
    g_particle_manager.animations = asset_manager_load<AnimGroup*>("particles.anm");

    nk_array_reserve(&g_particle_manager.particles, 256);
    nk_hashset_clear(&g_particle_manager.free_slots);
    nk_hashmap_clear(&g_particle_manager.ticks);
    nk_hashmap_clear(&g_particle_manager.spawns);

    // @Incomplete: Fill out the tick and spawn map here...
}

GLOBAL void particle_quit(void)
{
    nk_hashmap_free(&g_particle_manager.spawns);
    nk_hashmap_free(&g_particle_manager.ticks);
    nk_hashset_free(&g_particle_manager.free_slots);
    nk_array_free(&g_particle_manager.particles);
}

GLOBAL void particle_tick(nkF32 dt)
{
    nkU64 index = 0;
    for(auto& particle: g_particle_manager.particles)
    {
        if(particle.active)
        {
            // Do the custom particle behavior if it has it, otherwise do the default behavior.
            if(particle.tick)
                particle.tick(particle, dt);
            else
            {
                if(is_animation_done(&particle.anim_state))
                    particle.active = NK_FALSE;
            }

            // Update animation logic.
            update_animation(&particle.anim_state, dt);

            // If we've been killed then mark the slot free.
            if(!particle.active && !nk_hashset_contains(&g_particle_manager.free_slots, index))
            {
                nk_hashset_insert(&g_particle_manager.free_slots, index);
            }
        }
        ++index;
    }
}

GLOBAL void particle_draw(void)
{
    imm_begin_texture_batch(g_particle_manager.texture);
    for(auto& particle: g_particle_manager.particles)
    {
        if(particle.active)
        {
            AnimFrame frame = get_current_animation_frame(&particle.anim_state);
            ImmClip clip = { frame.x,frame.y,frame.w,frame.h };

            nkF32 x = particle.position.x;
            nkF32 y = particle.position.y;
            nkF32 s = particle.scale;
            nkF32 r = particle.rotation;

            imm_texture_batched_ex(x,y-particle.z_depth, s,s, r, NULL, &clip, particle.color);
        }
    }
    imm_end_texture_batch();
}

GLOBAL void particle_spawn(const nkChar* name, nkF32 x, nkF32 y)
{
    nkString particle_name = name;

    if(!nk_hashmap_contains(&g_particle_manager.animations->anims, particle_name)) return;

    Particle particle   = NK_ZERO_MEM;
    particle.position   = { x,y };
    particle.velocity   = NK_ZERO_MEM;
    particle.color      = NK_V4_WHITE;
    particle.rotation   = 0.0f;
    particle.scale      = 1.0f;
    particle.z_depth    = 0.0f;
    particle.anim_state = create_animation_state(g_particle_manager.animations);
    particle.tick       = nk_hashmap_getref(&g_particle_manager.ticks, particle_name);
    particle.active     = NK_TRUE;

    set_animation(&particle.anim_state, particle_name.cstr);

    // If we have custom spawn logic then do it now.

    if(nk_hashmap_contains(&g_particle_manager.spawns, particle_name))
    {
        ParticleSpawn spawn = nk_hashmap_getref(&g_particle_manager.spawns, particle_name);
        spawn(particle);
    }

    // If there are free slots available then use them, otherwise append on the end (potentially grow memory).
    if(!nk_hashset_empty(&g_particle_manager.free_slots))
    {
        nkU64 index = g_particle_manager.free_slots.begin()->key;
        g_particle_manager.particles[index] = particle;
        nk_hashset_remove(&g_particle_manager.free_slots, index);
    }
    else
    {
        if(g_particle_manager.particles.length < MAX_PARTICLES)
        {
            nk_array_append(&g_particle_manager.particles, particle);
        }
    }
}

GLOBAL void particle_spawn(const nkChar* name, nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkS32 min, nkS32 max)
{
    nkS32 count = rng_s32(min,max);
    for(nkS32 i=0; i<count; ++i)
    {
        nkF32 px = rng_f32(x, x+w);
        nkF32 py = rng_f32(y, y+h);
        particle_spawn(name, px,py);
    }
}

/*////////////////////////////////////////////////////////////////////////////*/
