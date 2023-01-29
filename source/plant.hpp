/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(PlantID, nkU32)
{
    PlantID_None,
    PlantID_Flower,
    PlantID_TOTAL
};

struct Plant
{
    PlantID id;
    nkS32   x,y;
    nkS32   health;
    nkS32   phase;
    nkS32   width;
    nkS32   height;
    nkF32   anim_timer;
    nkS32   anim_frame;
};

typedef void(*PlantTick)(Plant*, nkF32);

struct PlantDesc
{
    PlantTick     tick;
    const nkChar* texture;
    nkS32         health;
    nkS32         width;
    nkS32         height;
};

GLOBAL void    plant_tick            (Plant* plants, nkU64 count, nkF32 dt);
GLOBAL void    plant_draw            (Plant* plants, nkU64 count);
GLOBAL ImmClip get_plant_id_icon_clip(PlantID id);
GLOBAL Texture get_plant_id_texture  (PlantID id);
GLOBAL ImmClip get_plant_clip        (Plant* plant);

/*////////////////////////////////////////////////////////////////////////////*/
