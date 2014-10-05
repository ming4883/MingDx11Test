struct vs_in
{
    float4 position	: POSITION;
};

struct ps_in
{
    float4 position	: SV_POSITION; // this is needed for correct linkage
    float4 texcoord : COLOR;
};

cbuffer AppData : register (b0)
{
    float time;
    float4x4 viewprojMatrix;
};

#if VERTEX_SHADER

ps_in main (vs_in input)
{
    ps_in output;
    
    output.position = mul (input.position, viewprojMatrix);
    output.texcoord.xy = input.position.xy * float2 (time,-time);
    output.texcoord.zw = float2 (0.0, 1.0);
    
    return output;
}

#endif

#if PIXEL_SHADER

Texture2D uTex : register (t0);
SamplerState uSam : register (s0);

float4 main (ps_in input) : SV_TARGET
{
    return uTex.Sample (uSam, input.texcoord.xy);
}

#endif
