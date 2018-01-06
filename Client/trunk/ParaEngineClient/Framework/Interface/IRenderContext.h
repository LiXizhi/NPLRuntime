#pragma once;
#include <vector>
#include <memory>

namespace ParaEngine
{

	class IRenderDevice;
	class IRenderWindow;

	enum class DeviceType
	{
		HAL = 1,
		REF = 2,
		SW = 3,
		NullRef = 4
	};

	/* Formats
	* Most of these names have the following convention:
	*      A = Alpha
	*      R = Red
	*      G = Green
	*      B = Blue
	*      X = Unused Bits
	*      P = Palette
	*      L = Luminance
	*      U = dU coordinate for BumpMap
	*      V = dV coordinate for BumpMap
	*      S = Stencil
	*      D = Depth (e.g. Z or W buffer)
	*      C = Computed from other channels (typically on certain read operations)
	*
	*      Further, the order of the pieces are from MSB first; hence
	*      A8L8 indicates that the high byte of this two byte
	*      format is alpha.
	*
	*      D16_LOCKABLE indicates:
	*           - An integer 16-bit value.
	*           - An app-lockable surface.
	*
	*      D32F_LOCKABLE indicates:
	*           - An IEEE 754 floating-point value.
	*           - An app-lockable surface.
	*
	*      All Depth/Stencil formats except D16_LOCKABLE and D32F_LOCKABLE indicate:
	*          - no particular bit ordering per pixel, and
	*          - are not app lockable, and
	*          - the driver is allowed to consume more than the indicated
	*            number of bits per Depth channel (but not Stencil channel).
	*/
	enum class PixelFormat
	{
		Unkonwn,
		R8G8B8 ,
		A8R8G8B8,
		X8R8G8B8 ,
		R5G6B5,
		X1R5G5B5,
		A1R5G5B5,
		A4R4G4B4,
		R3G3B2,
		A8,
		A8R3G3B2,
		X4R4G4B4,
		A2B10G10R10,
		A8B8G8R8,

		X8B8G8R8,
		G16R16,
		A2R10G10B10,
		A16B16G16R16,

		A8P8,
		P8,

		L8,
		A8L8,
		A4L4,

		V8U8,
		L6V5U5,
		X8L8V8U8,
		Q8W8V8U8,
		V16U16,
		A2W10V10U10,

		UYVY,
		R8G8_B8G8 ,
		YUY2,
		G8R8_G8B8,
		DXT1,
		DXT2,
		DXT3,
		DXT4,
		DXT5,


		D16_LOCKABLE,
		D32,
		D15S1,
		D24S8,
		D24X8,
		D24X4S4,
		D16,
		D32F_LOCKABLE,
		D24FS8,
		COUNT
	};

	enum class MultiSampleType
	{
		None = 0,
		NONMASKABLE = 1,
		Samples_2 = 2,
		Samples_3 = 3,
		Samples_4 = 4,
		Samples_5 = 5,
		Samples_6 = 6,
		Samples_7 = 7,
		Samples_8 = 8,
		Samples_9 = 9,
		Samples_10 = 10,
		Samples_11 = 11,
		Samples_12 = 12,
		Samples_13 = 13,
		Samples_14 = 14,
		Samples_15 = 15,
		Samples_16 = 16,
	};

	enum class PresentInterval
	{
		Default,
		Immediate,
		One,
		Two,
	};


	enum class SwapEffectType
	{
		Discard,
		Flip,
		Copy
	};
	
	enum class VertexProcessing
	{
		Software,
		Hardware,
		Mixed
	};

	struct RenderDeviceConfiguration
	{
		bool isWindowed;
		PixelFormat colorFormat;
		PixelFormat depthStencilFormat;
		uint32_t msaaSamples; ///< MSAA samples
		uint32_t screenWidth;
		uint32_t screenHeight;

		IRenderWindow* renderWindow;
		
		RenderDeviceConfiguration()
			:isWindowed(true)
			,colorFormat(PixelFormat::A8R8G8B8)
			,depthStencilFormat(PixelFormat::D24S8)
			,msaaSamples(0)
			,screenWidth(1024)
			,screenHeight(768)
			,renderWindow(nullptr)

		{

		}

	};

	class IRenderContext
	{
	public: 
		IRenderContext() = default;
		virtual ~IRenderContext() = default;
		virtual IRenderDevice* CreateDevice(const RenderDeviceConfiguration& cfg) = 0;
	};
	using IRenderContextPtr = std::shared_ptr<IRenderContext>;
}
