//-----------------------------------------------------------------------------
// RenderWindowAndroid.h
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#pragma once

#include "ParaEngineRenderBase.h"
#include <android/native_activity.h>

namespace ParaEngine
{
	class RenderWindowAndroid : public CParaEngineRenderBase
	{
	public:
		RenderWindowAndroid(int w, int h);
		virtual ~RenderWindowAndroid();

		virtual unsigned int GetWidth() const override;
		virtual unsigned int GetHeight() const override;
		virtual intptr_t GetNativeHandle() const override;

	private:
		int m_width;
		int m_height;
	};
}