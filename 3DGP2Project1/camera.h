#pragma once
#include "stdafx.h"
#include "player.h"

#define MAX_ROLL +20
#define MIN_ROLL -10

struct CameraInfo
{
	XMFLOAT4X4			viewMatrix;		// 뷰변환 행렬
	XMFLOAT4X4			projMatrix;		// 투영변환 행렬
	XMFLOAT3			cameraPosition;	// 카메라 위치
};

class Camera
{
public:
	Camera();
	~Camera() = default;

	void CreateShaderVariable(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList);
	void UpdateLocalAxis();
	virtual void Update(FLOAT timeElapsed) { };

	void Move(const XMFLOAT3& shift);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);

	XMFLOAT4X4 GetViewMatrix() const { return m_viewMatrix; }
	XMFLOAT4X4 GetProjMatrix() const { return m_projMatrix; }
	void SetViewMatrix(const XMFLOAT4X4& viewMatrix) { m_viewMatrix = viewMatrix; }
	void SetProjMatrix(const XMFLOAT4X4& projMatrix) { m_projMatrix = projMatrix; }

	XMFLOAT3 GetEye() const { return m_eye; }
	XMFLOAT3 GetAt() const { return m_look; }
	XMFLOAT3 GetUp() const { return m_up; }
	void SetEye(const XMFLOAT3& eye) { m_eye = eye; UpdateLocalAxis(); }
	void SetAt(const XMFLOAT3& at) { m_look = at; UpdateLocalAxis(); }
	void SetUp(const XMFLOAT3& up) { m_up = up; UpdateLocalAxis(); }

	XMFLOAT3 GetU() const { return m_u; }
	XMFLOAT3 GetV() const { return m_v; }
	XMFLOAT3 GetN() const { return m_n; }

	shared_ptr<Player> GetPlayer() const { return m_player; }
	void SetPlayer(const shared_ptr<Player>& player);

protected:
	XMFLOAT4X4				m_viewMatrix;	// 뷰변환 행렬
	XMFLOAT4X4				m_projMatrix;	// 투영변환 행렬

	ComPtr<ID3D12Resource>	m_cameraBuffer;
	CameraInfo*				m_cameraBufferPointer;

	XMFLOAT3				m_eye;			// 카메라 위치
	XMFLOAT3				m_look;			// 카메라가 바라보는 방향
	XMFLOAT3				m_up;			// 카메라 Up벡터

	XMFLOAT3				m_u;			// 로컬 x축
	XMFLOAT3				m_v;			// 로컬 y축
	XMFLOAT3				m_n;			// 로컬 z축

	FLOAT					m_roll;			// x축 회전각
	FLOAT					m_pitch;		// y축 회전각
	FLOAT					m_yaw;			// z축 회전각

	FLOAT					m_delay;		// 움직임 딜레이 (0.0 ~ 1.0)

	shared_ptr<Player>		m_player;		// 플레이어
};

class ThirdPersonCamera : public Camera
{
public:
	ThirdPersonCamera();
	~ThirdPersonCamera() = default;

	virtual void Update(FLOAT timeElapsed);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);

	XMFLOAT3 GetOffset() const { return m_offset; }
	void SetOffset(const XMFLOAT3& offset) { m_offset = offset; }
	void SetDelay(FLOAT delay) { m_delay = delay; }

private:
	XMFLOAT3	m_offset;
	FLOAT		m_delay;
};