#ifndef NK_ENDIAN_H__ /*//////////////////////////////////////////////////////*/
#define NK_ENDIAN_H__

#include "nk_define.h"

#define NK_LIL_ENDIAN 1234
#define NK_BIG_ENDIAN 4321

// Determine the endianness of the system. Currently we aren't really supporting
// or running on any big endian systems, so if we don't have access to endian.h
// then we just fall back to determining we are little endian for now. We can
// expand this define check to handle more cases in the future as is necessary.
#if !defined(NK_OS_LINUX)
#define NK_ENDIANNESS NK_LIL_ENDIAN
#else
#include <endian.h>
#define NK_ENDIANNESS __BYTE_ORDER
#endif

// Easy queries for the current endianness.
#define NK_IS_LIL_ENDIAN (NK_ENDIANNESS == NK_LIL_ENDIAN)
#define NK_IS_BIG_ENDIAN (NK_ENDIANNESS == NK_BIG_ENDIAN)

// Macros for swapping to a specific endianness.
#if NK_IS_LIL_ENDIAN
#define NK_LILSWAP16(x) (x)
#define NK_LILSWAP32(x) (x)
#define NK_LILSWAP64(x) (x)
#define NK_BIGSWAP16(x) nk_bswap16(x)
#define NK_BIGSWAP32(x) nk_bswap32(x)
#define NK_BIGSWAP64(x) nk_bswap64(x)
#else
#define NK_LILSWAP16(x) nk_bswap16(x)
#define NK_LILSWAP32(x) nk_bswap32(x)
#define NK_LILSWAP64(x) nk_bswap64(x)
#define NK_BIGSWAP16(x) (x)
#define NK_BIGSWAP32(x) (x)
#define NK_BIGSWAP64(x) (x)
#endif

NKAPI nkU16 nk_bswap16(nkU16 x);
NKAPI nkU32 nk_bswap32(nkU32 x);
NKAPI nkU64 nk_bswap64(nkU64 x);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

#ifdef NK_ENDIAN_IMPLEMENTATION /*////////////////////////////////////////////*/

NKAPI nkU16 nk_bswap16(nkU16 x)
{
   return NK_CAST(nkU16, (x << 8) | (x >> 8));
}

NKAPI nkU32 nk_bswap32(nkU32 x)
{
   return NK_CAST(nkU32, ((x << 24) | ((x << 8) & 0x00FF0000) |
                         ((x >> 8) & 0x0000FF00) | (x >> 24)));
}

NKAPI nkU64 nk_bswap64(nkU64 x)
{
   nkU32 hi;
   nkU32 lo;

   lo = NK_CAST(nkU32, (x & 0xFFFFFFFF));
   x >>= 32;
   hi = NK_CAST(nkU32, (x & 0xFFFFFFFF));

   x  = nk_bswap32(lo);
   x <<= 32;
   x |= nk_bswap32(hi);

   return x;
}

#endif /* NK_ENDIAN_IMPLEMENTATION ///////////////////////////////////////////*/

#endif /* NK_ENDIAN_H__ //////////////////////////////////////////////////////*/

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
