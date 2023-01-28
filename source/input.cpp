/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkS16 INPUT_GAMEPAD_STICK_DEADZONE = 12000;

INTERNAL const SDL_Scancode KEYCODE_TO_SDL[] =
{
    SDL_SCANCODE_UNKNOWN,
    SDL_SCANCODE_F1, SDL_SCANCODE_F2, SDL_SCANCODE_F3, SDL_SCANCODE_F4, SDL_SCANCODE_F5, SDL_SCANCODE_F6,
    SDL_SCANCODE_F7, SDL_SCANCODE_F8, SDL_SCANCODE_F9, SDL_SCANCODE_F10, SDL_SCANCODE_F11, SDL_SCANCODE_F12,
    SDL_SCANCODE_0, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
    SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R, SDL_SCANCODE_T, SDL_SCANCODE_Y, SDL_SCANCODE_U,
    SDL_SCANCODE_I, SDL_SCANCODE_O, SDL_SCANCODE_P, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
    SDL_SCANCODE_G, SDL_SCANCODE_H, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L, SDL_SCANCODE_Z, SDL_SCANCODE_X,
    SDL_SCANCODE_C, SDL_SCANCODE_V, SDL_SCANCODE_B, SDL_SCANCODE_N, SDL_SCANCODE_M,
    SDL_SCANCODE_ESCAPE,
    SDL_SCANCODE_TAB,
    SDL_SCANCODE_CAPSLOCK,
    SDL_SCANCODE_BACKSPACE,
    SDL_SCANCODE_DELETE,
    SDL_SCANCODE_HOME,
    SDL_SCANCODE_END,
    SDL_SCANCODE_RETURN,
    SDL_SCANCODE_SPACE,
    SDL_SCANCODE_GRAVE,
    SDL_SCANCODE_UP, SDL_SCANCODE_RIGHT, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT,
    SDL_SCANCODE_LSHIFT, SDL_SCANCODE_RSHIFT,
    SDL_SCANCODE_LCTRL, SDL_SCANCODE_RCTRL,
    SDL_SCANCODE_LALT, SDL_SCANCODE_RALT
};

INTERNAL const nkS32 MOUSE_BUTTON_TO_SDL[] =
{
    0, SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT
};

INTERNAL const SDL_GameControllerButton GAMEPAD_BUTTON_TO_SDL[] =
{
    SDL_CONTROLLER_BUTTON_INVALID,
    SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_START,
};

INTERNAL const SDL_GameControllerAxis GAMEPAD_AXIS_TO_SDL[] =
{
    SDL_CONTROLLER_AXIS_INVALID,
    SDL_CONTROLLER_AXIS_LEFTX, SDL_CONTROLLER_AXIS_RIGHTX,
    SDL_CONTROLLER_AXIS_LEFTY, SDL_CONTROLLER_AXIS_RIGHTY,
    SDL_CONTROLLER_AXIS_TRIGGERLEFT, SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(       KEYCODE_TO_SDL) ==       KeyCode_TOTAL,        keycode_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(  MOUSE_BUTTON_TO_SDL) ==   MouseButton_TOTAL,   mouse_button_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(GAMEPAD_BUTTON_TO_SDL) == GamepadButton_TOTAL, gameapd_button_size_mismatch);
NK_STATIC_ASSERT(NK_ARRAY_SIZE(  GAMEPAD_AXIS_TO_SDL) ==   GamepadAxis_TOTAL,   gameapd_axis_size_mismatch);

struct InputState
{
    SDL_GameController* gamepad;

    nkBool prev_key_state[KeyCode_TOTAL];
    nkBool curr_key_state[KeyCode_TOTAL];
    nkBool prev_mouse_state[MouseButton_TOTAL];
    nkBool curr_mouse_state[MouseButton_TOTAL];
    nkBool prev_pad_state[GamepadButton_TOTAL];
    nkBool curr_pad_state[GamepadButton_TOTAL];
    nkS16  prev_axis_state[GamepadAxis_TOTAL];
    nkS16  curr_axis_state[GamepadAxis_TOTAL];
    nkVec2 mouse_pos;
    nkVec2 mouse_pos_relative;
    nkChar text_input[256];
};

INTERNAL InputState g_input;

INTERNAL void remove_gamepad(void)
{
    if(g_input.gamepad)
    {
        SDL_GameControllerClose(g_input.gamepad);
        g_input.gamepad = NULL;
    }
}

INTERNAL void add_gamepad(void)
{
    // Search for a plugged in gamepad and if there is one use it.
    remove_gamepad();
    for(nkS32 i=0,n=SDL_NumJoysticks(); i<n; ++i)
    {
        if(SDL_IsGameController(i))
        {
            g_input.gamepad = SDL_GameControllerOpen(i);
            if(g_input.gamepad)
            {
                break;
            }
        }
    }
}

GLOBAL void init_input_system(void)
{
    // Does nothing...
}

GLOBAL void quit_input_system(void)
{
    remove_gamepad();
}

GLOBAL void process_input_events(void* event)
{
    SDL_Event* sdl_event = NK_CAST(SDL_Event*, event);
    switch(sdl_event->type)
    {
        case SDL_CONTROLLERDEVICEADDED: add_gamepad(); break;
        case SDL_CONTROLLERDEVICEREMOVED: remove_gamepad(); break;

        case SDL_TEXTINPUT:
        {
            nkU32 new_length = NK_CAST(nkU32, strlen(g_input.text_input) + strlen(sdl_event->text.text));
            if(new_length < NK_ARRAY_SIZE(g_input.text_input))
            {
                strcat(g_input.text_input, sdl_event->text.text);
            }
        } break;
    }
}

GLOBAL void update_input_state()
{
    // Update the keyboard state.
    memcpy(g_input.prev_key_state, g_input.curr_key_state, sizeof(g_input.prev_key_state));
    const nkU8* sdl_keyboard_state = SDL_GetKeyboardState(NULL);
    for(nkS32 i=0; i<KeyCode_TOTAL; ++i)
        g_input.curr_key_state[i] = (sdl_keyboard_state[KEYCODE_TO_SDL[NK_CAST(KeyCode, i)]] != 0);

    // Update the mouse state.
    nkS32 pmx,pmy;
    nkS32 rmx,rmy;

    SDL_GetMouseState(&pmx,&pmy);
    SDL_GetRelativeMouseState(&rmx,&rmy);

    g_input.mouse_pos         .x = NK_CAST(nkF32, pmx);
    g_input.mouse_pos         .y = NK_CAST(nkF32, pmy);
    g_input.mouse_pos_relative.x = NK_CAST(nkF32, rmx);
    g_input.mouse_pos_relative.y = NK_CAST(nkF32, rmy);

    // Update the mouse button state.
    nkU32 sdl_mouse = SDL_GetMouseState(NULL,NULL);
    memcpy(g_input.prev_mouse_state, g_input.curr_mouse_state, sizeof(g_input.prev_mouse_state));
    for(nkS32 i=0; i<MouseButton_TOTAL; ++i)
        g_input.curr_mouse_state[i] = (NK_CHECK_FLAGS(sdl_mouse, SDL_BUTTON(MOUSE_BUTTON_TO_SDL[NK_CAST(MouseButton, i)])) != 0);

    // Update the gamepad.
    if(g_input.gamepad)
    {
        // Update the gamepad button state.
        memcpy(g_input.prev_pad_state, g_input.curr_pad_state, sizeof(g_input.prev_pad_state));
        for(nkS32 i=0; i<GamepadButton_TOTAL; ++i)
            g_input.curr_pad_state[i] = (SDL_GameControllerGetButton(g_input.gamepad, GAMEPAD_BUTTON_TO_SDL[i]) != 0);

        // Update the gamepad axis state.
        memcpy(g_input.prev_axis_state, g_input.curr_axis_state, sizeof(g_input.prev_axis_state));
        for(nkS32 i=0; i<GamepadAxis_TOTAL; ++i)
            g_input.curr_axis_state[i] = SDL_GameControllerGetAxis(g_input.gamepad, GAMEPAD_AXIS_TO_SDL[i]);
    }
    else
    {
        // If a gamepad isn't connected just report everything as not pressed/active.
        memset(g_input.prev_pad_state, 0, sizeof(g_input.prev_pad_state));
        memset(g_input.prev_axis_state, 0, sizeof(g_input.prev_axis_state));
        memset(g_input.curr_pad_state, 0, sizeof(g_input.curr_pad_state));
        memset(g_input.curr_axis_state, 0, sizeof(g_input.curr_axis_state));
    }
}

GLOBAL void reset_input_state(void)
{
    memset(g_input.text_input, 0, sizeof(g_input.text_input));
}

//
// Text Input
//

GLOBAL nkChar* get_current_text_input(void)
{
    return g_input.text_input;
}

//
// Keyboard
//

GLOBAL nkBool is_key_down(KeyCode code)
{
    if(code == KeyCode_Invalid) return NK_FALSE;
    return (g_input.curr_key_state[code] != 0);
}

GLOBAL nkBool is_key_up(KeyCode code)
{
    if(code == KeyCode_Invalid) return NK_FALSE;
    return (g_input.curr_key_state[code] == 0);
}

GLOBAL nkBool is_key_pressed(KeyCode code)
{
    if(code == KeyCode_Invalid) return NK_FALSE;
    return (g_input.curr_key_state[code] != 0 &&
            g_input.prev_key_state[code] == 0);
}

GLOBAL nkBool is_key_released(KeyCode code)
{
    if(code == KeyCode_Invalid) return NK_FALSE;
    return (g_input.curr_key_state[code] == 0 &&
            g_input.prev_key_state[code] != 0);
}

GLOBAL nkBool is_any_key_down(void)
{
    for(nkS32 i=0; i<NK_CAST(nkS32, KeyCode_TOTAL); ++i)
        if(is_key_down(NK_CAST(KeyCode, i))) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_any_key_up(void)
{
    for(nkS32 i=0; i<NK_CAST(nkS32, KeyCode_TOTAL); ++i)
        if(is_key_up(NK_CAST(KeyCode, i))) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_any_key_pressed(void)
{
    for(nkS32 i=0; i<NK_CAST(nkS32, KeyCode_TOTAL); ++i)
        if(is_key_pressed(NK_CAST(KeyCode, i))) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_any_key_released(void)
{
    for(nkS32 i=0; i<NK_CAST(nkS32, KeyCode_TOTAL); ++i)
        if(is_key_released(NK_CAST(KeyCode, i))) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_key_mod_active(void)
{
    return (is_key_down(KeyCode_LeftShift) || is_key_down(KeyCode_RightShift) ||
            is_key_down(KeyCode_LeftCtrl) || is_key_down(KeyCode_RightCtrl) ||
            is_key_down(KeyCode_LeftAlt) || is_key_down(KeyCode_RightAlt));
}

//
// Mouse
//

GLOBAL nkBool is_mouse_button_down(MouseButton button)
{
    if(button == MouseButton_Invalid) return NK_FALSE;
    return (g_input.curr_mouse_state[button] != 0);
}

GLOBAL nkBool is_mouse_button_up(MouseButton button)
{
    if(button == MouseButton_Invalid) return NK_FALSE;
    return (g_input.curr_mouse_state[button] == 0);
}

GLOBAL nkBool is_mouse_button_pressed(MouseButton button)
{
    if(button == MouseButton_Invalid) return NK_FALSE;
    return (g_input.curr_mouse_state[button] != 0 &&
            g_input.prev_mouse_state[button] == 0);
}

GLOBAL nkBool is_mouse_button_released(MouseButton button)
{
    if(button == MouseButton_Invalid) return NK_FALSE;
    return (g_input.curr_mouse_state[button] == 0 &&
            g_input.prev_mouse_state[button] != 0);
}

GLOBAL nkBool is_any_mouse_button_down(void)
{
    for(nkS32 i=0; i<NK_CAST(nkS32, MouseButton_TOTAL); ++i)
        if(is_mouse_button_down(NK_CAST(MouseButton, i))) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_any_mouse_button_up(void)
{
    for(nkS32 i=0; i<NK_CAST(nkS32, MouseButton_TOTAL); ++i)
        if(is_mouse_button_up(NK_CAST(MouseButton, i))) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_any_mouse_button_pressed(void)
{
    for(nkS32 i=0; i<NK_CAST(nkS32, MouseButton_TOTAL); ++i)
        if(is_mouse_button_pressed(NK_CAST(MouseButton, i))) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_any_mouse_button_released(void)
{
    for(nkS32 i=0; i<NK_CAST(nkS32, MouseButton_TOTAL); ++i)
        if(is_mouse_button_released(NK_CAST(MouseButton, i))) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL void set_mouse_to_relative(nkBool enable)
{
    SDL_SetRelativeMouseMode(NK_CAST(SDL_bool,enable));
}

GLOBAL nkBool is_mouse_relative(void)
{
    return NK_CAST(nkBool, SDL_GetRelativeMouseMode());
}

GLOBAL nkVec2 get_window_mouse_pos(void)
{
    return g_input.mouse_pos;
}

GLOBAL nkVec2 get_relative_mouse_pos(void)
{
    return g_input.mouse_pos_relative;
}

//
// Gamepad
//

GLOBAL nkBool is_gamepad_button_down(GamepadButton button)
{
    if(button == GamepadButton_Invalid) return NK_FALSE;
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.curr_pad_state[button] != 0);
}

GLOBAL nkBool is_gamepad_button_up(GamepadButton button)
{
    if(button == GamepadButton_Invalid) return NK_FALSE;
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.curr_pad_state[button] == 0);
}

GLOBAL nkBool is_gamepad_button_pressed(GamepadButton button)
{
    if(button == GamepadButton_Invalid) return NK_FALSE;
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.curr_pad_state[button] != 0 &&
            g_input.prev_pad_state[button] == 0);
}

GLOBAL nkBool is_gamepad_button_released(GamepadButton button)
{
    if(button == GamepadButton_Invalid) return NK_FALSE;
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.curr_pad_state[button] == 0 &&
            g_input.prev_pad_state[button] != 0);
}

GLOBAL nkBool is_any_gamepad_button_down(void)
{
    if(g_input.gamepad)
        for(nkS32 i=0; i<GamepadButton_TOTAL; ++i)
            if(is_gamepad_button_down(i)) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_any_gamepad_button_up(void)
{
    if(g_input.gamepad)
        for(nkS32 i=0; i<GamepadButton_TOTAL; ++i)
            if(is_gamepad_button_up(i)) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_any_gamepad_button_pressed(void)
{
    if(g_input.gamepad)
        for(nkS32 i=0; i<GamepadButton_TOTAL; ++i)
            if(is_gamepad_button_pressed(i)) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkBool is_any_gamepad_button_released(void)
{
    if(g_input.gamepad)
        for(nkS32 i=0; i<GamepadButton_TOTAL; ++i)
            if(is_gamepad_button_released(i)) return NK_TRUE;
    return NK_FALSE;
}

GLOBAL nkS16 get_gamepad_axis(GamepadAxis axis)
{
    if(axis == GamepadAxis_Invalid) return 0;
    if(!g_input.gamepad) return 0;
    return g_input.curr_axis_state[axis];
}

GLOBAL nkBool is_gamepad_rstick_up(void)
{
    return (get_gamepad_axis(GamepadAxis_RightStickY) < -INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_rstick_right(void)
{
    return (get_gamepad_axis(GamepadAxis_RightStickX) > INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_rstick_down(void)
{
    return (get_gamepad_axis(GamepadAxis_RightStickY) > INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_rstick_left(void)
{
    return (get_gamepad_axis(GamepadAxis_RightStickX) < -INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_rstick_pressed_up(void)
{
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.prev_axis_state[GamepadAxis_RightStickY] >= -INPUT_GAMEPAD_STICK_DEADZONE &&
            g_input.curr_axis_state[GamepadAxis_RightStickY] <  -INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_rstick_pressed_right(void)
{
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.prev_axis_state[GamepadAxis_RightStickX] <= INPUT_GAMEPAD_STICK_DEADZONE &&
            g_input.curr_axis_state[GamepadAxis_RightStickX] >  INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_rstick_pressed_down(void)
{
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.prev_axis_state[GamepadAxis_RightStickY] <= INPUT_GAMEPAD_STICK_DEADZONE &&
            g_input.curr_axis_state[GamepadAxis_RightStickY] >  INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_rstick_pressed_left(void)
{
    if(!g_input.gamepad) return false;
    return (g_input.prev_axis_state[GamepadAxis_RightStickX] >= -INPUT_GAMEPAD_STICK_DEADZONE &&
            g_input.curr_axis_state[GamepadAxis_RightStickX] <  -INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_lstick_up(void)
{
    return (get_gamepad_axis(GamepadAxis_LeftStickY) < -INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_lstick_right(void)
{
    return (get_gamepad_axis(GamepadAxis_LeftStickX) > INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_lstick_down(void)
{
    return (get_gamepad_axis(GamepadAxis_LeftStickY) > INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_lstick_left(void)
{
    return (get_gamepad_axis(GamepadAxis_LeftStickX) < -INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_lstick_pressed_up(void)
{
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.prev_axis_state[GamepadAxis_LeftStickY] >= -INPUT_GAMEPAD_STICK_DEADZONE &&
            g_input.curr_axis_state[GamepadAxis_LeftStickY] <  -INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_lstick_pressed_right(void)
{
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.prev_axis_state[GamepadAxis_LeftStickX] <= INPUT_GAMEPAD_STICK_DEADZONE &&
            g_input.curr_axis_state[GamepadAxis_LeftStickX] >  INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_lstick_pressed_down(void)
{
    if(!g_input.gamepad) return NK_FALSE;
    return (g_input.prev_axis_state[GamepadAxis_LeftStickY] <= INPUT_GAMEPAD_STICK_DEADZONE &&
            g_input.curr_axis_state[GamepadAxis_LeftStickY] >  INPUT_GAMEPAD_STICK_DEADZONE);
}

GLOBAL nkBool is_gamepad_lstick_pressed_left(void)
{
    if(!g_input.gamepad) return false;
    return (g_input.prev_axis_state[GamepadAxis_LeftStickX] >= -INPUT_GAMEPAD_STICK_DEADZONE &&
            g_input.curr_axis_state[GamepadAxis_LeftStickX] <  -INPUT_GAMEPAD_STICK_DEADZONE);
}

/*////////////////////////////////////////////////////////////////////////////*/
