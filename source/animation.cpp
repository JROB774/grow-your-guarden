/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL Animation* create_animation(void* data, nkU64 size)
{
    Animation* anim = NK_CALLOC_TYPES(Animation, 1);
    if(!anim) fatal_error("Failed to allocate animation!");

    // Allocate a copy of the input data so that we can operate on it.
    nkU64 buffer_size = (size+1)*sizeof(nkChar);
    nkChar* buffer = NK_CALLOC_TYPES(nkChar, buffer_size);
    if(!buffer) fatal_error("Failed to allocate animation buffer!");
    strncpy(buffer, NK_CAST(nkChar*,data), size);
    NK_DEFER(NK_FREE(buffer));

    // Parse the content of the animation file line-by-line.
    nkChar* cursor = buffer;
    while(*cursor)
    {
        nkChar* line = str_get_line(&cursor);
        if(!line || strlen(line) == 0)
        {
            continue;
        }

        // Handle different identifiers.
        const nkChar* ident = str_get_word(&line);
        if(strcmp(ident, "LOOPED"))
        {
            anim->looped = NK_TRUE;
            continue;
        }
        if(strcmp(ident, "FRAME") == 0)
        {
            AnimFrame frame;
            frame.x        = str_get_f32(&line);
            frame.y        = str_get_f32(&line);
            frame.w        = str_get_f32(&line);
            frame.h        = str_get_f32(&line);
            frame.duration = str_get_f32(&line);

            nk_array_append(&anim->frames, frame);

            continue;
        }

        // If we got down here then it's an unknown identifier.
        printf("Unknown identifier when parsing animation: '%s'\n", ident);
    }

    return anim;
}

GLOBAL void free_animation(Animation* anim)
{
    NK_ASSERT(anim);

    nk_array_free(&anim->frames);
    NK_FREE(anim);
}

GLOBAL void set_animation(AnimState* state, Animation* anim)
{
    NK_ASSERT(state);
    NK_ASSERT(anim);

    state->anim  = anim;
    state->frame = 0;
    state->timer = 0.0f;
}

GLOBAL void set_animation(AnimState* state, const nkChar* anim_name)
{
    set_animation(state, asset_manager_load<Animation*>(anim_name));
}

GLOBAL void update_animation(AnimState* state, nkF32 dt)
{
    NK_ASSERT(state);
    NK_ASSERT(state->anim); // State needs to have an animation is is using (call set_animation)!

    Animation* anim = state->anim;

    if(anim->looped || state->frame < anim->frames.length)
    {
        nkF32 time = 0.0f;
        for(nkU64 i=0; i<anim->frames.length; ++i)
        {
            time += anim->frames[i].duration;
            if(state->timer <= time)
            {
                state->frame = NK_CAST(nkU32, i);
                break;
            }
        }

        state->timer += dt;

        // Handles wrapping back round to the start of the animation if we're looped.
        if(anim->looped)
        {
            nkF32 total_time = 0.0f;
            for(auto& frame: anim->frames)
            {
                total_time += frame.duration;
            }

            if(total_time > 0.0f)
            {
                while(state->timer >= total_time)
                {
                    state->timer -= total_time;
                }
            }
        }
    }
}

GLOBAL void reset_animation(AnimState* state)
{
    NK_ASSERT(state);

    state->frame = 0;
    state->timer = 0.0f;
}

GLOBAL nkBool is_animation_done(AnimState* state)
{
    NK_ASSERT(state);
    NK_ASSERT(state->anim); // State needs to have an animation is is using (call set_animation)!

    Animation* anim = state->anim;

    // Looped animations never finish.
    if(anim->looped) return NK_FALSE;

    nkF32 total_time = 0.0f;
    for(auto& frame: anim->frames)
    {
        total_time += frame.duration;
    }

    return (state->timer >= total_time);
}

GLOBAL AnimFrame get_current_animation_frame(AnimState* state)
{
    NK_ASSERT(state);
    NK_ASSERT(state->anim); // State needs to have an animation is is using (call set_animation)!

    return state->anim->frames[state->frame];
}

/*////////////////////////////////////////////////////////////////////////////*/
