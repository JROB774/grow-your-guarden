Texture2D    u_texture;
SamplerState u_sampler;

cbuffer uniforms: register(b0)
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
    output.pos = mul(u_projection, mul(u_view, mul(u_model, float4(input.pos,0,1))));
    output.tex = input.tex;
    output.col = input.col;
    return output;
}

float4 ps_main(FragInput input) : SV_TARGET
{
    float4 frag_color = input.col;
    if(u_usetex)
        frag_color *= u_texture.Sample(u_sampler, input.tex);
    return frag_color;
}
