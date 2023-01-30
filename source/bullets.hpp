/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(BulletID, nkU32)
{
    BulletID_None,
    BulletID_Flower,
    BulletID_TOTAL
};

NK_ENUM(BulletTeam, nkU32)
{
    BulletTeam_None,
    BulletTeam_Plants,
    BulletTeam_Monsters,
    BulletTeam_TOTAL
};

struct Bullet
{
    BulletID   id;
    BulletTeam team;
    nkVec2     position;
    nkVec2     spawn;
    nkVec2     velocity;
    fRect      bounds;
    nkF32      anim_timer;
    nkS32      anim_frame;
};

typedef void(*BulletTick)(Bullet*, nkF32);

struct BulletDesc
{
    BulletTick    tick;
    const nkChar* texture;
    BulletTeam    team;
    nkS32         damage;
    nkF32         speed;
    nkF32         range;
    fRect         bounds;
};

GLOBAL void    bullet_init          (void);
GLOBAL void    bullet_tick          (nkF32 dt);
GLOBAL void    bullet_draw          (void);
GLOBAL void    spawn_bullet         (BulletID id, nkF32 x, nkF32 y, nkF32 tx, nkF32 ty);
GLOBAL Texture get_bullet_id_texture(BulletID id);
GLOBAL ImmClip get_bullet_clip      (Bullet* bullet);

/*////////////////////////////////////////////////////////////////////////////*/
