#pragma once
// Self-contained minimal Vulkan 1.0/1.1 + WSI C ABI declarations & dynamic loader.
// Matches Khronos vulkan_core.h / vulkan_win32.h ABI byte-for-byte with zero external SDK dependency.

#include <cstddef>
#include <cstdint>

#if defined(_WIN32)
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
	#define NOMINMAX
	#endif
	#include <windows.h>
	#define VKAPI_ATTR
	#define VKAPI_CALL __stdcall
	#define VKAPI_PTR  VKAPI_CALL
#else
	#define VKAPI_ATTR
	#define VKAPI_CALL
	#define VKAPI_PTR
#endif

#define VK_MAKE_VERSION(major, minor, patch) \
	((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))
#define VK_API_VERSION_1_0 VK_MAKE_VERSION(1, 0, 0)
#define VK_API_VERSION_1_1 VK_MAKE_VERSION(1, 1, 0)

#define VK_TRUE  1U
#define VK_FALSE 0U
#define VK_LOD_CLAMP_NONE 1000.0f
#define VK_REMAINING_MIP_LEVELS   (~0U)
#define VK_REMAINING_ARRAY_LAYERS (~0U)
#define VK_WHOLE_SIZE             (~0ULL)
#define VK_SUBPASS_EXTERNAL       (~0U)
#define VK_QUEUE_FAMILY_IGNORED   (~0U)
#define VK_MAX_PHYSICAL_DEVICE_NAME_SIZE 256U
#define VK_UUID_SIZE              16U
#define VK_MAX_MEMORY_TYPES       32U
#define VK_MAX_MEMORY_HEAPS       16U
#define VK_MAX_EXTENSION_NAME_SIZE 256U
#define VK_MAX_DESCRIPTION_SIZE   256U

using VkFlags = uint32_t;
using VkBool32 = uint32_t;
using VkDeviceSize = uint64_t;
using VkSampleMask = uint32_t;

// Dispatchable handles (pointer-sized on all platforms)
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
// Non-dispatchable handles (always 64-bit integer even on 32-bit platforms)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;

#define VK_NULL_HANDLE 0

VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_HANDLE(VkPhysicalDevice)
VK_DEFINE_HANDLE(VkDevice)
VK_DEFINE_HANDLE(VkQueue)
VK_DEFINE_HANDLE(VkCommandBuffer)

VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSemaphore)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFence)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDeviceMemory)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkEvent)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkQueryPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBufferView)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImageView)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderModule)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineCache)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineLayout)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkRenderPass)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipeline)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSetLayout)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSampler)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSet)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFramebuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkCommandPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSwapchainKHR)

enum VkResult : int32_t
{
	VK_SUCCESS = 0,
	VK_NOT_READY = 1,
	VK_TIMEOUT = 2,
	VK_EVENT_SET = 3,
	VK_EVENT_RESET = 4,
	VK_INCOMPLETE = 5,
	VK_SUBOPTIMAL_KHR = 1000001003,
	VK_ERROR_OUT_OF_HOST_MEMORY = -1,
	VK_ERROR_OUT_OF_DEVICE_MEMORY = -2,
	VK_ERROR_INITIALIZATION_FAILED = -3,
	VK_ERROR_DEVICE_LOST = -4,
	VK_ERROR_MEMORY_MAP_FAILED = -5,
	VK_ERROR_LAYER_NOT_PRESENT = -6,
	VK_ERROR_EXTENSION_NOT_PRESENT = -7,
	VK_ERROR_FEATURE_NOT_PRESENT = -8,
	VK_ERROR_INCOMPATIBLE_DRIVER = -9,
	VK_ERROR_TOO_MANY_OBJECTS = -10,
	VK_ERROR_FORMAT_NOT_SUPPORTED = -11,
	VK_ERROR_FRAGMENTED_POOL = -12,
	VK_ERROR_OUT_OF_POOL_MEMORY = -1000069000,
	VK_ERROR_SURFACE_LOST_KHR = -1000000000,
	VK_ERROR_OUT_OF_DATE_KHR = -1000001004,
};

enum VkStructureType : int32_t
{
	VK_STRUCTURE_TYPE_APPLICATION_INFO = 0,
	VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1,
	VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO = 2,
	VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 3,
	VK_STRUCTURE_TYPE_SUBMIT_INFO = 4,
	VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO = 5,
	VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE = 6,
	VK_STRUCTURE_TYPE_FENCE_CREATE_INFO = 8,
	VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO = 9,
	VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO = 11,
	VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO = 12,
	VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO = 14,
	VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO = 15,
	VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO = 16,
	VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO = 18,
	VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO = 19,
	VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO = 20,
	VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO = 22,
	VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO = 23,
	VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO = 24,
	VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO = 25,
	VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO = 26,
	VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO = 27,
	VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO = 28,
	VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO = 30,
	VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO = 31,
	VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO = 32,
	VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO = 33,
	VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO = 34,
	VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET = 35,
	VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO = 37,
	VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO = 38,
	VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO = 39,
	VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO = 40,
	VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO = 42,
	VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO = 43,
	VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER = 44,
	VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER = 45,
	VK_STRUCTURE_TYPE_MEMORY_BARRIER = 46,
	VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR = 1000001000,
	VK_STRUCTURE_TYPE_PRESENT_INFO_KHR = 1000001001,
	VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR = 1000009000,
};

enum VkFormat : int32_t
{
	VK_FORMAT_UNDEFINED = 0,
	VK_FORMAT_R8_UNORM = 9,
	VK_FORMAT_R8G8B8A8_UNORM = 37,
	VK_FORMAT_R8G8B8A8_SNORM = 38,
	VK_FORMAT_R8G8B8A8_USCALED = 39,
	VK_FORMAT_R8G8B8A8_UINT = 41,
	VK_FORMAT_R8G8B8A8_SRGB = 43,
	VK_FORMAT_B8G8R8A8_UNORM = 44,
	VK_FORMAT_B8G8R8A8_SRGB = 50,
	VK_FORMAT_A2B10G10R10_UNORM_PACK32 = 64,
	VK_FORMAT_R16_UNORM = 70,
	VK_FORMAT_R16_SFLOAT = 76,
	VK_FORMAT_R16G16_UNORM = 77,
	VK_FORMAT_R16G16_SNORM = 78,
	VK_FORMAT_R16G16_SSCALED = 80,
	VK_FORMAT_R16G16_SINT = 82,
	VK_FORMAT_R16G16_SFLOAT = 83,
	VK_FORMAT_R16G16B16A16_UNORM = 91,
	VK_FORMAT_R16G16B16A16_SNORM = 92,
	VK_FORMAT_R16G16B16A16_SSCALED = 94,
	VK_FORMAT_R16G16B16A16_SINT = 96,
	VK_FORMAT_R16G16B16A16_SFLOAT = 97,
	VK_FORMAT_R32_UINT = 98,
	VK_FORMAT_R32_SINT = 99,
	VK_FORMAT_R32_SFLOAT = 100,
	VK_FORMAT_R32G32_SFLOAT = 103,
	VK_FORMAT_R32G32B32_SFLOAT = 106,
	VK_FORMAT_R32G32B32A32_UINT = 107,
	VK_FORMAT_R32G32B32A32_SINT = 108,
	VK_FORMAT_R32G32B32A32_SFLOAT = 109,
	VK_FORMAT_D16_UNORM = 124,
	VK_FORMAT_X8_D24_UNORM_PACK32 = 125,
	VK_FORMAT_D32_SFLOAT = 126,
	VK_FORMAT_D24_UNORM_S8_UINT = 129,
	VK_FORMAT_D32_SFLOAT_S8_UINT = 130,
	VK_FORMAT_BC1_RGBA_UNORM_BLOCK = 133,
	VK_FORMAT_BC2_UNORM_BLOCK = 135,
	VK_FORMAT_BC3_UNORM_BLOCK = 137,
	VK_FORMAT_BC7_UNORM_BLOCK = 145,
};

enum VkColorSpaceKHR : int32_t
{
	VK_COLOR_SPACE_SRGB_NONLINEAR_KHR = 0,
};

enum VkPresentModeKHR : int32_t
{
	VK_PRESENT_MODE_IMMEDIATE_KHR = 0,
	VK_PRESENT_MODE_MAILBOX_KHR = 1,
	VK_PRESENT_MODE_FIFO_KHR = 2,
	VK_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
};

enum VkPhysicalDeviceType : int32_t
{
	VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
	VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
	VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
	VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
	VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
};

enum VkImageType : int32_t { VK_IMAGE_TYPE_1D = 0, VK_IMAGE_TYPE_2D = 1, VK_IMAGE_TYPE_3D = 2 };
enum VkImageViewType : int32_t { VK_IMAGE_VIEW_TYPE_1D = 0, VK_IMAGE_VIEW_TYPE_2D = 1, VK_IMAGE_VIEW_TYPE_3D = 2, VK_IMAGE_VIEW_TYPE_CUBE = 3 };
enum VkImageTiling : int32_t { VK_IMAGE_TILING_OPTIMAL = 0, VK_IMAGE_TILING_LINEAR = 1 };
enum VkSharingMode : int32_t { VK_SHARING_MODE_EXCLUSIVE = 0, VK_SHARING_MODE_CONCURRENT = 1 };

enum VkImageLayout : int32_t
{
	VK_IMAGE_LAYOUT_UNDEFINED = 0,
	VK_IMAGE_LAYOUT_GENERAL = 1,
	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
	VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5,
	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
	VK_IMAGE_LAYOUT_PREINITIALIZED = 8,
	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR = 1000001002,
};

enum VkComponentSwizzle : int32_t
{
	VK_COMPONENT_SWIZZLE_IDENTITY = 0,
	VK_COMPONENT_SWIZZLE_ZERO = 1,
	VK_COMPONENT_SWIZZLE_ONE = 2,
	VK_COMPONENT_SWIZZLE_R = 3,
	VK_COMPONENT_SWIZZLE_G = 4,
	VK_COMPONENT_SWIZZLE_B = 5,
	VK_COMPONENT_SWIZZLE_A = 6,
};

enum VkAttachmentLoadOp : int32_t { VK_ATTACHMENT_LOAD_OP_LOAD = 0, VK_ATTACHMENT_LOAD_OP_CLEAR = 1, VK_ATTACHMENT_LOAD_OP_DONT_CARE = 2 };
enum VkAttachmentStoreOp : int32_t { VK_ATTACHMENT_STORE_OP_STORE = 0, VK_ATTACHMENT_STORE_OP_DONT_CARE = 1 };
enum VkPipelineBindPoint : int32_t { VK_PIPELINE_BIND_POINT_GRAPHICS = 0, VK_PIPELINE_BIND_POINT_COMPUTE = 1 };
enum VkCommandBufferLevel : int32_t { VK_COMMAND_BUFFER_LEVEL_PRIMARY = 0, VK_COMMAND_BUFFER_LEVEL_SECONDARY = 1 };
enum VkSubpassContents : int32_t { VK_SUBPASS_CONTENTS_INLINE = 0, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS = 1 };
enum VkIndexType : int32_t { VK_INDEX_TYPE_UINT16 = 0, VK_INDEX_TYPE_UINT32 = 1 };
enum VkVertexInputRate : int32_t { VK_VERTEX_INPUT_RATE_VERTEX = 0, VK_VERTEX_INPUT_RATE_INSTANCE = 1 };

enum VkPrimitiveTopology : int32_t
{
	VK_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
	VK_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
	VK_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 5,
};

enum VkPolygonMode : int32_t { VK_POLYGON_MODE_FILL = 0, VK_POLYGON_MODE_LINE = 1, VK_POLYGON_MODE_POINT = 2 };
enum VkFrontFace : int32_t { VK_FRONT_FACE_COUNTER_CLOCKWISE = 0, VK_FRONT_FACE_CLOCKWISE = 1 };

enum VkCompareOp : int32_t
{
	VK_COMPARE_OP_NEVER = 0,
	VK_COMPARE_OP_LESS = 1,
	VK_COMPARE_OP_EQUAL = 2,
	VK_COMPARE_OP_LESS_OR_EQUAL = 3,
	VK_COMPARE_OP_GREATER = 4,
	VK_COMPARE_OP_NOT_EQUAL = 5,
	VK_COMPARE_OP_GREATER_OR_EQUAL = 6,
	VK_COMPARE_OP_ALWAYS = 7,
};

enum VkStencilOp : int32_t
{
	VK_STENCIL_OP_KEEP = 0,
	VK_STENCIL_OP_ZERO = 1,
	VK_STENCIL_OP_REPLACE = 2,
	VK_STENCIL_OP_INCREMENT_AND_CLAMP = 3,
	VK_STENCIL_OP_DECREMENT_AND_CLAMP = 4,
	VK_STENCIL_OP_INVERT = 5,
	VK_STENCIL_OP_INCREMENT_AND_WRAP = 6,
	VK_STENCIL_OP_DECREMENT_AND_WRAP = 7,
};

enum VkLogicOp : int32_t { VK_LOGIC_OP_CLEAR = 0, VK_LOGIC_OP_COPY = 3 };

enum VkBlendFactor : int32_t
{
	VK_BLEND_FACTOR_ZERO = 0,
	VK_BLEND_FACTOR_ONE = 1,
	VK_BLEND_FACTOR_SRC_COLOR = 2,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR = 3,
	VK_BLEND_FACTOR_DST_COLOR = 4,
	VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR = 5,
	VK_BLEND_FACTOR_SRC_ALPHA = 6,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = 7,
	VK_BLEND_FACTOR_DST_ALPHA = 8,
	VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA = 9,
	VK_BLEND_FACTOR_CONSTANT_COLOR = 10,
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 11,
	VK_BLEND_FACTOR_CONSTANT_ALPHA = 12,
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 13,
	VK_BLEND_FACTOR_SRC_ALPHA_SATURATE = 14,
};

enum VkBlendOp : int32_t
{
	VK_BLEND_OP_ADD = 0,
	VK_BLEND_OP_SUBTRACT = 1,
	VK_BLEND_OP_REVERSE_SUBTRACT = 2,
	VK_BLEND_OP_MIN = 3,
	VK_BLEND_OP_MAX = 4,
};

enum VkDynamicState : int32_t
{
	VK_DYNAMIC_STATE_VIEWPORT = 0,
	VK_DYNAMIC_STATE_SCISSOR = 1,
	VK_DYNAMIC_STATE_LINE_WIDTH = 2,
	VK_DYNAMIC_STATE_DEPTH_BIAS = 3,
	VK_DYNAMIC_STATE_BLEND_CONSTANTS = 4,
	VK_DYNAMIC_STATE_DEPTH_BOUNDS = 5,
	VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK = 6,
	VK_DYNAMIC_STATE_STENCIL_WRITE_MASK = 7,
	VK_DYNAMIC_STATE_STENCIL_REFERENCE = 8,
};

enum VkFilter : int32_t { VK_FILTER_NEAREST = 0, VK_FILTER_LINEAR = 1 };
enum VkSamplerMipmapMode : int32_t { VK_SAMPLER_MIPMAP_MODE_NEAREST = 0, VK_SAMPLER_MIPMAP_MODE_LINEAR = 1 };

enum VkSamplerAddressMode : int32_t
{
	VK_SAMPLER_ADDRESS_MODE_REPEAT = 0,
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER = 3,
	VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = 4,
};

enum VkBorderColor : int32_t
{
	VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK = 0,
	VK_BORDER_COLOR_INT_TRANSPARENT_BLACK = 1,
	VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK = 2,
	VK_BORDER_COLOR_INT_OPAQUE_BLACK = 3,
	VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE = 4,
	VK_BORDER_COLOR_INT_OPAQUE_WHITE = 5,
};

enum VkDescriptorType : int32_t
{
	VK_DESCRIPTOR_TYPE_SAMPLER = 0,
	VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
	VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,
	VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
};

enum VkQueryType : int32_t { VK_QUERY_TYPE_OCCLUSION = 0 };

// Bitmask constants
static constexpr VkFlags VK_QUEUE_GRAPHICS_BIT = 0x00000001u;
static constexpr VkFlags VK_QUEUE_COMPUTE_BIT  = 0x00000002u;
static constexpr VkFlags VK_QUEUE_TRANSFER_BIT = 0x00000004u;

static constexpr VkFlags VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT  = 0x00000001u;
static constexpr VkFlags VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  = 0x00000002u;
static constexpr VkFlags VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004u;
static constexpr VkFlags VK_MEMORY_PROPERTY_HOST_CACHED_BIT   = 0x00000008u;

static constexpr VkFlags VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT             = 0x00000001u;
static constexpr VkFlags VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT             = 0x00000040u;
static constexpr VkFlags VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT          = 0x00000080u;
static constexpr VkFlags VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT  = 0x00000200u;
static constexpr VkFlags VK_FORMAT_FEATURE_BLIT_SRC_BIT                  = 0x00000400u;
static constexpr VkFlags VK_FORMAT_FEATURE_BLIT_DST_BIT                  = 0x00000800u;

static constexpr VkFlags VK_IMAGE_USAGE_TRANSFER_SRC_BIT             = 0x00000001u;
static constexpr VkFlags VK_IMAGE_USAGE_TRANSFER_DST_BIT             = 0x00000002u;
static constexpr VkFlags VK_IMAGE_USAGE_SAMPLED_BIT                  = 0x00000004u;
static constexpr VkFlags VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT         = 0x00000010u;
static constexpr VkFlags VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000020u;

static constexpr VkFlags VK_BUFFER_USAGE_TRANSFER_SRC_BIT   = 0x00000001u;
static constexpr VkFlags VK_BUFFER_USAGE_TRANSFER_DST_BIT   = 0x00000002u;
static constexpr VkFlags VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010u;
static constexpr VkFlags VK_BUFFER_USAGE_INDEX_BUFFER_BIT   = 0x00000040u;
static constexpr VkFlags VK_BUFFER_USAGE_VERTEX_BUFFER_BIT  = 0x00000080u;

static constexpr VkFlags VK_SAMPLE_COUNT_1_BIT  = 0x00000001u;
static constexpr VkFlags VK_SAMPLE_COUNT_2_BIT  = 0x00000002u;
static constexpr VkFlags VK_SAMPLE_COUNT_4_BIT  = 0x00000004u;
static constexpr VkFlags VK_SAMPLE_COUNT_8_BIT  = 0x00000008u;

static constexpr VkFlags VK_IMAGE_ASPECT_COLOR_BIT   = 0x00000001u;
static constexpr VkFlags VK_IMAGE_ASPECT_DEPTH_BIT   = 0x00000002u;
static constexpr VkFlags VK_IMAGE_ASPECT_STENCIL_BIT = 0x00000004u;

static constexpr VkFlags VK_SHADER_STAGE_VERTEX_BIT   = 0x00000001u;
static constexpr VkFlags VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010u;
static constexpr VkFlags VK_SHADER_STAGE_ALL_GRAPHICS = 0x0000001Fu;

static constexpr VkFlags VK_CULL_MODE_NONE      = 0x00000000u;
static constexpr VkFlags VK_CULL_MODE_FRONT_BIT = 0x00000001u;
static constexpr VkFlags VK_CULL_MODE_BACK_BIT  = 0x00000002u;

static constexpr VkFlags VK_COLOR_COMPONENT_R_BIT = 0x00000001u;
static constexpr VkFlags VK_COLOR_COMPONENT_G_BIT = 0x00000002u;
static constexpr VkFlags VK_COLOR_COMPONENT_B_BIT = 0x00000004u;
static constexpr VkFlags VK_COLOR_COMPONENT_A_BIT = 0x00000008u;

static constexpr VkFlags VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT = 0x00000002u;
static constexpr VkFlags VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT     = 0x00000001u;
static constexpr VkFlags VK_FENCE_CREATE_SIGNALED_BIT                    = 0x00000001u;
static constexpr VkFlags VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT = 0x00000001u;
static constexpr VkFlags VK_QUERY_RESULT_64_BIT                          = 0x00000001u;
static constexpr VkFlags VK_QUERY_RESULT_WAIT_BIT                        = 0x00000002u;

static constexpr VkFlags VK_STENCIL_FACE_FRONT_BIT    = 0x00000001u;
static constexpr VkFlags VK_STENCIL_FACE_BACK_BIT     = 0x00000002u;
static constexpr VkFlags VK_STENCIL_FACE_FRONT_AND_BACK = 0x00000003u;

static constexpr VkFlags VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT             = 0x00000001u;
static constexpr VkFlags VK_PIPELINE_STAGE_VERTEX_INPUT_BIT            = 0x00000004u;
static constexpr VkFlags VK_PIPELINE_STAGE_VERTEX_SHADER_BIT           = 0x00000008u;
static constexpr VkFlags VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT         = 0x00000080u;
static constexpr VkFlags VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT    = 0x00000100u;
static constexpr VkFlags VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT     = 0x00000200u;
static constexpr VkFlags VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400u;
static constexpr VkFlags VK_PIPELINE_STAGE_TRANSFER_BIT                = 0x00001000u;
static constexpr VkFlags VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT          = 0x00002000u;
static constexpr VkFlags VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT            = 0x00008000u;
static constexpr VkFlags VK_PIPELINE_STAGE_ALL_COMMANDS_BIT            = 0x00010000u;

static constexpr VkFlags VK_ACCESS_INDEX_READ_BIT                     = 0x00000002u;
static constexpr VkFlags VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT          = 0x00000004u;
static constexpr VkFlags VK_ACCESS_UNIFORM_READ_BIT                   = 0x00000008u;
static constexpr VkFlags VK_ACCESS_SHADER_READ_BIT                    = 0x00000020u;
static constexpr VkFlags VK_ACCESS_COLOR_ATTACHMENT_READ_BIT          = 0x00000080u;
static constexpr VkFlags VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT         = 0x00000100u;
static constexpr VkFlags VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT  = 0x00000200u;
static constexpr VkFlags VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT = 0x00000400u;
static constexpr VkFlags VK_ACCESS_TRANSFER_READ_BIT                  = 0x00000800u;
static constexpr VkFlags VK_ACCESS_TRANSFER_WRITE_BIT                 = 0x00001000u;
static constexpr VkFlags VK_ACCESS_HOST_READ_BIT                      = 0x00002000u;
static constexpr VkFlags VK_ACCESS_HOST_WRITE_BIT                     = 0x00004000u;
static constexpr VkFlags VK_ACCESS_MEMORY_READ_BIT                    = 0x00008000u;
static constexpr VkFlags VK_ACCESS_MEMORY_WRITE_BIT                   = 0x00010000u;

static constexpr VkFlags VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR = 0x00000001u;
static constexpr VkFlags VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR     = 0x00000001u;

// Core Vulkan structures (exact Khronos vulkan_core.h layout)
struct VkOffset2D { int32_t x; int32_t y; };
struct VkExtent2D { uint32_t width; uint32_t height; };
struct VkOffset3D { int32_t x; int32_t y; int32_t z; };
struct VkExtent3D { uint32_t width; uint32_t height; uint32_t depth; };
struct VkRect2D { VkOffset2D offset; VkExtent2D extent; };
struct VkViewport { float x; float y; float width; float height; float minDepth; float maxDepth; };

struct VkApplicationInfo
{
	VkStructureType sType;
	const void* pNext;
	const char* pApplicationName;
	uint32_t applicationVersion;
	const char* pEngineName;
	uint32_t engineVersion;
	uint32_t apiVersion;
};

struct VkInstanceCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	const VkApplicationInfo* pApplicationInfo;
	uint32_t enabledLayerCount;
	const char* const* ppEnabledLayerNames;
	uint32_t enabledExtensionCount;
	const char* const* ppEnabledExtensionNames;
};

struct VkExtensionProperties
{
	char extensionName[VK_MAX_EXTENSION_NAME_SIZE];
	uint32_t specVersion;
};

struct VkLayerProperties
{
	char layerName[VK_MAX_EXTENSION_NAME_SIZE];
	uint32_t specVersion;
	uint32_t implementationVersion;
	char description[VK_MAX_DESCRIPTION_SIZE];
};

struct VkPhysicalDeviceFeatures
{
	VkBool32 robustBufferAccess;
	VkBool32 fullDrawIndexUint32;
	VkBool32 imageCubeArray;
	VkBool32 independentBlend;
	VkBool32 geometryShader;
	VkBool32 tessellationShader;
	VkBool32 sampleRateShading;
	VkBool32 dualSrcBlend;
	VkBool32 logicOp;
	VkBool32 multiDrawIndirect;
	VkBool32 drawIndirectFirstInstance;
	VkBool32 depthClamp;
	VkBool32 depthBiasClamp;
	VkBool32 fillModeNonSolid;
	VkBool32 depthBounds;
	VkBool32 wideLines;
	VkBool32 largePoints;
	VkBool32 alphaToOne;
	VkBool32 multiViewport;
	VkBool32 samplerAnisotropy;
	VkBool32 textureCompressionETC2;
	VkBool32 textureCompressionASTC_LDR;
	VkBool32 textureCompressionBC;
	VkBool32 occlusionQueryPrecise;
	VkBool32 pipelineStatisticsQuery;
	VkBool32 vertexPipelineStoresAndAtomics;
	VkBool32 fragmentStoresAndAtomics;
	VkBool32 shaderTessellationAndGeometryPointSize;
	VkBool32 shaderImageGatherExtended;
	VkBool32 shaderStorageImageExtendedFormats;
	VkBool32 shaderStorageImageMultisample;
	VkBool32 shaderStorageImageReadWithoutFormat;
	VkBool32 shaderStorageImageWriteWithoutFormat;
	VkBool32 shaderUniformBufferArrayDynamicIndexing;
	VkBool32 shaderSampledImageArrayDynamicIndexing;
	VkBool32 shaderStorageBufferArrayDynamicIndexing;
	VkBool32 shaderStorageImageArrayDynamicIndexing;
	VkBool32 shaderClipDistance;
	VkBool32 shaderCullDistance;
	VkBool32 shaderFloat64;
	VkBool32 shaderInt64;
	VkBool32 shaderInt16;
	VkBool32 shaderResourceResidency;
	VkBool32 shaderResourceMinLod;
	VkBool32 sparseBinding;
	VkBool32 sparseResidencyBuffer;
	VkBool32 sparseResidencyImage2D;
	VkBool32 sparseResidencyImage3D;
	VkBool32 sparseResidency2Samples;
	VkBool32 sparseResidency4Samples;
	VkBool32 sparseResidency8Samples;
	VkBool32 sparseResidency16Samples;
	VkBool32 sparseResidencyAliased;
	VkBool32 variableMultisampleRate;
	VkBool32 inheritedQueries;
};

struct VkPhysicalDeviceLimits
{
	uint32_t maxImageDimension1D;
	uint32_t maxImageDimension2D;
	uint32_t maxImageDimension3D;
	uint32_t maxImageDimensionCube;
	uint32_t maxImageArrayLayers;
	uint32_t maxTexelBufferElements;
	uint32_t maxUniformBufferRange;
	uint32_t maxStorageBufferRange;
	uint32_t maxPushConstantsSize;
	uint32_t maxMemoryAllocationCount;
	uint32_t maxSamplerAllocationCount;
	VkDeviceSize bufferImageGranularity;
	VkDeviceSize sparseAddressSpaceSize;
	uint32_t maxBoundDescriptorSets;
	uint32_t maxPerStageDescriptorSamplers;
	uint32_t maxPerStageDescriptorUniformBuffers;
	uint32_t maxPerStageDescriptorStorageBuffers;
	uint32_t maxPerStageDescriptorSampledImages;
	uint32_t maxPerStageDescriptorStorageImages;
	uint32_t maxPerStageDescriptorInputAttachments;
	uint32_t maxPerStageResources;
	uint32_t maxDescriptorSetSamplers;
	uint32_t maxDescriptorSetUniformBuffers;
	uint32_t maxDescriptorSetUniformBuffersDynamic;
	uint32_t maxDescriptorSetStorageBuffers;
	uint32_t maxDescriptorSetStorageBuffersDynamic;
	uint32_t maxDescriptorSetSampledImages;
	uint32_t maxDescriptorSetStorageImages;
	uint32_t maxDescriptorSetInputAttachments;
	uint32_t maxVertexInputAttributes;
	uint32_t maxVertexInputBindings;
	uint32_t maxVertexInputAttributeOffset;
	uint32_t maxVertexInputBindingStride;
	uint32_t maxVertexOutputComponents;
	uint32_t maxTessellationGenerationLevel;
	uint32_t maxTessellationPatchSize;
	uint32_t maxTessellationControlPerVertexInputComponents;
	uint32_t maxTessellationControlPerVertexOutputComponents;
	uint32_t maxTessellationControlPerPatchOutputComponents;
	uint32_t maxTessellationControlTotalOutputComponents;
	uint32_t maxTessellationEvaluationInputComponents;
	uint32_t maxTessellationEvaluationOutputComponents;
	uint32_t maxGeometryShaderInvocations;
	uint32_t maxGeometryInputComponents;
	uint32_t maxGeometryOutputComponents;
	uint32_t maxGeometryOutputVertices;
	uint32_t maxGeometryTotalOutputComponents;
	uint32_t maxFragmentInputComponents;
	uint32_t maxFragmentOutputAttachments;
	uint32_t maxFragmentDualSrcAttachments;
	uint32_t maxFragmentCombinedOutputResources;
	uint32_t maxComputeSharedMemorySize;
	uint32_t maxComputeWorkGroupCount[3];
	uint32_t maxComputeWorkGroupInvocations;
	uint32_t maxComputeWorkGroupSize[3];
	uint32_t subPixelPrecisionBits;
	uint32_t subTexelPrecisionBits;
	uint32_t mipmapPrecisionBits;
	uint32_t maxDrawIndexedIndexValue;
	uint32_t maxDrawIndirectCount;
	float maxSamplerLodBias;
	float maxSamplerAnisotropy;
	uint32_t maxViewports;
	uint32_t maxViewportDimensions[2];
	float viewportBoundsRange[2];
	uint32_t viewportSubPixelBits;
	size_t minMemoryMapAlignment;
	VkDeviceSize minTexelBufferOffsetAlignment;
	VkDeviceSize minUniformBufferOffsetAlignment;
	VkDeviceSize minStorageBufferOffsetAlignment;
	int32_t minTexelOffset;
	uint32_t maxTexelOffset;
	int32_t minTexelGatherOffset;
	uint32_t maxTexelGatherOffset;
	float minInterpolationOffset;
	float maxInterpolationOffset;
	uint32_t subPixelInterpolationOffsetBits;
	uint32_t maxFramebufferWidth;
	uint32_t maxFramebufferHeight;
	uint32_t maxFramebufferLayers;
	VkFlags framebufferColorSampleCounts;
	VkFlags framebufferDepthSampleCounts;
	VkFlags framebufferStencilSampleCounts;
	VkFlags framebufferNoAttachmentsSampleCounts;
	uint32_t maxColorAttachments;
	VkFlags sampledImageColorSampleCounts;
	VkFlags sampledImageIntegerSampleCounts;
	VkFlags sampledImageDepthSampleCounts;
	VkFlags sampledImageStencilSampleCounts;
	VkFlags storageImageSampleCounts;
	uint32_t maxSampleMaskWords;
	VkBool32 timestampComputeAndGraphics;
	float timestampPeriod;
	uint32_t maxClipDistances;
	uint32_t maxCullDistances;
	uint32_t maxCombinedClipAndCullDistances;
	uint32_t discreteQueuePriorities;
	float pointSizeRange[2];
	float lineWidthRange[2];
	float pointSizeGranularity;
	float lineWidthGranularity;
	VkBool32 strictLines;
	VkBool32 standardSampleLocations;
	VkDeviceSize optimalBufferCopyOffsetAlignment;
	VkDeviceSize optimalBufferCopyRowPitchAlignment;
	VkDeviceSize nonCoherentAtomSize;
};

struct VkPhysicalDeviceSparseProperties
{
	VkBool32 residencyStandard2DBlockShape;
	VkBool32 residencyStandard2DMultisampleBlockShape;
	VkBool32 residencyStandard3DBlockShape;
	VkBool32 residencyAlignedMipSize;
	VkBool32 residencyNonResidentStrict;
};

struct VkPhysicalDeviceProperties
{
	uint32_t apiVersion;
	uint32_t driverVersion;
	uint32_t vendorID;
	uint32_t deviceID;
	VkPhysicalDeviceType deviceType;
	char deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
	uint8_t pipelineCacheUUID[VK_UUID_SIZE];
	VkPhysicalDeviceLimits limits;
	VkPhysicalDeviceSparseProperties sparseProperties;
};

struct VkFormatProperties
{
	VkFlags linearTilingFeatures;
	VkFlags optimalTilingFeatures;
	VkFlags bufferFeatures;
};

struct VkQueueFamilyProperties
{
	VkFlags queueFlags;
	uint32_t queueCount;
	uint32_t timestampValidBits;
	VkExtent3D minImageTransferGranularity;
};

struct VkMemoryType
{
	VkFlags propertyFlags;
	uint32_t heapIndex;
};

struct VkMemoryHeap
{
	VkDeviceSize size;
	VkFlags flags;
};

struct VkPhysicalDeviceMemoryProperties
{
	uint32_t memoryTypeCount;
	VkMemoryType memoryTypes[VK_MAX_MEMORY_TYPES];
	uint32_t memoryHeapCount;
	VkMemoryHeap memoryHeaps[VK_MAX_MEMORY_HEAPS];
};

struct VkDeviceQueueCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t queueFamilyIndex;
	uint32_t queueCount;
	const float* pQueuePriorities;
};

struct VkDeviceCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t queueCreateInfoCount;
	const VkDeviceQueueCreateInfo* pQueueCreateInfos;
	uint32_t enabledLayerCount;
	const char* const* ppEnabledLayerNames;
	uint32_t enabledExtensionCount;
	const char* const* ppEnabledExtensionNames;
	const VkPhysicalDeviceFeatures* pEnabledFeatures;
};

struct VkMemoryRequirements
{
	VkDeviceSize size;
	VkDeviceSize alignment;
	uint32_t memoryTypeBits;
};

struct VkMemoryAllocateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkDeviceSize allocationSize;
	uint32_t memoryTypeIndex;
};

struct VkBufferCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkDeviceSize size;
	VkFlags usage;
	VkSharingMode sharingMode;
	uint32_t queueFamilyIndexCount;
	const uint32_t* pQueueFamilyIndices;
};

struct VkImageCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkImageType imageType;
	VkFormat format;
	VkExtent3D extent;
	uint32_t mipLevels;
	uint32_t arrayLayers;
	VkFlags samples;
	VkImageTiling tiling;
	VkFlags usage;
	VkSharingMode sharingMode;
	uint32_t queueFamilyIndexCount;
	const uint32_t* pQueueFamilyIndices;
	VkImageLayout initialLayout;
};

struct VkComponentMapping
{
	VkComponentSwizzle r;
	VkComponentSwizzle g;
	VkComponentSwizzle b;
	VkComponentSwizzle a;
};

struct VkImageSubresourceRange
{
	VkFlags aspectMask;
	uint32_t baseMipLevel;
	uint32_t levelCount;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
};

struct VkImageSubresourceLayers
{
	VkFlags aspectMask;
	uint32_t mipLevel;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
};

struct VkImageViewCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkImage image;
	VkImageViewType viewType;
	VkFormat format;
	VkComponentMapping components;
	VkImageSubresourceRange subresourceRange;
};

struct VkShaderModuleCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	size_t codeSize;
	const uint32_t* pCode;
};

struct VkSamplerCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkFilter magFilter;
	VkFilter minFilter;
	VkSamplerMipmapMode mipmapMode;
	VkSamplerAddressMode addressModeU;
	VkSamplerAddressMode addressModeV;
	VkSamplerAddressMode addressModeW;
	float mipLodBias;
	VkBool32 anisotropyEnable;
	float maxAnisotropy;
	VkBool32 compareEnable;
	VkCompareOp compareOp;
	float minLod;
	float maxLod;
	VkBorderColor borderColor;
	VkBool32 unnormalizedCoordinates;
};

struct VkDescriptorSetLayoutBinding
{
	uint32_t binding;
	VkDescriptorType descriptorType;
	uint32_t descriptorCount;
	VkFlags stageFlags;
	const VkSampler* pImmutableSamplers;
};

struct VkDescriptorSetLayoutCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t bindingCount;
	const VkDescriptorSetLayoutBinding* pBindings;
};

struct VkDescriptorPoolSize
{
	VkDescriptorType type;
	uint32_t descriptorCount;
};

struct VkDescriptorPoolCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t maxSets;
	uint32_t poolSizeCount;
	const VkDescriptorPoolSize* pPoolSizes;
};

struct VkDescriptorSetAllocateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkDescriptorPool descriptorPool;
	uint32_t descriptorSetCount;
	const VkDescriptorSetLayout* pSetLayouts;
};

struct VkDescriptorImageInfo
{
	VkSampler sampler;
	VkImageView imageView;
	VkImageLayout imageLayout;
};

struct VkDescriptorBufferInfo
{
	VkBuffer buffer;
	VkDeviceSize offset;
	VkDeviceSize range;
};

struct VkWriteDescriptorSet
{
	VkStructureType sType;
	const void* pNext;
	VkDescriptorSet dstSet;
	uint32_t dstBinding;
	uint32_t dstArrayElement;
	uint32_t descriptorCount;
	VkDescriptorType descriptorType;
	const VkDescriptorImageInfo* pImageInfo;
	const VkDescriptorBufferInfo* pBufferInfo;
	const VkBufferView* pTexelBufferView;
};

struct VkPushConstantRange
{
	VkFlags stageFlags;
	uint32_t offset;
	uint32_t size;
};

struct VkPipelineLayoutCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t setLayoutCount;
	const VkDescriptorSetLayout* pSetLayouts;
	uint32_t pushConstantRangeCount;
	const VkPushConstantRange* pPushConstantRanges;
};

struct VkAttachmentDescription
{
	VkFlags flags;
	VkFormat format;
	VkFlags samples;
	VkAttachmentLoadOp loadOp;
	VkAttachmentStoreOp storeOp;
	VkAttachmentLoadOp stencilLoadOp;
	VkAttachmentStoreOp stencilStoreOp;
	VkImageLayout initialLayout;
	VkImageLayout finalLayout;
};

struct VkAttachmentReference
{
	uint32_t attachment;
	VkImageLayout layout;
};

struct VkSubpassDescription
{
	VkFlags flags;
	VkPipelineBindPoint pipelineBindPoint;
	uint32_t inputAttachmentCount;
	const VkAttachmentReference* pInputAttachments;
	uint32_t colorAttachmentCount;
	const VkAttachmentReference* pColorAttachments;
	const VkAttachmentReference* pResolveAttachments;
	const VkAttachmentReference* pDepthStencilAttachment;
	uint32_t preserveAttachmentCount;
	const uint32_t* pPreserveAttachments;
};

struct VkSubpassDependency
{
	uint32_t srcSubpass;
	uint32_t dstSubpass;
	VkFlags srcStageMask;
	VkFlags dstStageMask;
	VkFlags srcAccessMask;
	VkFlags dstAccessMask;
	VkFlags dependencyFlags;
};

struct VkRenderPassCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t attachmentCount;
	const VkAttachmentDescription* pAttachments;
	uint32_t subpassCount;
	const VkSubpassDescription* pSubpasses;
	uint32_t dependencyCount;
	const VkSubpassDependency* pDependencies;
};

struct VkFramebufferCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkRenderPass renderPass;
	uint32_t attachmentCount;
	const VkImageView* pAttachments;
	uint32_t width;
	uint32_t height;
	uint32_t layers;
};

struct VkPipelineShaderStageCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkFlags stage;
	VkShaderModule module;
	const char* pName;
	const void* pSpecializationInfo;
};

struct VkVertexInputBindingDescription
{
	uint32_t binding;
	uint32_t stride;
	VkVertexInputRate inputRate;
};

struct VkVertexInputAttributeDescription
{
	uint32_t location;
	uint32_t binding;
	VkFormat format;
	uint32_t offset;
};

struct VkPipelineVertexInputStateCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t vertexBindingDescriptionCount;
	const VkVertexInputBindingDescription* pVertexBindingDescriptions;
	uint32_t vertexAttributeDescriptionCount;
	const VkVertexInputAttributeDescription* pVertexAttributeDescriptions;
};

struct VkPipelineInputAssemblyStateCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkPrimitiveTopology topology;
	VkBool32 primitiveRestartEnable;
};

struct VkPipelineViewportStateCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t viewportCount;
	const VkViewport* pViewports;
	uint32_t scissorCount;
	const VkRect2D* pScissors;
};

struct VkPipelineRasterizationStateCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkBool32 depthClampEnable;
	VkBool32 rasterizerDiscardEnable;
	VkPolygonMode polygonMode;
	VkFlags cullMode;
	VkFrontFace frontFace;
	VkBool32 depthBiasEnable;
	float depthBiasConstantFactor;
	float depthBiasClamp;
	float depthBiasSlopeFactor;
	float lineWidth;
};

struct VkPipelineMultisampleStateCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkFlags rasterizationSamples;
	VkBool32 sampleShadingEnable;
	float minSampleShading;
	const VkSampleMask* pSampleMask;
	VkBool32 alphaToCoverageEnable;
	VkBool32 alphaToOneEnable;
};

struct VkStencilOpState
{
	VkStencilOp failOp;
	VkStencilOp passOp;
	VkStencilOp depthFailOp;
	VkCompareOp compareOp;
	uint32_t compareMask;
	uint32_t writeMask;
	uint32_t reference;
};

struct VkPipelineDepthStencilStateCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkBool32 depthTestEnable;
	VkBool32 depthWriteEnable;
	VkCompareOp depthCompareOp;
	VkBool32 depthBoundsTestEnable;
	VkBool32 stencilTestEnable;
	VkStencilOpState front;
	VkStencilOpState back;
	float minDepthBounds;
	float maxDepthBounds;
};

struct VkPipelineColorBlendAttachmentState
{
	VkBool32 blendEnable;
	VkBlendFactor srcColorBlendFactor;
	VkBlendFactor dstColorBlendFactor;
	VkBlendOp colorBlendOp;
	VkBlendFactor srcAlphaBlendFactor;
	VkBlendFactor dstAlphaBlendFactor;
	VkBlendOp alphaBlendOp;
	VkFlags colorWriteMask;
};

struct VkPipelineColorBlendStateCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkBool32 logicOpEnable;
	VkLogicOp logicOp;
	uint32_t attachmentCount;
	const VkPipelineColorBlendAttachmentState* pAttachments;
	float blendConstants[4];
};

struct VkPipelineDynamicStateCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t dynamicStateCount;
	const VkDynamicState* pDynamicStates;
};

struct VkGraphicsPipelineCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t stageCount;
	const VkPipelineShaderStageCreateInfo* pStages;
	const VkPipelineVertexInputStateCreateInfo* pVertexInputState;
	const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState;
	const void* pTessellationState;
	const VkPipelineViewportStateCreateInfo* pViewportState;
	const VkPipelineRasterizationStateCreateInfo* pRasterizationState;
	const VkPipelineMultisampleStateCreateInfo* pMultisampleState;
	const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState;
	const VkPipelineColorBlendStateCreateInfo* pColorBlendState;
	const VkPipelineDynamicStateCreateInfo* pDynamicState;
	VkPipelineLayout layout;
	VkRenderPass renderPass;
	uint32_t subpass;
	VkPipeline basePipelineHandle;
	int32_t basePipelineIndex;
};

union VkClearColorValue
{
	float float32[4];
	int32_t int32[4];
	uint32_t uint32[4];
};

struct VkClearDepthStencilValue
{
	float depth;
	uint32_t stencil;
};

union VkClearValue
{
	VkClearColorValue color;
	VkClearDepthStencilValue depthStencil;
};

struct VkClearAttachment
{
	VkFlags aspectMask;
	uint32_t colorAttachment;
	VkClearValue clearValue;
};

struct VkClearRect
{
	VkRect2D rect;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
};

struct VkRenderPassBeginInfo
{
	VkStructureType sType;
	const void* pNext;
	VkRenderPass renderPass;
	VkFramebuffer framebuffer;
	VkRect2D renderArea;
	uint32_t clearValueCount;
	const VkClearValue* pClearValues;
};

struct VkCommandPoolCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	uint32_t queueFamilyIndex;
};

struct VkCommandBufferAllocateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkCommandPool commandPool;
	VkCommandBufferLevel level;
	uint32_t commandBufferCount;
};

struct VkCommandBufferBeginInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	const void* pInheritanceInfo;
};

struct VkBufferCopy
{
	VkDeviceSize srcOffset;
	VkDeviceSize dstOffset;
	VkDeviceSize size;
};

struct VkBufferImageCopy
{
	VkDeviceSize bufferOffset;
	uint32_t bufferRowLength;
	uint32_t bufferImageHeight;
	VkImageSubresourceLayers imageSubresource;
	VkOffset3D imageOffset;
	VkExtent3D imageExtent;
};

struct VkImageCopy
{
	VkImageSubresourceLayers srcSubresource;
	VkOffset3D srcOffset;
	VkImageSubresourceLayers dstSubresource;
	VkOffset3D dstOffset;
	VkExtent3D extent;
};

struct VkImageBlit
{
	VkImageSubresourceLayers srcSubresource;
	VkOffset3D srcOffsets[2];
	VkImageSubresourceLayers dstSubresource;
	VkOffset3D dstOffsets[2];
};

struct VkImageMemoryBarrier
{
	VkStructureType sType;
	const void* pNext;
	VkFlags srcAccessMask;
	VkFlags dstAccessMask;
	VkImageLayout oldLayout;
	VkImageLayout newLayout;
	uint32_t srcQueueFamilyIndex;
	uint32_t dstQueueFamilyIndex;
	VkImage image;
	VkImageSubresourceRange subresourceRange;
};

struct VkFenceCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
};

struct VkSemaphoreCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
};

struct VkQueryPoolCreateInfo
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkQueryType queryType;
	uint32_t queryCount;
	VkFlags pipelineStatistics;
};

struct VkSubmitInfo
{
	VkStructureType sType;
	const void* pNext;
	uint32_t waitSemaphoreCount;
	const VkSemaphore* pWaitSemaphores;
	const VkFlags* pWaitDstStageMask;
	uint32_t commandBufferCount;
	const VkCommandBuffer* pCommandBuffers;
	uint32_t signalSemaphoreCount;
	const VkSemaphore* pSignalSemaphores;
};

// WSI / Swapchain structures
struct VkSurfaceCapabilitiesKHR
{
	uint32_t minImageCount;
	uint32_t maxImageCount;
	VkExtent2D currentExtent;
	VkExtent2D minImageExtent;
	VkExtent2D maxImageExtent;
	uint32_t maxImageArrayLayers;
	VkFlags supportedTransforms;
	VkFlags currentTransform;
	VkFlags supportedCompositeAlpha;
	VkFlags supportedUsageFlags;
};

struct VkSurfaceFormatKHR
{
	VkFormat format;
	VkColorSpaceKHR colorSpace;
};

struct VkSwapchainCreateInfoKHR
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	VkSurfaceKHR surface;
	uint32_t minImageCount;
	VkFormat imageFormat;
	VkColorSpaceKHR imageColorSpace;
	VkExtent2D imageExtent;
	uint32_t imageArrayLayers;
	VkFlags imageUsage;
	VkSharingMode imageSharingMode;
	uint32_t queueFamilyIndexCount;
	const uint32_t* pQueueFamilyIndices;
	VkFlags preTransform;
	VkFlags compositeAlpha;
	VkPresentModeKHR presentMode;
	VkBool32 clipped;
	VkSwapchainKHR oldSwapchain;
};

struct VkPresentInfoKHR
{
	VkStructureType sType;
	const void* pNext;
	uint32_t waitSemaphoreCount;
	const VkSemaphore* pWaitSemaphores;
	uint32_t swapchainCount;
	const VkSwapchainKHR* pSwapchains;
	const uint32_t* pImageIndices;
	VkResult* pResults;
};

#if defined(_WIN32)
struct VkWin32SurfaceCreateInfoKHR
{
	VkStructureType sType;
	const void* pNext;
	VkFlags flags;
	HINSTANCE hinstance;
	HWND hwnd;
};
#endif

typedef void (VKAPI_PTR *PFN_vkVoidFunction)(void);
typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_vkGetInstanceProcAddr)(VkInstance instance, const char* pName);
typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_vkGetDeviceProcAddr)(VkDevice device, const char* pName);

namespace ParaEngine
{
	struct VulkanDispatch
	{
		// Global functions
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
		VkResult (VKAPI_PTR *vkEnumerateInstanceExtensionProperties)(const char*, uint32_t*, VkExtensionProperties*) = nullptr;
		VkResult (VKAPI_PTR *vkEnumerateInstanceLayerProperties)(uint32_t*, VkLayerProperties*) = nullptr;
		VkResult (VKAPI_PTR *vkCreateInstance)(const VkInstanceCreateInfo*, const void*, VkInstance*) = nullptr;

		// Instance functions
		void     (VKAPI_PTR *vkDestroyInstance)(VkInstance, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkEnumeratePhysicalDevices)(VkInstance, uint32_t*, VkPhysicalDevice*) = nullptr;
		void     (VKAPI_PTR *vkGetPhysicalDeviceProperties)(VkPhysicalDevice, VkPhysicalDeviceProperties*) = nullptr;
		void     (VKAPI_PTR *vkGetPhysicalDeviceFeatures)(VkPhysicalDevice, VkPhysicalDeviceFeatures*) = nullptr;
		void     (VKAPI_PTR *vkGetPhysicalDeviceFormatProperties)(VkPhysicalDevice, VkFormat, VkFormatProperties*) = nullptr;
		void     (VKAPI_PTR *vkGetPhysicalDeviceMemoryProperties)(VkPhysicalDevice, VkPhysicalDeviceMemoryProperties*) = nullptr;
		void     (VKAPI_PTR *vkGetPhysicalDeviceQueueFamilyProperties)(VkPhysicalDevice, uint32_t*, VkQueueFamilyProperties*) = nullptr;
		VkResult (VKAPI_PTR *vkEnumerateDeviceExtensionProperties)(VkPhysicalDevice, const char*, uint32_t*, VkExtensionProperties*) = nullptr;
		VkResult (VKAPI_PTR *vkCreateDevice)(VkPhysicalDevice, const VkDeviceCreateInfo*, const void*, VkDevice*) = nullptr;
		PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr;

		// WSI surface functions
		void     (VKAPI_PTR *vkDestroySurfaceKHR)(VkInstance, VkSurfaceKHR, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkGetPhysicalDeviceSurfaceSupportKHR)(VkPhysicalDevice, uint32_t, VkSurfaceKHR, VkBool32*) = nullptr;
		VkResult (VKAPI_PTR *vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(VkPhysicalDevice, VkSurfaceKHR, VkSurfaceCapabilitiesKHR*) = nullptr;
		VkResult (VKAPI_PTR *vkGetPhysicalDeviceSurfaceFormatsKHR)(VkPhysicalDevice, VkSurfaceKHR, uint32_t*, VkSurfaceFormatKHR*) = nullptr;
		VkResult (VKAPI_PTR *vkGetPhysicalDeviceSurfacePresentModesKHR)(VkPhysicalDevice, VkSurfaceKHR, uint32_t*, VkPresentModeKHR*) = nullptr;
#if defined(_WIN32)
		VkResult (VKAPI_PTR *vkCreateWin32SurfaceKHR)(VkInstance, const VkWin32SurfaceCreateInfoKHR*, const void*, VkSurfaceKHR*) = nullptr;
#endif

		// Device functions
		void     (VKAPI_PTR *vkDestroyDevice)(VkDevice, const void*) = nullptr;
		void     (VKAPI_PTR *vkGetDeviceQueue)(VkDevice, uint32_t, uint32_t, VkQueue*) = nullptr;
		VkResult (VKAPI_PTR *vkDeviceWaitIdle)(VkDevice) = nullptr;
		VkResult (VKAPI_PTR *vkQueueSubmit)(VkQueue, uint32_t, const VkSubmitInfo*, VkFence) = nullptr;
		VkResult (VKAPI_PTR *vkQueueWaitIdle)(VkQueue) = nullptr;

		VkResult (VKAPI_PTR *vkAllocateMemory)(VkDevice, const VkMemoryAllocateInfo*, const void*, VkDeviceMemory*) = nullptr;
		void     (VKAPI_PTR *vkFreeMemory)(VkDevice, VkDeviceMemory, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkMapMemory)(VkDevice, VkDeviceMemory, VkDeviceSize, VkDeviceSize, VkFlags, void**) = nullptr;
		void     (VKAPI_PTR *vkUnmapMemory)(VkDevice, VkDeviceMemory) = nullptr;

		VkResult (VKAPI_PTR *vkCreateBuffer)(VkDevice, const VkBufferCreateInfo*, const void*, VkBuffer*) = nullptr;
		void     (VKAPI_PTR *vkDestroyBuffer)(VkDevice, VkBuffer, const void*) = nullptr;
		void     (VKAPI_PTR *vkGetBufferMemoryRequirements)(VkDevice, VkBuffer, VkMemoryRequirements*) = nullptr;
		VkResult (VKAPI_PTR *vkBindBufferMemory)(VkDevice, VkBuffer, VkDeviceMemory, VkDeviceSize) = nullptr;

		VkResult (VKAPI_PTR *vkCreateImage)(VkDevice, const VkImageCreateInfo*, const void*, VkImage*) = nullptr;
		void     (VKAPI_PTR *vkDestroyImage)(VkDevice, VkImage, const void*) = nullptr;
		void     (VKAPI_PTR *vkGetImageMemoryRequirements)(VkDevice, VkImage, VkMemoryRequirements*) = nullptr;
		VkResult (VKAPI_PTR *vkBindImageMemory)(VkDevice, VkImage, VkDeviceMemory, VkDeviceSize) = nullptr;

		VkResult (VKAPI_PTR *vkCreateImageView)(VkDevice, const VkImageViewCreateInfo*, const void*, VkImageView*) = nullptr;
		void     (VKAPI_PTR *vkDestroyImageView)(VkDevice, VkImageView, const void*) = nullptr;

		VkResult (VKAPI_PTR *vkCreateShaderModule)(VkDevice, const VkShaderModuleCreateInfo*, const void*, VkShaderModule*) = nullptr;
		void     (VKAPI_PTR *vkDestroyShaderModule)(VkDevice, VkShaderModule, const void*) = nullptr;

		VkResult (VKAPI_PTR *vkCreateSampler)(VkDevice, const VkSamplerCreateInfo*, const void*, VkSampler*) = nullptr;
		void     (VKAPI_PTR *vkDestroySampler)(VkDevice, VkSampler, const void*) = nullptr;

		VkResult (VKAPI_PTR *vkCreateDescriptorSetLayout)(VkDevice, const VkDescriptorSetLayoutCreateInfo*, const void*, VkDescriptorSetLayout*) = nullptr;
		void     (VKAPI_PTR *vkDestroyDescriptorSetLayout)(VkDevice, VkDescriptorSetLayout, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkCreateDescriptorPool)(VkDevice, const VkDescriptorPoolCreateInfo*, const void*, VkDescriptorPool*) = nullptr;
		void     (VKAPI_PTR *vkDestroyDescriptorPool)(VkDevice, VkDescriptorPool, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkResetDescriptorPool)(VkDevice, VkDescriptorPool, VkFlags) = nullptr;
		VkResult (VKAPI_PTR *vkAllocateDescriptorSets)(VkDevice, const VkDescriptorSetAllocateInfo*, VkDescriptorSet*) = nullptr;
		void     (VKAPI_PTR *vkUpdateDescriptorSets)(VkDevice, uint32_t, const VkWriteDescriptorSet*, uint32_t, const void*) = nullptr;

		VkResult (VKAPI_PTR *vkCreatePipelineLayout)(VkDevice, const VkPipelineLayoutCreateInfo*, const void*, VkPipelineLayout*) = nullptr;
		void     (VKAPI_PTR *vkDestroyPipelineLayout)(VkDevice, VkPipelineLayout, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkCreateRenderPass)(VkDevice, const VkRenderPassCreateInfo*, const void*, VkRenderPass*) = nullptr;
		void     (VKAPI_PTR *vkDestroyRenderPass)(VkDevice, VkRenderPass, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkCreateFramebuffer)(VkDevice, const VkFramebufferCreateInfo*, const void*, VkFramebuffer*) = nullptr;
		void     (VKAPI_PTR *vkDestroyFramebuffer)(VkDevice, VkFramebuffer, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkCreateGraphicsPipelines)(VkDevice, VkPipelineCache, uint32_t, const VkGraphicsPipelineCreateInfo*, const void*, VkPipeline*) = nullptr;
		void     (VKAPI_PTR *vkDestroyPipeline)(VkDevice, VkPipeline, const void*) = nullptr;

		VkResult (VKAPI_PTR *vkCreateCommandPool)(VkDevice, const VkCommandPoolCreateInfo*, const void*, VkCommandPool*) = nullptr;
		void     (VKAPI_PTR *vkDestroyCommandPool)(VkDevice, VkCommandPool, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkResetCommandPool)(VkDevice, VkCommandPool, VkFlags) = nullptr;
		VkResult (VKAPI_PTR *vkAllocateCommandBuffers)(VkDevice, const VkCommandBufferAllocateInfo*, VkCommandBuffer*) = nullptr;
		void     (VKAPI_PTR *vkFreeCommandBuffers)(VkDevice, VkCommandPool, uint32_t, const VkCommandBuffer*) = nullptr;

		VkResult (VKAPI_PTR *vkBeginCommandBuffer)(VkCommandBuffer, const VkCommandBufferBeginInfo*) = nullptr;
		VkResult (VKAPI_PTR *vkEndCommandBuffer)(VkCommandBuffer) = nullptr;
		VkResult (VKAPI_PTR *vkResetCommandBuffer)(VkCommandBuffer, VkFlags) = nullptr;

		void     (VKAPI_PTR *vkCmdBeginRenderPass)(VkCommandBuffer, const VkRenderPassBeginInfo*, VkSubpassContents) = nullptr;
		void     (VKAPI_PTR *vkCmdEndRenderPass)(VkCommandBuffer) = nullptr;
		void     (VKAPI_PTR *vkCmdBindPipeline)(VkCommandBuffer, VkPipelineBindPoint, VkPipeline) = nullptr;
		void     (VKAPI_PTR *vkCmdSetViewport)(VkCommandBuffer, uint32_t, uint32_t, const VkViewport*) = nullptr;
		void     (VKAPI_PTR *vkCmdSetScissor)(VkCommandBuffer, uint32_t, uint32_t, const VkRect2D*) = nullptr;
		void     (VKAPI_PTR *vkCmdSetBlendConstants)(VkCommandBuffer, const float[4]) = nullptr;
		void     (VKAPI_PTR *vkCmdSetStencilReference)(VkCommandBuffer, VkFlags, uint32_t) = nullptr;
		void     (VKAPI_PTR *vkCmdBindDescriptorSets)(VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t, uint32_t, const VkDescriptorSet*, uint32_t, const uint32_t*) = nullptr;
		void     (VKAPI_PTR *vkCmdBindIndexBuffer)(VkCommandBuffer, VkBuffer, VkDeviceSize, VkIndexType) = nullptr;
		void     (VKAPI_PTR *vkCmdBindVertexBuffers)(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*, const VkDeviceSize*) = nullptr;
		void     (VKAPI_PTR *vkCmdDraw)(VkCommandBuffer, uint32_t, uint32_t, uint32_t, uint32_t) = nullptr;
		void     (VKAPI_PTR *vkCmdDrawIndexed)(VkCommandBuffer, uint32_t, uint32_t, uint32_t, int32_t, uint32_t) = nullptr;
		void     (VKAPI_PTR *vkCmdClearAttachments)(VkCommandBuffer, uint32_t, const VkClearAttachment*, uint32_t, const VkClearRect*) = nullptr;

		void     (VKAPI_PTR *vkCmdCopyBuffer)(VkCommandBuffer, VkBuffer, VkBuffer, uint32_t, const VkBufferCopy*) = nullptr;
		void     (VKAPI_PTR *vkCmdCopyImage)(VkCommandBuffer, VkImage, VkImageLayout, VkImage, VkImageLayout, uint32_t, const VkImageCopy*) = nullptr;
		void     (VKAPI_PTR *vkCmdBlitImage)(VkCommandBuffer, VkImage, VkImageLayout, VkImage, VkImageLayout, uint32_t, const VkImageBlit*, VkFilter) = nullptr;
		void     (VKAPI_PTR *vkCmdCopyBufferToImage)(VkCommandBuffer, VkBuffer, VkImage, VkImageLayout, uint32_t, const VkBufferImageCopy*) = nullptr;
		void     (VKAPI_PTR *vkCmdCopyImageToBuffer)(VkCommandBuffer, VkImage, VkImageLayout, VkBuffer, uint32_t, const VkBufferImageCopy*) = nullptr;
		void     (VKAPI_PTR *vkCmdPipelineBarrier)(VkCommandBuffer, VkFlags, VkFlags, VkFlags, uint32_t, const void*, uint32_t, const void*, uint32_t, const VkImageMemoryBarrier*) = nullptr;

		VkResult (VKAPI_PTR *vkCreateFence)(VkDevice, const VkFenceCreateInfo*, const void*, VkFence*) = nullptr;
		void     (VKAPI_PTR *vkDestroyFence)(VkDevice, VkFence, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkResetFences)(VkDevice, uint32_t, const VkFence*) = nullptr;
		VkResult (VKAPI_PTR *vkGetFenceStatus)(VkDevice, VkFence) = nullptr;
		VkResult (VKAPI_PTR *vkWaitForFences)(VkDevice, uint32_t, const VkFence*, VkBool32, uint64_t) = nullptr;

		VkResult (VKAPI_PTR *vkCreateSemaphore)(VkDevice, const VkSemaphoreCreateInfo*, const void*, VkSemaphore*) = nullptr;
		void     (VKAPI_PTR *vkDestroySemaphore)(VkDevice, VkSemaphore, const void*) = nullptr;

		VkResult (VKAPI_PTR *vkCreateQueryPool)(VkDevice, const VkQueryPoolCreateInfo*, const void*, VkQueryPool*) = nullptr;
		void     (VKAPI_PTR *vkDestroyQueryPool)(VkDevice, VkQueryPool, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkGetQueryPoolResults)(VkDevice, VkQueryPool, uint32_t, uint32_t, size_t, void*, VkDeviceSize, VkFlags) = nullptr;
		void     (VKAPI_PTR *vkCmdResetQueryPool)(VkCommandBuffer, VkQueryPool, uint32_t, uint32_t) = nullptr;
		void     (VKAPI_PTR *vkCmdBeginQuery)(VkCommandBuffer, VkQueryPool, uint32_t, VkFlags) = nullptr;
		void     (VKAPI_PTR *vkCmdEndQuery)(VkCommandBuffer, VkQueryPool, uint32_t) = nullptr;

		// Swapchain functions
		VkResult (VKAPI_PTR *vkCreateSwapchainKHR)(VkDevice, const VkSwapchainCreateInfoKHR*, const void*, VkSwapchainKHR*) = nullptr;
		void     (VKAPI_PTR *vkDestroySwapchainKHR)(VkDevice, VkSwapchainKHR, const void*) = nullptr;
		VkResult (VKAPI_PTR *vkGetSwapchainImagesKHR)(VkDevice, VkSwapchainKHR, uint32_t*, VkImage*) = nullptr;
		VkResult (VKAPI_PTR *vkAcquireNextImageKHR)(VkDevice, VkSwapchainKHR, uint64_t, VkSemaphore, VkFence, uint32_t*) = nullptr;
		VkResult (VKAPI_PTR *vkQueuePresentKHR)(VkQueue, const VkPresentInfoKHR*) = nullptr;

		bool LoadGlobal();
		bool LoadInstance(VkInstance instance);
		bool LoadDevice(VkDevice device);
	};
}
