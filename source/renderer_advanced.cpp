/*////////////////////////////////////////////////////////////////////////////*/

#if defined(BUILD_NATIVE)
#include <glew.c>
#endif // BUILD_NATIVE

#if defined(BUILD_WEB)
#include <GLES3/gl3.h>
#endif // BUILD_WEB

INTERNAL constexpr GLenum BUFFER_TYPE_TO_GL[] = { GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_UNIFORM_BUFFER };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(BUFFER_TYPE_TO_GL) == BufferType_TOTAL, buffer_type_size_mismatch);

INTERNAL constexpr GLenum BUFFER_USAGE_TO_GL[] = { GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(BUFFER_USAGE_TO_GL) == BufferUsage_TOTAL, buffer_usage_size_mismatch);

INTERNAL constexpr GLenum SAMPLER_FILTER_TO_GL[] = { GL_NEAREST, GL_LINEAR };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(SAMPLER_FILTER_TO_GL) == SamplerFilter_TOTAL, sampler_filter_size_mismatch);

INTERNAL constexpr GLenum SAMPLER_WRAP_TO_GL[] = { GL_REPEAT, GL_CLAMP_TO_EDGE };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(SAMPLER_WRAP_TO_GL) == SamplerWrap_TOTAL, sampler_wrap_size_mismatch);

INTERNAL constexpr GLenum TEXTURE_TYPE_TO_GL[] = { GL_TEXTURE_2D };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(TEXTURE_TYPE_TO_GL) == TextureType_TOTAL, texture_type_size_mistmatch);

INTERNAL constexpr GLenum DRAW_MODE_TO_GL[] = { GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(DRAW_MODE_TO_GL) == DrawMode_TOTAL, draw_mode_size_mismatch);

INTERNAL constexpr GLenum DEPTH_OP_TO_GL[] = { GL_NEVER, GL_EQUAL, GL_NOTEQUAL, GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_ALWAYS };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(DEPTH_OP_TO_GL) == DepthOp_TOTAL, depth_op_size_mismatch);

INTERNAL constexpr GLenum ELEMENT_TYPE_TO_GL[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(ELEMENT_TYPE_TO_GL) == ElementType_TOTAL, element_type_size_mismatch);

INTERNAL constexpr GLenum ATTRIB_TYPE_TO_GL[] = { GL_BYTE, GL_UNSIGNED_BYTE, GL_INT, GL_UNSIGNED_INT, GL_FLOAT };
NK_STATIC_ASSERT(NK_ARRAY_SIZE(ATTRIB_TYPE_TO_GL) == AttribType_TOTAL, attrib_type_size_mismatch);

struct OpenGLTextureFormat
{
    GLenum internal_format;
    GLenum format;
    GLenum type;
};

INTERNAL constexpr OpenGLTextureFormat TEXTURE_FORMAT_TO_GL[] =
{
    { GL_R8, GL_RED, GL_UNSIGNED_BYTE },
    { GL_RGB, GL_RGB, GL_UNSIGNED_BYTE },
    { GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE },
    { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 }
};

NK_STATIC_ASSERT(NK_ARRAY_SIZE(TEXTURE_FORMAT_TO_GL) == TextureFormat_TOTAL, texture_format_size_mismatch);

INTERNAL nkBool   g_pass_started;
INTERNAL DrawMode g_current_draw_mode;
INTERNAL GLuint   g_vao;

GLOBAL void init_render_system(void)
{
    printf("[OpenGL]: Initializing System\n");

    #if defined(BUILD_NATIVE)
    glewInit();
    #endif // BUILD_NATIVE

    printf("[OpenGL]: GPU Renderer   : %s\n", glGetString(GL_RENDERER));
    printf("[OpenGL]: GPU Vendor     : %s\n", glGetString(GL_VENDOR));
    printf("[OpenGL]: OpenGL Version : %s\n", glGetString(GL_VERSION));
    printf("[OpenGL]: GLSL Version   : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // We need one Vertex Attribute Object in order to render with modern OpenGL.
    #if defined(BUILD_NATIVE)
    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);
    #endif // BUILD_NATIVE
}

GLOBAL void quit_render_system(void)
{
    #if defined(BUILD_NATIVE)
    glDeleteVertexArrays(1, &g_vao);
    #endif // BUILD_NATIVE
}

GLOBAL void setup_renderer_platform(void)
{
    #if defined(BUILD_NATIVE)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    #endif // BUILD_NATIVE
    #if defined(BUILD_WEB)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    #endif // BUILD_WEB
}

GLOBAL void do_render_frame(void)
{
    imm_begin_frame();
    app_draw();
    imm_end_frame();
}

// Buffer ======================================================================

DEFINE_PRIVATE_TYPE(Buffer)
{
    GLenum usage;
    GLenum type;
    GLuint handle;
    nkU64  bytes;
};

GLOBAL Buffer create_buffer(const BufferDesc& desc)
{
    Buffer buffer = ALLOCATE_PRIVATE_TYPE(Buffer);
    if(!buffer) fatal_error("Failed to allocate buffer!");

    buffer->usage = BUFFER_USAGE_TO_GL[desc.usage];
    buffer->type = BUFFER_TYPE_TO_GL[desc.type];

    glGenBuffers(1, &buffer->handle);

    glBindBuffer(buffer->type, buffer->handle);
    glBufferData(buffer->type, desc.bytes, desc.data, buffer->usage);
    glBindBuffer(buffer->type, GL_NONE);

    return buffer;
}

GLOBAL void free_buffer(Buffer buffer)
{
    NK_ASSERT(buffer);
    glDeleteBuffers(1, &buffer->handle);
    NK_FREE(buffer);
}

GLOBAL void update_buffer(Buffer buffer, void* data, nkU64 bytes)
{
    NK_ASSERT(buffer);

    glBindBuffer(buffer->type, buffer->handle);
    glBufferData(buffer->type, bytes, data, buffer->usage);
}

// =============================================================================

// Shader ======================================================================

DEFINE_PRIVATE_TYPE(Shader)
{
    GLenum program;
};

INTERNAL GLuint compile_shader(void* data, nkU64 bytes, GLenum type)
{
    const nkChar* sources[2] = { NULL, NK_CAST(const nkChar*, data) };
    const GLint lengths[2] = { -1, NK_CAST(GLint, bytes) };

    #if defined(BUILD_NATIVE)
    if(type == GL_VERTEX_SHADER) sources[0] = "#version 330\n#define VERT_SHADER 1\n";
    if(type == GL_FRAGMENT_SHADER) sources[0] = "#version 330\n#define FRAG_SHADER 1\n";
    #endif // BUILD_NATIVE
    #if defined(BUILD_WEB)
    if(type == GL_VERTEX_SHADER) sources[0] = "#version 300 es\n#define VERT_SHADER 1\nprecision mediump float;\n";
    if(type == GL_FRAGMENT_SHADER) sources[0] = "#version 300 es\n#define FRAG_SHADER 1\nprecision mediump float;\n";
    #endif // BUILD_WEB

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 2, sources, lengths);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        GLint info_log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
        nkChar* info_log = NK_MALLOC_TYPES(nkChar, info_log_length);
        if(info_log)
        {
            glGetShaderInfoLog(shader, info_log_length, NULL, info_log);
            printf("Failed to compile shader:\n%s\n", info_log);
            NK_FREE(info_log);
        }
        return GL_NONE;
    }

    return shader;
}

GLOBAL Shader create_shader(const ShaderDesc& desc)
{
    Shader shader = ALLOCATE_PRIVATE_TYPE(Shader);
    if(!shader) fatal_error("Failed to allocate shader!");

    GLuint vert = compile_shader(desc.data, desc.bytes, GL_VERTEX_SHADER);
    GLuint frag = compile_shader(desc.data, desc.bytes, GL_FRAGMENT_SHADER);

    shader->program = glCreateProgram();

    glAttachShader(shader->program, vert);
    glAttachShader(shader->program, frag);

    glLinkProgram(shader->program);

    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint success;
    glGetProgramiv(shader->program, GL_LINK_STATUS, &success);
    if(!success)
    {
        GLint info_log_length;
        glGetProgramiv(shader->program, GL_INFO_LOG_LENGTH, &info_log_length);
        nkChar* info_log = NK_MALLOC_TYPES(nkChar, info_log_length);
        if(info_log)
        {
            glGetProgramInfoLog(shader->program, info_log_length, NULL, info_log);
            printf("Failed to link shader:\n%s\n", info_log);
            NK_FREE(info_log);
        }
    }

    return shader;
}

GLOBAL void free_shader(Shader shader)
{
    NK_ASSERT(shader);
    glDeleteProgram(shader->program);
    NK_FREE(shader);
}

// =============================================================================

// Sampler =====================================================================

DEFINE_PRIVATE_TYPE(Sampler)
{
    GLuint handle;
};

GLOBAL Sampler create_sampler(const SamplerDesc& desc)
{
    Sampler sampler = ALLOCATE_PRIVATE_TYPE(Sampler);
    if(!sampler) fatal_error("Failed to allocate sampler!");

    glGenSamplers(1, &sampler->handle);

    glSamplerParameteri(sampler->handle, GL_TEXTURE_MIN_FILTER, SAMPLER_FILTER_TO_GL[desc.filter]);
    glSamplerParameteri(sampler->handle, GL_TEXTURE_MAG_FILTER, SAMPLER_FILTER_TO_GL[desc.filter]);

    glSamplerParameteri(sampler->handle, GL_TEXTURE_WRAP_S, SAMPLER_WRAP_TO_GL[desc.wrap_x]);
    glSamplerParameteri(sampler->handle, GL_TEXTURE_WRAP_T, SAMPLER_WRAP_TO_GL[desc.wrap_y]);
    glSamplerParameteri(sampler->handle, GL_TEXTURE_WRAP_R, SAMPLER_WRAP_TO_GL[desc.wrap_z]);

    return sampler;
}

GLOBAL void free_sampler(Sampler sampler)
{
    NK_ASSERT(sampler);
    glDeleteSamplers(1,&sampler->handle);
    NK_FREE(sampler);
}

// =============================================================================

// Texture =====================================================================

DEFINE_PRIVATE_TYPE(Texture)
{
    GLuint              handle;
    GLenum              type;
    OpenGLTextureFormat format;
    nkS32               width;
    nkS32               height;
};

GLOBAL Texture create_texture(const TextureDesc& desc)
{
    Texture texture = ALLOCATE_PRIVATE_TYPE(Texture);
    if(!texture) fatal_error("Failed to allocate texture!");

    glGenTextures(1, &texture->handle);

    texture->type = TEXTURE_TYPE_TO_GL[desc.type];
    texture->format = TEXTURE_FORMAT_TO_GL[desc.format];

    glBindTexture(texture->type, texture->handle);

    switch(desc.type)
    {
        case TextureType_2D:
        {
            glTexImage2D(texture->type, 0, texture->format.internal_format, desc.width,desc.height,
                0, texture->format.format, texture->format.type, desc.data);
        } break;
        default:
        {
            NK_ASSERT(NK_FALSE); // Unknown texture type!
        } break;
    }

    glBindTexture(texture->type, GL_NONE);

    texture->width = desc.width;
    texture->height = desc.height;

    return texture;
}

GLOBAL void free_texture(Texture texture)
{
    NK_ASSERT(texture);
    glDeleteTextures(1,&texture->handle);
    NK_FREE(texture);
}

GLOBAL void resize_texture(Texture texture, nkS32 width, nkS32 height)
{
    NK_ASSERT(texture);

    glBindTexture(texture->type, texture->handle);
    glTexImage2D(texture->type, 0, texture->format.internal_format, width,height,
        0, texture->format.format, texture->format.type, NULL);
    glBindTexture(texture->type, GL_NONE);

    texture->width = width;
    texture->height = height;
}

GLOBAL nkVec2 get_texture_size(Texture texture)
{
    NK_ASSERT(texture);
    nkVec2 size;
    size.x = NK_CAST(nkF32, texture->width);
    size.y = NK_CAST(nkF32, texture->height);
    return size;
}

GLOBAL nkS32 get_texture_width(Texture texture)
{
    NK_ASSERT(texture);
    return texture->width;
}

GLOBAL nkS32 get_texture_height(Texture texture)
{
    NK_ASSERT(texture);
    return texture->height;
}

// =============================================================================

// Render Pass =================================================================

DEFINE_PRIVATE_TYPE(RenderPass)
{
    GLuint         framebuffer;
    RenderPassDesc desc;
};

INTERNAL void bind_vertex_layout(const VertexLayout& vertex_layout)
{
    // Setup the vertex attributes using the provided layout.
    for(nkS32 i=0; i<vertex_layout.attrib_count; ++i)
    {
        const VertexAttrib* attrib = &vertex_layout.attribs[i];
        if(attrib->enabled)
        {
            GLenum type = ATTRIB_TYPE_TO_GL[attrib->type];
            glEnableVertexAttribArray(attrib->index);
            glVertexAttribPointer(attrib->index, attrib->components, type, GL_FALSE,
                NK_CAST(GLsizei, vertex_layout.byte_stride), NK_CAST(const void*, attrib->byte_offset));
        }
        else
        {
            glDisableVertexAttribArray(attrib->index);
        }
    }
}

GLOBAL RenderPass create_render_pass(const RenderPassDesc& desc)
{
    RenderPass pass = ALLOCATE_PRIVATE_TYPE(RenderPass);
    if(!pass) fatal_error("Failed to allocate render pass!");

    pass->desc = desc;

    if(pass->desc.color_targets[0] == BACKBUFFER && pass->desc.num_color_targets == 1)
    {
        pass->framebuffer = GL_NONE;
    }
    else
    {
        glGenFramebuffers(1, &pass->framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, pass->framebuffer);

        for(nkU32 i=0; i<pass->desc.num_color_targets; ++i)
        {
            Texture target = pass->desc.color_targets[i];
            NK_ASSERT(target->type == GL_TEXTURE_2D);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, target->handle, 0);
        }
        if(pass->desc.depth_stencil_target)
        {
            Texture target = pass->desc.depth_stencil_target;
            NK_ASSERT(target->type == GL_TEXTURE_2D);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, target->handle, 0);
        }

        NK_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    }

    return pass;
}

GLOBAL void free_render_pass(RenderPass pass)
{
    NK_ASSERT(pass);
    if(pass->framebuffer != GL_NONE)
        glDeleteFramebuffers(1, &pass->framebuffer);
    NK_FREE(pass);
}

GLOBAL void begin_render_pass(RenderPass pass)
{
    NK_ASSERT(!g_pass_started); // Render pass is already in progress!

    g_pass_started = NK_TRUE;

    glBindFramebuffer(GL_FRAMEBUFFER, pass->framebuffer);

    // Clear the target.
    if(pass->desc.clear)
    {
        // @Improve: Only clear necessary targets...
        glClearColor(pass->desc.clear_color.r,pass->desc.clear_color.g,pass->desc.clear_color.b,pass->desc.clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    }

    // Setup depth read/write.
    if(pass->desc.depth_read) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
    glDepthMask(pass->desc.depth_write);
    GLenum depth_func = DEPTH_OP_TO_GL[pass->desc.depth_op];
    glDepthFunc(depth_func);

    // Setup cull face mode.
    switch(pass->desc.cull_face)
    {
        case CullFace_None:
        {
            glDisable(GL_CULL_FACE);
        } break;
        case CullFace_Front:
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        } break;
        case CullFace_Back:
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } break;
    }

    // Setup blend mode.
    switch(pass->desc.blend_mode)
    {
        case BlendMode_None:
        {
            glDisable(GL_BLEND);
        } break;
        case BlendMode_Alpha:
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            glEnable(GL_BLEND);
        } break;
        case BlendMode_PremultipliedAlpha:
        {
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            glEnable(GL_BLEND);
        } break;
    }

    // Set the draw mode.
    g_current_draw_mode = pass->desc.draw_mode;
}

GLOBAL void end_render_pass(void)
{
    NK_ASSERT(g_pass_started); // Render pass has not been started!
    g_pass_started = NK_FALSE;
}

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    GLint   vx = NK_CAST(GLint,   x);
    GLint   vy = NK_CAST(GLint,   y);
    GLsizei vw = NK_CAST(GLsizei, w);
    GLsizei vh = NK_CAST(GLsizei, h);

    glViewport(vx,vy,vw,vh);
}

GLOBAL void begin_scissor(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    GLint   sx = NK_CAST(GLint,   x);
    GLint   sy = NK_CAST(GLint,   get_window_height()-(y+h));
    GLsizei sw = NK_CAST(GLsizei, w);
    GLsizei sh = NK_CAST(GLsizei, h);

    glScissor(sx,sy,sw,sh);
    glEnable(GL_SCISSOR_TEST);
}

GLOBAL void end_scissor(void)
{
    glDisable(GL_SCISSOR_TEST);
}

GLOBAL void bind_buffer(Buffer buffer, nkS32 slot)
{
    NK_ASSERT(g_pass_started); // Cannot bind outside of a render pass!
    NK_ASSERT(buffer);

    // @Incomplete: Handle properly setting uniform and texture bindings in GLES3!
    if(buffer->type != GL_UNIFORM_BUFFER) glBindBuffer(buffer->type, buffer->handle);
    else glBindBufferBase(buffer->type, slot, buffer->handle);
}

GLOBAL void bind_shader(Shader shader)
{
    NK_ASSERT(g_pass_started); // Cannot bind outside of a render pass!
    NK_ASSERT(shader);

    glUseProgram(shader->program);
}

GLOBAL void bind_texture(Texture texture, Sampler sampler, nkS32 unit)
{
    NK_ASSERT(g_pass_started); // Cannot bind outside of a render pass!
    NK_ASSERT(texture);

    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(texture->type, texture->handle);
    if(sampler) glBindSampler(unit, sampler->handle);
    else glBindSampler(unit, GL_NONE);
}

GLOBAL void draw_arrays(const VertexLayout& vertex_layout, nkU64 vertex_count)
{
    NK_ASSERT(g_pass_started); // Cannot draw outside of a render pass!

    GLenum mode = DRAW_MODE_TO_GL[g_current_draw_mode];
    bind_vertex_layout(vertex_layout);
    glDrawArrays(mode, 0, NK_CAST(GLsizei,vertex_count));
}

GLOBAL void draw_elements(const VertexLayout& vertex_layout, nkU64 element_count, ElementType element_type)
{
    NK_ASSERT(g_pass_started); // Cannot draw outside of a render pass!

    GLenum type = ELEMENT_TYPE_TO_GL[element_type];
    GLenum mode = DRAW_MODE_TO_GL[g_current_draw_mode];
    bind_vertex_layout(vertex_layout);
    glDrawElements(mode, NK_CAST(GLsizei,element_count), type, NULL);
}

// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/
/*////////////////////////////////////////////////////////////////////////////*/
/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkU32 IMM_MAX_VERTICES = 16384;
INTERNAL constexpr nkU32 IMM_MAX_UNIFORMS = 8;
INTERNAL constexpr nkU32 IMM_MAX_TEXTURES = 16;

struct ImmVertex
{
    nkVec4 position;
    nkVec4 normal;
    nkVec4 color;
    nkVec4 texcoord;
    nkVec4 userdata0;
    nkVec4 userdata1;
    nkVec4 userdata2;
    nkVec4 userdata3;
};

struct ImmData
{
    void* data;
    nkU64 size;
};

struct ImmUniform
{
    nkMat4 u_projection;
    nkMat4 u_view;
    nkMat4 u_model;
    nkBool u_usetex;
};

struct ImmContext
{
    VertexLayout vertex_layout;
    ImmVertex    vertices[IMM_MAX_VERTICES];
    Buffer       vertex_buffer;
    Buffer       uniform_buffers[IMM_MAX_UNIFORMS];
    RenderPass   render_pass;

    Shader       default_shader;
    Sampler      default_samplers[ImmSampler_TOTAL];

    DrawMode     current_draw_mode;
    Texture      current_color_target;
    Texture      current_depth_target;
    ImmData      current_uniforms[IMM_MAX_UNIFORMS];
    Shader       current_shader;
    Sampler      current_samplers[IMM_MAX_TEXTURES];
    Texture      current_textures[IMM_MAX_TEXTURES];
    nkVec4       current_viewport;
    nkMat4       current_projection;
    nkMat4       current_view;
    nkMat4       current_model;
    nkBool       current_depth_read;
    nkBool       current_depth_write;

    nkU64        position_count;
    nkU64        normal_count;
    nkU64        color_count;
    nkU64        texcoord_count;
    nkU64        userdata0_count;
    nkU64        userdata1_count;
    nkU64        userdata2_count;
    nkU64        userdata3_count;

    nkBool       draw_started;
    nkBool       pass_needs_rebuild;
};

INTERNAL ImmContext g_imm;

GLOBAL void imm_init(void)
{
    g_imm.vertex_layout.attribs[0] = { 0, AttribType_Float, 4, offsetof(ImmVertex, position ), NK_FALSE };
    g_imm.vertex_layout.attribs[1] = { 1, AttribType_Float, 4, offsetof(ImmVertex, normal   ), NK_FALSE };
    g_imm.vertex_layout.attribs[2] = { 2, AttribType_Float, 4, offsetof(ImmVertex, color    ), NK_FALSE };
    g_imm.vertex_layout.attribs[3] = { 3, AttribType_Float, 4, offsetof(ImmVertex, texcoord ), NK_FALSE };
    g_imm.vertex_layout.attribs[4] = { 4, AttribType_Float, 4, offsetof(ImmVertex, userdata0), NK_FALSE };
    g_imm.vertex_layout.attribs[5] = { 5, AttribType_Float, 4, offsetof(ImmVertex, userdata1), NK_FALSE };
    g_imm.vertex_layout.attribs[6] = { 6, AttribType_Float, 4, offsetof(ImmVertex, userdata2), NK_FALSE };
    g_imm.vertex_layout.attribs[7] = { 7, AttribType_Float, 4, offsetof(ImmVertex, userdata3), NK_FALSE };
    g_imm.vertex_layout.attrib_count = 8;
    g_imm.vertex_layout.byte_stride = sizeof(ImmVertex);

    BufferDesc vbuffer_desc;
    vbuffer_desc.usage = BufferUsage_Dynamic;
    vbuffer_desc.type  = BufferType_Vertex;
    g_imm.vertex_buffer = create_buffer(vbuffer_desc);

    BufferDesc ubuffer_desc;
    ubuffer_desc.usage = BufferUsage_Dynamic;
    ubuffer_desc.type  = BufferType_Uniform;
    for(nkS32 i=0; i<IMM_MAX_UNIFORMS; ++i)
    {
        g_imm.uniform_buffers[i] = create_buffer(ubuffer_desc);
    }

    g_imm.default_shader = asset_manager_load<Shader>("advanced_basic.shader");

    // Create some common samplers.
    SamplerDesc sd;

    sd.wrap_x = SamplerWrap_Clamp;
    sd.wrap_y = SamplerWrap_Clamp;
    sd.wrap_z = SamplerWrap_Clamp;

    sd.filter = SamplerFilter_Nearest;
    g_imm.default_samplers[ImmSampler_ClampNearest] = create_sampler(sd);
    sd.filter = SamplerFilter_Linear;
    g_imm.default_samplers[ImmSampler_ClampLinear] = create_sampler(sd);

    sd.wrap_x = SamplerWrap_Repeat;
    sd.wrap_y = SamplerWrap_Repeat;
    sd.wrap_z = SamplerWrap_Repeat;

    sd.filter = SamplerFilter_Nearest;
    g_imm.default_samplers[ImmSampler_RepeatNearest] = create_sampler(sd);
    sd.filter = SamplerFilter_Linear;
    g_imm.default_samplers[ImmSampler_RepeatLinear] = create_sampler(sd);
}

GLOBAL void imm_quit(void)
{
    for(nkS32 i=0; i<ImmSampler_TOTAL; ++i)
        free_sampler(g_imm.default_samplers[i]);

    free_buffer(g_imm.vertex_buffer);

    for(nkS32 i=0; i<IMM_MAX_UNIFORMS; ++i)
        free_buffer(g_imm.uniform_buffers[i]);

    if(g_imm.render_pass)
        free_render_pass(g_imm.render_pass);
}

GLOBAL void imm_begin_frame(void)
{
    imm_reset();
}

GLOBAL void imm_end_frame(void)
{
    // Does nothing currently...
}

// General =====================================================================

GLOBAL void imm_reset(void)
{
    nkF32 ww = NK_CAST(nkF32, get_window_width());
    nkF32 wh = NK_CAST(nkF32, get_window_height());

    g_imm.current_color_target = NULL;
    g_imm.current_depth_target = NULL;
    g_imm.current_shader       = NULL;
    g_imm.current_viewport     = NK_V4_ZERO;
    g_imm.current_projection   = nk_orthographic(0,ww,wh,0,-1,1);
    g_imm.current_view         = nk_m4_identity();
    g_imm.current_model        = nk_m4_identity();
    g_imm.current_depth_read   = NK_FALSE;
    g_imm.current_depth_write  = NK_FALSE;

    memset(g_imm.current_uniforms, 0, sizeof(g_imm.current_uniforms));
    memset(g_imm.current_samplers, 0, sizeof(g_imm.current_samplers));
    memset(g_imm.current_textures, 0, sizeof(g_imm.current_textures));

    g_imm.pass_needs_rebuild = NK_TRUE;
}

// =============================================================================

// State Setters ===============================================================

GLOBAL void imm_set_color_target(Texture target)
{
    g_imm.current_color_target = target;
    g_imm.pass_needs_rebuild = NK_TRUE;
}

GLOBAL void imm_set_depth_target(Texture target)
{
    g_imm.current_depth_target = target;
    g_imm.pass_needs_rebuild = NK_TRUE;
}

GLOBAL void imm_set_uniforms(void* data, nkU64 bytes, nkU32 slot)
{
    NK_ASSERT(slot > 0 && slot < IMM_MAX_UNIFORMS); // Slot 0 is off limits (it is used internally).
    g_imm.current_uniforms[slot] = { data, bytes };
}

GLOBAL void imm_set_shader(Shader shader)
{
    g_imm.current_shader = shader;
}

GLOBAL void imm_set_sampler(Sampler sampler, nkS32 slot)
{
    NK_ASSERT(slot >= 0 && slot < IMM_MAX_TEXTURES);
    g_imm.current_samplers[slot] = sampler;
}

GLOBAL void imm_set_texture(Texture texture, nkS32 slot)
{
    NK_ASSERT(slot >= 0 && slot < IMM_MAX_TEXTURES);
    g_imm.current_textures[slot] = texture;
}

GLOBAL void imm_set_viewport(nkVec4 viewport)
{
    g_imm.current_viewport = viewport;
}

GLOBAL void imm_set_projection(nkMat4 projection)
{
    g_imm.current_projection = projection;
}

GLOBAL void imm_set_view(nkMat4 view)
{
    g_imm.current_view = view;
}

GLOBAL void imm_set_model(nkMat4 model)
{
    g_imm.current_model = model;
}

GLOBAL void imm_set_depth_read(nkBool enable)
{
    g_imm.current_depth_read = enable;
}

GLOBAL void imm_set_depth_write(nkBool enable)
{
    g_imm.current_depth_write = enable;
}

// =============================================================================

// State Getters ===============================================================

GLOBAL Texture imm_get_color_target(void)
{
    return g_imm.current_color_target;
}

GLOBAL Texture imm_get_depth_target(void)
{
    return g_imm.current_depth_target;
}

GLOBAL Shader imm_get_shader(void)
{
    return g_imm.current_shader;
}

GLOBAL Sampler imm_get_sampler(nkS32 slot)
{
    NK_ASSERT(slot >= 0 && slot < IMM_MAX_TEXTURES);
    return g_imm.current_samplers[slot];
}

GLOBAL Texture imm_get_texture(nkS32 slot)
{
    NK_ASSERT(slot >= 0 && slot < IMM_MAX_TEXTURES);
    return g_imm.current_textures[slot];
}

GLOBAL nkVec4 imm_get_viewport(void)
{
    return g_imm.current_viewport;
}

GLOBAL nkMat4 imm_get_projection(void)
{
    return g_imm.current_projection;
}

GLOBAL nkMat4 imm_get_view(void)
{
    return g_imm.current_view;
}

GLOBAL nkMat4 imm_get_model(void)
{
    return g_imm.current_model;
}

GLOBAL nkBool imm_get_depth_read(void)
{
    return g_imm.current_depth_read;
}

GLOBAL nkBool imm_get_depth_write(void)
{
    return g_imm.current_depth_write;
}

GLOBAL Sampler imm_get_def_sampler(ImmSampler samp)
{
    return g_imm.default_samplers[samp];
}

// =============================================================================

// Polygon Drawing =============================================================

GLOBAL void imm_begin(DrawMode draw_mode)
{
    NK_ASSERT(!g_imm.draw_started); // Cannot start a new draw inside an existing one!

    g_imm.draw_started = NK_TRUE;

    if(g_imm.current_draw_mode != draw_mode)
        g_imm.pass_needs_rebuild = NK_TRUE;
    g_imm.current_draw_mode = draw_mode;

    g_imm.position_count  = 0;
    g_imm.normal_count    = 0;
    g_imm.color_count     = 0;
    g_imm.texcoord_count  = 0;
    g_imm.userdata0_count = 0;
    g_imm.userdata1_count = 0;
    g_imm.userdata2_count = 0;
    g_imm.userdata3_count = 0;

    // Rebuild the render pass if necessary, e.g. when some parameter has changed.
    if(g_imm.pass_needs_rebuild)
    {
        if(g_imm.render_pass) free_render_pass(g_imm.render_pass);

        RenderPassDesc pass_desc;
        pass_desc.color_targets[0]     = g_imm.current_color_target;
        pass_desc.depth_stencil_target = g_imm.current_depth_target;
        pass_desc.num_color_targets    = 1;
        pass_desc.draw_mode            = g_imm.current_draw_mode;
        pass_desc.blend_mode           = BlendMode_Alpha;
        pass_desc.cull_face            = CullFace_None;
        pass_desc.depth_read           = g_imm.current_depth_read;
        pass_desc.depth_write          = g_imm.current_depth_write;
        pass_desc.clear                = NK_FALSE;
        g_imm.render_pass = create_render_pass(pass_desc);

        g_imm.pass_needs_rebuild = NK_FALSE;
    }
}

GLOBAL void imm_end(void)
{
    NK_ASSERT(g_imm.draw_started); // Cannot end a draw that has not been started!

    set_viewport(g_imm.current_viewport.x, g_imm.current_viewport.y, g_imm.current_viewport.z, g_imm.current_viewport.w);

    begin_render_pass(g_imm.render_pass);

    g_imm.vertex_layout.attribs[0].enabled = (g_imm.position_count  > 0);
    g_imm.vertex_layout.attribs[1].enabled = (g_imm.normal_count    > 0);
    g_imm.vertex_layout.attribs[2].enabled = (g_imm.color_count     > 0);
    g_imm.vertex_layout.attribs[3].enabled = (g_imm.texcoord_count  > 0);
    g_imm.vertex_layout.attribs[4].enabled = (g_imm.userdata0_count > 0);
    g_imm.vertex_layout.attribs[5].enabled = (g_imm.userdata1_count > 0);
    g_imm.vertex_layout.attribs[6].enabled = (g_imm.userdata2_count > 0);
    g_imm.vertex_layout.attribs[7].enabled = (g_imm.userdata3_count > 0);

    // Setup default samplers.
    for(nkS32 i=0; i<IMM_MAX_TEXTURES; ++i)
    {
        if(!g_imm.current_samplers[i])
            g_imm.current_samplers[i] = g_imm.default_samplers[ImmSampler_ClampNearest];
    }

    // Setup default shader.
    if(!g_imm.current_shader)
    {
        g_imm.current_shader = g_imm.default_shader;
    }

    // Bind data and shader.
    bind_buffer(g_imm.vertex_buffer);
    bind_shader(g_imm.current_shader);

    // Bind textures.
    nkBool use_texture = NK_FALSE;
    if(g_imm.texcoord_count != 0)
    {
        for(nkS32 i=0; i<IMM_MAX_TEXTURES; ++i)
        {
            if(g_imm.current_textures[i])
            {
                use_texture = NK_TRUE;
                break;
            }
        }
    }
    if(use_texture)
    {
        for(nkS32 i=0; i<IMM_MAX_TEXTURES; ++i)
        {
            if(g_imm.current_textures[i])
                bind_texture(g_imm.current_textures[i], g_imm.current_samplers[i], i);
        }
    }

    // Bind uniforms.
    ImmUniform uniforms = NK_ZERO_MEM;
    uniforms.u_projection = g_imm.current_projection;
    uniforms.u_view = g_imm.current_view;
    uniforms.u_model = g_imm.current_model;
    uniforms.u_usetex = use_texture;

    g_imm.current_uniforms[0].data = &uniforms;
    g_imm.current_uniforms[0].size = sizeof(uniforms);

    for(nkS32 i=0; i<IMM_MAX_UNIFORMS; ++i)
    {
        if(g_imm.current_uniforms[i].data)
        {
            bind_buffer(g_imm.uniform_buffers[i], i);
            update_buffer(g_imm.uniform_buffers[i], g_imm.current_uniforms[i].data, g_imm.current_uniforms[i].size);
        }
    }

    // Update data and draw.
    update_buffer(g_imm.vertex_buffer, g_imm.vertices, g_imm.position_count * sizeof(ImmVertex));

    draw_arrays(g_imm.vertex_layout, g_imm.position_count);

    end_render_pass();

    g_imm.draw_started = NK_FALSE;
}

GLOBAL void imm_position(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    g_imm.vertices[g_imm.position_count++].position = { x,y,z,w };
}

GLOBAL void imm_normal(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    g_imm.vertices[g_imm.normal_count++].normal = { x,y,z,w };
}

GLOBAL void imm_color(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    g_imm.vertices[g_imm.color_count++].color = { x,y,z,w };
}

GLOBAL void imm_texcoord(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    g_imm.vertices[g_imm.texcoord_count++].texcoord = { x,y,z,w };
}

GLOBAL void imm_userdata0(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    g_imm.vertices[g_imm.userdata0_count++].userdata0 = { x,y,z,w };
}

GLOBAL void imm_userdata1(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    g_imm.vertices[g_imm.userdata1_count++].userdata1 = { x,y,z,w };
}

GLOBAL void imm_userdata2(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    g_imm.vertices[g_imm.userdata2_count++].userdata2 = { x,y,z,w };
}

GLOBAL void imm_userdata3(nkF32 x, nkF32 y, nkF32 z, nkF32 w)
{
    NK_ASSERT(g_imm.draw_started); // Attempting to add draw data before calling imm_begin!
    g_imm.vertices[g_imm.userdata3_count++].userdata3 = { x,y,z,w };
}

// =============================================================================

// 2D Primitives ===============================================================

GLOBAL void imm_point(nkF32 x, nkF32 y, nkVec4 color)
{
    imm_begin(DrawMode_Points);
    imm_position(x,y); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_line(nkF32 x1, nkF32 y1, nkF32 x2, nkF32 y2, nkVec4 color)
{
    imm_begin(DrawMode_Lines);
    imm_position(x1,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_rect_outline(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkVec4 color)
{
    nkF32 x1 = x+0.5f;
    nkF32 y1 = y+0.5f;
    nkF32 x2 = x1+w;
    nkF32 y2 = y1+h;

    imm_begin(DrawMode_LineLoop);
    imm_position(x1,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x1,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_rect_filled(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkVec4 color)
{
    nkF32 x1 = x;
    nkF32 y1 = y;
    nkF32 x2 = x1+w;
    nkF32 y2 = y1+h;

    imm_begin(DrawMode_TriangleStrip);
    imm_position(x1,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x1,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y1); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_circle_outline(nkF32 x, nkF32 y, nkF32 r, nkS32 n, nkVec4 color)
{
    imm_begin(DrawMode_LineLoop);
    for(nkS32 i=0; i<n; ++i)
    {
        nkF32 theta = 2.0f * NK_TAU_F32 * NK_CAST(nkF32,i) / NK_CAST(nkF32,n);
        nkF32 xx = r * cosf(theta);
        nkF32 yy = r * sinf(theta);
        imm_position(xx+x,yy+y);
        imm_color(color.x,color.y,color.z,color.w);
    }
    imm_end();
}

GLOBAL void imm_circle_filled(nkF32 x, nkF32 y, nkF32 r, nkS32 n, nkVec4 color)
{
    imm_begin(DrawMode_TriangleFan);
    imm_position(x,y);
    imm_color(color.x,color.y,color.z,color.w);
    for(nkS32 i=0; i<=n; ++i)
    {
        nkF32 theta = 2.0f * NK_TAU_F32 * NK_CAST(nkF32,i) / NK_CAST(nkF32,n);
        nkF32 xx = r * cosf(theta);
        nkF32 yy = r * sinf(theta);
        imm_position(xx+x,yy+y);
        imm_color(color.x,color.y,color.z,color.w);
    }
    imm_end();
}

// =============================================================================

// 2D Textures =================================================================

GLOBAL void imm_begin_texture_batch(Texture tex)
{
    NK_ASSERT(tex); // Need to specify an actual texture for drawing!
    imm_set_texture(tex);
    imm_begin(DrawMode_Triangles);
}

GLOBAL void imm_end_texture_batch(void)
{
    imm_end();
}

GLOBAL void imm_texture(Texture tex, nkF32 x, nkF32 y, const ImmClip* clip, nkVec4 color)
{
    NK_ASSERT(tex); // Need to specify an actual texture for drawing!

    nkF32 w = NK_CAST(nkF32,get_texture_width(tex));
    nkF32 h = NK_CAST(nkF32,get_texture_height(tex));

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 x1 = x-((s2-s1)*0.5f);
    nkF32 y1 = y-((t2-t1)*0.5f);
    nkF32 x2 = x1+(s2-s1);
    nkF32 y2 = y1+(t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    imm_set_texture(tex);
    imm_begin(DrawMode_TriangleStrip);
    imm_position(x1,y2); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x1,y1); imm_texcoord(s1,t1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_texcoord(s2,t2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y1); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
}

GLOBAL void imm_texture_ex(Texture tex, nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip, nkVec4 color)
{
    NK_ASSERT(tex); // Need to specify an actual texture for drawing!

    nkF32 w = NK_CAST(nkF32,get_texture_width(tex));
    nkF32 h = NK_CAST(nkF32,get_texture_height(tex));

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 ox = x;
    nkF32 oy = y;

    nkF32 ax = ((anchor) ? (anchor->x*(s2-s1)) : (s2-s1)*0.5f);
    nkF32 ay = ((anchor) ? (anchor->y*(t2-t1)) : (t2-t1)*0.5f);

    x -= ax;
    y -= ay;

    nkF32 x1 = 0.0f;
    nkF32 y1 = 0.0f;
    nkF32 x2 = (s2-s1);
    nkF32 y2 = (t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    nkMat4 cached_matrix = imm_get_model();
    nkMat4 model_matrix = nk_m4_identity();

    model_matrix = nk_translate(model_matrix, {   ox,   oy, 0.0f });
    model_matrix = nk_scale    (model_matrix, {   sx,   sy, 1.0f });
    model_matrix = nk_rotate   (model_matrix, { 0.0f, 0.0f, 1.0f }, angle);
    model_matrix = nk_translate(model_matrix, {  -ox,  -oy, 0.0f });
    model_matrix = nk_translate(model_matrix, {    x,    y, 0.0f });

    imm_set_model(model_matrix);
    imm_set_texture(tex);
    imm_begin(DrawMode_TriangleStrip);
    imm_position(x1,y2); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x1,y1); imm_texcoord(s1,t1); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y2); imm_texcoord(s2,t2); imm_color(color.x,color.y,color.z,color.w);
    imm_position(x2,y1); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w);
    imm_end();
    imm_set_model(cached_matrix);
}

GLOBAL void imm_texture_batched(nkF32 x, nkF32 y, const ImmClip* clip, nkVec4 color)
{
    NK_ASSERT(g_imm.current_textures[0]); // Need to call imm_begin_texture_batch first!

    nkF32 w = NK_CAST(nkF32, get_texture_width(g_imm.current_textures[0]));
    nkF32 h = NK_CAST(nkF32, get_texture_height(g_imm.current_textures[0]));

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 x1 = x-((s2-s1)*0.5f);
    nkF32 y1 = y-((t2-t1)*0.5f);
    nkF32 x2 = x1+(s2-s1);
    nkF32 y2 = y1+(t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    imm_position(x1,y2); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w); // BL
    imm_position(x1,y1); imm_texcoord(s1,t1); imm_color(color.x,color.y,color.z,color.w); // TL
    imm_position(x2,y1); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w); // TR
    imm_position(x2,y1); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w); // TR
    imm_position(x2,y2); imm_texcoord(s2,t2); imm_color(color.x,color.y,color.z,color.w); // BR
    imm_position(x1,y2); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w); // BL
}

GLOBAL void imm_texture_batched_ex(nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip, nkVec4 color)
{
    NK_ASSERT(g_imm.current_textures[0]); // Need to call imm_begin_texture_batch first!

    nkF32 w = NK_CAST(nkF32, get_texture_width(g_imm.current_textures[0]));
    nkF32 h = NK_CAST(nkF32, get_texture_height(g_imm.current_textures[0]));

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = clip->x;
        t1 = clip->y;
        s2 = s1+clip->w;
        t2 = t1+clip->h;
    }

    nkF32 ox = x;
    nkF32 oy = y;

    nkF32 ax = ((anchor) ? (anchor->x*(s2-s1)) : (s2-s1)*0.5f);
    nkF32 ay = ((anchor) ? (anchor->y*(t2-t1)) : (t2-t1)*0.5f);

    x -= ax;
    y -= ay;

    nkF32 x1 = 0.0f;
    nkF32 y1 = 0.0f;
    nkF32 x2 = (s2-s1);
    nkF32 y2 = (t2-t1);

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    nkMat4 model_matrix = nk_m4_identity();

    model_matrix = nk_translate(model_matrix, {   ox,   oy, 0.0f });
    model_matrix = nk_scale    (model_matrix, {   sx,   sy, 1.0f });
    model_matrix = nk_rotate   (model_matrix, { 0.0f, 0.0f, 1.0f }, angle);
    model_matrix = nk_translate(model_matrix, {  -ox,  -oy, 0.0f });
    model_matrix = nk_translate(model_matrix, {    x,    y, 0.0f });

    nkVec4 tl = { x1,y1,0.0f,1.0f };
    nkVec4 tr = { x2,y1,0.0f,1.0f };
    nkVec4 bl = { x1,y2,0.0f,1.0f };
    nkVec4 br = { x2,y2,0.0f,1.0f };

    tl = model_matrix * tl;
    tr = model_matrix * tr;
    bl = model_matrix * bl;
    br = model_matrix * br;

    imm_position(bl.x,bl.y); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w); // BL
    imm_position(tl.x,tl.y); imm_texcoord(s1,t1); imm_color(color.x,color.y,color.z,color.w); // TL
    imm_position(tr.x,tr.y); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w); // TR
    imm_position(tr.x,tr.y); imm_texcoord(s2,t1); imm_color(color.x,color.y,color.z,color.w); // TR
    imm_position(br.x,br.y); imm_texcoord(s2,t2); imm_color(color.x,color.y,color.z,color.w); // BR
    imm_position(bl.x,bl.y); imm_texcoord(s1,t2); imm_color(color.x,color.y,color.z,color.w); // BL
}

// =============================================================================

/*////////////////////////////////////////////////////////////////////////////*/

