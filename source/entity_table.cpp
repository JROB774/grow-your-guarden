/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr EntityDesc ENTITY_TABLE[] =
{
{ NULL,                NULL,                NULL,          NULL,           EntityType_None,     0, {   0.0f,   0.0f,  0.0f,  0.0f } }, // EntityID_None
{ "ent_p/flower.png",  "ent_p/flower.anm",  "phase0_idle", ptick__flower,  EntityType_Plant,    3, { -16.0f, -16.0f, 32.0f, 32.0f } }, // EntityID_Plant_Flower
{ "ent_p/bramble.png", "ent_p/bramble.anm", "phase0_idle", ptick__bramble, EntityType_Plant,    3, { -16.0f, -16.0f, 32.0f, 32.0f } }, // EntityID_Plant_Bramble
{ "ent_m/walker.png",  "ent_m/walker.anm",  "walk",        mtick__walker,  EntityType_Monster, 10, { -16.0f, -16.0f, 32.0f, 32.0f } }, // EntityID_Monster_Walker
{ "ent_b/flower.png",  "ent_b/flower.anm",  "idle",        btick__flower,  EntityType_Bullet,   0, {  -8.0f,  -8.0f, 16.0f, 16.0f } }, // EntityID_Bullet_Flower
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(ENTITY_TABLE) == EntityID_TOTAL, entity_table_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/
