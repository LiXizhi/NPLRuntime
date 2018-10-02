#include "ParaEngine.h"
#include "S3TCParser.h"
#include "s3tc.h"


struct DDColorKey
{
	uint32_t colorSpaceLowValue;
	uint32_t colorSpaceHighValue;
};

struct DDSCaps
{
	uint32_t caps;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
};

struct DDPixelFormat
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourCC;
	uint32_t RGBBitCount;
	uint32_t RBitMask;
	uint32_t GBitMask;
	uint32_t BBitMask;
	uint32_t ABitMask;
};


struct DDSURFACEDESC2
{
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;

	union
	{
		uint32_t pitch;
		uint32_t linearSize;
	} DUMMYUNIONNAMEN1;

	union
	{
		uint32_t backBufferCount;
		uint32_t depth;
	} DUMMYUNIONNAMEN5;

	union
	{
		uint32_t mipMapCount;
		uint32_t refreshRate;
		uint32_t srcVBHandle;
	} DUMMYUNIONNAMEN2;

	uint32_t alphaBitDepth;
	uint32_t reserved;
	uint32_t surface;

	union
	{
		DDColorKey ddckCKDestOverlay;
		uint32_t emptyFaceColor;
	} DUMMYUNIONNAMEN3;

	DDColorKey ddckCKDestBlt;
	DDColorKey ddckCKSrcOverlay;
	DDColorKey ddckCKSrcBlt;

	union
	{
		DDPixelFormat ddpfPixelFormat;
		uint32_t FVF;
	} DUMMYUNIONNAMEN4;

	DDSCaps ddsCaps;
	uint32_t textureStage;
};

#pragma pack(push,1)

struct S3TCTexHeader
{
	char fileCode[4];
	DDSURFACEDESC2 ddsd;
};
#pragma pack(pop)

using namespace ParaEngine;
using namespace IParaEngine;


uint32_t make_four_cc(char ch0, char ch1, char ch2, char ch3)
{
	const uint32_t fourCC = ((uint32_t)(char)(ch0) | ((uint32_t)(char)(ch1) << 8) | ((uint32_t)(char)(ch2) << 16) | ((uint32_t)(char)(ch3) << 24));
	return fourCC;
}


ImagePtr soft_decode(const unsigned char* buffer, uint32_t buffer_size, S3TCDecodeFlag s3tc_format,uint32_t width,uint32_t height,uint32_t mipmaps)
{
	uint32_t blockSize = s3tc_format == S3TCDecodeFlag::DXT1 ? 8 : 16;



	ImagePtr img = std::make_shared<Image>();
	uint32_t mipmap_width = width;
	uint32_t mipmap_height = height;
	uint32_t mipmap_offset = 0;
	// calc data size
	for (int i =0;i<mipmaps;i++)
	{
		if (mipmap_width == 0) mipmap_width = 1;
		if (mipmap_height == 0) mipmap_height = 1;

		uint32_t bpp = 4;
		uint32_t pitch = mipmap_width * bpp;
		uint32_t mipmap_size = height * pitch;

		ImageMipmap mipmap;
		mipmap.width = mipmap_width;
		mipmap.height = mipmap_height;
		mipmap.offset = mipmap_offset;
		img->mipmaps.push_back(mipmap);

		// next
		mipmap_offset += mipmap_size;
		mipmap_width /= 2;
		mipmap_height /= 2;
	}
	img->data_size = mipmap_offset;
	img->data = new unsigned char[img->data_size];

	uint32_t offset = 0;
	for ( int i =0;i<img->mipmaps.size();i++)
	{
		ImageMipmap& mipmap = img->mipmaps[i];
		uint32_t size = ((mipmap.width + 3) / 4)*((mipmap.height + 3) / 4)*blockSize;
		s3tc_decode(buffer + offset,((unsigned char*)img->data)+mipmap.offset, mipmap.width, mipmap.height, s3tc_format);
		offset += size;
	}
	img->Format = Image::IPF_R8G8B8A8;
	return img;
}
ImagePtr hardware_decode(const unsigned char* buffer,uint32_t buffer_size,S3TCDecodeFlag s3tc_format, uint32_t width, uint32_t height, uint32_t mipmaps)
{
	uint32_t blockSize = s3tc_format == S3TCDecodeFlag::DXT1 ? 8 : 16;
	uint32_t data_size = buffer_size - sizeof(S3TCTexHeader);
	unsigned char* data = new unsigned char[data_size];
	memcpy(data, buffer + sizeof(S3TCTexHeader), data_size);
	ImagePtr img = std::make_shared<Image>();
	img->data = data;
	img->data_size = data_size;
	uint32_t mipmap_width = width;
	uint32_t mipmap_height = height;
	uint32_t mipmap_offset = 0;
	for ( int i = 0;i<mipmaps && (mipmap_width || mipmap_height);i++)
	{
		if (mipmap_width == 0) mipmap_width = 1;
		if (mipmap_height == 0) mipmap_height = 1;

		ImageMipmap mipmap;
		uint32_t mipmap_size = ((mipmap_width + 3) / 4)*((mipmap_height + 3) / 4)*blockSize;
		mipmap.width = mipmap_width;
		mipmap.height = mipmap_height;
		mipmap.size = mipmap_size;
		mipmap.offset = mipmap_offset;
		img->mipmaps.push_back(mipmap);
		
		// next
		mipmap_offset += mipmap_size;
		mipmap_width /= 2;
		mipmap_height /= 2;

	}
	switch (s3tc_format)
	{
	case S3TCDecodeFlag::DXT1:
		img->Format = Image::IPF_COMPRESSED_DXT1;
		break;
	case S3TCDecodeFlag::DXT3:
		img->Format = Image::IPF_COMPRESSED_DXT3;
		break;
	case S3TCDecodeFlag::DXT5:
		img->Format = Image::IPF_COMPRESSED_DXT5;
		break;
	default:
		break;
	}
	return img;
}

ImagePtr S3TCParser::Parse(const unsigned char* buffer, size_t buffer_size)
{
	const uint32_t FOURCC_DXT1 = make_four_cc('D', 'X', 'T', '1');
	const uint32_t FOURCC_DXT3 = make_four_cc('D', 'X', 'T', '3');
	const uint32_t FOURCC_DXT5 = make_four_cc('D', 'X', 'T', '5');

	S3TCTexHeader *header = (S3TCTexHeader *)buffer;

	uint32_t img_width = header->ddsd.width;
	uint32_t img_height = header->ddsd.height;
	uint32_t img_mipmaps = header->ddsd.DUMMYUNIONNAMEN2.mipMapCount <=0 ? 1  : header->ddsd.DUMMYUNIONNAMEN2.mipMapCount;
	S3TCDecodeFlag img_fmt = S3TCDecodeFlag::DXT1;
	uint32_t fourcc = header->ddsd.DUMMYUNIONNAMEN4.ddpfPixelFormat.fourCC;
	if (fourcc == FOURCC_DXT1){
		img_fmt = S3TCDecodeFlag::DXT1;
	}else if (fourcc == FOURCC_DXT3){
		img_fmt = S3TCDecodeFlag::DXT3;
	}else if (fourcc == FOURCC_DXT5){
		img_fmt = S3TCDecodeFlag::DXT5;
	}

	bool support_hardware_decode = CGlobals::GetRenderDevice()->GetCaps().SupportS3TC;

	if (support_hardware_decode)
	{
		return hardware_decode(buffer,buffer_size, img_fmt, img_width, img_height, img_mipmaps);
	}
	else {
		return soft_decode(buffer, buffer_size, img_fmt, img_width, img_height, img_mipmaps);
	}
}

