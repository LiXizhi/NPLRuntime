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

		virtual void OnMouseWhell(float x, float y, float delta) override;


		virtual void OnKey(EVirtualKey key, EKeyState state) override;


		virtual void OnChar(char character) override;

	};
}