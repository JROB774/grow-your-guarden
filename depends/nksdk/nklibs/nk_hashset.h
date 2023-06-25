#ifndef NK_HASHSET_H__ /*/////////////////////////////////////////////////////*/
#define NK_HASHSET_H__

#include "nk_define.h"
#include "nk_string.h"

#if !defined(NK_HAS_CPP)
#error nk_hashset requires C++ in order to be used
#endif

template<typename K>
struct nkHashSet
{
    struct Slot;

    struct Iterator
    {
        nkHashSet<K>* m_set   = NULL;
        nkU64         m_index = 0;

        explicit Iterator(nkHashSet<K>* set);
        explicit Iterator(nkHashSet<K>* set, nkU64 index);

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
    };

    nkF32 load_factor = 0.0f;
    nkU64 count       = 0;
    nkU64 allocated   = 0;
    Slot* slots       = NULL;

    nkHashSet(void);
    nkHashSet(nkU64 initial_count, nkF32 load_factor = 0.7f);
    nkHashSet(const nkHashSet<K>& other);
    nkHashSet(nkHashSet<K>&& other);

   ~nkHashSet(void);

    nkHashSet<K>& operator=(const nkHashSet<K>& other);
    nkHashSet<K>& operator=(nkHashSet<K>&& other);

    // These functions exist to make the container work in range-based for loops.
    const Iterator begin(void) const;
          Iterator begin(void);
    const Iterator end  (void) const;
          Iterator end  (void);
};

template<typename K> NKAPI void         nk_hashset_init    (nkHashSet<K>* set, nkU64 initial_count = 32, nkF32 load_factor = 0.7f);
template<typename K> NKAPI void         nk_hashset_free    (nkHashSet<K>* set);
template<typename K> NKAPI void         nk_hashset_clear   (nkHashSet<K>* set);
template<typename K> NKAPI nkBool       nk_hashset_empty   (nkHashSet<K>* set);
template<typename K> NKAPI void         nk_hashset_insert  (nkHashSet<K>* set, const K& key);
template<typename K> NKAPI void         nk_hashset_remove  (nkHashSet<K>* set, const K& key);
template<typename K> NKAPI nkBool       nk_hashset_contains(nkHashSet<K>* set, const K& key);
template<typename K> NKAPI void         nk_hashset_copy    (nkHashSet<K>* set, const nkHashSet<K>* other);
template<typename K> NKAPI void         nk_hashset_move    (nkHashSet<K>* set,       nkHashSet<K>* other);
template<typename K> NKAPI nkHashSet<K> nk_hashset_copy    (                   const nkHashSet<K>* other);
template<typename K> NKAPI nkHashSet<K> nk_hashset_move    (                         nkHashSet<K>* other);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

// We use hash values of 0 and 1 to denote whether a slot is currently empty or
// freed. Empty slots will be filled when encountered during an insert. Freed
// slots are used to keep collision runs working when doing a search into the
// set whilst also providing a way for the insert system to see that the slot
// is available for re-assignment if it is hit during hashing.
//
// As a result of this, hash functions that return 0 and 1 as valid hashes will
// not work with our hash-set because they will be interpreted incorrectly!
NKINTERNAL NKCONSTEXPR nkU32 NK_HASHSET_SLOT_EMPTY = 0;
NKINTERNAL NKCONSTEXPR nkU32 NK_HASHSET_SLOT_FREED = 1;

// Iterator ====================================================================

template<typename K>
nkHashSet<K>::Iterator::Iterator(nkHashSet<K>* set)
{
    m_set = set;
    advance();
}
template<typename K>
nkHashSet<K>::Iterator::Iterator(nkHashSet<K>* set, nkU64 index)
{
    m_set = set;
    m_index = index;
}

template<typename K>
typename nkHashSet<K>::Iterator& nkHashSet<K>::Iterator::operator++(void)
{
    ++m_index;
    advance();
    return *this;
}

template<typename K>
typename nkHashSet<K>::Slot& nkHashSet<K>::Iterator::operator*(void) const
{
    return m_set->slots[m_index];
}
template<typename K>
typename nkHashSet<K>::Slot* nkHashSet<K>::Iterator::operator->(void) const
{
    return &m_set->slots[m_index];
}

template<typename K>
nkBool nkHashSet<K>::Iterator::operator==(const Iterator& other) const
{
    return ((m_set == other.m_set) && (m_index == other.m_index));
}
template<typename K>
nkBool nkHashSet<K>::Iterator::operator!=(const Iterator& other) const
{
    return !(*this == other);
}

template<typename K>
void nkHashSet<K>::Iterator::advance(void)
{
    while((m_index < m_set->allocated) &&
          (m_set->slots[m_index].hash == NK_HASHSET_SLOT_EMPTY ||
           m_set->slots[m_index].hash == NK_HASHSET_SLOT_FREED))
    {
        ++m_index;
    }
}

// =============================================================================

// HashSet =====================================================================

template<typename K>
nkHashSet<K>::nkHashSet(void)
{
    // Does nothing...
}
template<typename K>
nkHashSet<K>::nkHashSet(nkU64 initial_count, nkF32 load_factor)
{
    nk_hashset_init(this, initial_count, load_factor);
}
template<typename K>
nkHashSet<K>::nkHashSet(const nkHashSet<K>& other)
{
    nk_hashset_copy(this, &other);
}
template<typename K>
nkHashSet<K>::nkHashSet(nkHashSet<K>&& other)
{
    nk_hashset_move(this, &other);
}

template<typename K>
nkHashSet<K>::~nkHashSet(void)
{
    nk_hashset_free(this);
}

template<typename K>
nkHashSet<K>& nkHashSet<K>::operator=(const nkHashSet<K>& other)
{
    nk_hashset_copy(this, &other);
    return *this;
}
template<typename K>
nkHashSet<K>& nkHashSet<K>::operator=(nkHashSet<K>&& other)
{
    nk_hashset_move(this, &other);
    return *this;
}

template<typename K>
const typename nkHashSet<K>::Iterator nkHashSet<K>::begin(void) const
{
    return Iterator(this);
}
template<typename K>
typename nkHashSet<K>::Iterator nkHashSet<K>::begin(void)
{
    return Iterator(this);
}

template<typename K>
const typename nkHashSet<K>::Iterator nkHashSet<K>::end(void) const
{
    return Iterator(this, allocated);
}
template<typename K>
typename nkHashSet<K>::Iterator nkHashSet<K>::end(void)
{
    return Iterator(this, allocated);
}

// The default hashing function will hash using the bytes of the key. There
// is a built in specializations for the nkString type. If the user wants to
// add more they are free to create their own specializations for new types.
template<typename K>
nkU32 nk__hashset_hash(const K& key)
{
    const nkU8* bytes = NK_CAST(nkU8*, &key);
    nkU32 hash = 1;
    for(nkU64 i=0; i<sizeof(K); ++i)
        hash = bytes[i] + (hash << 6) + (hash << 16) - hash;
    return hash;
}
template<>
nkU32 nk__hashset_hash(const nkString& key)
{
    nkU32 hash = 1;
    const nkChar* cstr = key.cstr;
    nkS32 c;
    while((c = *cstr++) != 0)
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}

template<typename K>
NKINTERNAL typename nkHashSet<K>::Slot* nk__hashset_get_slot(nkHashSet<K>* set, const K& key)
{
    NK_ASSERT(set);

    if(!set->slots) return NULL;
    if(!set->count) return NULL; // Important to check, if the whole set is freed slots then we will inifinite loop otherwise.

    nkU32 hash = nk__hashset_hash<K>(key);

    // Make sure the hash function isn't generating 0 or 1 as values. This will
    // corrupt the data of the hash set as those values have special meanings.
    NK_ASSERT(hash != NK_HASHSET_SLOT_EMPTY);
    NK_ASSERT(hash != NK_HASHSET_SLOT_FREED);

    nkU32 iterations = 0; // Track if we've done a full lap so we can escape the loop if necessary.
    nkU32 probe = (hash % set->allocated);
    while(set->slots[probe].hash != hash)
    {
        if(set->slots[probe].hash == NK_HASHSET_SLOT_EMPTY || iterations > set->allocated)
            return NULL;
        probe = (probe + 1) % set->allocated;
        iterations++;
    }

    return &set->slots[probe];
}

template<typename K>
NKINTERNAL void nk__hashset_grow_and_rehash(nkHashSet<K>* set)
{
    NK_ASSERT(set);
    NK_ASSERT(set->allocated != 0);

    typedef typename nkHashSet<K>::Slot SlotType;

    SlotType* old_slots = set->slots;
    nkU64 old_allocated = set->allocated;

    set->count      = 0;
    set->allocated *= 2;

    set->slots = NK_CALLOC_TYPES(SlotType, set->allocated);

    for(nkU64 i=0; i<old_allocated; ++i)
    {
        SlotType* slot = &old_slots[i];
        if(slot->hash != NK_HASHSET_SLOT_EMPTY && slot->hash != NK_HASHSET_SLOT_FREED)
        {
            nk_hashset_insert(set, slot->key);
        }
    }

    NK_FREE(old_slots);
}

template<typename K>
NKAPI void nk_hashset_init(nkHashSet<K>* set, nkU64 initial_count, nkF32 load_factor)
{
    NK_ASSERT(set);

    typedef typename nkHashSet<K>::Slot SlotType;

    set->slots       = NK_CALLOC_TYPES(SlotType, initial_count);
    set->load_factor = load_factor;
    set->allocated   = initial_count;
}

template<typename K>
NKAPI void nk_hashset_free(nkHashSet<K>* set)
{
    NK_ASSERT(set);

    typedef typename nkHashSet<K>::Slot SlotType;

    if(set->slots)
    {
        for(nkU64 i=0; i<set->allocated; ++i)
        {
            // Explicitally call the destructor because types for K could internally handle
            // memory that needs to be freed, a common example of this is using nkString for K.
            SlotType* slot = &set->slots[i];
            if(slot->hash != NK_HASHSET_SLOT_EMPTY)
                slot->~Slot();
        }
        NK_FREE(set->slots);
    }

    set->count     = 0;
    set->allocated = 0;
    set->slots     = NULL;
}

template<typename K>
NKAPI void nk_hashset_clear(nkHashSet<K>* set)
{
    NK_ASSERT(set);

    set->count = 0;
    if(set->slots)
    {
        for(nkU64 i=0; i<set->allocated; ++i)
        {
            set->slots[i].hash = NK_HASHSET_SLOT_EMPTY;
        }
    }
}

template<typename K>
NKAPI nkBool nk_hashset_empty(nkHashSet<K>* set)
{
    NK_ASSERT(set);
    return (set->count == 0);
}

template<typename K>
NKAPI void nk_hashset_insert(nkHashSet<K>* set, const K& key)
{
    NK_ASSERT(set);
    NK_ASSERT(!nk_hashset_contains(set, key)); // We don't allow multiple of the same key.

    // If the nk_hashset_init hasn't been called yet then we just do it internally.
    if(!set->slots) nk_hashset_init(set);

    NK_ASSERT(set->load_factor > 0.0f && set->load_factor < 1.0f);
    nkF32 load_factor = NK_CAST(nkF32, set->count+1) / NK_CAST(nkF32, set->allocated);
    if(load_factor >= set->load_factor) nk__hashset_grow_and_rehash(set);

    nkU32 hash = nk__hashset_hash<K>(key);

    // Make sure the hash function isn't generating 0 or 1 as values. This will
    // corrupt the data of the hash set as those values have special meanings.
    NK_ASSERT(hash != NK_HASHSET_SLOT_EMPTY);
    NK_ASSERT(hash != NK_HASHSET_SLOT_FREED);

    nkU32 probe = (hash % set->allocated);
    while(set->slots[probe].hash != NK_HASHSET_SLOT_EMPTY && set->slots[probe].hash != NK_HASHSET_SLOT_FREED)
    {
        probe = (probe + 1) % set->allocated;
    }

    set->slots[probe].hash = hash;
    set->slots[probe].key  = key;

    set->count++;
}

template<typename K>
NKAPI void nk_hashset_remove(nkHashSet<K>* set, const K& key)
{
    NK_ASSERT(set);

    typedef typename nkHashSet<K>::Slot SlotType;

    SlotType* slot = nk__hashset_get_slot(set, key);
    if(slot)
    {
        slot->hash = NK_HASHSET_SLOT_FREED;
        set->count--;
    }
}

template<typename K>
NKAPI nkBool nk_hashset_contains(nkHashSet<K>* set, const K& key)
{
    NK_ASSERT(set);
    return nk__hashset_get_slot(set, key);
}

template<typename K>
NKAPI void nk_hashset_copy(nkHashSet<K>* set, const nkHashSet<K>* other)
{
    NK_ASSERT(set);
    NK_ASSERT(other);

    typedef typename nkHashSet<K>::Slot SlotType;

    nk_hashset_free(set);

    if(other->count > 0)
    {
        nk_hashset_init(set, other->allocated);
        for(nkU64 i=0; i<other->allocated; ++i)
        {
            SlotType* slot = &other->slots[i];
            if(slot->hash != NK_HASHSET_SLOT_EMPTY && slot->hash != NK_HASHSET_SLOT_FREED)
            {
                nk_hashset_insert(set, slot->key);
            }
        }
    }
}

template<typename K>
NKAPI void nk_hashset_move(nkHashSet<K>* set, nkHashSet<K>* other)
{
    NK_ASSERT(set);
    NK_ASSERT(other);

    nk_hashset_free(set);

    set->count       = other->count;
    set->allocated   = other->allocated;
    set->slots       = other->slots;

    other->count     = 0;
    other->allocated = 0;
    other->slots     = NULL;
}

template<typename K>
NKAPI nkHashSet<K> nk_hashset_copy(const nkHashSet<K>* other)
{
    nkHashSet<K> set;
    nk_hashset_copy(&set, other);
    return set;
}

template<typename K>
NKAPI nkHashSet<K> nk_hashset_move(nkHashSet<K>* other)
{
    nkHashSet<K> set;
    nk_hashset_move(&set, other);
    return set;
}

// =============================================================================

#endif /* NK_HASHSET_H__ /////////////////////////////////////////////////////*/

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
