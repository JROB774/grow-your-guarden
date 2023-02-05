/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL AnimGroup* create_animation_group(void* data, nkU64 size)
{
    AnimGroup* group = NK_CALLOC_TYPES(AnimGroup, 1);
    if(!group) fatal_error("Failed to allocate animation group!");

    // Allocate a copy of the input data so that we can operate on it.
    nkU64 buffer_size = (size+1)*sizeof(nkChar);
    nkChar* buffer = NK_CALLOC_TYPES(nkChar, buffer_size);
    if(!buffer) fatal_error("Failed to allocate animation group buffer!");
    strncpy(buffer, NK_CAST(nkChar*,data), size);
    NK_DEFER(NK_FREE(buffer));

    // Parse the content of the animation group file line-by-line.
    nkString anim_name;

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
        if(strcmp(ident, "BEGINANIM") == 0)
        {
            Anim anim = NK_ZERO_MEM;

            // Enter into a sub-loop and parse per-animation commands until ENDAMIN.
            while(*cursor)
            {
                line = str_get_line(&cursor);
                if(!line || strlen(line) == 0)
                {
                    continue;
                }

                // Handle different identifiers.
                ident = str_get_word(&line);
                if(strcmp(ident, "ENDANIM") == 0)
                {
                    // We are done now so we can add to the group and exit the animation sub-loop.
                    nk_hashmap_insert(&group->anims, anim_name, anim);
                    break;
                }
                if(strcmp(ident, "NAME") == 0)
                {
                    str_eat_space(&line);
                    nk_string_assign(&anim_name, line);
                    continue;
                }
                if(strcmp(ident, "LOOP") == 0)
                {
                    anim.looped = NK_TRUE;
                    continue;
                }
                if(strcmp(ident, "CLIP") == 0)
                {
                    AnimFrame frame;
                    frame.x        = str_get_f32(&line);
                    frame.y        = str_get_f32(&line);
                    frame.w        = str_get_f32(&line);
                    frame.h        = str_get_f32(&line);
                    frame.duration = str_get_f32(&line);

                    nk_array_append(&anim.frames, frame);

                    continue;
                }

                // If we got down here then it's an unknown identifier.
                printf("Unknown identifier when parsing animation: '%s'\n", ident);
            }

            continue;
        }

        // If we got down here then it's an unknown identifier.
        printf("Unknown identifier when parsing animation set: '%s'\n", ident);
    }

    return group;
}

GLOBAL void free_animation_group(AnimGroup* group)
{
    NK_ASSERT(group);
    nk_hashmap_free(&group->anims);
    NK_FREE(group);
}

GLOBAL AnimState create_animation_state(AnimGroup* group)
{
    NK_ASSERT(group);

    AnimState state;
    state.anims   = group;
    state.current = &group->anims.begin()->value; // By default just pick whatever animation is first in the group.
    state.frame   = 0;
    state.timer   = 0.0f;
    return state;
}

GLOBAL AnimState create_animation_state(const nkChar* group_name)
{
    return create_animation_state(asset_manager_load<AnimGroup*>(group_name));
}

GLOBAL void set_animation(AnimState* state, const nkChar* anim_name, nkBool reset)
{
    NK_ASSERT(state);
    NK_ASSERT(state->anims);

    // Early out if neceesary.
    Anim* anim = nk_hashmap_getptr(&state->anims->anims, nkString(anim_name));
    if(!anim || (state->current == anim && !reset)) return;

    state->current = anim;
    state->frame   = 0;
    state->timer   = 0.0f;
}

GLOBAL void set_animation_frame(AnimState* state, nkS32 frame)
{
    NK_ASSERT(state);
    NK_ASSERT(state->current);

    Anim* anim = state->current;

    state->frame = nk_clamp(frame, 0, NK_CAST(nkS32,anim->frames.length)-1);
    state->timer = 0.0f;

    for(nkU64 i=0; i<state->frame; ++i)
    {
        state->timer += anim->frames[i].duration;
    }
}

GLOBAL void update_animation(AnimState* state, nkF32 dt)
{
    NK_ASSERT(state);
    NK_ASSERT(state->anims);
    NK_ASSERT(state->current); // State needs to have an animation it is using (call set_animation)!

    Anim* anim = state->current;

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

GLOBAL nkBool has_animation(AnimState* state, const nkChar* anim_name)
{
    NK_ASSERT(state);
    NK_ASSERT(state->anims);

    return nk_hashmap_contains(&state->anims->anims, nkString(anim_name));
}

GLOBAL nkBool is_animation_done(AnimState* state)
{
    NK_ASSERT(state);
    NK_ASSERT(state->anims);
    NK_ASSERT(state->current); // State needs to have an animation is is using (call set_animation)!

    Anim* anim = state->current;

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
    NK_ASSERT(state->anims);
    NK_ASSERT(state->current); // State needs to have an animation is is using (call set_animation)!

    return state->current->frames[state->frame];
}

/*////////////////////////////////////////////////////////////////////////////*/
