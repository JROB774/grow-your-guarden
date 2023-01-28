/*////////////////////////////////////////////////////////////////////////////*/

// Base asset type, all assets should be specializations of Asset<T>.
struct AssetBase
{
    nkString file_name;
    nkString file_path;
    nkU64    file_size;
    nkBool   from_npak;

    virtual ~AssetBase(void) {}
};
template<typename T>
struct Asset: AssetBase
{
    T data;

    ~Asset<T>(void);
};

GLOBAL void init_asset_manager(void);
GLOBAL void quit_asset_manager(void);

template<typename T> GLOBAL T      asset_manager_load(const nkChar* name, void* userdata = NULL, const nkChar* override_path = NULL); // Use userdata to pass extra data through to asset_load.
template<typename T> GLOBAL void   asset_manager_free(const nkChar* name);
template<typename T> GLOBAL T      asset_manager_get (const nkChar* name);
template<typename T> GLOBAL nkBool asset_manager_has (const nkChar* name);

// Specialize and create these functions with your own T to implement a new asset type.
template<typename T> GLOBAL T             asset_load(void* data, nkU64 size, nkBool from_npak, void* userdata);
template<typename T> GLOBAL void          asset_free(Asset<T>& asset);
template<typename T> GLOBAL const nkChar* asset_path(void);

template<typename T>
Asset<T>::~Asset(void)
{
    asset_free<T>(*this);
}

/*////////////////////////////////////////////////////////////////////////////*/

template<>
nkFileContent asset_load<nkFileContent>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    nkFileContent file_content = { data, size };
    return file_content;
}
template<>
void asset_free<nkFileContent>(Asset<nkFileContent>& asset)
{
    if(!asset.from_npak) nk_free_file_content(&asset.data);
}
template<>
const nkChar* asset_path<nkFileContent>(void)
{
    return "defines/";
}

/*////////////////////////////////////////////////////////////////////////////*/
