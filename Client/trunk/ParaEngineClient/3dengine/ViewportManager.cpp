//-----------------------------------------------------------------------------
// Class:	CViewportManager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8.8
// Desc: managing all view ports
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneState.h"
#include "ViewportManager.h"

using namespace ParaEngine;

CViewportManager::CViewportManager()
	:m_nWidth(1), m_nHeight(1), m_nActiveViewPortIndex(1), m_nLayout(VIEW_LAYOUT_INVALID)
{
	m_viewport.X = 0;
	m_viewport.Y = 0;
	m_viewport.Width = 960;
	m_viewport.Height = 560;
	m_viewport.MinZ = 0.5;
	m_viewport.MaxZ = 200;
}

CViewportManager::~CViewportManager(void)
{
	Cleanup();
}

void ParaEngine::CViewportManager::UpdateViewport(int nBackbufferWidth, int nBackbufferHeight)
{
	if (m_nHeight != nBackbufferHeight || m_nWidth != nBackbufferWidth)
	{
		m_nWidth = nBackbufferWidth;
		m_nHeight = nBackbufferHeight;
		m_viewport.Width = m_nWidth;
		m_viewport.Height = m_nHeight;
		UpdateLayout();
		
		for (auto iter = m_viewportList.begin(); iter != m_viewportList.end(); iter++)
		{
			CViewport* pViewPort = *iter;
			if (pViewPort)
			{
				pViewPort->OnParentSizeChanged(m_nWidth, m_nHeight);
			}
		}
	}
}

CViewport* ParaEngine::CViewportManager::CreateGetViewPort(int nIndex/*=0*/)
{
	CViewport* pViewPort = NULL;
	if (nIndex >= 0 && nIndex < (int)m_viewportList.size())
	{
		pViewPort = m_viewportList[nIndex];
	}
	else if (nIndex < 8)
	{
		m_viewportList.resize(nIndex + 1, NULL);
	}
	else
	{
		OUTPUT_LOG("error: view port index is too big\n");
		return NULL;
	}

	if (!pViewPort)
	{
		pViewPort = new CViewport(this);
		m_viewportList[nIndex] = pViewPort;
	}
	return pViewPort;
}

void ParaEngine::CViewportManager::DeleteViewPort(int nIndex/*=0*/)
{
	if (nIndex >= 0 && nIndex < (int)m_viewportList.size())
	{
		SAFE_DELETE(m_viewportList[nIndex]);
	}
}

void ParaEngine::CViewportManager::Cleanup()
{
	for (auto iter = m_viewportList.begin(); iter != m_viewportList.end(); iter++)
	{
		CViewport* pViewPort = *iter;
		SAFE_DELETE(pViewPort);
	}
	m_viewportList.clear();
}

void ParaEngine::CViewportManager::SortViewport()
{
	m_viewportSorted = m_viewportList;
	std::stable_sort(m_viewportSorted.begin(), m_viewportSorted.end(), CViewport::ZOrderLessCompare());
}

HRESULT ParaEngine::CViewportManager::Render(double dTimeDelta, int nPipelineOrder)
{
	SortViewport();
	for (auto iter = m_viewportSorted.begin(); iter != m_viewportSorted.end(); iter++)
	{
		CViewport* pViewPort = *iter;
		if (pViewPort)
		{
			pViewPort->Render(dTimeDelta, nPipelineOrder);
		}
	}
	CreateGetViewPort(1)->SetActive();
	return S_OK;
}

void ParaEngine::CViewportManager::SetActiveViewPortIndex(int val)
{
	m_nActiveViewPortIndex = val;
}

CViewport* ParaEngine::CViewportManager::GetActiveViewPort()
{
	return CreateGetViewPort(m_nActiveViewPortIndex);
}

void ParaEngine::CViewportManager::GetPointOnViewport(int& x, int& y, int* pWidth, int* pHeight)
{
	CViewport* pViewPort = GetActiveViewPort();
	if (pViewPort)
	{
		pViewPort->GetPointOnViewport(x, y, pWidth, pHeight);
	}
}

CViewport* ParaEngine::CViewportManager::GetViewportByPoint(int x, int y)
{
	for (auto iter = m_viewportList.begin(); iter != m_viewportList.end(); iter++)
	{
		CViewport* pViewPort = *iter;
		if (pViewPort)
		{
			if (pViewPort->IsPointOnViewport(x, y))
				return pViewPort;
		}
	}
	return NULL;
}

int ParaEngine::CViewportManager::GetWidth() const
{
	return m_nWidth;
}

int ParaEngine::CViewportManager::GetHeight() const
{
	return m_nHeight;
}

int ParaEngine::CViewportManager::GetViewportCount()
{
	for (size_t i = 0; i < m_viewportList.size(); ++i)
	{
		if (m_viewportList[i] == 0)
			return i;
	}
	return (int)m_viewportList.size();
}

void ParaEngine::CViewportManager::SetViewportCount(int nCount)
{
	if ((int)m_viewportList.size() >= nCount)
	{
		for (size_t i = nCount; i < m_viewportList.size(); ++i)
		{
			DeleteViewPort(i);
		}
	}
}


void ParaEngine::CViewportManager::UpdateLayout()
{
	if (GetLayout() == VIEW_LAYOUT_STEREO_LEFT_RIGHT)
	{
		int nHalfWidth = (int)(GetWidth() / 2);
		CViewport* pUIViewportLeft = CreateGetViewPort(0);
		pUIViewportLeft->SetPosition("_ml", 0, 0, nHalfWidth, 0);
		CViewport* pMainSceneViewportLeft = CreateGetViewPort(1);
		pMainSceneViewportLeft->SetPosition("_ml", 0, 0, nHalfWidth, 0);

		CViewport* pUIViewportRight = CreateGetViewPort(2);
		pUIViewportRight->SetPosition("_mr", 0, 0, nHalfWidth, 0);
		CViewport* pMainSceneViewportRight = CreateGetViewPort(3);
		pMainSceneViewportRight->SetPosition("_mr", 0, 0, nHalfWidth, 0);
	}
}

ParaEngine::VIEWPORT_LAYOUT ParaEngine::CViewportManager::GetLayout()
{
	return m_nLayout;
}

void ParaEngine::CViewportManager::SetLayout(VIEWPORT_LAYOUT nLayout, CSceneObject* pMainScene, CGUIRoot* pGUIRoot)
{
	if (!pMainScene)
		pMainScene = CGlobals::GetScene();
	if (!pGUIRoot)
		pGUIRoot = CGlobals::GetGUI();
	if (m_nLayout == nLayout && GetViewportCount() != 0)
		return;
	// clear layout
	SetViewportCount(0);

	m_nLayout = nLayout;
	if (nLayout == VIEW_LAYOUT_STEREO_LEFT_RIGHT)
	{
		int nHalfWidth = (int)(GetWidth() / 2);
		CViewport* pUIViewportLeft = CreateGetViewPort(0);
		pUIViewportLeft->SetIdentifier("left_GUI");
		pUIViewportLeft->SetGUIRoot(pGUIRoot);
		pUIViewportLeft->SetPosition("_ml", 0, 0, nHalfWidth, 0);
		pUIViewportLeft->SetZOrder(100);
		pUIViewportLeft->SetEyeMode(STEREO_EYE_LEFT);
		CViewport* pMainSceneViewportLeft = CreateGetViewPort(1);
		pMainSceneViewportLeft->SetIdentifier("left_scene");
		pMainSceneViewportLeft->SetScene(pMainScene);
		pMainSceneViewportLeft->SetPosition("_ml", 0, 0, nHalfWidth, 0);
		pMainSceneViewportLeft->SetEyeMode(STEREO_EYE_LEFT);

		CViewport* pUIViewportRight = CreateGetViewPort(2);
		pUIViewportRight->SetIdentifier("right_GUI");
		pUIViewportRight->SetGUIRoot(pGUIRoot);
		pUIViewportRight->SetPosition("_mr", 0, 0, nHalfWidth, 0);
		pUIViewportRight->SetZOrder(101);
		pUIViewportRight->SetEyeMode(STEREO_EYE_RIGHT);
		CViewport* pMainSceneViewportRight = CreateGetViewPort(3);
		pMainSceneViewportRight->SetIdentifier("right_scene");
		pMainSceneViewportRight->SetScene(pMainScene);
		pMainSceneViewportRight->SetPosition("_mr", 0, 0, nHalfWidth, 0);
		pMainSceneViewportRight->SetZOrder(1);
		pMainSceneViewportRight->SetEyeMode(STEREO_EYE_RIGHT);

		SetViewportCount(4);
	}
	else if (nLayout == VIEW_LAYOUT_STEREO_RED_BLUE)
	{
		CViewport* pUIViewport = CreateGetViewPort(0);
		pUIViewport->SetIdentifier("GUI");
		pUIViewport->SetGUIRoot(pGUIRoot);
		pUIViewport->SetPosition("_fi", 0, 0, 0, 0);
		pUIViewport->SetZOrder(100);
		pUIViewport->SetEyeMode(STEREO_EYE_NORMAL);
		CViewport* pMainSceneViewportLeft = CreateGetViewPort(1);
		pMainSceneViewportLeft->SetIdentifier("left_scene");
		pMainSceneViewportLeft->SetScene(pMainScene);
		pMainSceneViewportLeft->SetPosition("_fi", 0, 0, 0, 0);
		pMainSceneViewportLeft->SetEyeMode(STEREO_EYE_LEFT);
		pMainSceneViewportLeft->SetRenderTargetName("_LeftViewRT"); // use private render target

		CViewport* pMainSceneViewportRight = CreateGetViewPort(2);
		pMainSceneViewportRight->SetIdentifier("right_scene");
		pMainSceneViewportRight->SetScene(pMainScene);
		pMainSceneViewportRight->SetPosition("_fi", 0, 0, 0, 0);
		pMainSceneViewportRight->SetZOrder(1);
		pMainSceneViewportRight->SetEyeMode(STEREO_EYE_RIGHT);

		// final full screen quad
		CViewport* pFinalViewPort = CreateGetViewPort(3);
		pFinalViewPort->SetIdentifier("final_composite");
		pFinalViewPort->SetPosition("_fi", 0, 0, 0, 0);
		pFinalViewPort->SetEyeMode(STEREO_EYE_NORMAL);
		pFinalViewPort->SetZOrder(99); // before GUI
		pFinalViewPort->SetPipelineOrder(PIPELINE_3D_SCENE);
		SetViewportCount(4);
	}
	else // if (nLayout == VIEW_LAYOUT_DEFAULT)
	{
		CViewport* pUIViewport = CreateGetViewPort(0);
		pUIViewport->SetIdentifier("GUI");
		pUIViewport->SetGUIRoot(pGUIRoot);
		pUIViewport->SetPosition("_fi", 0, 0, 0, 0);
		pUIViewport->SetZOrder(100);
		pUIViewport->SetEyeMode(STEREO_EYE_NORMAL);
		CViewport* pMainSceneViewport = CreateGetViewPort(1);
		pMainSceneViewport->SetIdentifier("scene");
		pMainSceneViewport->SetScene(pMainScene);
		pMainSceneViewport->SetPosition("_fi", 0, 0, 0, 0);
		pMainSceneViewport->SetEyeMode(STEREO_EYE_NORMAL);
		SetViewportCount(2);
	}
}

void ParaEngine::CViewportManager::SetActiveViewPort(CViewport* pViewport)
{
	for (int i=0; i<(int)m_viewportList.size(); i++)
	{
		if (m_viewportList[i] == pViewport)
		{
			SetActiveViewPortIndex(i);
			return;
		}
	}
}

void ParaEngine::CViewportManager::ApplyViewport()
{
	ParaViewport CurrentViewport;
	CGlobals::GetRenderDevice()->GetViewport(reinterpret_cast<D3DVIEWPORT9*>(&CurrentViewport));
	ParaViewport myViewport = CurrentViewport;
	myViewport.X = 0;
	myViewport.Y = 0;
	myViewport.Width = GetWidth();
	myViewport.Height = GetHeight();
	CGlobals::GetRenderDevice()->SetViewport(reinterpret_cast<D3DVIEWPORT9*>(&myViewport));
}

void ParaEngine::CViewportManager::GetCurrentViewport(ParaViewport& out)
{
	CGlobals::GetRenderDevice()->GetViewport(reinterpret_cast<D3DVIEWPORT9*> (&out));
}

int ParaEngine::CViewportManager::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	return (int)m_viewportList.size();
}

IAttributeFields* ParaEngine::CViewportManager::GetChildAttributeObject(const std::string& sName)
{
	for (CViewport* viewport : m_viewportList)
	{
		if (viewport && viewport->GetIdentifier() == sName)
			return viewport;
	}
	return NULL;
}

IAttributeFields* ParaEngine::CViewportManager::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	return (nRowIndex < GetChildAttributeObjectCount()) ? m_viewportList[nRowIndex] : NULL;
}

int ParaEngine::CViewportManager::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);
	return S_OK;
}
