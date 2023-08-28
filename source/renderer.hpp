/*////////////////////////////////////////////////////////////////////////////*/

DECLARE_PRIVATE_TYPE(VertexBuffer);
DECLARE_PRIVATE_TYPE(RenderTarget);
DECLARE_PRIVATE_TYPE(Shader);
DECLARE_PRIVATE_TYPE(Texture);

NK_ENUM(DrawMode, nkS32)
{
    DrawMode_Points,
    DrawMode_LineStrip,
    DrawMode_LineLoop,
    DrawMode_Lines,
    DrawMode_TriangleStrip,
    DrawMode_TriangleFan,
    DrawMode_Triangles
};

NK_ENUM(AttribSemantic, nkS32)
{
    AttribSemantic_Pos,
    AttribSemantic_Tex,
    AttribSemantic_Col,
};

NK_ENUM(AttribType, nkS32)
{
    AttribType_Float1,
    AttribType_Float2,
    AttribType_Float3,
    AttribType_Float4,
};

NK_ENUM(BufferType, nkS32)
{
    BufferType_Static,
    BufferType_Dynamic,
    BufferType_Stream
};

NK_ENUM(SamplerFilter, nkS32)
{
    SamplerFilter_Nearest,
    SamplerFilter_Linear
};

NK_ENUM(SamplerWrap, nkS32)
{
    SamplerWrap_Repeat,
    SamplerWrap_Clamp
};

NK_ENUM(BlendMode, nkS32)
{
    BlendMode_None,
    BlendMode_Alpha,
    BlendMode_PremultipliedAlpha
};

struct VertexAttrib
{
    nkBool         enabled;
    AttribSemantic semantic;
    AttribType     type;
    nkU64          byte_offset;
};

GLOBAL void init_render_system     (void);
GLOBAL void quit_render_system     (void);
GLOBAL void setup_renderer_platform(void);
GLOBAL void begin_render_frame     (void);
GLOBAL void renderer_present       (void);

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h);

GLOBAL void set_blend_mode(BlendMode blend_mode);

GLOBAL void begin_scissor(nkF32 x, nkF32 y, nkF32 w, nkF32 h);
GLOBAL void end_scissor  (void);

GLOBAL void clear_screen(nkVec4 color);
GLOBAL void clear_screen(nkVec3 color);
GLOBAL void clear_screen(nkF32 r, nkF32 g, nkF32 b, nkF32 a = 1.0f);

GLOBAL VertexBuffer create_vertex_buffer        (void);
GLOBAL void         free_vertex_buffer          (VertexBuffer vbuf);
GLOBAL void         set_vertex_buffer_stride    (VertexBuffer vbuf, nkU64 byte_stride);
GLOBAL void         enable_vertex_buffer_attrib (VertexBuffer vbuf, nkU32 index, AttribSemantic semantic, AttribType type, nkU64 byte_offset);
GLOBAL void         disable_vertex_buffer_attrib(VertexBuffer vbuf, nkU32 index);
GLOBAL void         update_vertex_buffer        (VertexBuffer vbuf, void* data, nkU64 bytes, BufferType type);
GLOBAL void         draw_vertex_buffer          (VertexBuffer vbuf, DrawMode draw_mode, nkU64 vert_count);

GLOBAL RenderTarget create_render_target(nkS32 w, nkS32 h, SamplerFilter filter, SamplerWrap wrap);
GLOBAL void         free_render_target  (RenderTarget target);
GLOBAL void         resize_render_target(RenderTarget target, nkS32 w, nkS32 h);
GLOBAL void         bind_render_target  (RenderTarget target);

GLOBAL Shader create_shader      (void* data, nkU64 bytes);
GLOBAL void   free_shader        (Shader shader);
GLOBAL void   bind_shader        (Shader shader);
GLOBAL void   set_shader_uniforms(void* data, nkU64 bytes);

GLOBAL Texture create_texture    (nkS32 w, nkS32 h, nkS32 bpp, void* data, SamplerFilter filter, SamplerWrap wrap);
GLOBAL void    free_texture      (Texture texture);
GLOBAL void    bind_texture      (Texture texture, nkS32 unit);
GLOBAL nkVec2  get_texture_size  (Texture texture);
GLOBAL nkS32   get_texture_width (Texture texture);
GLOBAL nkS32   get_texture_height(Texture texture);

/*////////////////////////////////////////////////////////////////////////////*/

template<>
Shader asset_load<Shader>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    return create_shader(data, size);
}
template<>
void asset_free<Shader>(Asset<Shader>& asset)
{
    free_shader(asset.data);
}
template<>
const nkChar* asset_path<Shader>(void)
{
    // :WindowsOpenGL
    //
    // We are just always using OpenGL even though we have the Direct3D backend, as that's what we originally shipped with.
    //
    // #if defined(NK_OS_WIN32)
    // return "shaders/hlsl/";
    // #else
    return "shaders/glsl/";
    // #endif
}

template<>
Texture asset_load<Texture>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    nkS32 w,h,bpp;

    nkU8* pixels = NK_CAST(nkU8*, stbi_load_from_memory(NK_CAST(stbi_uc*, data), NK_CAST(int,size), &w,&h,&bpp, 4));
    if(!pixels) return NULL;
    NK_DEFER(stbi_image_free(pixels));

    return create_texture(w,h,4, pixels, SamplerFilter_Nearest, SamplerWrap_Clamp);
}
template<>
void asset_free<Texture>(Asset<Texture>& asset)
{
    free_texture(asset.data);
}
template<>
const nkChar* asset_path<Texture>(void)
{
    return "textures/";
}

/*////////////////////////////////////////////////////////////////////////////*/
