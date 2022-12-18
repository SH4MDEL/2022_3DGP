struct MATERIAL
{
	float4					diffuse;
	float4					emissive;
	float4					specular; //a = power
	float4					ambient;
};

cbuffer cbGameObject : register(b0)
{
	matrix worldMatrix : packoffset(c0);
	MATERIAL material : packoffset(c4);
	uint textureMask : packoffset(c8);
};

cbuffer cbCamera : register(b1)
{
	matrix viewMatrix : packoffset(c0);
	matrix projMatrix : packoffset(c4);
	float3 cameraPosition : packoffset(c8);
};

SamplerState g_samplerWrap : register(s0);
SamplerState g_samplerClamp : register(s1);

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

/*
 *  STANDARD_SHADER 
 */
struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_STANDARD_OUTPUT VS_STANDARD_MAIN(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;
	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix);
	return output;
}

[earlydepthstencil]
float4 PS_STANDARD_MAIN(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 color = material.diffuse + material.emissive + material.specular + material.ambient;
	return color;
}

/*
 *  INSTANCE_SHADER
 */
struct VS_INSTANCE_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	matrix worldMatrix : INSTANCE;
};

struct VS_INSTANCE_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_INSTANCE_OUTPUT VS_INSTANCE_MAIN(VS_INSTANCE_INPUT input)
{
	VS_INSTANCE_OUTPUT output;
	output.position = mul(float4(input.position, 1.0f), input.worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix);
	output.color = input.color;
	return output;
}

[earlydepthstencil]
float4 PS_INSTANCE_MAIN(VS_INSTANCE_OUTPUT input) : SV_TARGET
{
	return input.color;
}

/*
 * TEXTUREHIERARCHY_SHADER
 */
Texture2D g_albedoTexture : register(t4);
Texture2D g_specularTexture : register(t5);
Texture2D g_normalTexture : register(t6);
Texture2D g_metallicTexture : register(t7);
Texture2D g_emissionTexture : register(t8);
Texture2D g_detailAlbedoTexture : register(t9);
Texture2D g_detailNormalTexture : register(t10);

struct VS_TEXTUREHIERARCHY_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 biTangent : BITANGENT;
	float2 uv : TEXCOORD;
};

struct VS_TEXTUREHIERARCHY_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 biTangent : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_TEXTUREHIERARCHY_OUTPUT VS_TEXTUREHIERARCHY_MAIN(VS_TEXTUREHIERARCHY_INPUT input)
{
	VS_TEXTUREHIERARCHY_OUTPUT output;
	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix);
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	output.biTangent = mul(input.biTangent, (float3x3)worldMatrix);
	output.uv = input.uv;
	return output;
}

[earlydepthstencil]
float4 PS_TEXTUREHIERARCHY_MAIN(VS_TEXTUREHIERARCHY_OUTPUT input) : SV_TARGET
{
	float4 albedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 specularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 normalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 metallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 emissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (textureMask & MATERIAL_ALBEDO_MAP) albedoColor = g_albedoTexture.Sample(g_samplerWrap, input.uv);
	if (textureMask & MATERIAL_SPECULAR_MAP) specularColor = g_specularTexture.Sample(g_samplerWrap, input.uv);
	if (textureMask & MATERIAL_NORMAL_MAP) normalColor = g_normalTexture.Sample(g_samplerWrap, input.uv);
	if (textureMask & MATERIAL_METALLIC_MAP) metallicColor = g_metallicTexture.Sample(g_samplerWrap, input.uv);
	if (textureMask & MATERIAL_EMISSION_MAP) emissionColor = g_emissionTexture.Sample(g_samplerWrap, input.uv);

	float4 color = albedoColor + specularColor + emissionColor;

	return color;
}

/*
 *  TERRAIN_SHADER
 */
Texture2D g_baseTexture : register(t0);
Texture2D g_detailTexture : register(t1);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VS_TERRAIN_MAIN(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix);
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;
	return output;
}

[earlydepthstencil]
float4 PS_TERRAIN_MAIN(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 baseTexColor = g_baseTexture.Sample(g_samplerWrap, input.uv0);
	float4 detailTexColor = g_detailTexture.Sample(g_samplerWrap, input.uv1);

	return saturate((baseTexColor * 0.5f) + (detailTexColor * 0.5f));
}

/*
 *  SKYBOX_SHADER
 */
TextureCube g_skyboxTexture : register(t2);

struct VS_SKYBOX_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_OUTPUT VS_SKYBOX_MAIN(VS_SKYBOX_INPUT input)
{
	VS_SKYBOX_OUTPUT output;

	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix).xyww;
	output.positionL = input.position;

	return output;
}

[earlydepthstencil]
float4 PS_SKYBOX_MAIN(VS_SKYBOX_OUTPUT input) : SV_TARGET
{
	float4 color = g_skyboxTexture.Sample(g_samplerClamp, input.positionL);
	return color;
}

/*
 *  BLENDING_SHADER
 */
Texture2D g_riverTexture : register(t3);

struct VS_BLENDING_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_BLENDING_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_BLENDING_OUTPUT VS_BLENDING_MAIN(VS_BLENDING_INPUT input)
{
	VS_BLENDING_OUTPUT output;

	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projMatrix);
	output.uv = input.uv;

	return output;
}

float4 PS_BLENDING_MAIN(VS_BLENDING_OUTPUT input) : SV_TARGET
{
	float4 color = g_riverTexture.Sample(g_samplerWrap, input.uv);
	color.a = material.diffuse.a;
	return color;
}

/*
 *  BILLBOARD_SHADER
 */

struct VS_BILLBOARD_INPUT
{
	float3 position : POSITION;
	float2 size : SIZE;
	float3 wposition : WPOSITION;
};

struct VS_BILLBOARD_OUTPUT
{
	float4 position : POSITION;
	float2 size : SIZE;
};

struct GS_BILLBOARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

VS_BILLBOARD_OUTPUT VS_BILLBOARD_MAIN(VS_BILLBOARD_INPUT input)
{
	VS_BILLBOARD_OUTPUT output;
	output.position = float4(input.wposition, 1.0f);
	output.size = input.size;
	return output;
}

[maxvertexcount(4)]
void GS_BILLBOARD_MAIN(point VS_BILLBOARD_OUTPUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_BILLBOARD_OUTPUT> outStream)
{
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = cameraPosition - input[0].position.xyz;
	look = normalize(look);
	float3 right = cross(up, look);
	float halfW = input[0].size.x * 0.5f;
	float halfH = input[0].size.y * 0.5f;
	float4 vertices[4];
	vertices[0] = float4(input[0].position.xyz + halfW * right - halfH * up, 1.0f);
	vertices[1] = float4(input[0].position.xyz + halfW * right + halfH * up, 1.0f);
	vertices[2] = float4(input[0].position.xyz - halfW * right - halfH * up, 1.0f);
	vertices[3] = float4(input[0].position.xyz - halfW * right + halfH * up, 1.0f);
	float2 uv[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };

	GS_BILLBOARD_OUTPUT output;
	for (int i = 0; i < 4; ++i) {
		output.position = mul(vertices[i], viewMatrix);
		output.position = mul(output.position, projMatrix);
		output.normal = look;
		output.uv = uv[i];
		outStream.Append(output);
	}
}

[earlydepthstencil]
float4 PS_BILLBOARD_MAIN(GS_BILLBOARD_OUTPUT input) : SV_TARGET
{
	float4 color = g_baseTexture.Sample(g_samplerWrap, input.uv);
	return color;
}