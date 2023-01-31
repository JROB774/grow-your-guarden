/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr EntityDesc ENTITY_TABLE[] =
{
// Texture              Animation Group      Start Anim     Behavior        Type                Stats (H/D/S/R)   Collision           Bounds                            Phases
{  NULL,                NULL,                NULL,          NULL,           EntityType_None,     0, 0,  0,  0,    EntityType_None,    {   0.0f,   0.0f,  0.0f,  0.0f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_None
{  "ent_p/flower.png",  "ent_p/flower.anm",  "phase0_idle", ptick__flower,  EntityType_Plant,    3, 0,  0,140,    EntityType_None,    { -16.0f, -16.0f, 32.0f, 32.0f }, {  5, 5, 0, 0, 0, 0, 0, 0 } }, // EntityID_Plant_Flower
{  "ent_p/bramble.png", "ent_p/bramble.anm", "phase0_idle", ptick__bramble, EntityType_Plant,    3, 1,  0,  0,    EntityType_None,    { -16.0f, -16.0f, 32.0f, 32.0f }, {  2, 2, 2, 0, 0, 0, 0, 0 } }, // EntityID_Plant_Bramble
{  "ent_m/walker.png",  "ent_m/walker.anm",  "walk",        mtick__walker,  EntityType_Monster, 10, 1,  0,  0,    EntityType_None,    { -16.0f, -16.0f, 32.0f, 32.0f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_Monster_Walker
{  "ent_b/flower.png",  "ent_b/flower.anm",  "idle",        btick__flower,  EntityType_Bullet,   0, 2,100,160,    EntityType_Monster, {  -8.0f,  -8.0f, 16.0f, 16.0f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_Bullet_Flower
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(ENTITY_TABLE) == EntityID_TOTAL, entity_table_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/
