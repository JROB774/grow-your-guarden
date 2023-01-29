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

NK_ENUM(AttribType, nkS32)
{
    AttribType_SignedByte,
    AttribType_UnsignedByte,
    AttribType_SignedInt,
    AttribType_UnsignedInt,
    AttribType_Float
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
    SamplerFilter_Linear,
    SamplerFilter_NearestWithNearestMips,
    SamplerFilter_LinearWithNearestMips,
    SamplerFilter_NearestWithLinearMips,
    SamplerFilter_LinearWithLinearMips
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

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h);

GLOBAL void set_blend_mode(BlendMode blend_mode);

GLOBAL void clear_screen(nkVec4 color);
GLOBAL void clear_screen(nkVec3 color);
GLOBAL void clear_screen(nkF32 r, nkF32 g, nkF32 b, nkF32 a = 1.0f);

GLOBAL VertexBuffer create_vertex_buffer        (void);
GLOBAL void         free_vertex_buffer          (VertexBuffer vbuf);
GLOBAL void         set_vertex_buffer_stride    (VertexBuffer vbuf, nkU64 byte_stride);
GLOBAL void         enable_vertex_buffer_attrib (VertexBuffer vbuf, nkU32 index, AttribType type, nkU32 comps, nkU64 byte_offset);
GLOBAL void         disable_vertex_buffer_attrib(VertexBuffer vbuf, nkU32 index);
GLOBAL void         update_vertex_buffer        (VertexBuffer vbuf, void* data, nkU64 bytes, BufferType type);
GLOBAL void         draw_vertex_buffer          (VertexBuffer vbuf, DrawMode draw_mode, nkU64 vert_count);

GLOBAL RenderTarget create_render_target(nkS32 w, nkS32 h, SamplerFilter filter, SamplerWrap wrap);
GLOBAL void         free_render_target  (RenderTarget target);
GLOBAL void         resize_render_target(RenderTarget target, nkS32 w, nkS32 h);
GLOBAL void         bind_render_target  (RenderTarget target);

GLOBAL Shader create_shader   (void* data, nkU64 bytes);
GLOBAL void   free_shader     (Shader shader);
GLOBAL void   bind_shader     (Shader shader);
GLOBAL void   set_shader_bool (Shader shader, const nkChar* name, nkBool val);
GLOBAL void   set_shader_int  (Shader shader, const nkChar* name, nkS32  val);
GLOBAL void   set_shader_float(Shader shader, const nkChar* name, nkF32  val);
GLOBAL void   set_shader_vec2 (Shader shader, const nkChar* name, nkVec2 val);
GLOBAL void   set_shader_vec3 (Shader shader, const nkChar* name, nkVec3 val);
GLOBAL void   set_shader_vec4 (Shader shader, const nkChar* name, nkVec4 val);
GLOBAL void   set_shader_mat2 (Shader shader, const nkChar* name, nkMat2 val);
GLOBAL void   set_shader_mat3 (Shader shader, const nkChar* name, nkMat3 val);
GLOBAL void   set_shader_mat4 (Shader shader, const nkChar* name, nkMat4 val);

GLOBAL Texture create_texture    (nkS32 w, nkS32 h, nkS32 bpp, void* data, SamplerFilter filter, SamplerWrap wrap);
GLOBAL void    free_texture      (Texture texture);
GLOBAL void    bind_texture      (Texture texture, nkS32 unit);
GLOBAL nkVec2  get_texture_size  (Texture texture);
GLOBAL nkS32   get_texture_width (Texture texture);
GLOBAL nkS32   get_texture_height(Texture texture);

/*////////////////////////////////////////////////////////////////////////////*/
/*////////////////////////////////////////////////////////////////////////////*/
/*////////////////////////////////////////////////////////////////////////////*/

struct ImmVertex
{
    nkVec2 pos;
    nkVec2 tex;
    nkVec4 col;
};

struct ImmClip
{
    nkF32 x,y,w,h;
};

GLOBAL void imm_init(void);
GLOBAL void imm_quit(void);

GLOBAL nkMat4 imm_get_projection(void);
GLOBAL nkMat4 imm_get_view      (void);
GLOBAL nkMat4 imm_get_model     (void);

GLOBAL void imm_set_projection(nkMat4 projection);
GLOBAL void imm_set_view      (nkMat4 view);
GLOBAL void imm_set_model     (nkMat4 model);

GLOBAL void imm_set_viewport(nkVec4 viewport);

GLOBAL void imm_begin (DrawMode draw_mode, Texture tex, Shader shader);
GLOBAL void imm_end   (void);
GLOBAL void imm_vertex(ImmVertex v);

GLOBAL void imm_point         (nkF32  x, nkF32  y,                     nkVec4 color);
GLOBAL void imm_line          (nkF32 x1, nkF32 y1, nkF32 x2, nkF32 y2, nkVec4 color);
GLOBAL void imm_rect_outline  (nkF32  x, nkF32  y, nkF32  w, nkF32  h, nkVec4 color);
GLOBAL void imm_rect_filled   (nkF32  x, nkF32  y, nkF32  w, nkF32  h, nkVec4 color);
GLOBAL void imm_circle_outline(nkF32  x, nkF32  y, nkF32  r, nkS32  n, nkVec4 color);
GLOBAL void imm_circle_filled (nkF32  x, nkF32  y, nkF32  r, nkS32  n, nkVec4 color);

GLOBAL void imm_begin_texture_batch(Texture tex);
GLOBAL void imm_end_texture_batch (void);
GLOBAL void imm_texture           (Texture tex, nkF32 x, nkF32 y,                                                  const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE);
GLOBAL void imm_texture_ex        (Texture tex, nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE);
GLOBAL void imm_texture_batched   (             nkF32 x, nkF32 y,                                                  const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE);
GLOBAL void imm_texture_batched_ex(             nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE);

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
    return "shaders/";
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
