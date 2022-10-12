#include "mesh.h"

Mesh::Mesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const vector<Vertex>& vertices, const vector<UINT>& indices)
{
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// ���� ���� ����
	m_nVertices = (UINT)vertices.size();
	const UINT vertexBufferSize = (UINT)sizeof(Vertex) * (UINT)vertices.size();

	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		NULL,
		IID_PPV_ARGS(&m_vertexBuffer)));

	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&m_vertexUploadBuffer)));

	// DEFAULT ���ۿ� UPLOAD ������ ������ ����
	D3D12_SUBRESOURCE_DATA vertextData{};
	vertextData.pData = vertices.data();
	vertextData.RowPitch = vertexBufferSize;
	vertextData.SlicePitch = vertextData.RowPitch;
	UpdateSubresources<1>(commandList.Get(), m_vertexBuffer.Get(), m_vertexUploadBuffer.Get(), 0, 0, 1, &vertextData);

	// ���� ���� ���� ����
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// ���� ���� �� ����
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);

	//--------------------------------------------------------------------

	m_nIndices = (UINT)indices.size();
	if (m_nIndices) {
		const UINT indexBufferSize = (UINT)sizeof(UINT) * (UINT)indices.size();

		DX::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			NULL,
			IID_PPV_ARGS(&m_indexBuffer)));

		DX::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			NULL,
			IID_PPV_ARGS(&m_indexUploadBuffer)));

		D3D12_SUBRESOURCE_DATA indexData{};
		indexData.pData = indices.data();
		indexData.RowPitch = indexBufferSize;
		indexData.SlicePitch = indexData.RowPitch;
		UpdateSubresources<1>(commandList.Get(), m_indexBuffer.Get(), m_indexUploadBuffer.Get(), 0, 0, 1, &indexData);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		m_indexBufferView.SizeInBytes = indexBufferSize;
	}
}

void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& m_commandList) const
{
	m_commandList->IASetPrimitiveTopology(m_primitiveTopology);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	if (m_nIndices) {
		m_commandList->IASetIndexBuffer(&m_indexBufferView);
		m_commandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else {
		m_commandList->DrawInstanced(m_nVertices, 1, 0, 0);
	}
}

void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& m_commandList, const D3D12_VERTEX_BUFFER_VIEW& instanceBufferView) const
{
	m_commandList->IASetPrimitiveTopology(m_primitiveTopology);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetVertexBuffers(1, 1, &instanceBufferView);
	if (m_nIndices) {
		m_commandList->IASetIndexBuffer(&m_indexBufferView);
		m_commandList->DrawIndexedInstanced(m_nIndices, instanceBufferView.SizeInBytes / instanceBufferView.StrideInBytes, 0, 0, 0);
	}
	else {
		m_commandList->DrawInstanced(m_nVertices, instanceBufferView.SizeInBytes / instanceBufferView.StrideInBytes, 0, 0);
	}
}

void Mesh::ReleaseUploadBuffer()
{
	if (m_vertexUploadBuffer) m_vertexUploadBuffer.Reset();
	if (m_indexUploadBuffer) m_indexUploadBuffer.Reset();
}

void MeshFromFile::Render(const ComPtr<ID3D12GraphicsCommandList>& m_commandList) const
{
	m_commandList->IASetPrimitiveTopology(m_primitiveTopology);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	if ((m_nSubMeshes > 0))
	{
		for (int i = 0; i < m_nSubMeshes; ++i) {
			//m_textures[i]->UpdateShaderVariable(m_commandList);

			m_commandList->IASetIndexBuffer(&(m_subsetIndexBufferViews[i]));
			m_commandList->DrawIndexedInstanced(m_vSubsetIndices[i], 1, 0, 0, 0);
		}
	}
	else
	{
		m_commandList->DrawInstanced(m_nVertices, 1, 0, 0);
	}
}

void MeshFromFile::LoadMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, ifstream& in)
{
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	vector<NormalVertex> vertices;
	vector<UINT> indices;

	BYTE strLength;

	INT positionNum, colorNum, normalNum;

	while (1) {
		in.read((char*)(&strLength), sizeof(BYTE));
		string strToken(strLength, '\0');
		in.read((&strToken[0]), sizeof(char) * strLength);

		if (strToken == "<Bounds>:") {
			XMFLOAT3 dummy;
			in.read((char*)(&dummy), sizeof(XMFLOAT3));
			in.read((char*)(&dummy), sizeof(XMFLOAT3));
		}
		else if (strToken == "<Positions>:") {
			in.read((char*)(&positionNum), sizeof(INT));
			if (vertices.size() < positionNum) {
				m_nVertices = positionNum;
				vertices.resize(positionNum);
			}
			for (int i = 0; i < positionNum; ++i) {
				in.read((char*)(&vertices.at(i).position), sizeof(XMFLOAT3));
			}
		}
		else if (strToken == "<Colors>:") {
			in.read((char*)(&colorNum), sizeof(INT));
			if (vertices.size() < colorNum) {
				m_nVertices = colorNum;
				vertices.resize(colorNum);
			}
			for (int i = 0; i < colorNum; ++i) {
				in.read((char*)(&vertices.at(i).color), sizeof(XMFLOAT3));
			}
		}
		else if (strToken == "<Normals>:") {
			in.read((char*)(&normalNum), sizeof(INT));
			if (vertices.size() < normalNum) {
				m_nVertices = normalNum;
				vertices.resize(normalNum);
			}
			for (int i = 0; i < normalNum; ++i) {
				in.read((char*)(&vertices.at(i).normal), sizeof(XMFLOAT3));
			}
		}
		else if (strToken == "<Indices>:") {
			in.read((char*)(&m_nIndices), sizeof(INT));
			indices.reserve(m_nIndices);
			in.read((char*)(&indices), sizeof(UINT) * m_nIndices);
		}
		else if (strToken == "<SubMeshes>:") {
			in.read((char*)(&m_nSubMeshes), sizeof(INT));
			if (m_nSubMeshes > 0) {
				m_vSubsetIndices.reserve(m_nSubMeshes);
				m_vvSubsetIndices.reserve(m_nSubMeshes);
				m_subsetIndexBuffers.reserve(m_nSubMeshes);
				m_subsetIndexUploadBuffers.reserve(m_nSubMeshes);
				m_subsetIndexBufferViews.reserve(m_nSubMeshes);

				for (int i = 0; i < m_nSubMeshes; ++i) {
					in.read((char*)(&strLength), sizeof(BYTE));
					strToken.resize(64, '\0');
					in.read((&strToken[0]), sizeof(char) * strLength);

					if (strToken == "<SubMesh>:") {
						int index;
						in.read((char*)(&index), sizeof(INT));
						in.read((char*)(&m_vSubsetIndices[i]), sizeof(INT));
						if (m_vSubsetIndices[i] > 0) {
							m_vvSubsetIndices[i].reserve(m_vSubsetIndices[i]);
							in.read((char*)(&m_vvSubsetIndices[i]), sizeof(INT) * m_vSubsetIndices[i]);

							m_subsetIndexBuffers[i] = CreateBufferResource(device, commandList, m_vvSubsetIndices[i].data(),
								sizeof(UINT) * m_vSubsetIndices[i], D3D12_HEAP_TYPE_DEFAULT,
								D3D12_RESOURCE_STATE_INDEX_BUFFER, m_subsetIndexUploadBuffers[i]);

							m_subsetIndexBufferViews[i].BufferLocation = m_subsetIndexBuffers[i]->GetGPUVirtualAddress();
							m_subsetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_subsetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_vSubsetIndices[i];
						}
					}
				}
			}
		}
		else if (strToken == "</Mesh>:") {
			break;
		}
	}

	m_nVertices = (UINT)vertices.size();
	m_vertexBuffer = CreateBufferResource(device, commandList, vertices.data(),
		sizeof(NormalVertex) * vertices.size(), D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_vertexUploadBuffer);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(NormalVertex);
	m_vertexBufferView.SizeInBytes = sizeof(NormalVertex) * vertices.size();

	m_nIndices = indices.size();
	m_indexBuffer = CreateBufferResource(device, commandList, indices.data(),
		sizeof(UINT) * indices.size(), D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_indexUploadBuffer);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = sizeof(UINT) * indices.size();
}

HeightMapImage::HeightMapImage(const wstring& fileName, INT width, INT length, XMFLOAT3 scale) :
	m_width(width), m_length(length), m_scale(scale), m_pixels{ new BYTE[width * length] }
{
	unique_ptr<BYTE[]> pixels{ new BYTE[m_width * m_length] };
	HANDLE hFile{ CreateFile(fileName.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr) };
	DWORD bytesRead;
	ReadFile(hFile, pixels.get(), m_width * m_length, &bytesRead, NULL);
	CloseHandle(hFile);

	for (int y = 0; y < m_length; ++y) {
		for (int x = 0; x < m_width; ++x) {
			m_pixels[x + (y * m_width)] = pixels[x + ((m_length - y - 1) * m_width)];
		}
	}
}

FLOAT HeightMapImage::GetHeight(FLOAT x, FLOAT z) const
{
	//������ ��ǥ(x, z)�� �̹��� ��ǥ���̴�.
	//���� ���� x-��ǥ�� z-��ǥ�� ���� ���� ������ ����� ������ ���̴� 0�̴�.
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_width) || (z >= m_length)) return 0.0f;

	//���� ���� ��ǥ�� ���� �κа� �Ҽ� �κ��� ����Ѵ�. 
	int mx{ (int)x };
	int mz{ (int)z };
	float px{ x - mx };
	float pz{ z - mz };

	float bottomLeft{ (float)m_pixels[mx + mz * m_width] };
	float bottomRight{ (float)m_pixels[(mx + 1) + (mz * m_width)] };
	float topLeft{ (float)m_pixels[mx + (mz + 1) * m_width] };
	float topRight{ (float)m_pixels[(mx + 1) + ((mz + 1) * m_width)] };

	//�簢���� �� ���� �����Ͽ� ����(�ȼ� ��)�� ����Ѵ�. 
	float fTopHeight{ topLeft * (1 - px) + topRight * px };
	float fBottomHeight{ bottomLeft * (1 - px) + bottomRight * px };
	return fBottomHeight * (1 - pz) + fTopHeight * pz;
}

XMFLOAT3 HeightMapImage::GetNormal(INT x, INT z) const
{
	//x-��ǥ�� z-��ǥ�� ���� ���� ������ ����� ������ ���� ���ʹ� y-�� ���� �����̴�. 
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_width) || (z >= m_length))
		return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	/*���� �ʿ��� (x, z) ��ǥ�� �ȼ� ���� ������ �� ���� �� (x+1, z), (z, z+1)�� ���� �ȼ� ���� ����Ͽ� ���� ���͸�
	����Ѵ�.*/
	int heightMapIndex { x + z * m_width };
	int xAdd { x < m_width - 1 ? 1 : -1 };
	int zAdd { z < m_length - 1 ? m_width : -m_width };

	//(x, z), (x+1, z), (z, z+1)�� �ȼ����� ������ ���̸� ���Ѵ�. 
	float y1 { (float)m_pixels[heightMapIndex] * m_scale.y };
	float y2 { (float)m_pixels[heightMapIndex + xAdd] * m_scale.y };
	float y3 { (float)m_pixels[heightMapIndex + zAdd] * m_scale.y };

	//xmf3Edge1�� (0, y3, m_xmf3Scale.z) - (0, y1, 0) �����̴�. 
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_scale.z);
	//xmf3Edge2�� (m_xmf3Scale.x, y2, 0) - (0, y1, 0) �����̴�. 
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_scale.x, y2 - y1, 0.0f);
	//���� ���ʹ� xmf3Edge1�� xmf3Edge2�� ������ ����ȭ�ϸ� �ȴ�. 
	return Vector3::Cross(xmf3Edge1, xmf3Edge2);
}

HeightMapGridMesh::HeightMapGridMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
	INT xStart, INT zStart, INT width, INT length, XMFLOAT3 scale, HeightMapImage* heightMapImage)
{
	//���ڴ� �ﰢ�� ��Ʈ������ �����Ѵ�. 
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	m_width = width;
	m_length = length;
	m_scale = scale;

	vector<TerrainVertex> vertices;

	/*xStart�� zStart�� ������ ���� ��ġ(x-��ǥ�� z-��ǥ)�� ��Ÿ����. Ŀ�ٶ� ������ ���ڵ��� ������ �迭�� ���� ��
	�䰡 �ֱ� ������ ��ü �������� �� ������ ���� ��ġ�� ��Ÿ���� ������ �ʿ��ϴ�.*/
	for (int i = 0, z = zStart; z < (zStart + length); ++z) {
		for (int x = xStart; x < (xStart + width); ++x, ++i)
		{
			//������ ���̿� ������ ���� �����κ��� ���Ѵ�.
			XMFLOAT3 anormal = Vector3::Add(heightMapImage->GetNormal(x, z), heightMapImage->GetNormal(x + 1, z));
			anormal = Vector3::Add(heightMapImage->GetNormal(x + 1, z + 1), heightMapImage->GetNormal(x, z + 1));
			Vector3::Normalize(anormal);

			vertices.emplace_back(XMFLOAT3((x * m_scale.x), heightMapImage->GetHeight((FLOAT)x, (FLOAT)z), (z * m_scale.z)),
				XMFLOAT2((FLOAT)x / (FLOAT)heightMapImage->GetWidth(), 1.0f - ((FLOAT)z / (FLOAT)heightMapImage->GetLength())),
				XMFLOAT2(FLOAT(x) / FLOAT(m_scale.x * 0.5f), FLOAT(z) / FLOAT(m_scale.z * 0.5f)));
		}
	}

	m_nVertices = (UINT)vertices.size();
	m_vertexBuffer = CreateBufferResource(device, commandList, vertices.data(),
		sizeof(TerrainVertex) * vertices.size(), D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_vertexUploadBuffer);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(TerrainVertex);
	m_vertexBufferView.SizeInBytes = sizeof(TerrainVertex) * vertices.size();


	vector<UINT> indices;

	for (int j = 0, z = 0; z < length - 1; ++z)
	{
		if ((z % 2) == 0)
		{
			//Ȧ�� ��° ���̹Ƿ�(z = 0, 2, 4, ...) �ε����� ���� ������ ���ʿ��� ������ �����̴�. 
			for (int x = 0; x < width; x++)
			{
				//ù ��° ���� �����ϰ� ���� �ٲ� ������(x == 0) ù ��° �ε����� �߰��Ѵ�. 
				if (x == 0 && z > 0) indices.push_back(x + (z * width));
				//�Ʒ�(x, z), ��(x, z+1)�� ������ �ε����� �߰��Ѵ�. 
				indices.push_back(x + (z * width));
				indices.push_back(x + (z * width) + width);
			}
		}
		else
		{
			//¦�� ��° ���̹Ƿ�(z = 1, 3, 5, ...) �ε����� ���� ������ �����ʿ��� ���� �����̴�. 
			for (int x = width - 1; x >= 0; --x)
			{
				//���� �ٲ� ������(x == (nWidth-1)) ù ��° �ε����� �߰��Ѵ�. 
				if (x == width - 1) indices.push_back(x + (z * width));
				//�Ʒ�(x, z), ��(x, z+1)�� ������ �ε����� �߰��Ѵ�. 
				indices.push_back(x + (z * width));
				indices.push_back(x + (z * width) + width);
			}
		}
	}

	m_nIndices = indices.size();
	m_indexBuffer = CreateBufferResource(device, commandList, indices.data(),
		sizeof(UINT) * indices.size(), D3D12_HEAP_TYPE_DEFAULT, 
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_indexUploadBuffer);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = sizeof(UINT) * indices.size();
}

TextureRectMesh::TextureRectMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, XMFLOAT3 position, FLOAT width, FLOAT height, FLOAT depth)
{
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_nIndices = 0;
	m_nVertices = 6;

	vector<TextureVertex> vertices;
	FLOAT fx = (width * 0.5f) + position.x, fy = (height * 0.5f) + position.y, fz = (depth * 0.5f) + position.z;

	if (width == 0.0f)
	{
		if (position.x > 0.0f)
		{
			vertices.emplace_back(XMFLOAT3(fx, +fy, -fz), XMFLOAT2(1.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(fx, -fy, -fz), XMFLOAT2(1.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(fx, -fy, +fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(fx, -fy, +fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(fx, +fy, +fz), XMFLOAT2(0.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(fx, +fy, -fz), XMFLOAT2(1.0f, 0.0f));
		}
		else
		{
			vertices.emplace_back(XMFLOAT3(fx, +fy, +fz), XMFLOAT2(1.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(fx, -fy, +fz), XMFLOAT2(1.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(fx, -fy, -fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(fx, -fy, -fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(fx, +fy, -fz), XMFLOAT2(0.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(fx, +fy, +fz), XMFLOAT2(1.0f, 0.0f));
		}
	}
	else if (height == 0.0f)
	{
		if (position.y > 0.0f)
		{
			vertices.emplace_back(XMFLOAT3(+fx, fy, -fz), XMFLOAT2(1.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(+fx, fy, +fz), XMFLOAT2(1.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(-fx, fy, +fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(-fx, fy, +fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(-fx, fy, -fz), XMFLOAT2(0.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(+fx, fy, -fz), XMFLOAT2(1.0f, 0.0f));
		}
		else
		{
			vertices.emplace_back(XMFLOAT3(+fx, fy, +fz), XMFLOAT2(1.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(+fx, fy, -fz), XMFLOAT2(1.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(-fx, fy, -fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(-fx, fy, -fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(-fx, fy, +fz), XMFLOAT2(0.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(+fx, fy, +fz), XMFLOAT2(1.0f, 0.0f));
		}
	}
	else if (depth == 0.0f)
	{
		if (position.z > 0.0f)
		{
			vertices.emplace_back(XMFLOAT3(+fx, +fy, fz), XMFLOAT2(1.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(+fx, -fy, fz), XMFLOAT2(1.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(-fx, -fy, fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(-fx, -fy, fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(-fx, +fy, fz), XMFLOAT2(0.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(+fx, +fy, fz), XMFLOAT2(1.0f, 0.0f));
		}
		else
		{
			vertices.emplace_back(XMFLOAT3(-fx, +fy, fz), XMFLOAT2(1.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(-fx, -fy, fz), XMFLOAT2(1.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(+fx, -fy, fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(+fx, -fy, fz), XMFLOAT2(0.0f, 1.0f));
			vertices.emplace_back(XMFLOAT3(+fx, +fy, fz), XMFLOAT2(0.0f, 0.0f));
			vertices.emplace_back(XMFLOAT3(-fx, +fy, fz), XMFLOAT2(1.0f, 0.0f));
		}
	}
	
	m_nVertices = vertices.size();
	m_vertexBuffer = CreateBufferResource(device, commandList, vertices.data(),
		sizeof(TextureVertex) * vertices.size(), D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_vertexUploadBuffer);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(TextureVertex);
	m_vertexBufferView.SizeInBytes = sizeof(TextureVertex) * vertices.size();
}

SkyboxMesh::SkyboxMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, FLOAT width, FLOAT height, FLOAT depth)
{
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_nIndices = 0;
	m_nVertices = 36;

	vector<SkyboxVertex> vertices;
	FLOAT fx = width * 0.5f, fy = height * 0.5f, fz = depth * 0.5f;

	vertices.emplace_back(XMFLOAT3(-fx, +fx, +fx));
	vertices.emplace_back(XMFLOAT3(+fx, +fx, +fx));
	vertices.emplace_back(XMFLOAT3(-fx, -fx, +fx));
	vertices.emplace_back(XMFLOAT3(-fx, -fx, +fx));
	vertices.emplace_back(XMFLOAT3(+fx, +fx, +fx));
	vertices.emplace_back(XMFLOAT3(+fx, -fx, +fx));
		   
	vertices.emplace_back(XMFLOAT3(+fx, +fx, -fx));
	vertices.emplace_back(XMFLOAT3(-fx, +fx, -fx));
	vertices.emplace_back(XMFLOAT3(+fx, -fx, -fx));
	vertices.emplace_back(XMFLOAT3(+fx, -fx, -fx));
	vertices.emplace_back(XMFLOAT3(-fx, +fx, -fx));
	vertices.emplace_back(XMFLOAT3(-fx, -fx, -fx));
		   
	vertices.emplace_back(XMFLOAT3(-fx, +fx, -fx));
	vertices.emplace_back(XMFLOAT3(-fx, +fx, +fx));
	vertices.emplace_back(XMFLOAT3(-fx, -fx, -fx));
	vertices.emplace_back(XMFLOAT3(-fx, -fx, -fx));
	vertices.emplace_back(XMFLOAT3(-fx, +fx, +fx));
	vertices.emplace_back(XMFLOAT3(-fx, -fx, +fx));
		   
	vertices.emplace_back(XMFLOAT3(+fx, +fx, +fx));
	vertices.emplace_back(XMFLOAT3(+fx, +fx, -fx));
	vertices.emplace_back(XMFLOAT3(+fx, -fx, +fx));
	vertices.emplace_back(XMFLOAT3(+fx, -fx, +fx));
	vertices.emplace_back(XMFLOAT3(+fx, +fx, -fx));
	vertices.emplace_back(XMFLOAT3(+fx, -fx, -fx));
		   
	vertices.emplace_back(XMFLOAT3(-fx, +fx, -fx));
	vertices.emplace_back(XMFLOAT3(+fx, +fx, -fx));
	vertices.emplace_back(XMFLOAT3(-fx, +fx, +fx));
	vertices.emplace_back(XMFLOAT3(-fx, +fx, +fx));
	vertices.emplace_back(XMFLOAT3(+fx, +fx, -fx));
	vertices.emplace_back(XMFLOAT3(+fx, +fx, +fx));
		   
	vertices.emplace_back(XMFLOAT3(-fx, -fx, +fx));
	vertices.emplace_back(XMFLOAT3(+fx, -fx, +fx));
	vertices.emplace_back(XMFLOAT3(-fx, -fx, -fx));
	vertices.emplace_back(XMFLOAT3(-fx, -fx, -fx));
	vertices.emplace_back(XMFLOAT3(+fx, -fx, +fx));
	vertices.emplace_back(XMFLOAT3(+fx, -fx, -fx));

	m_nVertices = vertices.size();
	m_vertexBuffer = CreateBufferResource(device, commandList, vertices.data(),
		sizeof(SkyboxVertex) * vertices.size(), D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_vertexUploadBuffer);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(SkyboxVertex);
	m_vertexBufferView.SizeInBytes = sizeof(SkyboxVertex) * vertices.size();
}