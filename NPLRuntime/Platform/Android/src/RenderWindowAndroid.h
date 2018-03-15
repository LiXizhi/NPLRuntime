#pragma once
#include "Framework/Interface/Render/IRenderWindow.h"
#include <android/native_activity.h>
namespace ParaEngine
{
	class RenderWindowAndroid : public IRenderWindow
	{

	public:
		RenderWindowAndroid(ANativeWindow* nativeWindow);
		~RenderWindowAndroid();
		virtual unsigned int GetWidth() const override;
		virtual unsigned int GetHeight() const override;
		ANativeWindow* GetNativeWindow() { return m_NativeWindow; }
		void SetNativeWindow(ANativeWindow* nativeWindow) { m_NativeWindow = nativeWindow; }
		virtual intptr_t GetNativeHandle() const override;

	private:
		ANativeWindow* m_NativeWindow;
	};
}