/*////////////////////////////////////////////////////////////////////////////*/

#define STB_IMAGE_IMPLEMENTATION
#define NK_FILESYS_IMPLEMENTATION
#define NK_NPAK_IMPLEMENTATION

#define STB_IMAGE_STATIC
#define GLEW_STATIC
#define NK_STATIC

#define USE_RENDERER_SIMPLE

#include <nk_define.h>
#include <nk_math.h>
#include <nk_filesys.h>
#include <nk_npak.h>
#include <nk_array.h>
#include <nk_string.h>
#include <nk_hashmap.h>
#include <nk_defer.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <float.h>

#include <stb_image.h>

#include "utility.hpp"
#include "application.hpp"
#include "platform.hpp"
#include "asset_manager.hpp"
#include "audio.hpp"
#include "input.hpp"
#include "bitmap_font.hpp"
#include "truetype_font.hpp"
#include "animation.hpp"
#include "renderer.hpp"
#include "controller.hpp"
#include "plant.hpp"
#include "world.hpp"

#include "utility.cpp"
#include "platform.cpp"
#include "asset_manager.cpp"
#include "audio.cpp"
#include "input.cpp"
#include "bitmap_font.cpp"
#include "truetype_font.cpp"
#include "animation.cpp"
#include "renderer.cpp"
#include "controller.cpp"
#include "plant.cpp"
#include "world.cpp"

struct AppContext
{
    RenderTarget screen_target;
    VertexBuffer screen_buffer;
    Shader       screen_shader;
};

INTERNAL AppContext g_app;

INTERNAL void begin_frame_draw(void)
{
    nkF32 vx = 0.0f;
    nkF32 vy = 0.0f;
    nkF32 vw = NK_CAST(nkF32, get_texture_width(g_app.screen_target->color_target));
    nkF32 vh = NK_CAST(nkF32, get_texture_height(g_app.screen_target->color_target));

    bind_render_target(g_app.screen_target);

    clear_screen(0.2f,0.2f,0.2f);

    set_blend_mode(BlendMode_Alpha);

    imm_set_viewport({ vx,vy,vw,vh });
    imm_set_projection(nk_orthographic(0,vw,vh,0,0,1));
    imm_set_view(nk_m4_identity());
    imm_set_model(nk_m4_identity());
}

INTERNAL void end_frame_draw(void)
{
    RenderTarget output = g_app.screen_target;

    bind_render_target(NULL);

    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    set_viewport(0.0f,0.0f,ww,wh);

    clear_screen(0,0,0);

    nkF32 dstw = SCREEN_WIDTH;
    nkF32 dsth = SCREEN_HEIGHT;

    while((dstw+SCREEN_WIDTH <= ww) && (dsth+SCREEN_HEIGHT <= wh))
    {
        dstw += SCREEN_WIDTH;
        dsth += SCREEN_HEIGHT;
    }

    nkF32 dstx0 = (ww-dstw)*0.5f;
    nkF32 dsty0 = (wh-dsth)*0.5f;
    nkF32 dstx1 = dstx0+dstw;
    nkF32 dsty1 = dsty0+dsth;

    nkVec4 vertices[4];

    vertices[0] = { dstx0,dsty1, 0.0f,0.0f };
    vertices[1] = { dstx0,dsty0, 0.0f,1.0f };
    vertices[2] = { dstx1,dsty1, 1.0f,0.0f };
    vertices[3] = { dstx1,dsty0, 1.0f,1.0f };

    nkMat4 projection = nk_orthographic(0,ww,wh,0,0,1);

    set_blend_mode(BlendMode_PremultipliedAlpha);

    bind_texture(output->color_target, 0);
    bind_shader(g_app.screen_shader);

    set_shader_mat4(g_app.screen_shader, "u_projection", projection);

    update_vertex_buffer(g_app.screen_buffer, vertices, sizeof(vertices), BufferType_Dynamic);
    draw_vertex_buffer(g_app.screen_buffer, DrawMode_TriangleStrip, NK_ARRAY_SIZE(vertices));
}

GLOBAL void app_main(AppDesc* desc)
{
    desc->title       = "PLANT";
    desc->name        = "PLANT";
    desc->window_size = { SCREEN_WIDTH*2,SCREEN_HEIGHT*2 };
    desc->window_min  = { SCREEN_WIDTH,SCREEN_HEIGHT };
}

GLOBAL void app_init(void)
{
    // Setup the necessary resources for rendering the off-screen target.
    g_app.screen_target = create_render_target(SCREEN_WIDTH,SCREEN_HEIGHT, SamplerFilter_Nearest, SamplerWrap_Clamp);
    g_app.screen_shader = asset_manager_load<Shader>("copy.shader");
    g_app.screen_buffer = create_vertex_buffer();

    set_vertex_buffer_stride   (g_app.screen_buffer, sizeof(nkF32)*4);
    enable_vertex_buffer_attrib(g_app.screen_buffer, 0, AttribType_Float, 4, 0);

    world_init();
    controller_init();

    show_cursor(NK_FALSE);
}

GLOBAL void app_quit(void)
{
    world_quit();

    free_vertex_buffer(g_app.screen_buffer);
    free_render_target(g_app.screen_target);
}

GLOBAL void app_tick(nkF32 dt)
{
    controller_tick(dt);
    world_tick(dt);
}

GLOBAL void app_draw(void)
{
    begin_frame_draw();

    set_controller_camera();

    world_draw();
    controller_draw();

    end_frame_draw();
}

/*////////////////////////////////////////////////////////////////////////////*/
