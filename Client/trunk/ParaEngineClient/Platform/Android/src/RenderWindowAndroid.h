#pragma once
#include "Framework/Interface/Render/IRenderWindow.h"

namespace ParaEngine
{
	class RenderWindowAndroid : public IRenderWindow
	{

	public:
		virtual unsigned int GetWidth() const override;
		virtual unsigned int GetHeight() const override;
	private:

	};
}