/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL void btick__flower(Bullet* b, nkF32 dt)
{
    // @Incomplete: ...
}

/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr BulletDesc BULLET_TABLE[] =
{
{ NULL,          NULL,             BulletTeam_None,    0,   0.0f,   0.0f, {   0.0f,   0.0f,  0.0f,  0.0f } }, // None
{ btick__flower, "projectile.png", BulletTeam_Plants,  2, 100.0f, 160.0f, {  -8.0f,  -8.0f, 16.0f, 16.0f } }, // Flower
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(BULLET_TABLE) == BulletID_TOTAL, bullet_table_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkF32 BULLET_ANIM_SPEED = 0.3f;

GLOBAL void bullet_init(void)
{
    // Pre-load a bunch of assets.
    for(nkU32 i=0; i<BulletID_TOTAL; ++i)
    {
        const BulletDesc& desc = BULLET_TABLE[i];
        if(desc.texture)
        {
            asset_manager_load<Texture>(desc.texture);
        }
    }
}

GLOBAL void bullet_tick(nkF32 dt)
{
    nkArray<nkU64> to_kill;

    nkU64 index = 0;

    for(auto& b: g_world.bullets)
    {
        if(b.id != BulletID_None)
        {
            const BulletDesc& desc = BULLET_TABLE[b.id];

            // If the bullet has travelled its full range then kill it.
            nkF32 distance = distance_between_points(b.position, b.spawn);
            if(distance >= desc.range)
            {
                nk_array_append(&to_kill, index);
            }

            // Do the bullet's custom update logic.
            if(desc.tick)
            {
                desc.tick(&b, dt);
            }

            // Apply velocity.
            b.position += b.velocity * dt;

            // Do animation logic.
            b.anim_timer += dt;
            if(b.anim_timer >= BULLET_ANIM_SPEED)
            {
                nkS32 max_frames = get_texture_width(get_bullet_id_texture(b.id)) / 16;

                b.anim_timer -= BULLET_ANIM_SPEED;
                b.anim_frame = ((b.anim_frame + 1) % max_frames);
            }
        }

        ++index;
    }

    for(auto& i: to_kill)
    {
        nk_array_remove(&g_world.bullets, i);
    }
}

GLOBAL void bullet_draw(void)
{
    for(auto& b: g_world.bullets)
    {
        if(b.id != BulletID_None)
        {
            Texture texture = get_bullet_id_texture(b.id);
            ImmClip clip = get_bullet_clip(&b);

            nkF32 bx = b.position.x;
            nkF32 by = b.position.y;

            imm_texture(texture, bx,by, &clip);
        }
    }
}

GLOBAL void spawn_bullet(BulletID id, nkF32 x, nkF32 y, nkF32 tx, nkF32 ty)
{
    const BulletDesc& desc = BULLET_TABLE[id];

    nkVec2 src = {  x, y };
    nkVec2 dst = { tx,ty };

    nkVec2 dir = nk_normalize(dst - src);

    Bullet bullet = NK_ZERO_MEM;

    bullet.id          = id;
    bullet.team        = desc.team;
    bullet.position    = { x,y };
    bullet.spawn       = { x,y };
    bullet.velocity    = dir * desc.speed;
    bullet.bounds      = desc.bounds;
    bullet.anim_timer  = rng_f32(0.0f, BULLET_ANIM_SPEED);
    bullet.anim_frame  = 0;

    nk_array_append(&g_world.bullets, bullet);
}

GLOBAL Texture get_bullet_id_texture(BulletID id)
{
    NK_ASSERT(id < BulletID_TOTAL);
    return asset_manager_load<Texture>(BULLET_TABLE[id].texture);
}

GLOBAL ImmClip get_bullet_clip(Bullet* bullet)
{
    NK_ASSERT(bullet);
    ImmClip clip;
    clip.x = NK_CAST(nkF32, bullet->anim_frame * 16);
    clip.y = 0.0f;
    clip.w = NK_CAST(nkF32, 16);
    clip.h = NK_CAST(nkF32, 16);
    return clip;
}

/*////////////////////////////////////////////////////////////////////////////*/
