//----------------------------------------------------------------------
// Class: main app file
// Authors:	yuanquanwei
// Company: ParaEngine
// Date: 2018.3
//-----------------------------------------------------------------------
#include "ParaAppAndroid.h"
#include "jni/ParaEngineGLSurfaceView.h"
#include "3dengine/AudioEngine2.h"
using namespace  ParaEngine;

IParaEngineApp* CreateParaEngineApp()
{
	return new CParaEngineAppAndroid();
}

namespace ParaEngine {

	CParaEngineAppAndroid::CParaEngineAppAndroid() {
		SetTouchInputting(true);
	}

	void CParaEngineAppAndroid::GameToClient(int &inout_x, int &inout_y,
														 bool bInBackbuffer /*= true*/) {
		//throw std::logic_error("The method or operation is not implemented.");
	}

	void CParaEngineAppAndroid::ClientToGame(int &inout_x, int &inout_y,
														 bool bInBackbuffer /*= true*/) {
		//throw std::logic_error("The method or operation is not implemented.");
	}

	bool CParaEngineAppAndroid::AppHasFocus() {
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}

	void CParaEngineAppAndroid::GetStats(string &output, DWORD dwFields) {
		//throw std::logic_error("The method or operation is not implemented.");
	}

	bool CParaEngineAppAndroid::WriteRegStr(const string &root_key, const string &sSubKey,
												   const string &name, const string &value) {
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}

	void CParaEngineAppAndroid::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled) {
		//throw std::logic_error("The method or operation is not implemented.");
	}

	const char * CParaEngineAppAndroid::ReadRegStr(const string &root_key, const string &sSubKey,
												  const string &name) {
		//throw std::logic_error("The method or operation is not implemented.");
		return "";
	}

	bool CParaEngineAppAndroid::WriteRegDWORD(const string &root_key, const string &sSubKey,
													 const string &name, DWORD value) {
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}

	DWORD CParaEngineAppAndroid::ReadRegDWORD(const string &root_key, const string &sSubKey,
													const string &name) {
		//throw std::logic_error("The method or operation is not implemented.");
		return 0;
	}

	bool CParaEngineAppAndroid::GetAutoLowerFrameRateWhenNotFocused() {
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}

	void CParaEngineAppAndroid::SetToggleSoundWhenNotFocused(bool bEnabled) {
		//	throw std::logic_error("The method or operation is not implemented.");

	}

	bool CParaEngineAppAndroid::GetToggleSoundWhenNotFocused() {
		//throw std::logic_error("The method or operation is not implemented.");
		return true;
	}

	HRESULT CParaEngineAppAndroid::DoWork() {
		return CParaEngineAppBase::DoWork();
	}

	void CParaEngineAppAndroid::GetScreenResolution(Vector2 *pOut) {
		// should return full render window
		pOut->x = CGlobals::GetApp()->GetRenderWindow()->GetWidth();
		pOut->y = CGlobals::GetApp()->GetRenderWindow()->GetHeight();
	}

	void CParaEngineAppAndroid::setIMEKeyboardState(bool bOpen, bool bMoveView, int ctrlBottom) {

		int x = 0;
		this->GameToClient(x, ctrlBottom);
		ParaEngineGLSurfaceView::setIMEKeyboardState(bOpen, bMoveView, ctrlBottom);
	}

	void CParaEngineAppAndroid::OnPause()
	{
		CAudioEngine2::GetInstance()->PauseAll();

		CParaEngineAppBase::OnPause();
	}

	void CParaEngineAppAndroid::OnResume()
	{
		CAudioEngine2::GetInstance()->ResumeAll();

		CParaEngineAppBase::OnResume();
	}

} //namespace ParaEngine