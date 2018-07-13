#pragma once
#include "Framework/Interface/Render/IDeviceResource.h"
namespace IParaEngine
{
	class IRenderTarget : public IDeviceResource
	{
	public:
		IRenderTarget() = default;
		virtual ~IRenderTarget() override = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
	};
}