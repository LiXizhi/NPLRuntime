#pragma once

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
	class CParaEngineServerApp : public CParaEngineAppBase
	{
	public:
		CParaEngineServerApp(const char* lpCmdLine = NULL);
		virtual ~CParaEngineServerApp();

	public:
		virtual bool StartApp(const char* sCommandLine);
		virtual void StopApp() override;
		virtual bool FrameMove(double fTime) override;

		virtual DWORD GetCoreUsage() override;
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
