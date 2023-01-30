/*////////////////////////////////////////////////////////////////////////////*/

typedef nkU32 EntityID;

NK_ENUM(EntityType, nkU32)
{
    EntityType_None    = (   0),
    EntityType_Plant   = (1<<0),
    EntityType_Monster = (1<<1),
    EntityType_Decor   = (1<<2),
    EntityType_Bullet  = (1<<3),
    EntityType_All     = (  -1)
};

struct Entity
{
    EntityType type;
    EntityID   id;
    nkVec2     position;
    nkVec2     velocity;
    nkS32      health;
    fRect      bounds;
    AnimState  anim_state;
    nkBool     active;
};

typedef void(*EntityTick)(Entity&, nkF32);

struct EntityDesc
{
    const nkChar* texture_file;
    const nkChar* anim_file;
    const nkChar* start_anim;
    EntityTick    tick;
    EntityType    type;
    nkS32         health;
    fRect         bounds;
};

GLOBAL void entity_init (void);
GLOBAL void entity_quit (void);
GLOBAL void entity_tick (nkF32 dt);
GLOBAL void entity_draw (void);
GLOBAL void entity_spawn(EntityID id, nkF32 x, nkF32 y);

/*////////////////////////////////////////////////////////////////////////////*/
