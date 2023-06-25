#ifndef NK_FILESYS_H__ /*/////////////////////////////////////////////////////*/
#define NK_FILESYS_H__

#include "nk_define.h"

#include <stdarg.h>

NK_ENUM(nkFileReadMode, nkS32)
{
    nkFileReadMode_Text,
    nkFileReadMode_Binary
};

typedef struct nkFileContent
{
    void* data;
    nkU64 size;
}
nkFileContent;

NK_ENUM(nkPathListFlags, nkU32)
{
    nkPathListFlags_None      = (     0),
    nkPathListFlags_Recursive = (1 << 0),
    nkPathListFlags_Files     = (1 << 1),
    nkPathListFlags_Paths     = (1 << 2),
    nkPathListFlags_All       = (    -1)
};

NKAPI nkBool nk_read_file_content       (nkFileContent* content, const nkChar* file_name, nkFileReadMode mode);
NKAPI nkBool nk_write_file_content      (nkFileContent* content, const nkChar* file_name);
NKAPI void   nk_free_file_content       (nkFileContent* content);
NKAPI nkBool nk_get_exe_path            (nkChar* buffer, nkU64 max_size); // If (buffer==NULL) then we allocate a buffer for you.
NKAPI nkBool nk_get_data_path           (nkChar* buffer, nkU64 max_size); // If (buffer==NULL) then we allocate a buffer for you.
NKAPI nkBool nk_list_path_content       (const nkChar* path_name, nkPathListFlags flags, nkChar*** items, nkU64* item_count);
NKAPI void   nk_free_path_content       (nkChar** items, nkU64 item_count);
NKAPI nkBool nk_create_file             (const nkChar* file_name);
NKAPI nkBool nk_create_path             (const nkChar* path_name);
NKAPI nkBool nk_delete_file             (const nkChar* file_name);
NKAPI nkBool nk_delete_path             (const nkChar* path_name);
NKAPI nkBool nk_rename_file             (const nkChar* old_file, const nkChar* new_file);
NKAPI nkBool nk_rename_path             (const nkChar* old_path, const nkChar* new_path);
NKAPI nkBool nk_copy_file               (const nkChar* src_file, const nkChar* dst_file);
NKAPI nkBool nk_move_file               (const nkChar* src_file, const nkChar* dst_file);
NKAPI nkBool nk_strip_file_path         (nkChar* file_name); // "some/file/name.txt" => "name.txt"
NKAPI nkBool nk_strip_file_ext          (nkChar* file_name); // "some/file/name.txt" => "some/file/name"
NKAPI nkBool nk_strip_file_name         (nkChar* file_name); // "some/file/name.txt" => "some/file/"
NKAPI nkBool nk_strip_file_path_and_ext (nkChar* file_name); // "some/file/name.txt" => "name"
NKAPI nkBool nk_strip_file_path_and_name(nkChar* file_name); // "some/file/name.txt" => ".txt"
NKAPI nkBool nk_fixup_path              (nkChar** path_name);
NKAPI nkBool nk_make_path_absolute      (nkChar* buffer, nkU64 max_size, const nkChar* path_name);        // If (buffer==NULL) then we allocate a buffer for you.
NKAPI nkBool nk_join_paths              (nkChar* buffer, nkU64 max_size, nkU32 path_count, ...);          // If (buffer==NULL) then we allocate a buffer for you.
NKAPI nkBool nk_join_paths_v            (nkChar* buffer, nkU64 max_size, nkU32 path_count, va_list args); // If (buffer==NULL) then we allocate a buffer for you.
NKAPI nkU64  nk_file_size               (const nkChar* file_name);
NKAPI nkBool nk_file_exists             (const nkChar* file_name);
NKAPI nkBool nk_path_exists             (const nkChar* path_name);
NKAPI nkBool nk_is_file                 (const nkChar* file_name);
NKAPI nkBool nk_is_path                 (const nkChar* path_name);
NKAPI nkBool nk_is_path_absolute        (const nkChar* path_name);
NKAPI nkBool nk_is_path_relative        (const nkChar* path_name);

/*============================================================================*/
/*============================== IMPLEMENTATION ==============================*/
/*============================================================================*/

#ifdef NK_FILESYS_IMPLEMENTATION /*///////////////////////////////////////////*/

#ifdef NK_OS_WIN32 /* ------------------------------------------------------- */

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>

#include <stdlib.h>
#include <string.h>

// @Temporary: We should require these to be properly linked?
#pragma comment(lib, "shell32")
#pragma comment(lib, "shlwapi")

NKINTERNAL nkChar* nk__allocate_path_buffer(nkU64* max_size)
{
    // @Todo: Custom memory allocators.
    NK_ASSERT(max_size);
    nkChar* buffer = NK_MALLOC_TYPES(nkChar, MAX_PATH);
    (*max_size) = MAX_PATH;
    return buffer;
}

NKINTERNAL nkU64 nk__get_file_size(HANDLE file)
{
    LARGE_INTEGER file_size = NK_ZERO_MEM;
    GetFileSizeEx(file, &file_size);
    return file_size.QuadPart;
}

NKAPI nkBool nk_read_file_content(nkFileContent* content, const nkChar* file_name, nkFileReadMode mode)
{
    // @Todo: Custom memory allocators.

    NK_ASSERT(content);

    HANDLE file = CreateFileA(file_name, GENERIC_READ, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE) return NK_FALSE;

    nkBool success = NK_FALSE;

    nkU64 bytes_to_read = nk__get_file_size(file);

    content->size = bytes_to_read + ((mode == nkFileReadMode_Text) ? 1 : 0);
    content->data = NK_MALLOC_BYTES(content->size);
    if(content->data)
    {
        success = NK_CAST(nkBool, ReadFile(file, content->data, NK_CAST(DWORD,bytes_to_read), NULL, NULL));
        if(success && (mode == nkFileReadMode_Text))
            NK_CAST(nkChar*,content->data)[bytes_to_read] = '\0';
    }

    CloseHandle(file);

    return success;
}

NKAPI nkBool nk_write_file_content(nkFileContent* content, const nkChar* file_name)
{
    // @Incomplete: This should consider the null-terminator and not include it potentially.
    // If you were to use read_file_content and write_file_content over and over in a loop on
    // the same file it would most likely grow by one byte every time which isn't good.

    NK_ASSERT(content);

    HANDLE file = CreateFileA(file_name, GENERIC_WRITE, 0,
        NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE) return NK_FALSE;

    nkBool success = NK_FALSE;
    success = NK_CAST(nkBool, WriteFile(file, content->data, NK_CAST(DWORD,content->size), NULL, NULL));

    CloseHandle(file);

    return success;
}

NKAPI void nk_free_file_content(nkFileContent* content)
{
    // @Todo: Custom memory allocators.
    NK_ASSERT(content);
    NK_FREE(content->data);
    content->size = 0;
    content->data = NULL;
}

NKAPI nkBool nk_get_exe_path(nkChar* buffer, nkU64 max_size)
{
    // If we have been given a NULL buffer then we need to allocate one.
    if(!buffer) buffer = nk__allocate_path_buffer(&max_size);
    if(!buffer) return NK_FALSE;

    GetModuleFileNameA(NULL, buffer, NK_CAST(DWORD,max_size));

    nk_fixup_path(&buffer);
    nk_strip_file_name(buffer);

    return NK_TRUE;
}

NKAPI nkBool nk_get_data_path(nkChar* buffer, nkU64 max_size)
{
    // If we have been given a NULL buffer then we need to allocate one.
    if(!buffer) buffer = nk__allocate_path_buffer(&max_size);
    if(!buffer) return NK_FALSE;

    nkBool success = NK_FALSE;

    // We use a temporary buffer because the function requires MAX_PATH.
    nkChar temp_buffer[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, temp_buffer)))
    {
        nkU64 end = strlen(temp_buffer);
        if(end+1 < MAX_PATH) // Add a trailing slash if we have space.
        {
            temp_buffer[end+0] = '/';
            temp_buffer[end+1] = '\0';
        }

        strncpy(buffer, temp_buffer, max_size);

        nk_fixup_path(&buffer);

        success = NK_TRUE;
    }

    return success;
}

NKAPI nkBool nk_list_path_content(const nkChar* path_name, nkPathListFlags flags, nkChar*** items, nkU64* item_count)
{
    // @Todo: Handling paths is quite gross right now.
    // @Todo: Custom memory allocators.

    NK_ASSERT(item_count);

    nkChar* clean_path_name = NK_MALLOC_TYPES(nkChar, strlen(path_name)+1);
    if(!clean_path_name) return NK_FALSE;
    strcpy(clean_path_name, path_name);
    nk_fixup_path(&clean_path_name);

    nkChar* find_path_name = NK_MALLOC_TYPES(nkChar, strlen(clean_path_name)+2);
    if(!find_path_name) return NK_FALSE;
    strcpy(find_path_name, clean_path_name);
    nkU64 find_path_name_length = strlen(find_path_name);

    find_path_name[find_path_name_length-1] = '\\';
    find_path_name[find_path_name_length  ] = '*';
    find_path_name[find_path_name_length+1] = '\0';

    nkChar** item_buffer = (*items);

    // Gets the first file/dir in the directory.
    WIN32_FIND_DATAA file_data = NK_ZERO_MEM;
    HANDLE find_file = FindFirstFileA(find_path_name, &file_data);

    if(find_file != INVALID_HANDLE_VALUE)
    {
        do
        {
            // Prevents listing self and parent dir in content.
            const nkChar* item_name = file_data.cFileName;
            if((strcmp(item_name, ".") != 0) && (strcmp(item_name, "..") != 0))
            {
                nkU64 item_length = strlen(clean_path_name) + strlen(item_name) + 1;
                nkChar* full_item_name = NK_MALLOC_TYPES(nkChar, item_length);
                if(!full_item_name)
                {
                    break; // Failed.
                }
                else
                {
                    strcpy(full_item_name, clean_path_name);
                    strcat(full_item_name, item_name);
                }

                // Determine if the caller wants this item added to the list.
                nkBool should_add_item = NK_FALSE;
                if((nk_is_path(full_item_name) && NK_CHECK_FLAGS(flags, nkPathListFlags_Paths)) ||
                   (nk_is_file(full_item_name) && NK_CHECK_FLAGS(flags, nkPathListFlags_Files)))
                {
                    should_add_item = NK_TRUE;
                }
                if(should_add_item)
                {
                    // Allocates space for the item in the list.
                    (*item_count)++;
                    nkChar** temp_list = NK_RALLOC_TYPES(nkChar*, item_buffer, *item_count);
                    if(!temp_list)
                    {
                        break; // Failed.
                    }
                    else
                    {
                        // Add the item to the new list entry.
                        nkU64 index = (*item_count) - 1;
                        item_buffer = temp_list;
                        item_buffer[index] = full_item_name;
                    }
                }

                // If the caller wants recursive results then explore.
                if(NK_CHECK_FLAGS(flags, nkPathListFlags_Recursive) && nk_is_path(full_item_name))
                {
                    nk_list_path_content(full_item_name, flags, &item_buffer, item_count);
                }

                if(!should_add_item)
                {
                    NK_FREE(full_item_name);
                }
            }
        }
        while(FindNextFileA(find_file, &file_data));

        // Fixup all of the paths.
        for(nkU64 i=0; i<(*item_count); ++i)
        {
            nk_fixup_path(&item_buffer[i]);
        }
    }

    FindClose(find_file);

    (*items) = item_buffer;

    return NK_TRUE;
}

NKAPI void nk_free_path_content(nkChar** items, nkU64 item_count)
{
    // @Todo: Custom memory allocators.
    NK_ASSERT(items);
    for(nkU64 i=0; i<item_count; ++i)
        NK_FREE(items[i]);
    NK_FREE(items);
}

NKAPI nkBool nk_create_file(const nkChar* file_name)
{
    HANDLE file = CreateFileA(file_name, GENERIC_READ, 0,
        NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    CloseHandle(file);
    return (file != INVALID_HANDLE_VALUE);
}

NKAPI nkBool nk_create_path(const nkChar* path_name)
{
    nkChar buffer[MAX_PATH] = NK_ZERO_MEM;
    nk_make_path_absolute(buffer, NK_ARRAY_SIZE(buffer), path_name);
    return (SHCreateDirectoryExA(NULL, buffer, NULL) == ERROR_SUCCESS);
}

NKAPI nkBool nk_delete_file(const nkChar* file_name)
{
    return NK_CAST(nkBool, DeleteFileA(file_name));
}

NKAPI nkBool nk_delete_path(const nkChar* path_name)
{
    SHFILEOPSTRUCTA fileop = NK_ZERO_MEM;
    fileop.hwnd = NULL;
    fileop.wFunc = FO_DELETE;
    fileop.pFrom = path_name;
    fileop.pTo = NULL;
    fileop.fFlags = FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_SILENT;
    fileop.fAnyOperationsAborted = NK_FALSE;
    fileop.hNameMappings = 0;
    fileop.lpszProgressTitle = NULL;
    return (SHFileOperationA(&fileop) == 0);
}

NKAPI nkBool nk_rename_file(const nkChar* old_file, const nkChar* new_file)
{
    return NK_CAST(nkBool, MoveFileA(old_file, new_file)); // Renaming is just implemented as moving.
}

NKAPI nkBool nk_rename_path(const nkChar* old_path, const nkChar* new_path)
{
    return NK_CAST(nkBool, MoveFileA(old_path, new_path)); // Renaming is just implemented as moving.
}

NKAPI nkBool nk_copy_file(const nkChar* src_file, const nkChar* dst_file)
{
    return NK_CAST(nkBool, CopyFileA(src_file, dst_file, NK_FALSE));
}

NKAPI nkBool nk_move_file(const nkChar* src_file, const nkChar* dst_file)
{
    return NK_CAST(nkBool, MoveFileA(src_file, dst_file));
}

NKAPI nkBool nk_strip_file_path(nkChar* file_name)
{
    nkChar* name = PathFindFileNameA(file_name);
    if(name != file_name) strcpy(file_name, name);
    return NK_TRUE;
}

NKAPI nkBool nk_strip_file_ext(nkChar* file_name)
{
    PathFindExtensionA(file_name)[0] = '\0';
    return NK_TRUE;
}

NKAPI nkBool nk_strip_file_name(nkChar* file_name)
{
    PathFindFileNameA(file_name)[0] = '\0';
    return NK_TRUE;
}

NKAPI nkBool nk_strip_file_path_and_ext(nkChar* file_name)
{
    nkChar* name = PathFindFileNameA(file_name);
    if(name != file_name) strcpy(file_name, name);
    PathFindExtensionA(file_name)[0] = '\0';
    return NK_TRUE;
}

NKAPI nkBool nk_strip_file_path_and_name(nkChar* file_name)
{
    nkChar* ext = PathFindExtensionA(file_name);
    if(ext[0] != '\0' && ext != file_name)
        strcpy(file_name, ext);
    return NK_TRUE;
}

NKAPI nkBool nk_fixup_path(nkChar** path_name)
{
    // @Improve: We want a version of this that doesn't reallocate (e.g. for static length strings).
    // @Todo: Custom memory allocators.

    // Fix up the slashes inside of the path.
    nkChar* path = (*path_name);
    for(nkChar* c=path; *c; ++c)
    {
        if(*c == '\\') *c = '/';
    }

    // Append slash if there isn't one and we're a path.
    if(nk_is_path(path))
    {
        nkU64 path_length = strlen(path);
        if(path_length && path[path_length-1] != '/')
        {
            nkChar* new_path = NK_RALLOC_TYPES(nkChar, path, path_length+2);
            if(!new_path)
                return NK_FALSE;
            new_path[path_length  ] = '/';
            new_path[path_length+1] = '\0';
            (*path_name) = new_path;
        }
    }

    return NK_TRUE;
}

NKAPI nkBool nk_make_path_absolute(nkChar* buffer, nkU64 max_size, const nkChar* path_name)
{
    // If we have been given a NULL buffer then we need to allocate one.
    if(!buffer) buffer = nk__allocate_path_buffer(&max_size);
    if(!buffer) return NK_FALSE;

    GetFullPathNameA(path_name, NK_CAST(DWORD,max_size), buffer, NULL);

    return NK_TRUE;
}

NKAPI nkBool nk_join_paths(nkChar* buffer, nkU64 max_size, nkU32 path_count, ...)
{
    // @Incomplete: ...
    NK_UNUSED(buffer);
    NK_UNUSED(max_size);
    NK_UNUSED(path_count);
    NK_ASSERT(NK_FALSE);
    return NK_FALSE;
}

NKAPI nkBool nk_join_paths_v(nkChar* buffer, nkU64 max_size, nkU32 path_count, va_list args)
{
    // @Incomplete: ...
    NK_UNUSED(buffer);
    NK_UNUSED(max_size);
    NK_UNUSED(path_count);
    NK_UNUSED(args);
    NK_ASSERT(NK_FALSE);
    return NK_FALSE;
}

NKAPI nkU64 nk_file_size(const nkChar* file_name)
{
    HANDLE file = CreateFileA(file_name, GENERIC_READ, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE) return 0;
    nkU64 file_size = nk__get_file_size(file);
    CloseHandle(file);
    return file_size;
}

NKAPI nkBool nk_file_exists(const nkChar* file_name)
{
    DWORD attribs = GetFileAttributesA(file_name);
    return ((attribs != INVALID_FILE_ATTRIBUTES) &&
           !(attribs & FILE_ATTRIBUTE_DIRECTORY));
}

NKAPI nkBool nk_path_exists(const nkChar* path_name)
{
    DWORD attribs = GetFileAttributesA(path_name);
    return ((attribs != INVALID_FILE_ATTRIBUTES) &&
            (attribs & FILE_ATTRIBUTE_DIRECTORY));
}

NKAPI nkBool nk_is_file(const nkChar* file_name)
{
    // This is the same as nk_file_exists but sometimes this makes more sense to show the intention.
    return nk_file_exists(file_name);
}

NKAPI nkBool nk_is_path(const nkChar* path_name)
{
    // This is the same as nk_path_exists but sometimes this makes more sense to show the intention.
    return nk_path_exists(path_name);
}

NKAPI nkBool nk_is_path_absolute(const nkChar* path_name)
{
    return !PathIsRelativeA(path_name);
}

NKAPI nkBool nk_is_path_relative(const nkChar* path_name)
{
    return NK_CAST(nkBool, PathIsRelativeA(path_name));
}

#endif /* NK_OS_WIN32 ------------------------------------------------------- */

#ifdef NK_OS_MACOS /* ------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NKINTERNAL nkU64 nk__get_file_size(FILE* file)
{
    fseek(file, 0L, SEEK_END);
    nkU64 file_size = ftell(file);
    rewind(file);
    return file_size;
}

NKAPI nkBool nk_read_file_content(nkFileContent* content, const nkChar* file_name, nkFileReadMode mode)
{
    // @Todo: Custom memory allocators.

    NK_ASSERT(content);

    FILE* file = fopen(file_name, "rb");
    if(!file) return NK_FALSE;

    nkBool success = NK_FALSE;

    nkU64 bytes_to_read = nk__get_file_size(file);

    content->size = bytes_to_read + ((mode == nkFileReadMode_Text) ? 1 : 0);
    content->data = NK_MALLOC_BYTES(content->size);
    if(content->data)
    {
        success = (fread(content->data, bytes_to_read, 1, file) == 1);
        if(success && (mode == nkFileReadMode_Text))
            NK_CAST(nkChar*,content->data)[bytes_to_read] = '\0';
    }

    fclose(file);

    return success;
}

NKAPI nkBool nk_write_file_content(nkFileContent* content, const nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI void nk_free_file_content(nkFileContent* content)
{
    // @Todo: Custom memory allocators.
    NK_ASSERT(content);
    NK_FREE(content->data);
    content->size = 0;
    content->data = NULL;
}

NKAPI nkBool nk_get_exe_path(nkChar* buffer, nkU64 max_size)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_get_data_path(nkChar* buffer, nkU64 max_size)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_list_path_content(const nkChar* path_name, nkPathListFlags flags, nkChar*** items, nkU64* item_count)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI void nk_free_path_content(nkChar** items, nkU64 item_count)
{
    // @Incomplete: ...
}

NKAPI nkBool nk_create_file(const nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_create_path(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_delete_file(const nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_delete_path(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_rename_file(const nkChar* old_file, const nkChar* new_file)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_rename_path(const nkChar* old_path, const nkChar* new_path)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_copy_file(const nkChar* src_file, const nkChar* dst_file)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_move_file(const nkChar* src_file, const nkChar* dst_file)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_path(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_ext(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_name(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_path_and_ext(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_path_and_name(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_fixup_path(nkChar** path_name)
{
    // @Improve: We want a version of this that doesn't reallocate (e.g. for static length strings).
    // @Todo: Custom memory allocators.

    // Fix up the slashes inside of the path.
    nkChar* path = (*path_name);
    for(nkChar* c=path; *c; ++c)
    {
        if(*c == '\\') *c = '/';
    }

    // Append slash if there isn't one and we're a path.
    if(nk_is_path(path))
    {
        nkU64 path_length = strlen(path);
        if(path_length && path[path_length-1] != '/')
        {
            nkChar* new_path = NK_RALLOC_TYPES(nkChar, path, path_length+2);
            if(!new_path)
                return NK_FALSE;
            new_path[path_length  ] = '/';
            new_path[path_length+1] = '\0';
            (*path_name) = new_path;
        }
    }

    return NK_TRUE;
}

NKAPI nkBool nk_make_path_absolute(nkChar* buffer, nkU64 max_size, const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_join_paths(nkChar* buffer, nkU64 max_size, nkU32 path_count, ...)
{
    // @Incomplete: ...
    NK_UNUSED(buffer);
    NK_UNUSED(max_size);
    NK_UNUSED(path_count);
    NK_ASSERT(NK_FALSE);
    return NK_FALSE;
}

NKAPI nkBool nk_join_paths_v(nkChar* buffer, nkU64 max_size, nkU32 path_count, va_list args)
{
    // @Incomplete: ...
    NK_UNUSED(buffer);
    NK_UNUSED(max_size);
    NK_UNUSED(path_count);
    NK_UNUSED(args);
    NK_ASSERT(NK_FALSE);
    return NK_FALSE;
}

NKAPI nkU64 nk_file_size(const nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_file_exists(const nkChar* file_name)
{
    // @Incomplete: This is a poor way of checking a file exists...
    FILE* f = fopen(file_name, "r");
    nkBool exists = (f != NULL);
    if(f) fclose(f);
    return exists;
}

NKAPI nkBool nk_path_exists(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_is_file(const nkChar* file_name)
{
    // This is the same as nk_file_exists but sometimes this makes more sense to show the intention.
    return nk_file_exists(file_name);
}

NKAPI nkBool nk_is_path(const nkChar* path_name)
{
    // This is the same as nk_path_exists but sometimes this makes more sense to show the intention.
    return nk_path_exists(path_name);
}

NKAPI nkBool nk_is_path_absolute(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_is_path_relative(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

#endif /* NK_OS_MACOS ------------------------------------------------------- */

#ifdef NK_OS_WEB /* --------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

NKINTERNAL nkU64 nk__get_file_size(FILE* file)
{
    fseek(file, 0L, SEEK_END);
    nkU64 file_size = ftell(file);
    rewind(file);
    return file_size;
}

NKAPI nkBool nk_read_file_content(nkFileContent* content, const nkChar* file_name, nkFileReadMode mode)
{
    // @Todo: Custom memory allocators.

    NK_ASSERT(content);

    FILE* file = fopen(file_name, "rb");
    if(!file) return NK_FALSE;

    nkBool success = NK_FALSE;

    nkU64 bytes_to_read = nk__get_file_size(file);

    content->size = bytes_to_read + ((mode == nkFileReadMode_Text) ? 1 : 0);
    content->data = NK_MALLOC_BYTES(content->size);
    if(content->data)
    {
        success = (fread(content->data, bytes_to_read, 1, file) == 1);
        if(success && (mode == nkFileReadMode_Text))
            NK_CAST(nkChar*,content->data)[bytes_to_read] = '\0';
    }

    fclose(file);

    return success;
}

NKAPI nkBool nk_write_file_content(nkFileContent* content, const nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI void nk_free_file_content(nkFileContent* content)
{
    // @Todo: Custom memory allocators.
    NK_ASSERT(content);
    NK_FREE(content->data);
    content->size = 0;
    content->data = NULL;
}

NKAPI nkBool nk_get_exe_path(nkChar* buffer, nkU64 max_size)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_get_data_path(nkChar* buffer, nkU64 max_size)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_list_path_content(const nkChar* path_name, nkPathListFlags flags, nkChar*** items, nkU64* item_count)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI void nk_free_path_content(nkChar** items, nkU64 item_count)
{
    // @Incomplete: ...
}

NKAPI nkBool nk_create_file(const nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_create_path(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_delete_file(const nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_delete_path(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_rename_file(const nkChar* old_file, const nkChar* new_file)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_rename_path(const nkChar* old_path, const nkChar* new_path)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_copy_file(const nkChar* src_file, const nkChar* dst_file)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_move_file(const nkChar* src_file, const nkChar* dst_file)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_path(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_ext(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_name(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_path_and_ext(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_strip_file_path_and_name(nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_fixup_path(nkChar** path_name)
{
    // @Improve: We want a version of this that doesn't reallocate (e.g. for static length strings).
    // @Todo: Custom memory allocators.

    // Fix up the slashes inside of the path.
    nkChar* path = (*path_name);
    for(nkChar* c=path; *c; ++c)
    {
        if(*c == '\\') *c = '/';
    }

    // Append slash if there isn't one and we're a path.
    if(nk_is_path(path))
    {
        nkU64 path_length = strlen(path);
        if(path_length && path[path_length-1] != '/')
        {
            nkChar* new_path = NK_RALLOC_TYPES(nkChar, path, path_length+2);
            if(!new_path)
                return NK_FALSE;
            new_path[path_length  ] = '/';
            new_path[path_length+1] = '\0';
            (*path_name) = new_path;
        }
    }

    return NK_TRUE;
}

NKAPI nkBool nk_make_path_absolute(nkChar* buffer, nkU64 max_size, const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_join_paths(nkChar* buffer, nkU64 max_size, nkU32 path_count, ...)
{
    // @Incomplete: ...
    NK_UNUSED(buffer);
    NK_UNUSED(max_size);
    NK_UNUSED(path_count);
    NK_ASSERT(NK_FALSE);
    return NK_FALSE;
}

NKAPI nkBool nk_join_paths_v(nkChar* buffer, nkU64 max_size, nkU32 path_count, va_list args)
{
    // @Incomplete: ...
    NK_UNUSED(buffer);
    NK_UNUSED(max_size);
    NK_UNUSED(path_count);
    NK_UNUSED(args);
    NK_ASSERT(NK_FALSE);
    return NK_FALSE;
}

NKAPI nkU64 nk_file_size(const nkChar* file_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_file_exists(const nkChar* file_name)
{
    // @Incomplete: This is a poor way of checking a file exists...
    FILE* f = fopen(file_name, "r");
    nkBool exists = (f != NULL);
    if(f) fclose(f);
    return exists;
}

NKAPI nkBool nk_path_exists(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_is_file(const nkChar* file_name)
{
    // This is the same as nk_file_exists but sometimes this makes more sense to show the intention.
    return nk_file_exists(file_name);
}

NKAPI nkBool nk_is_path(const nkChar* path_name)
{
    // This is the same as nk_path_exists but sometimes this makes more sense to show the intention.
    return nk_path_exists(path_name);
}

NKAPI nkBool nk_is_path_absolute(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

NKAPI nkBool nk_is_path_relative(const nkChar* path_name)
{
    // @Incomplete: ...
    return NK_FALSE;
}

#endif /* NK_OS_WEB --------------------------------------------------------- */

#endif /* NK_FILESYS_IMPLEMENTATION //////////////////////////////////////////*/

#endif /* NK_FILESYS_H__ /////////////////////////////////////////////////////*/

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
