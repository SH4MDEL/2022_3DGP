//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

public:
	XMFLOAT4X4						m_xmf4x4World;

public:

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) = 0;
	virtual void ReleaseShaderVariables() = 0;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) = 0;

	virtual void OnPrepareRender() = 0;
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL) = 0;

	virtual void ReleaseUploadBuffers() = 0;

	virtual XMFLOAT3 GetPosition() = 0;
	virtual XMFLOAT3 GetLook() = 0;
	virtual XMFLOAT3 GetUp() = 0;
	virtual XMFLOAT3 GetRight() = 0;

	virtual void SetPosition(float x, float y, float z) = 0;
	virtual void SetPosition(XMFLOAT3 xmf3Position) = 0;

	virtual void MoveStrafe(float fDistance = 1.0f) = 0;
	virtual void MoveUp(float fDistance = 1.0f) = 0;
	virtual void MoveForward(float fDistance = 1.0f) = 0;

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f) = 0;
	virtual void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle) = 0;

};
