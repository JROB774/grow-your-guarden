/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkS32 MIXER_FREQUENCY     = MIX_DEFAULT_FREQUENCY;
INTERNAL constexpr nkU16 MIXER_SAMPLE_FORMAT = MIX_DEFAULT_FORMAT;
INTERNAL constexpr nkS32 MIXER_CHANNELS      = 2; // Stereo Sound
INTERNAL constexpr nkS32 MIXER_SAMPLE_SIZE   = 2048;

INTERNAL constexpr nkS32 MAX_AUDIO_CHANNELS = 64;

struct AudioContext
{
    nkF32 sound_volume;
    nkF32 music_volume;

    Music next_track;
    nkS32 next_track_loops;
    nkF32 next_track_fade;
};

INTERNAL AudioContext g_audio;

GLOBAL void init_audio_system(void)
{
    if(!NK_CHECK_FLAGS(Mix_Init(MIX_INIT_OGG), MIX_INIT_OGG))
        fatal_error("Failed to initialize SDL2 Mixer OGG support: %s", Mix_GetError());
    if(Mix_OpenAudio(MIXER_FREQUENCY, MIXER_SAMPLE_FORMAT, MIXER_CHANNELS, MIXER_SAMPLE_SIZE) != 0)
        fatal_error("Failed to open SDL2 Mixer audio device: %s", Mix_GetError());

    Mix_AllocateChannels(MAX_AUDIO_CHANNELS);

    set_sound_volume(0.8f);
    set_music_volume(0.7f);
}

GLOBAL void quit_audio_system(void)
{
    Mix_CloseAudio();
    Mix_Quit();
}

// Audio =======================================================================

GLOBAL void set_sound_volume(nkF32 volume)
{
    g_audio.sound_volume = nk_clamp(volume, 0.0f, 1.0f);
    nkS32 ivolume = NK_CAST(nkS32, NK_CAST(nkF32, MIX_MAX_VOLUME) * g_audio.sound_volume);
    Mix_Volume(-1, ivolume);
}

GLOBAL void set_music_volume(nkF32 volume)
{
    g_audio.music_volume = nk_clamp(volume, 0.0f, 1.0f);
    nkS32 ivolume = NK_CAST(nkS32, NK_CAST(nkF32, MIX_MAX_VOLUME) * g_audio.music_volume);
    Mix_VolumeMusic(ivolume);
}

GLOBAL nkF32 get_sound_volume(void)
{
    return g_audio.sound_volume;
}

GLOBAL nkF32 get_music_volume(void)
{
    return g_audio.music_volume;
}

GLOBAL nkBool is_sound_on(void)
{
    return (g_audio.sound_volume > 0.0f);
}

GLOBAL nkBool is_music_on(void)
{
    return (g_audio.music_volume > 0.0f);
}

GLOBAL nkBool is_music_playing(void)
{
    return NK_CAST(nkBool, Mix_PlayingMusic());
}

// =============================================================================

// Sound =======================================================================

DEFINE_PRIVATE_TYPE(Sound)
{
    Mix_Chunk* chunk;
};

GLOBAL Sound create_sound_from_file(const nkChar* file_name)
{
    Sound sound = ALLOCATE_PRIVATE_TYPE(Sound);
    if(!sound)
        fatal_error("Failed to allocate sound!");
    sound->chunk = Mix_LoadWAV(file_name);
    if(!sound->chunk)
        fatal_error("Failed to load sound from file: %s (%s)", file_name, Mix_GetError());
    return sound;
}

GLOBAL Sound create_sound_from_data(void* data, nkU64 size)
{
    Sound sound = ALLOCATE_PRIVATE_TYPE(Sound);
    if(!sound)
        fatal_error("Failed to allocate sound!");
    SDL_RWops* rwops = SDL_RWFromMem(data, NK_CAST(int, size));
    if(!rwops)
        fatal_error("Failed to create RWops from data! (%s)", SDL_GetError());
    sound->chunk = Mix_LoadWAV_RW(rwops, SDL_TRUE);
    if(!sound->chunk)
        fatal_error("Failed to load sound from data! (%s)", Mix_GetError());
    return sound;
}

GLOBAL void free_sound(Sound sound)
{
    if(!sound) return;
    Mix_FreeChunk(sound->chunk);
    NK_FREE(sound);
}

GLOBAL SoundRef play_sound(Sound sound, nkS32 loops)
{
    return play_sound_on_channel(sound, loops, -1);
}

GLOBAL SoundRef play_sound_on_channel(Sound sound, nkS32 loops, nkS32 channel)
{
    NK_ASSERT(channel < MAX_AUDIO_CHANNELS);
    NK_ASSERT(sound);
    channel = Mix_PlayChannel(channel, sound->chunk, loops);
    if(channel == -1)
        printf("Failed to play sound effect: %s\n", Mix_GetError());
    return NK_CAST(SoundRef,channel);
}

GLOBAL void resume_sound(SoundRef sound_ref)
{
    Mix_Resume(sound_ref);
}

GLOBAL void pause_sound(SoundRef sound_ref)
{
    Mix_Pause(sound_ref);
}

GLOBAL void stop_sound(SoundRef sound_ref)
{
    Mix_HaltChannel(sound_ref);
}

GLOBAL void fade_out_sound(SoundRef sound_ref, nkF32 seconds)
{
    nkS32 ms = NK_CAST(nkS32, seconds * 1000.0f);
    Mix_FadeOutChannel(sound_ref, ms);
}

// =============================================================================

// Music =======================================================================

DEFINE_PRIVATE_TYPE(Music)
{
    Mix_Music* music;
};

INTERNAL void play_music_audio_hook(void)
{
    if(g_audio.next_track)
    {
        play_music_fade_in(g_audio.next_track, g_audio.next_track_loops, g_audio.next_track_fade);
        g_audio.next_track = NULL;
    }

    // Remove the hook because we no longer need it.
    Mix_HookMusicFinished(NULL);
}

GLOBAL Music create_music_from_file(const nkChar* file_name)
{
    Music music = ALLOCATE_PRIVATE_TYPE(Music);
    if(!music)
        fatal_error("Failed to allocate music!");
    music->music = Mix_LoadMUS(file_name);
    if(!music->music)
        fatal_error("Failed to load music from file: %s (%s)", file_name, Mix_GetError());
    return music;
}

GLOBAL Music create_music_from_data(void* data, nkU64 size)
{
    Music music = ALLOCATE_PRIVATE_TYPE(Music);
    if(!music)
        fatal_error("Failed to allocate music!");
    SDL_RWops* rwops = SDL_RWFromMem(data, NK_CAST(int, size));
    if(!rwops)
        fatal_error("Failed to create RWops from data! (%s)", SDL_GetError());
    music->music = Mix_LoadMUS_RW(rwops, SDL_TRUE);
    if(!music->music)
        fatal_error("Failed to load music from data! (%s)", Mix_GetError());
    return music;
}

GLOBAL void free_music(Music music)
{
    if(!music) return;
    Mix_FreeMusic(music->music);
    NK_FREE(music);
}

GLOBAL void play_music(Music music, nkS32 loops)
{
    NK_ASSERT(music);
    stop_music(); // Stop any previous music to prevent scenarios where we get blocked forever...
    if(Mix_PlayMusic(music->music, loops) == -1)
        printf("Failed to play music: %s\n", Mix_GetError());
}

GLOBAL void play_music_fade_in(Music music, nkS32 loops, nkF32 seconds)
{
    NK_ASSERT(music);

    // SDL Mixer does not handle cross-fading music-tracks. So what we need to do
    // is check music is currently playing and if so fade it out. Then once it has
    // finished fading out we will fade in the new music. This prevents blocking
    // and is the best we can do.
    if(Mix_PlayingMusic())
    {
        g_audio.next_track       = music;
        g_audio.next_track_fade  = seconds;
        g_audio.next_track_loops = loops;

        stop_music_fade_out(seconds * 0.5f);
        Mix_HookMusicFinished(play_music_audio_hook);
    }
    else
    {
        nkS32 ms = NK_CAST(nkS32, seconds * 1000.0f);
        if(Mix_FadeInMusic(music->music, loops, ms) == -1)
            printf("Failed to play music fade in: %s\n", Mix_GetError());
    }
}

GLOBAL void resume_music(void)
{
    Mix_ResumeMusic();
}

GLOBAL void pause_music(void)
{
    Mix_PauseMusic();
}

GLOBAL void stop_music(void)
{
    g_audio.next_track = NULL;
    Mix_HaltMusic();
}

GLOBAL void stop_music_fade_out(nkF32 seconds)
{
    nkS32 ms = NK_CAST(nkS32, seconds * 1000.0f);
    Mix_FadeOutMusic(ms);
}

// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
