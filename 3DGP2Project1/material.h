#pragma once
#include "stdafx.h"
#include "texture.h"

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

// 재질(Material)
// 빛이 물체의 표면과 상호작용하는 방식을 결정하는 속성들의 집합
struct Material
{
	Material() : m_albedoColor{ XMFLOAT4(0.f, 0.f, 0.f, 1.f) }, m_emissiveColor{ XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) },
		m_specularColor{ XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) }, m_ambientColor{ XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) }, m_type{ 0x00 } {}
	~Material() = default;

	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	UINT					m_type;

	XMFLOAT4				m_albedoColor;
	XMFLOAT4				m_emissiveColor;
	XMFLOAT4				m_specularColor;
	XMFLOAT4				m_ambientColor;

	FLOAT					m_glossiness;
	FLOAT					m_smoothness;
	FLOAT					m_metalic;
	FLOAT					m_specularHighlight;
	FLOAT					m_glossyReflection;

	shared_ptr<Texture>		m_albedoMap;
	shared_ptr<Texture>		m_specularMap;
	shared_ptr<Texture>		m_normalMap;
	shared_ptr<Texture>		m_metallicMap;
	shared_ptr<Texture>		m_emissionMap;
	shared_ptr<Texture>		m_detailAlbedoMap;
	shared_ptr<Texture>		m_detailNormalMap;
};
