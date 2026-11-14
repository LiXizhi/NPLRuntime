#include "ParaEngine.h"
#include "RenderContextVulkan.h"
#include "RenderDeviceVulkan.h"

#include <cstring>
#include <string>
#include <vector>

namespace ParaEngine
{
	namespace
	{
		bool HasCmdFlagVk(const char* flag)
		{
#if defined(_WIN32)
			const char* cmd = ::GetCommandLineA();
			if (!cmd || !flag)
				return false;
			return std::strstr(cmd, flag) != nullptr;
#else
			(void)flag;
			return false;
#endif
		}

		bool HasExtensionName(const std::vector<VkExtensionProperties>& exts, const char* name)
		{
			for (const auto& e : exts)
			{
				if (std::strcmp(e.extensionName, name) == 0)
					return true;
			}
			return false;
		}

		bool HasLayerName(const std::vector<VkLayerProperties>& layers, const char* name)
		{
			for (const auto& l : layers)
			{
				if (std::strcmp(l.layerName, name) == 0)
					return true;
			}
			return false;
		}

		VkBool32 VKAPI_PTR VulkanDebugUtilsCallback(
			VkFlags messageSeverity,
			VkFlags /*messageTypes*/,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* /*pUserData*/)
		{
			if (!pCallbackData || !pCallbackData->pMessage)
				return VK_FALSE;

			const char* sev = (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "ERROR" : "WARN";
			const char* msgId = pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "VK";
			OUTPUT_LOG("VULKAN_VALIDATION [%s] (%s): %s\n", sev, msgId, pCallbackData->pMessage);
#if defined(_WIN32)
			std::string dbgLine = std::string("VULKAN_VALIDATION [") + sev + "] (" + msgId + "): " + pCallbackData->pMessage + "\n";
			::OutputDebugStringA(dbgLine.c_str());
#endif
			return VK_FALSE;
		}
	}

	IRenderContext* CreateRenderContextVulkan()
	{
		return new RenderContextVulkan();
	}

	IRenderDevice* RenderContextVulkan::CreateDevice(const RenderConfiguration& cfg)
	{
		if (!cfg.renderWindow)
		{
			OUTPUT_LOG("ERROR: RenderContextVulkan::CreateDevice called with null renderWindow\n");
			return nullptr;
		}

		VulkanDispatch vk;
		if (!vk.LoadGlobal())
		{
			OUTPUT_LOG("ERROR: RenderContextVulkan failed to load Vulkan loader (vulkan-1.dll / libvulkan)\n");
			return nullptr;
		}

		uint32_t instExtCount = 0;
		vk.vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, nullptr);
		std::vector<VkExtensionProperties> instExts(instExtCount);
		if (instExtCount > 0)
			vk.vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, instExts.data());

		std::vector<const char*> enabledInstExts;
		if (!HasExtensionName(instExts, "VK_KHR_surface"))
		{
			OUTPUT_LOG("ERROR: Vulkan instance missing VK_KHR_surface\n");
			return nullptr;
		}
		enabledInstExts.push_back("VK_KHR_surface");

#if defined(_WIN32)
		if (!HasExtensionName(instExts, "VK_KHR_win32_surface"))
		{
			OUTPUT_LOG("ERROR: Vulkan instance missing VK_KHR_win32_surface\n");
			return nullptr;
		}
		enabledInstExts.push_back("VK_KHR_win32_surface");
#endif

		const bool wantValidation = cfg.debugValidation || HasCmdFlagVk("--rhi-validation");
		bool hasDebugUtilsExt = false;
		std::vector<const char*> enabledLayers;
		if (wantValidation)
		{
			uint32_t layerCount = 0;
			vk.vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
			std::vector<VkLayerProperties> layers(layerCount);
			if (layerCount > 0)
				vk.vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
			if (HasLayerName(layers, "VK_LAYER_KHRONOS_validation"))
			{
				enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
				// Also check extensions exposed by VK_LAYER_KHRONOS_validation itself
				uint32_t layerExtCount = 0;
				vk.vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerExtCount, nullptr);
				std::vector<VkExtensionProperties> layerExts(layerExtCount);
				if (layerExtCount > 0)
					vk.vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerExtCount, layerExts.data());
				if (HasExtensionName(instExts, "VK_EXT_debug_utils") || HasExtensionName(layerExts, "VK_EXT_debug_utils"))
				{
					enabledInstExts.push_back("VK_EXT_debug_utils");
					hasDebugUtilsExt = true;
				}
			}
		}

		VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo = {};
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		dbgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		dbgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		dbgCreateInfo.pfnUserCallback = &VulkanDebugUtilsCallback;

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "ParaEngine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "ParaRHI";
		appInfo.engineVersion = VK_MAKE_VERSION(2, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo instInfo = {};
		instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instInfo.pNext = hasDebugUtilsExt ? &dbgCreateInfo : nullptr;
		instInfo.pApplicationInfo = &appInfo;
		instInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
		instInfo.ppEnabledLayerNames = enabledLayers.empty() ? nullptr : enabledLayers.data();
		instInfo.enabledExtensionCount = static_cast<uint32_t>(enabledInstExts.size());
		instInfo.ppEnabledExtensionNames = enabledInstExts.data();

		VkInstance instance = VK_NULL_HANDLE;
		VkResult res = vk.vkCreateInstance(&instInfo, nullptr, &instance);
		if (res != VK_SUCCESS)
		{
			// Fallback to Vulkan 1.0 instance if loader is strictly 1.0
			appInfo.apiVersion = VK_API_VERSION_1_0;
			instInfo.pNext = nullptr;
			hasDebugUtilsExt = false;
			if (!enabledInstExts.empty() && std::strcmp(enabledInstExts.back(), "VK_EXT_debug_utils") == 0)
				enabledInstExts.pop_back();
			instInfo.enabledExtensionCount = static_cast<uint32_t>(enabledInstExts.size());
			instInfo.enabledLayerCount = 0;
			instInfo.ppEnabledLayerNames = nullptr;
			res = vk.vkCreateInstance(&instInfo, nullptr, &instance);
		}
		if (res != VK_SUCCESS || !instance)
		{
			OUTPUT_LOG("ERROR: vkCreateInstance failed (VkResult=%d)\n", static_cast<int>(res));
			return nullptr;
		}

		if (!vk.LoadInstance(instance))
		{
			OUTPUT_LOG("ERROR: VulkanDispatch::LoadInstance failed\n");
			if (vk.vkDestroyInstance)
				vk.vkDestroyInstance(instance, nullptr);
			return nullptr;
		}

		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
		if (hasDebugUtilsExt && vk.vkCreateDebugUtilsMessengerEXT)
		{
			vk.vkCreateDebugUtilsMessengerEXT(instance, &dbgCreateInfo, nullptr, &debugMessenger);
		}

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		auto cleanupInstance = [&]()
		{
			if (surface != VK_NULL_HANDLE && vk.vkDestroySurfaceKHR)
				vk.vkDestroySurfaceKHR(instance, surface, nullptr);
			if (debugMessenger != VK_NULL_HANDLE && vk.vkDestroyDebugUtilsMessengerEXT)
				vk.vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
			if (instance != VK_NULL_HANDLE && vk.vkDestroyInstance)
				vk.vkDestroyInstance(instance, nullptr);
		};
#if defined(_WIN32)
		HWND hWnd = reinterpret_cast<HWND>(cfg.renderWindow->GetNativeHandle());
		if (!hWnd)
		{
			OUTPUT_LOG("ERROR: RenderContextVulkan got null HWND\n");
			cleanupInstance();
			return nullptr;
		}
		VkWin32SurfaceCreateInfoKHR surfInfo = {};
		surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfInfo.hinstance = ::GetModuleHandleW(nullptr);
		surfInfo.hwnd = hWnd;
		res = vk.vkCreateWin32SurfaceKHR(instance, &surfInfo, nullptr, &surface);
		if (res != VK_SUCCESS || !surface)
		{
			OUTPUT_LOG("ERROR: vkCreateWin32SurfaceKHR failed (VkResult=%d)\n", static_cast<int>(res));
			cleanupInstance();
			return nullptr;
		}
#else
		OUTPUT_LOG("ERROR: WSI surface creation not yet configured for this platform in RenderContextVulkan\n");
		cleanupInstance();
		return nullptr;
#endif

		uint32_t gpuCount = 0;
		vk.vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
		if (gpuCount == 0)
		{
			OUTPUT_LOG("ERROR: vkEnumeratePhysicalDevices returned 0 GPUs\n");
			cleanupInstance();
			return nullptr;
		}
		std::vector<VkPhysicalDevice> gpus(gpuCount);
		vk.vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());

		VkPhysicalDevice bestGpu = VK_NULL_HANDLE;
		uint32_t bestQueueFamily = UINT32_MAX;
		int bestScore = -1;
		VkPhysicalDeviceProperties bestProps = {};
		VkPhysicalDeviceFeatures bestAvailFeatures = {};
		VkPhysicalDeviceMemoryProperties bestMemProps = {};

		for (VkPhysicalDevice gpu : gpus)
		{
			uint32_t devExtCount = 0;
			vk.vkEnumerateDeviceExtensionProperties(gpu, nullptr, &devExtCount, nullptr);
			std::vector<VkExtensionProperties> devExts(devExtCount);
			if (devExtCount > 0)
				vk.vkEnumerateDeviceExtensionProperties(gpu, nullptr, &devExtCount, devExts.data());
			if (!HasExtensionName(devExts, "VK_KHR_swapchain"))
				continue;

			uint32_t qfCount = 0;
			vk.vkGetPhysicalDeviceQueueFamilyProperties(gpu, &qfCount, nullptr);
			std::vector<VkQueueFamilyProperties> qfs(qfCount);
			if (qfCount > 0)
				vk.vkGetPhysicalDeviceQueueFamilyProperties(gpu, &qfCount, qfs.data());

			uint32_t foundQF = UINT32_MAX;
			for (uint32_t q = 0; q < qfCount; ++q)
			{
				if ((qfs[q].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 || qfs[q].queueCount == 0)
					continue;
				VkBool32 presentSupported = VK_FALSE;
				if (vk.vkGetPhysicalDeviceSurfaceSupportKHR(gpu, q, surface, &presentSupported) == VK_SUCCESS && presentSupported)
				{
					foundQF = q;
					break;
				}
			}
			if (foundQF == UINT32_MAX)
				continue;

			VkPhysicalDeviceProperties props = {};
			VkPhysicalDeviceFeatures feats = {};
			VkPhysicalDeviceMemoryProperties mem = {};
			vk.vkGetPhysicalDeviceProperties(gpu, &props);
			vk.vkGetPhysicalDeviceFeatures(gpu, &feats);
			vk.vkGetPhysicalDeviceMemoryProperties(gpu, &mem);

			if (!cfg.allowSoftwareFallback && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
				continue;

			int score = 100;
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 10000;
			else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 2000;
			else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) score += 500;

			if (score > bestScore)
			{
				bestScore = score;
				bestGpu = gpu;
				bestQueueFamily = foundQF;
				bestProps = props;
				bestAvailFeatures = feats;
				bestMemProps = mem;
			}
		}

		if (!bestGpu || bestQueueFamily == UINT32_MAX)
		{
			OUTPUT_LOG("ERROR: No suitable Vulkan physical device with graphics+present+swapchain found\n");
			cleanupInstance();
			return nullptr;
		}

		VkPhysicalDeviceFeatures enabledFeatures = {};
		if (bestAvailFeatures.samplerAnisotropy) enabledFeatures.samplerAnisotropy = VK_TRUE;
		if (bestAvailFeatures.fillModeNonSolid) enabledFeatures.fillModeNonSolid = VK_TRUE;
		if (bestAvailFeatures.independentBlend) enabledFeatures.independentBlend = VK_TRUE;
		if (bestAvailFeatures.textureCompressionBC) enabledFeatures.textureCompressionBC = VK_TRUE;
		if (bestAvailFeatures.occlusionQueryPrecise) enabledFeatures.occlusionQueryPrecise = VK_TRUE;
		if (bestAvailFeatures.depthClamp) enabledFeatures.depthClamp = VK_TRUE;
		if (bestAvailFeatures.fullDrawIndexUint32) enabledFeatures.fullDrawIndexUint32 = VK_TRUE;

		const float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo qInfo = {};
		qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qInfo.queueFamilyIndex = bestQueueFamily;
		qInfo.queueCount = 1;
		qInfo.pQueuePriorities = &queuePriority;

		const char* devExtNames[] = { "VK_KHR_swapchain" };
		VkDeviceCreateInfo devInfo = {};
		devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		devInfo.queueCreateInfoCount = 1;
		devInfo.pQueueCreateInfos = &qInfo;
		devInfo.enabledExtensionCount = 1;
		devInfo.ppEnabledExtensionNames = devExtNames;
		devInfo.pEnabledFeatures = &enabledFeatures;

		VkDevice device = VK_NULL_HANDLE;
		res = vk.vkCreateDevice(bestGpu, &devInfo, nullptr, &device);
		if (res != VK_SUCCESS || !device)
		{
			OUTPUT_LOG("ERROR: vkCreateDevice failed (VkResult=%d)\n", static_cast<int>(res));
			cleanupInstance();
			return nullptr;
		}

		if (!vk.LoadDevice(device))
		{
			OUTPUT_LOG("ERROR: VulkanDispatch::LoadDevice failed\n");
			if (vk.vkDestroyDevice)
				vk.vkDestroyDevice(device, nullptr);
			cleanupInstance();
			return nullptr;
		}

		VkQueue graphicsQueue = VK_NULL_HANDLE;
		vk.vkGetDeviceQueue(device, bestQueueFamily, 0, &graphicsQueue);

		uint32_t width = cfg.screenWidth > 0 ? cfg.screenWidth : cfg.renderWindow->GetWidth();
		uint32_t height = cfg.screenHeight > 0 ? cfg.screenHeight : cfg.renderWindow->GetHeight();
		if (width == 0) width = 1024;
		if (height == 0) height = 768;

		std::string adapterName = bestProps.deviceName[0] ? bestProps.deviceName : "Vulkan Device";
		uint32_t verMaj = (bestProps.apiVersion >> 22) & 0x3FFu;
		uint32_t verMin = (bestProps.apiVersion >> 12) & 0x3FFu;
		uint32_t verPat = bestProps.apiVersion & 0xFFFu;
		OUTPUT_LOG(
			"ParaRHI Vulkan initialized: Adapter='%s', API=%u.%u.%u, Size=%ux%u\n",
			adapterName.c_str(),
			verMaj, verMin, verPat,
			width, height);

		auto* renderDevice = new RenderDeviceVulkan(
			vk,
			instance,
			debugMessenger,
			surface,
			bestGpu,
			device,
			graphicsQueue,
			bestQueueFamily,
			bestProps,
			enabledFeatures,
			bestMemProps,
			adapterName);

		if (!renderDevice->Initialize(width, height))
		{
			OUTPUT_LOG("ERROR: RenderDeviceVulkan::Initialize failed\n");
			delete renderDevice;
			return nullptr;
		}

		return renderDevice;
	}

	bool RenderContextVulkan::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
	{
		if (!device)
			return false;
		auto* vkDev = static_cast<RenderDeviceVulkan*>(device);
		uint32_t width = cfg.screenWidth > 0 ? cfg.screenWidth : (cfg.renderWindow ? cfg.renderWindow->GetWidth() : vkDev->GetBackBufferWidth());
		uint32_t height = cfg.screenHeight > 0 ? cfg.screenHeight : (cfg.renderWindow ? cfg.renderWindow->GetHeight() : vkDev->GetBackBufferHeight());
		return vkDev->ResizeSwapChain(width, height, cfg.isWindowed);
	}
}
