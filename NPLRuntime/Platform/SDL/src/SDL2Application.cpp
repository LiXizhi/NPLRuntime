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
#include "FramePacing.h"
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
#include "RenderSystem/rhi_core/RHIBackendRegistry.h"

#include "SDL2/SDL.h"
#include "json/json.h"
#include <fstream>
#include <chrono>

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#elif defined(__linux__) && !defined(ANDROID)
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#endif

ParaEngine::IParaEngineApp *CreateParaEngineApp()
{
	return new ParaEngine::CSDL2Application();
}

namespace ParaEngine
{
	namespace
	{
		std::string NormalizeRegPath(const std::string& root_key, const std::string& sSubKey)
		{
			std::string key = root_key + "/" + sSubKey;
			for (size_t i = 0; i < key.size(); ++i)
			{
				if (key[i] == '\\')
					key[i] = '/';
			}
			return key;
		}

		std::string GetLocalRegistryFilePath()
		{
			return CParaFile::GetWritablePath() + "temp/para_registry.json";
		}

		Json::Value& GetLocalRegistryStore()
		{
			static Json::Value s_store(Json::objectValue);
			static bool s_loaded = false;
			if (!s_loaded)
			{
				s_loaded = true;
				std::string filePath = GetLocalRegistryFilePath();
				std::ifstream ifs(filePath, std::ios::binary);
				if (ifs.is_open())
				{
					Json::Reader reader;
					reader.parse(ifs, s_store, false);
					if (!s_store.isObject())
						s_store = Json::Value(Json::objectValue);
				}
			}
			return s_store;
		}

		bool SaveLocalRegistryStore()
		{
			std::string filePath = GetLocalRegistryFilePath();
			CParaFile::MakeDirectoryFromFilePath(filePath.c_str());
			std::ofstream ofs(filePath, std::ios::binary | std::ios::trunc);
			if (!ofs.is_open())
				return false;
			Json::StyledWriter writer;
			std::string serialized = writer.write(GetLocalRegistryStore());
			ofs.write(serialized.data(), (std::streamsize)serialized.size());
			ofs.flush();
#ifdef __EMSCRIPTEN__
			EM_ASM(if (typeof FS !== "undefined" && FS.syncfs) { FS.syncfs(false, function(err) {}); });
#endif
			return true;
		}
#if defined(__linux__) && !defined(ANDROID) && !defined(__EMSCRIPTEN__)
		std::string GetLinuxSingleInstanceSocketPath()
		{
			return "/tmp/paracraft_ipc_" + std::to_string((unsigned long)::getuid()) + ".sock";
		}
#endif
	}

	bool CSDL2Application::TryForwardToExistingInstance(const std::string& cmdLine)
	{
#if defined(__linux__) && !defined(ANDROID) && !defined(__EMSCRIPTEN__)
		if (cmdLine.empty())
			return false;
		std::string sockPath = GetLinuxSingleInstanceSocketPath();
		int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
		if (fd < 0)
			return false;
		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, sockPath.c_str(), sizeof(addr.sun_path) - 1);
		bool sent = false;
		if (::connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
		{
			ssize_t written = ::write(fd, cmdLine.data(), cmdLine.size());
			sent = (written == (ssize_t)cmdLine.size());
		}
		::close(fd);
		return sent;
#else
		return false;
#endif
	}

	void CSDL2Application::InitSingleInstanceListener()
	{
#if defined(__linux__) && !defined(ANDROID) && !defined(__EMSCRIPTEN__)
		if (m_ipcListenFd >= 0)
			return;
		const char* sMulti = GetAppCommandLineByParam("multipleinstance", nullptr);
		if (sMulti && strcmp(sMulti, "true") == 0)
			return;

		std::string sockPath = GetLinuxSingleInstanceSocketPath();
		// Probe first so we don't unlink a live primary instance's socket
		int probeFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
		if (probeFd >= 0)
		{
			struct sockaddr_un probeAddr;
			memset(&probeAddr, 0, sizeof(probeAddr));
			probeAddr.sun_family = AF_UNIX;
			strncpy(probeAddr.sun_path, sockPath.c_str(), sizeof(probeAddr.sun_path) - 1);
			bool alive = (::connect(probeFd, (struct sockaddr*)&probeAddr, sizeof(probeAddr)) == 0);
			::close(probeFd);
			if (alive)
				return;
		}
		::unlink(sockPath.c_str());

		int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
		if (fd < 0)
			return;
		int flags = ::fcntl(fd, F_GETFL, 0);
		if (flags >= 0)
			::fcntl(fd, F_SETFL, flags | O_NONBLOCK);

		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, sockPath.c_str(), sizeof(addr.sun_path) - 1);
		if (::bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0 && ::listen(fd, 4) == 0)
		{
			m_ipcListenFd = fd;
			m_ipcSockPath = sockPath;
		}
		else
		{
			::close(fd);
		}
#endif
	}

	void CSDL2Application::PollSingleInstanceSocket()
	{
#if defined(__linux__) && !defined(ANDROID) && !defined(__EMSCRIPTEN__)
		if (m_ipcListenFd < 0)
			return;
		while (true)
		{
			int clientFd = ::accept(m_ipcListenFd, nullptr, nullptr);
			if (clientFd < 0)
				break;
			std::string cmd;
			char buf[2048];
			ssize_t n = 0;
			while ((n = ::read(clientFd, buf, sizeof(buf))) > 0)
			{
				cmd.append(buf, (size_t)n);
			}
			::close(clientFd);
			if (!cmd.empty())
			{
				SDL_Window* sdlWin = (SDL_Window*)m_renderWindow.GetNativeHandle();
				if (sdlWin)
					SDL_RaiseWindow(sdlWin);
				this->onCmdLine(cmd);
			}
		}
#endif
	}

	void CSDL2Application::CloseSingleInstanceListener()
	{
#if defined(__linux__) && !defined(ANDROID) && !defined(__EMSCRIPTEN__)
		if (m_ipcListenFd >= 0)
		{
			::close(m_ipcListenFd);
			m_ipcListenFd = -1;
		}
		if (!m_ipcSockPath.empty())
		{
			::unlink(m_ipcSockPath.c_str());
			m_ipcSockPath.clear();
		}
#endif
	}

	void CSDL2Application::GameToClient(int &inout_x, int &inout_y, bool bInBackbuffer /*= true*/)
	{
	}

	void CSDL2Application::ClientToGame(int &inout_x, int &inout_y, bool bInBackbuffer /*= true*/)
	{
	}

	void CSDL2Application::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
	{
#ifdef EMSCRIPTEN_SINGLE_THREAD
		// Browser pacing is owned by requestAnimationFrame; preserve its controls.
		m_fRefreshTimerInterval = fTimeInterval;
		m_nFrameRateControl = nFrameRateControl;
#else
		CParaEngineAppBase::SetRefreshTimer(fTimeInterval, nFrameRateControl);
#endif
	}

	bool CSDL2Application::AppHasFocus()
	{
		SDL_Window* sdlWin = (SDL_Window*)m_renderWindow.GetNativeHandle();
		if (sdlWin)
			return (SDL_GetWindowFlags(sdlWin) & SDL_WINDOW_INPUT_FOCUS) != 0;
		return true;
	}

	void CSDL2Application::GetStats(string &output, DWORD dwFields)
	{
		if (dwFields == 0)
		{
			output = RHIBackendRegistry::GetBackendName(m_cfg.preferredBackend);
		}
	}

	bool CSDL2Application::WriteRegStr(const string &root_key, const string &sSubKey, const string &name, const string &value)
	{
		Json::Value& store = GetLocalRegistryStore();
		std::string section = NormalizeRegPath(root_key, sSubKey);
		store[section][name] = value;
		return SaveLocalRegistryStore();
	}

	void CSDL2Application::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
	{
		m_bAutoLowerFrameRateWhenNotFocused = bEnabled;
	}

	const char *CSDL2Application::ReadRegStr(const string &root_key, const string &sSubKey, const string &name)
	{
		static std::string s_lastRegVal;
		Json::Value& store = GetLocalRegistryStore();
		std::string section = NormalizeRegPath(root_key, sSubKey);
		if (!store.isMember(section) || !store[section].isObject() || !store[section].isMember(name))
			return nullptr;
		const Json::Value& val = store[section][name];
		if (!val.isString())
			return nullptr;
		s_lastRegVal = val.asString();
		return s_lastRegVal.c_str();
	}

	bool CSDL2Application::WriteRegDWORD(const string &root_key, const string &sSubKey, const string &name, DWORD value)
	{
		Json::Value& store = GetLocalRegistryStore();
		std::string section = NormalizeRegPath(root_key, sSubKey);
		store[section][name] = (Json::UInt)value;
		return SaveLocalRegistryStore();
	}

	DWORD CSDL2Application::ReadRegDWORD(const string &root_key, const string &sSubKey, const string &name)
	{
		Json::Value& store = GetLocalRegistryStore();
		std::string section = NormalizeRegPath(root_key, sSubKey);
		if (!store.isMember(section) || !store[section].isObject() || !store[section].isMember(name))
			return 0;
		const Json::Value& val = store[section][name];
		if (val.isIntegral())
			return (DWORD)val.asUInt();
		return 0;
	}

	bool CSDL2Application::GetAutoLowerFrameRateWhenNotFocused()
	{
		return m_bAutoLowerFrameRateWhenNotFocused;
	}

	void CSDL2Application::SetToggleSoundWhenNotFocused(bool bEnabled)
	{
		m_bToggleSoundWhenNotFocused = bEnabled;
	}

	bool CSDL2Application::GetToggleSoundWhenNotFocused()
	{
		return m_bToggleSoundWhenNotFocused;
	}

	bool CSDL2Application::IsInputing()
	{
#ifdef __EMSCRIPTEN__
		int inputing = EM_ASM_INT({ return Module.IsInputing(); });
		return inputing == 1;
#else
		return 0;
#endif
	}

	void CSDL2Application::setIMEKeyboardState(bool bOpen, bool bMoveView, int ctrlBottom, const string &editParams)
	{
#ifdef __EMSCRIPTEN__
		if (bOpen)
		{
			// std::cout << bMoveView << " " << ctrlBottom << " " << editParams << std::endl;
			EM_ASM({
				Module.StartTextInput($0, $1);
			}, ctrlBottom, editParams.c_str());
		}
		else
		{
			EM_ASM({
				Module.StopTextInput();
			});
		}
#endif
	}

	CSDL2Application::CSDL2Application()
		: m_bUpdateScreenDevice(false), m_bSizeChanged(false)
	{
		SDL_Init(SDL_INIT_EVERYTHING);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#ifdef __EMSCRIPTEN__
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

		SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
		
		m_exit = false;
		m_is_wps_office_app = false;
		m_bAutoLowerFrameRateWhenNotFocused = false;
		m_bToggleSoundWhenNotFocused = false;
		m_ipcListenFd = -1;
	}

	CSDL2Application::~CSDL2Application()
	{
		CloseSingleInstanceListener();
		SDL_Quit();
	}

	bool CSDL2Application::InitApp(IRenderWindow *pWindow, const char *sCommandLine)
	{
		SetAppCommandLine(sCommandLine);
		BootStrapAndLoadConfig();
		LoadAndApplySettings();
		InitSingleInstanceListener();

		auto pWinDelegate = (RenderWindowDelegate *)&m_renderWindow;
		assert(pWinDelegate);

		m_cfg.renderWindow = pWinDelegate;
		m_cfg.preferredBackend = RHIBackendRegistry::GetInstance().ResolvePreferredBackend(sCommandLine);
		RHIBackendRegistry::GetInstance().SetPreferredBackend(m_cfg.preferredBackend);
		pWinDelegate->Create(m_cfg.screenWidth, m_cfg.screenHeight);

		return CParaEngineAppBase::InitApp(pWinDelegate, sCommandLine);
	}

	int CSDL2Application::Run()
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		NextLoop(50, &CSDL2Application::handle_mainloop_timer, this);
		MainLoopRun();
		FinalCleanup();
#endif
		return GetReturnCode();
	}

#ifndef EMSCRIPTEN_SINGLE_THREAD
	void CSDL2Application::handle_mainloop_timer(const boost::system::error_code &err)
	{
		if (!err)
		{
			auto pWindow = (RenderWindowDelegate *)m_pRenderWindow;
			assert(pWindow);

			if (!m_exit && !pWindow->ShouldClose())
			{
				const auto frameStart = std::chrono::steady_clock::now();
				PollSingleInstanceSocket();
				pWindow->PollEvents();
				UpdateScreenDevice();
				this->DoWork();
				if (!m_exit && !pWindow->ShouldClose())
				{
					const double elapsed = std::chrono::duration<double>(
						std::chrono::steady_clock::now() - frameStart).count();
					NextLoop(GetRemainingFrameDelayMs(GetRefreshTimer(), elapsed),
						&CSDL2Application::handle_mainloop_timer, this);
				}
				else
				{
					MainLoopStop();
				}
			}
			else
			{
				MainLoopStop();
			}
		}
	}
#endif
	void CSDL2Application::WriteConfigFile(const char *FileName)
	{
		std::string sFileName;
		if (FileName == nullptr || FileName[0] == '\0')
			sFileName = "config/config.txt";
		else
			sFileName = FileName;

		{
			// remove the read-only file attribute
			// DWORD dwAttrs = ::GetFileAttributes(sFileName.c_str());
			// if (dwAttrs != INVALID_FILE_ATTRIBUTES)
			//{
			// if ((dwAttrs & FILE_ATTRIBUTE_READONLY))
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

		ParaEngineSettings &settings = ParaEngineSettings::GetSingleton();
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

	void CSDL2Application::GetScreenResolution(Vector2 *pOut)
	{
		if (pOut)
		{
			GetResolution(&(pOut->x), &(pOut->y));
		}
	}

	void CSDL2Application::SetScreenResolution(const Vector2 &vSize)
	{
#ifdef __EMSCRIPTEN__
		// The web shell owns the canvas size (including devicePixelRatio).
		// Haqi's desktop settings page still submits resolutions such as 1280x720.
		// Applying those only to m_cfg/RenderWindow leaves the canvas and SDL mouse
		// coordinates unchanged, breaking GUI picking and text clipping. Browser
		// resizes reach SetResolution through RenderWindowDelegate::OnSize instead.
		if (m_pRenderWindow && m_pRenderWindow->GetWidth() > 0 && m_pRenderWindow->GetHeight() > 0)
		{
			// Leave any pending canvas resize intact as well.
			return;
		}
#endif
		SetResolution(vSize.x, vSize.y);
		UpdateScreenMode();

		// if (m_cfg.isWindowed && m_pRenderWindow)
		//{
		//	auto pWinDelegate = (RenderWindowDelegate*)m_pRenderWindow;
		//	pWinDelegate->SetSize((int)vSize.x, (int)vSize.y);
		// }
	}

	void CSDL2Application::GetResolution(float *pX, float *pY)
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

			ParaEngineSettings &settings = ParaEngineSettings::GetSingleton();
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
		ParaEngineSettings &settings = ParaEngineSettings::GetSingleton();

		CDynamicAttributeField *pField = nullptr;
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

#ifdef __EMSCRIPTEN__
		{
			int canvasW = EM_ASM_INT({
				var c = (typeof Module !== "undefined" && Module.canvas) ? Module.canvas : document.getElementById("canvas");
				if (c && c.width > 0) return c.width;
				return Math.ceil(document.documentElement.clientWidth * (window.devicePixelRatio || 1));
			});
			int canvasH = EM_ASM_INT({
				var c = (typeof Module !== "undefined" && Module.canvas) ? Module.canvas : document.getElementById("canvas");
				if (c && c.height > 0) return c.height;
				return Math.ceil(document.documentElement.clientHeight * (window.devicePixelRatio || 1));
			});
			if (canvasW > 0 && canvasH > 0)
			{
				m_cfg.screenWidth = (uint32_t)canvasW;
				m_cfg.screenHeight = (uint32_t)canvasH;
				CVariable val;
				val = canvasW;
				settings.SetDynamicField("ScreenWidth", val);
				val = canvasH;
				settings.SetDynamicField("ScreenHeight", val);
			}
		}
#endif

		if ((pField = settings.GetDynamicField("ScriptEditor")))
			settings.SetScriptEditor((const std::string &)(*pField));

		if ((pField = settings.GetDynamicField("InverseMouse")))
			settings.SetMouseInverse((bool)(*pField));

		if ((pField = settings.GetDynamicField("language")))
			settings.SetLocale((const char *)(*pField));

		if ((pField = settings.GetDynamicField("TextureLOD")))
			settings.SetTextureLOD((int)(*pField));

		const char *sIsFullScreen = GetAppCommandLineByParam("fullscreen", nullptr);
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

	void CSDL2Application::SetWindowText(const char *pChar)
	{
		m_title = pChar;
		auto pWinDelegate = (RenderWindowDelegate *)m_pRenderWindow;
		if (pWinDelegate) pWinDelegate->SetWindowText(m_title);
	}

	const char *CSDL2Application::GetWindowText()
	{
		return m_title.c_str();
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

			auto pWinDelegate = (RenderWindowDelegate *)m_pRenderWindow;
			// auto hWnd = pWinDelegate->GetHandle();

			bool bReset = false;

			{
				// RECT rect;
				//::GetClientRect(hWnd, &rect);
				// if (m_cfg.screenWidth != rect.right - rect.left
				//	|| m_cfg.screenHeight != rect.bottom - rect.top)
				if (m_bSizeChanged)
				{
					if (m_pRenderDevice)
						m_pRenderDevice->Flush();
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
#ifndef __EMSCRIPTEN__
		SDL_Window* sdlWin = (SDL_Window*)m_renderWindow.GetNativeHandle();
		if (IsWindowedMode() && sdlWin)
		{
			SDL_SetWindowResizable(sdlWin, fixed ? SDL_FALSE : SDL_TRUE);
		}
#endif
	}

	void CSDL2Application::ShowWindow(bool bShow)
	{
#ifndef __EMSCRIPTEN__
		SDL_Window* sdlWin = (SDL_Window*)m_renderWindow.GetNativeHandle();
		if (IsWindowedMode() && sdlWin)
		{
			if (bShow)
				SDL_ShowWindow(sdlWin);
			else
				SDL_HideWindow(sdlWin);
		}
#endif
	}


	void CSDL2Application::SetLandscapeMode(std::string landscapeMode)
	{
		auto pWinDelegate = (RenderWindowDelegate *)&m_renderWindow;
		if (landscapeMode == "on")
		{
			pWinDelegate->SetScreenOrientation(RenderWindowDelegate::s_screen_orientation_landscape);
		}
		else if (landscapeMode == "off")
		{
			pWinDelegate->SetScreenOrientation(RenderWindowDelegate::s_screen_orientation_portrait);
		}
		else
		{
			pWinDelegate->SetScreenOrientation(RenderWindowDelegate::s_screen_orientation_auto);
		}
	}

	std::string CSDL2Application::GetLandscapeMode()
	{
		auto pWinDelegate = (RenderWindowDelegate *)&m_renderWindow;
		auto screen_orientation = pWinDelegate->GetScreenOrientation();
		if (screen_orientation == RenderWindowDelegate::s_screen_orientation_landscape)
		{
			return "on";
		}
		else if (screen_orientation == RenderWindowDelegate::s_screen_orientation_portrait)
		{
			return "off";
		}
		else
		{
			return "auto";
		}
	}

	bool CSDL2Application::IsRotateScreen()
	{
		auto pWinDelegate = (RenderWindowDelegate *)&m_renderWindow;
		return pWinDelegate->IsRotateScreen();
	}

} // end namespace
