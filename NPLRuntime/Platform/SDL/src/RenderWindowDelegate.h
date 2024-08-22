#pragma once
#include "RenderWindowSDL2.h"

namespace ParaEngine
{
	class RenderWindowDelegate : public RenderWindowSDL2
	{
	public:
		RenderWindowDelegate() {};

		virtual void OnTouch(int nType, TouchEvent::TouchEventMsgType nTouchType, int touch_id, float x, float y, int nTimeTick) override;

		virtual void OnMouseButton(EMouseButton button, EKeyState state, uint32_t x, uint32_t y) override;
		virtual void OnMouseMove(uint32_t x, uint32_t y) override;

		virtual void OnMouseWheel(float x, float y, float delta) override;

		void PollEvents();

		virtual void OnKey(EVirtualKey key, EKeyState state) override;

		virtual void OnChar(unsigned int character) override;
		virtual void OnChar(std::string text) override;
		virtual void OnClearChar(std::string text);

		virtual void OnSize(int w, int h) override;

		virtual void OnDropFiles(const std::string& files) override;

	};
}
