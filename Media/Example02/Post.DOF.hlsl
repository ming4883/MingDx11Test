//--------------------------------------------------------------------------------------
// File: Post.DOF.hlsl
//
// The pixel shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPostCommon : register( b0 )
{
	matrix g_InvViewProjScaleBias	: packoffset(c0);
	float4 g_ZParams				: packoffset(c4);
	float4 g_UserParams				: packoffset(c5);
};

// g_UserParams = OutFocusBegin, OutFocusEnd, BlurRadius, Unused

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
};

SamplerState g_samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D g_txColor : register( t0 );
Texture2D g_txDepth : register( t1 );
Texture2D g_txDOFFocus : register( t2 );

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float LinearDepth(float zbufDepth)
{
	// http://www.humus.name/index.php?page=Comments&ID=256
	return 1 / (zbufDepth * g_ZParams.x + g_ZParams.y);
}

float4 ScreenToWorldPosition(float4 screenPos)
{
	// http://www.humus.name/index.php?page=Comments&ID=256
	float4 wPos = mul(screenPos, g_InvViewProjScaleBias);
	return float4(wPos.xyz / wPos.w, 1);
}

#define USE_DEPTH_ONLY 0

float DofFactor(float dist)
{
	const float fOutFocusBegin = g_UserParams.x;
	const float fOutFocusEnd = g_UserParams.y;
	
	//return smoothstep(fOutFocusBegin, fOutFocusEnd, dist);
	return min(max(dist - fOutFocusBegin, 0) / (fOutFocusEnd - fOutFocusBegin), 1);
}

float4 Main( PS_INPUT Input ) : SV_TARGET
{
	int3 vTexcoord = int3((int2)Input.vPosition.xy, 0);
	
	const float fBlurRadius = g_UserParams.z;
	
	int iWDepth, iHDepth;
	g_txDepth.GetDimensions(iWDepth, iHDepth);
	
#if USE_DEPTH_ONLY
	float fDepthFocus = LinearDepth(g_txDOFFocus.Load(int3(0, 0, 0)).x);
	float fDepthScene = LinearDepth(g_txDepth.Load(vTexcoord).x);
	float fDofFactor = DofFactor(abs(fDepthScene - fDepthFocus));
	
#else
	
	float fDepthScene = g_txDepth.Load(vTexcoord).x;
	
	float3 fPosFocus = g_txDOFFocus.Load(int3(0, 0, 0)).xyz;
	float3 fPosScene = ScreenToWorldPosition(float4(Input.vPosition.xy, fDepthScene, 1)).xyz; 
	float fDofFactor = DofFactor(distance(fPosScene, fPosFocus));
	
#endif
	
	// 2d unit poisson disk samplers
	const int iNumSamples = 16;
	
	float2 vSamples[iNumSamples];
	vSamples[ 0] = float2( 0.007937789, 0.73124397);
	vSamples[ 1] = float2(-0.10177308,-0.6509396);
	vSamples[ 2] = float2(-0.9906806,-0.63400936);
	vSamples[ 3] = float2(-0.5583586,-0.3614012);
	vSamples[ 4] = float2( 0.7163085, 0.22836149);
	vSamples[ 5] = float2(-0.65210974, 0.37117887);
	vSamples[ 6] = float2(-0.12714535, 0.112056136);
	vSamples[ 7] = float2( 0.48898065,-0.66669613);
	vSamples[ 8] = float2(-0.9744036, 0.9155904);
	vSamples[ 9] = float2( 0.9274436,-0.9896486);
	vSamples[10] = float2( 0.9782181, 0.90990245);
	vSamples[11] = float2( 0.96427417,-0.25506377);
	vSamples[12] = float2(-0.5021933,-0.9712455);
	vSamples[13] = float2( 0.3091557,-0.17652994);
	vSamples[14] = float2( 0.4665941, 0.96454906);
	vSamples[15] = float2(-0.461774, 0.9360856);
	
	float4 vOutput;
	
	[branch]
	if(fDofFactor < 1 / fBlurRadius)
	{
		vOutput = g_txColor.Load(vTexcoord);
	}
	else
	{
		vOutput = g_txColor.Load(vTexcoord) * 2;
		
		const float2 vBlurRadius = fDofFactor * fBlurRadius * float2(1.0 / iWDepth, 1.0 / iHDepth);
		
		[unroll]
		for(int i=0; i<iNumSamples; ++i)
		{
			float2 vTapTexcoord = Input.vTexcoord + vSamples[i] * vBlurRadius;
			vOutput += g_txColor.SampleLevel(g_samLinear, vTapTexcoord, 0);
		}
		
		vOutput /= (iNumSamples + 2);
	}
	
	return vOutput;
}

