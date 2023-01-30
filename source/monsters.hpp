/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(MonsterID, nkU32)
{
    MonsterID_None,
    MonsterID_Walker,
    MonsterID_TOTAL
};

struct Monster
{
    MonsterID id;
    nkVec2    position;
    fRect     bounds;
    nkS32     health;
    nkF32     anim_timer;
    nkS32     anim_frame;
};

typedef void(*MonsterTick)(Monster*, nkF32);

struct MonsterDesc
{
    MonsterTick   tick;
    const nkChar* texture;
    nkS32         health;
    fRect         bounds;
};

GLOBAL void    monster_tick          (nkF32 dt);
GLOBAL void    monster_draw          (void);
GLOBAL void    monster_spawn         (MonsterID id, nkF32 x, nkF32 y);
GLOBAL Texture get_monster_id_texture(MonsterID id);
GLOBAL ImmClip get_monster_clip      (Monster* monster);

/*////////////////////////////////////////////////////////////////////////////*/
