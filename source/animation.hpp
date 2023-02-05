/*////////////////////////////////////////////////////////////////////////////*/

struct AnimFrame
{
    nkF32 x,y,w,h;
    nkF32 duration;
};

struct Anim
{
    nkBool             looped;
    nkArray<AnimFrame> frames;
};

struct AnimGroup
{
    nkHashMap<nkString,Anim> anims;
};

struct AnimState
{
    AnimGroup* anims;
    Anim*      current;
    nkU32      frame;
    nkF32      timer;
};

GLOBAL AnimGroup* create_animation_group     (void* data, nkU64 size);
GLOBAL void       free_animation_group       (AnimGroup* group);
GLOBAL AnimState  create_animation_state     (AnimGroup* group);
GLOBAL AnimState  create_animation_state     (const nkChar* group_name); // Goes through the asset manager directly.
GLOBAL void       set_animation              (AnimState* state, const nkChar* anim_name, nkBool reset = NK_FALSE); // Does nothing if the animation is already playing (unless reset flag is true).
GLOBAL void       set_animation_frame        (AnimState* state, nkS32 frame); // Jump to the specified frame in the current animation.
GLOBAL void       update_animation           (AnimState* state, nkF32 dt);
GLOBAL void       reset_animation            (AnimState* state);
GLOBAL nkBool     has_animation              (AnimState* state, const nkChar* anim_name);
GLOBAL nkBool     is_animation_done          (AnimState* state);
GLOBAL AnimFrame  get_current_animation_frame(AnimState* state);

/*////////////////////////////////////////////////////////////////////////////*/

template<>
AnimGroup* asset_load<AnimGroup*>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_animation_group(data, size);
}
template<>
void asset_free<AnimGroup*>(Asset<AnimGroup*>& asset)
{
    free_animation_group(asset.data);
}
template<>
const nkChar* asset_path<AnimGroup*>(void)
{
    return "textures/";
}

/*////////////////////////////////////////////////////////////////////////////*/
