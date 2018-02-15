#pragma once
#include <memory>
#include "Framework/RenderSystem/RenderTypes.h"
namespace ParaEngine
{
	class ISurface
	{
	public:
		ISurface() = default;
		virtual ~ISurface() = default;

		//virtual PixelFormat GetFormat() const = 0;
		//virtual uint32_t GetWidth() const  = 0;
		//virtual uint32_t GetHeight() const = 0;
		//virtual bool LockRect(LockedRect& ret, const Rect* rect, ELockOption option) = 0;
		//virtual bool UnlockRect() = 0;
		
	};
	using ISurfacePtr = std::shared_ptr<ISurface>;
}