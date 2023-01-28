/*////////////////////////////////////////////////////////////////////////////*/

// Buffer ======================================================================
DECLARE_PRIVATE_TYPE(Buffer);

NK_ENUM(BufferType, nkS32)
{
    BufferType_Vertex,
    BufferType_Element,
    BufferType_Uniform,
    BufferType_TOTAL
};

NK_ENUM(BufferUsage, nkS32)
{
    BufferUsage_Static,
    BufferUsage_Dynamic,
    BufferUsage_Stream,
    BufferUsage_TOTAL
};

struct BufferDesc
{
    BufferType  type  = BufferType_Vertex;
    BufferUsage usage = BufferUsage_Static;
    void*       data  = NULL;
    nkU64       bytes = 0;
};

GLOBAL Buffer create_buffer(const BufferDesc& desc);
GLOBAL void   free_buffer  (Buffer buffer);
GLOBAL void   update_buffer(Buffer buffer, void* data, nkU64 bytes);
// =============================================================================

// Shader ======================================================================
DECLARE_PRIVATE_TYPE(Shader);

struct ShaderDesc
{
    void* data  = NULL;
    nkU64 bytes = 0;
};

GLOBAL Shader create_shader(const ShaderDesc& desc);
GLOBAL void   free_shader  (Shader shader);
// =============================================================================

// Sampler =====================================================================
DECLARE_PRIVATE_TYPE(Sampler);

NK_ENUM(SamplerFilter, nkS32)
{
    SamplerFilter_Nearest,
    SamplerFilter_Linear,
    SamplerFilter_TOTAL
};

NK_ENUM(SamplerWrap, nkS32)
{
    SamplerWrap_Repeat,
    SamplerWrap_Clamp,
    SamplerWrap_TOTAL
};

struct SamplerDesc
{
    SamplerFilter filter = SamplerFilter_Nearest;
    SamplerWrap   wrap_x = SamplerWrap_Clamp;
    SamplerWrap   wrap_y = SamplerWrap_Clamp;
    SamplerWrap   wrap_z = SamplerWrap_Clamp;
};

GLOBAL Sampler create_sampler(const SamplerDesc& desc);
GLOBAL void    free_sampler  (Sampler sampler);
// =============================================================================

// Texture =====================================================================
DECLARE_PRIVATE_TYPE(Texture);

NK_ENUM(TextureType, nkS32)
{
    TextureType_2D,
    TextureType_TOTAL
};

NK_ENUM(TextureFormat, nkS32)
{
    TextureFormat_R,
    TextureFormat_RGB,
    TextureFormat_RGBA,
    TextureFormat_D24S8,
    TextureFormat_TOTAL
};

struct TextureDesc
{
    TextureType   type   = TextureType_2D;
    TextureFormat format = TextureFormat_RGBA;
    nkS32         width  = 0;
    nkS32         height = 0;
    void*         data   = NULL;
};

GLOBAL Texture create_texture    (const TextureDesc& desc);
GLOBAL void    free_texture      (Texture texture);
GLOBAL void    resize_texture    (Texture texture, nkS32 width, nkS32 height);
GLOBAL nkVec2  get_texture_size  (Texture texture);
GLOBAL nkS32   get_texture_width (Texture texture);
GLOBAL nkS32   get_texture_height(Texture texture);
// =============================================================================

// Render Pass =================================================================
INTERNAL constexpr Texture BACKBUFFER = NULL;

DECLARE_PRIVATE_TYPE(RenderPass);

NK_ENUM(DrawMode, nkS32)
{
    DrawMode_Points,
    DrawMode_LineStrip,
    DrawMode_LineLoop,
    DrawMode_Lines,
    DrawMode_TriangleStrip,
    DrawMode_TriangleFan,
    DrawMode_Triangles,
    DrawMode_TOTAL
};

NK_ENUM(BlendMode, nkS32)
{
    BlendMode_None,
    BlendMode_Alpha,
    BlendMode_PremultipliedAlpha,
    BlendMode_TOTAL
};

NK_ENUM(CullFace, nkS32)
{
    CullFace_None,
    CullFace_Front,
    CullFace_Back,
    CullFace_FrontAndBack,
    CullFace_TOTAL
};

NK_ENUM(DepthOp, nkS32)
{
    DepthOp_Never,
    DepthOp_Equal,
    DepthOp_NotEqual,
    DepthOp_Less,
    DepthOp_LessEqual,
    DepthOp_Greater,
    DepthOp_GreaterEqual,
    DepthOp_Always,
    DepthOp_TOTAL
};

NK_ENUM(ElementType, nkS32)
{
    ElementType_UnsignedByte,
    ElementType_UnsignedShort,
    ElementType_UnsignedInt,
    ElementType_TOTAL
};

NK_ENUM(AttribType, nkS32)
{
    AttribType_SignedByte,
    AttribType_UnsignedByte,
    AttribType_SignedInt,
    AttribType_UnsignedInt,
    AttribType_Float,
    AttribType_TOTAL
};

struct VertexAttrib
{
    nkU32      index       = 0;
    AttribType type        = AttribType_Float;
    nkU32      components  = 4;
    nkU64      byte_offset = 0;
    nkBool     enabled     = NK_FALSE;
};

struct VertexLayout
{
    VertexAttrib attribs[16]  = {};
    nkU64        attrib_count = 0;
    nkU64        byte_stride  = 0;
};

struct RenderPassDesc
{
    Texture      color_targets[16]    = { BACKBUFFER };
    Texture      depth_stencil_target = NULL;
    nkU32        num_color_targets    = 1;
    DrawMode     draw_mode            = DrawMode_Triangles;
    BlendMode    blend_mode           = BlendMode_None;
    CullFace     cull_face            = CullFace_Back;
    DepthOp      depth_op             = DepthOp_Less;
    nkBool       depth_read           = NK_TRUE;
    nkBool       depth_write          = NK_TRUE;
    nkBool       clear                = NK_FALSE;
    nkVec4       clear_color          = NK_V4_BLACK;
};

GLOBAL RenderPass create_render_pass(const RenderPassDesc& desc);
GLOBAL void       free_render_pass  (RenderPass pass);
GLOBAL void       begin_render_pass (RenderPass pass);
GLOBAL void       end_render_pass   (void);
GLOBAL void       set_viewport      (nkF32 x, nkF32 y, nkF32 w, nkF32 h);
GLOBAL void       begin_scissor     (nkF32 x, nkF32 y, nkF32 w, nkF32 h);
GLOBAL void       end_scissor       (void);
GLOBAL void       bind_buffer       (Buffer buffer, nkS32 slot = 0);
GLOBAL void       bind_shader       (Shader shader);
GLOBAL void       bind_texture      (Texture texture, Sampler sampler, nkS32 unit);
GLOBAL void       draw_arrays       (const VertexLayout& vertex_layout, nkU64 vertex_count);
GLOBAL void       draw_elements     (const VertexLayout& vertex_layout, nkU64 element_count, ElementType element_type);
// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
/*////////////////////////////////////////////////////////////////////////////*/
/*////////////////////////////////////////////////////////////////////////////*/

NK_ENUM(ImmSampler, nkS32)
{
    ImmSampler_ClampNearest,
    ImmSampler_ClampLinear,
    ImmSampler_RepeatNearest,
    ImmSampler_RepeatLinear,
    ImmSampler_TOTAL
};

// System functions that should already be getting called, no need to use these!
GLOBAL void imm_init       (void);
GLOBAL void imm_quit       (void);
GLOBAL void imm_begin_frame(void);
GLOBAL void imm_end_frame  (void);

// General =====================================================================
GLOBAL void imm_reset(); // Reset all of the imm state values back to their defaults.
// =============================================================================

// State Setters ===============================================================
GLOBAL void imm_set_color_target(Texture target);                      // Set a color target to use for rendering (NULL or BACKBUFFER for the backbuffer).
GLOBAL void imm_set_depth_target(Texture target);                      // Set a depth target to use for rendering (NULL for none).
GLOBAL void imm_set_uniforms    (void* data, nkU64 bytes, nkU32 slot); // Set some custom uniform data to use for rendering, set to NULL to disable. (Data must survive until a call to imm_end).
GLOBAL void imm_set_shader      (Shader shader);                       // Set a shader to use for rendering, set to NULL to use the built-in immediate mode shader.
GLOBAL void imm_set_sampler     (Sampler sampler, nkS32 slot = 0);     // Set a sampler to use for rendering, set to NULL to use the built-in immediate mode sampler.
GLOBAL void imm_set_texture     (Texture texture, nkS32 slot = 0);     // Set a texture to use for rendering, set to NULL for no texture to be used.
GLOBAL void imm_set_viewport    (nkVec4 viewport);                     // Set the viewport rect to use for rendering.
GLOBAL void imm_set_projection  (nkMat4 projection);                   // Set the projection matrix to use for rendering.
GLOBAL void imm_set_view        (nkMat4 view);                         // Set the view matrix to use for rendering.
GLOBAL void imm_set_model       (nkMat4 model);                        // Set the model matrix to use for rendering.
GLOBAL void imm_set_depth_read  (nkBool enable);                       // Set whether rendering should read depth or not.
GLOBAL void imm_set_depth_write (nkBool enable);                       // Set whether rendering should write depth or not.
// =============================================================================

// State Getters ===============================================================
GLOBAL Texture imm_get_color_target(void);            // Get the color render target currently in use.
GLOBAL Texture imm_get_depth_target(void);            // Get the depth render target currently in use.
GLOBAL Shader  imm_get_shader      (void);            // Get the shader currently in use.
GLOBAL Sampler imm_get_sampler     (nkS32 slot = 0);  // Get the sampler currently in use.
GLOBAL Texture imm_get_texture     (nkS32 slot = 0);  // Get the texture currently in use.
GLOBAL nkVec4  imm_get_viewport    (void);            // Get the viewport currently in use.
GLOBAL nkMat4  imm_get_projection  (void);            // Get the projection matrix currently in use.
GLOBAL nkMat4  imm_get_view        (void);            // Get the view matrix currently in use.
GLOBAL nkMat4  imm_get_model       (void);            // Get the model matrix currently in use.
GLOBAL nkBool  imm_get_depth_read  (void);            // Get the current depth read status.
GLOBAL nkBool  imm_get_depth_write (void);            // Get the current depth write status.
GLOBAL Sampler imm_get_def_sampler (ImmSampler samp); // Imm defines some common samplers for systems to use, you can get them from here.
// =============================================================================

// Polygon Drawing =============================================================
// A flexible immediate mode drawing system that allows for specifying a number
// of different built-in vertex data types, as well as custom user data. Coupled
// with binding custom shaders and textures, this system can do quite a bit.
GLOBAL void imm_begin    (DrawMode draw_mode);                                      // Begin a draw operation with a specifical draw mode.
GLOBAL void imm_end      (void);                                                    // End a draw operation and render to the current target.
GLOBAL void imm_position (nkF32 x, nkF32 y,        nkF32 z = 0.0f, nkF32 w = 1.0f); // Add a vertex position to the current draw data.
GLOBAL void imm_normal   (nkF32 x, nkF32 y,        nkF32 z = 0.0f, nkF32 w = 0.0f); // Add a vertex normal to the current draw data.
GLOBAL void imm_color    (nkF32 x, nkF32 y,        nkF32 z,        nkF32 w = 1.0f); // Add a vertex color to the current draw data.
GLOBAL void imm_texcoord (nkF32 x, nkF32 y,        nkF32 z = 0.0f, nkF32 w = 0.0f); // Add a vertex texcoord to the current draw data.
GLOBAL void imm_userdata0(nkF32 x, nkF32 y = 0.0f, nkF32 z = 0.0f, nkF32 w = 0.0f); // Add some custom vertex data to the current draw data.
GLOBAL void imm_userdata1(nkF32 x, nkF32 y = 0.0f, nkF32 z = 0.0f, nkF32 w = 0.0f); // Add some custom vertex data to the current draw data.
GLOBAL void imm_userdata2(nkF32 x, nkF32 y = 0.0f, nkF32 z = 0.0f, nkF32 w = 0.0f); // Add some custom vertex data to the current draw data.
GLOBAL void imm_userdata3(nkF32 x, nkF32 y = 0.0f, nkF32 z = 0.0f, nkF32 w = 0.0f); // Add some custom vertex data to the current draw data.
// =============================================================================

// 2D Primitives ===============================================================
// Convenience functions for drawing different primitives without needing to
// build the vertex data yourself and do the imm_begin and imm_end calls.
GLOBAL void imm_point         (nkF32  x, nkF32  y,                     nkVec4 color); // Draw a single point.
GLOBAL void imm_line          (nkF32 x1, nkF32 y1, nkF32 x2, nkF32 y2, nkVec4 color); // Draw a single line.
GLOBAL void imm_rect_outline  (nkF32  x, nkF32  y, nkF32  w, nkF32  h, nkVec4 color); // Draw a single rect outline.
GLOBAL void imm_rect_filled   (nkF32  x, nkF32  y, nkF32  w, nkF32  h, nkVec4 color); // Draw a single rect fill.
GLOBAL void imm_circle_outline(nkF32  x, nkF32  y, nkF32  r, nkS32  n, nkVec4 color); // Draw a single circle outline.
GLOBAL void imm_circle_filled (nkF32  x, nkF32  y, nkF32  r, nkS32  n, nkVec4 color); // Draw a single circle fill.
// =============================================================================

// 2D Textures =================================================================
// Convenience functions for rendering 2D textures (e.g. sprites) with clipping,
// simple transforms, and color modulation all built-in. Also has a simple
// texture batching system that can be used for doing some basic optimizing if
// being used to render lots of data with the same texture consecutively.
struct ImmClip { nkF32 x,y,w,h; };
GLOBAL void imm_begin_texture_batch(Texture tex);                                                                                                                            // Begin a new texture batch.
GLOBAL void imm_end_texture_batch  (void);                                                                                                                                   // End a texture batch.
GLOBAL void imm_texture            (Texture tex, nkF32 x, nkF32 y,                                                  const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE); // Draw a texture.
GLOBAL void imm_texture_ex         (Texture tex, nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE); // Draw a texture with scale and rotation.
GLOBAL void imm_texture_batched    (             nkF32 x, nkF32 y,                                                  const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE); // Draw a batched texture.
GLOBAL void imm_texture_batched_ex (             nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip = NULL, nkVec4 color = NK_V4_WHITE); // Draw a batched texture with scale and rotation.
// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/

template<>
Shader asset_load<Shader>(void* data, nkU64 size, nkBool from_npak, void* userdata)
{
    ShaderDesc sd = { data, size };
    return create_shader(sd);
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

    TextureDesc desc;
    desc.format = TextureFormat_RGBA;
    desc.width  = w;
    desc.height = h;
    desc.data   = pixels;

    return create_texture(desc);
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
