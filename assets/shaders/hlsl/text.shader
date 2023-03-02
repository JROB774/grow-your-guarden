Texture2D    u_texture;
SamplerState u_sampler;

cbuffer imm: register(b0)
{
    float4x4 u_projection;
    float4x4 u_view;
    float4x4 u_model;
    bool     u_usetex;
};

struct VertInput
{
    float2 pos : POS;
    float2 tex : TEXCOORD0;
    float4 col : COL;
};

struct FragInput
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 col : COL;
};

FragInput vs_main(VertInput input)
{
    FragInput output;
    output.pos = mul(mul(mul(float4(input.pos,0,1), u_model), u_view), u_projection);
    output.tex = input.tex;
    output.col = input.col;
    return output;
}

float4 ps_main(FragInput input) : SV_TARGET
{
    float4 frag_color = input.col;
    if(u_usetex)
        frag_color.a *= u_texture.Sample(u_sampler, input.tex).r;
    return frag_color;
}
