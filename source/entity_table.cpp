/*////////////////////////////////////////////////////////////////////////////*/

DEF_ETICK(daisy);
DEF_ETICK(bramble);

enum: nkU32
{
EntityID_None,
EntityID_Daisy,
EntityID_Bramble,
EntityID_Walker,
EntityID_Pollen,
EntityID_TOTAL
};

INTERNAL constexpr EntityDesc ENTITY_TABLE[] =
{
// Texture              Animation Group      Start Anim     Behavior        Type                Stats (H/D/S/R)   Collision           Bounds                            Phases
{  NULL,                NULL,                NULL,          NULL,           EntityType_None,     0, 0,  0,  0,    EntityType_None,    {  0.0f,  0.0f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_None
{  "ent_p/daisy.png",   "ent_p/daisy.anm",   "phase0_idle", ETICK(daisy),   EntityType_Plant,    3, 0,  0,140,    EntityType_None,    { 32.0f, 32.0f }, {  5, 5, 0, 0, 0, 0, 0, 0 } }, // EntityID_Daisy
{  "ent_p/bramble.png", "ent_p/bramble.anm", "phase0_idle", ETICK(bramble), EntityType_Plant,    3, 1,  0,  0,    EntityType_None,    { 32.0f, 32.0f }, {  2, 2, 2, 0, 0, 0, 0, 0 } }, // EntityID_Bramble
{  "ent_m/walker.png",  "ent_m/walker.anm",  "walk",        NULL,           EntityType_Monster, 10, 1,  0,  0,    EntityType_None,    { 32.0f, 32.0f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_Walker
{  "ent_b/pollen.png",  "ent_b/pollen.anm",  "idle",        NULL,           EntityType_Bullet,   0, 2,100,160,    EntityType_Monster, { 16.0f, 16.0f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_Pollen
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(ENTITY_TABLE) == EntityID_TOTAL, entity_table_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/
