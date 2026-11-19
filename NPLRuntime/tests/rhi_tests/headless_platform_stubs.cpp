#include "ParaEngine.h"
#include "3dengine/ParaEngineSettings.h"
#include "IParaWebXR.h"
#include "2dengine/GUIEdit.h"
#include "util/NetworkAdapter.h"
#include "util/StringHelper.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "Framework/Common/PlatformBridge/PlatformBridge.h"
#include "Framework/FileSystem/ParaFileUtils.h"
#include "NPL/NPLScriptingState.h"
#include "ParaScriptBindings/ParaScriptingGlobal.h"
#include "IParaWebView.h"
#if defined(PLATFORM_MAC)
#include "IOMac/SerialPortMac.h"
#include "INPLRuntimeState.h"
// These optional native services are registered by NPL even in headless tests.
extern "C" NPL::NPLReturnCode NPL_activate_LuaObjcBridge_cpp(NPL::INPLRuntimeState*)
{
	return NPL::NPL_Error;
}
std::vector<std::string> ParaEngine::SerialPortMac::GetPortNames() { return {}; }
#endif

// Minimal headless platform stubs for linking ParaEngine static library without a GUI application target
#ifdef CreateProcess
#undef CreateProcess
#endif
#ifdef ShellExecute
#undef ShellExecute
#endif

ParaEngine::IParaEngineApp* CreateParaEngineApp() { return nullptr; }
ParaEngine::IRenderWindow* CreateParaRenderWindow(const int, const int) { return nullptr; }

namespace ParaEngine
{
	void NetworkAdapter::Init() { m_bInitialized = true; }
	const std::string& NetworkAdapter::GetMaxMacAddress() { static const std::string s; return s; }
	const std::string& NetworkAdapter::GetMaxIPAddress() { static const std::string s; return s; }
	const std::string& ParaEngineSettings::GetMachineID() { static const std::string s = "headless_test"; return s; }

	bool CEditorHelper::OpenWithDefaultEditor(const char*, bool) { return false; }
	bool CEditorHelper::CreateEmptyCharacterEventFile(const char*, const char*) { return false; }
	bool CEditorHelper::CreateProcess(const char*, const char*, bool) { return false; }
	bool CEditorHelper::ShellExecute(const char*, const char*, const char*, const char*, int) { return false; }

#if (PARA_TARGET_PLATFORM != PARA_PLATFORM_LINUX) && !defined(MAC_SERVER) && !defined(__EMSCRIPTEN__) && !defined(NPLRUNTIME_SERVER) && !defined(NPL_TEST_PLATFORM_FILE_UTILS)
	CParaFileUtils* CParaFileUtils::GetInstance() { static CParaFileUtils files; return &files; }
#endif

	void CGUIEditBox::CopyToClipboard() {}
	void CGUIEditBox::PasteFromClipboard() {}
	bool StringHelper::CopyTextToClipboard(const std::string&) { return false; }
	const char* StringHelper::GetTextFromClipboard() { return ""; }

	IParaWebView* IParaWebView::createWebView(int, int, int, int) { return nullptr; }
	IParaWebView* IParaWebView::createSubViewView(int, int, int, int) { return nullptr; }

	std::string PlatformBridge::call_native(std::string, const std::string) { return {}; }
	void PlatformBridge::call_native_withCB(std::string, LuaCB*, const std::string) {}

#if (defined(__EMSCRIPTEN__) || defined(WIN32) || defined(PICO_VR)) && !defined(NPLRUNTIME_SERVER)
	class CParaWebXR : public IParaWebXR
	{
	public:
		virtual void SetIsXR(bool isXR) override;
		virtual bool GetIsXR() override;
		virtual bool IsPressLeftHandKey(int keyCode) override;
		virtual bool IsPressRightHandKey(int keyCode) override;
		virtual void UpdateWebXRView(
			int time,
			Vector4 leftView, Vector3 leftPosition, Vector4 leftOrientation,
			Vector4 rightView, Vector3 rightPosition, Vector4 rightOrientation,
			int viewCount,
			Vector3 leftHandPosition, Vector3 rightHandPosition,
			Vector4 leftHandOrientation, Vector4 rightHandOrientation,
			int leftHandKey[7], int rightHandKey[7], Vector4 leftAxes, Vector4 rightAxes) override;
	};
	void CParaWebXR::SetIsXR(bool isXR) { m_isXR = isXR; }
	bool CParaWebXR::GetIsXR() { return m_isXR; }
	bool CParaWebXR::IsPressLeftHandKey(int) { return false; }
	bool CParaWebXR::IsPressRightHandKey(int) { return false; }
	void CParaWebXR::UpdateWebXRView(
		int,
		Vector4, Vector3, Vector4,
		Vector4, Vector3, Vector4,
		int,
		Vector3, Vector3,
		Vector4, Vector4,
		int[7], int[7], Vector4, Vector4) {}
#endif
}

bool ParaScripting::ParaGlobal::OpenFileDialog(const luabind::adl::object&) { return false; }
void ParaScripting::CNPLScriptingState::LoadHAPI_Platform() {}
