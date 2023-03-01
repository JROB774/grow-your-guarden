/*////////////////////////////////////////////////////////////////////////////*/

#include <SDL_syswm.h> // For getting the HWND from the SDL window.

#include <d3d11.h>

DEFINE_PRIVATE_TYPE(VertexBuffer)
{
    // @Incomplete: ...
};

DEFINE_PRIVATE_TYPE(RenderTarget)
{
    // @Incomplete: ...
};

DEFINE_PRIVATE_TYPE(Shader)
{
    // @Incomplete: ...
};

DEFINE_PRIVATE_TYPE(Texture)
{
    // @Incomplete: ...
};

struct Direct3DContext
{
    ID3D11Device*           device;
    ID3D11DeviceContext*    device_context;
    IDXGISwapChain*         swap_chain;
    ID3D11RenderTargetView* render_target;
};

INTERNAL Direct3DContext g_d3d;

GLOBAL void init_render_system(void)
{
    HWND hwnd = NULL;

    // Extract the HWND from the SDL window so we can use it for setting up D3D.
    SDL_SysWMinfo win_info = NK_ZERO_MEM;
    SDL_VERSION(&win_info.version);
    if(SDL_GetWindowWMInfo(NK_CAST(SDL_Window*, get_window()), &win_info))
        hwnd = win_info.info.win.window;;
    if(!hwnd)
    {
        fatal_error("Could not get SDL window internal handle!");
    }

    // Setup our D3D11 rendering context.
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };

    nkU32 device_flags = 0;
    #if defined(BUILD_DEBUG)
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif // BUILD_DEBUG

    HRESULT res;

    res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, device_flags, feature_levels,
        NK_ARRAY_SIZE(feature_levels), D3D11_SDK_VERSION, &g_d3d.device, NULL, &g_d3d.device_context);
    if(!SUCCEEDED(res))
    {
        fatal_error("Failed to create D3D11 device context!");
    }

    IDXGIDevice*  dxgi_device  = NULL;
    IDXGIAdapter* dxgi_adapter = NULL;
    IDXGIFactory* dxgi_factory = NULL;

    g_d3d.device->QueryInterface(__uuidof(IDXGIDevice ), NK_CAST(void**,&dxgi_device ));
    dxgi_device->GetParent(      __uuidof(IDXGIAdapter), NK_CAST(void**,&dxgi_adapter));
    dxgi_adapter->GetParent(     __uuidof(IDXGIFactory), NK_CAST(void**,&dxgi_factory));

    // Setup our swap chain.
    DXGI_SWAP_CHAIN_DESC swap_chain_desc               = NK_ZERO_MEM;
    swap_chain_desc.BufferDesc.Width                   = get_window_width();
    swap_chain_desc.BufferDesc.Height                  = get_window_height();
    swap_chain_desc.BufferDesc.RefreshRate.Numerator   = NK_CAST(nkU32,get_app_desc()->tick_rate); // Our preferred refresh rate without VSync.
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count                   = 1; // We don't care about multi-sampling.
    swap_chain_desc.SampleDesc.Quality                 = 0;
    swap_chain_desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount                        = 2; // Double-buffered rendering.
    swap_chain_desc.OutputWindow                       = hwnd;
    swap_chain_desc.Windowed                           = NK_TRUE;
    swap_chain_desc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    res = dxgi_factory->CreateSwapChain(g_d3d.device, &swap_chain_desc, &g_d3d.swap_chain);
    if(!SUCCEEDED(res))
    {
        fatal_error("Failed to create D3D11 swap chain!");
    }

    // Setup our main render target.
    ID3D11Texture2D* backbuffer;
    g_d3d.swap_chain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));
    g_d3d.device->CreateRenderTargetView(backbuffer, NULL, &g_d3d.render_target);
    backbuffer->Release();

    // Disable DXGI intercepting window events as it is annoying.
    dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES);
}

GLOBAL void quit_render_system(void)
{
    if(g_d3d.render_target)
    {
        g_d3d.render_target->Release();
        g_d3d.render_target = NULL;
    }
    if(g_d3d.swap_chain)
    {
        g_d3d.swap_chain->Release();
        g_d3d.swap_chain = NULL;
    }
    if(g_d3d.device_context)
    {
        g_d3d.device_context->Release();
        g_d3d.device_context = NULL;
    }
    if(g_d3d.device)
    {
        g_d3d.device->Release();
        g_d3d.device = NULL;
    }
}

GLOBAL void setup_renderer_platform(void)
{
    // Does nothing...
}

GLOBAL void renderer_present(void)
{
    g_d3d.swap_chain->Present(1, 0); // Present with Vsync.
}

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    // @Incomplete: ...
}

GLOBAL void set_blend_mode(BlendMode blend_mode)
{
    // @Incomplete: ...
}

GLOBAL void begin_scissor(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    // @Incomplete: ...
}

GLOBAL void end_scissor(void)
{
    // @Incomplete: ...
}

GLOBAL void clear_screen(nkVec4 color)
{
    // @Incomplete: ...
}

GLOBAL void clear_screen(nkVec3 color)
{
    // @Incomplete: ...
}

GLOBAL void clear_screen(nkF32 r, nkF32 g, nkF32 b, nkF32 a)
{
    // @Incomplete: ...
}

GLOBAL VertexBuffer create_vertex_buffer(void)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_vertex_buffer(VertexBuffer vbuf)
{
    // @Incomplete: ...
}

GLOBAL void set_vertex_buffer_stride(VertexBuffer vbuf, nkU64 byte_stride)
{
    // @Incomplete: ...
}

GLOBAL void enable_vertex_buffer_attrib(VertexBuffer vbuf, nkU32 index, AttribType type, nkU32 comps, nkU64 byte_offset)
{
    // @Incomplete: ...
}

GLOBAL void disable_vertex_buffer_attrib(VertexBuffer vbuf, nkU32 index)
{
    // @Incomplete: ...
}

GLOBAL void update_vertex_buffer(VertexBuffer vbuf, void* data, nkU64 bytes, BufferType type)
{
    // @Incomplete: ...
}

GLOBAL void draw_vertex_buffer(VertexBuffer vbuf, DrawMode draw_mode, nkU64 vert_count)
{
    // @Incomplete: ...
}

GLOBAL RenderTarget create_render_target(nkS32 w, nkS32 h, SamplerFilter filter, SamplerWrap wrap)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_render_target(RenderTarget target)
{
    // @Incomplete: ...
}

GLOBAL void resize_render_target(RenderTarget target, nkS32 w, nkS32 h)
{
    // @Incomplete: ...
}

GLOBAL void bind_render_target(RenderTarget target)
{
    // @Incomplete: ...
}

GLOBAL Shader create_shader(void* data, nkU64 bytes)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_shader(Shader shader)
{
    // @Incomplete: ...
}

GLOBAL void bind_shader(Shader shader)
{
    // @Incomplete: ...
}

GLOBAL void set_shader_bool(Shader shader, const nkChar* name, nkBool val)
{
    // @Incomplete: ...
}

GLOBAL void set_shader_int(Shader shader, const nkChar* name, nkS32 val)
{
    // @Incomplete: ...
}

GLOBAL void set_shader_float(Shader shader, const nkChar* name, nkF32 val)
{
    // @Incomplete: ...
}

GLOBAL void set_shader_vec2(Shader shader, const nkChar* name, nkVec2 val)
{
    // @Incomplete: ...
}

GLOBAL void set_shader_vec3(Shader shader, const nkChar* name, nkVec3 val)
{
    // @Incomplete: ...
}

GLOBAL void set_shader_vec4(Shader shader, const nkChar* name, nkVec4 val)
{
    // @Incomplete: ...
}

GLOBAL void set_shader_mat2(Shader shader, const nkChar* name, nkMat2 val)
{
    // @Incomplete: ...
}

GLOBAL void set_shader_mat3(Shader shader, const nkChar* name, nkMat3 val)
{
    // @Incomplete: ...
}

GLOBAL void set_shader_mat4(Shader shader, const nkChar* name, nkMat4 val)
{
    // @Incomplete: ...
}

GLOBAL Texture create_texture(nkS32 w, nkS32 h, nkS32 bpp, void* data, SamplerFilter filter, SamplerWrap wrap)
{
    // @Incomplete: ...
    return NULL;
}

GLOBAL void free_texture(Texture texture)
{
    // @Incomplete: ...
}

GLOBAL void bind_texture(Texture texture, nkS32 unit)
{
    // @Incomplete: ...
}

GLOBAL nkVec2 get_texture_size(Texture texture)
{
    // @Incomplete: ...
    return NK_V2_ZERO;
}

GLOBAL nkS32 get_texture_width(Texture texture)
{
    // @Incomplete: ...
    return 0;
}

GLOBAL nkS32 get_texture_height(Texture texture)
{
    // @Incomplete: ...
    return 0;
}

/*////////////////////////////////////////////////////////////////////////////*/
