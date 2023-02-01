/*////////////////////////////////////////////////////////////////////////////*/

DEF_ETICK(daisy);
DEF_ETICK(bramble);
DEF_ETICK(walker);

enum: nkU32
{
EntityID_None,
EntityID_Daisy,
EntityID_Bramble,
EntityID_Walker,
EntityID_Pollen,
EntityID_House,
EntityID_TOTAL
};

INTERNAL constexpr EntityDesc ENTITY_TABLE[] =
{
// Texture              Animation Group      Start Anim     Behavior        Type                Health    Damage    Speed    Range   Collision           Bounds         Phases
{  NULL,                NULL,                NULL,          NULL,           EntityType_None,      0.0f,     0.0f,    0.0f,    0.0f,  EntityType_None,    { 0.0f,0.0f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_None
{  "ent_p/daisy.png",   "ent_p/daisy.anm",   "phase0_idle", ETICK(daisy),   EntityType_Plant,     3.0f,     0.0f,    0.0f,    4.5f,  EntityType_None,    { 1.0f,1.0f }, { 15,20, 0, 0, 0, 0, 0, 0 } }, // EntityID_Daisy
{  "ent_p/bramble.png", "ent_p/bramble.anm", "phase0_idle", ETICK(bramble), EntityType_Plant,     3.0f,     0.0f,    0.0f,    0.0f,  EntityType_None,    { 1.0f,1.0f }, { 30,40,50, 0, 0, 0, 0, 0 } }, // EntityID_Bramble
{  "ent_m/walker.png",  "ent_m/walker.anm",  "walk",        ETICK(walker),  EntityType_Monster,  10.0f,     1.0f,    0.5f,    0.0f,  EntityType_None,    { 1.0f,1.0f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_Walker
{  "ent_b/pollen.png",  "ent_b/pollen.anm",  "idle",        NULL,           EntityType_Bullet,    0.0f,     2.0f,    3.1f,    5.0f,  EntityType_Monster, { 0.5f,0.5f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_Pollen
{  "ent_o/house.png",   "ent_o/house.anm",   "idle",        NULL,           EntityType_Object,  100.0f,     0.0f,    0.0f,    0.0f,  EntityType_None,    { 3.0f,3.0f }, {  0, 0, 0, 0, 0, 0, 0, 0 } }, // EntityID_House
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(ENTITY_TABLE) == EntityID_TOTAL, entity_table_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/
