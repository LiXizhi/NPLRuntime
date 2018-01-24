#pragma once
#include "Render/WindowsRenderWindow.h"
#include "windows.h"
namespace ParaEngine
{
	class ApplicationWindow : public WindowsRenderWindow
	{
	public:
		ApplicationWindow(HINSTANCE hInstance, int width, int height, bool windowed) :WindowsRenderWindow(hInstance, width, height, windowed) {};
	protected:
		virtual void OnMouseButton(EMouseButton button, EKeyState state) override;
		virtual void OnMouseMove(uint32_t x, uint32_t y) override;
	};
}