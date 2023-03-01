/*////////////////////////////////////////////////////////////////////////////*/

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

GLOBAL void init_render_system(void)
{
    // @Incomplete: ...
}

GLOBAL void quit_render_system(void)
{
    // @Incomplete: ...
}

GLOBAL void setup_renderer_platform(void)
{
    // @Incomplete: ...
}

GLOBAL void do_render_frame(void)
{
    // @Incomplete: ...
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
