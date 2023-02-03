/*////////////////////////////////////////////////////////////////////////////*/

struct Particle;

typedef void(*ParticleTick )(Particle&, nkF32);
typedef void(*ParticleSpawn)(Particle&);

struct Particle
{
    nkVec2       position;
    nkVec2       velocity;
    nkVec4       color;
    nkF32        thrust; // Applies to z-depth.
    nkF32        weight;
    nkF32        rotation;
    nkF32        scale;
    nkF32        z_depth;
    AnimState    anim_state;
    ParticleTick tick;
    nkBool       active;
};

GLOBAL void particle_init(void);
GLOBAL void particle_quit(void);
GLOBAL void particle_tick(nkF32 dt);
GLOBAL void particle_draw(void);

// Use animations within the "particles.anm" group to define the different particle clips/animations.
// The name passed into these functions not only picks what animation to use but also the behavior to
// give to the particles. Each name will map to a corresponding function in a large map. If there is
// no matching function for the name then the default particle behavior will be assigned.
GLOBAL void particle_spawn(const nkChar* name, nkF32 x, nkF32 y);
GLOBAL void particle_spawn(const nkChar* name, nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkS32 min, nkS32 max); // Spawn multiple in a box region.

/*////////////////////////////////////////////////////////////////////////////*/
