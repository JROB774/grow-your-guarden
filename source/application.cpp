/*////////////////////////////////////////////////////////////////////////////*/

#define STB_IMAGE_IMPLEMENTATION
#define NK_FILESYS_IMPLEMENTATION
#define NK_NPAK_IMPLEMENTATION

#define STB_IMAGE_STATIC
#define GLEW_STATIC
#define NK_STATIC

#include <nk_define.h>
#include <nk_math.h>
#include <nk_filesys.h>
#include <nk_npak.h>
#include <nk_array.h>
#include <nk_string.h>
#include <nk_hashmap.h>
#include <nk_hashset.h>
#include <nk_defer.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <wchar.h>

#include <stb_image.h>

static constexpr nkS32 VERSION_MAJOR = 1;
static constexpr nkS32 VERSION_MINOR = 0;
static constexpr nkS32 VERSION_PATCH = 2;

#include "utility.hpp"
#include "asset_manager.hpp"
#include "audio.hpp"
#include "input.hpp"
#include "bitmap_font.hpp"
#include "truetype_font.hpp"
#include "animation.hpp"
#include "renderer.hpp"
#include "imm.hpp"
#include "application.hpp"
#include "platform.hpp"
#include "entity.hpp"
#include "controller.hpp"
#include "world.hpp"
#include "decals.hpp"
#include "particles.hpp"
#include "menu.hpp"
#include "game.hpp"
#include "cursor.hpp"
#include "hud_clips.hpp"
#include "wave_manager.hpp"

#include "utility.cpp"
#include "platform.cpp"
#include "asset_manager.cpp"
#include "audio.cpp"
#include "input.cpp"
#include "bitmap_font.cpp"
#include "truetype_font.cpp"
#include "animation.cpp"
#include "renderer.cpp"
#include "imm.cpp"
#include "entity_table.cpp"
#include "entity.cpp"
#include "entity_ticks.cpp"
#include "controller.cpp"
#include "world.cpp"
#include "decals.cpp"
#include "particles.cpp"
#include "menu.cpp"
#include "game.cpp"
#include "cursor.cpp"
#include "wave_manager.cpp"

struct AppContext
{
    RenderTarget screen_target;
    VertexBuffer screen_buffer;
    Shader       screen_shader;
    TrueTypeFont font;
    AppState     state;
    AppState     state_target;
    nkF32        hud_scale;
};

INTERNAL AppContext g_app;

INTERNAL void begin_frame_draw(void)
{
    nkS32 ww = get_window_width();
    nkS32 wh = get_window_height();

    /*
    nkS32 sw = get_texture_width(g_app.screen_target->color_target);
    nkS32 sh = get_texture_height(g_app.screen_target->color_target);

    if(ww != sw || wh != sh)
        resize_render_target(g_app.screen_target, ww,wh); // The screen target should always fit the window.
    bind_render_target(g_app.screen_target);
    */

    clear_screen(0.0f,0.0f,0.0f);

    set_blend_mode(BlendMode_Alpha);

    nkF32 vw = NK_CAST(nkF32, ww);
    nkF32 vh = NK_CAST(nkF32, wh);

    imm_set_viewport({ 0,0,vw,vh });

    nkMat4 projection = nk_orthographic(0,vw,vh,0);

    imm_set_projection(projection);
    imm_set_view(nk_m4_identity());
}

INTERNAL void end_frame_draw(void)
{
    /*
    RenderTarget output = g_app.screen_target;

    bind_render_target(NULL);

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    set_viewport(0.0f,0.0f,ww,wh);

    clear_screen(0,0,0);

    nkVec4 vertices[4];

    vertices[0] = { 0.0f,  wh, 0.0f,0.0f };
    vertices[1] = { 0.0f,0.0f, 0.0f,1.0f };
    vertices[2] = {   ww,  wh, 1.0f,0.0f };
    vertices[3] = {   ww,0.0f, 1.0f,1.0f };

    nkMat4 projection = nk_orthographic(0,ww,wh,0);

    set_blend_mode(BlendMode_PremultipliedAlpha);

    bind_texture(output->color_target, 0);
    bind_shader(g_app.screen_shader);

    set_shader_mat4(g_app.screen_shader, "u_projection", projection);

    update_vertex_buffer(g_app.screen_buffer, vertices, sizeof(vertices), BufferType_Dynamic);
    draw_vertex_buffer(g_app.screen_buffer, DrawMode_TriangleStrip, NK_ARRAY_SIZE(vertices));
    */
}

GLOBAL void app_main(AppDesc* desc)
{
    desc->title       = "GROW YOUR GUARDEN";
    desc->name        = "GROW";
    desc->window_size = { 1280,720 };
    desc->window_min  = { 1280,720 };
}

GLOBAL void app_init(void)
{
    // Setup the necessary resources for rendering the off-screen target.
    /*
    nkS32 ww = get_window_width();
    nkS32 wh = get_window_height();

    g_app.screen_target = create_render_target(ww,wh, SamplerFilter_Nearest, SamplerWrap_Clamp);
    g_app.screen_shader = asset_manager_load<Shader>("copy.shader");
    g_app.screen_buffer = create_vertex_buffer();

    set_vertex_buffer_stride   (g_app.screen_buffer, sizeof(nkF32)*4);
    enable_vertex_buffer_attrib(g_app.screen_buffer, 0, AttribType_Float, 4, 0);
    */

    // Setup the font.
    TrueTypeFontDesc font_desc;
    font_desc.px_sizes = { 20, 30, 35, 40, 50, 100 };
    g_app.font = asset_manager_load<TrueTypeFont>("helsinki.ttf", &font_desc);

    // Hide the cursor (we have a custom one).
    show_cursor(NK_FALSE);

    // Allow the HUD to scale so that it is clearly visible on large displays.
    g_app.hud_scale = 2.0f;

    g_app.state        = AppState_Menu;
    g_app.state_target = AppState_Menu;

    menu_init();
    game_init();
}

GLOBAL void app_quit(void)
{
    game_quit();
    menu_quit();

    /*
    free_vertex_buffer(g_app.screen_buffer);
    free_render_target(g_app.screen_target);
    */
}

GLOBAL void app_tick(nkF32 dt)
{
    // Defer app changes to here to make sure we never end up with a frame where we tick in one state and draw in the other.
    if(g_app.state != g_app.state_target)
    {
        g_app.state = g_app.state_target;

        switch(g_app.state)
        {
            case AppState_Menu: play_music(asset_manager_load<Music>("menu.ogg")); break;
            case AppState_Game: play_music(asset_manager_load<Music>("garden.ogg")); break;
        }
    }

    set_cursor(CursorType_Pointer); // Always reset the cursor, other systems have to set it to what they want.

    cursor_tick(dt);

    switch(g_app.state)
    {
        case AppState_Menu: menu_tick(dt); break;
        case AppState_Game: game_tick(dt); break;
    }
}

GLOBAL void app_draw(void)
{
    begin_frame_draw();

    switch(g_app.state)
    {
        case AppState_Menu: menu_draw(); break;
        case AppState_Game: game_draw(); break;
    }

    cursor_draw();

    end_frame_draw();
}

GLOBAL void set_app_state(AppState state)
{
    g_app.state_target = state;
}

GLOBAL AppState get_app_state(void)
{
    return g_app.state;
}

GLOBAL nkF32 get_hud_scale(void)
{
    return g_app.hud_scale;
}

GLOBAL TrueTypeFont get_font(void)
{
    return g_app.font;
}

/*////////////////////////////////////////////////////////////////////////////*/
