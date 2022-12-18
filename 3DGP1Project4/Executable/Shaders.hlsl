
cbuffer cbCameraInfo : register(b0)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b1)
{
	matrix		gmtxGameObject : packoffset(c0);
};

cbuffer cbConstantsInfo : register(b4)
{
	int			gnMaterial : packoffset(c0.x);
	float4		gcAlbedoColor : packoffset(c1);
	float4		gcEmissionColor : packoffset(c2);
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 positionH : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
#ifdef _WITH_VERTEX_LIGHTING
	float4 color : COLOR;
#endif
};

VS_OUTPUT VSLighting(VS_INPUT input)
{
	VS_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.positionH = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	float3 normalW = mul(input.normal, (float3x3)gmtxGameObject);
#ifdef _WITH_VERTEX_LIGHTING
	output.color = Lighting(output.positionW, normalize(normalW));
	output.color = float4(0.5f * normalize(gvCameraPosition - output.positionW) + 0.5f, 1.0f);
#else
	output.normalW = normalW;
#endif

	return(output);
}

float4 PSLighting(VS_OUTPUT input) : SV_TARGET
{
#ifdef _WITH_VERTEX_LIGHTING
//	return(float4(input.positionW, 1.0f));
	return(input.color);
#else
	float3 normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, normalW);
	return(cIllumination * gcAlbedoColor + gcEmissionColor);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
static float3 gDirectionalLight = float3(0.0f, -1.0f, 1.0f);
static float4 gLightColor = float4(0.0135f, 0.035f, 0.357f, 1.0f);

float4 PSPlayer(VS_OUTPUT input) : SV_TARGET
{
	float4 color;
#ifdef _WITH_VERTEX_LIGHTING
	color = input.color;
	return(color);
#else
	float3 normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, normalW);
	cIllumination = saturate(float4(0.12f, 0.12f, 0.12f, 1.0f) + gLightColor * abs(dot(normalize(input.normalW), normalize(-gDirectionalLight))));
	return(cIllumination);
#endif
//	return(cIllumination + gLightColor + gcEmissionColor * 0.15f);
//	return(color * cIllumination * gcAlbedoColor + gcEmissionColor);
}


