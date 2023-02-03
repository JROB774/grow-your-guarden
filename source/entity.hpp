/*////////////////////////////////////////////////////////////////////////////*/

#define DEF_ETICK(name) INTERNAL void etick__##name(Entity& e, nkF32 dt)
#define     ETICK(name) etick__##name

typedef nkU32 EntityID;

// These are implemented as flags so that some functions can take in multiple types
// e.g. as a filter on the current entity set, etc. Entities themselves will only
// ever be assigned one of these types though and should not ever be multiple.
NK_ENUM(EntityType, nkU32)
{
    EntityType_None    = (   0),
    EntityType_Plant   = (1<<0),
    EntityType_Monster = (1<<1),
    EntityType_Base    = (1<<2),
    EntityType_Bullet  = (1<<3),
    EntityType_Object  = (1<<4),
    EntityType_All     = (  -1)
};

// Set entity state to trigger specific animations. The names of these animations
// are listed next to the states below. For plants they are also prefixed with
// "phaseX_" where X is the value stored in current_phase. This allows for plants
// to have multiple variants of animations depending on their current growth.
//
// States are driven by their animations so once an animation is complete the state
// will reset to whatever has been assigned as the default state in the entity's
// descriptor. If a state's animation has been set to lop then the state will only
// be able to change if it is explictially changed via the change state function.
NK_ENUM(EntityState, nkU32)
{
    EntityState_None,   // (NULL)
    EntityState_Idle,   // "idle"
    EntityState_Move,   // "move"
    EntityState_Hurt,   // "hurt"
    EntityState_Attack, // "attack"
    EntityState_Dead,   // "dead"
    EntityState_TOTAL
};

struct Entity
{
    EntityType    type;
    EntityID      id;
    EntityState   state;         // Do not set this manually! Go through change_entity_state.
    nkVec2        position;
    nkVec2        spawn;
    nkVec2        velocity;
    nkF32         health;
    nkF32         damage;
    nkF32         speed;
    nkF32         range;
    nkF32         radius;
    nkF32         z_depth;       // Controls how "high" the visual of the entity is compared to its shadow.
    nkF32         flip;          // Plants get this randomly set on spawn to add some visual variance.
    AnimState     anim_state;
    EntityType    collision_mask;
    nkVec2        draw_offset;
    nkVec2        bounds;
    nkF32         damage_timer;  // How long to flash red for when damaged.
    nkS32         current_phase; // Only used by plants for tracking growth stages.
    nkF32         phase_timer;   // Only used by plants for tracking growth stages.
    nkF32         timer0;
    nkF32         timer1;
    nkF32         timer2;
    nkF32         timer3;
    nkBool        active;
};

typedef void(*EntityTick)(Entity&, nkF32);

struct EntityDesc
{
    const nkChar* texture;
    const nkChar* animation;
    EntityType    type;
    EntityState   default_state;
    EntityTick    tick;
    nkF32         health;
    nkF32         damage;
    nkF32         speed;
    nkF32         range;
    nkF32         radius;
    nkF32         z_depth;
    EntityType    collision_mask;
    nkVec2        draw_offset;
    nkVec2        bounds;
    nkF32         phase_times[8]; // Only used by plants, can be ignored by other entity types.
    const nkChar* death_sounds[8];
    const nkChar* death_decal;
    nkS32         death_decal_min;
    nkS32         death_decal_max;
};

GLOBAL void  entity_init  (void);
GLOBAL void  entity_quit  (void);
GLOBAL void  entity_tick  (nkF32 dt);
GLOBAL void  entity_draw  (void);
GLOBAL void  entity_damage(nkU64 index, nkF32 damage);
GLOBAL void  entity_kill  (nkU64 index);
GLOBAL nkU64 entity_spawn (EntityID id, nkF32 x, nkF32 y); // Returns the index in the world entity array where the entity
                                                           // was inserted. Indicies should remain valid until an entity
                                                           // is marked dead, in which case other entities can claim it.

GLOBAL void change_entity_state(nkU64 index, EntityState state);
GLOBAL void change_entity_state(Entity& e, EntityState state);

// Return the index of the entity collided with or NK_U64_MAX if there was no collision.
GLOBAL nkU64  check_entity_bounds   (nkF32 x, nkF32 y, nkF32 w, nkF32 h, EntityType collision_mask = EntityType_All);
GLOBAL nkU64  check_entity_collision(const Entity& e, EntityType collision_mask = EntityType_All);
GLOBAL nkBool check_entity_collision(const Entity& a, const Entity& b);

GLOBAL nkU64   get_entity_count              (void);
GLOBAL Entity* get_entity                    (nkU64 index);
GLOBAL Entity* get_first_entity_with_id      (EntityID id);
GLOBAL nkU64   get_first_entity_index_with_id(EntityID id);

/*////////////////////////////////////////////////////////////////////////////*/
