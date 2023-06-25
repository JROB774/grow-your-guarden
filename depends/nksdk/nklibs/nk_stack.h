#ifndef NK_STACK_H__ /*///////////////////////////////////////////////////////*/
#define NK_STACK_H__

#include "nk_define.h"

#if !defined(NK_HAS_CPP)
#error nk_stack requires C++ in order to be used
#endif

template<typename T, nkU64 N>
struct nkStack
{
    T     data[N] = NK_ZERO_MEM;
    nkU64 size    = 0;
};

template<typename T, nkU64 N> NKAPI void   nk_stack_push (nkStack<T,N>* stack, T value);
template<typename T, nkU64 N> NKAPI T      nk_stack_pop  (nkStack<T,N>* stack);
template<typename T, nkU64 N> NKAPI T      nk_stack_peek (nkStack<T,N>* stack);
template<typename T, nkU64 N> NKAPI nkBool nk_stack_empty(nkStack<T,N>* stack);
template<typename T, nkU64 N> NKAPI void   nk_stack_clear(nkStack<T,N>* stack);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

template<typename T, nkU64 N>
NKAPI void nk_stack_push(nkStack<T,N>* stack, T value)
{
    NK_ASSERT(stack);
    NK_ASSERT(stack->size < N);
    stack->buffer[stack->size++] = value;
}

template<typename T, nkU64 N>
NKAPI T nk_stack_pop(nkStack<T,N>* stack)
{
    NK_ASSERT(stack);
    NK_ASSERT(stack->size > 0);
    return stack->buffer[--stack->size];
}

template<typename T, nkU64 N>
NKAPI T nk_stack_peek(nkStack<T,N>* stack)
{
    NK_ASSERT(stack);
    NK_ASSERT(stack->size > 0);
    return stack->buffer[stack->size-1];
}

template<typename T, nkU64 N>
NKAPI nkBool nk_stack_empty(nkStack<T,N>* stack)
{
    NK_ASSERT(stack);
    return (stack->size == 0);
}

template<typename T, nkU64 N>
NKAPI void nk_stack_clear(nkStack<T,N>* stack)
{
    NK_ASSERT(stack);
    stack->size = 0;
}

#endif /* NK_STACK_H__ ///////////////////////////////////////////////////////*/

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
