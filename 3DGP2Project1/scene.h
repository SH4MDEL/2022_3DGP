#pragma once
#include "stdafx.h"
#include "shader.h"
#include "object.h"
#include "player.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"

class Scene
{
public:
	Scene() = default;
	~Scene();

	void BuildObjects(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandlist, const ComPtr<ID3D12RootSignature>& rootsignature, FLOAT	aspectRatio);
	void OnProcessingMouseMessage(HWND hWnd, UINT width, UINT height, FLOAT deltaTime) const;
	void OnProcessingMouseMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) const;
	void OnProcessingKeyboardMessage(FLOAT timeElapsed) const;
	void Update(FLOAT timeElapsed);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	unique_ptr<Shader>& GetShader(const string& key) { return m_shader[key]; }
	unordered_map<string, unique_ptr<Shader>>& GetShaders() { return m_shader; }
	shared_ptr<Player> GetPlayer() const { return m_player; }
	shared_ptr<Camera> GetCamera() const { return m_camera; }

	void CheckPlayerByObjectCollisions();
	void CheckMissileByObjectCollisions();
	void CheckTerrainBorderLimit();

private:
	unordered_map<string, unique_ptr<Shader>>	m_shader;
	unordered_map<string, unique_ptr<Shader>>	m_blending;
	shared_ptr<Player>							m_player;
	shared_ptr<Camera>							m_camera;
};