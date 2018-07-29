#pragma once;

#include <cstdint>
#include <memory>
#include <vector>
namespace ParaEngine
{

	struct ImageMipmap
	{
		uint32_t offset;
		uint32_t size;
		uint32_t width;
		uint32_t height;
		ImageMipmap()
			:offset(0)
			,size(0)
			,width(0)
			,height(0)
		{

		}
	};

	class Image
	{
	public:

		enum EImagePixelFormat
		{
			IPF_A8,//ALPHA
			IPF_L8,//GRAY
			IPF_A8L8,//ALPHA,GRAY
			IPF_R8G8B8,
			IPF_R8G8B8A8,
			IPF_B8G8R8A8,
			IPF_COMPRESSED_DXT1,
			IPF_COMPRESSED_DXT3,
			IPF_COMPRESSED_DXT5
		};


		Image()
			:data(nullptr)
			,data_size(0)
			,Format(IPF_R8G8B8A8)
		{

		}
		~Image()
		{
			if (data)
			{
				delete[] data;
				data = nullptr;
			}
			data_size = 0;
			Format = IPF_R8G8B8A8;
			mipmaps.clear();
		}

		static bool IsCompressedImageFormat(EImagePixelFormat format);
	public:
		void* data;
		size_t data_size;
		EImagePixelFormat Format;
		std::vector<ImageMipmap> mipmaps;
	};

	bool Image::IsCompressedImageFormat(EImagePixelFormat format)
	{
		if (format == Image::IPF_COMPRESSED_DXT1 ||
			format == Image::IPF_COMPRESSED_DXT3 ||
			format == Image::IPF_COMPRESSED_DXT5)
		{
			return true;
		}
		else {
			return false;
		}
	}

	using ImagePtr = std::shared_ptr<Image>;
}