/*////////////////////////////////////////////////////////////////////////////*/

struct AnimFrame
{
    nkF32 x,y,w,h;
    nkF32 duration;
};

struct Animation
{
    nkBool             looped;
    nkArray<AnimFrame> frames;
};

struct AnimState
{
    Animation* anim;
    nkU32      frame;
    nkF32      timer;
};

GLOBAL Animation* create_animation           (void* data, nkU64 size);
GLOBAL void       free_animation             (Animation* anim);
GLOBAL void       set_animation              (AnimState* state, Animation* anim);
GLOBAL void       set_animation              (AnimState* state, const nkChar* anim_name); // Goes through the asset manager.
GLOBAL void       update_animation           (AnimState* state, nkF32 dt);
GLOBAL void       reset_animation            (AnimState* state);
GLOBAL nkBool     is_animation_done          (AnimState* state);
GLOBAL AnimFrame  get_current_animation_frame(AnimState* state);

/*////////////////////////////////////////////////////////////////////////////*/

template<>
Animation* asset_load<Animation*>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_animation(data, size);
}
template<>
void asset_free<Animation*>(Asset<Animation*>& asset)
{
    free_animation(asset.data);
}
template<>
const nkChar* asset_path<Animation*>(void)
{
    return "anims/";
}

/*////////////////////////////////////////////////////////////////////////////*/
