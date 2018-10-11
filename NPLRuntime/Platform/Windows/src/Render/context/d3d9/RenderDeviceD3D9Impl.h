#pragma once
#include "RenderDeviceD3D9.h"
namespace ParaEngine
{
	class RenderDeviceD3D9Impl : public RenderDeviceD3D9
	{
	public:	 
		RenderDeviceD3D9Impl() = default;
		virtual ~RenderDeviceD3D9Impl() override = default;


	public:
		virtual bool Reset(const RenderConfiguration& cfg) override;

	};
}