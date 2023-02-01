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

struct Entity
{
    EntityType    type;
    EntityID      id;
    nkVec2        position;
    nkVec2        spawn;
    nkVec2        velocity;
    nkF32         health;
    nkF32         damage;
    nkF32         speed;
    nkF32         range;
    nkVec2        bounds;
    AnimState     anim_state;
    EntityType    collision_mask;
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
    const nkChar* texture_file;
    const nkChar* anim_file;
    const nkChar* start_anim;
    EntityTick    tick;
    EntityType    type;
    nkF32         health;
    nkF32         damage;
    nkF32         speed;
    nkF32         range;
    EntityType    collision_mask;
    nkVec2        bounds;
    nkF32         phase_times[8]; // Only used by plants, can be ignored by other entity types.
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

// Return the index of the entity collided with or NK_U64_MAX if there was no collision.
GLOBAL nkU64 check_entity_collision(nkF32 x, nkF32 y, nkF32 w, nkF32 h, EntityType collision_mask = EntityType_All);
GLOBAL nkU64 check_entity_collision(const Entity& e,                    EntityType collision_mask = EntityType_All);

GLOBAL nkU64 get_first_entity_with_id(EntityID id);

/*////////////////////////////////////////////////////////////////////////////*/
