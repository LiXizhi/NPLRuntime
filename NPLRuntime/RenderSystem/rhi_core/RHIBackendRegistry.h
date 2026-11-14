#pragma once
#include <vector>
#include "Framework/Interface/Render/RHITypes.h"
#include "Framework/Interface/Render/IRenderContext.h"

namespace ParaEngine
{
	using RHIBackendFactoryFn = IRenderContext* (*)();

	struct RHIBackendEntry
	{
		ERHIBackend backend = ERHIBackend::Null;
		const char* name = "Null";
		RHIBackendFactoryFn factory = nullptr;
		int priority = 0; // Higher priority is preferred when ERHIBackend::Auto is selected
	};

	/**
	 * Runtime registry & fallback dispatcher for ParaRHI backends.
	 * Supports --renderer=d3d11|d3d12|vulkan|metal|opengl|webgpu|null and PARA_RENDERER env var.
	 */
	class RHIBackendRegistry
	{
	public:
		static RHIBackendRegistry& GetInstance();

		void RegisterBackend(ERHIBackend backend, const char* name, RHIBackendFactoryFn factory, int priority);
		bool HasBackend(ERHIBackend backend) const;
		const std::vector<RHIBackendEntry>& GetEntries() const { return m_entries; }

		void SetPreferredBackend(ERHIBackend backend) { m_preferredBackend = backend; }
		ERHIBackend GetPreferredBackend() const { return m_preferredBackend; }

		/** Parses command line (--renderer=xxx or renderer="xxx") and PARA_RENDERER env var. */
		ERHIBackend ResolvePreferredBackend(const char* commandLine = nullptr) const;

		static ERHIBackend ParseBackendName(const char* name);
		static const char* GetBackendName(ERHIBackend backend);

		IRenderContext* CreateBackendContext(ERHIBackend backend = ERHIBackend::Auto);

	private:
		RHIBackendRegistry();
		std::vector<RHIBackendEntry> m_entries;
		ERHIBackend m_preferredBackend;
	};
}
