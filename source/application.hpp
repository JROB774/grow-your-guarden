/*////////////////////////////////////////////////////////////////////////////*/

struct AppDesc
{
    const nkChar* title       = "Template Game"; // Display name of the game (shown in the title bar).
    const nkChar* name        = "template";      // Internal name of the game (alphanumeric characters only).
    iPoint        window_size = { 1280,720 };    // Starting size of the game window.
    iPoint        window_min  = { 1280,720 };    // Minimum size of the game window.
    nkF32         tick_rate   = 60.0f;           // How many times to run the tick callback each second.
};

GLOBAL void app_main(AppDesc* desc);
GLOBAL void app_init(void);
GLOBAL void app_quit(void);
GLOBAL void app_tick(nkF32 dt);
GLOBAL void app_draw(void);

/*////////////////////////////////////////////////////////////////////////////*/
