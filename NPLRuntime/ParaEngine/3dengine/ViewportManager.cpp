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
#include "AutoCamera.h"
#include "SceneObject.h"
#include "MoviePlatform.h"
#include "3dengine/RenderTarget.h"
#include "2dengine/GUIRoot.h"

using namespace ParaEngine;

CViewportManager::CViewportManager()
	:m_nWidth(1), m_nHeight(1), m_nActiveViewPortIndex(1), m_nLayout(VIEW_LAYOUT_INVALID)
	, m_nCurrentFrameNumber(0), m_normalScenePortInOdsSingleEye(NULL)
{
	m_viewport.X = 0;
	m_viewport.Y = 0;
	m_viewport.Width = 960;
	m_viewport.Height = 560;
	m_viewport.MinZ = 0.5;
	m_viewport.MaxZ = 200;
	ods_fov = 1.57f;
	widthPerDegree = 4;
	m_bOmniAlwaysUseUpFrontCamera = true;
	m_nOmniForceLookatDistance = 20;
}

CViewportManager::~CViewportManager(void)
{
	Cleanup();
}

void ParaEngine::CViewportManager::UpdateViewport(int nBackbufferWidth, int nBackbufferHeight)
{
	++m_nCurrentFrameNumber;
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
	int maxNum = 10;
	if (GetLayout() >= VIEW_LAYOUT_STEREO_OMNI && GetLayout() <= VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE_1) {
		maxNum = 10000;
	}
	CViewport* pViewPort = NULL;
	if (nIndex >= 0 && nIndex < (int)m_viewportList.size())
	{
		pViewPort = m_viewportList[nIndex];
	}
	else if (nIndex < maxNum)
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

void ParaEngine::CViewportManager::DeleteViewPort(const std::string& name)
{
	for (auto iter = m_viewportList.begin(); iter != m_viewportList.end(); iter++)
	{
		CViewport* pViewPort = *iter;
		if (pViewPort && pViewPort->GetIdentifier() == name)
		{
			m_viewportList.erase(iter);
			SAFE_DELETE(pViewPort);
			return;
		}
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
	bool needRecoverCamera = false;
	CAutoCamera* pCamera = NULL;
	DVector3 oldEyePos, oldLookAtPos;
	float oldFov = 0;
	float oldAspect = 1;
	float oldCameraRotX, oldCameraDistance, oldLiftUp;
	Vector3 oldRightDir;
	auto pMainScene = CGlobals::GetScene();
	if (pMainScene) {
		pCamera = (CAutoCamera*)pMainScene->GetCurrentCamera();
		oldEyePos = pCamera->GetEyePosition();
		oldLookAtPos = pCamera->GetLookAtPosition();
		oldFov = pCamera->GetFieldOfView();
		oldAspect = pCamera->GetAspectRatio();
		oldCameraRotX = (float)pCamera->GetCameraRotX();
		oldCameraDistance = (float)pCamera->GetCameraObjectDistance();
		oldLiftUp = (float)pCamera->GetCameraLiftupAngle();
		oldRightDir = pCamera->GetWorldRight();
	}

	float uiScaleX, uiScaleY;
	float uiHeight = 0;
	for (auto iter = m_viewportSorted.begin(); iter != m_viewportSorted.end(); iter++)
	{
		CViewport* pViewPort = *iter;
		if (pViewPort)
		{
			auto& _param = pViewPort->GetStereoODSparam();
			_param.oldEyePos = oldEyePos;
			_param.oldLookAtPos = oldLookAtPos;
			_param.oldFov = oldFov;
			_param.oldPitch = oldLiftUp;
			_param.oldCameraDistance = oldCameraDistance;
			_param.oldRightDir = oldRightDir;

			if (pViewPort->GetIdentifier() == "GUI_ods_user") {
				uiScaleX = pViewPort->GetGUIRoot()->GetUIScalingX();
				uiScaleY = pViewPort->GetGUIRoot()->GetUIScalingY();
				uiHeight = (float)pViewPort->GetHeight();
			}else if (pViewPort->GetIdentifier() == "GUI_ods") {
				uiScaleX = pViewPort->GetGUIRoot()->GetUIScalingX();
				uiScaleY = pViewPort->GetGUIRoot()->GetUIScalingY();
				float scale = pViewPort->GetHeight() / uiHeight;
				float _uiScaleX = uiScaleX * scale;
				float _uiScaleY = uiScaleY * scale;
				pViewPort->GetGUIRoot()->SetUIScale(_uiScaleX, _uiScaleY, false, false, false);
			}
			pViewPort->Render(dTimeDelta, nPipelineOrder);

			if (_param.needRecoverCamera) {
				needRecoverCamera = true;
			}
			if (pViewPort->GetIdentifier() == "GUI_ods") {
				pViewPort->GetGUIRoot()->SetUIScale(uiScaleX, uiScaleY, false, false, false);
			}
		}
	}
	if (pCamera && needRecoverCamera) {
		if (m_normalScenePortInOdsSingleEye) {
			pCamera->SetAspectRatio(m_normalScenePortInOdsSingleEye->GetAspectRatio());
		}
		else {
			pCamera->SetAspectRatio(oldAspect);
		}
		pCamera->SetFieldOfView(oldFov);
		pCamera->SetLookAtPosition(oldLookAtPos);
		pCamera->SetCameraRotX(oldCameraRotX);
		pCamera->SetForceOmniCameraPitch(-10000);
		pCamera->SetForceOmniCameraObjectDistance(-10000);
		pCamera->SetCameraObjectDistance(oldCameraDistance);
		pCamera->SetViewParams(oldEyePos, oldLookAtPos);
		pCamera->SetCameraLiftupAngle(oldLiftUp);
	}
	CreateGetViewPort(1)->SetActive();
	return S_OK;
}

int ParaEngine::CViewportManager::CheckInViewPortGroup(CViewport* pViewport)
{
	return 0;
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
	if (m_normalScenePortInOdsSingleEye) {
		if (x > 0 && x < m_normalScenePortInOdsSingleEye->GetWidth() && y>0 && y < m_normalScenePortInOdsSingleEye->GetHeight()) {
			*pWidth = m_normalScenePortInOdsSingleEye->GetWidth();
			*pHeight = m_normalScenePortInOdsSingleEye->GetHeight();
			return;
		}
	}
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
	if (GetLayout() == VIEW_LAYOUT_STEREO_LEFT_RIGHT || GetLayout() == VIEW_LAYOUT_STEREO_WEBXR)
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
	else if (GetLayout() == VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE)
	{

	}
	else if (GetLayout() == VIEW_LAYOUT_STEREO_OMNI)
	{

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
	m_normalScenePortInOdsSingleEye = NULL;
	m_nLayout = nLayout;
	if (nLayout == VIEW_LAYOUT_STEREO_LEFT_RIGHT || GetLayout() == VIEW_LAYOUT_STEREO_WEBXR)
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
		pUIViewportRight->DisableDeltaTime();
		CViewport* pMainSceneViewportRight = CreateGetViewPort(3);
		pMainSceneViewportRight->SetIdentifier("right_scene");
		pMainSceneViewportRight->SetScene(pMainScene);
		pMainSceneViewportRight->SetPosition("_mr", 0, 0, nHalfWidth, 0);
		pMainSceneViewportRight->SetZOrder(1);
		pMainSceneViewportRight->SetEyeMode(STEREO_EYE_RIGHT);
		pMainSceneViewportRight->DisableDeltaTime();

		SetViewportCount(4);
	}
	else if (nLayout == VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE_1)
	{
		int portNum = 0;

		//offscreen rendering
		const int num = 4;//横向4个方向
		const int cubeWidth = (GetWidth() / num);

		const bool needCompositeUI = GetHeight() - cubeWidth * 2 > 200;
		if (!needCompositeUI) {//没有足够的位置留给UI了,直接显示一个全屏UI
			CViewport* pUIViewport = CreateGetViewPort(portNum);
			pUIViewport->SetIdentifier("GUI");
			pUIViewport->SetGUIRoot(pGUIRoot);
			pUIViewport->SetPosition("_fi", 0, 0, 0, 0);
			pUIViewport->SetZOrder(103);
			pUIViewport->SetEyeMode(STEREO_EYE_NORMAL);
			pUIViewport->SetPipelineOrder(PIPELINE_UI);
			portNum += 1;
		}
		else {
			float fov_v = 60 * MATH_PI / 180;
			float fov_h = MATH_PI / 2;
			const float aspect = tan(fov_h / 2) / tan(fov_v / 2);

			// must be same as cube UI to avoid Root GUI onsize event be fired at each frame if manip GUI and cube GUI size differs.
			int _height = cubeWidth;
			int _width = (int)(_height * aspect);

			/*
			int _width = GetWidth();
			int _height = GetHeight() - cubeWidth * 2;

			if (_width > _height * aspect) {
				_width = (int)(_height * aspect);
			}
			else {
				_height = (int)(_width / aspect);
			}
			*/

			//操作区域场景+UI
			CViewport* pUIViewport = CreateGetViewPort(portNum);
			pUIViewport->SetIdentifier("GUI_ods_user");
			pUIViewport->SetGUIRoot(pGUIRoot);
			pUIViewport->SetPosition("_lt", 0, cubeWidth * 2, _width, _height);
			pUIViewport->SetZOrder(98);
			pUIViewport->SetEyeMode(STEREO_EYE_NORMAL);
			pUIViewport->SetPipelineOrder(PIPELINE_UI);
			portNum += 1;

			CViewport* pMainSceneViewport = CreateGetViewPort(portNum);
			pMainSceneViewport->SetIdentifier("scene");
			pMainSceneViewport->SetScene(pMainScene);
			pMainSceneViewport->SetPosition("_lt", 0, cubeWidth * 2, _width, _height);
			pMainSceneViewport->SetEyeMode(STEREO_EYE_NORMAL);
			pMainSceneViewport->SetZOrder(0);
			pMainSceneViewport->SetPipelineOrder(PIPELINE_3D_SCENE);
			m_normalScenePortInOdsSingleEye = pMainSceneViewport;
			{
				auto& viewport = pMainSceneViewport;
				CViewport::StereoODSparam& param = viewport->GetStereoODSparam();
				param.isODS = false;

				viewport->SetStereoODSparam(param);
			}
			portNum += 1;

			//全景区域场景+UI
			{
				_height = cubeWidth;
				_width = (int)(_height * aspect);
				CViewport* pUIViewport = CreateGetViewPort(portNum);
				pUIViewport->SetIdentifier("GUI_ods");
				pUIViewport->SetGUIRoot(pGUIRoot);
				pUIViewport->SetPosition("_lt", cubeWidth * 2, cubeWidth * 1, _width, _height);
				pUIViewport->SetZOrder(98);
				pUIViewport->SetEyeMode(STEREO_EYE_ODS);
				pUIViewport->SetPipelineOrder(PIPELINE_UI);
				pUIViewport->DisableDeltaTime();
				portNum += 1;

				CViewport* pMainSceneViewport = CreateGetViewPort(portNum);
				pMainSceneViewport->SetIdentifier("scene_ods_ui");
				pMainSceneViewport->SetScene(pMainScene);
				pMainSceneViewport->SetPosition("_lt", cubeWidth*2, cubeWidth * 1, _width, _height);
				pMainSceneViewport->SetEyeMode(STEREO_EYE_ODS);
				pMainSceneViewport->SetZOrder(0);
				pMainSceneViewport->SetPipelineOrder(PIPELINE_3D_SCENE);
				pMainSceneViewport->DisableDeltaTime();
				{
					auto& viewport = pMainSceneViewport;
					CViewport::StereoODSparam& param = viewport->GetStereoODSparam();
					param.isODS = true;
					param.aspectRatio = aspect;
					param.fov = fov_v;
					param.eyeShiftDistance = 0;
					param.fov_h = fov_h;
					param.m_bOmniAlwaysUseUpFrontCamera = m_bOmniAlwaysUseUpFrontCamera;
					param.m_nOmniForceLookatDistance = m_nOmniForceLookatDistance;
					viewport->SetStereoODSparam(param);
				}
				portNum += 1;
			}
		}

		const float diffRotY = MATH_2PI / (num);
		const float aspect = 1.0;

		const float fov_h = diffRotY;
		const float fov_v = atan(tan(MATH_PI / num) / aspect) * 2;

		const int ods_group_size = 6;//有几个viewPort共用一个renderTarget
		const std::string randerTargetname = "ods_render_target";

		const int portCoords[6][2] = {
			{0,0},{1,0},{2,0},{3,0},
			{0,1},{1,1}
		};
		const int rotYXs[6][3] = {
			{0,0,0},{-90,0,0},{-180,0,0},{-270,0,0},
			{0,90,360},{0,270,360}
		};

		for (int i = 0; i < ods_group_size; i++) {
			CViewport* viewport = CreateGetViewPort(portNum + i);
			std::string key = "scene_ods_cube_" + to_string(i);
			viewport->SetIdentifier(key);
			viewport->SetScene(pMainScene);
			viewport->SetPosition("_lt", cubeWidth * portCoords[i][0], cubeWidth * portCoords[i][1], cubeWidth, cubeWidth);
			viewport->SetEyeMode(STEREO_EYE_ODS);
			viewport->DisableDeltaTime();
			viewport->SetZOrder(50 + i);

			CViewport::StereoODSparam& param = viewport->GetStereoODSparam();
			param.isODS = true;
			param.aspectRatio = aspect;
			param.fov = fov_v;
			param.eyeShiftDistance = 0;
			param.moreRotY = rotYXs[i][0] * MATH_PI / 180;
			//param.moreRotZ = rotYXs[i][2] * MATH_PI / 180;
			param.moreRotX = rotYXs[i][1] * MATH_PI / 180;;
			param.fov_h = fov_h;
			param.m_bOmniAlwaysUseUpFrontCamera = m_bOmniAlwaysUseUpFrontCamera;
			param.m_nOmniForceLookatDistance = m_nOmniForceLookatDistance;
			param.ods_group_idx = i;
			param.ods_group_size = ods_group_size;
			viewport->SetStereoODSparam(param);
		}

		portNum += ods_group_size;

		// final full screen quad
		CViewport* pFinalViewPort = CreateGetViewPort(portNum);
		pFinalViewPort->SetIdentifier("ods_final_composite");
		pFinalViewPort->SetPosition("_lt", 0, 0, cubeWidth * 4, cubeWidth * 2);
		//pFinalViewPort->SetPosition("_lt", 0, 0, GetWidth(), GetHeight());
		pFinalViewPort->SetEyeMode(STEREO_EYE_NORMAL);
		pFinalViewPort->DisableDeltaTime();
		pFinalViewPort->SetZOrder(102);
		if (needCompositeUI) {
			pFinalViewPort->SetPipelineOrder(PIPELINE_POST_UI_3D_SCENE);
		}
		else {
			pFinalViewPort->SetPipelineOrder(PIPELINE_3D_SCENE);
		}

		portNum += 1;


		SetViewportCount(portNum);
	}

	else if (nLayout == VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE)
	{
		int portNum = 0;
		//default show
		CViewport* pUIViewport = CreateGetViewPort(0);
		pUIViewport->SetIdentifier("GUI");
		pUIViewport->SetGUIRoot(pGUIRoot);
		pUIViewport->SetPosition("_fi", 0, 0, 0, 0);
		pUIViewport->SetZOrder(100);
		pUIViewport->SetEyeMode(STEREO_EYE_NORMAL);
		pUIViewport->SetRenderTargetName("ODS_UI");
		portNum += 1;

		/*CViewport* pMainSceneViewport = CreateGetViewPort(1);
		pMainSceneViewport->SetIdentifier("scene");
		pMainSceneViewport->SetScene(pMainScene);
		pMainSceneViewport->SetPosition("_fi", 0, 0, 0, 0);
		pMainSceneViewport->SetEyeMode(STEREO_EYE_NORMAL);
		pUIViewport->SetZOrder(99);
		portNum += 1;*/

		//offscreen rendering
		int perWidth = widthPerDegree;//每一个viewPort的宽度
		//perWidth = 1;//理想情况是宽度为1
		int num = GetWidth() / perWidth;//横向上分成多少个viewPort
		int halfHeight = (GetHeight() / 2);

		const float diffRotY = MATH_2PI / (num);
		const float aspect = (float)perWidth / (float)halfHeight;

		float fov = atan(tan(MATH_PI / num) / aspect) * 2;
		fov = 90 * MATH_PI / 180;//竖直方向的视角，上看90度、下看90度，一共180度
		ods_fov = fov;
		float morePitch = fov / 2;//抬头45°角看天,垂直fov范围90°
		const std::string randerTargetname = "ods_render_target";
		for (int i = 0; i < num; i++) {
			CViewport* viewport = CreateGetViewPort(portNum + i);
			std::string key = "singleEye_scene_ods_up_" + to_string(i);
			viewport->SetIdentifier(key);
			viewport->SetScene(pMainScene);
			int offset = i;
			offset = num - 1 - offset;
			offset -= (num / 2);
			int x;
			if (offset < 0) {
				offset += num;
				x = perWidth * offset;
			}
			else {
				x = perWidth * offset;
			}
			viewport->SetPosition("_lt", x, 0, perWidth, halfHeight);
			viewport->SetEyeMode(STEREO_EYE_LEFT);

			CViewport::StereoODSparam& param = viewport->GetStereoODSparam();
			param.isODS = true;
			param.aspectRatio = aspect;
			param.fov = fov;
			param.eyeShiftDistance = 0;
			param.moreRotY = diffRotY * (i + 0);
			param.moreRotX = morePitch;
			param.fov_h = diffRotY;
			param.m_bOmniAlwaysUseUpFrontCamera = m_bOmniAlwaysUseUpFrontCamera;
			param.m_nOmniForceLookatDistance = m_nOmniForceLookatDistance;
			viewport->SetStereoODSparam(param);
		}
		portNum += num;
		morePitch = -fov / 2;
		for (int i = 0; i < num; i++) {
			//left eye,down 90
			CViewport* viewport = CreateGetViewPort(portNum + i);
			std::string key = "singleEye_scene_ods_down_" + to_string(i);
			viewport->SetIdentifier(key);
			viewport->SetScene(pMainScene);
			int offset = i;
			offset = num - 1 - offset;
			offset -= (num / 2);
			if (offset < 0) {
				offset += num;
			}
			viewport->SetPosition("_lt", perWidth * offset, halfHeight, perWidth, halfHeight);
			viewport->SetEyeMode(STEREO_EYE_LEFT);

			CViewport::StereoODSparam& param = viewport->GetStereoODSparam();
			param.isODS = true;
			param.aspectRatio = aspect;
			param.fov = fov;
			param.eyeShiftDistance = 0;
			param.moreRotY = diffRotY * (i + 0);
			param.moreRotX = morePitch;
			param.fov_h = diffRotY;
			param.m_bOmniAlwaysUseUpFrontCamera = m_bOmniAlwaysUseUpFrontCamera;
			param.m_nOmniForceLookatDistance = m_nOmniForceLookatDistance;
			viewport->SetStereoODSparam(param);
			//viewport->SetRenderTargetName(randerTargetname);
		}
		portNum += num;

		SetViewportCount(portNum);
	}
	else if (nLayout == VIEW_LAYOUT_STEREO_OMNI)
	{
		int portNum = 0;
		//default show
		CViewport* pUIViewport = CreateGetViewPort(0);
		pUIViewport->SetIdentifier("GUI");
		pUIViewport->SetGUIRoot(pGUIRoot);
		pUIViewport->SetPosition("_fi", 0, 0, 0, 0);
		pUIViewport->SetZOrder(100);
		pUIViewport->SetEyeMode(STEREO_EYE_NORMAL);
		pUIViewport->SetRenderTargetName("ODS_UI");
		portNum += 1;

		/*CViewport* pMainSceneViewport = CreateGetViewPort(1);
		pMainSceneViewport->SetIdentifier("scene");
		pMainSceneViewport->SetScene(pMainScene);
		pMainSceneViewport->SetPosition("_fi", 0, 0, 0, 0);
		pMainSceneViewport->SetEyeMode(STEREO_EYE_NORMAL);
		pUIViewport->SetZOrder(99);
		portNum += 1;*/


		int perWidth = widthPerDegree;//每一个viewPort的宽度
		int num = GetWidth() / perWidth;//横向上分成多少个viewPort
		int halfHeight = GetHeight() / 4;

		int extraWidth = GetWidth() - perWidth * num;

		const float diffRotY = MATH_2PI / (num);
		const float aspect = (float)perWidth / (float)halfHeight;

		float fov = atan(tan(MATH_PI / num) / aspect) * 2;
		fov = 90 * MATH_PI / 180;//竖直方向的视角，上看90度、下看90度，一共180度
		ods_fov = fov;
		float morePitch = fov / 2;//抬头45°角看天,垂直fov范围90°
		const std::string randerTargetname = "ods_render_target";
		for (int i = 0; i < num; i++) {
			//left eye,up 90
			CViewport* viewport = CreateGetViewPort(portNum + i);
			std::string key = "left_scene_ods_up_" + to_string(i);
			viewport->SetIdentifier(key);
			viewport->SetScene(pMainScene);
			int offset = num - 1 - i;
			offset -= (num / 2);
			if (offset < 0) {
				offset += num;
			}
			viewport->SetPosition("_lt", perWidth * offset, 0, perWidth, halfHeight);
			viewport->SetEyeMode(STEREO_EYE_LEFT);

			CViewport::StereoODSparam& param = viewport->GetStereoODSparam();
			param.isODS = true;
			param.aspectRatio = aspect;
			param.fov = fov;
			param.eyeShiftDistance = -CGlobals::GetMoviePlatform()->GetStereoEyeSeparation();
			param.moreRotY = diffRotY * (i + 0);
			param.moreRotX = morePitch;
			param.fov_h = diffRotY;
			param.m_bOmniAlwaysUseUpFrontCamera = m_bOmniAlwaysUseUpFrontCamera;
			param.m_nOmniForceLookatDistance = m_nOmniForceLookatDistance;
			viewport->SetStereoODSparam(param);
			viewport->DisableDeltaTime(i>0);
			//viewport->SetRenderTargetName(randerTargetname);
		}
		portNum += num;

		morePitch = -fov / 2;
		for (int i = 0; i < num; i++) {
			//left eye,down 90
			CViewport* viewport = CreateGetViewPort(portNum + i);
			std::string key = "left_scene_ods_down_" + to_string(i);
			viewport->SetIdentifier(key);
			viewport->SetScene(pMainScene);
			int offset = num - 1 - i;
			offset -= (num / 2);
			if (offset < 0) {
				offset += num;
			}
			viewport->SetPosition("_lt", perWidth * offset, halfHeight, perWidth, halfHeight);
			viewport->SetEyeMode(STEREO_EYE_LEFT);

			CViewport::StereoODSparam& param = viewport->GetStereoODSparam();
			param.isODS = true;
			param.aspectRatio = aspect;
			param.fov = fov;
			param.eyeShiftDistance = -CGlobals::GetMoviePlatform()->GetStereoEyeSeparation();
			param.moreRotY = diffRotY * (i + 0);
			param.moreRotX = morePitch;
			param.fov_h = diffRotY;
			param.m_bOmniAlwaysUseUpFrontCamera = m_bOmniAlwaysUseUpFrontCamera;
			param.m_nOmniForceLookatDistance = m_nOmniForceLookatDistance;
			viewport->SetStereoODSparam(param);
			viewport->DisableDeltaTime();
			//viewport->SetRenderTargetName(randerTargetname);
		}
		portNum += num;

		morePitch = fov / 2;
		for (int i = 0; i < num; i++) {
			//right eye,up 90
			CViewport* viewport = CreateGetViewPort(portNum + i);
			std::string key = "right_scene_ods_up_" + to_string(i);
			viewport->SetIdentifier(key);
			viewport->SetScene(pMainScene);
			int offset = num - 1 - i;
			offset -= (num / 2);
			if (offset < 0) {
				offset += num;
			}
			viewport->SetPosition("_lt", perWidth * offset, halfHeight * 2, perWidth, halfHeight);
			viewport->SetEyeMode(STEREO_EYE_LEFT);

			CViewport::StereoODSparam& param = viewport->GetStereoODSparam();
			param.isODS = true;
			param.aspectRatio = aspect;
			param.fov = fov;
			param.eyeShiftDistance = CGlobals::GetMoviePlatform()->GetStereoEyeSeparation();
			param.moreRotY = diffRotY * (i + 0);
			param.moreRotX = morePitch;
			param.fov_h = diffRotY;
			param.m_bOmniAlwaysUseUpFrontCamera = m_bOmniAlwaysUseUpFrontCamera;
			param.m_nOmniForceLookatDistance = m_nOmniForceLookatDistance;
			viewport->SetStereoODSparam(param);
			viewport->DisableDeltaTime();
			//viewport->SetRenderTargetName(randerTargetname);
		}
		portNum += num;

		morePitch = -fov / 2;
		for (int i = 0; i < num; i++) {
			//right eye,down 90
			CViewport* viewport = CreateGetViewPort(portNum + i);
			std::string key = "right_scene_ods_down_" + to_string(i);
			viewport->SetIdentifier(key);
			viewport->SetScene(pMainScene);
			int offset = num - 1 - i;
			offset -= (num / 2);
			if (offset < 0) {
				offset += num;
			}
			viewport->SetPosition("_lt", perWidth * offset, halfHeight * 3, perWidth, halfHeight);
			viewport->SetEyeMode(STEREO_EYE_LEFT);

			CViewport::StereoODSparam& param = viewport->GetStereoODSparam();
			param.isODS = true;
			param.aspectRatio = aspect;
			param.fov = fov;
			param.eyeShiftDistance = CGlobals::GetMoviePlatform()->GetStereoEyeSeparation();
			param.moreRotY = diffRotY * (i + 0);
			param.moreRotX = morePitch;
			param.fov_h = diffRotY;
			param.m_bOmniAlwaysUseUpFrontCamera = m_bOmniAlwaysUseUpFrontCamera;
			param.m_nOmniForceLookatDistance = m_nOmniForceLookatDistance;
			viewport->SetStereoODSparam(param);
			viewport->DisableDeltaTime();
			//viewport->SetRenderTargetName(randerTargetname);
		}
		portNum += num;


		SetViewportCount(portNum);
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
		pMainSceneViewportRight->SetRenderTargetName("_ColorRT"); // use private render target
		pMainSceneViewportRight->DisableDeltaTime();

		// final full screen quad
		CViewport* pFinalViewPort = CreateGetViewPort(3);
		pFinalViewPort->SetIdentifier("final_composite");
		pFinalViewPort->SetPosition("_fi", 0, 0, 0, 0);
		pFinalViewPort->SetEyeMode(STEREO_EYE_NORMAL);
		pFinalViewPort->SetZOrder(99); // before GUI
		pFinalViewPort->SetPipelineOrder(PIPELINE_3D_SCENE);
		pFinalViewPort->DisableDeltaTime();
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

	// refresh all viewport size on next time
	m_nHeight = m_nWidth = -1;
}

void ParaEngine::CViewportManager::SetActiveViewPort(CViewport* pViewport)
{
	for (int i = 0; i < (int)m_viewportList.size(); i++)
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
	auto CurrentViewport = CGlobals::GetRenderDevice()->GetViewport();

	CurrentViewport.X = 0;
	CurrentViewport.Y = 0;
	CurrentViewport.Width = GetWidth();
	CurrentViewport.Height = GetHeight();

	CGlobals::GetRenderDevice()->SetViewport(CurrentViewport);
}

void ParaEngine::CViewportManager::GetCurrentViewport(ParaViewport& out)
{
	out = CGlobals::GetRenderDevice()->GetViewport();
}

int ParaEngine::CViewportManager::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	return (int)m_viewportList.size();
}

IAttributeFields* ParaEngine::CViewportManager::GetChildAttributeObject(const char* sName)
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
	pClass->AddField("viewPortCount", FieldType_Int, NULL, (void*)GetViewportCount_s, NULL, NULL, bOverride);
	pClass->AddField("ods_fov", FieldType_Float, (void*)SetODSFov_s, (void*)GetODSFov_s, NULL, NULL, bOverride);
	pClass->AddField("layout", FieldType_Int, NULL, (void*)GetLayout_s, NULL, NULL, bOverride);
	pClass->AddField("widthPerDegree", FieldType_Int, (void*)SetWidthPerDegree_s, (void*)GetWidthPerDegree_s, NULL, NULL, bOverride);
	pClass->AddField("OmniAlwaysUseUpFrontCamera", FieldType_Bool, (void*)SetOmniAlwaysUseUpFrontCamera_s, (void*)GetOmniAlwaysUseUpFrontCamera_s, NULL, NULL, bOverride);
	pClass->AddField("OmniForceLookatDistance", FieldType_Int, (void*)SetOmniForceLookatDistance_s, (void*)GetOmniForceLookatDistance_s, NULL, NULL, bOverride);
	pClass->AddField("DeleteViewportByName", FieldType_String, (void*)DeleteViewportByName_s, (void*)0, NULL, NULL, bOverride);
#ifdef EMSCRIPTEN
	pClass->AddField("isXR", FieldType_Bool, (void*)SetIsXR_s, (void*)GetIsXR_s, NULL, NULL, bOverride);
	pClass->AddField("webXRTime", FieldType_Int, NULL, (void*)GetWebXRTime_s, NULL, NULL, bOverride);
	pClass->AddField("webXRHeadPose", FieldType_String, NULL, (void*)GetWebXRHeadPose_s, NULL, NULL, bOverride);
	pClass->AddField("webXRViews", FieldType_String, NULL, (void*)GetWebXRViews_s, NULL, NULL, bOverride);
	pClass->AddField("webXRViewsCount", FieldType_Int, NULL, (void*)GetWebXRViewsCount_s, NULL, NULL, bOverride);
#endif
	return S_OK;
}
