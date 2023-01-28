/*////////////////////////////////////////////////////////////////////////////*/

// Only native builds use NPAK assets, the web has its own package format.
#if defined(BUILD_NATIVE)
#define USE_NPAK_ASSETS
#endif // BUILD_NATIVE

INTERNAL constexpr const nkChar* ASSET_PATH = "../../assets/";

struct AssetManager
{
    nkHashMap<nkString,AssetBase*> assets;
    nkNPAK                         npak;
    nkBool                         npak_loaded;
};

INTERNAL AssetManager g_asset_manager;

GLOBAL void init_asset_manager(void)
{
    #if defined(USE_NPAK_ASSETS)
    g_asset_manager.npak_loaded = nk_npak_load(&g_asset_manager.npak, "assets.npak");
    if(!g_asset_manager.npak_loaded)
        printf("[Assets]: Failed to load assets NPAK file!\n");
    #endif // USE_NPAK_ASSETS
}

GLOBAL void quit_asset_manager(void)
{
    // Free any assets that have not been cleaned up yet... the hash map will not handle
    // deleting our pointers for us so we have to do that manually anyway. Calling delete
    // will invoke the Asset<T> destructor, which will call the asset_free<T> function.
    for(auto& slot: g_asset_manager.assets)
        delete slot.value;
    nk_hashmap_free(&g_asset_manager.assets);

    #if defined(USE_NPAK_ASSETS)
    nk_npak_free(&g_asset_manager.npak);
    #endif // USE_NPAK_ASSETS
}

template<typename T>
GLOBAL T asset_manager_load(const nkChar* name, void* userdata, const nkChar* override_path)
{
    // If the asset has already been loaded then we can just return it.
    if(asset_manager_has<T>(name)) return asset_manager_get<T>(name);

    // Check the NPAK for assets first...
    nkBool loaded_from_npak = NK_FALSE;

    void* file_data = NULL;
    nkU64 file_size = 0;

    const nkChar* base_path = (override_path) ? override_path : asset_path<T>();
    nkString file_path = potentially_append_slash(base_path);
    nk_string_append(&file_path, name);

    if(g_asset_manager.npak_loaded)
    {
        file_data = nk_npak_get_file_data(&g_asset_manager.npak, file_path.cstr, &file_size);
        if(file_data)
        {
            loaded_from_npak = NK_TRUE;
        }
    }
    if(!file_data)
    {
        // ...if not found then check on disk as well.
        nk_string_insert(&file_path, 0, ASSET_PATH);
        if(nk_file_exists(file_path.cstr))
        {
            nkFileContent file_content = NK_ZERO_MEM;
            if(nk_read_file_content(&file_content, file_path.cstr, nkFileReadMode_Binary))
            {
                file_data = file_content.data;
                file_size = file_content.size;
            }
        }
    }

    // If still not found then we're done.
    if(!file_data)
    {
        printf("[Assets]: Could not find asset: %s\n", file_path.cstr);
        return NK_ZERO_MEM;
    }

    Asset<T>* asset = new Asset<T>;

    asset->file_name = name;
    asset->file_path = file_path;
    asset->file_size = file_size;
    asset->from_npak = loaded_from_npak;
    asset->data      = asset_load<T>(file_data, file_size, loaded_from_npak, userdata);

    nk_hashmap_insert(&g_asset_manager.assets, nkString(name), NK_CAST(AssetBase*, asset));

    printf("[Assets]: Loaded asset: %s\n", name);

    return asset->data;
}

template<typename T>
GLOBAL void asset_manager_free(const nkChar* name)
{
    if(!asset_manager_has<T>(name)) return; // Asset doesn't exist so it can't be freed.
    nkString asset_name = name;
    Asset<T>* asset = NK_CAST(Asset<T>*, nk_hashmap_getref(&g_asset_manager.assets, asset_name));
    delete asset;
    nk_hashmap_remove(&g_asset_manager.assets, asset_name);
}

template<typename T>
GLOBAL T asset_manager_get(const nkChar* name)
{
    if(!asset_manager_has<T>(name))
    {
        printf("[Assets]: Attempting to find data for unknown asset: %s\n", name);
        return NK_ZERO_MEM;
    }
    Asset<T>* asset = NK_CAST(Asset<T>*, nk_hashmap_getref(&g_asset_manager.assets, nkString(name)));
    return asset->data;
}

template<typename T>
GLOBAL nkBool asset_manager_has(const nkChar* name)
{
    return nk_hashmap_contains(&g_asset_manager.assets, nkString(name));
}

/*////////////////////////////////////////////////////////////////////////////*/
