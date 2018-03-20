#pragma once
#include "ParaEngineAppBase.h"
namespace ParaEngine
{
	class CWindowsApplication : public CParaEngineAppBase
	{
	public:


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

	};
}