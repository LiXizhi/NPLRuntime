#include "Framework/Interface/Render/IRenderContext.h"
namespace ParaEngine
{
	class RenderContextNull : public IRenderContext
	{

	public:
		RenderContextNull() = default;
		~RenderContextNull() = default;
		virtual IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override;
		virtual bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override;

	};
}