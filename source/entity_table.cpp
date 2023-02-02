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
/* Animation Group */ NULL,
/* Start Animation */ NULL,
/* Tick            */ NULL,
/* Type            */ EntityType_None,
/* Health          */ 0.0f,
/* Damage          */ 0.0f,
/* Speed           */ 0.0f,
/* Range           */ 0.0f,
/* Collision Mask  */ EntityType_None,
/* Bounds          */ { 0.0f,0.0f },
/* Growth Phases   */ { 0,0,0,0,0,0,0,0 }
},

// EntityID_Daisy
{
/* Texture         */ "ent_p/daisy.png",
/* Animation Group */ "ent_p/daisy.anm",
/* Start Animation */ "phase0_idle",
/* Tick            */ ETICK(daisy),
/* Type            */ EntityType_Plant,
/* Health          */ 5.0f,
/* Damage          */ 0.0f,
/* Speed           */ 0.0f,
/* Range           */ 4.5f,
/* Collision Mask  */ EntityType_None,
/* Bounds          */ { 1.0f,1.0f },
/* Growth Phases   */ { 2,2,0,0,0,0,0,0 }
},

// EntityID_Bramble
{
/* Texture         */ "ent_p/bramble.png",
/* Animation Group */ "ent_p/bramble.anm",
/* Start Animation */ "phase0_idle",
/* Tick            */ ETICK(bramble),
/* Type            */ EntityType_Plant,
/* Health          */ 5.0f,
/* Damage          */ 0.0f,
/* Speed           */ 0.0f,
/* Range           */ 0.0f,
/* Collision Mask  */ EntityType_None,
/* Bounds          */ { 1.0f,1.0f },
/* Growth Phases   */ { 30,40,50,0,0,0,0,0 }
},

// EntityID_Walker
{
/* Texture         */ "ent_m/walker.png",
/* Animation Group */ "ent_m/walker.anm",
/* Start Animation */ "walk",
/* Tick            */ ETICK(walker),
/* Type            */ EntityType_Monster,
/* Health          */ 10.0f,
/* Damage          */ 1.0f,
/* Speed           */ 0.5f,
/* Range           */ 0.0f,
/* Collision Mask  */ EntityType_None,
/* Bounds          */ { 1.0f,1.0f },
/* Growth Phases   */ { 0,0,0,0,0,0,0,0 }
},

// EntityID_Pollen
{
/* Texture         */ "ent_b/pollen.png",
/* Animation Group */ "ent_b/pollen.anm",
/* Start Animation */ "idle",
/* Tick            */ NULL,
/* Type            */ EntityType_Bullet,
/* Health          */ 0.0f,
/* Damage          */ 2.0f,
/* Speed           */ 3.1f,
/* Range           */ 5.0f,
/* Collision Mask  */ EntityType_Monster,
/* Bounds          */ { 0.5f,0.5f },
/* Growth Phases   */ { 0,0,0,0,0,0,0,0 }
},

// EntityID_House
{
/* Texture         */ "ent_o/house.png",
/* Animation Group */ "ent_o/house.anm",
/* Start Animation */ "idle",
/* Tick            */ NULL,
/* Type            */ EntityType_Base,
/* Health          */ 100.0f,
/* Damage          */ 0.0f,
/* Speed           */ 0.0f,
/* Range           */ 0.0f,
/* Collision Mask  */ EntityType_None,
/* Bounds          */ { 3.0f,3.0f },
/* Growth Phases   */ { 0,0,0,0,0,0,0,0 }
},

}; // ENTITY_TABLE

NK_STATIC_ASSERT(NK_ARRAY_SIZE(ENTITY_TABLE) == EntityID_TOTAL, entity_table_size_mismatch);

/*////////////////////////////////////////////////////////////////////////////*/
