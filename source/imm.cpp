/*////////////////////////////////////////////////////////////////////////////*/

INTERNAL constexpr nkU32 IMM_INITIAL_VERTS = 16384;

struct ImmContext
{
    DrawMode           draw_mode;
    VertexBuffer       buffer;
    Shader             shader;
    Shader             bound_shader;
    Texture            bound_texture;
    nkMat4             projection;
    nkMat4             view;
    nkMat4             model;
    nkArray<ImmVertex> vertices;
};

INTERNAL ImmContext g_imm;

GLOBAL void imm_init(void)
{
    g_imm.shader = asset_manager_load<Shader>("basic.shader");
    g_imm.buffer = create_vertex_buffer();
    set_vertex_buffer_stride   (g_imm.buffer, sizeof(ImmVertex));
    enable_vertex_buffer_attrib(g_imm.buffer, 0, AttribSemantic_Pos, AttribType_Float2, offsetof(ImmVertex, pos));
    enable_vertex_buffer_attrib(g_imm.buffer, 1, AttribSemantic_Tex, AttribType_Float2, offsetof(ImmVertex, tex));
    enable_vertex_buffer_attrib(g_imm.buffer, 2, AttribSemantic_Col, AttribType_Float4, offsetof(ImmVertex, col));

    g_imm.projection = nk_m4_identity();
    g_imm.view       = nk_m4_identity();
    g_imm.model      = nk_m4_identity();

    nk_array_reserve(&g_imm.vertices, IMM_INITIAL_VERTS);
}

GLOBAL void imm_quit(void)
{
    nk_array_free(&g_imm.vertices);
    free_vertex_buffer(g_imm.buffer);
}

GLOBAL nkMat4 imm_get_projection(void)
{
    return g_imm.projection;
}

GLOBAL nkMat4 imm_get_view(void)
{
    return g_imm.view;
}

GLOBAL nkMat4 imm_get_model(void)
{
    return g_imm.model;
}

GLOBAL void imm_set_projection(nkMat4 projection)
{
    g_imm.projection = projection;
}

GLOBAL void imm_set_view(nkMat4 view)
{
    g_imm.view = view;
}

GLOBAL void imm_set_model(nkMat4 model)
{
    g_imm.model = model;
}

GLOBAL void imm_set_viewport(nkVec4 viewport)
{
    set_viewport(viewport.x,viewport.y,viewport.z,viewport.w);
}

GLOBAL void imm_reset(void)
{
    g_imm.projection = nk_m4_identity();
    g_imm.view = nk_m4_identity();
    g_imm.model = nk_m4_identity();
}

GLOBAL void imm_begin(DrawMode draw_mode, Texture tex, Shader shader)
{
    g_imm.draw_mode = draw_mode;
    g_imm.bound_texture = tex;
    g_imm.bound_shader = shader;

    nk_array_clear(&g_imm.vertices);

    if(!g_imm.bound_shader)
    {
        g_imm.bound_shader = g_imm.shader;
    }
}

GLOBAL void imm_end(void)
{
    bind_texture(g_imm.bound_texture, 0);
    bind_shader(g_imm.bound_shader);

    set_shader_bool(g_imm.bound_shader, "u_usetex",    (g_imm.bound_texture != NULL));
    set_shader_mat4(g_imm.bound_shader, "u_projection", g_imm.projection);
    set_shader_mat4(g_imm.bound_shader, "u_view",       g_imm.view);
    set_shader_mat4(g_imm.bound_shader, "u_model",      g_imm.model);

    update_vertex_buffer(g_imm.buffer, g_imm.vertices.data, g_imm.vertices.length * sizeof(ImmVertex), BufferType_Dynamic);
    draw_vertex_buffer(g_imm.buffer, g_imm.draw_mode, g_imm.vertices.length);
}

GLOBAL void imm_vertex(ImmVertex v)
{
    nk_array_append(&g_imm.vertices, v);
}

GLOBAL void imm_point(nkF32 x, nkF32 y, nkVec4 color)
{
    imm_begin(DrawMode_Points, NULL, NULL);
    imm_vertex({ { x,y }, { 0,0 }, color });
    imm_end();
}

GLOBAL void imm_Line(nkF32 x1, nkF32 y1, nkF32 x2, nkF32 y2, nkVec4 color)
{
    imm_begin(DrawMode_Lines, NULL, NULL);
    imm_vertex({ { x1,y1 }, { 0,0 }, color });
    imm_vertex({ { x2,y2 }, { 1,1 }, color });
    imm_end();
}

GLOBAL void imm_rect_outline(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkVec4 color)
{
    x += 0.5f;
    y += 0.5f;

    nkF32 x1 = x;
    nkF32 y1 = y;
    nkF32 x2 = x+w;
    nkF32 y2 = y+h;

    imm_begin(DrawMode_LineLoop, NULL, NULL);
    imm_vertex({ { x1,y1 }, { 0,0 }, color });
    imm_vertex({ { x2,y1 }, { 1,0 }, color });
    imm_vertex({ { x2,y2 }, { 1,1 }, color });
    imm_vertex({ { x1,y2 }, { 0,1 }, color });
    imm_end();
}

GLOBAL void imm_rect_filled(nkF32 x, nkF32 y, nkF32 w, nkF32 h, nkVec4 color)
{
    nkF32 x1 = x;
    nkF32 y1 = y;
    nkF32 x2 = x+w;
    nkF32 y2 = y+h;

    imm_begin(DrawMode_TriangleStrip, NULL, NULL);
    imm_vertex({ { x1,y2 }, { 0,1 }, color });
    imm_vertex({ { x1,y1 }, { 0,0 }, color });
    imm_vertex({ { x2,y2 }, { 1,1 }, color });
    imm_vertex({ { x2,y1 }, { 1,0 }, color });
    imm_end();
}

GLOBAL void imm_circle_outline(nkF32 x, nkF32 y, nkF32 r, nkS32 n, nkVec4 color)
{
    imm_begin(DrawMode_LineLoop, NULL, NULL);
    for(nkS32 i=0; i<n; ++i)
    {
        nkF32 theta = 2.0f * NK_TAU_F32 * NK_CAST(nkF32,i) / NK_CAST(nkF32,n);
        nkF32 xx = r * cosf(theta);
        nkF32 yy = r * sinf(theta);
        imm_vertex({ { xx+x,yy+y }, { 0,0 }, color });
    }
    imm_end();
}

GLOBAL void imm_circle_filled(nkF32 x, nkF32 y, nkF32 r, nkS32 n, nkVec4 color)
{
    imm_begin(DrawMode_TriangleFan, NULL, NULL);
    imm_vertex({ { x,y }, { 0,0 }, color });
    for(nkS32 i=0; i<=n; ++i)
    {
        nkF32 theta = 2.0f * NK_TAU_F32 * NK_CAST(nkF32,i) / NK_CAST(nkF32,n);
        nkF32 xx = r * cosf(theta);
        nkF32 yy = r * sinf(theta);
        imm_vertex({ { xx+x,yy+y }, { 0,0 }, color });
    }
    imm_end();
}

GLOBAL void imm_begin_texture_batch(Texture tex)
{
    imm_begin(DrawMode_Triangles, tex, NULL);
}

GLOBAL void imm_end_texture_batch(void)
{
    imm_end();
}

GLOBAL void imm_texture(Texture tex, nkF32 x, nkF32 y, const ImmClip* clip, nkVec4 color)
{
    nkF32 w = NK_CAST(nkF32, get_texture_width(tex));
    nkF32 h = NK_CAST(nkF32, get_texture_height(tex));

    nkF32 s1 = 0;
    nkF32 t1 = 0;
    nkF32 s2 = w;
    nkF32 t2 = h;

    if(clip)
    {
        s1 = (clip->x   );
        t1 = (clip->y   );
        s2 = (s1+clip->w);
        t2 = (t1+clip->h);
    }

    nkF32 x1 = x-((s2-s1)*0.5f);
    nkF32 y1 = y-((t2-t1)*0.5f);
    nkF32 x2 = x1+(s2-s1);
    nkF32 y2 = y1+(t2-t1);

    s1 += 0.5f;
    t1 += 0.5f;
    s2 -= 0.5f;
    t2 -= 0.5f;

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    imm_begin(DrawMode_TriangleStrip, tex, NULL);
    imm_vertex({ { x1,y2 }, { s1,t2 }, color });
    imm_vertex({ { x1,y1 }, { s1,t1 }, color });
    imm_vertex({ { x2,y2 }, { s2,t2 }, color });
    imm_vertex({ { x2,y1 }, { s2,t1 }, color });
    imm_end();
}

GLOBAL void imm_texture_ex(Texture tex, nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip, nkVec4 color)
{
    nkF32 w = NK_CAST(nkF32, get_texture_width(tex));
    nkF32 h = NK_CAST(nkF32, get_texture_height(tex));

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

    s1 += 0.5f;
    t1 += 0.5f;
    s2 -= 0.5f;
    t2 -= 0.5f;

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
    imm_begin(DrawMode_TriangleStrip, tex, NULL);
    imm_vertex({ { x1,y2 }, { s1,t2 }, color });
    imm_vertex({ { x1,y1 }, { s1,t1 }, color });
    imm_vertex({ { x2,y2 }, { s2,t2 }, color });
    imm_vertex({ { x2,y1 }, { s2,t1 }, color });
    imm_end();
    imm_set_model(cached_matrix);
}

GLOBAL void imm_texture_batched(nkF32 x, nkF32 y, const ImmClip* clip, nkVec4 color)
{
    NK_ASSERT(g_imm.bound_texture);

    nkF32 w = NK_CAST(nkF32, get_texture_width(g_imm.bound_texture));
    nkF32 h = NK_CAST(nkF32, get_texture_height(g_imm.bound_texture));

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

    s1 += 0.5f;
    t1 += 0.5f;
    s2 -= 0.5f;
    t2 -= 0.5f;

    s1 /= w;
    t1 /= h;
    s2 /= w;
    t2 /= h;

    imm_vertex({ { x1,y2 }, { s1,t2 }, color }); // BL
    imm_vertex({ { x1,y1 }, { s1,t1 }, color }); // TL
    imm_vertex({ { x2,y1 }, { s2,t1 }, color }); // TR
    imm_vertex({ { x2,y1 }, { s2,t1 }, color }); // TR
    imm_vertex({ { x2,y2 }, { s2,t2 }, color }); // BR
    imm_vertex({ { x1,y2 }, { s1,t2 }, color }); // BL
}

GLOBAL void imm_texture_batched_ex(nkF32 x, nkF32 y, nkF32 sx, nkF32 sy, nkF32 angle, nkVec2* anchor, const ImmClip* clip, nkVec4 color)
{
    NK_ASSERT(g_imm.bound_texture);

    nkF32 w = NK_CAST(nkF32, get_texture_width(g_imm.bound_texture));
    nkF32 h = NK_CAST(nkF32, get_texture_height(g_imm.bound_texture));

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

    s1 += 0.5f;
    t1 += 0.5f;
    s2 -= 0.5f;
    t2 -= 0.5f;

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

    imm_vertex({ { bl.x,bl.y }, { s1,t2 }, color });
    imm_vertex({ { tl.x,tl.y }, { s1,t1 }, color });
    imm_vertex({ { tr.x,tr.y }, { s2,t1 }, color });
    imm_vertex({ { tr.x,tr.y }, { s2,t1 }, color });
    imm_vertex({ { br.x,br.y }, { s2,t2 }, color });
    imm_vertex({ { bl.x,bl.y }, { s1,t2 }, color });
}

/*////////////////////////////////////////////////////////////////////////////*/
