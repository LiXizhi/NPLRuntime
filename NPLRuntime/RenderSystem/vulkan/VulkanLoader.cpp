#include "ParaEngine.h"
#include "VulkanLoader.h"

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

namespace ParaEngine
{
	namespace
	{
		PFN_vkGetInstanceProcAddr LoadVulkanLibraryAndEntry()
		{
			static PFN_vkGetInstanceProcAddr s_proc = nullptr;
			static bool s_attempted = false;
			if (s_attempted)
				return s_proc;
			s_attempted = true;

#if defined(_WIN32)
			static HMODULE s_module = ::LoadLibraryA("vulkan-1.dll");
			if (s_module)
			{
				s_proc = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
					::GetProcAddress(s_module, "vkGetInstanceProcAddr"));
			}
#elif defined(__APPLE__)
			const char* candidates[] = {
				"libvulkan.1.dylib",
				"libvulkan.dylib",
				"libMoltenVK.dylib"
			};
			for (const char* name : candidates)
			{
				void* lib = ::dlopen(name, RTLD_NOW | RTLD_LOCAL);
				if (lib)
				{
					s_proc = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
						::dlsym(lib, "vkGetInstanceProcAddr"));
					if (s_proc)
						break;
				}
			}
#else
			const char* candidates[] = {
				"libvulkan.so.1",
				"libvulkan.so"
			};
			for (const char* name : candidates)
			{
				void* lib = ::dlopen(name, RTLD_NOW | RTLD_LOCAL);
				if (lib)
				{
					s_proc = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
						::dlsym(lib, "vkGetInstanceProcAddr"));
					if (s_proc)
						break;
				}
			}
#endif
			return s_proc;
		}
	}

	bool VulkanDispatch::LoadGlobal()
	{
		vkGetInstanceProcAddr = LoadVulkanLibraryAndEntry();
		if (!vkGetInstanceProcAddr)
			return false;

		#define PARA_VK_GLOBAL(fn) \
			fn = reinterpret_cast<decltype(fn)>(vkGetInstanceProcAddr(VK_NULL_HANDLE, #fn)); \
			if (!fn) return false

		PARA_VK_GLOBAL(vkEnumerateInstanceExtensionProperties);
		PARA_VK_GLOBAL(vkEnumerateInstanceLayerProperties);
		PARA_VK_GLOBAL(vkCreateInstance);

		#undef PARA_VK_GLOBAL
		return true;
	}

	bool VulkanDispatch::LoadInstance(VkInstance instance)
	{
		if (!vkGetInstanceProcAddr || !instance)
			return false;

		#define PARA_VK_INST(fn) \
			fn = reinterpret_cast<decltype(fn)>(vkGetInstanceProcAddr(instance, #fn)); \
			if (!fn) return false

		PARA_VK_INST(vkDestroyInstance);
		PARA_VK_INST(vkEnumeratePhysicalDevices);
		PARA_VK_INST(vkGetPhysicalDeviceProperties);
		PARA_VK_INST(vkGetPhysicalDeviceFeatures);
		PARA_VK_INST(vkGetPhysicalDeviceFormatProperties);
		PARA_VK_INST(vkGetPhysicalDeviceMemoryProperties);
		PARA_VK_INST(vkGetPhysicalDeviceQueueFamilyProperties);
		PARA_VK_INST(vkEnumerateDeviceExtensionProperties);
		PARA_VK_INST(vkCreateDevice);
		PARA_VK_INST(vkGetDeviceProcAddr);

		PARA_VK_INST(vkDestroySurfaceKHR);
		PARA_VK_INST(vkGetPhysicalDeviceSurfaceSupportKHR);
		PARA_VK_INST(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
		PARA_VK_INST(vkGetPhysicalDeviceSurfaceFormatsKHR);
		PARA_VK_INST(vkGetPhysicalDeviceSurfacePresentModesKHR);
#if defined(_WIN32)
		PARA_VK_INST(vkCreateWin32SurfaceKHR);
#endif
		vkCreateDebugUtilsMessengerEXT = reinterpret_cast<decltype(vkCreateDebugUtilsMessengerEXT)>(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<decltype(vkDestroyDebugUtilsMessengerEXT)>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

		#undef PARA_VK_INST
		return true;
	}

	bool VulkanDispatch::LoadDevice(VkDevice device)
	{
		if (!vkGetDeviceProcAddr || !device)
			return false;

		#define PARA_VK_DEV(fn) \
			fn = reinterpret_cast<decltype(fn)>(vkGetDeviceProcAddr(device, #fn)); \
			if (!fn) return false

		PARA_VK_DEV(vkDestroyDevice);
		PARA_VK_DEV(vkGetDeviceQueue);
		PARA_VK_DEV(vkDeviceWaitIdle);
		PARA_VK_DEV(vkQueueSubmit);
		PARA_VK_DEV(vkQueueWaitIdle);

		PARA_VK_DEV(vkAllocateMemory);
		PARA_VK_DEV(vkFreeMemory);
		PARA_VK_DEV(vkMapMemory);
		PARA_VK_DEV(vkUnmapMemory);

		PARA_VK_DEV(vkCreateBuffer);
		PARA_VK_DEV(vkDestroyBuffer);
		PARA_VK_DEV(vkGetBufferMemoryRequirements);
		PARA_VK_DEV(vkBindBufferMemory);

		PARA_VK_DEV(vkCreateImage);
		PARA_VK_DEV(vkDestroyImage);
		PARA_VK_DEV(vkGetImageMemoryRequirements);
		PARA_VK_DEV(vkBindImageMemory);

		PARA_VK_DEV(vkCreateImageView);
		PARA_VK_DEV(vkDestroyImageView);

		PARA_VK_DEV(vkCreateShaderModule);
		PARA_VK_DEV(vkDestroyShaderModule);

		PARA_VK_DEV(vkCreateSampler);
		PARA_VK_DEV(vkDestroySampler);

		PARA_VK_DEV(vkCreateDescriptorSetLayout);
		PARA_VK_DEV(vkDestroyDescriptorSetLayout);
		PARA_VK_DEV(vkCreateDescriptorPool);
		PARA_VK_DEV(vkDestroyDescriptorPool);
		PARA_VK_DEV(vkResetDescriptorPool);
		PARA_VK_DEV(vkAllocateDescriptorSets);
		PARA_VK_DEV(vkUpdateDescriptorSets);

		PARA_VK_DEV(vkCreatePipelineLayout);
		PARA_VK_DEV(vkDestroyPipelineLayout);
		PARA_VK_DEV(vkCreateRenderPass);
		PARA_VK_DEV(vkDestroyRenderPass);
		PARA_VK_DEV(vkCreateFramebuffer);
		PARA_VK_DEV(vkDestroyFramebuffer);
		PARA_VK_DEV(vkCreateGraphicsPipelines);
		PARA_VK_DEV(vkDestroyPipeline);

		PARA_VK_DEV(vkCreateCommandPool);
		PARA_VK_DEV(vkDestroyCommandPool);
		PARA_VK_DEV(vkResetCommandPool);
		PARA_VK_DEV(vkAllocateCommandBuffers);
		PARA_VK_DEV(vkFreeCommandBuffers);

		PARA_VK_DEV(vkBeginCommandBuffer);
		PARA_VK_DEV(vkEndCommandBuffer);
		PARA_VK_DEV(vkResetCommandBuffer);

		PARA_VK_DEV(vkCmdBeginRenderPass);
		PARA_VK_DEV(vkCmdEndRenderPass);
		PARA_VK_DEV(vkCmdBindPipeline);
		PARA_VK_DEV(vkCmdSetViewport);
		PARA_VK_DEV(vkCmdSetScissor);
		PARA_VK_DEV(vkCmdSetBlendConstants);
		PARA_VK_DEV(vkCmdSetStencilReference);
		PARA_VK_DEV(vkCmdBindDescriptorSets);
		PARA_VK_DEV(vkCmdBindIndexBuffer);
		PARA_VK_DEV(vkCmdBindVertexBuffers);
		PARA_VK_DEV(vkCmdDraw);
		PARA_VK_DEV(vkCmdDrawIndexed);
		PARA_VK_DEV(vkCmdClearAttachments);

		PARA_VK_DEV(vkCmdCopyBuffer);
		PARA_VK_DEV(vkCmdCopyImage);
		PARA_VK_DEV(vkCmdBlitImage);
		PARA_VK_DEV(vkCmdCopyBufferToImage);
		PARA_VK_DEV(vkCmdCopyImageToBuffer);
		PARA_VK_DEV(vkCmdPipelineBarrier);

		PARA_VK_DEV(vkCreateFence);
		PARA_VK_DEV(vkDestroyFence);
		PARA_VK_DEV(vkResetFences);
		PARA_VK_DEV(vkGetFenceStatus);
		PARA_VK_DEV(vkWaitForFences);

		PARA_VK_DEV(vkCreateSemaphore);
		PARA_VK_DEV(vkDestroySemaphore);

		PARA_VK_DEV(vkCreateQueryPool);
		PARA_VK_DEV(vkDestroyQueryPool);
		PARA_VK_DEV(vkGetQueryPoolResults);
		PARA_VK_DEV(vkCmdResetQueryPool);
		PARA_VK_DEV(vkCmdBeginQuery);
		PARA_VK_DEV(vkCmdEndQuery);

		PARA_VK_DEV(vkCreateSwapchainKHR);
		PARA_VK_DEV(vkDestroySwapchainKHR);
		PARA_VK_DEV(vkGetSwapchainImagesKHR);
		PARA_VK_DEV(vkAcquireNextImageKHR);
		PARA_VK_DEV(vkQueuePresentKHR);

		#undef PARA_VK_DEV
		return true;
	}
}
