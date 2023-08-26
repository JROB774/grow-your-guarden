#ifndef NK_DEFINE_H__ /*//////////////////////////////////////////////////////*/
#define NK_DEFINE_H__

#include <stdint.h>
#include <stdlib.h>

// Defines for what platform/system we are being compiled for.
#if(defined(WIN32) || defined(_WIN32) || defined(__WIN32__)) || \
   (defined(WIN64) || defined(_WIN64) || defined(__WIN64__)) || \
   (defined(__NT__))
#define NK_OS_WIN32
#endif
#if(defined(__APPLE__) && __APPLE__) || defined(__MACH__)
#define NK_OS_MACOS
#endif
#if(defined(__linux__))
#define NK_OS_LINUX
#endif
#if(defined(__EMSCRIPTEN__))
#define NK_OS_WEB
#endif

// Defines for what compiler we are being built with.
#if defined(_MSC_VER)
#define NK_COMPILER_MSVC
#endif
#if defined(__GNUC__)
#define NK_COMPILER_GCC
#endif
#if defined(__clang__)
#define NK_COMPILER_CLANG
#endif
#if defined(__TINYC__)
#define NK_COMPILER_TCC
#endif

// Determines if we have C++ support and to what extent.
#if defined(__cplusplus)
#define NK_HAS_CPP
#endif
#if defined(NK_HAS_CPP) && (__cplusplus >= 201103L)
#define NK_HAS_CPP11
#endif
#if defined(NK_HAS_CPP) && (__cplusplus >= 201402L)
#define NK_HAS_CPP14
#endif
#if defined(NK_HAS_CPP) && (__cplusplus >= 201703L)
#define NK_HAS_CPP17
#endif
#if defined(NK_HAS_CPP) && (__cplusplus >= 202002L)
#define NK_HAS_CPP20
#endif

// Let the user decide what kind of linkage we have.
#if defined(NK_STATIC)
#define NKAPI static
#else
#define NKAPI extern
#endif

// More descriptive names for the static keyword.
#define NKPERSISTENT static
#define NKINTERNAL static
#define NKGLOBAL static

// Custom inline and force inline keywords.
#define NKINLINE inline

#if defined(NK_COMPILER_MSVC)
#define NKFORCEINLINE __forceinline
#else
#define NKFORCEINLINE inline __attribute__((always_inline))
#endif

// Custom constexpr keyword, if available, otherwise fallback to const.
#if defined(NK_HAS_CPP11)
#define NKCONSTEXPR constexpr
#else
#define NKCONSTEXPR const
#endif

// Macro utilities.

#if defined(NK_HAS_CPP11)
#define NK_STATIC_ASSERT(e,msg) static_assert(e,#msg)
#else
#define NK_STATIC_ASSERT2(pre,post) pre##post
#define NK_STATIC_ASSERT1(pre,post) NK_STATIC_ASSERT2(pre,post)
#define NK_STATIC_ASSERT(e,msg) \
typedef struct { int NK_STATIC_ASSERT1(static_assertion_failed__,msg) : !!(e); } \
    NK_STATIC_ASSERT1(static_assertion_failed_,__COUNTER__)
#endif

// You can redefine NK_ASSERT if you want to use a custom implementation.
#if !defined(NK_ASSERT)
#if defined(NK_DEBUG)
#include <assert.h> // @Todo: Implement our own assertion function instead of using the CRT one.
#define NK_ASSERT(e) assert(e)
#else
#define NK_ASSERT(e) (void)(0)
#endif
#endif

#if defined(NK_HAS_CPP)
#define NK_ZERO_MEM {}
#else
#define NK_ZERO_MEM {0}
#endif

#define NK_STRINGIFY( x) NK_STRINGIFY1(x)
#define NK_STRINGIFY1(x) #x

#define NK_JOIN( a, b) NK_JOIN2(a,b)
#define NK_JOIN2(a, b) NK_JOIN1(a,b)
#define NK_JOIN1(a, b) a##b

#define NK_FOURCC(x) ((((nkU32)(x)&0x000000FF)<<24) | (((nkU32)(x)&0x0000FF00)<< 8) | \
                      (((nkU32)(x)&0x00FF0000)>> 8) | (((nkU32)(x)&0xFF000000)>>24))

#define NK_ENUM(name,type) typedef type name; enum name##_

#define NK_ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

#define NK_CAST(t,x) ((t)(x))

#define NK_UNUSED(x) (NK_CAST(void,(x)))

#define NK_SWAP(t,a,b) do { t tmp__ = a; a = b; b = tmp__; } while(0)

#define NK_KB_TO_BYTES(kb) ((kb)*1024)
#define NK_MB_TO_BYTES(mb) ((mb)*1024*1024)
#define NK_GB_TO_BYTES(gb) ((gb)*1024*1024*1024)
#define NK_TB_TO_BYTES(tb) ((tb)*1024*1024*1024*1024)

#define NK_BYTES_TO_KB(b) ((b)/1024)
#define NK_BYTES_TO_MB(b) ((b)/1024/1024)
#define NK_BYTES_TO_GB(b) ((b)/1024/1024/1024)
#define NK_BYTES_TO_TB(b) ((b)/1024/1024/1024/1024)

#define NK_CHECK_FLAGS( x,f) (((x)&  (f)) != 0)
#define NK_SET_FLAGS(   x,f) (((x)|= (f)))
#define NK_UNSET_FLAGS( x,f) (((x)&=~(f)))
#define NK_TOGGLE_FLAGS(x,f) (((x)^= (f)))

// If we're in C++ then we want to make use of the buil-in boolean type, in C we just implement our own.
#if defined(NK_HAS_CPP)
#define nkBool_  bool
#define NK_FALSE false
#define NK_TRUE  true
#else
#define nkBool_  char
#define NK_FALSE 0
#define NK_TRUE  1
#endif

// You can redefine the basic memory allocation macros nk uses if you don't want to use CRT.
#if !defined(NK_CUSTOM_MEMORY_MACROS)
#define NK_MALLOC_BYTES(    n) malloc (  n)
#define NK_CALLOC_BYTES(    n) calloc (1,n)
#define NK_RALLOC_BYTES(  p,n) realloc(p,n)
#define NK_MALLOC_TYPES(t,  n) NK_CAST(t*,malloc (  n*sizeof(t)))
#define NK_CALLOC_TYPES(t,  n) NK_CAST(t*,calloc (  n,sizeof(t)))
#define NK_RALLOC_TYPES(t,p,n) NK_CAST(t*,realloc(p,n*sizeof(t)))
#define NK_FREE(          p  ) free(p)
#endif

#define NK_U8_MAX  UINT8_MAX
#define NK_U16_MAX UINT16_MAX
#define NK_U32_MAX UINT32_MAX
#define NK_U64_MAX UINT64_MAX
#define NK_S8_MIN  INT8_MIN
#define NK_S16_MIN INT16_MIN
#define NK_S32_MIN INT32_MIN
#define NK_S64_MIN INT64_MIN
#define NK_S8_MAX  INT8_MAX
#define NK_S16_MAX INT16_MAX
#define NK_S32_MAX INT32_MAX
#define NK_S64_MAX INT64_MAX

typedef  nkBool_ nkBool;
typedef     char nkChar;
typedef  uint8_t nkU8;
typedef uint16_t nkU16;
typedef uint32_t nkU32;
typedef uint64_t nkU64;
typedef   int8_t nkS8;
typedef  int16_t nkS16;
typedef  int32_t nkS32;
typedef  int64_t nkS64;
typedef    float nkF32;
typedef   double nkF64;

// Ensure that the types are all the correct size.
NK_STATIC_ASSERT(sizeof(nkBool)==1, nkBool_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkChar)==1, nkChar_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkU8  )==1, nkU8_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkU16 )==2, nkU16_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkU32 )==4, nkU32_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkU64 )==8, nkU64_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkS8  )==1, nkS8_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkS16 )==2, nkS16_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkS32 )==4, nkS32_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkS64 )==8, nkS64_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkF32 )==4, nkF32_is_not_the_correct_size);
NK_STATIC_ASSERT(sizeof(nkF64 )==8, nkF64_is_not_the_correct_size);

// Custom implementation of initialize_list to avoid the need to include extra
// code or STL headers. This needs to be in the std namespace or else it will
// not work, so there is no real way to get around that issue.

#if defined(NK_HAS_CPP) && defined(NK_OS_WIN32) // =============================

#if !defined(_INITIALIZER_LIST_)

namespace std
{

template<typename T>
class initializer_list
{
public:
    initializer_list(const T* first, const T* last): m_first(first), m_last(last) {}

    const T* begin(void) const { return m_first;            }
    const T* end  (void) const { return m_last;             }
    size_t   size (void) const { return (m_last - m_first); }

private:
    const T* m_first = NULL;
    const T* m_last  = NULL;
};

template<typename T> const T* begin(initializer_list<T> list) { return list.begin(); }
template<typename T> const T* end  (initializer_list<T> list) { return list.end();   }

} // std

#endif // !_INITIALIZER_LIST_

template<typename T> using nkInitializerList = std::initializer_list<T>;

#else

#include <initializer_list>

template<typename T> using nkInitializerList = std::initializer_list<T>;

#endif // NK_HAS_CPP && NK_OS_WIN32 ============================================

// Custom implementation of move to avoid the need to include the STL headers.

#if defined(NK_HAS_CPP) // =====================================================

template<typename T> struct nkRemoveReference      { using Type = T; };
template<typename T> struct nkRemoveReference<T&>  { using Type = T; };
template<typename T> struct nkRemoveReference<T&&> { using Type = T; };

template<typename T>
NKCONSTEXPR typename nkRemoveReference<T>::Type&& nk_move(T&& arg)
{
    return (typename nkRemoveReference<T>::Type&&)arg;
}

#endif // NK_HAS_CPP ===========================================================

#endif /* NK_DEFINE_H__ //////////////////////////////////////////////////////*/

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
