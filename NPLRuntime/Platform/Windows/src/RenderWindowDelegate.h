#pragma once
#include "RenderWindowWin32.h"
#include "windows.h"
namespace ParaEngine
{
	class RenderWindowDelegate : public RenderWindowWin32
	{
	public:
		RenderWindowDelegate(){};
	protected:
		virtual void OnMouseButton(EMouseButton button, EKeyState state,uint32_t x,uint32_t y) override;
		virtual void OnMouseMove(uint32_t x, uint32_t y) override;

		virtual void OnMouseWhell(float x, float y, float delta) override;


		virtual void OnKey(EVirtualKey key, EKeyState state) override;


		virtual void OnChar(unsigned int character) override;

		virtual void OnSize(int w, int h) override;

		virtual void OnDropFiles(const std::string& files) override;

	};
}