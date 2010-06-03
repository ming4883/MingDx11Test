//--------------------------------------------------------------------------------------
// File: VolLight.PS.hlsl
//
// The vertex shader file for the MingDx11Test.  
// 
// Copyright (c) Chan Ka Ming. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbSceneConstants : register( b0 )
{
	matrix g_mWorld;
	matrix g_mView;
	matrix g_mProjection;
	matrix g_mViewProjection;
	float4 g_vCameraPosition;
	float4 g_vCameraParams;
	float4 g_vAmbientColor;
	float4 g_vLightVector;
	float4 g_vLightColor;
};

cbuffer cbVolLightConstants : register( b1 )
{
	float4 g_vVolSphere;	// x y z r
	float4 g_vVolColor;
};

cbuffer cbPostConstants : register( b2 )
{
	matrix g_mInvViewProjScaleBias	: packoffset(c0);
	float4 g_vZParams				: packoffset(c4);
	float4 g_vUserParams			: packoffset(c5);
};

struct GS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
};

float4 ScreenToWorldPosition(float4 screenPos)
{
	// http://www.humus.name/index.php?page=Comments&ID=256
	float4 wPos = mul(screenPos, g_mInvViewProjScaleBias);
	return float4(wPos.xyz / wPos.w, 1);
}

float4 Main( GS_OUTPUT Input ) : SV_TARGET
{
	return g_vVolColor;
}