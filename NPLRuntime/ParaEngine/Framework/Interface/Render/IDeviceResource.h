#pragma once
#include "Framework/Common/RefCountedObject.hpp"
namespace IParaEngine
{
	class IDeviceResource : public ParaEngine::RefCountedObject
	{
	public:
		IDeviceResource() = default;
		virtual ~IDeviceResource() override = default;
	};
}