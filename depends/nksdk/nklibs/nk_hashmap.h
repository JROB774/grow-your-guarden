#ifndef NK_HASHMAP_H__ /*/////////////////////////////////////////////////////*/
#define NK_HASHMAP_H__

#include "nk_define.h"
#include "nk_string.h"

#if !defined(NK_HAS_CPP)
#error nk_hashmap requires C++ in order to be used
#endif

template<typename K, typename V>
struct nkHashMap
{
    struct Slot;

    struct Iterator
    {
        nkHashMap<K,V>* m_map   = NULL;
        nkU64           m_index = 0;

        explicit Iterator(nkHashMap<K,V>* map);
        explicit Iterator(nkHashMap<K,V>* map, nkU64 index);

        Iterator& operator++(void);

        Slot& operator* (void) const;
        Slot* operator->(void) const;

        nkBool operator==(const Iterator& other) const;
        nkBool operator!=(const Iterator& other) const;

        void advance(void);
    };

    struct Slot
    {
        nkU32 hash;
        K     key;
        V     value;
    };

    nkF32 load_factor = 0.0f;
    nkU64 count       = 0;
    nkU64 allocated   = 0;
    Slot* slots       = NULL;

    nkHashMap(void);
    nkHashMap(nkU64 initial_count, nkF32 load_factor = 0.7f);
    nkHashMap(const nkHashMap<K,V>& other);
    nkHashMap(nkHashMap<K,V>&& other);

   ~nkHashMap(void);

    nkHashMap<K,V>& operator=(const nkHashMap<K,V>& other);
    nkHashMap<K,V>& operator=(nkHashMap<K,V>&& other);

    const V& operator[](const K& key) const;
          V& operator[](const K& key);

    // These functions exist to make the container work in range-based for loops.
    const Iterator begin(void) const;
          Iterator begin(void);
    const Iterator end  (void) const;
          Iterator end  (void);
};

template<typename K, typename V> NKAPI void           nk_hashmap_init    (nkHashMap<K,V>* map, nkU64 initial_count = 32, nkF32 load_factor = 0.7f);
template<typename K, typename V> NKAPI void           nk_hashmap_free    (nkHashMap<K,V>* map);
template<typename K, typename V> NKAPI void           nk_hashmap_clear   (nkHashMap<K,V>* map);
template<typename K, typename V> NKAPI nkBool         nk_hashmap_empty   (nkHashMap<K,V>* map);
template<typename K, typename V> NKAPI void           nk_hashmap_insert  (nkHashMap<K,V>* map, const K& key, const V& value);
template<typename K, typename V> NKAPI void           nk_hashmap_remove  (nkHashMap<K,V>* map, const K& key);
template<typename K, typename V> NKAPI nkBool         nk_hashmap_contains(nkHashMap<K,V>* map, const K& key);
template<typename K, typename V> NKAPI V*             nk_hashmap_getptr  (nkHashMap<K,V>* map, const K& key);
template<typename K, typename V> NKAPI V&             nk_hashmap_getref  (nkHashMap<K,V>* map, const K& key);
template<typename K, typename V> NKAPI void           nk_hashmap_copy    (nkHashMap<K,V>* map, const nkHashMap<K,V>* other);
template<typename K, typename V> NKAPI void           nk_hashmap_move    (nkHashMap<K,V>* map,       nkHashMap<K,V>* other);
template<typename K, typename V> NKAPI nkHashMap<K,V> nk_hashmap_copy    (                     const nkHashMap<K,V>* other);
template<typename K, typename V> NKAPI nkHashMap<K,V> nk_hashmap_move    (                           nkHashMap<K,V>* other);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

// We use hash values of 0 and 1 to denote whether a slot is currently empty or
// freed. Empty slots will be filled when encountered during an insert. Freed
// slots are used to keep collision runs working when doing a search into the
// map whilst also providing a way for the insert system to see that the slot
// is available for re-assignment if it is hit during hashing.
//
// As a result of this, hash functions that return 0 and 1 as valid hashes will
// not work with our hash-map because they will be interpreted incorrectly!
NKINTERNAL NKCONSTEXPR nkU32 NK_HASHMAP_SLOT_EMPTY = 0;
NKINTERNAL NKCONSTEXPR nkU32 NK_HASHMAP_SLOT_FREED = 1;

// Iterator ====================================================================

template<typename K, typename V>
nkHashMap<K,V>::Iterator::Iterator(nkHashMap<K,V>* map)
{
    m_map = map;
    advance();
}
template<typename K, typename V>
nkHashMap<K,V>::Iterator::Iterator(nkHashMap<K,V>* map, nkU64 index)
{
    m_map = map;
    m_index = index;
}

template<typename K, typename V>
typename nkHashMap<K,V>::Iterator& nkHashMap<K,V>::Iterator::operator++(void)
{
    ++m_index;
    advance();
    return *this;
}

template<typename K, typename V>
typename nkHashMap<K,V>::Slot& nkHashMap<K,V>::Iterator::operator*(void) const
{
    return m_map->slots[m_index];
}
template<typename K, typename V>
typename nkHashMap<K,V>::Slot* nkHashMap<K,V>::Iterator::operator->(void) const
{
    return &m_map->slots[m_index];
}

template<typename K, typename V>
nkBool nkHashMap<K,V>::Iterator::operator==(const Iterator& other) const
{
    return ((m_map == other.m_map) && (m_index == other.m_index));
}
template<typename K, typename V>
nkBool nkHashMap<K,V>::Iterator::operator!=(const Iterator& other) const
{
    return !(*this == other);
}

template<typename K, typename V>
void nkHashMap<K,V>::Iterator::advance(void)
{
    while((m_index < m_map->allocated) &&
          (m_map->slots[m_index].hash == NK_HASHMAP_SLOT_EMPTY ||
           m_map->slots[m_index].hash == NK_HASHMAP_SLOT_FREED))
    {
        ++m_index;
    }
}

// =============================================================================

// HashMap =====================================================================

template<typename K, typename V>
nkHashMap<K,V>::nkHashMap(void)
{
    // Does nothing...
}
template<typename K, typename V>
nkHashMap<K,V>::nkHashMap(nkU64 initial_count, nkF32 load_factor)
{
    nk_hashmap_init(this, initial_count, load_factor);
}
template<typename K, typename V>
nkHashMap<K,V>::nkHashMap(const nkHashMap<K,V>& other)
{
    nk_hashmap_copy(this, &other);
}
template<typename K, typename V>
nkHashMap<K,V>::nkHashMap(nkHashMap<K,V>&& other)
{
    nk_hashmap_move(this, &other);
}

template<typename K, typename V>
nkHashMap<K,V>::~nkHashMap(void)
{
    nk_hashmap_free(this);
}

template<typename K, typename V>
nkHashMap<K,V>& nkHashMap<K,V>::operator=(const nkHashMap<K,V>& other)
{
    nk_hashmap_copy(this, &other);
    return *this;
}
template<typename K, typename V>
nkHashMap<K,V>& nkHashMap<K,V>::operator=(nkHashMap<K,V>&& other)
{
    nk_hashmap_move(this, &other);
    return *this;
}

template<typename K, typename V>
const V& nkHashMap<K,V>::operator[](const K& key) const
{
    return nk_hashmap_getref(this, key);
}
template<typename K, typename V>
V& nkHashMap<K,V>::operator[](const K& key)
{
    return nk_hashmap_getref(this, key);
}

template<typename K, typename V>
const typename nkHashMap<K,V>::Iterator nkHashMap<K,V>::begin(void) const
{
    return Iterator(this);
}
template<typename K, typename V>
typename nkHashMap<K,V>::Iterator nkHashMap<K,V>::begin(void)
{
    return Iterator(this);
}

template<typename K, typename V>
const typename nkHashMap<K,V>::Iterator nkHashMap<K,V>::end(void) const
{
    return Iterator(this, allocated);
}
template<typename K, typename V>
typename nkHashMap<K,V>::Iterator nkHashMap<K,V>::end(void)
{
    return Iterator(this, allocated);
}

// The default hashing function will hash using the bytes of the key. There
// is a built in specializations for the nkString type. If the user wants to
// add more they are free to create their own specializations for new types.
template<typename K>
nkU32 nk__hashmap_hash(const K& key)
{
    const nkU8* bytes = NK_CAST(nkU8*, &key);
    nkU32 hash = 1;
    for(nkU64 i=0; i<sizeof(K); ++i)
         hash = bytes[i] + (hash << 6) + (hash << 16) - hash;
    return hash;
}
template<>
nkU32 nk__hashmap_hash(const nkString& key)
{
    nkU32 hash = 1;
    const nkChar* cstr = key.cstr;
    nkS32 c;
    while((c = *cstr++) != 0)
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}

template<typename K, typename V>
NKINTERNAL typename nkHashMap<K,V>::Slot* nk__hashmap_get_slot(nkHashMap<K,V>* map, const K& key)
{
    NK_ASSERT(map);

    if(!map->slots) return NULL;
    if(!map->count) return NULL; // Important to check, if the whole map is freed slots then we will inifinite loop otherwise.

    nkU32 hash = nk__hashmap_hash<K>(key);

    // Make sure the hash function isn't generating 0 or 1 as values. This will
    // corrupt the data of the hash map as those values have special meanings.
    NK_ASSERT(hash != NK_HASHMAP_SLOT_EMPTY);
    NK_ASSERT(hash != NK_HASHMAP_SLOT_FREED);

    nkU32 iterations = 0; // Track if we've done a full lap so we can escape the loop if necessary.
    nkU32 probe = (hash % map->allocated);
    while(map->slots[probe].hash != hash)
    {
        if(map->slots[probe].hash == NK_HASHMAP_SLOT_EMPTY || iterations > map->allocated)
            return NULL;
        probe = (probe + 1) % map->allocated;
        iterations++;
    }

    return &map->slots[probe];
}

template<typename K, typename V>
NKINTERNAL void nk__hashmap_grow_and_rehash(nkHashMap<K,V>* map)
{
    NK_ASSERT(map);
    NK_ASSERT(map->allocated != 0);

    typedef typename nkHashMap<K,V>::Slot SlotType;

    SlotType* old_slots = map->slots;
    nkU64 old_allocated = map->allocated;

    map->count      = 0;
    map->allocated *= 2;

    map->slots = NK_CALLOC_TYPES(SlotType, map->allocated);

    for(nkU64 i=0; i<old_allocated; ++i)
    {
        SlotType* slot = &old_slots[i];
        if(slot->hash != NK_HASHMAP_SLOT_EMPTY && slot->hash != NK_HASHMAP_SLOT_FREED)
        {
            nk_hashmap_insert(map, slot->key, slot->value);
        }
    }

    NK_FREE(old_slots);
}

template<typename K, typename V>
NKAPI void nk_hashmap_init(nkHashMap<K,V>* map, nkU64 initial_count, nkF32 load_factor)
{
    NK_ASSERT(map);

    typedef typename nkHashMap<K,V>::Slot SlotType;

    map->slots       = NK_CALLOC_TYPES(SlotType, initial_count);
    map->load_factor = load_factor;
    map->allocated   = initial_count;
}

template<typename K, typename V>
NKAPI void nk_hashmap_free(nkHashMap<K,V>* map)
{
    NK_ASSERT(map);

    typedef typename nkHashMap<K,V>::Slot SlotType;

    if(map->slots)
    {
        for(nkU64 i=0; i<map->allocated; ++i)
        {
            // Explicitally call the destructor because types for K or V could internally handle
            // memory that needs to be freed, a common example of this is using nkString for K.
            SlotType* slot = &map->slots[i];
            if(slot->hash != NK_HASHMAP_SLOT_EMPTY)
                slot->~Slot();
        }
        NK_FREE(map->slots);
    }

    map->count     = 0;
    map->allocated = 0;
    map->slots     = NULL;
}

template<typename K, typename V>
NKAPI void nk_hashmap_clear(nkHashMap<K,V>* map)
{
    NK_ASSERT(map);

    map->count = 0;
    if(map->slots)
    {
        for(nkU64 i=0; i<map->allocated; ++i)
        {
            map->slots[i].hash = NK_HASHMAP_SLOT_EMPTY;
        }
    }
}

template<typename K, typename V>
NKAPI nkBool nk_hashmap_empty(nkHashMap<K,V>* map)
{
    NK_ASSERT(map);
    return (map->count == 0);
}

template<typename K, typename V>
NKAPI void nk_hashmap_insert(nkHashMap<K,V>* map, const K& key, const V& value)
{
    NK_ASSERT(map);
    NK_ASSERT(!nk_hashmap_contains(map, key)); // We don't allow multiple values under the same key.
    // If the nk_hashmap_init hasn't been called yet then we just do it internally.
    if(!map->slots) nk_hashmap_init(map);

    NK_ASSERT(map->load_factor > 0.0f && map->load_factor < 1.0f);
    nkF32 load_factor = NK_CAST(nkF32, map->count+1) / NK_CAST(nkF32, map->allocated);
    if(load_factor >= map->load_factor) nk__hashmap_grow_and_rehash(map);

    nkU32 hash = nk__hashmap_hash<K>(key);

    // Make sure the hash function isn't generating 0 or 1 as values. This will
    // corrupt the data of the hash map as those values have special meanings.
    NK_ASSERT(hash != NK_HASHMAP_SLOT_EMPTY);
    NK_ASSERT(hash != NK_HASHMAP_SLOT_FREED);

    nkU32 probe = (hash % map->allocated);
    while(map->slots[probe].hash != NK_HASHMAP_SLOT_EMPTY && map->slots[probe].hash != NK_HASHMAP_SLOT_FREED)
    {
        probe = (probe + 1) % map->allocated;
    }

    map->slots[probe].hash  = hash;
    map->slots[probe].key   = key;
    map->slots[probe].value = value;

    map->count++;
}

template<typename K, typename V>
NKAPI void nk_hashmap_remove(nkHashMap<K,V>* map, const K& key)
{
    NK_ASSERT(map);

    typedef typename nkHashMap<K,V>::Slot SlotType;

    SlotType* slot = nk__hashmap_get_slot(map, key);
    if(slot)
    {
        slot->hash = NK_HASHMAP_SLOT_FREED;
        map->count--;
    }
}

template<typename K, typename V>
NKAPI nkBool nk_hashmap_contains(nkHashMap<K,V>* map, const K& key)
{
    NK_ASSERT(map);

    return (nk_hashmap_getptr(map, key) != NULL);
}

template<typename K, typename V>
NKAPI V* nk_hashmap_getptr(nkHashMap<K,V>* map, const K& key)
{
    NK_ASSERT(map);

    typedef typename nkHashMap<K,V>::Slot SlotType;

    SlotType* slot = nk__hashmap_get_slot(map, key);
    return (!slot) ? NULL : &slot->value;
}

template<typename K, typename V>
NKAPI V& nk_hashmap_getref(nkHashMap<K,V>* map, const K& key)
{
    NK_ASSERT(map);

    V* value = nk_hashmap_getptr(map, key);
    if(value) return (*value);

    // If we couldn't find the value in the hash map then we return a dummy
    // that will just get assigned to whatever the default is for type V.
    NKPERSISTENT V dummy;
    return dummy;
}

template<typename K, typename V>
NKAPI void nk_hashmap_copy(nkHashMap<K,V>* map, const nkHashMap<K,V>* other)
{
    NK_ASSERT(map);
    NK_ASSERT(other);

    typedef typename nkHashMap<K,V>::Slot SlotType;

    nk_hashmap_free(map);

    if(other->count > 0)
    {
        nk_hashmap_init(map, other->allocated, other->load_factor);
        for(nkU64 i=0; i<other->allocated; ++i)
        {
            SlotType* slot = &other->slots[i];
            if(slot->hash != NK_HASHMAP_SLOT_EMPTY && slot->hash != NK_HASHMAP_SLOT_FREED)
            {
                nk_hashmap_insert(map, slot->key, slot->value);
            }
        }
    }
}

template<typename K, typename V>
NKAPI void nk_hashmap_move(nkHashMap<K,V>* map, nkHashMap<K,V>* other)
{
    NK_ASSERT(map);
    NK_ASSERT(other);

    nk_hashmap_free(map);

    map->count       = other->count;
    map->allocated   = other->allocated;
    map->slots       = other->slots;

    other->count     = 0;
    other->allocated = 0;
    other->slots     = NULL;
}

template<typename K, typename V>
NKAPI nkHashMap<K,V> nk_hashmap_copy(const nkHashMap<K,V>* other)
{
    nkHashMap<K,V> map;
    nk_hashmap_copy(&map, other);
    return map;
}

template<typename K, typename V>
NKAPI nkHashMap<K,V> nk_hashmap_move(nkHashMap<K,V>* other)
{
    nkHashMap<K,V> map;
    nk_hashmap_move(&map, other);
    return map;
}

// =============================================================================

#endif /* NK_HASHMAP_H__ /////////////////////////////////////////////////////*/

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
