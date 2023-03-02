uniform sampler2D u_texture;

layout(std140) uniform Uniforms
{
    mat4 u_projection;
    mat4 u_view;
    mat4 u_model;
    bool u_usetex;
};

#ifdef VERT_SHADER /*/////////////////////////////////////////////////////////*/

layout (location = 0) in vec2 i_pos;
layout (location = 1) in vec2 i_tex;
layout (location = 2) in vec4 i_col;

out vec2 v_texcoord;
out vec4 v_color;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(i_pos,0,1);
    v_texcoord = i_tex;
    v_color = i_col;
}

#endif /* VERT_SHADER ////////////////////////////////////////////////////////*/

#ifdef FRAG_SHADER /*/////////////////////////////////////////////////////////*/

in vec2 v_texcoord;
in vec4 v_color;

out vec4 o_fragcolor;

void main()
{
    o_fragcolor = v_color;
    if(u_usetex)
    {
        o_fragcolor *= texture(u_texture, v_texcoord);
    }
}

#endif /* FRAG_SHADER ////////////////////////////////////////////////////////*/
