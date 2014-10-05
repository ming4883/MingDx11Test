struct vs_in
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
    
#if USE_TEXCOORD1
    float2 uv2      : TEXCOORD1;
#endif
#if USE_COLOR
    float4 color    : COLOR;
#endif
};

struct ps_in
{
    float4 svposition   : SV_POSITION; // this is needed for correct linkage
    float4 texcoord : TEXCOORD0;
    float3 normal   : TEXCOORD1;
    float3 position : TEXCOORD2;
    float4 color    : COLOR;

};

cbuffer SceneData : register (b0)
{
    float4 scnAnimationTime;
    float4 scnViewPos; // camera position in world space
    float4x4 scnViewProjMatrix;
};

cbuffer ObjectData : register (b1)
{
    float4 objAnimationTime;
    float4x4 objWorldMatrix;
    float4x4 objNormalMatrix;
    float4x4 objWorldViewProjMatrix;
};

#if VERTEX_SHADER

ps_in main (vs_in input)
{
    ps_in output;
    
    output.svposition = mul (float4 (input.position, 1.0), objWorldViewProjMatrix);
    output.normal = mul (float4 (input.normal, 0.0), objNormalMatrix).xyz;
    output.position = mul (float4 (input.position, 1.0), objWorldMatrix).xyz;
    output.texcoord.xy = input.uv;
    
#if USE_TEXCOORD1
    output.texcoord.zw = input.uv2;
#else
    output.texcoord.zw = float2 (0.0, 0.0);
#endif

#if USE_COLOR
    output.color = input.color;
#else
    output.color = float4 (0.0, 0.0, 0.0, 0.0);
#endif
    return output;
}

#endif

#if PIXEL_SHADER

Texture2D uTex : register (t0);
SamplerState uSam : register (s0);

float4 main (ps_in input) : SV_TARGET
{
    float3 viewDir = normalize (scnViewPos.xyz - input.position);
    float ndotv = dot (viewDir, normalize (input.normal));
    ndotv = clamp (ndotv, 0.0, 1.0);
    ndotv = lerp (ndotv, 1.0, 0.75);

    float4 texClr = uTex.Sample (uSam, input.texcoord.xy);
    texClr.xyz *= ndotv;

    return texClr;
}

#endif
