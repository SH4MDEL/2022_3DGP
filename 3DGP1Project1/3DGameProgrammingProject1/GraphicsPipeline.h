#pragma once

#include "GameObject.h"
#include "Camera.h"

class CGraphicsPipeline
{
private:
	static XMFLOAT4X4*		m_pxmf4x4World;
	static XMFLOAT4X4*		m_pxmf4x4ViewProject;
	static CViewport*		m_pViewport;

public:
	static void SetWorldTransform(XMFLOAT4X4* pxmf4x4World) { m_pxmf4x4World = pxmf4x4World; }
	static void SetViewPerspectiveProjectTransform(XMFLOAT4X4* pxmf4x4ViewPerspectiveProject);
	static void SetViewOrthographicProjectTransform(XMFLOAT4X4* pxmf4x4OrthographicProject);
	static void SetViewport(CViewport* pViewport) { m_pViewport = pViewport; }

	static XMFLOAT3 ScreenTransform(XMFLOAT3& xmf3Project);
	static XMFLOAT3 Project(XMFLOAT3& xmf3Model);
	static XMFLOAT3 Transform(XMFLOAT3& xmf3Model);
};
