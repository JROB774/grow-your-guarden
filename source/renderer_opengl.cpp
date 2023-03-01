/*////////////////////////////////////////////////////////////////////////////*/

#if defined(BUILD_NATIVE)
#include <glew.c>
#endif // BUILD_NATIVE

#if defined(BUILD_WEB)
#include <GLES2/gl2.h>
#endif // BUILD_WEB

struct VertexAttrib
{
    nkBool     enabled;
    AttribType type;
    nkU32      components;
    nkU64      byte_offset;
};

DEFINE_PRIVATE_TYPE(VertexBuffer)
{
    nkU64        byte_stride;
    GLuint       handle;
    VertexAttrib attribs[16];
};

DEFINE_PRIVATE_TYPE(RenderTarget)
{
    GLuint        handle;
    Texture       color_target;
    SamplerFilter filter;
    SamplerWrap   wrap;
};

DEFINE_PRIVATE_TYPE(Shader)
{
    GLuint program;
};

DEFINE_PRIVATE_TYPE(Texture)
{
    GLuint handle;
    nkVec2 size;
};

struct OpenGLContext
{
    SDL_GLContext context;
    GLuint        vao;
};

INTERNAL OpenGLContext g_ogl;

GLOBAL void init_render_system(void)
{
    g_ogl.context = SDL_GL_CreateContext(NK_CAST(SDL_Window*, get_window()));
    if(!g_ogl.context)
    {
        fatal_error("Failed to create OpenGL context: %s", SDL_GetError());
    }

    // Enable VSync by default, if we don't get it then oh well.
    if(SDL_GL_SetSwapInterval(1) == 0)
    {
        printf("[OpenGL]: VSync Enabled!\n");
    }

    // Load OpenGL functions in a native build.
    #if defined(BUILD_NATIVE)
    glewInit();
    #endif // BUILD_NATIVE

    printf("[OpenGL]: GPU Renderer   : %s\n", glGetString(GL_RENDERER));
    printf("[OpenGL]: GPU Vendor     : %s\n", glGetString(GL_VENDOR));
    printf("[OpenGL]: OpenGL Version : %s\n", glGetString(GL_VERSION));
    printf("[OpenGL]: GLSL Version   : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // We need one Vertex Attribute Object in order to render with modern OpenGL.
    #if defined(BUILD_NATIVE)
    glGenVertexArrays(1, &g_ogl.vao);
    glBindVertexArray(g_ogl.vao);
    #endif // BUILD_NATIVE

    set_blend_mode(BlendMode_Alpha);
}

GLOBAL void quit_render_system(void)
{
    #if defined(BUILD_NATIVE)
    glDeleteVertexArrays(1, &g_ogl.vao);
    #endif // BUILD_NATIVE

    SDL_GL_DeleteContext(g_ogl.context);
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

GLOBAL void renderer_present(void)
{
    SDL_GL_SwapWindow(NK_CAST(SDL_Window*,get_window()));
}

GLOBAL void set_viewport(nkF32 x, nkF32 y, nkF32 w, nkF32 h)
{
    GLint   vx = NK_CAST(GLint,   x);
    GLint   vy = NK_CAST(GLint,   y);
    GLsizei vw = NK_CAST(GLsizei, w);
    GLsizei vh = NK_CAST(GLsizei, h);

    glViewport(vx,vy,vw,vh);
}

GLOBAL void set_blend_mode(BlendMode blend_mode)
{
    switch(blend_mode)
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

GLOBAL void clear_screen(nkVec4 color)
{
    glClearColor(color.r,color.g,color.b,color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

GLOBAL void clear_screen(nkVec3 color)
{
    glClearColor(color.r,color.g,color.b,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

GLOBAL void clear_screen(nkF32 r, nkF32 g, nkF32 b, nkF32 a)
{
    glClearColor(r,g,b,a);
    glClear(GL_COLOR_BUFFER_BIT);
}

GLOBAL VertexBuffer create_vertex_buffer(void)
{
    VertexBuffer vbuf = ALLOCATE_PRIVATE_TYPE(VertexBuffer);
    if(!vbuf)
        fatal_error("Failed to allocate vertex buffer!");
    glGenBuffers(1, &vbuf->handle);
    memset(vbuf->attribs, 0, sizeof(vbuf->attribs));
    return vbuf;
}

GLOBAL void free_vertex_buffer(VertexBuffer vbuf)
{
    if(!vbuf) return;
    glDeleteBuffers(1, &vbuf->handle);
    free(vbuf);
}

GLOBAL void set_vertex_buffer_stride(VertexBuffer vbuf, nkU64 byte_stride)
{
    NK_ASSERT(vbuf);

    vbuf->byte_stride = byte_stride;
}

GLOBAL void enable_vertex_buffer_attrib(VertexBuffer vbuf, nkU32 index, AttribType type, nkU32 comps, nkU64 byte_offset)
{
    NK_ASSERT(vbuf);
    NK_ASSERT(index < NK_ARRAY_SIZE(vbuf->attribs));

    vbuf->attribs[index].enabled = NK_TRUE;
    vbuf->attribs[index].type = type;
    vbuf->attribs[index].components = comps;
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

    GLenum gl_type = GL_NONE;
    switch(type)
    {
        case BufferType_Static: gl_type = GL_STATIC_DRAW; break;
        case BufferType_Dynamic: gl_type = GL_DYNAMIC_DRAW; break;
        case BufferType_Stream: gl_type = GL_STREAM_DRAW; break;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbuf->handle);
    glBufferData(GL_ARRAY_BUFFER, bytes, data, gl_type);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
}

GLOBAL void draw_vertex_buffer(VertexBuffer vbuf, DrawMode draw_mode, nkU64 vert_count)
{
    NK_ASSERT(vbuf);

    if(!vert_count) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbuf->handle);

    // Map the primitive type to the appropriate GL enum.
    GLenum primitive = GL_NONE;
    switch(draw_mode)
    {
        case DrawMode_Points: primitive = GL_POINTS; break;
        case DrawMode_LineStrip: primitive = GL_LINE_STRIP; break;
        case DrawMode_LineLoop: primitive = GL_LINE_LOOP; break;
        case DrawMode_Lines: primitive = GL_LINES; break;
        case DrawMode_TriangleStrip: primitive = GL_TRIANGLE_STRIP; break;
        case DrawMode_TriangleFan: primitive = GL_TRIANGLE_FAN; break;
        case DrawMode_Triangles: primitive = GL_TRIANGLES; break;
    }

    // Setup the attributes for the buffer.
    for(nkU64 i=0, n=NK_ARRAY_SIZE(vbuf->attribs); i<n; ++i)
    {
        VertexAttrib* attrib = &vbuf->attribs[i];
        if(attrib->enabled)
        {
            GLenum attrib_type = GL_NONE;
            switch(attrib->type)
            {
                case AttribType_SignedByte: attrib_type = GL_BYTE; break;
                case AttribType_UnsignedByte: attrib_type = GL_UNSIGNED_BYTE; break;
                case AttribType_SignedInt: attrib_type = GL_INT; break;
                case AttribType_UnsignedInt: attrib_type = GL_UNSIGNED_INT; break;
                case AttribType_Float: attrib_type = GL_FLOAT; break;
            }

            glVertexAttribPointer(NK_CAST(GLuint, i), attrib->components, attrib_type, GL_FALSE,
                NK_CAST(GLsizei, vbuf->byte_stride), NK_CAST(void*,attrib->byte_offset));
            glEnableVertexAttribArray(NK_CAST(GLuint, i));
        }
    }

    // Draw the buffer data using the provided primitive type.
    glDrawArrays(primitive, 0, NK_CAST(GLsizei,vert_count));

    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
}

GLOBAL RenderTarget create_render_target(nkS32 w, nkS32 h, SamplerFilter filter, SamplerWrap wrap)
{
    RenderTarget target = ALLOCATE_PRIVATE_TYPE(RenderTarget);
    if(!target)
        fatal_error("Failed to allocate render target!");
    target->handle = GL_NONE;
    target->color_target = NULL;
    target->filter = filter;
    target->wrap = wrap;
    resize_render_target(target, w, h);
    return target;
}

GLOBAL void free_render_target(RenderTarget target)
{
    if(!target) return;
    glDeleteFramebuffers(1, &target->handle);
    free_texture(target->color_target);
    free(target);
}

GLOBAL void resize_render_target(RenderTarget target, nkS32 w, nkS32 h)
{
    NK_ASSERT(target);

    if(w <= 0 || h <= 0) return;

    // Delete the old contents (if any).
    glDeleteFramebuffers(1, &target->handle);
    free_texture(target->color_target);

    glGenFramebuffers(1, &target->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, target->handle);

    target->color_target = create_texture(w,h, 4, NULL, target->filter, target->wrap);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->color_target->handle, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fatal_error("Failed to complete framebuffer resize!");

    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

GLOBAL void bind_render_target(RenderTarget target)
{
    if(!target) glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    else glBindFramebuffer(GL_FRAMEBUFFER, target->handle);
}

INTERNAL GLuint compile_shader(const nkChar* source, nkU64 bytes, GLenum type)
{
    const nkChar* sources[2] = { NULL, source };
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
            printf("[OpenGL]: Failed to compile shader:\n%s\n", info_log);
            NK_FREE(info_log);
        }
        return GL_NONE;
    }

    return shader;
}

GLOBAL Shader create_shader(void* data, nkU64 bytes)
{
    Shader shader = ALLOCATE_PRIVATE_TYPE(Shader);
    if(!shader)
        fatal_error("Failed to allocate shader!");

    GLuint vert = compile_shader(NK_CAST(const nkChar*, data), bytes, GL_VERTEX_SHADER);
    GLuint frag = compile_shader(NK_CAST(const nkChar*, data), bytes, GL_FRAGMENT_SHADER);

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
            printf("[OpenGL]: Failed to link shader:\n%s\n", info_log);
            free(info_log);
        }
    }

    return shader;
}

GLOBAL void free_shader(Shader shader)
{
    if(!shader) return;
    glDeleteProgram(shader->program);
    free(shader);
}

GLOBAL void bind_shader(Shader shader)
{
    if(!shader) glUseProgram(GL_NONE);
    else glUseProgram(shader->program);
}

GLOBAL void set_shader_bool(Shader shader, const nkChar* name, nkBool val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("[OpenGL]: No shader uniform found: %s\n", name);
    glUniform1i(location, NK_CAST(nkS32, val));
}

GLOBAL void set_shader_int(Shader shader, const nkChar* name, nkS32 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("[OpenGL]: No shader uniform found: %s\n", name);
    glUniform1i(location, val);
}

GLOBAL void set_shader_float(Shader shader, const nkChar* name, nkF32 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("[OpenGL]: No shader uniform found: %s\n", name);
    glUniform1f(location, val);
}

GLOBAL void set_shader_vec2(Shader shader, const nkChar* name, nkVec2 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("[OpenGL]: No shader uniform found: %s\n", name);
    glUniform2fv(location, 1, val.raw);
}

GLOBAL void set_shader_vec3(Shader shader, const nkChar* name, nkVec3 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("[OpenGL]: No shader uniform found: %s\n", name);
    glUniform3fv(location, 1, val.raw);
}

GLOBAL void set_shader_vec4(Shader shader, const nkChar* name, nkVec4 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("[OpenGL]: No shader uniform found: %s\n", name);
    glUniform4fv(location, 1, val.raw);
}

GLOBAL void set_shader_mat2(Shader shader, const nkChar* name, nkMat2 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("[OpenGL]: No shader uniform found: %s\n", name);
    glUniformMatrix2fv(location, 1, GL_FALSE, val.raw);
}

GLOBAL void set_shader_mat3(Shader shader, const nkChar* name, nkMat3 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("[OpenGL]: No shader uniform found: %s\n", name);
    glUniformMatrix3fv(location, 1, GL_FALSE, val.raw);
}

GLOBAL void set_shader_mat4(Shader shader, const nkChar* name, nkMat4 val)
{
    NK_ASSERT(shader);
    GLint location = glGetUniformLocation(shader->program, name);
    if(location == -1) printf("[OpenGL]: No shader uniform found: %s\n", name);
    glUniformMatrix4fv(location, 1, GL_FALSE, val.raw);
}

INTERNAL GLenum sampler_filter_to_gl(SamplerFilter filter)
{
    switch(filter)
    {
        case SamplerFilter_Nearest: return GL_NEAREST;
        case SamplerFilter_Linear: return GL_LINEAR;
        case SamplerFilter_NearestWithNearestMips: return GL_NEAREST_MIPMAP_NEAREST;
        case SamplerFilter_LinearWithNearestMips: return GL_LINEAR_MIPMAP_NEAREST;
        case SamplerFilter_NearestWithLinearMips: return GL_NEAREST_MIPMAP_LINEAR;
        case SamplerFilter_LinearWithLinearMips: return GL_LINEAR_MIPMAP_LINEAR;
        default:
        {
            // Unsupported texture filter.
            NK_ASSERT(NK_FALSE);
        } break;
    }
    return GL_NONE;
}

INTERNAL GLenum sampler_wrap_to_gl(SamplerWrap wrap)
{
    switch(wrap)
    {
        case SamplerWrap_Repeat: return GL_REPEAT;
        case SamplerWrap_Clamp: return GL_CLAMP_TO_EDGE;
        default:
        {
            // Unsupported texture wrap.
            NK_ASSERT(NK_FALSE);
        } break;
    }
    return GL_NONE;
}

INTERNAL GLenum bpp_to_gl_format(nkS32 bpp)
{
    switch(bpp)
    {
        #if defined(BUILD_NATIVE)
        case 1: return GL_RED;
        case 2: return GL_RG;
        #endif // BUILD_NATIVE

        #if defined(BUILD_WEB)
        case 1: return GL_LUMINANCE;
        case 2: return GL_LUMINANCE_ALPHA;
        #endif // BUILD_WEB

        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default:
        {
            // Unsupported BPP that has no appropriate format.
            NK_ASSERT(NK_FALSE);
        } break;
    }
    return GL_NONE;
}

GLOBAL Texture create_texture(nkS32 w, nkS32 h, nkS32 bpp, void* data, SamplerFilter filter, SamplerWrap wrap)
{
    Texture texture = ALLOCATE_PRIVATE_TYPE(Texture);
    if(!texture)
        fatal_error("Failed to allocate texture!");

    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &texture->handle);
    glBindTexture(GL_TEXTURE_2D, texture->handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler_filter_to_gl(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler_filter_to_gl(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler_wrap_to_gl(wrap));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler_wrap_to_gl(wrap));

    GLenum gl_format = bpp_to_gl_format(bpp);
    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, w,h, 0, gl_format, GL_UNSIGNED_BYTE, data);

    if(filter > SamplerFilter_Linear)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    texture->size.x = NK_CAST(nkF32, w);
    texture->size.y = NK_CAST(nkF32, h);

    return texture;
}

GLOBAL void free_texture(Texture texture)
{
    if(!texture) return;
    glDeleteTextures(1, &texture->handle);
    free(texture);
}

GLOBAL void bind_texture(Texture texture, nkS32 unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    if(!texture) glBindTexture(GL_TEXTURE_2D, GL_NONE);
    else glBindTexture(GL_TEXTURE_2D, texture->handle);
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
