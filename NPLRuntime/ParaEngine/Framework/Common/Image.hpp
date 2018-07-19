#pragma once;

#include <cstdint>
#include <memory>
namespace ParaEngine
{

	

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
			IPF_B8G8R8A8
		};


		Image()
			:data(nullptr)
			,width(0)
			,height(0)
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
			width = 0;
			height = 0;
			data_size = 0;
			Format = IPF_R8G8B8A8;
		}
	public:
		void* data;
		uint32_t width;
		uint32_t height;
		size_t data_size;
		EImagePixelFormat Format;
	};

	using ImagePtr = std::shared_ptr<Image>;
}