#pragma once
#include "ParaEngine.h"
#include "ParaEngineSettings.h"
#include "util/os_calls.h"
#include "util/StringHelper.h"
#include "ObjectAutoReleasePool.h"
#include "AttributesManager.h"
#include "PluginManager.h"
#include "MeshPhysicsObject.h"
#include "MeshObject.h"
#include "MissileObject.h"
#include "BipedObject.h"
#include "BMaxModel/BMaxObject.h"
#include "SkyMesh.h"
#include "BlockPieceParticle.h"
#include "ContainerObject.h"
#include "RenderTarget.h"
#include "WeatherEffect.h"
#include "OverlayObject.h"
#include "LightObject.h"
#include "NPLRuntime.h"
#include "EventsCenter.h"
#include "BootStrapper.h"
#include "NPL/NPLHelper.h"
#include "AISimulator.h"
#include "AsyncLoader.h"
#include "FileManager.h"
#include "Archive.h"
#include "NPLPackageConfig.h"
#include "IO/ResourceEmbedded.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIMouseVirtual.h"
#include "FrameRateController.h"
#include "FileLogger.h"
#include "Render/IRenderContext.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICConfigManager.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "terrain/Terrain.h"
#include "ViewportManager.h"
#include "MoviePlatform.h"
#include "Framework/Common/Time/ParaTimer.h"
#include "EnvironmentSim.h"
#include "AutoCamera.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockWorldManager.h"
#include "ObjectManager.h"
#include "2dengine/GUIHighlight.h"
#include "ParaEngineAppBase.h"
#include "MainLoopBase.h"

namespace ParaEngine
{
	class CParaEngineAppOSX : public CParaEngineAppBase, public MainLoopBase
	{
	public:
        CParaEngineAppOSX();
		virtual void GameToClient(int& inout_x, int & inout_y, bool bInBackbuffer = true) override;
		virtual void ClientToGame(int& inout_x, int & inout_y, bool bInBackbuffer = true) override;
		virtual void SetRefreshTimer(float fTimeInterval, int nFrameRateControl = 0) override;
		virtual bool AppHasFocus() override;
		virtual void GetStats(string& output, DWORD dwFields) override;
		virtual bool WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value) override;
		virtual void SetAutoLowerFrameRateWhenNotFocused(bool bEnabled) override;
		virtual const char* ReadRegStr(const string& root_key, const string& sSubKey, const string& name) override;
		virtual bool WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value) override;
		virtual DWORD ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name) override;
		virtual bool GetAutoLowerFrameRateWhenNotFocused() override;
		virtual void SetToggleSoundWhenNotFocused(bool bEnabled) override;
		virtual bool GetToggleSoundWhenNotFocused() override;
		virtual int Run(HINSTANCE hInstance) override;
        virtual void Exit(int nReturnCode /*= 0*/) override;
        /** set the window title when at windowed mode */
        virtual void SetWindowText(const char* pChar) override;
        /** get the window title when at windowed mode */
        virtual const char* GetWindowText() override;

		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		virtual void GetScreenResolution(Vector2* pOut) override;
		virtual void SetScreenResolution(const Vector2& vSize) override;

        void handle_mainloop_timer(const boost::system::error_code& err);
	protected:

	};
}
