#include "camera.h"

Camera::Camera() :
	m_eye{ 0.0f, 0.0f, 0.0f }, m_look{ 0.0f, 0.0f, 1.0f }, m_up{ 0.0f, 1.0f, 0.0f },
	m_u{ 1.0f, 0.0f, 0.0f }, m_v{ 0.0f, 1.0f, 0.0f }, m_n{ 0.0f, 0.0f, 1.0f },
	m_roll{ 0.0f }, m_pitch{ 0.0f }, m_yaw{ 0.0f }, m_delay{ 0.0f }
{
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_projMatrix, XMMatrixIdentity());
}

void Camera::CreateShaderVariable(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(CameraInfo)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_cameraBuffer)));

	// 카메라 버퍼 포인터
	m_cameraBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_cameraBufferPointer));
}

void Camera::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&Vector3::Add(m_eye, m_look)), XMLoadFloat3(&m_up)));

	XMFLOAT4X4 viewMatrix;
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_viewMatrix)));
	::memcpy(&m_cameraBufferPointer->viewMatrix, &viewMatrix, sizeof(XMFLOAT4X4));

	XMFLOAT4X4 projMatrix;
	XMStoreFloat4x4(&projMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_projMatrix)));
	::memcpy(&m_cameraBufferPointer->projMatrix, &projMatrix, sizeof(XMFLOAT4X4));

	XMFLOAT3 eye = m_eye;
	::memcpy(&m_cameraBufferPointer->cameraPosition, &eye, sizeof(XMFLOAT3));

	D3D12_GPU_VIRTUAL_ADDRESS virtualAddress = m_cameraBuffer->GetGPUVirtualAddress();
	commandList->SetGraphicsRootConstantBufferView(1, virtualAddress);
}

void Camera::UpdateLocalAxis()
{
	// 로컬 z축
	m_n = Vector3::Normalize(m_look);

	// 로컬 x축
	m_u = Vector3::Normalize(Vector3::Cross(m_up, m_n));

	// 로컬 y축
	m_v = Vector3::Cross(m_n, m_u);
}

void Camera::Move(const XMFLOAT3& shift)
{
	m_eye = Vector3::Add(m_eye, shift);
}

void Camera::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	//if (roll != 0.0f)
	//{
	//	XMMATRIX rotate{ XMMatrixIdentity() };
	//	if (m_roll + roll > MAX_ROLL)
	//	{
	//		rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_u), XMConvertToRadians(MAX_ROLL - m_roll));
	//		m_roll = MAX_ROLL;
	//	}
	//	else if (m_roll + roll < MIN_ROLL)
	//	{
	//		rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_u), XMConvertToRadians(MIN_ROLL - m_roll));
	//		m_roll = MIN_ROLL;
	//	}
	//	else
	//	{
	//		rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_u), XMConvertToRadians(roll));
	//		m_roll += roll;
	//	}
	//	XMStoreFloat3(&m_look, XMVector3TransformNormal(XMLoadFloat3(&m_look), rotate));
	//}
	//if (pitch != 0.0f)
	//{
	//	m_pitch += pitch;

	//	XMMATRIX rotate{ XMMatrixRotationAxis(XMLoadFloat3(&m_v), XMConvertToRadians(pitch)) };
	//	XMStoreFloat3(&m_look, XMVector3TransformNormal(XMLoadFloat3(&m_look), rotate));
	//}
	//if (yaw != 0.0f)
	//{

	//}
	//UpdateLocalAxis();
}

void Camera::SetPlayer(const shared_ptr<Player>& player)
{
	if (m_player) m_player.reset();
	m_player = player;
	SetEye(m_player->GetPosition());
}


ThirdPersonCamera::ThirdPersonCamera() : Camera{}, m_offset{ 0.0f, 1.0f, -5.0f }, m_delay{ 0.1f }
{

}

void ThirdPersonCamera::Update(FLOAT timeElapsed)
{
	XMFLOAT3 destination{ Vector3::Add(m_player->GetPosition(), m_offset) };
	XMFLOAT3 direction{ Vector3::Sub(destination, GetEye()) };
	XMFLOAT3 shift{ Vector3::Mul(direction, timeElapsed * 1 / m_delay) };
	SetEye(Vector3::Add(GetEye(), shift));
}

void ThirdPersonCamera::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	XMMATRIX rotate{ XMMatrixIdentity() };

	if (roll != 0.0f)
	{
		m_roll += roll;
		rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_player->GetRight()), XMConvertToRadians(roll));
		XMStoreFloat3(&m_offset, XMVector3TransformNormal(XMLoadFloat3(&m_offset), rotate));
	}
	if (pitch != 0.0f)
	{
		m_pitch += pitch;
		rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_player->GetUp()), XMConvertToRadians(pitch));
		XMStoreFloat3(&m_offset, XMVector3TransformNormal(XMLoadFloat3(&m_offset), rotate));
	}
	if (yaw != 0.0f)
	{
		m_yaw += yaw;
		rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_player->GetFront()), XMConvertToRadians(yaw));
		XMStoreFloat3(&m_offset, XMVector3TransformNormal(XMLoadFloat3(&m_offset), rotate));
	}

	XMFLOAT3 look{ Vector3::Sub(m_player->GetPosition(), m_eye) };
	if (Vector3::Length(look)) m_look = look;
}

FirstPersonCamera::FirstPersonCamera() : Camera{}, m_offset{ 0.0f, 1.0f, 0.65f }, m_delay{ 0.01f }
{

}

void FirstPersonCamera::Update(FLOAT timeElapsed)
{
	XMFLOAT3 destination{ Vector3::Add(m_player->GetPosition(), m_offset) };
	XMFLOAT3 direction{ Vector3::Sub(destination, GetEye()) };
	XMFLOAT3 shift{ Vector3::Mul(direction, timeElapsed * 1 / m_delay) };
	SetEye(Vector3::Add(GetEye(), shift));
}

void FirstPersonCamera::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	XMMATRIX rotate{ XMMatrixIdentity() };

	if (roll != 0.0f)
	{
		m_roll += roll;
		rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_player->GetRight()), XMConvertToRadians(roll));
		XMStoreFloat3(&m_offset, XMVector3TransformNormal(XMLoadFloat3(&m_offset), rotate));
	}
	if (pitch != 0.0f)
	{
		m_pitch += pitch;
		rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_player->GetUp()), XMConvertToRadians(pitch));
		XMStoreFloat3(&m_offset, XMVector3TransformNormal(XMLoadFloat3(&m_offset), rotate));
	}
	if (yaw != 0.0f)
	{
		m_yaw += yaw;
		rotate = XMMatrixRotationAxis(XMLoadFloat3(&m_player->GetFront()), XMConvertToRadians(yaw));
		XMStoreFloat3(&m_offset, XMVector3TransformNormal(XMLoadFloat3(&m_offset), rotate));
	}

	XMFLOAT3 look{ Vector3::Sub(m_player->GetPosition(), m_eye) };
	look.x = -look.x;
	look.z = -look.z;
	if (Vector3::Length(look)) m_look = look;
}