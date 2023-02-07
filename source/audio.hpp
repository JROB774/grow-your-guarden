/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void init_audio_system(void);
GLOBAL void quit_audio_system(void);

// Audio =======================================================================
GLOBAL void   set_sound_volume(nkF32 volume); // [0-1]
GLOBAL void   set_music_volume(nkF32 volume); // [0-1]
GLOBAL nkF32  get_sound_volume(void);
GLOBAL nkF32  get_music_volume(void);
GLOBAL nkBool is_sound_on     (void);
GLOBAL nkBool is_music_on     (void);
GLOBAL nkBool is_music_playing(void);
// =============================================================================

// Sound =======================================================================
DECLARE_PRIVATE_TYPE(Sound);

typedef nkU32 SoundRef;

GLOBAL constexpr SoundRef INVALID_SOUND_REF = 0xFFFFFFFF;

GLOBAL Sound    create_sound_from_file(const nkChar* file_name);
GLOBAL Sound    create_sound_from_data(void* data, nkU64 size);
GLOBAL void     free_sound            (Sound sound);
GLOBAL SoundRef play_sound            (Sound sound, nkS32 loops = 0);
GLOBAL SoundRef play_sound_on_channel (Sound sound, nkS32 loop, nkS32 channel);
GLOBAL void     resume_sound          (SoundRef sound_ref);
GLOBAL void     pause_sound           (SoundRef sound_ref);
GLOBAL void     stop_sound            (SoundRef sound_ref);
GLOBAL void     fade_out_sound        (SoundRef sound_ref, nkF32 seconds);
// =============================================================================

// Music =======================================================================
DECLARE_PRIVATE_TYPE(Music);

GLOBAL Music create_music_from_file(const nkChar* file_name);
GLOBAL Music create_music_from_data(void* data, nkU64 size);
GLOBAL void  free_music            (Music music);
GLOBAL void  play_music            (Music music, nkS32 loops = -1);
GLOBAL void  play_music_fade_in    (Music music, nkS32 loops = -1, nkF32 seconds = 5.0f);
GLOBAL void  resume_music          (void);
GLOBAL void  pause_music           (void);
GLOBAL void  stop_music            (void);
GLOBAL void  stop_music_fade_out   (nkF32 seconds = 5.0f);
// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/

template<>
Sound asset_load<Sound>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_sound_from_data(data, size);
}
template<>
void asset_free<Sound>(Asset<Sound>& asset)
{
    return free_sound(asset.data);
}
template<>
const nkChar* asset_path<Sound>(void)
{
    return "sounds/";
}

template<>
Music asset_load<Music>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_music_from_data(data, size);
}
template<>
void asset_free<Music>(Asset<Music>& asset)
{
    return free_music(asset.data);
}
template<>
const nkChar* asset_path<Music>(void)
{
    return "music/";
}

/*////////////////////////////////////////////////////////////////////////////*/
