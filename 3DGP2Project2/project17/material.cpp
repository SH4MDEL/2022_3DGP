#include "material.h"

void Material::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	commandList->SetGraphicsRoot32BitConstants(0, 4, &(m_albedoColor), 16);
	commandList->SetGraphicsRoot32BitConstants(0, 4, &(m_emissiveColor), 20);
	commandList->SetGraphicsRoot32BitConstants(0, 4, &(m_specularColor), 24);
	commandList->SetGraphicsRoot32BitConstants(0, 4, &(m_ambientColor), 28);

	commandList->SetGraphicsRoot32BitConstants(0, 1, &(m_type), 32);

	if (m_type & MATERIAL_ALBEDO_MAP) m_albedoMap->UpdateShaderVariable(commandList);
	if (m_type & MATERIAL_SPECULAR_MAP) m_specularMap->UpdateShaderVariable(commandList);
	if (m_type & MATERIAL_NORMAL_MAP) m_normalMap->UpdateShaderVariable(commandList);
	if (m_type & MATERIAL_METALLIC_MAP) m_metallicMap->UpdateShaderVariable(commandList);
	if (m_type & MATERIAL_EMISSION_MAP) m_emissionMap->UpdateShaderVariable(commandList);
}