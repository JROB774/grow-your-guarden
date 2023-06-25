#ifndef NK_ARRAY_H__ /*///////////////////////////////////////////////////////*/
#define NK_ARRAY_H__

#include "nk_define.h"

#include <string.h>

#if !defined(NK_HAS_CPP)
#error nk_array requires C++ in order to be used
#endif

template<typename T>
struct nkArray
{
    nkU64 length    = 0;
    nkU64 allocated = 0;

    T* data = NULL;

    nkArray(void);
    nkArray(nkInitializerList<T> list);
    nkArray(const nkArray<T>& other);
    nkArray(nkArray<T>&& other);
    nkArray(T elem);
    nkArray(const T* elems, nkU64 count);

   ~nkArray(void);

    nkArray<T>& operator=(const nkArray<T>& other);
    nkArray<T>& operator=(nkArray<T>&& other);

    const T& operator[](size_t idx) const;
          T& operator[](size_t idx);

    // These functions exist to make the container work in range-based for loops.
    const T* begin(void) const;
          T* begin(void);
    const T* end  (void) const;
          T* end  (void);
};

template<typename T> NKAPI void       nk_array_free   (nkArray<T>* array);
template<typename T> NKAPI void       nk_array_fill   (nkArray<T>* array, T value, nkU64 count);
template<typename T> NKAPI void       nk_array_reserve(nkArray<T>* array, nkU64 size);
template<typename T> NKAPI void       nk_array_clear  (nkArray<T>* array);
template<typename T> NKAPI T&         nk_array_last   (nkArray<T>* array);
template<typename T> NKAPI nkBool     nk_array_empty  (nkArray<T>* array);
template<typename T> NKAPI nkArray<T> nk_array_create (                              nkInitializerList<T> list);
template<typename T> NKAPI nkArray<T> nk_array_create (                              const nkArray<T>* other);
template<typename T> NKAPI nkArray<T> nk_array_create (                              T elem);
template<typename T> NKAPI nkArray<T> nk_array_create (                              const T* elems, nkU64 count);
template<typename T> NKAPI void       nk_array_assign (nkArray<T>* array,            nkInitializerList<T> list);
template<typename T> NKAPI void       nk_array_assign (nkArray<T>* array,            const nkArray<T>* other);
template<typename T> NKAPI void       nk_array_assign (nkArray<T>* array,            T elem);
template<typename T> NKAPI void       nk_array_assign (nkArray<T>* array,            const T* elems, nkU64 count);
template<typename T> NKAPI void       nk_array_append (nkArray<T>* array,            nkInitializerList<T> list);
template<typename T> NKAPI void       nk_array_append (nkArray<T>* array,            const nkArray<T>* other);
template<typename T> NKAPI void       nk_array_append (nkArray<T>* array,            T elem);
template<typename T> NKAPI void       nk_array_append (nkArray<T>* array,            const T* elems, nkU64 count);
template<typename T> NKAPI void       nk_array_insert (nkArray<T>* array, nkU64 pos, nkInitializerList<T> list);
template<typename T> NKAPI void       nk_array_insert (nkArray<T>* array, nkU64 pos, const nkArray<T>* other);
template<typename T> NKAPI void       nk_array_insert (nkArray<T>* array, nkU64 pos, T elem);
template<typename T> NKAPI void       nk_array_insert (nkArray<T>* array, nkU64 pos, const T* elems, nkU64 count);
template<typename T> NKAPI void       nk_array_remove (nkArray<T>* array, nkU64 pos, nkU64 count = 1);
template<typename T> NKAPI void       nk_array_copy   (nkArray<T>* array, const nkArray<T>* other);
template<typename T> NKAPI void       nk_array_move   (nkArray<T>* array,       nkArray<T>* other);
template<typename T> NKAPI nkArray<T> nk_array_copy   (                   const nkArray<T>* other);
template<typename T> NKAPI nkArray<T> nk_array_move   (                         nkArray<T>* other);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

template<typename T>
nkArray<T>::nkArray(void)
{
    // Does nothing...
}
template<typename T>
nkArray<T>::nkArray(nkInitializerList<T> list)
{
    nk_array_assign(this, list);
}
template<typename T>
nkArray<T>::nkArray(const nkArray<T>& other)
{
    nk_array_copy(this, &other);
}
template<typename T>
nkArray<T>::nkArray(nkArray<T>&& other)
{
    nk_array_move(this, &other);
}
template<typename T>
nkArray<T>::nkArray(T elem)
{
    nk_array_assign(this, elem);
}
template<typename T>
nkArray<T>::nkArray(const T* elems, nkU64 count)
{
    nk_array_assign(this, elems, count);
}

template<typename T>
nkArray<T>::~nkArray(void)
{
    nk_array_free(this);
}

template<typename T>
nkArray<T>& nkArray<T>::operator=(const nkArray<T>& other)
{
    nk_array_copy(this, &other);
    return *this;
}
template<typename T>
nkArray<T>& nkArray<T>::operator=(nkArray<T>&& other)
{
    nk_array_move(this, &other);
    return *this;
}

template<typename T>
const T& nkArray<T>::operator[](size_t idx) const
{
    return data[idx];
}
template<typename T>
T& nkArray<T>::operator[](size_t idx)
{
    return data[idx];
}

template<typename T>
const T* nkArray<T>::begin(void) const
{
    return data;
}
template<typename T>
T* nkArray<T>::begin(void)
{
    return data;
}

template<typename T>
const T* nkArray<T>::end(void) const
{
    return (data + length);
}
template<typename T>
T* nkArray<T>::end(void)
{
    return (data + length);
}

// Used internally by the array functions to grow the array and should not be
// used externally. If you want to grow an array use nk_array_reserve instead.
template<typename T>
NKINTERNAL void nk__array_grow_if_necessary(nkArray<T>* array, nkU64 new_size)
{
    NKPERSISTENT NKCONSTEXPR nkU64 NK_ARRAY_SMALLEST_SIZE = 32;

    NK_ASSERT(array);

    // If the requested new size amount is less than our currently allocated size
    // then we do not need to grow the array data and can just early return here.
    if(array->allocated >= new_size) return;

    nkU64 allocate_size = new_size * 2;
    if(allocate_size < NK_ARRAY_SMALLEST_SIZE)
        allocate_size = NK_ARRAY_SMALLEST_SIZE;
    array->data = NK_RALLOC_TYPES(T, array->data, allocate_size);
    array->allocated = allocate_size;

    // @Improve: Currently we are just doing this so that everything is always
    // zero initialized. We want to handle constructors, etc. properly for
    // more complex types but for now this should work fine for what we want.
    nkU64 empty_space_length = array->allocated - array->length;
    memset(array->data + array->length, 0, empty_space_length * sizeof(T));
}

template<typename T>
NKAPI void nk_array_free(nkArray<T>* array)
{
    NK_ASSERT(array);

    if(array->data)
    {
        for(nkU64 i=0; i<array->length; ++i)
        {
            // Explicitally call the destructor because the type for T could internally handle
            // memory that needs to be freed, a common example of this is using nkString for T.
            T* elem = &array->data[i];
            elem->~T();
        }
        NK_FREE(array->data);
    }

    array->length    = 0;
    array->allocated = 0;
    array->data      = NULL;
}

template<typename T>
NKAPI void nk_array_fill(nkArray<T>* array, T value, nkU64 count)
{
    NK_ASSERT(array);

    nk_array_clear(array);
    nk__array_grow_if_necessary(array, count);
    for(nkU64 i=0; i<count; ++i)
    {
        array->data[i] = value;
    }
}

template<typename T>
NKAPI void nk_array_reserve(nkArray<T>* array, nkU64 size)
{
    NK_ASSERT(array);
    nk__array_grow_if_necessary(array, size);
}

template<typename T>
NKAPI void nk_array_clear(nkArray<T>* array)
{
    NK_ASSERT(array);
    array->length = 0;
}

template<typename T>
NKAPI T& nk_array_last(nkArray<T>* array)
{
    NK_ASSERT(array && array->data && array->length > 0);
    return array->data[array->length-1];
}

template<typename T>
NKAPI nkBool nk_array_empty(nkArray<T>* array)
{
    NK_ASSERT(array);
    return (array->length == 0);
}

template<typename T>
NKAPI nkArray<T> nk_array_create(nkInitializerList<T> list)
{
    nkArray<T> array;
    nk_array_assign(&array, list);
    return array;
}

template<typename T>
NKAPI nkArray<T> nk_array_create(const nkArray<T>* other)
{
    NK_ASSERT(other);

    nkArray<T> array;
    nk_array_assign(&array, other);
    return array;
}

template<typename T>
NKAPI nkArray<T> nk_array_create(T elem)
{
    nkArray<T> array;
    nk_array_assign(&array, elem);
    return array;
}

template<typename T>
NKAPI nkArray<T> nk_array_create(const T* elems, nkU64 count)
{
    NK_ASSERT(elems);

    nkArray<T> array;
    nk_array_assign(&array, elems, count);
    return array;
}

template<typename T>
NKAPI void nk_array_assign(nkArray<T>* array, nkInitializerList<T> list)
{
    NK_ASSERT(array);

    nk_array_clear(array);
    nk_array_append(array, list);
}

template<typename T>
NKAPI void nk_array_assign(nkArray<T>* array, const nkArray<T>* other)
{
    NK_ASSERT(array);
    NK_ASSERT(other);

    nk_array_clear(array);
    nk_array_append(array, other);
}

template<typename T>
NKAPI void nk_array_assign(nkArray<T>* array, T elem)
{
    NK_ASSERT(array);

    nk_array_clear(array);
    nk_array_append(array, elem);
}

template<typename T>
NKAPI void nk_array_assign(nkArray<T>* array, const T* elems, nkU64 count)
{
    NK_ASSERT(array);
    NK_ASSERT(elems);

    nk_array_clear(array);
    nk_array_append(array, elems, count);
}

template<typename T>
NKAPI void nk_array_append(nkArray<T>* array, nkInitializerList<T> list)
{
    NK_ASSERT(array);

    nk_array_append(array, list.begin(), list.size());
}

template<typename T>
NKAPI void nk_array_append(nkArray<T>* array, const nkArray<T>* other)
{
    NK_ASSERT(array);
    NK_ASSERT(other);

    nk_array_append(array, other->data, other->length);
}

template<typename T>
NKAPI void nk_array_append(nkArray<T>* array, T elem)
{
    NK_ASSERT(array);

    nk__array_grow_if_necessary(array, array->length + 1);
    array->data[array->length++] = elem;
}

template<typename T>
NKAPI void nk_array_append(nkArray<T>* array, const T* elems, nkU64 count)
{
    NK_ASSERT(array);
    NK_ASSERT(elems);

    if(count == 0) return;

    nk__array_grow_if_necessary(array, array->length + count);

    memcpy(array->data + array->length, elems, count * sizeof(T));
    array->length += count;
}

template<typename T>
NKAPI void nk_array_insert(nkArray<T>* array, nkU64 pos, nkInitializerList<T> list)
{
    NK_ASSERT(array);
    NK_ASSERT(pos <= array->length);

    nk_array_insert(array, pos, list.begin(), list.size());
}

template<typename T>
NKAPI void nk_array_insert(nkArray<T>* array, nkU64 pos, const nkArray<T>* other)
{
    NK_ASSERT(array);
    NK_ASSERT(pos <= array->length);
    NK_ASSERT(other);

    nk_array_insert(array, pos, other->data, other->length);
}

template<typename T>
NKAPI void nk_array_insert(nkArray<T>* array, nkU64 pos, T elem)
{
    NK_ASSERT(array);
    NK_ASSERT(pos <= array->length);

    nk_array_insert(array, pos, &elem, 1);
}

template<typename T>
NKAPI void nk_array_insert(nkArray<T>* array, nkU64 pos, const T* elems, nkU64 count)
{
    NK_ASSERT(array);
    NK_ASSERT(pos <= array->length);
    NK_ASSERT(elems);

    nk__array_grow_if_necessary(array, array->length + count);

    // Move the contents after pos forward to make room.
    nkU64 length_after_pos = array->length - pos;
    memmove(array->data + pos + count, array->data + pos, length_after_pos * sizeof(T));
    memcpy(array->data + pos, elems, count * sizeof(T));
    array->length += count;
}

template<typename T>
NKAPI void nk_array_remove(nkArray<T>* array, nkU64 pos, nkU64 count)
{
    NK_ASSERT(array && array->length > 0);
    NK_ASSERT(pos < array->length);
    NK_ASSERT(pos + count <= array->length);

    // Move the contents after pos back by the removal count.
    nkU64 length_after_end = array->length - (pos + count);
    memmove(array->data + pos, array->data + pos + count, length_after_end * sizeof(T));
    array->length -= count;
}

template<typename T>
NKAPI void nk_array_copy(nkArray<T>* array, const nkArray<T>* other)
{
    NK_ASSERT(array);
    NK_ASSERT(other);

    nk_array_assign(array, other);
}

template<typename T>
NKAPI void nk_array_move(nkArray<T>* array, nkArray<T>* other)
{
    NK_ASSERT(array);
    NK_ASSERT(other);

    nk_array_free(array);

    array->length    = other->length;
    array->allocated = other->allocated;
    array->data      = other->data;

    other->length    = 0;
    other->allocated = 0;
    other->data      = NULL;
}

template<typename T>
NKAPI nkArray<T> nk_array_copy(const nkArray<T>* other)
{
    nkArray<T> array;
    nk_array_copy(&array, other);
    return array;
}

template<typename T>
NKAPI nkArray<T> nk_array_move(nkArray<T>* other)
{
    nkArray<T> array;
    nk_array_move(&array, other);
    return array;
}

#endif /* NK_ARRAY_H__ ///////////////////////////////////////////////////////*/

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
