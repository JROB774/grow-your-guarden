/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(KeyCode, nkS32)
{
    KeyCode_Invalid,
    KeyCode_F1, KeyCode_F2, KeyCode_F3, KeyCode_F4, KeyCode_F5, KeyCode_F6,
    KeyCode_F7, KeyCode_F8, KeyCode_F9, KeyCode_F10, KeyCode_F11, KeyCode_F12,
    KeyCode_0, KeyCode_1, KeyCode_2, KeyCode_3, KeyCode_4,
    KeyCode_5, KeyCode_6, KeyCode_7, KeyCode_8, KeyCode_9,
    KeyCode_Q, KeyCode_W, KeyCode_E, KeyCode_R, KeyCode_T, KeyCode_Y, KeyCode_U,
    KeyCode_I, KeyCode_O, KeyCode_P, KeyCode_A, KeyCode_S, KeyCode_D, KeyCode_F,
    KeyCode_G, KeyCode_H, KeyCode_J, KeyCode_K, KeyCode_L, KeyCode_Z, KeyCode_X,
    KeyCode_C, KeyCode_V, KeyCode_B, KeyCode_N, KeyCode_M,
    KeyCode_Escape,
    KeyCode_Tab,
    KeyCode_CapsLock,
    KeyCode_Backspace,
    KeyCode_Delete,
    KeyCode_Home,
    KeyCode_End,
    KeyCode_Enter,
    KeyCode_Space,
    KeyCode_Grave,
    KeyCode_Up, KeyCode_Right, KeyCode_Down, KeyCode_Left,
    KeyCode_LeftShift, KeyCode_RightShift,
    KeyCode_LeftCtrl, KeyCode_RightCtrl,
    KeyCode_LeftAlt, KeyCode_RightAlt,
    KeyCode_TOTAL
};

NK_ENUM(MouseButton, nkS32)
{
    MouseButton_Invalid,
    MouseButton_Left,
    MouseButton_Middle,
    MouseButton_Right,
    MouseButton_TOTAL
};

NK_ENUM(GamepadButton, nkS32)
{
    GamepadButton_Invalid,
    GamepadButton_DPadUp, GamepadButton_DPadRight, GamepadButton_DPadDown, GamepadButton_DPadLeft,
    GamepadButton_A, GamepadButton_B, GamepadButton_X, GamepadButton_Y,
    GamepadButton_LeftStick, GamepadButton_RightStick,
    GamepadButton_LeftShoulder, GamepadButton_RightShoulder,
    GamepadButton_Select,
    GamepadButton_Start,
    GamepadButton_TOTAL
};

NK_ENUM(GamepadAxis, nkS32)
{
    GamepadAxis_Invalid,
    GamepadAxis_LeftStickX, GamepadAxis_RightStickX,
    GamepadAxis_LeftStickY, GamepadAxis_RightStickY,
    GamepadAxis_LeftTrigger, GamepadAxis_RightTrigger,
    GamepadAxis_TOTAL
};

GLOBAL void init_input_system   (void);
GLOBAL void quit_input_system   (void);
GLOBAL void process_input_events(void* event);
GLOBAL void update_input_state  (void);
GLOBAL void reset_input_state   (void);

// Text Input
GLOBAL nkChar* get_current_text_input(void);

// Keyboard
GLOBAL nkBool is_key_down        (KeyCode code);
GLOBAL nkBool is_key_up          (KeyCode code);
GLOBAL nkBool is_key_pressed     (KeyCode code);
GLOBAL nkBool is_key_released    (KeyCode code);
GLOBAL nkBool is_any_key_down    (void);
GLOBAL nkBool is_any_key_up      (void);
GLOBAL nkBool is_any_key_pressed (void);
GLOBAL nkBool is_any_key_released(void);
GLOBAL nkBool is_key_mod_active  (void); // Are any of the key modifiers pressed Ctrl/Shift/Alt.

// Mouse
GLOBAL nkBool is_mouse_button_down        (MouseButton button);
GLOBAL nkBool is_mouse_button_up          (MouseButton button);
GLOBAL nkBool is_mouse_button_pressed     (MouseButton button);
GLOBAL nkBool is_mouse_button_released    (MouseButton button);
GLOBAL nkBool is_any_mouse_button_down    (void);
GLOBAL nkBool is_any_mouse_button_up      (void);
GLOBAL nkBool is_any_mouse_button_pressed (void);
GLOBAL nkBool is_any_mouse_button_released(void);
GLOBAL void   set_mouse_to_relative       (nkBool enable);
GLOBAL nkBool is_mouse_relative           (void);
GLOBAL nkVec2 get_window_mouse_pos        (void);
GLOBAL nkVec2 get_relative_mouse_pos      (void);
GLOBAL nkVec2 get_mouse_wheel             (void);

// Gamepad
GLOBAL nkBool is_gamepad_button_down         (GamepadButton button);
GLOBAL nkBool is_gamepad_button_up           (GamepadButton button);
GLOBAL nkBool is_gamepad_button_pressed      (GamepadButton button);
GLOBAL nkBool is_gamepad_button_released     (GamepadButton button);
GLOBAL nkBool is_any_gamepad_button_down     (void);
GLOBAL nkBool is_any_gamepad_button_up       (void);
GLOBAL nkBool is_any_gamepad_button_pressed  (void);
GLOBAL nkBool is_any_gamepad_button_released (void);
GLOBAL nkS16  get_gamepad_axis               (GamepadAxis axis);
GLOBAL nkBool is_gamepad_rstick_up           (void);
GLOBAL nkBool is_gamepad_rstick_right        (void);
GLOBAL nkBool is_gamepad_rstick_down         (void);
GLOBAL nkBool is_gamepad_rstick_left         (void);
GLOBAL nkBool is_gamepad_rstick_pressed_up   (void);
GLOBAL nkBool is_gamepad_rstick_pressed_right(void);
GLOBAL nkBool is_gamepad_rstick_pressed_down (void);
GLOBAL nkBool is_gamepad_rstick_pressed_left (void);
GLOBAL nkBool is_gamepad_lstick_up           (void);
GLOBAL nkBool is_gamepad_lstick_right        (void);
GLOBAL nkBool is_gamepad_lstick_down         (void);
GLOBAL nkBool is_gamepad_lstick_left         (void);
GLOBAL nkBool is_gamepad_lstick_pressed_up   (void);
GLOBAL nkBool is_gamepad_lstick_pressed_right(void);
GLOBAL nkBool is_gamepad_lstick_pressed_down (void);
GLOBAL nkBool is_gamepad_lstick_pressed_left (void);

/*////////////////////////////////////////////////////////////////////////////*/
