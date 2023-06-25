#ifndef NK_PRINT_H__ /*///////////////////////////////////////////////////////*/
#define NK_PRINT_H__

#include "nk_define.h"

#include <stdarg.h>

NK_ENUM(nkTextColor, nkS32)
{
    nkTextColor_Ignore  = -1, // If you just want to use whatever the current color is.
    nkTextColor_Black   =  0,
    nkTextColor_Red     =  1,
    nkTextColor_Green   =  2,
    nkTextColor_Yellow  =  3,
    nkTextColor_Blue    =  4,
    nkTextColor_Magenta =  5,
    nkTextColor_Cyan    =  6,
    nkTextColor_White   =  7,
    nkTextColor_Default =  9  // The original color of the console before modification.
};

// These two are needed on Windows to allow for ANSI escape codes, colors, etc.
// But on other platforms these are just implemented as empty stub functions.
NKAPI nkBool nk_print_setup    (void);
NKAPI nkBool nk_print_shutdown (void);
NKAPI void   nk_print_colored  (nkTextColor bg, nkTextColor fg, const nkChar* fmt, ...);
NKAPI void   nk_print_colored_v(nkTextColor bg, nkTextColor fg, const nkChar* fmt, va_list args);
NKAPI void   nk_print_reset    (void);
NKAPI void   nk_print_set_bg   (nkTextColor bg);
NKAPI void   nk_print_set_fg   (nkTextColor fg);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

#ifdef NK_PRINT_IMPLEMENTATION /*/////////////////////////////////////////////*/

#if defined(NK_OS_WIN32)
#include <windows.h>
NKINTERNAL DWORD nk__initial_console_mode; // @Improve + @Cleanup: This is gross, handle it in a better way!
#endif // NK_OS_WIN32

#include <stdio.h>

NKAPI nkBool nk_print_setup(void)
{
    #if defined(NK_OS_WIN32)

    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(stdout_handle == INVALID_HANDLE_VALUE) return NK_FALSE;

    // Enable the ability to use ANSI codes in the console. We also cache
    // the initial mode so we can set the console back to that on shutdown.
    DWORD console_mode;
    if(!GetConsoleMode(stdout_handle, &console_mode))
        return NK_FALSE;
    nk__initial_console_mode = console_mode;
    NK_SET_FLAGS(console_mode, ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    if(!SetConsoleMode(stdout_handle, console_mode))
        return NK_FALSE;

    #endif // NK_OS_WIN32

    return NK_TRUE;
}

NKAPI nkBool nk_print_shutdown(void)
{
    #if defined(NK_OS_WIN32)

    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(stdout_handle == INVALID_HANDLE_VALUE) return NK_FALSE;

    // Clear any graphical changes that were applied and reset the mode.
    printf("\x1b[0m");
    SetConsoleMode(stdout_handle, nk__initial_console_mode);

    #endif // NK_OS_WIN32

    return NK_TRUE;
}

NKAPI void nk_print_colored(nkTextColor bg, nkTextColor fg, const nkChar* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nk_print_colored_v(bg,fg, fmt, args);
    va_end(args);
}

NKAPI void nk_print_colored_v(nkTextColor bg, nkTextColor fg, const nkChar* fmt, va_list args)
{
    nk_print_set_bg(bg);
    nk_print_set_fg(fg);
    vprintf(fmt, args);
}

NKAPI void nk_print_reset(void)
{
    printf("\x1b[0m");
}

NKAPI void nk_print_set_bg(nkTextColor bg)
{
    if(bg != nkTextColor_Ignore) printf("\x1b[4%dm", bg);
}

NKAPI void nk_print_set_fg(nkTextColor fg)
{
    if(fg != nkTextColor_Ignore) printf("\x1b[3%dm", fg);
}

#endif /* NK_PRINT_IMPLEMENTATION ////////////////////////////////////////////*/

#endif /* NK_PRINT_H__ ///////////////////////////////////////////////////////*/

/*******************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2023 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
