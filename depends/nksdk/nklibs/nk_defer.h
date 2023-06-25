#ifndef NK_DEFER_H__ /*///////////////////////////////////////////////////////*/
#define NK_DEFER_H__

#include "nk_define.h"

#if !defined(NK_HAS_CPP11)
#error nk_defer requires C++11 in order to be used
#endif

#define NK_DEFER(...) auto NK_JOIN(nk_defer_,__LINE__) = nk_make_defer([&](){__VA_ARGS__;}); NK_UNUSED(NK_JOIN(nk_defer_,__LINE__))

template<typename T>
struct nkDefer
{
    T f;

    nkDefer(T f): f(f) {}
   ~nkDefer() { f(); }
};

template<typename T>
nkDefer<T> nk_make_defer(T f)
{
    return nkDefer<T>(f);
}

#endif /* NK_DEFER_H__ ///////////////////////////////////////////////////////*/

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
