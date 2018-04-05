#pragma once
#include "RenderWindowWin32.h"
#include "windows.h"
namespace ParaEngine
{
	class RenderWindowDelegate : public RenderWindowWin32
	{
	public:
		RenderWindowDelegate(HINSTANCE hInstance, int width, int height) :RenderWindowWin32(hInstance, width, height) {};
	protected:
		virtual void OnMouseButton(EMouseButton button, EKeyState state,uint32_t x,uint32_t y) override;
		virtual void OnMouseMove(uint32_t x, uint32_t y) override;

		virtual void OnMouseWhell(float x, float y, float delta) override;


		virtual void OnKey(EVirtualKey key, EKeyState state) override;


		virtual void OnChar(unsigned int character) override;

	};
}