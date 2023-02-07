/*////////////////////////////////////////////////////////////////////////////*/

#if defined(BUILD_DEBUG)
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...) ((void)0)
#endif

#define NO_PHASE NK_ZERO_MEM

INTERNAL constexpr nkU32 MAX_PHASES = 8;
INTERNAL constexpr nkU32 MAX_SPAWNS = 8;

NK_ENUM(SpawnType, nkU32)
{
    SpawnType_None   = (   0),
    SpawnType_Walker = (1<<0),
    SpawnType_All    = (  -1)
};

struct PhaseDesc
{
    nkF32     start_time;  // How much time should elapse from the start of the previous phase before this phase should start. (If the previous phase completes early the next phase starts immediately).
    SpawnType spawn_types; // What types of enemies should spawn during this phase.
    nkU32     min_spawns;  // The minimum bound for the total number of monsters to spawn.
    nkU32     max_spawns;  // The maximum bound for the total number of monsters to spawn.
};

struct WaveDesc
{
    nkU32     wave_number;        // What number wave this description should be used for.
    nkU32     num_phases;         // The number of phases in the wave.
    nkU32     spawn_points;       // How many spawn points should be used during the wave.
    nkF32     prep_timer;         // How long the player has to prepare before the wave starts.
    nkS32     wave_bonus;         // The amount of money to reward upon completion.
    PhaseDesc phases[MAX_PHASES]; // Descriptions of the different phases.
};

// Stores information about how each wave should roughly be structured. If the player
// manages to reach wave numbers past what is stored within this list then the wave
// manager system will just repeat the final wave in the list but with spawn numbers,
// etc. getting higher and higher each time.
//
// Note that the array indices do not map to wave number. The WaveDesc struct actually
// stores what number wave it should be used for. This allows us to have multiple
// definitions for a wave that the manager can randomly pick between. This adds a bit
// more variance to repeat playthroughs of the game.
GLOBAL constexpr WaveDesc WAVE_LIST[] =
{

// WAVE 01 ---------------------------------------
{
/* Wave Number  */ 1,
/* Phases       */ 1,
/* Spawn Points */ 1,
/* Prep Timer   */ 15.0f,
/* Wave Bonus   */ 500,
{
/* Phase 1      */ { 0.0f, SpawnType_Walker, 8,10 },
/* Phase 2      */ NO_PHASE,
/* Phase 3      */ NO_PHASE,
/* Phase 4      */ NO_PHASE,
/* Phase 5      */ NO_PHASE,
/* Phase 6      */ NO_PHASE,
/* Phase 7      */ NO_PHASE,
/* Phase 8      */ NO_PHASE,
}
},
// -----------------------------------------------

// WAVE 02 ---------------------------------------
{
/* Wave Number  */ 2,
/* Phases       */ 1,
/* Spawn Points */ 2,
/* Prep Timer   */ 30.0f,
/* Wave Bonus   */ 500,
{
/* Phase 1      */ { 0.0f, SpawnType_Walker, 15,17 },
/* Phase 2      */ NO_PHASE,
/* Phase 3      */ NO_PHASE,
/* Phase 4      */ NO_PHASE,
/* Phase 5      */ NO_PHASE,
/* Phase 6      */ NO_PHASE,
/* Phase 7      */ NO_PHASE,
/* Phase 8      */ NO_PHASE,
}
},
// -----------------------------------------------

}; // WAVE_LIST

/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkF32 WAVE_MULTIPLICATION_RATE = 0.5f;

INTERNAL constexpr nkF32 MESSAGE_TIME = 3.5f;

INTERNAL constexpr nkF32 BATTLE_MUSIC_FADE_IN_TIME  = 8.0f;
INTERNAL constexpr nkF32 BATTLE_MUSIC_FADE_OUT_TIME = 8.0f;

NK_ENUM(WaveState, nkS32)
{
    WaveState_Prepare,
    WaveState_Fight,
    WaveState_TOTAL
};

NK_ENUM(SpawnerState, nkS32)
{
    SpawnerState_Waiting,
    SpawnerState_InProgress,
    SpawnerState_Complete,
    SpawnerState_TOTAL
};

struct SpawnPoint
{
    nkVec2 position;
    fRect  region;
    nkBool active;
};

struct Spawner
{
    SpawnerState state;
    nkF32        timer;
    EntityID     spawn_types;
    nkU32        spawns_left;
};

struct WaveManager
{
    const WaveDesc*  wave_desc; // Descriptor for the current wave.
    SpawnPoint       spawn_points[MAX_SPAWNS];
    WaveState        state;
    nkF32            timer;
    nkF32            wave_multiplier;
    nkU32            waves_beaten;
    nkU32            current_wave;
    Spawner          spawners[MAX_PHASES];

    // Current message to display to the player.
    nkString         message;
    nkF32            message_timer;
    nkF32            message_alpha;
    nkVec3           message_color;
};

INTERNAL WaveManager g_wave_manager;

INTERNAL nkS32 get_monster_spawn_rate(SpawnType type)
{
    // These should add up to 100.
    if(NK_CHECK_FLAGS(type, SpawnType_Walker)) return 100;
    NK_ASSERT(NK_FALSE);
    return 0;
}

INTERNAL void post_wave_message(const nkChar* message, nkVec3 color, const nkChar* sound = NULL)
{
    if(sound) play_sound(asset_manager_load<Sound>(sound));

    g_wave_manager.message       = message;
    g_wave_manager.message_timer = MESSAGE_TIME;
    g_wave_manager.message_alpha = 1.0f;
    g_wave_manager.message_color = color;
}

INTERNAL void setup_next_wave(void)
{
    auto& wm = g_wave_manager;

    // Pick which wave will be the next one (we can have multiple descs for a wave number).
    nkArray<nkS32> possible_waves;
    for(nkS32 i=0,n=NK_ARRAY_SIZE(WAVE_LIST); i<n; ++i)
    {
        const WaveDesc& wd = WAVE_LIST[i];
        if(wd.wave_number == (wm.current_wave+1))
        {
            nk_array_append(&possible_waves, i);
        }
    }
    if(nk_array_empty(&possible_waves))
    {
        // If we couldn't find a wave for the wave number just use the last wave in the list.
        nk_array_append(&possible_waves, NK_CAST(nkS32,NK_ARRAY_SIZE(WAVE_LIST)-1));
        wm.wave_multiplier += WAVE_MULTIPLICATION_RATE; // Increase the multiplier to make the wave harder!
    }
    else
    {
        wm.wave_multiplier = 1.0f;
    }

    nkS32 wave_index = rng_s32(0,NK_CAST(nkS32,possible_waves.length)-1);
    wm.wave_desc = &WAVE_LIST[possible_waves[wave_index]];

    // Activate the necessary number of spawn points.
    nkU32 total_spawn_points = wm.wave_desc->spawn_points;
    nkU32 spawn_points_activated = 0;

    while(spawn_points_activated < total_spawn_points)
    {
        nkS32 index = rng_s32(0,NK_ARRAY_SIZE(wm.spawn_points)-1);
        if(!wm.spawn_points[index].active)
        {
            wm.spawn_points[index].active = NK_TRUE;
            spawn_points_activated++;
        }
    }

    // Setup the phase spawners and set them all to wait.
    memset(wm.spawners, 0, sizeof(wm.spawners));

    for(nkU32 i=0; i<wm.wave_desc->num_phases; ++i)
    {
        const PhaseDesc& phase = wm.wave_desc->phases[i];

        nkS32 min_spawns = NK_CAST(nkS32, NK_CAST(nkF32, phase.min_spawns) * wm.wave_multiplier);
        nkS32 max_spawns = NK_CAST(nkS32, NK_CAST(nkF32, phase.max_spawns) * wm.wave_multiplier);

        wm.spawners[i].state       = SpawnerState_Waiting;
        wm.spawners[i].timer       = phase.start_time;
        wm.spawners[i].spawn_types = phase.spawn_types;
        wm.spawners[i].spawns_left = rng_s32(min_spawns, max_spawns);
    }

    // Start the timer until the wave begins.
    wm.timer = wm.wave_desc->prep_timer;

    // Debug print for making sure waves are setting up correctly.
    DEBUG_LOG("[Wave]: Setup wave %d using %d!\n", wm.current_wave+1, wave_index);
}

INTERNAL void start_wave(void)
{
    post_wave_message("GET READY! THEY HAVE ARRIVED...", NK_V3_YELLOW);

    auto& wm = g_wave_manager;

    wm.state = WaveState_Fight;
    wm.current_wave++;

    // Pick one of the battle songs to play.
    Music music[2] = NK_ZERO_MEM;
    music[0] = asset_manager_load<Music>("battle_01.ogg");
    music[1] = asset_manager_load<Music>("battle_02.ogg");
    play_music_fade_in(music[rng_s32() % 2], -1, BATTLE_MUSIC_FADE_IN_TIME);
}

INTERNAL void end_wave(void)
{
    auto& wm = g_wave_manager;

    nkS32 wave_bonus = NK_CAST(nkS32, NK_CAST(nkF32, wm.wave_desc->wave_bonus) * wm.wave_multiplier);

    add_money(wave_bonus);

    wm.state = WaveState_Prepare;
    wm.waves_beaten++;
    wm.wave_desc = NULL;

    for(nkS32 i=0,n=NK_ARRAY_SIZE(wm.spawn_points); i<n; ++i)
    {
        wm.spawn_points[i].active = NK_FALSE;
    }

    setup_next_wave();

    const nkChar* sound = "fanfare_normal.wav";
    nkString message = "WAVE COMPLETE!";
    if(anything_unlocked_this_wave())
    {
        nk_string_append(&message, "\nNEW PLANT/ITEM UNLOCKED!");
        sound = "fanfare_unlock.wav";
    }
    post_wave_message(message.cstr, NK_V3_YELLOW, sound);

    play_music_fade_in(asset_manager_load<Music>("garden.ogg"), -1, BATTLE_MUSIC_FADE_OUT_TIME);
}

INTERNAL void tick_wave_prepare_state(nkF32 dt)
{
    auto& wm = g_wave_manager;

    // Countdown until the next wave will spawn.
    wm.timer -= dt;
    if(wm.timer <= 0.0f)
    {
        start_wave();
    }
}

INTERNAL void tick_wave_fight_state(nkF32 dt)
{
    // Update the wave phase spawners.
    auto& wm = g_wave_manager;

    const WaveDesc& wave_desc = *wm.wave_desc;

    nkU32 phases_complete = 0;

    for(nkU32 i=0; i<wave_desc.num_phases; ++i)
    {
        Spawner& spawner = wm.spawners[i];

        switch(spawner.state)
        {
            // Wait until it is our time to start spawning.
            case SpawnerState_Waiting:
            {
                if(spawner.timer <= 0.0f)
                {
                    spawner.state = SpawnerState_InProgress;
                    DEBUG_LOG("[Wave]: Starting phase %d!\n", i);
                }
                else
                {
                    // If the phase before us is in progress then countdown our waiting timer.
                    // Otherwise, if the phase before us has completed and the player has killed
                    // all the enemies then just start now so they aren't stuck waiting around.
                    if(i > 0 && wm.spawners[i-1].state == SpawnerState_InProgress)
                    {
                        spawner.timer -= dt;
                    }
                    if(i == 0 || (wm.spawners[i-1].state == SpawnerState_Complete && !any_entities_of_type_alive(EntityType_Monster)))
                    {
                        spawner.timer = 0.0f;
                    }
                }
            } break;

            // Spawn monsters.
            case SpawnerState_InProgress:
            {
                // @Incomplete: This could be improved to be a bit nicer for better spawn distribution...
                if(rng_s32(0,100) < 2)
                {
                    nkS32 spawn_index = 0;
                    while(NK_TRUE)
                    {
                        nkS32 index = rng_s32(0,MAX_SPAWNS-1);
                        if(wm.spawn_points[index].active)
                        {
                            spawn_index = index;
                            break;
                        }
                    }

                    // Determine what monster type to spawn based on what is available to us.
                    const SpawnPoint& sp = wm.spawn_points[spawn_index];

                    nkS32 max_percent = 0;

                    if(NK_CHECK_FLAGS(spawner.spawn_types, SpawnType_Walker)) max_percent += get_monster_spawn_rate(SpawnType_Walker);

                    if(max_percent > 0)
                    {
                        nkF32 multiplier = 1.0f;
                        if(max_percent != 100.0f)
                            multiplier = 100.0f / (100.0f - NK_CAST(nkF32, max_percent));
                        nkF32 spawn = NK_CAST(nkF32, rng_s32(1,100));

                        EntityID id = EntityID_None;

                        // =========================================================================
                        // !!!IMPORTANT NOTE!!! Place these in highest to lowest spawn-rate order!!!
                        // =========================================================================
                        //
                        if(spawn < (NK_CAST(nkF32, get_monster_spawn_rate(SpawnType_Walker)) * multiplier)) id = EntityID_Walker;

                        if(id != EntityID_None)
                        {
                            nkF32 x = rng_f32(sp.region.x, sp.region.x + sp.region.w);
                            nkF32 y = rng_f32(sp.region.y, sp.region.y + sp.region.h);

                            entity_spawn(id, x,y);

                            spawner.spawns_left--;
                            if(spawner.spawns_left == 0)
                            {
                                spawner.state = SpawnerState_Complete;
                                DEBUG_LOG("[Wave]: Completing phase %d!\n", i);
                            }
                        }
                        else
                        {
                            DEBUG_LOG("[Wave]: Ended up with no spawn!\n");
                        }
                    }
                }
            } break;

            // We are done.
            case SpawnerState_Complete:
            {
                phases_complete++;
            } break;
        }
    }

    // If all phases are complete and all monsters have been killed
    // then the wave has been completed and can now be finished!!!
    if(phases_complete >= wave_desc.num_phases)
    {
        if(!any_entities_of_type_alive(EntityType_Monster))
        {
            end_wave();
        }
    }
}

GLOBAL void wave_manager_tick(nkF32 dt)
{
    auto& wm = g_wave_manager;

    // Update the current wave manager state.
    switch(wm.state)
    {
        case WaveState_Prepare: tick_wave_prepare_state(dt); break;
        case WaveState_Fight:   tick_wave_fight_state(dt);   break;
    }

    // Update the current message.
    if(!nk_string_empty(&wm.message))
    {
        wm.message_timer -= dt;
        if(wm.message_timer <= 0.0f)
        {
            // Start fading out the message.
            wm.message_alpha -= 0.5f * dt;
            if(wm.message_alpha <= 0.0f)
            {
                nk_string_clear(&wm.message);
            }
        }
    }
}

GLOBAL void wave_manager_draw_world(void)
{
    // Draw the active spawn point markers.
    auto& wm = g_wave_manager;

    Texture hud = asset_manager_load<Texture>("hud.png");

    for(nkS32 i=0,n=NK_ARRAY_SIZE(wm.spawn_points); i<n; ++i)
    {
        const SpawnPoint& sp = wm.spawn_points[i];
        if(sp.active)
        {
            nkVec2 inc = (sp.region.w > sp.region.h) ? nkVec2 { sp.region.w * 0.33f, 0.0f } : nkVec2 { 0.0f, sp.region.h * 0.33f };

            nkF32 x0 = sp.position.x - inc.x;
            nkF32 y0 = sp.position.y - inc.y;
            nkF32 x1 = sp.position.x;
            nkF32 y1 = sp.position.y;
            nkF32 x2 = sp.position.x + inc.x;
            nkF32 y2 = sp.position.y + inc.y;

            // We render a couple of markers just to show that the spawning area is fairly long.
            imm_texture(hud, x0,y0, &HUD_CLIP_MARKER);
            imm_texture(hud, x1,y1, &HUD_CLIP_MARKER);
            imm_texture(hud, x2,y2, &HUD_CLIP_MARKER);
        }
    }
}

GLOBAL void wave_manager_draw_hud(void)
{
    // Draw the current message if there is one.
    auto& wm = g_wave_manager;

    if(nk_string_empty(&wm.message) || is_game_paused()) return;

    nkF32 hud_scale = get_hud_scale();

    TrueTypeFont font = get_font();

    set_truetype_font_size(font, NK_CAST(nkS32, 20 * hud_scale));

    nkF32 ww = NK_CAST(nkF32, get_window_width());

    nkF32 x = (ww - get_truetype_text_width(font, wm.message.cstr)) * 0.5f;
    nkF32 y = (720.0f * 0.25f);

    nkVec4 bg_color = { 0.0f,0.0f,0.0f,wm.message_alpha };
    nkVec4 fg_color;

    fg_color.r = wm.message_color.r;
    fg_color.g = wm.message_color.g;
    fg_color.b = wm.message_color.b;
    fg_color.a = wm.message_alpha;

    draw_truetype_text(font, x+(2*hud_scale),y+(2*hud_scale), wm.message.cstr, bg_color, NK_TRUE);
    draw_truetype_text(font, x,y, wm.message.cstr, fg_color, NK_TRUE);
}

GLOBAL void wave_manager_reset(void)
{
    auto& wm = g_wave_manager;

    // Generate the spawn point locations based on the current world. Shaped like this:
    //
    //                           +--0-----1--+
    //                           |           |
    //                           2           4
    //                           |           |
    //                           3           5
    //                           |           |
    //                           +--6-----7--+
    //
    const nkF32 SPAWN_INSET = 320.0f;

    nkF32 ww = NK_CAST(nkF32, get_world_width()) * TILE_WIDTH;
    nkF32 wh = NK_CAST(nkF32, get_world_height()) * TILE_HEIGHT;

    wm.spawn_points[0] = { { ww * 0.25f,       -SPAWN_INSET }, {      0.0f, -(SPAWN_INSET*3.0f), ww * 0.5f, +(SPAWN_INSET*2.0f) }, NK_FALSE };
    wm.spawn_points[1] = { { ww * 0.75f,       -SPAWN_INSET }, { ww * 0.5f, -(SPAWN_INSET*3.0f), ww * 0.5f, +(SPAWN_INSET*2.0f) }, NK_FALSE };
    wm.spawn_points[2] = { {   -SPAWN_INSET, wh * 0.25f     }, { -(SPAWN_INSET*3.0f),      0.0f, +(SPAWN_INSET*2.0f), wh * 0.5f }, NK_FALSE };
    wm.spawn_points[3] = { {   -SPAWN_INSET, wh * 0.75f     }, { -(SPAWN_INSET*3.0f), wh * 0.5f, +(SPAWN_INSET*2.0f), wh * 0.5f }, NK_FALSE };
    wm.spawn_points[4] = { { ww+SPAWN_INSET, wh * 0.25f     }, { ww+(SPAWN_INSET   ),      0.0f, +(SPAWN_INSET*2.0f), wh * 0.5f }, NK_FALSE };
    wm.spawn_points[5] = { { ww+SPAWN_INSET, wh * 0.75f     }, { ww+(SPAWN_INSET   ), wh * 0.5f, +(SPAWN_INSET*2.0f), wh * 0.5f }, NK_FALSE };
    wm.spawn_points[6] = { { ww * 0.25f,     wh+SPAWN_INSET }, {      0.0f, wh+(SPAWN_INSET   ), ww * 0.5f, +(SPAWN_INSET*2.0f) }, NK_FALSE };
    wm.spawn_points[7] = { { ww * 0.75f,     wh+SPAWN_INSET }, { ww * 0.5f, wh+(SPAWN_INSET   ), ww * 0.5f, +(SPAWN_INSET*2.0f) }, NK_FALSE };

    // Reset the rest of the wave manager state.
    wm.state           = WaveState_Prepare;
    wm.timer           = 0.0f; // Will get set inside setup_next_wave.
    wm.wave_multiplier = 1.0f;
    wm.waves_beaten    = 0;
    wm.current_wave    = 0;
    wm.message_timer   = 0.0f;
    wm.message_alpha   = 1.0f;

    nk_string_clear(&wm.message);

    // Setup the first wave...
    setup_next_wave();
}

GLOBAL nkU32 get_wave_counter(void)
{
    return g_wave_manager.current_wave;
}

GLOBAL nkU32 get_waves_beaten(void)
{
    return g_wave_manager.waves_beaten;
}

/*////////////////////////////////////////////////////////////////////////////*/
