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

Texture2D g_txDepth : register( t0 );

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

// ray-sphere intersection
#define DIST_BIAS 0.01
bool RaySphereIntersect( float3 rO, float3 rD, float3 sO, float sR, inout float tnear, inout float tfar )
{
    float3 delta = rO - sO;
    
    float A = dot( rD, rD );
    float B = 2*dot( delta, rD );
    float C = dot( delta, delta ) - sR*sR;
    
    float disc = B*B - 4.0*A*C;
    if( disc < DIST_BIAS )
    {
        return false;
    }
    else
    {
        float sqrtDisc = sqrt( disc );
        tnear = (-B - sqrtDisc ) / (2*A);
        tfar = (-B + sqrtDisc ) / (2*A);
        return true;
    }
}

float VolumeOpacity(
	float tBegin,
	float tEnd,
	float radius
	)
{
	float opacity = tEnd - tBegin;
	opacity = opacity / (2 * radius);
	opacity = max(opacity, 0);
	return opacity;
}

float4 Main( GS_OUTPUT Input ) : SV_TARGET
{
	int3 vTexcoord = int3((int2)Input.vPosition.xy, 0);
	float fDepthScene = g_txDepth.Load(vTexcoord).x;
	float3 vPosScene = ScreenToWorldPosition(float4(Input.vPosition.xy, fDepthScene, 1)).xyz; 
	
	float3 vPosLight = ScreenToWorldPosition(float4(Input.vPosition.xy, Input.vPosition.z, 1)).xyz; 
	
	float3 vRayOrig = g_vCameraPosition.xyz;
	float3 vRayDir = vPosScene - g_vCameraPosition.xyz;
	float fScene = length(vRayDir);
	vRayDir /= fScene;
	
	float3 vSphereCenter = g_vVolSphere.xyz;
	float fSphereRadius = g_vVolSphere.w;
	
	float fHitNear, fHitFar;
	if(!RaySphereIntersect(vRayOrig, vRayDir, vSphereCenter, fSphereRadius, fHitNear, fHitFar))
		discard;
	
	float fOpacity;
	if(fScene < fHitNear)
		fOpacity = 0;
	else
	{
		if(fScene > fHitFar)
		{
			fOpacity = VolumeOpacity(fHitNear, fHitFar, fSphereRadius);
		}
		else
		{
			fOpacity = VolumeOpacity(fHitNear, fScene, fSphereRadius);
		}
	}
	
	fOpacity = pow(fOpacity, 64) * g_vVolColor.w;
	
	return float4(g_vVolColor.xyz * fOpacity, fOpacity);
}