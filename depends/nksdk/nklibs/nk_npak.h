#ifndef NK_NPAK_H__ /*////////////////////////////////////////////////////////*/
#define NK_NPAK_H__

#include "nk_define.h"

#define NK_NPAK_FILE_VERSION 1
#define NK_NPAK_FILE_FOURCC NK_FOURCC('NPAK')

typedef struct nkNPAKHeader
{
    nkU32 version;
    nkU32 fourcc;
    nkU64 entries;
    nkU64 table_offset;
    nkU32 padding[10];
}
nkNPAKHeader;

typedef struct nkNPAKEntry
{
    const nkChar* name;
    nkU64         offset;
    nkU64         size;
}
nkNPAKEntry;

typedef struct nkNPAK
{
    nkNPAKHeader* header;
    nkNPAKEntry*  entries; // @Speed: Hash map entries for faster file lookups.
    nkU8*         data_blob;
}
nkNPAK;

NKAPI nkBool       nk_npak_pack         (const nkChar* npak_name, const nkChar* src_path);
NKAPI nkBool       nk_npak_unpack       (const nkChar* npak_name, const nkChar* dst_path);
NKAPI nkBool       nk_npak_load         (nkNPAK* npak, const nkChar* npak_name);
NKAPI void         nk_npak_free         (nkNPAK* npak);
NKAPI void*        nk_npak_get_file_data(nkNPAK* npak, const nkChar* file_name, nkU64* size);
NKAPI nkNPAKEntry* nk_npak_get_file_meta(nkNPAK* npak, const nkChar* file_name);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

#ifdef NK_NPAK_IMPLEMENTATION /*//////////////////////////////////////////////*/

#include "nk_filesys.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NKAPI nkBool nk_npak_pack(const nkChar* npak_name, const nkChar* src_path)
{
    // @Todo: Handling paths is quite gross right now.
    // @Todo: Custom memory allocators.
    // @Todo: Do the write in one single go + use different file writing API.

    nkChar** items = NULL;
    nkU64 item_count = 0;

    nkChar* clean_src_path = NK_MALLOC_TYPES(nkChar, strlen(src_path)+1);
    if(!clean_src_path) return NK_FALSE;
    strcpy(clean_src_path, src_path);
    nk_fixup_path(&clean_src_path);

    nkU64 src_path_length = strlen(clean_src_path);

    // Get all the files within the directory the caller wants to pack.
    if(!nk_list_path_content(clean_src_path, nkPathListFlags_Recursive|nkPathListFlags_Files, &items, &item_count))
    {
        return NK_FALSE;
    }

    // Create entries for each of the files.
    nkNPAKEntry* entries = NK_MALLOC_TYPES(nkNPAKEntry, item_count);
    if(!entries) return NK_FALSE;

    nkU64 current_offset = sizeof(nkNPAKHeader);

    for(nkU64 i=0; i<item_count; ++i)
    {
        nkNPAKEntry* entry = entries+i;
        entry->name = items[i]+src_path_length; // Remove the source path from each of the entry file names.
        entry->offset = current_offset;
        entry->size = nk_file_size(items[i]);
        current_offset += entry->size;
    }

    // Setup the file header.
    nkNPAKHeader header = NK_ZERO_MEM;
    header.version = NK_NPAK_FILE_VERSION;
    header.fourcc = NK_NPAK_FILE_FOURCC;
    header.entries = item_count;
    header.table_offset = current_offset;

    // Write the file content.
    FILE* file = fopen(npak_name, "wb");
    if(!file) return NK_FALSE;

    fwrite(&header, sizeof(header), 1, file);

    for(nkU64 i=0; i<item_count; ++i)
    {
        nkFileContent file_content = {0};
        nk_read_file_content(&file_content, items[i], nkFileReadMode_Binary);
        if(!file_content.data) return NK_FALSE;
        fwrite(file_content.data, file_content.size, 1, file);
        nk_free_file_content(&file_content);
    }

    for(nkU64 i=0; i<item_count; ++i)
    {
        nkNPAKEntry* entry = entries+i;
        fwrite(entry->name, strlen(entry->name)+1, 1, file);
        fwrite(&entry->offset, sizeof(entry->offset), 1, file);
        fwrite(&entry->size, sizeof(entry->size), 1, file);
    }

    fclose(file);

    // Free memory resources.
    NK_FREE(entries);
    nk_free_path_content(items, item_count);
    NK_FREE(clean_src_path);

    return NK_TRUE;
}

NKAPI nkBool nk_npak_unpack(const nkChar* npak_name, const nkChar* dst_path)
{
    // @Todo: Handling paths is quite gross right now.
    // @Todo: Custom memory allocators.

    nkNPAK npak = NK_ZERO_MEM;
    if(!nk_npak_load(&npak, npak_name)) return NK_FALSE;

    // If the destination doesn't exist we will create it.
    if(!nk_path_exists(dst_path))
    {
        if(!nk_create_path(dst_path))
            return NK_FALSE;
    }

    nkChar* clean_dst_path = NK_MALLOC_TYPES(nkChar, strlen(dst_path)+1);
    if(!clean_dst_path) return NK_FALSE;
    strcpy(clean_dst_path, dst_path);
    nk_fixup_path(&clean_dst_path);

    // Extract each of the file entries to disk.
    for(nkU64 i=0; i<npak.header->entries; ++i)
    {
        nkNPAKEntry* entry = &npak.entries[i];
        nkFileContent file = NK_ZERO_MEM;
        file.data = nk_npak_get_file_data(&npak, entry->name, &file.size);
        if(file.data)
        {
            nkChar file_name[1024] = NK_ZERO_MEM;
            nkChar path_name[1024] = NK_ZERO_MEM;

            strcpy(file_name, clean_dst_path);
            strcat(file_name, entry->name);

            strcpy(path_name, file_name);
            nk_strip_file_name(path_name);

            if(!nk_path_exists(path_name))
                if(!nk_create_path(path_name))
                    return NK_FALSE;
            nk_write_file_content(&file, file_name);
        }
    }

    // Free memory resources.
    NK_FREE(clean_dst_path);

    return NK_TRUE;
}

NKAPI nkBool nk_npak_load(nkNPAK* npak, const nkChar* npak_name)
{
    // @Todo: Custom memory allocators.

    NK_ASSERT(npak);

    // Load the entire NPAK into a single data blob.
    nkFileContent file_content = NK_ZERO_MEM;
    if(!nk_read_file_content(&file_content, npak_name, nkFileReadMode_Binary))
        return NK_FALSE;
    npak->data_blob = NK_CAST(nkU8*,file_content.data);

    // The header can just point into the blob.
    npak->header = NK_CAST(nkNPAKHeader*, npak->data_blob);

    // Do some validation to make sure everything is valid.
    if(npak->header->version != NK_NPAK_FILE_VERSION) return NK_FALSE;
    if(npak->header->fourcc != NK_NPAK_FILE_FOURCC) return NK_FALSE;

    // Build the map of entries.
    npak->entries = NK_MALLOC_TYPES(nkNPAKEntry, npak->header->entries);
    if(!npak->entries) return NK_FALSE;

    nkU64 current_offset = npak->header->table_offset;

    for(nkU64 i=0; i<npak->header->entries; ++i)
    {
        nkNPAKEntry* entry = npak->entries+i;
        entry->name   =  NK_CAST(const nkChar*, npak->data_blob+current_offset);
        current_offset += strlen(entry->name)+1;
        entry->offset = *NK_CAST(nkU64*,        npak->data_blob+current_offset);
        current_offset += sizeof(entry->offset);
        entry->size   = *NK_CAST(nkU64*,        npak->data_blob+current_offset);
        current_offset += sizeof(entry->offset);
    }

    return NK_TRUE;
}

NKAPI void nk_npak_free(nkNPAK* npak)
{
    // @Todo: Custom memory allocators.

    NK_ASSERT(npak);

    NK_FREE(npak->data_blob);
    NK_FREE(npak->entries);
}

NKAPI void* nk_npak_get_file_data(nkNPAK* npak, const nkChar* file_name, nkU64* size)
{
    NK_ASSERT(npak);
    NK_ASSERT(size);

    for(nkU64 i=0; i<npak->header->entries; ++i)
    {
        if(strcmp(file_name, npak->entries[i].name) == 0)
        {
            *size = npak->entries[i].size;
            return (npak->data_blob + npak->entries[i].offset);
        }
    }

    return NULL; // Couldn't find an entry with that name.
}

NKAPI nkNPAKEntry* nk_npak_get_file_meta(nkNPAK* npak, const nkChar* file_name)
{
    NK_ASSERT(npak);
    for(nkU64 i=0; i<npak->header->entries; ++i)
        if(strcmp(file_name, npak->entries[i].name) == 0)
            return &npak->entries[i];
    return NULL; // Couldn't find an entry with that name.
}

#endif /* NK_NPAK_IMPLEMENTATION /////////////////////////////////////////////*/

#endif /* NK_NPAK_H__ ////////////////////////////////////////////////////////*/

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
