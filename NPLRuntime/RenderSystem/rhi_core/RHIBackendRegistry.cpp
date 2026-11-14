#include "RHIBackendRegistry.h"
#include "Framework/Interface/Render/IRenderDevice.h"
#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace ParaEngine
{
	// Always available from RenderSystemNull
	IRenderContext* CreateRenderContextNull();

	namespace
	{
		bool EqualsIgnoreCase(const std::string& a, const char* b)
		{
			if (!b) return false;
			size_t len = strlen(b);
			if (a.size() != len) return false;
			for (size_t i = 0; i < len; ++i)
			{
				if (tolower(static_cast<unsigned char>(a[i])) != tolower(static_cast<unsigned char>(b[i])))
					return false;
			}
			return true;
		}

		std::string ExtractRendererArg(const char* cmdLine)
		{
			if (!cmdLine || !cmdLine[0])
				return std::string();

			std::string s(cmdLine);
			const char* keys[] = { "--renderer=", "-renderer=", "renderer=" };
			for (const char* key : keys)
			{
				size_t pos = s.find(key);
				if (pos != std::string::npos)
				{
					size_t valStart = pos + strlen(key);
					if (valStart < s.size() && (s[valStart] == '"' || s[valStart] == '\''))
					{
						char quote = s[valStart++];
						size_t valEnd = s.find(quote, valStart);
						return (valEnd != std::string::npos) ? s.substr(valStart, valEnd - valStart) : s.substr(valStart);
					}
					size_t valEnd = valStart;
					while (valEnd < s.size() && !isspace(static_cast<unsigned char>(s[valEnd])))
						++valEnd;
					return s.substr(valStart, valEnd - valStart);
				}
			}
			return std::string();
		}

		/**
		 * Multi-backend delegating context that creates the context + device in priority order,
		 * automatically falling back to lower-priority backends (down to Null) if creation fails.
		 */
		class RHIDispatchRenderContext : public IRenderContext
		{
		public:
			explicit RHIDispatchRenderContext(ERHIBackend requested)
				: m_requested(requested)
				, m_activeBackend(ERHIBackend::Auto)
				, m_activeContext(nullptr)
			{
			}

			~RHIDispatchRenderContext() override
			{
				delete m_activeContext;
				m_activeContext = nullptr;
			}

			ERHIBackend GetBackendType() const override
			{
				return m_activeContext ? m_activeBackend : m_requested;
			}

			IRenderDevice* CreateDevice(const RenderConfiguration& cfg) override
			{
				auto& reg = RHIBackendRegistry::GetInstance();
				ERHIBackend target = (cfg.preferredBackend != ERHIBackend::Auto) ? cfg.preferredBackend : m_requested;
				if (target == ERHIBackend::Auto)
					target = reg.ResolvePreferredBackend(nullptr);

				const auto& entries = reg.GetEntries();

				auto tryEntry = [&](const RHIBackendEntry& entry) -> IRenderDevice*
				{
					if (!entry.factory)
						return nullptr;
					IRenderContext* ctx = entry.factory();
					if (!ctx)
						return nullptr;
					IRenderDevice* dev = ctx->CreateDevice(cfg);
					if (!dev)
					{
						delete ctx;
						return nullptr;
					}
					delete m_activeContext;
					m_activeContext = ctx;
					m_activeBackend = entry.backend;
					return dev;
				};

				// 1. Try explicit target backend first if specified
				if (target != ERHIBackend::Auto)
				{
					for (const auto& e : entries)
					{
						if (e.backend == target)
						{
							if (IRenderDevice* dev = tryEntry(e))
								return dev;
							break;
						}
					}
					if (!cfg.allowSoftwareFallback)
						return nullptr;
				}

				// 2. Try all registered backends in descending priority order
				for (const auto& e : entries)
				{
					if (e.backend == target)
						continue; // already tried
					if (IRenderDevice* dev = tryEntry(e))
						return dev;
				}

				return nullptr;
			}

			bool ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg) override
			{
				return m_activeContext ? m_activeContext->ResetDevice(device, cfg) : true;
			}

		private:
			ERHIBackend m_requested;
			ERHIBackend m_activeBackend;
			IRenderContext* m_activeContext;
		};
	}

	RHIBackendRegistry& RHIBackendRegistry::GetInstance()
	{
		static RHIBackendRegistry s_instance;
		return s_instance;
	}

	RHIBackendRegistry::RHIBackendRegistry()
		: m_preferredBackend(ERHIBackend::Auto)
	{
		RegisterBackend(ERHIBackend::Null, "Null", &CreateRenderContextNull, -100);
	}

	void RHIBackendRegistry::RegisterBackend(ERHIBackend backend, const char* name, RHIBackendFactoryFn factory, int priority)
	{
		if (!factory)
			return;

		for (auto& e : m_entries)
		{
			if (e.backend == backend)
			{
				e.name = name;
				e.factory = factory;
				e.priority = priority;
				std::sort(m_entries.begin(), m_entries.end(),
					[](const RHIBackendEntry& a, const RHIBackendEntry& b) { return a.priority > b.priority; });
				return;
			}
		}

		RHIBackendEntry entry;
		entry.backend = backend;
		entry.name = name;
		entry.factory = factory;
		entry.priority = priority;
		m_entries.push_back(entry);

		std::sort(m_entries.begin(), m_entries.end(),
			[](const RHIBackendEntry& a, const RHIBackendEntry& b) { return a.priority > b.priority; });
	}

	bool RHIBackendRegistry::HasBackend(ERHIBackend backend) const
	{
		for (const auto& e : m_entries)
		{
			if (e.backend == backend && e.factory != nullptr)
				return true;
		}
		return false;
	}

	ERHIBackend RHIBackendRegistry::ParseBackendName(const char* name)
	{
		if (!name || !name[0])
			return ERHIBackend::Auto;
		std::string s(name);
		if (EqualsIgnoreCase(s, "auto"))
			return ERHIBackend::Auto;
		if (EqualsIgnoreCase(s, "d3d11") || EqualsIgnoreCase(s, "dx11") || EqualsIgnoreCase(s, "directx11") || EqualsIgnoreCase(s, "directx"))
			return ERHIBackend::D3D11;
		if (EqualsIgnoreCase(s, "d3d12") || EqualsIgnoreCase(s, "dx12") || EqualsIgnoreCase(s, "directx12"))
			return ERHIBackend::D3D12;
		if (EqualsIgnoreCase(s, "vulkan") || EqualsIgnoreCase(s, "vk"))
			return ERHIBackend::Vulkan;
		if (EqualsIgnoreCase(s, "metal") || EqualsIgnoreCase(s, "mtl"))
			return ERHIBackend::Metal;
		if (EqualsIgnoreCase(s, "opengl") || EqualsIgnoreCase(s, "gl") || EqualsIgnoreCase(s, "gles") || EqualsIgnoreCase(s, "webgl"))
			return ERHIBackend::OpenGL;
		if (EqualsIgnoreCase(s, "webgpu") || EqualsIgnoreCase(s, "wgpu"))
			return ERHIBackend::WebGPU;
		if (EqualsIgnoreCase(s, "null") || EqualsIgnoreCase(s, "headless") || EqualsIgnoreCase(s, "none"))
			return ERHIBackend::Null;
		return ERHIBackend::Auto;
	}

	const char* RHIBackendRegistry::GetBackendName(ERHIBackend backend)
	{
		switch (backend)
		{
		case ERHIBackend::Auto:   return "Auto";
		case ERHIBackend::D3D11:  return "D3D11";
		case ERHIBackend::D3D12:  return "D3D12";
		case ERHIBackend::Vulkan: return "Vulkan";
		case ERHIBackend::Metal:  return "Metal";
		case ERHIBackend::OpenGL: return "OpenGL";
		case ERHIBackend::WebGPU: return "WebGPU";
		case ERHIBackend::Null:   return "Null";
		default:                  return "Unknown";
		}
	}

	ERHIBackend RHIBackendRegistry::ResolvePreferredBackend(const char* commandLine) const
	{
		if (commandLine && commandLine[0])
		{
			std::string arg = ExtractRendererArg(commandLine);
			if (!arg.empty())
			{
				ERHIBackend parsed = ParseBackendName(arg.c_str());
				if (parsed != ERHIBackend::Auto)
					return parsed;
			}
		}

		if (m_preferredBackend != ERHIBackend::Auto)
			return m_preferredBackend;

		const char* envVal = getenv("PARA_RENDERER");
		if (envVal && envVal[0])
		{
			ERHIBackend parsed = ParseBackendName(envVal);
			if (parsed != ERHIBackend::Auto)
				return parsed;
		}

		return ERHIBackend::Auto;
	}

	IRenderContext* RHIBackendRegistry::CreateBackendContext(ERHIBackend backend)
	{
		return new RHIDispatchRenderContext(backend);
	}

#ifdef PARA_RHI_MULTI_BACKEND
	IRenderContext* IRenderContext::Create()
	{
		return RHIBackendRegistry::GetInstance().CreateBackendContext(ERHIBackend::Auto);
	}

	IRenderContext* IRenderContext::Create(ERHIBackend backend)
	{
		return RHIBackendRegistry::GetInstance().CreateBackendContext(backend);
	}
#endif
}
