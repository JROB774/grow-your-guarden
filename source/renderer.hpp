/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL void init_render_system     (void);
GLOBAL void quit_render_system     (void);
GLOBAL void setup_renderer_platform(void);
GLOBAL void do_render_frame        (void);

// Pick which rendering backend you want to use with the following defines.
// There are a couple of different renderers, each with varying levels of
// hardware demands which you can opt into depending on what you need.

#if defined(USE_RENDERER_ADVANCED)
#include "renderer_advanced.hpp"
#endif // USE_RENDERER_ADVANCED
#if defined(USE_RENDERER_SIMPLE)
#include "renderer_simple.hpp"
#endif // USE_RENDERER_SIMPLE

/*////////////////////////////////////////////////////////////////////////////*/
