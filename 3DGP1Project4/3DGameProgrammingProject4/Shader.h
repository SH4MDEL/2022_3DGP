//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Camera.h"

extern CGameObject_Village **LoadGameObjectsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName, int *pnGameObjects, int *pnSceneTextures);

class CShader
{
public:
	CShader();
	virtual ~CShader();

protected:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() = 0;
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() = 0;
	virtual D3D12_BLEND_DESC CreateBlendState() = 0;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() = 0;

	virtual D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob) = 0;


	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) = 0;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) = 0;
	virtual void ReleaseShaderVariables() = 0;

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World) = 0;

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL) = 0;
	virtual void AnimateObjects(float fTimeElapsed) = 0;
	virtual void ReleaseObjects() = 0;

protected:
	ID3D12PipelineState				*m_pd3dPipelineState = NULL;
};