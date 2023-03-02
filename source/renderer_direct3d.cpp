/*////////////////////////////////////////////////////////////////////////////*/

#include <SDL_syswm.h> // For getting the HWND from the SDL window.

#include <d3d11.h>
#include <d3dcompiler.h> // @Incomplete: Would we need to ship with the compiler?

INTERNAL constexpr nkU64 MAX_CONSTANTS_SIZE = 256;

DEFINE_PRIVATE_TYPE(VertexBuffer)
{
    ID3D11Buffer* buffer;
    nkU64         size;
    nkArray<nkU8> data;
    nkU64         byte_stride;
    VertexAttrib  attribs[16];
};

DEFINE_PRIVATE_TYPE(RenderTarget)
{
    // Not going to bother implementing as the game doesn't use it...
};

DEFINE_PRIVATE_TYPE(Shader)
{
    ID3D11VertexShader* vert_shader;
    ID3DBlob*           vert_blob;
    ID3D11PixelShader*  frag_shader;
    ID3DBlob*           frag_blob;
};

DEFINE_PRIVATE_TYPE(Texture)
{
    ID3D11Texture2D*          texture;
    ID3D11ShaderResourceView* shader_view;
    ID3D11SamplerState*       sampler;
    nkVec2                    size;
};

struct Direct3DContext
{
    ID3D11Device*           device;
    ID3D11DeviceContext*    device_context;
    IDXGISwapChain*         swap_chain;
    nkS32                   backbuffer_w;
    nkS32                   backbuffer_h;
    ID3D11RenderTargetView* backbuffer;
    ID3D11Texture2D*        backbuffer_texture;
    ID3D11RasterizerState*  rasterizer_state;
    ID3D11BlendState*       blend_state;
    ID3D11Buffer*           constants;
    fRect                   viewport;
    ID3D11RenderTargetView* current_target;
    Shader                  current_shader;
    Texture                 current_texture[8];
};

INTERNAL Direct3DContext g_d3d;

INTERNAL const nkChar* attrib_semantic_to_d3d_name(AttribSemantic semantic)
{
    switch(semantic)
    {
        case AttribSemantic_Pos: return "POS";
        case AttribSemantic_Tex: return "TEXCOORD";
        case AttribSemantic_Col: return "COL";
    }
    NK_ASSERT(NK_FALSE);
    return NULL;
}

INTERNAL DXGI_FORMAT attrib_type_to_d3d_format(AttribType type)
{
    switch(type)
    {
        case AttribType_Float1: return DXGI_FORMAT_R32_FLOAT; break;
        case AttribType_Float2: return DXGI_FORMAT_R32G32_FLOAT; break;
        case AttribType_Float3: return DXGI_FORMAT_R32G32B32_FLOAT; break;
        case AttribType_Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT; break;
    }
    NK_ASSERT(NK_FALSE);
    return DXGI_FORMAT_UNKNOWN;
}

INTERNAL DXGI_FORMAT bpp_to_d3d_format(nkS32 bpp)
{
    switch(bpp)
    {
        case 1: return DXGI_FORMAT_R8_UNORM;
        case 2: return DXGI_FORMAT_R8G8_UNORM;
        case 4: return DXGI_FORMAT_R8G8B8A8_UNORM;
    }
    NK_ASSERT(NK_FALSE);
    return DXGI_FORMAT_UNKNOWN;
}

INTERNAL D3D11_FILTER sampler_filter_to_d3d(SamplerFilter filter)
{
    switch(filter)
    {
        case SamplerFilter_Nearest: return D3D11_FILTER_MIN_MAG_MIP_POINT;
        case SamplerFilter_Linear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }
    NK_ASSERT(NK_FALSE);
    return D3D11_FILTER_MIN_MAG_MIP_POINT;
}

INTERNAL D3D11_TEXTURE_ADDRESS_MODE sampler_wrap_to_d3d(SamplerWrap wrap)
{
    switch(wrap)
    {
        case SamplerWrap_Repeat: return D3D11_TEXTURE_ADDRESS_WRAP;
        case SamplerWrap_Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
    }
    NK_ASSERT(NK_FALSE);
    return D3D11_TEXTURE_ADDRESS_WRAP;
}

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
        fatal_error("Failed to create Direct3D context!");
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
    swap_chain_desc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    res = dxgi_factory->CreateSwapChain(g_d3d.device, &swap_chain_desc, &g_d3d.swap_chain);
    if(!SUCCEEDED(res))
    {
        fatal_error("Failed to create Direct3D swap chain!");
    }

    // Setup our main backbuffer render target.
    g_d3d.swap_chain->GetBuffer(0, IID_PPV_ARGS(&g_d3d.backbuffer_texture));
    res = g_d3d.device->CreateRenderTargetView(g_d3d.backbuffer_texture, NULL, &g_d3d.backbuffer);
    if(!SUCCEEDED(res))
        fatal_error("Failed to create Direct3D backbuffer!");
    g_d3d.backbuffer_w = get_window_width();
    g_d3d.backbuffer_h = get_window_height();

    // Set the backbuffer as our initial/main target.
    g_d3d.device_context->OMSetRenderTargets(1, &g_d3d.backbuffer, NULL);
    g_d3d.current_target = g_d3d.backbuffer;

    // Setup our rasterizer state object.
    D3D11_RASTERIZER_DESC rasterizer_desc = NK_ZERO_MEM;
    rasterizer_desc.FillMode              = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode              = D3D11_CULL_NONE;
    rasterizer_desc.FrontCounterClockwise = NK_TRUE;

    res = g_d3d.device->CreateRasterizerState(&rasterizer_desc, &g_d3d.rasterizer_state);
    if(!SUCCEEDED(res))
    {
        fatal_error("Failed to create Direct3D rasterizer!");
    }

    // Disable DXGI intercepting window events as it is annoying.
    dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES);

    // Set the initial blend mode to be alpha.
    set_blend_mode(BlendMode_Alpha);

    // Setup our constant buffer (we just have one fixed size buffer).
    D3D11_BUFFER_DESC buffer_desc = NK_ZERO_MEM;
    buffer_desc.ByteWidth      = MAX_CONSTANTS_SIZE;
    buffer_desc.Usage          = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    res = g_d3d.device->CreateBuffer(&buffer_desc, NULL, &g_d3d.constants);
    if(!SUCCEEDED(res))
    {
        fatal_error("Failed to create Direct3D constant buffer!");
    }
}

GLOBAL void quit_render_system(void)
{
    if(g_d3d.constants)
    {
        g_d3d.constants->Release();
        g_d3d.constants = NULL;
    }
    if(g_d3d.rasterizer_state)
    {
        g_d3d.rasterizer_state->Release();
        g_d3d.rasterizer_state = NULL;
    }
    if(g_d3d.backbuffer_texture)
    {
        g_d3d.backbuffer_texture->Release();
        g_d3d.backbuffer_texture = NULL;
    }
    if(g_d3d.backbuffer)
    {
        g_d3d.backbuffer->Release();
        g_d3d.backbuffer = NULL;
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

GLOBAL void begin_render_frame(void)
{
    // We need to resize the backbuffer if the window has changed size.
    nkS32 ww = get_window_width();
    nkS32 wh = get_window_height();

    if(ww != g_d3d.backbuffer_w || wh != g_d3d.backbuffer_h)
    {
        g_d3d.device_context->OMSetRenderTargets(0, NULL, NULL);

        if(g_d3d.backbuffer_texture)
        {
            if(g_d3d.current_target == g_d3d.backbuffer)
            {
                g_d3d.current_target = NULL;
            }

            g_d3d.backbuffer_texture->Release();
            g_d3d.backbuffer_texture = NULL;

            g_d3d.backbuffer->Release();
            g_d3d.backbuffer = NULL;

            g_d3d.swap_chain->ResizeBuffers(0, ww, wh, DXGI_FORMAT_UNKNOWN, 0);

            g_d3d.swap_chain->GetBuffer(0, IID_PPV_ARGS(&g_d3d.backbuffer_texture));
            HRESULT res = g_d3d.device->CreateRenderTargetView(g_d3d.backbuffer_texture, NULL, &g_d3d.backbuffer);
            if(!SUCCEEDED(res))
            {
                fatal_error("Failed to resize Direct3D backbuffer!");
            }

            g_d3d.backbuffer_w = ww;
            g_d3d.backbuffer_h = wh;

            // Update the current target if it was the backbuffer before destroying.
            if(!g_d3d.current_target)
            {
                g_d3d.current_target = g_d3d.backbuffer;
            }
        }
    }
}

GLOBAL void renderer_present(void)
{
    g_d3d.swap_chain->Present(1, 0); // Present with Vsync.
}

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    g_d3d.viewport = { x,y,w,h };
}

GLOBAL void set_blend_mode(BlendMode blend_mode)
{
    // If the blend state has already been created then get rid of it.
    if(g_d3d.blend_state)
    {
        g_d3d.blend_state->Release();
        g_d3d.blend_state = NULL;
    }

    // Setup the new blend state.
    D3D11_BLEND_DESC desc = NK_ZERO_MEM;

    switch(blend_mode)
    {
        case BlendMode_None:
        {
            desc.RenderTarget[0].BlendEnable = NK_FALSE;
        } break;
        case BlendMode_Alpha:
        {
            desc.RenderTarget[0].BlendEnable           = NK_TRUE;
            desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
            desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ONE;
            desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        } break;
        case BlendMode_PremultipliedAlpha:
        {
            desc.RenderTarget[0].BlendEnable           = NK_TRUE;
            desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
            desc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
            desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ONE;
            desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        } break;
    }

    HRESULT res = g_d3d.device->CreateBlendState(&desc, &g_d3d.blend_state);
    if(!SUCCEEDED(res))
    {
        fatal_error("Failed to set Direct3D blend state!");
    }
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
    nkF32 clear_color[4] = { color.r, color.g, color.b, color.a };
    g_d3d.device_context->ClearRenderTargetView(g_d3d.current_target, clear_color);
}

GLOBAL void clear_screen(nkVec3 color)
{
    nkF32 clear_color[4] = { color.r, color.g, color.b, 1.0f };
    g_d3d.device_context->ClearRenderTargetView(g_d3d.current_target, clear_color);
}

GLOBAL void clear_screen(nkF32 r, nkF32 g, nkF32 b, nkF32 a)
{
    nkF32 clear_color[4] = { r, g, b, a };
    g_d3d.device_context->ClearRenderTargetView(g_d3d.current_target, clear_color);
}

GLOBAL VertexBuffer create_vertex_buffer(void)
{
    VertexBuffer vbuf = ALLOCATE_PRIVATE_TYPE(VertexBuffer);
    if(!vbuf) fatal_error("Failed to allocate vertex buffer!");

    vbuf->size = 0;
    vbuf->data = NULL;

    // We don't actually create a D3D buffer until it is time to draw...

    return vbuf;
}

GLOBAL void free_vertex_buffer(VertexBuffer vbuf)
{
    if(!vbuf) return;
    if(vbuf->buffer)
        vbuf->buffer->Release();
    nk_array_free(&vbuf->data);
    free(vbuf);
}

GLOBAL void set_vertex_buffer_stride(VertexBuffer vbuf, nkU64 byte_stride)
{
    NK_ASSERT(vbuf);

    vbuf->byte_stride = byte_stride;
}

GLOBAL void enable_vertex_buffer_attrib(VertexBuffer vbuf, nkU32 index, AttribSemantic semantic, AttribType type, nkU64 byte_offset)
{
    NK_ASSERT(vbuf);
    NK_ASSERT(index < NK_ARRAY_SIZE(vbuf->attribs));

    vbuf->attribs[index].enabled = NK_TRUE;
    vbuf->attribs[index].semantic = semantic;
    vbuf->attribs[index].type = type;
    vbuf->attribs[index].byte_offset = byte_offset;
}

GLOBAL void disable_vertex_buffer_attrib(VertexBuffer vbuf, nkU32 index)
{
    NK_ASSERT(vbuf);
    NK_ASSERT(index < NK_ARRAY_SIZE(vbuf->attribs));

    vbuf->attribs[index].enabled = NK_FALSE;
}

GLOBAL void update_vertex_buffer(VertexBuffer vbuf, void* data, nkU64 bytes, BufferType type)
{
    NK_ASSERT(vbuf);

    nk_array_assign(&vbuf->data, NK_CAST(nkU8*,data), bytes);
}

GLOBAL void draw_vertex_buffer(VertexBuffer vbuf, DrawMode draw_mode, nkU64 vert_count)
{
    NK_ASSERT(vbuf);

    if(!vert_count) return;

    // If the D3D buffer does not have enough space for the data the caller wants to draw then we allocate it here.
    // Otherwise we can just map the buffer and update the contents of the buffer that way, which is faster.
    if(vbuf->size < vbuf->data.length)
    {
        // If the buffer already exists it needs to be freed first.
        if(vbuf->buffer)
        {
            vbuf->buffer->Release();
        }

        D3D11_BUFFER_DESC desc = NK_ZERO_MEM;
        desc.ByteWidth      = NK_CAST(UINT,vbuf->data.length);
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA data = NK_ZERO_MEM;
        data.pSysMem = vbuf->data.data;

        HRESULT res = g_d3d.device->CreateBuffer(&desc, &data, &vbuf->buffer);
        if(!SUCCEEDED(res)) fatal_error("Failed to create Direct3D buffer!");

        vbuf->size = vbuf->data.length;
    }
    else
    {
        D3D11_MAPPED_SUBRESOURCE data = NK_ZERO_MEM;
        g_d3d.device_context->Map(vbuf->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
        memcpy(data.pData, vbuf->data.data, vbuf->data.length);
        g_d3d.device_context->Unmap(vbuf->buffer, 0);
    }

    // Map the primitive type to the appropriate topology.
    D3D11_PRIMITIVE_TOPOLOGY primitive = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    switch(draw_mode)
    {
        case DrawMode_Points: primitive = D3D_PRIMITIVE_TOPOLOGY_POINTLIST; break;
        case DrawMode_LineStrip: primitive = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP; break;
        case DrawMode_Lines: primitive = D3D_PRIMITIVE_TOPOLOGY_LINELIST; break;
        case DrawMode_TriangleStrip: primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
        case DrawMode_Triangles: primitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;

        // NOTE: Unsupported!
        case DrawMode_LineLoop: NK_ASSERT(NK_FALSE); break;
        case DrawMode_TriangleFan: NK_ASSERT(NK_FALSE); break;
    }

    // Create the input layout given the bound data.
    D3D11_INPUT_ELEMENT_DESC input_desc[16] = NK_ZERO_MEM;
    UINT input_count = 0;
    for(nkU64 i=0, n=NK_ARRAY_SIZE(vbuf->attribs); i<n; ++i)
    {
        VertexAttrib* attrib = &vbuf->attribs[i];
        if(attrib->enabled)
        {
            D3D11_INPUT_ELEMENT_DESC* desc = &input_desc[input_count++];
            desc->SemanticName      = attrib_semantic_to_d3d_name(attrib->semantic);
            desc->SemanticIndex     = 0;
            desc->Format            = attrib_type_to_d3d_format(attrib->type);
            desc->InputSlot         = 0; // This should be fine.
            desc->AlignedByteOffset = NK_CAST(UINT,attrib->byte_offset);
            desc->InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
        }
    }

    ID3D11InputLayout* input_layout = NULL;
    g_d3d.device->CreateInputLayout(input_desc, input_count, g_d3d.current_shader->vert_blob->GetBufferPointer(),
        g_d3d.current_shader->vert_blob->GetBufferSize(), &input_layout);

    // Do the actual draw.
    UINT byte_stride = NK_CAST(UINT, vbuf->byte_stride);
    UINT offset = 0;

    D3D11_VIEWPORT viewport = NK_ZERO_MEM;
    viewport.TopLeftX = g_d3d.viewport.x;
    viewport.TopLeftY = g_d3d.viewport.y;
    viewport.Width    = g_d3d.viewport.w;
    viewport.Height   = g_d3d.viewport.h;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    g_d3d.device_context->IASetPrimitiveTopology(primitive);
    g_d3d.device_context->IASetInputLayout(input_layout);
    g_d3d.device_context->IASetVertexBuffers(0, 1, &vbuf->buffer, &byte_stride, &offset);

    g_d3d.device_context->RSSetState(g_d3d.rasterizer_state);
    g_d3d.device_context->RSSetViewports(1, &viewport);

    g_d3d.device_context->VSSetShader(g_d3d.current_shader->vert_shader, NULL, 0);
    g_d3d.device_context->PSSetShader(g_d3d.current_shader->frag_shader, NULL, 0);

    g_d3d.device_context->VSSetConstantBuffers(0, 1, &g_d3d.constants);
    g_d3d.device_context->PSSetConstantBuffers(0, 1, &g_d3d.constants);

    for(nkS32 i=0,n=NK_ARRAY_SIZE(g_d3d.current_texture); i<n; ++i)
    {
        Texture tex = g_d3d.current_texture[i];
        if(tex)
        {
            g_d3d.device_context->PSSetShaderResources(i, 1, &tex->shader_view);
            g_d3d.device_context->PSSetSamplers(i, 1, &tex->sampler);
        }
    }

    g_d3d.device_context->OMSetRenderTargets(1, &g_d3d.current_target, NULL);
    g_d3d.device_context->OMSetBlendState(g_d3d.blend_state, NULL, 0xFFFFFFFF);

    g_d3d.device_context->Draw(NK_CAST(UINT,vert_count), 0);

    if(input_layout) input_layout->Release();
}

GLOBAL RenderTarget create_render_target(nkS32 w, nkS32 h, SamplerFilter filter, SamplerWrap wrap)
{
    // Not going to bother implementing as the game doesn't use it...
    return NULL;
}

GLOBAL void free_render_target(RenderTarget target)
{
    // Not going to bother implementing as the game doesn't use it...
}

GLOBAL void resize_render_target(RenderTarget target, nkS32 w, nkS32 h)
{
    // Not going to bother implementing as the game doesn't use it...
}

GLOBAL void bind_render_target(RenderTarget target)
{
    // Not going to bother implementing as the game doesn't use it...
}

GLOBAL Shader create_shader(void* data, nkU64 bytes)
{
    Shader shader = ALLOCATE_PRIVATE_TYPE(Shader);
    if(!shader) fatal_error("Failed to allocate shader!");

    ID3DBlob* vs_error_blob;
    ID3DBlob* fs_error_blob;

    D3DCompile(data, bytes, NULL, NULL, NULL, "vs_main", "vs_5_0", 0, 0, &shader->vert_blob, &vs_error_blob);
    D3DCompile(data, bytes, NULL, NULL, NULL, "ps_main", "ps_5_0", 0, 0, &shader->frag_blob, &fs_error_blob);

    if(vs_error_blob)
    {
        printf("[Direct3D]: Vert shader error:\n\n%.*s\n", NK_CAST(nkS32,vs_error_blob->GetBufferSize()), NK_CAST(nkChar*,vs_error_blob->GetBufferPointer()));
    }
    if(fs_error_blob)
    {
        printf("[Direct3D]: Frag shader error:\n\n%.*s\n", NK_CAST(nkS32,fs_error_blob->GetBufferSize()), NK_CAST(nkChar*,fs_error_blob->GetBufferPointer()));
    }

    g_d3d.device->CreateVertexShader(shader->vert_blob->GetBufferPointer(), shader->vert_blob->GetBufferSize(), NULL, &shader->vert_shader);
    g_d3d.device->CreatePixelShader(shader->frag_blob->GetBufferPointer(), shader->frag_blob->GetBufferSize(), NULL, &shader->frag_shader);

    return shader;
}

GLOBAL void free_shader(Shader shader)
{
    if(!shader) return;

    if(shader->vert_blob) shader->vert_blob->Release();
    if(shader->vert_shader) shader->vert_shader->Release();
    if(shader->frag_blob) shader->frag_blob->Release();
    if(shader->frag_shader) shader->frag_shader->Release();

    free(shader);
}

GLOBAL void bind_shader(Shader shader)
{
    g_d3d.current_shader = shader;
}

GLOBAL void set_shader_uniforms(void* data, nkU64 bytes)
{
    D3D11_MAPPED_SUBRESOURCE subres = NK_ZERO_MEM;
    g_d3d.device_context->Map(g_d3d.constants, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
    memcpy(subres.pData, data, bytes);
    g_d3d.device_context->Unmap(g_d3d.constants, 0);
}

GLOBAL Texture create_texture(nkS32 w, nkS32 h, nkS32 bpp, void* data, SamplerFilter filter, SamplerWrap wrap)
{
    Texture texture = ALLOCATE_PRIVATE_TYPE(Texture);
    if(!texture) fatal_error("Failed to allocate texture!");

    D3D11_TEXTURE2D_DESC texture_desc = NK_ZERO_MEM;
    texture_desc.Width              = w;
    texture_desc.Height             = h;
    texture_desc.MipLevels          = 1;
    texture_desc.ArraySize          = 1;
    texture_desc.Format             = bpp_to_d3d_format(bpp);
    texture_desc.SampleDesc.Count   = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Usage              = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    texture_desc.CPUAccessFlags     = 0;
    texture_desc.MiscFlags          = 0;

    D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = NK_ZERO_MEM;
    view_desc.Format                    = bpp_to_d3d_format(bpp);
    view_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    view_desc.Texture2D.MostDetailedMip = 0;
    view_desc.Texture2D.MipLevels       = 1;

    D3D11_SAMPLER_DESC sampler_desc = NK_ZERO_MEM;
    sampler_desc.Filter         = sampler_filter_to_d3d(filter);
    sampler_desc.AddressU       = sampler_wrap_to_d3d(wrap);
    sampler_desc.AddressV       = sampler_wrap_to_d3d(wrap);
    sampler_desc.AddressW       = sampler_wrap_to_d3d(wrap);
    sampler_desc.MipLODBias     = 0;
    sampler_desc.MaxAnisotropy  = 1;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampler_desc.MinLOD         = 0;
    sampler_desc.MaxLOD         = 0;

    D3D11_SUBRESOURCE_DATA resource = NK_ZERO_MEM;
    resource.pSysMem     = data;
    resource.SysMemPitch = w*bpp;

    g_d3d.device->CreateTexture2D(&texture_desc, &resource, &texture->texture);
    g_d3d.device->CreateShaderResourceView(texture->texture, &view_desc, &texture->shader_view);
    g_d3d.device->CreateSamplerState(&sampler_desc, &texture->sampler);

    texture->size.x = NK_CAST(nkF32, w);
    texture->size.y = NK_CAST(nkF32, h);

    return texture;
}

GLOBAL void free_texture(Texture texture)
{
    if(!texture) return;

    if(texture->texture) texture->texture->Release();
    if(texture->shader_view) texture->shader_view->Release();
    if(texture->sampler) texture->sampler->Release();

    free(texture);
}

GLOBAL void bind_texture(Texture texture, nkS32 unit)
{
    NK_ASSERT(unit < NK_ARRAY_SIZE(g_d3d.current_texture));
    g_d3d.current_texture[unit] = texture;
}

GLOBAL nkVec2 get_texture_size(Texture texture)
{
    NK_ASSERT(texture);
    return texture->size;
}

GLOBAL nkS32 get_texture_width(Texture texture)
{
    NK_ASSERT(texture);
    return NK_CAST(nkS32, texture->size.x);
}

GLOBAL nkS32 get_texture_height(Texture texture)
{
    NK_ASSERT(texture);
    return NK_CAST(nkS32, texture->size.y);
}

/*////////////////////////////////////////////////////////////////////////////*/
