uniform sampler2D u_texture;

layout(std140) uniform Imm
{
    mat4 u_projection;
    mat4 u_view;
    mat4 u_model;
    bool u_usetex;
};

#ifdef VERT_SHADER /*/////////////////////////////////////////////////////////*/

layout (location = 0) in vec4 i_position;
layout (location = 1) in vec4 i_normal;
layout (location = 2) in vec4 i_color;
layout (location = 3) in vec4 i_texcoord;
layout (location = 4) in vec4 i_userdata0;
layout (location = 5) in vec4 i_userdata1;
layout (location = 6) in vec4 i_userdata2;
layout (location = 7) in vec4 i_userdata3;

out vec4 v_color;
out vec2 v_texcoord;

void main()
{
    gl_Position = u_projection * u_view * u_model * i_position;
    v_color = i_color;
    v_texcoord = i_texcoord.xy;
}

#endif /* VERT_SHADER ////////////////////////////////////////////////////////*/

#ifdef FRAG_SHADER /*/////////////////////////////////////////////////////////*/

in vec4 v_color;
in vec2 v_texcoord;

out vec4 o_fragcolor;

void main()
{
    o_fragcolor = vec4(v_color.rgb, v_color.a * texture(u_texture, v_texcoord).r);
}

#endif /* FRAG_SHADER ////////////////////////////////////////////////////////*/
