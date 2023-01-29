uniform mat4 u_projection;

uniform sampler2D u_texture;

#ifdef VERT_SHADER /*/////////////////////////////////////////////////////////*/

layout (location = 0) in vec4 i_xyuv;

out vec2 v_texcoord;

void main()
{
    gl_Position = u_projection * vec4(i_xyuv.xy,0,1);
    v_texcoord = i_xyuv.zw;
}

#endif /* VERT_SHADER ////////////////////////////////////////////////////////*/

#ifdef FRAG_SHADER /*/////////////////////////////////////////////////////////*/

in vec2 v_texcoord;

out vec4 o_fragcolor;

void main()
{
    o_fragcolor = texture(u_texture, v_texcoord);
}

#endif /* FRAG_SHADER ////////////////////////////////////////////////////////*/
