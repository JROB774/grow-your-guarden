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

GLOBAL void imm_reset(void);

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
