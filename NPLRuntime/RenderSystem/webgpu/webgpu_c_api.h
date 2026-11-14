#pragma once
//-----------------------------------------------------------------------------
// webgpu_c_api.h
// Standalone W3C WebGPU C API (webgpu.h) declarations for desktop dynamic
// loading (Dawn / wgpu-native) + direct <webgpu/webgpu.h> inclusion under
// Emscripten (-sUSE_WEBGPU=1). Zero external SDK required on Desktop.
//-----------------------------------------------------------------------------

#include <cstddef>
#include <cstdint>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu.h>
#else

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define WGPU_CALL
#else
#define WGPU_CALL
#endif

typedef uint32_t WGPUBool;
typedef uint32_t WGPUFlags;

typedef struct WGPUAdapterImpl* WGPUAdapter;
typedef struct WGPUBindGroupImpl* WGPUBindGroup;
typedef struct WGPUBindGroupLayoutImpl* WGPUBindGroupLayout;
typedef struct WGPUBufferImpl* WGPUBuffer;
typedef struct WGPUCommandBufferImpl* WGPUCommandBuffer;
typedef struct WGPUCommandEncoderImpl* WGPUCommandEncoder;
typedef struct WGPUDeviceImpl* WGPUDevice;
typedef struct WGPUInstanceImpl* WGPUInstance;
typedef struct WGPUPipelineLayoutImpl* WGPUPipelineLayout;
typedef struct WGPUQuerySetImpl* WGPUQuerySet;
typedef struct WGPUQueueImpl* WGPUQueue;
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;
typedef struct WGPURenderPipelineImpl* WGPURenderPipeline;
typedef struct WGPUSamplerImpl* WGPUSampler;
typedef struct WGPUShaderModuleImpl* WGPUShaderModule;
typedef struct WGPUSurfaceImpl* WGPUSurface;
typedef struct WGPUTextureImpl* WGPUTexture;
typedef struct WGPUTextureViewImpl* WGPUTextureView;

#define WGPU_ARRAY_LAYER_COUNT_UNDEFINED (0xffffffffUL)
#define WGPU_COPY_STRIDE_UNDEFINED       (0xffffffffUL)
#define WGPU_LIMIT_U32_UNDEFINED         (0xffffffffUL)
#define WGPU_LIMIT_U64_UNDEFINED         (0xffffffffffffffffULL)
#define WGPU_MIP_LEVEL_COUNT_UNDEFINED   (0xffffffffUL)
#define WGPU_WHOLE_MAP_SIZE              SIZE_MAX
#define WGPU_WHOLE_SIZE                  (0xffffffffffffffffULL)

typedef enum WGPUSType
{
	WGPUSType_Invalid = 0x00000000,
	WGPUSType_SurfaceDescriptorFromMetalLayer = 0x00000001,
	WGPUSType_SurfaceDescriptorFromWindowsHWND = 0x00000002,
	WGPUSType_SurfaceDescriptorFromXlibWindow = 0x00000003,
	WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector = 0x00000004,
	WGPUSType_ShaderModuleSPIRVDescriptor = 0x00000005,
	WGPUSType_ShaderModuleWGSLDescriptor = 0x00000006,
	WGPUSType_Force32 = 0x7FFFFFFF
} WGPUSType;

typedef struct WGPUChainedStruct
{
	const struct WGPUChainedStruct* next;
	WGPUSType sType;
} WGPUChainedStruct;

typedef struct WGPUChainedStructOut
{
	struct WGPUChainedStructOut* next;
	WGPUSType sType;
} WGPUChainedStructOut;

typedef enum WGPUPowerPreference
{
	WGPUPowerPreference_Undefined = 0x00000000,
	WGPUPowerPreference_LowPower = 0x00000001,
	WGPUPowerPreference_HighPerformance = 0x00000002,
	WGPUPowerPreference_Force32 = 0x7FFFFFFF
} WGPUPowerPreference;

typedef enum WGPURequestAdapterStatus
{
	WGPURequestAdapterStatus_Success = 0x00000000,
	WGPURequestAdapterStatus_Unavailable = 0x00000001,
	WGPURequestAdapterStatus_Error = 0x00000002,
	WGPURequestAdapterStatus_Unknown = 0x00000003,
	WGPURequestAdapterStatus_Force32 = 0x7FFFFFFF
} WGPURequestAdapterStatus;

typedef enum WGPURequestDeviceStatus
{
	WGPURequestDeviceStatus_Success = 0x00000000,
	WGPURequestDeviceStatus_Error = 0x00000001,
	WGPURequestDeviceStatus_Unknown = 0x00000002,
	WGPURequestDeviceStatus_Force32 = 0x7FFFFFFF
} WGPURequestDeviceStatus;

typedef enum WGPUBufferMapAsyncStatus
{
	WGPUBufferMapAsyncStatus_Success = 0x00000000,
	WGPUBufferMapAsyncStatus_ValidationError = 0x00000001,
	WGPUBufferMapAsyncStatus_Unknown = 0x00000002,
	WGPUBufferMapAsyncStatus_DeviceLost = 0x00000003,
	WGPUBufferMapAsyncStatus_DestroyedBeforeCallback = 0x00000004,
	WGPUBufferMapAsyncStatus_UnmappedBeforeCallback = 0x00000005,
	WGPUBufferMapAsyncStatus_MappingAlreadyPending = 0x00000006,
	WGPUBufferMapAsyncStatus_OffsetOutOfRange = 0x00000007,
	WGPUBufferMapAsyncStatus_SizeOutOfRange = 0x00000008,
	WGPUBufferMapAsyncStatus_Force32 = 0x7FFFFFFF
} WGPUBufferMapAsyncStatus;

typedef enum WGPUErrorType
{
	WGPUErrorType_NoError = 0x00000000,
	WGPUErrorType_Validation = 0x00000001,
	WGPUErrorType_OutOfMemory = 0x00000002,
	WGPUErrorType_Internal = 0x00000003,
	WGPUErrorType_Unknown = 0x00000004,
	WGPUErrorType_DeviceLost = 0x00000005,
	WGPUErrorType_Force32 = 0x7FFFFFFF
} WGPUErrorType;

typedef enum WGPUFeatureName
{
	WGPUFeatureName_Undefined = 0x00000000,
	WGPUFeatureName_DepthClipControl = 0x00000001,
	WGPUFeatureName_Depth32FloatStencil8 = 0x00000002,
	WGPUFeatureName_TimestampQuery = 0x00000003,
	WGPUFeatureName_TextureCompressionBC = 0x00000004,
	WGPUFeatureName_TextureCompressionETC2 = 0x00000005,
	WGPUFeatureName_TextureCompressionASTC = 0x00000006,
	WGPUFeatureName_IndirectFirstInstance = 0x00000007,
	WGPUFeatureName_ShaderF16 = 0x00000008,
	WGPUFeatureName_RG11B10UfloatRenderable = 0x00000009,
	WGPUFeatureName_BGRA8UnormStorage = 0x0000000A,
	WGPUFeatureName_Float32Filterable = 0x0000000B,
	WGPUFeatureName_Force32 = 0x7FFFFFFF
} WGPUFeatureName;

typedef enum WGPUTextureFormat
{
	WGPUTextureFormat_Undefined = 0x00000000,
	WGPUTextureFormat_R8Unorm = 0x00000001,
	WGPUTextureFormat_RG8Unorm = 0x00000005,
	WGPUTextureFormat_R16Float = 0x0000000B,
	WGPUTextureFormat_R32Float = 0x0000000C,
	WGPUTextureFormat_RG16Float = 0x00000011,
	WGPUTextureFormat_RGBA8Unorm = 0x00000012,
	WGPUTextureFormat_RGBA8UnormSrgb = 0x00000013,
	WGPUTextureFormat_BGRA8Unorm = 0x00000017,
	WGPUTextureFormat_BGRA8UnormSrgb = 0x00000018,
	WGPUTextureFormat_RG32Float = 0x0000001C,
	WGPUTextureFormat_RGBA16Float = 0x00000021,
	WGPUTextureFormat_RGBA32Float = 0x00000022,
	WGPUTextureFormat_Depth16Unorm = 0x00000027,
	WGPUTextureFormat_Depth24Plus = 0x00000028,
	WGPUTextureFormat_Depth24PlusStencil8 = 0x00000029,
	WGPUTextureFormat_Depth32Float = 0x0000002A,
	WGPUTextureFormat_Depth32FloatStencil8 = 0x0000002B,
	WGPUTextureFormat_BC1RGBAUnorm = 0x0000002C,
	WGPUTextureFormat_BC2RGBAUnorm = 0x0000002E,
	WGPUTextureFormat_BC3RGBAUnorm = 0x00000030,
	WGPUTextureFormat_Force32 = 0x7FFFFFFF
} WGPUTextureFormat;

typedef enum WGPUTextureDimension
{
	WGPUTextureDimension_1D = 0x00000000,
	WGPUTextureDimension_2D = 0x00000001,
	WGPUTextureDimension_3D = 0x00000002,
	WGPUTextureDimension_Force32 = 0x7FFFFFFF
} WGPUTextureDimension;

typedef enum WGPUTextureViewDimension
{
	WGPUTextureViewDimension_Undefined = 0x00000000,
	WGPUTextureViewDimension_1D = 0x00000001,
	WGPUTextureViewDimension_2D = 0x00000002,
	WGPUTextureViewDimension_2DArray = 0x00000003,
	WGPUTextureViewDimension_Cube = 0x00000004,
	WGPUTextureViewDimension_CubeArray = 0x00000005,
	WGPUTextureViewDimension_3D = 0x00000006,
	WGPUTextureViewDimension_Force32 = 0x7FFFFFFF
} WGPUTextureViewDimension;

typedef enum WGPUTextureAspect
{
	WGPUTextureAspect_All = 0x00000000,
	WGPUTextureAspect_StencilOnly = 0x00000001,
	WGPUTextureAspect_DepthOnly = 0x00000002,
	WGPUTextureAspect_Force32 = 0x7FFFFFFF
} WGPUTextureAspect;

typedef enum WGPUTextureSampleType
{
	WGPUTextureSampleType_Undefined = 0x00000000,
	WGPUTextureSampleType_Float = 0x00000001,
	WGPUTextureSampleType_UnfilterableFloat = 0x00000002,
	WGPUTextureSampleType_Depth = 0x00000003,
	WGPUTextureSampleType_Sint = 0x00000004,
	WGPUTextureSampleType_Uint = 0x00000005,
	WGPUTextureSampleType_Force32 = 0x7FFFFFFF
} WGPUTextureSampleType;

typedef enum WGPUSamplerBindingType
{
	WGPUSamplerBindingType_Undefined = 0x00000000,
	WGPUSamplerBindingType_Filtering = 0x00000001,
	WGPUSamplerBindingType_NonFiltering = 0x00000002,
	WGPUSamplerBindingType_Comparison = 0x00000003,
	WGPUSamplerBindingType_Force32 = 0x7FFFFFFF
} WGPUSamplerBindingType;

typedef enum WGPUBufferBindingType
{
	WGPUBufferBindingType_Undefined = 0x00000000,
	WGPUBufferBindingType_Uniform = 0x00000001,
	WGPUBufferBindingType_Storage = 0x00000002,
	WGPUBufferBindingType_ReadOnlyStorage = 0x00000003,
	WGPUBufferBindingType_Force32 = 0x7FFFFFFF
} WGPUBufferBindingType;

typedef enum WGPUStorageTextureAccess
{
	WGPUStorageTextureAccess_Undefined = 0x00000000,
	WGPUStorageTextureAccess_WriteOnly = 0x00000001,
	WGPUStorageTextureAccess_Force32 = 0x7FFFFFFF
} WGPUStorageTextureAccess;

typedef enum WGPUAddressMode
{
	WGPUAddressMode_Repeat = 0x00000000,
	WGPUAddressMode_MirrorRepeat = 0x00000001,
	WGPUAddressMode_ClampToEdge = 0x00000002,
	WGPUAddressMode_Force32 = 0x7FFFFFFF
} WGPUAddressMode;

typedef enum WGPUFilterMode
{
	WGPUFilterMode_Nearest = 0x00000000,
	WGPUFilterMode_Linear = 0x00000001,
	WGPUFilterMode_Force32 = 0x7FFFFFFF
} WGPUFilterMode;

typedef enum WGPUMipmapFilterMode
{
	WGPUMipmapFilterMode_Nearest = 0x00000000,
	WGPUMipmapFilterMode_Linear = 0x00000001,
	WGPUMipmapFilterMode_Force32 = 0x7FFFFFFF
} WGPUMipmapFilterMode;

typedef enum WGPUCompareFunction
{
	WGPUCompareFunction_Undefined = 0x00000000,
	WGPUCompareFunction_Never = 0x00000001,
	WGPUCompareFunction_Less = 0x00000002,
	WGPUCompareFunction_LessEqual = 0x00000003,
	WGPUCompareFunction_Greater = 0x00000004,
	WGPUCompareFunction_GreaterEqual = 0x00000005,
	WGPUCompareFunction_Equal = 0x00000006,
	WGPUCompareFunction_NotEqual = 0x00000007,
	WGPUCompareFunction_Always = 0x00000008,
	WGPUCompareFunction_Force32 = 0x7FFFFFFF
} WGPUCompareFunction;

typedef enum WGPUStencilOperation
{
	WGPUStencilOperation_Keep = 0x00000000,
	WGPUStencilOperation_Zero = 0x00000001,
	WGPUStencilOperation_Replace = 0x00000002,
	WGPUStencilOperation_Invert = 0x00000003,
	WGPUStencilOperation_IncrementClamp = 0x00000004,
	WGPUStencilOperation_DecrementClamp = 0x00000005,
	WGPUStencilOperation_IncrementWrap = 0x00000006,
	WGPUStencilOperation_DecrementWrap = 0x00000007,
	WGPUStencilOperation_Force32 = 0x7FFFFFFF
} WGPUStencilOperation;

typedef enum WGPUBlendFactor
{
	WGPUBlendFactor_Zero = 0x00000000,
	WGPUBlendFactor_One = 0x00000001,
	WGPUBlendFactor_Src = 0x00000002,
	WGPUBlendFactor_OneMinusSrc = 0x00000003,
	WGPUBlendFactor_SrcAlpha = 0x00000004,
	WGPUBlendFactor_OneMinusSrcAlpha = 0x00000005,
	WGPUBlendFactor_Dst = 0x00000006,
	WGPUBlendFactor_OneMinusDst = 0x00000007,
	WGPUBlendFactor_DstAlpha = 0x00000008,
	WGPUBlendFactor_OneMinusDstAlpha = 0x00000009,
	WGPUBlendFactor_SrcAlphaSaturated = 0x0000000A,
	WGPUBlendFactor_Constant = 0x0000000B,
	WGPUBlendFactor_OneMinusConstant = 0x0000000C,
	WGPUBlendFactor_Force32 = 0x7FFFFFFF
} WGPUBlendFactor;

typedef enum WGPUBlendOperation
{
	WGPUBlendOperation_Add = 0x00000000,
	WGPUBlendOperation_Subtract = 0x00000001,
	WGPUBlendOperation_ReverseSubtract = 0x00000002,
	WGPUBlendOperation_Min = 0x00000003,
	WGPUBlendOperation_Max = 0x00000004,
	WGPUBlendOperation_Force32 = 0x7FFFFFFF
} WGPUBlendOperation;

typedef enum WGPUPrimitiveTopology
{
	WGPUPrimitiveTopology_PointList = 0x00000000,
	WGPUPrimitiveTopology_LineList = 0x00000001,
	WGPUPrimitiveTopology_LineStrip = 0x00000002,
	WGPUPrimitiveTopology_TriangleList = 0x00000003,
	WGPUPrimitiveTopology_TriangleStrip = 0x00000004,
	WGPUPrimitiveTopology_Force32 = 0x7FFFFFFF
} WGPUPrimitiveTopology;

typedef enum WGPUFrontFace
{
	WGPUFrontFace_CCW = 0x00000000,
	WGPUFrontFace_CW = 0x00000001,
	WGPUFrontFace_Force32 = 0x7FFFFFFF
} WGPUFrontFace;

typedef enum WGPUCullMode
{
	WGPUCullMode_None = 0x00000000,
	WGPUCullMode_Front = 0x00000001,
	WGPUCullMode_Back = 0x00000002,
	WGPUCullMode_Force32 = 0x7FFFFFFF
} WGPUCullMode;

typedef enum WGPUIndexFormat
{
	WGPUIndexFormat_Undefined = 0x00000000,
	WGPUIndexFormat_Uint16 = 0x00000001,
	WGPUIndexFormat_Uint32 = 0x00000002,
	WGPUIndexFormat_Force32 = 0x7FFFFFFF
} WGPUIndexFormat;

typedef enum WGPUVertexStepMode
{
	WGPUVertexStepMode_Vertex = 0x00000000,
	WGPUVertexStepMode_Instance = 0x00000001,
	WGPUVertexStepMode_VertexBufferNotUsed = 0x00000002,
	WGPUVertexStepMode_Force32 = 0x7FFFFFFF
} WGPUVertexStepMode;

typedef enum WGPUVertexFormat
{
	WGPUVertexFormat_Undefined = 0x00000000,
	WGPUVertexFormat_Uint8x2 = 0x00000001,
	WGPUVertexFormat_Uint8x4 = 0x00000002,
	WGPUVertexFormat_Sint8x2 = 0x00000003,
	WGPUVertexFormat_Sint8x4 = 0x00000004,
	WGPUVertexFormat_Unorm8x2 = 0x00000005,
	WGPUVertexFormat_Unorm8x4 = 0x00000006,
	WGPUVertexFormat_Snorm8x2 = 0x00000007,
	WGPUVertexFormat_Snorm8x4 = 0x00000008,
	WGPUVertexFormat_Uint16x2 = 0x00000009,
	WGPUVertexFormat_Uint16x4 = 0x0000000A,
	WGPUVertexFormat_Sint16x2 = 0x0000000B,
	WGPUVertexFormat_Sint16x4 = 0x0000000C,
	WGPUVertexFormat_Unorm16x2 = 0x0000000D,
	WGPUVertexFormat_Unorm16x4 = 0x0000000E,
	WGPUVertexFormat_Snorm16x2 = 0x0000000F,
	WGPUVertexFormat_Snorm16x4 = 0x00000010,
	WGPUVertexFormat_Float16x2 = 0x00000011,
	WGPUVertexFormat_Float16x4 = 0x00000012,
	WGPUVertexFormat_Float32 = 0x00000013,
	WGPUVertexFormat_Float32x2 = 0x00000014,
	WGPUVertexFormat_Float32x3 = 0x00000015,
	WGPUVertexFormat_Float32x4 = 0x00000016,
	WGPUVertexFormat_Uint32 = 0x00000017,
	WGPUVertexFormat_Uint32x2 = 0x00000018,
	WGPUVertexFormat_Uint32x3 = 0x00000019,
	WGPUVertexFormat_Uint32x4 = 0x0000001A,
	WGPUVertexFormat_Sint32 = 0x0000001B,
	WGPUVertexFormat_Sint32x2 = 0x0000001C,
	WGPUVertexFormat_Sint32x3 = 0x0000001D,
	WGPUVertexFormat_Sint32x4 = 0x0000001E,
	WGPUVertexFormat_Unorm10_10_10_2 = 0x0000001F,
	WGPUVertexFormat_Unorm8x4BGRA = 0x00000020,
	WGPUVertexFormat_Force32 = 0x7FFFFFFF
} WGPUVertexFormat;

typedef enum WGPULoadOp
{
	WGPULoadOp_Undefined = 0x00000000,
	WGPULoadOp_Clear = 0x00000001,
	WGPULoadOp_Load = 0x00000002,
	WGPULoadOp_Force32 = 0x7FFFFFFF
} WGPULoadOp;

typedef enum WGPUStoreOp
{
	WGPUStoreOp_Undefined = 0x00000000,
	WGPUStoreOp_Store = 0x00000001,
	WGPUStoreOp_Discard = 0x00000002,
	WGPUStoreOp_Force32 = 0x7FFFFFFF
} WGPUStoreOp;

typedef enum WGPUQueryType
{
	WGPUQueryType_Occlusion = 0x00000000,
	WGPUQueryType_Timestamp = 0x00000001,
	WGPUQueryType_Force32 = 0x7FFFFFFF
} WGPUQueryType;

typedef enum WGPUPresentMode
{
	WGPUPresentMode_Fifo = 0x00000000,
	WGPUPresentMode_FifoRelaxed = 0x00000001,
	WGPUPresentMode_Immediate = 0x00000002,
	WGPUPresentMode_Mailbox = 0x00000003,
	WGPUPresentMode_Force32 = 0x7FFFFFFF
} WGPUPresentMode;

typedef enum WGPUCompositeAlphaMode
{
	WGPUCompositeAlphaMode_Auto = 0x00000000,
	WGPUCompositeAlphaMode_Opaque = 0x00000001,
	WGPUCompositeAlphaMode_Premultiplied = 0x00000002,
	WGPUCompositeAlphaMode_Unpremultiplied = 0x00000003,
	WGPUCompositeAlphaMode_Inherit = 0x00000004,
	WGPUCompositeAlphaMode_Force32 = 0x7FFFFFFF
} WGPUCompositeAlphaMode;

typedef enum WGPUSurfaceGetCurrentTextureStatus
{
	WGPUSurfaceGetCurrentTextureStatus_Success = 0x00000000,
	WGPUSurfaceGetCurrentTextureStatus_Timeout = 0x00000001,
	WGPUSurfaceGetCurrentTextureStatus_Outdated = 0x00000002,
	WGPUSurfaceGetCurrentTextureStatus_Lost = 0x00000003,
	WGPUSurfaceGetCurrentTextureStatus_OutOfMemory = 0x00000004,
	WGPUSurfaceGetCurrentTextureStatus_DeviceLost = 0x00000005,
	WGPUSurfaceGetCurrentTextureStatus_Force32 = 0x7FFFFFFF
} WGPUSurfaceGetCurrentTextureStatus;

// Bitflags
typedef WGPUFlags WGPUBufferUsageFlags;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_None         = 0x00000000;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_MapRead      = 0x00000001;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_MapWrite     = 0x00000002;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_CopySrc      = 0x00000004;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_CopyDst      = 0x00000008;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_Index        = 0x00000010;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_Vertex       = 0x00000020;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_Uniform      = 0x00000040;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_Storage      = 0x00000080;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_Indirect     = 0x00000100;
static constexpr WGPUBufferUsageFlags WGPUBufferUsage_QueryResolve = 0x00000200;

typedef WGPUFlags WGPUMapModeFlags;
static constexpr WGPUMapModeFlags WGPUMapMode_None  = 0x00000000;
static constexpr WGPUMapModeFlags WGPUMapMode_Read  = 0x00000001;
static constexpr WGPUMapModeFlags WGPUMapMode_Write = 0x00000002;

typedef WGPUFlags WGPUTextureUsageFlags;
static constexpr WGPUTextureUsageFlags WGPUTextureUsage_None             = 0x00000000;
static constexpr WGPUTextureUsageFlags WGPUTextureUsage_CopySrc          = 0x00000001;
static constexpr WGPUTextureUsageFlags WGPUTextureUsage_CopyDst          = 0x00000002;
static constexpr WGPUTextureUsageFlags WGPUTextureUsage_TextureBinding   = 0x00000004;
static constexpr WGPUTextureUsageFlags WGPUTextureUsage_StorageBinding   = 0x00000008;
static constexpr WGPUTextureUsageFlags WGPUTextureUsage_RenderAttachment = 0x00000010;

typedef WGPUFlags WGPUShaderStageFlags;
static constexpr WGPUShaderStageFlags WGPUShaderStage_None     = 0x00000000;
static constexpr WGPUShaderStageFlags WGPUShaderStage_Vertex   = 0x00000001;
static constexpr WGPUShaderStageFlags WGPUShaderStage_Fragment = 0x00000002;
static constexpr WGPUShaderStageFlags WGPUShaderStage_Compute  = 0x00000004;

typedef WGPUFlags WGPUColorWriteMaskFlags;
static constexpr WGPUColorWriteMaskFlags WGPUColorWriteMask_None  = 0x00000000;
static constexpr WGPUColorWriteMaskFlags WGPUColorWriteMask_Red   = 0x00000001;
static constexpr WGPUColorWriteMaskFlags WGPUColorWriteMask_Green = 0x00000002;
static constexpr WGPUColorWriteMaskFlags WGPUColorWriteMask_Blue  = 0x00000004;
static constexpr WGPUColorWriteMaskFlags WGPUColorWriteMask_Alpha = 0x00000008;
static constexpr WGPUColorWriteMaskFlags WGPUColorWriteMask_All   = 0x0000000F;

// Structs
typedef struct WGPUColor
{
	double r;
	double g;
	double b;
	double a;
} WGPUColor;

typedef struct WGPUOrigin3D
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
} WGPUOrigin3D;

typedef struct WGPUExtent3D
{
	uint32_t width;
	uint32_t height;
	uint32_t depthOrArrayLayers;
} WGPUExtent3D;

typedef struct WGPUInstanceDescriptor
{
	const WGPUChainedStruct* nextInChain;
} WGPUInstanceDescriptor;

typedef struct WGPUSurfaceDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
} WGPUSurfaceDescriptor;

typedef struct WGPUSurfaceDescriptorFromWindowsHWND
{
	WGPUChainedStruct chain;
	void* hinstance;
	void* hwnd;
} WGPUSurfaceDescriptorFromWindowsHWND;

typedef struct WGPUSurfaceDescriptorFromXlibWindow
{
	WGPUChainedStruct chain;
	void* display;
	uint64_t window;
} WGPUSurfaceDescriptorFromXlibWindow;

typedef struct WGPUSurfaceDescriptorFromMetalLayer
{
	WGPUChainedStruct chain;
	void* layer;
} WGPUSurfaceDescriptorFromMetalLayer;

typedef struct WGPUSurfaceDescriptorFromCanvasHTMLSelector
{
	WGPUChainedStruct chain;
	const char* selector;
} WGPUSurfaceDescriptorFromCanvasHTMLSelector;

typedef struct WGPURequestAdapterOptions
{
	const WGPUChainedStruct* nextInChain;
	WGPUSurface compatibleSurface;
	WGPUPowerPreference powerPreference;
	uint32_t backendType;
	WGPUBool forceFallbackAdapter;
} WGPURequestAdapterOptions;

typedef struct WGPUQueueDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
} WGPUQueueDescriptor;

typedef void (*WGPUErrorCallback)(WGPUErrorType type, const char* message, void* userdata);
typedef void (*WGPUDeviceLostCallback)(uint32_t reason, const char* message, void* userdata);

typedef struct WGPUDeviceDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	size_t requiredFeatureCount;
	const WGPUFeatureName* requiredFeatures;
	const void* requiredLimits;
	WGPUQueueDescriptor defaultQueue;
	WGPUDeviceLostCallback deviceLostCallback;
	void* deviceLostUserdata;
} WGPUDeviceDescriptor;

typedef struct WGPUSurfaceConfiguration
{
	const WGPUChainedStruct* nextInChain;
	WGPUDevice device;
	WGPUTextureFormat format;
	WGPUTextureUsageFlags usage;
	size_t viewFormatCount;
	const WGPUTextureFormat* viewFormats;
	WGPUCompositeAlphaMode alphaMode;
	uint32_t width;
	uint32_t height;
	WGPUPresentMode presentMode;
} WGPUSurfaceConfiguration;

typedef struct WGPUSurfaceTexture
{
	WGPUTexture texture;
	WGPUBool suboptimal;
	WGPUSurfaceGetCurrentTextureStatus status;
} WGPUSurfaceTexture;

typedef struct WGPUBufferDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	WGPUBufferUsageFlags usage;
	uint64_t size;
	WGPUBool mappedAtCreation;
} WGPUBufferDescriptor;

typedef struct WGPUTextureDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	WGPUTextureUsageFlags usage;
	WGPUTextureDimension dimension;
	WGPUExtent3D size;
	WGPUTextureFormat format;
	uint32_t mipLevelCount;
	uint32_t sampleCount;
	size_t viewFormatCount;
	const WGPUTextureFormat* viewFormats;
} WGPUTextureDescriptor;

typedef struct WGPUTextureViewDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	WGPUTextureFormat format;
	WGPUTextureViewDimension dimension;
	uint32_t baseMipLevel;
	uint32_t mipLevelCount;
	uint32_t baseArrayLayer;
	uint32_t arrayLayerCount;
	WGPUTextureAspect aspect;
} WGPUTextureViewDescriptor;

typedef struct WGPUTextureDataLayout
{
	const WGPUChainedStruct* nextInChain;
	uint64_t offset;
	uint32_t bytesPerRow;
	uint32_t rowsPerImage;
} WGPUTextureDataLayout;

typedef struct WGPUImageCopyTexture
{
	const WGPUChainedStruct* nextInChain;
	WGPUTexture texture;
	uint32_t mipLevel;
	WGPUOrigin3D origin;
	WGPUTextureAspect aspect;
} WGPUImageCopyTexture;

typedef struct WGPUImageCopyBuffer
{
	const WGPUChainedStruct* nextInChain;
	WGPUTextureDataLayout layout;
	WGPUBuffer buffer;
} WGPUImageCopyBuffer;

typedef struct WGPUSamplerDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	WGPUAddressMode addressModeU;
	WGPUAddressMode addressModeV;
	WGPUAddressMode addressModeW;
	WGPUFilterMode magFilter;
	WGPUFilterMode minFilter;
	WGPUMipmapFilterMode mipmapFilter;
	float lodMinClamp;
	float lodMaxClamp;
	WGPUCompareFunction compare;
	uint16_t maxAnisotropy;
} WGPUSamplerDescriptor;

typedef struct WGPUBufferBindingLayout
{
	const WGPUChainedStruct* nextInChain;
	WGPUBufferBindingType type;
	WGPUBool hasDynamicOffset;
	uint64_t minBindingSize;
} WGPUBufferBindingLayout;

typedef struct WGPUSamplerBindingLayout
{
	const WGPUChainedStruct* nextInChain;
	WGPUSamplerBindingType type;
} WGPUSamplerBindingLayout;

typedef struct WGPUTextureBindingLayout
{
	const WGPUChainedStruct* nextInChain;
	WGPUTextureSampleType sampleType;
	WGPUTextureViewDimension viewDimension;
	WGPUBool multisampled;
} WGPUTextureBindingLayout;

typedef struct WGPUStorageTextureBindingLayout
{
	const WGPUChainedStruct* nextInChain;
	WGPUStorageTextureAccess access;
	WGPUTextureFormat format;
	WGPUTextureViewDimension viewDimension;
} WGPUStorageTextureBindingLayout;

typedef struct WGPUBindGroupLayoutEntry
{
	const WGPUChainedStruct* nextInChain;
	uint32_t binding;
	WGPUShaderStageFlags visibility;
	WGPUBufferBindingLayout buffer;
	WGPUSamplerBindingLayout sampler;
	WGPUTextureBindingLayout texture;
	WGPUStorageTextureBindingLayout storageTexture;
} WGPUBindGroupLayoutEntry;

typedef struct WGPUBindGroupLayoutDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	size_t entryCount;
	const WGPUBindGroupLayoutEntry* entries;
} WGPUBindGroupLayoutDescriptor;

typedef struct WGPUBindGroupEntry
{
	const WGPUChainedStruct* nextInChain;
	uint32_t binding;
	WGPUBuffer buffer;
	uint64_t offset;
	uint64_t size;
	WGPUSampler sampler;
	WGPUTextureView textureView;
} WGPUBindGroupEntry;

typedef struct WGPUBindGroupDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	WGPUBindGroupLayout layout;
	size_t entryCount;
	const WGPUBindGroupEntry* entries;
} WGPUBindGroupDescriptor;

typedef struct WGPUPipelineLayoutDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	size_t bindGroupLayoutCount;
	const WGPUBindGroupLayout* bindGroupLayouts;
} WGPUPipelineLayoutDescriptor;

typedef struct WGPUShaderModuleWGSLDescriptor
{
	WGPUChainedStruct chain;
	const char* code;
} WGPUShaderModuleWGSLDescriptor;

typedef struct WGPUShaderModuleDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
} WGPUShaderModuleDescriptor;

typedef struct WGPUConstantEntry
{
	const WGPUChainedStruct* nextInChain;
	const char* key;
	double value;
} WGPUConstantEntry;

typedef struct WGPUVertexAttribute
{
	WGPUVertexFormat format;
	uint64_t offset;
	uint32_t shaderLocation;
} WGPUVertexAttribute;

typedef struct WGPUVertexBufferLayout
{
	uint64_t arrayStride;
	WGPUVertexStepMode stepMode;
	size_t attributeCount;
	const WGPUVertexAttribute* attributes;
} WGPUVertexBufferLayout;

typedef struct WGPUVertexState
{
	const WGPUChainedStruct* nextInChain;
	WGPUShaderModule module;
	const char* entryPoint;
	size_t constantCount;
	const WGPUConstantEntry* constants;
	size_t bufferCount;
	const WGPUVertexBufferLayout* buffers;
} WGPUVertexState;

typedef struct WGPUPrimitiveState
{
	const WGPUChainedStruct* nextInChain;
	WGPUPrimitiveTopology topology;
	WGPUIndexFormat stripIndexFormat;
	WGPUFrontFace frontFace;
	WGPUCullMode cullMode;
} WGPUPrimitiveState;

typedef struct WGPUStencilFaceState
{
	WGPUCompareFunction compare;
	WGPUStencilOperation failOp;
	WGPUStencilOperation depthFailOp;
	WGPUStencilOperation passOp;
} WGPUStencilFaceState;

typedef struct WGPUDepthStencilState
{
	const WGPUChainedStruct* nextInChain;
	WGPUTextureFormat format;
	WGPUBool depthWriteEnabled;
	WGPUCompareFunction depthCompare;
	WGPUStencilFaceState stencilFront;
	WGPUStencilFaceState stencilBack;
	uint32_t stencilReadMask;
	uint32_t stencilWriteMask;
	int32_t depthBias;
	float depthBiasSlopeScale;
	float depthBiasClamp;
} WGPUDepthStencilState;

typedef struct WGPUMultisampleState
{
	const WGPUChainedStruct* nextInChain;
	uint32_t count;
	uint32_t mask;
	WGPUBool alphaToCoverageEnabled;
} WGPUMultisampleState;

typedef struct WGPUBlendComponent
{
	WGPUBlendOperation operation;
	WGPUBlendFactor srcFactor;
	WGPUBlendFactor dstFactor;
} WGPUBlendComponent;

typedef struct WGPUBlendState
{
	WGPUBlendComponent color;
	WGPUBlendComponent alpha;
} WGPUBlendState;

typedef struct WGPUColorTargetState
{
	const WGPUChainedStruct* nextInChain;
	WGPUTextureFormat format;
	const WGPUBlendState* blend;
	WGPUColorWriteMaskFlags writeMask;
} WGPUColorTargetState;

typedef struct WGPUFragmentState
{
	const WGPUChainedStruct* nextInChain;
	WGPUShaderModule module;
	const char* entryPoint;
	size_t constantCount;
	const WGPUConstantEntry* constants;
	size_t targetCount;
	const WGPUColorTargetState* targets;
} WGPUFragmentState;

typedef struct WGPURenderPipelineDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	WGPUPipelineLayout layout;
	WGPUVertexState vertex;
	WGPUPrimitiveState primitive;
	const WGPUDepthStencilState* depthStencil;
	WGPUMultisampleState multisample;
	const WGPUFragmentState* fragment;
} WGPURenderPipelineDescriptor;

typedef struct WGPURenderPassColorAttachment
{
	const WGPUChainedStruct* nextInChain;
	WGPUTextureView view;
	uint32_t depthSlice;
	WGPUTextureView resolveTarget;
	WGPULoadOp loadOp;
	WGPUStoreOp storeOp;
	WGPUColor clearValue;
} WGPURenderPassColorAttachment;

typedef struct WGPURenderPassDepthStencilAttachment
{
	WGPUTextureView view;
	WGPULoadOp depthLoadOp;
	WGPUStoreOp depthStoreOp;
	float depthClearValue;
	WGPUBool depthReadOnly;
	WGPULoadOp stencilLoadOp;
	WGPUStoreOp stencilStoreOp;
	uint32_t stencilClearValue;
	WGPUBool stencilReadOnly;
} WGPURenderPassDepthStencilAttachment;

typedef struct WGPURenderPassDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	size_t colorAttachmentCount;
	const WGPURenderPassColorAttachment* colorAttachments;
	const WGPURenderPassDepthStencilAttachment* depthStencilAttachment;
	WGPUQuerySet occlusionQuerySet;
	const void* timestampWrites;
} WGPURenderPassDescriptor;

typedef struct WGPUCommandEncoderDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
} WGPUCommandEncoderDescriptor;

typedef struct WGPUCommandBufferDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
} WGPUCommandBufferDescriptor;

typedef struct WGPUQuerySetDescriptor
{
	const WGPUChainedStruct* nextInChain;
	const char* label;
	WGPUQueryType type;
	uint32_t count;
} WGPUQuerySetDescriptor;

typedef struct WGPUAdapterProperties
{
	WGPUChainedStructOut* nextInChain;
	uint32_t vendorID;
	const char* vendorName;
	const char* architecture;
	uint32_t deviceID;
	const char* name;
	const char* driverDescription;
	uint32_t adapterType;
	uint32_t backendType;
} WGPUAdapterProperties;

typedef void (*WGPURequestAdapterCallback)(WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userdata);
typedef void (*WGPURequestDeviceCallback)(WGPURequestDeviceStatus status, WGPUDevice device, const char* message, void* userdata);
typedef void (*WGPUBufferMapCallback)(WGPUBufferMapAsyncStatus status, void* userdata);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !__EMSCRIPTEN__
