#ifndef NK_STRING_H__ /*//////////////////////////////////////////////////////*/
#define NK_STRING_H__

#include "nk_define.h"

#include <string.h>
#include <ctype.h>

#if !defined(NK_HAS_CPP)
#error nk_string requires C++ in order to be used
#endif

struct nkString
{
    nkU64 length    = 0;
    nkU64 allocated = 0;

    nkChar* cstr = NULL;

    nkString(void);
    nkString(const nkString& other);
    nkString(nkString&& other);
    nkString(const nkChar* str);
    nkString(const nkChar* str, nkU64 length);
    nkString(nkChar c);

   ~nkString(void);

    nkString& operator=(const nkString& other);
    nkString& operator=(nkString&& other);
    nkString& operator=(const nkChar* str);

    const nkChar& operator[](size_t idx) const;
          nkChar& operator[](size_t idx);

    // These functions exist to make the container work in range-based for loops.
    const nkChar* begin(void) const;
          nkChar* begin(void);
    const nkChar* end  (void) const;
          nkChar* end  (void);
};

NKAPI NKINLINE void     nk_string_free   (      nkString* string);
NKAPI NKINLINE void     nk_string_reserve(      nkString* string, nkU64 size);
NKAPI NKINLINE void     nk_string_clear  (      nkString* string);
NKAPI NKINLINE nkChar   nk_string_last   (      nkString* string);
NKAPI NKINLINE nkBool   nk_string_empty  (      nkString* string);
NKAPI NKINLINE nkString nk_string_create (                                   const nkString* other);
NKAPI NKINLINE nkString nk_string_create (                                   const nkChar* cstr);
NKAPI NKINLINE nkString nk_string_create (                                   const nkChar* cstr, nkU64 length);
NKAPI NKINLINE nkString nk_string_create (                                   nkChar c);
NKAPI NKINLINE void     nk_string_assign (      nkString* string,            const nkString* other);
NKAPI NKINLINE void     nk_string_assign (      nkString* string,            const nkChar* cstr);
NKAPI NKINLINE void     nk_string_assign (      nkString* string,            const nkChar* cstr, nkU64 length);
NKAPI NKINLINE void     nk_string_assign (      nkString* string,            nkChar c);
NKAPI NKINLINE void     nk_string_append (      nkString* string,            const nkString* other);
NKAPI NKINLINE void     nk_string_append (      nkString* string,            const nkChar* cstr);
NKAPI NKINLINE void     nk_string_append (      nkString* string,            const nkChar* cstr, nkU64 length);
NKAPI NKINLINE void     nk_string_append (      nkString* string,            nkChar c);
NKAPI NKINLINE void     nk_string_insert (      nkString* string, nkU64 pos, const nkString* other);
NKAPI NKINLINE void     nk_string_insert (      nkString* string, nkU64 pos, const nkChar* cstr);
NKAPI NKINLINE void     nk_string_insert (      nkString* string, nkU64 pos, const nkChar* cstr, nkU64 length);
NKAPI NKINLINE void     nk_string_insert (      nkString* string, nkU64 pos, nkChar c);
NKAPI NKINLINE void     nk_string_remove (      nkString* string, nkU64 pos, nkU64 length = 1);
NKAPI NKINLINE void     nk_string_copy   (      nkString* string, const nkString* other);
NKAPI NKINLINE void     nk_string_move   (      nkString* string,       nkString* other);
NKAPI NKINLINE nkString nk_string_copy   (                        const nkString* other);
NKAPI NKINLINE nkString nk_string_move   (                              nkString* other);
NKAPI NKINLINE nkBool   nk_string_compare(const nkString* string, const nkString* other);
NKAPI NKINLINE void     nk_string_lower  (      nkString* string);
NKAPI NKINLINE void     nk_string_upper  (      nkString* string);

NKAPI NKINLINE nkBool operator==(const nkString& a, const nkString& b)
{
    return nk_string_compare(&a, &b);
}
NKAPI NKINLINE nkBool operator!=(const nkString& a, const nkString& b)
{
    return !(a == b);
}

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

nkString::nkString(void)
{
    // Does nothing...
}
nkString::nkString(const nkString& other)
{
    nk_string_copy(this, &other);
}
nkString::nkString(nkString&& other)
{
    nk_string_move(this, &other);
}
nkString::nkString(const nkChar* str)
{
    nk_string_assign(this, str);
}
nkString::nkString(const nkChar* str, nkU64 length)
{
    nk_string_assign(this, str, length);
}
nkString::nkString(nkChar c)
{
    nk_string_assign(this, c);
}

nkString::~nkString(void)
{
    nk_string_free(this);
}

nkString& nkString::operator=(const nkString& other)
{
    nk_string_copy(this, &other);
    return *this;
}
nkString& nkString::operator=(nkString&& other)
{
    nk_string_move(this, &other);
    return *this;
}
nkString& nkString::operator=(const nkChar* str)
{
    nk_string_assign(this, str);
    return *this;
}

const nkChar& nkString::operator[](size_t idx) const
{
    return cstr[idx];
}
nkChar& nkString::operator[](size_t idx)
{
    return cstr[idx];
}

const nkChar* nkString::begin(void) const
{
    return cstr;
}
nkChar* nkString::begin(void)
{
    return cstr;
}

const nkChar* nkString::end(void) const
{
    return (cstr + length);
}
nkChar* nkString::end(void)
{
    return (cstr + length);
}

// Used internally by the string functions to grow the string and should not be
// used externally. If you want to grow an string use nk_string_reserve instead.
NKINTERNAL void nk__string_grow_if_necessary(nkString* string, nkU64 new_size)
{
    NKPERSISTENT NKCONSTEXPR nkU64 NK_STRING_SMALLEST_SIZE = 32;

    NK_ASSERT(string);

    new_size +=1; // For the null-terminator.

    // If the requested new size amount is less than our currently allocated size
    // then we do not need to grow the string data and can just early return here.
    if(string->allocated >= new_size) return;

    nkU64 allocate_size = new_size * 2;
    if(allocate_size < NK_STRING_SMALLEST_SIZE)
        allocate_size = NK_STRING_SMALLEST_SIZE;
    string->cstr = NK_RALLOC_TYPES(nkChar, string->cstr, allocate_size);
    string->cstr[string->length] = '\0';
    string->allocated = allocate_size;
}

NKAPI NKINLINE void nk_string_free(nkString* string)
{
    NK_ASSERT(string);

    if(string->cstr)
        NK_FREE(string->cstr);
    string->cstr = NULL;

    string->length    = 0;
    string->allocated = 0;
}

NKAPI NKINLINE void nk_string_reserve(nkString* string, nkU64 size)
{
    NK_ASSERT(string);
    nk__string_grow_if_necessary(string, size);
}

NKAPI NKINLINE void nk_string_clear(nkString* string)
{
    NK_ASSERT(string);
    if(string->cstr)
        string->cstr[0] = '\0';
    string->length = 0;
}

NKAPI NKINLINE nkChar nk_string_last(nkString* string)
{
    NK_ASSERT(string && string->cstr && string->length > 0);
    return string->cstr[string->length-1];
}

NKAPI NKINLINE nkBool nk_string_empty(nkString* string)
{
    NK_ASSERT(string);
    return (string->length == 0);
}

NKAPI NKINLINE nkString nk_string_create(const nkString* other)
{
    nkString string;
    nk_string_assign(&string, other);
    return string;
}

NKAPI NKINLINE nkString nk_string_create(const nkChar* cstr)
{
    nkString string;
    nk_string_assign(&string, cstr);
    return string;
}

NKAPI NKINLINE nkString nk_string_create(const nkChar* cstr, nkU64 length)
{
    nkString string;
    nk_string_assign(&string, cstr, length);
    return string;
}

NKAPI NKINLINE nkString nk_string_create(nkChar c)
{
    nkString string;
    nk_string_assign(&string, c);
    return string;
}

NKAPI NKINLINE void nk_string_assign(nkString* string, const nkString* other)
{
    NK_ASSERT(string);

    nk_string_clear(string);
    nk_string_append(string, other);
}

NKAPI NKINLINE void nk_string_assign(nkString* string, const nkChar* cstr)
{
    NK_ASSERT(string);

    nk_string_clear(string);
    nk_string_append(string, cstr);
}

NKAPI NKINLINE void nk_string_assign(nkString* string, const nkChar* cstr, nkU64 length)
{
    NK_ASSERT(string);

    nk_string_clear(string);
    nk_string_append(string, cstr, length);
}

NKAPI NKINLINE void nk_string_assign(nkString* string, nkChar c)
{
    NK_ASSERT(string);

    nk_string_clear(string);
    nk_string_append(string, c);
}

NKAPI NKINLINE void nk_string_append(nkString* string, const nkString* other)
{
    NK_ASSERT(string);

    if(!other) return;

    nk_string_append(string, other->cstr, other->length);
}

NKAPI NKINLINE void nk_string_append(nkString* string, const nkChar* cstr)
{
    NK_ASSERT(string);

    if(!cstr) return;

    nk_string_append(string, cstr, strlen(cstr));
}

NKAPI NKINLINE void nk_string_append(nkString* string, const nkChar* cstr, nkU64 length)
{
    NK_ASSERT(string);

    if(!cstr) return;
    if(length == 0) return;

    nk__string_grow_if_necessary(string, string->length + length);

    strncat(string->cstr, cstr, length);
    string->length += length;
}

NKAPI NKINLINE void nk_string_append(nkString* string, nkChar c)
{
    NK_ASSERT(string);

    nk__string_grow_if_necessary(string, string->length + 1);

    string->cstr[string->length++] = c;
    string->cstr[string->length] = '\0';
}

NKAPI NKINLINE void nk_string_insert(nkString* string, nkU64 pos, const nkString* other)
{
    NK_ASSERT(string);
    NK_ASSERT(pos <= string->length);

    if(!other) return;

    nk_string_insert(string, pos, other->cstr, other->length);
}

NKAPI NKINLINE void nk_string_insert(nkString* string, nkU64 pos, const nkChar* cstr)
{
    NK_ASSERT(string);
    NK_ASSERT(pos <= string->length);

    if(!cstr) return;

    nk_string_insert(string, pos, cstr, strlen(cstr));
}

NKAPI NKINLINE void nk_string_insert(nkString* string, nkU64 pos, const nkChar* cstr, nkU64 length)
{
    NK_ASSERT(string);
    NK_ASSERT(pos <= string->length);

    if(!cstr) return;
    if(length == 0) return;

    nk__string_grow_if_necessary(string, string->length + length);

    // Move the contents after pos forward to make room.
    nkU64 length_after_pos = string->length - pos;
    memmove(string->cstr + pos + length, string->cstr + pos, length_after_pos * sizeof(nkChar));
    memcpy(string->cstr + pos, cstr, length * sizeof(nkChar));
    string->length += length;
    string->cstr[string->length] = '\0';
}

NKAPI NKINLINE void nk_string_insert(nkString* string, nkU64 pos, nkChar c)
{
    NK_ASSERT(string);
    NK_ASSERT(pos <= string->length);

    nk_string_insert(string, pos, &c, 1);
}

NKAPI NKINLINE void nk_string_remove(nkString* string, nkU64 pos, nkU64 length)
{
    NK_ASSERT(string && string->length > 0);
    NK_ASSERT(pos < string->length);
    NK_ASSERT(pos + length <= string->length);

    // Move the contents after pos back by the removal length.
    nkU64 length_after_end = string->length - (pos + length);
    memmove(string->cstr + pos, string->cstr + pos + length, length_after_end * sizeof(nkChar));
    string->length -= length;
    string->cstr[string->length] = '\0';
}

NKAPI NKINLINE void nk_string_copy(nkString* string, const nkString* other)
{
    NK_ASSERT(string);
    NK_ASSERT(other);

    nk_string_assign(string, other);
}

NKAPI NKINLINE void nk_string_move(nkString* string, nkString* other)
{
    NK_ASSERT(string);
    NK_ASSERT(other);

    nk_string_free(string);

    string->length    = other->length;
    string->allocated = other->allocated;
    string->cstr      = other->cstr;

    other->length     = 0;
    other->allocated  = 0;
    other->cstr       = NULL;
}

NKAPI NKINLINE nkString nk_string_copy(const nkString* other)
{
    nkString string;
    nk_string_copy(&string, other);
    return string;
}

NKAPI NKINLINE nkString nk_string_move(nkString* other)
{
    nkString string;
    nk_string_move(&string, other);
    return string;
}

NKAPI NKINLINE nkBool nk_string_compare(const nkString* string, const nkString* other)
{
    NK_ASSERT(string);
    NK_ASSERT(other);

    return (strcmp(string->cstr, other->cstr) == 0);
}

NKAPI NKINLINE void nk_string_lower(nkString* string)
{
    NK_ASSERT(string);
    for(nkU64 i=0; i<string->length; ++i)
    {
        string->cstr[i] = NK_CAST(nkChar, tolower(string->cstr[i]));
    }
}

NKAPI NKINLINE void nk_string_upper(nkString* string)
{
    NK_ASSERT(string);
    for(nkU64 i=0; i<string->length; ++i)
    {
        string->cstr[i] = NK_CAST(nkChar, toupper(string->cstr[i]));
    }
}

#endif /* NK_STRING_H__ //////////////////////////////////////////////////////*/

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
