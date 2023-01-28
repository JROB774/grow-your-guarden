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

#include "utility.cpp"
#include "platform.cpp"
#include "asset_manager.cpp"
#include "audio.cpp"
#include "input.cpp"
#include "bitmap_font.cpp"
#include "truetype_font.cpp"
#include "animation.cpp"
#include "renderer.cpp"

GLOBAL void app_main(AppDesc* desc)
{
    // Nothing...
}

GLOBAL void app_init(void)
{
    // Nothing...
}

GLOBAL void app_quit(void)
{
    // Nothing...
}

GLOBAL void app_tick(nkF32 dt)
{
    // Nothing...
}

GLOBAL void app_draw(void)
{
    // Nothing...
}

/*////////////////////////////////////////////////////////////////////////////*/
