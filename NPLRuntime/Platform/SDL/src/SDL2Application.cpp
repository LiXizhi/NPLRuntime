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
#include "SDL2Application.h"
#include "RenderWindowDelegate.h"
#include "DynamicAttributeField.h"
#include "ParaEngineInfo.h"


#include "SDL2/SDL.h"

ParaEngine::IParaEngineApp* CreateParaEngineApp()
{
	return new ParaEngine::CSDL2Application();
}


namespace ParaEngine {


	void CSDL2Application::GameToClient(int& inout_x, int& inout_y, bool bInBackbuffer /*= true*/)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	void CSDL2Application::ClientToGame(int& inout_x, int& inout_y, bool bInBackbuffer /*= true*/)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	void CSDL2Application::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	bool CSDL2Application::AppHasFocus()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}

	void CSDL2Application::GetStats(string& output, DWORD dwFields)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	bool CSDL2Application::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}

	void CSDL2Application::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
	{
		//throw std::logic_error("The method or operation is not implemented.");
	}

	const char* CSDL2Application::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		return "";
	}

	bool CSDL2Application::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}

	DWORD CSDL2Application::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
	{
		//throw std::logic_error("The method or operation is not implemented.");
		return  0;
	}

	bool CSDL2Application::GetAutoLowerFrameRateWhenNotFocused()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}

	void CSDL2Application::SetToggleSoundWhenNotFocused(bool bEnabled)
	{
		//	throw std::logic_error("The method or operation is not implemented.");

	}

	bool CSDL2Application::GetToggleSoundWhenNotFocused()
	{
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}


	CSDL2Application::CSDL2Application()
		: m_bUpdateScreenDevice(false)
		, m_bSizeChanged(false)
	{
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_Init(SDL_INIT_EVERYTHING);
		m_exit = false;
	}

	CSDL2Application::~CSDL2Application()
	{
		SDL_Quit();
	}

	bool CSDL2Application::InitApp(IRenderWindow* pWindow, const char* sCommandLine)
	{
		BootStrapAndLoadConfig();
		LoadAndApplySettings();

		auto pWinDelegate = (RenderWindowDelegate*)&m_renderWindow;
		assert(pWinDelegate);

		m_cfg.renderWindow = pWinDelegate;
		pWinDelegate->Create(m_cfg.screenWidth, m_cfg.screenHeight);

		return CParaEngineAppBase::InitApp(pWinDelegate, sCommandLine);
	}


	int CSDL2Application::Run()
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		NextLoop(50, &CSDL2Application::handle_mainloop_timer, this);
		MainLoopRun();
#endif
		return 0;
	}

#ifndef EMSCRIPTEN_SINGLE_THREAD
	void CSDL2Application::handle_mainloop_timer(const boost::system::error_code& err)
	{
		if (!err)
		{
			auto pWindow = (RenderWindowDelegate*)m_pRenderWindow;
			assert(pWindow);

			if (!pWindow->ShouldClose())
			{
				pWindow->PollEvents();
				UpdateScreenDevice();
				this->DoWork();

				double fNextInterval = 0.01f; // as fast as possible
				//fNextInterval = this->GetRefreshTimer() - (ParaTimer::GetAbsoluteTime() - this->GetAppTime());
				//fNextInterval = (std::min)(0.1, (std::max)(0.02, fNextInterval));  // [0.02, 0.1] 

				NextLoop((int)(fNextInterval * 1000), &CSDL2Application::handle_mainloop_timer, this);
			}
		}
	}
#endif
	void CSDL2Application::WriteConfigFile(const char* FileName)
	{
		std::string sFileName;
		if (FileName == nullptr || FileName[0] == '\0')
			sFileName = "config/config.txt";
		else
			sFileName = FileName;

		{
			// remove the read-only file attribute
			//DWORD dwAttrs = ::GetFileAttributes(sFileName.c_str());
			//if (dwAttrs != INVALID_FILE_ATTRIBUTES)
			//{
				//if ((dwAttrs & FILE_ATTRIBUTE_READONLY))
				//{
					//::SetFileAttributes(sFileName.c_str(), dwAttrs & (~FILE_ATTRIBUTE_READONLY));
				//}
			//}
		}

		CParaFile file;
		if (!file.CreateNewFile(sFileName.c_str()))
		{
			OUTPUT_LOG("failed creating file %s\r\n", sFileName.c_str());
		}
		file.WriteFormated("-- ParaEngine startup settings.\n");
		file.WriteFormated("-- Auto generated by ParaEngine %s\n", ParaEngineInfo::CParaEngineInfo::GetVersion().c_str());

		file.WriteFormated("-- MultiSampleType: hardware fullscreen MultiSample AntiAliasing(AA): 0 to disable;2 to enable;4 is better\n");
		file.WriteFormated("-- MultiSampleQuality: 0 default quality, 1 for higher quality. the higher value, the more quality\n");
		file.WriteFormated("-- ScriptEditor: default is editor/notepad.exe, UltraEdit is another good choice if one has it.\n");
		file.WriteFormated("-- GameEffectSet: initial game effect set, 0 is recommended. possible value is [1024, 0,1,2,-1,-2]. 1024 means FF, otherwise the larger the better.\n");
		file.WriteFormated("-- language: possible values are enUS or zhCN\n");

		ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();
		CVariable value;

		value = !m_cfg.isWindowed; //  IsWindowedMode();
		settings.SetDynamicField("StartFullscreen", value);

		value = settings.GetScriptEditor();
		settings.SetDynamicField("ScriptEditor", value);

		value = settings.GetGameEffectSet();
		settings.SetDynamicField("GameEffectSet", value);

		value = settings.GetMouseInverse();
		settings.SetDynamicField("InverseMouse", value);

		value = settings.GetLocale();
		settings.SetDynamicField("language", value);

		value = settings.GetTextureLOD();
		settings.SetDynamicField("TextureLOD", value);

		string field_text;
		settings.SaveDynamicFieldsToString(field_text);
		file.WriteString(field_text);

		OUTPUT_LOG("ParaEngine config file is saved to %s\r\n", sFileName.c_str());
	}

	void CSDL2Application::GetScreenResolution(Vector2* pOut)
	{
		if (pOut)
		{
			GetResolution(&(pOut->x), &(pOut->y));
		}
	}

	void CSDL2Application::SetScreenResolution(const Vector2& vSize)
	{
		SetResolution(vSize.x, vSize.y);
		UpdateScreenMode();

		//if (m_cfg.isWindowed && m_pRenderWindow)
		//{
		//	auto pWinDelegate = (RenderWindowDelegate*)m_pRenderWindow;
		//	pWinDelegate->SetSize((int)vSize.x, (int)vSize.y);
		//}
	}

	void CSDL2Application::GetResolution(float* pX, float* pY)
	{
		if (pX)
		{
			*pX = (float)m_cfg.screenWidth;
		}
		if (pY)
		{
			*pY = (float)m_cfg.screenHeight;
		}
	}

	void CSDL2Application::SetResolution(float x, float y)
	{
		uint32_t _x = (uint32_t)(x);
		uint32_t _y = (uint32_t)(y);

		m_bSizeChanged = m_cfg.screenWidth != _x || m_cfg.screenHeight != _y;

		if (m_bSizeChanged)
		{
			m_cfg.screenWidth = _x;
			m_cfg.screenHeight = _y;


			ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();
			CVariable value;
			value = (int)(x);
			settings.SetDynamicField("ScreenWidth", value);
			value = (int)(y);
			settings.SetDynamicField("ScreenHeight", value);
		}
	}

	void CSDL2Application::LoadAndApplySettings()
	{
		// load from settings.
		ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();

		CDynamicAttributeField* pField = nullptr;
		if ((pField = settings.GetDynamicField("StartFullscreen")))
			m_cfg.isWindowed = !((bool)(*pField));
		else
			m_cfg.isWindowed = true;

		if ((pField = settings.GetDynamicField("ScreenWidth")))
			m_cfg.screenWidth = (int)(*pField);
		else
			m_cfg.screenWidth = 1020;

		if ((pField = settings.GetDynamicField("ScreenHeight")))
			m_cfg.screenHeight = (int)(*pField);
		else
			m_cfg.screenHeight = 680;

		if ((pField = settings.GetDynamicField("ScriptEditor")))
			settings.SetScriptEditor((const std::string&)(*pField));

		if ((pField = settings.GetDynamicField("InverseMouse")))
			settings.SetMouseInverse((bool)(*pField));

		if ((pField = settings.GetDynamicField("language")))
			settings.SetLocale((const char*)(*pField));

		if ((pField = settings.GetDynamicField("TextureLOD")))
			settings.SetTextureLOD((int)(*pField));

		const char* sIsFullScreen = GetAppCommandLineByParam("fullscreen", nullptr);
		if (sIsFullScreen)
			m_cfg.isWindowed = (strcmp("true", sIsFullScreen) != 0);
	}

	bool CSDL2Application::UpdateScreenMode()
	{
		m_bUpdateScreenDevice = true;
		return true;
	}

	bool CSDL2Application::SetWindowedMode(bool bWindowed)
	{
		if (IsWindowedMode() == bWindowed)
			return true;

		m_cfg.isWindowed = bWindowed;

		return UpdateScreenMode();
	}

	bool CSDL2Application::IsWindowedMode()
	{
		return m_cfg.isWindowed;
	}

	void CSDL2Application::SetWindowText(const char* pChar)
	{
		std::wstring s = StringHelper::MultiByteToWideChar(pChar, CP_UTF8);
		auto pWinDelegate = (RenderWindowDelegate*)m_pRenderWindow;
		//if (pWinDelegate)::SetWindowTextW(pWinDelegate->GetHandle(), s.c_str());
	}

	const char* CSDL2Application::GetWindowText()
	{
		WCHAR g_wstr_title[256];
		static std::string g_title;
		auto pWinDelegate = (RenderWindowDelegate*)m_pRenderWindow;
		if (pWinDelegate)
		{
			//::GetWindowTextW(pWinDelegate->GetHandle(), g_wstr_title, 255);
			g_title = StringHelper::WideCharToMultiByte(g_wstr_title, CP_UTF8);

			return g_title.c_str();
		}
		else
		{
			return "";
		}
	}

	bool CSDL2Application::UpdateViewPort()
	{
		if (m_pRenderDevice)
		{
			auto rc = m_pRenderDevice->GetViewport();
			if (rc.Width != m_cfg.screenWidth || rc.Height != m_cfg.screenHeight)
			{
				rc.Width = m_cfg.screenWidth;
				rc.Height = m_cfg.screenHeight;

				return m_pRenderDevice->SetViewport(rc);
			}
		}
		return false;
	}

	bool CSDL2Application::UpdateScreenDevice()
	{
		if (m_bUpdateScreenDevice && m_pRenderWindow)
		{
			OUTPUT_LOG("update screen device to (%d, %d) windowed: %s\n", m_cfg.screenWidth, m_cfg.screenHeight, m_cfg.isWindowed ? "true" : "false");
			m_bUpdateScreenDevice = false;

			auto pWinDelegate = (RenderWindowDelegate*)m_pRenderWindow;
			//auto hWnd = pWinDelegate->GetHandle();

			bool bReset = false;

			{
				//RECT rect;
				//::GetClientRect(hWnd, &rect);
				//if (m_cfg.screenWidth != rect.right - rect.left
				//	|| m_cfg.screenHeight != rect.bottom - rect.top)
				if (m_bSizeChanged)
				{
					pWinDelegate->SetSize(m_cfg.screenWidth, m_cfg.screenHeight);
					bReset = true;
					m_bSizeChanged = false;
				}
			}

			if (bReset)
			{
				if (!m_pRenderContext || !m_pRenderDevice)
					return false;

				InvalidateDeviceObjects();
				m_pRenderContext->ResetDevice(m_pRenderDevice, m_cfg);
				auto hr = RestoreDeviceObjects();

				if (FAILED(hr))
				{
					OUTPUT_LOG("reset d3d device failed because Restor func failed: %d\n", hr);
					InvalidateDeviceObjects();

					return false;
				}

				return UpdateViewPort();
			}
		}

		return true;
	}

	void CSDL2Application::FixWindowSize(bool fixed)
	{
		//auto pWinDelegate = (RenderWindowDelegate*)m_pRenderWindow;
		//if (IsWindowedMode() && pWinDelegate)
		//{
		//	auto hwnd = pWinDelegate->GetHandle();
		//	if (!hwnd)
		//		return;

		//	auto dwWindowStyle = GetWindowStyle(hwnd);

		//	if (fixed)
		//	{
		//		dwWindowStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
		//	}
		//	else
		//	{
		//		dwWindowStyle |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
		//	}

		//	SetWindowLong(hwnd, GWL_STYLE, dwWindowStyle);
		//}
	}

	void CSDL2Application::ShowWindow(bool bShow)
	{
		//	if (!IsFullScreenMode())
		//	{
		//		if ((GetCoreUsage() & PE_USAGE_WEB_BROWSER) != 0)
		//		{
		//			OUTPUT_LOG("ShowWindow doesn't know how to respond in web browser mode!\n");
		//		}
		//		else
		//		{
		//			// show or hide the main window if it is not from a web browser
		//			::ShowWindow(CGlobals::GetAppHWND(), bShow ? SW_SHOW : SW_HIDE);
		//		}
		//	}
	}

} // end namespace
