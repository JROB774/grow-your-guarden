/*////////////////////////////////////////////////////////////////////////////*/

GLOBAL nkChar* get_base_path     (void);
GLOBAL void*   get_window        (void);
GLOBAL void*   get_context       (void);
GLOBAL void    terminate_app     (void);
GLOBAL void    fatal_error       (const nkChar* fmt, ...);
GLOBAL void    user_error        (const nkChar* fmt, ...);
GLOBAL iPoint  get_window_size   (void);
GLOBAL nkS32   get_window_width  (void);
GLOBAL nkS32   get_window_height (void);
GLOBAL void    set_fullscreen    (nkBool enable);
GLOBAL nkBool  is_fullscreen     (void);
GLOBAL void    show_cursor       (nkBool show);
GLOBAL nkBool  is_cursor_visible (void);
GLOBAL nkU64   get_system_time_ms(void);
GLOBAL nkU64   get_system_time_us(void);

/*////////////////////////////////////////////////////////////////////////////*/
