#pragma once
#include "ParaEngineRenderBase.h"

@class UIView;
namespace ParaEngine
{
	class RenderWindowiOS : public CParaEngineRenderBase
	{

	public:
		RenderWindowiOS(UIView* view);
		~RenderWindowiOS();
		virtual unsigned int GetWidth() const override;
		virtual unsigned int GetHeight() const override;
        virtual float GetScaleX() const override;
        virtual float GetScaleY() const override;
		virtual intptr_t GetNativeHandle() const override;
        UIView* GetView() const;

	private:
        UIView* m_view;
		
	};
}
