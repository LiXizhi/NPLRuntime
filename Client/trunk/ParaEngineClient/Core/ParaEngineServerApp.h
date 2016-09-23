#pragma once

#if defined(PARAENGINE_SERVER)

#include "ParaEngineAppBase.h"


// forward declare
namespace ParaEngine
{
	struct SpriteFontEntity;
	class CGUIRoot;
	class CSceneObject;
	class CAISimulator;
	class CParaWorldAsset;
	class CViewportManager;
}


namespace ParaEngine
{
	class CParaEngineApp : public CParaEngineAppBase
	{
	public:
		CParaEngineApp(const char* lpCmdLine = NULL);
		virtual ~CParaEngineApp();

	public:
		virtual HRESULT StartApp(const char* sCommandLine = 0);
		virtual HRESULT StopApp();
		virtual HRESULT FrameMove(double fTime);

		virtual DWORD GetCoreUsage();
	protected:
		void BootStrapAndLoadConfig();
		void InitSystemModules();

	protected:
		/** 2d gui root */
		ref_ptr<CGUIRoot> m_pGUIRoot;
		/** 3d scene root object */
		ref_ptr<CSceneObject>		  m_pRootScene;
		/** viewport */
		ref_ptr<CViewportManager>	  m_pViewportManager;
		/** asset manager */
		ref_ptr<CParaWorldAsset>	  m_pParaWorldAsset;
	};
}

#endif

