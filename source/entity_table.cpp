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

// EntityID_None
{
/* Texture         */ NULL,
/* Animation       */ NULL,
/* Type            */ EntityType_None,
/* Default State   */ EntityState_Idle,
/* Tick            */ NULL,
/* Health          */ 0.0f,
/* Damage          */ 0.0f,
/* Speed           */ 0.0f,
/* Range           */ 0.0f,
/* Radius          */ 0.0f,
/* Z-Depth         */ 0.0f,
/* Collision Mask  */ EntityType_None,
/* Draw Offset     */ { 0.0f,0.0f },
/* Bounds          */ { 0.0f,0.0f },
/* Growth Phases   */ { 0,0,0,0,0,0,0,0 }
},

// EntityID_Daisy
{
/* Texture         */ "ent_p/daisy.png",
/* Animation       */ "ent_p/daisy.anm",
/* Type            */ EntityType_Plant,
/* Default State   */ EntityState_Idle,
/* Tick            */ ETICK(daisy),
/* Health          */ 5.0f,
/* Damage          */ 0.0f,
/* Speed           */ 0.0f,
/* Range           */ 4.5f,
/* Radius          */ 0.3f,
/* Z-Depth         */ 0.0f,
/* Collision Mask  */ EntityType_None,
/* Draw Offset     */ { 0.0f,-0.4f },
/* Bounds          */ { 1.0f,1.0f },
/* Growth Phases   */ { 2,2,0,0,0,0,0,0 }
},

// EntityID_Bramble
{
/* Texture         */ "ent_p/bramble.png",
/* Animation       */ "ent_p/bramble.anm",
/* Type            */ EntityType_Plant,
/* Default State   */ EntityState_Idle,
/* Tick            */ ETICK(bramble),
/* Health          */ 5.0f,
/* Damage          */ 0.0f,
/* Speed           */ 0.0f,
/* Range           */ 0.0f,
/* Radius          */ 0.3f,
/* Z-Depth         */ 0.0f,
/* Collision Mask  */ EntityType_None,
/* Draw Offset     */ { 0.0f,0.0f },
/* Bounds          */ { 1.0f,1.0f },
/* Growth Phases   */ { 30,40,50,0,0,0,0,0 }
},

// EntityID_Walker
{
/* Texture         */ "ent_m/walker.png",
/* Animation       */ "ent_m/walker.anm",
/* Type            */ EntityType_Monster,
/* Default State   */ EntityState_Move,
/* Tick            */ ETICK(walker),
/* Health          */ 10.0f,
/* Damage          */ 1.0f,
/* Speed           */ 0.5f,
/* Range           */ 0.0f,
/* Radius          */ 0.3f,
/* Z-Depth         */ 0.0f,
/* Collision Mask  */ EntityType_None,
/* Draw Offset     */ { 0.0f,-0.25f },
/* Bounds          */ { 1.0f,1.0f },
/* Growth Phases   */ { 0,0,0,0,0,0,0,0 }
},

// EntityID_Pollen
{
/* Texture         */ "ent_b/pollen.png",
/* Animation       */ "ent_b/pollen.anm",
/* Type            */ EntityType_Bullet,
/* Default State   */ EntityState_Idle,
/* Tick            */ NULL,
/* Health          */ 1.0f,
/* Damage          */ 2.0f,
/* Speed           */ 3.1f,
/* Range           */ 5.0f,
/* Radius          */ 0.1f,
/* Z-Depth         */ 0.5f,
/* Collision Mask  */ EntityType_Monster,
/* Draw Offset     */ { 0.0f,0.0f },
/* Bounds          */ { 0.5f,0.5f },
/* Growth Phases   */ { 0,0,0,0,0,0,0,0 }
},

// EntityID_House
{
/* Texture         */ "ent_o/house.png",
/* Animation       */ "ent_o/house.anm",
/* Type            */ EntityType_Base,
/* Default State   */ EntityState_Idle,
/* Tick            */ NULL,
/* Health          */ 100.0f,
/* Damage          */ 0.0f,
/* Speed           */ 0.0f,
/* Range           */ 0.0f,
/* Radius          */ 1.4f,
/* Z-Depth         */ 0.0f,
/* Collision Mask  */ EntityType_None,
/* Draw Offset     */ { 0.0f,0.0f },
/* Bounds          */ { 3.0f,3.0f },
/* Growth Phases   */ { 0,0,0,0,0,0,0,0 }
},

}; // ENTITY_TABLE

NK_STATIC_ASSERT(NK_ARRAY_SIZE(ENTITY_TABLE) == EntityID_TOTAL, entity_table_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/
