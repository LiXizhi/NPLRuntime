#include "PNGParser.h"
#include "png.h"
#include <memory>
using namespace ParaEngine;
using namespace IParaEngine;
#define PNGSIGSIZE  8



typedef struct
{
	const unsigned char * data;
	size_t size;
	int offset;
}tImageSource;


static void png_read_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	tImageSource* isource = (tImageSource*)png_get_io_ptr(png_ptr);

	if ((int)(isource->offset + length) <= isource->size)
	{
		memcpy(data, isource->data + isource->offset, length);
		isource->offset += length;
	}
	else
	{
		png_error(png_ptr, "png_read_callback failed");
	}
}



ImagePtr PNGParser::Parse(const unsigned char* buffer, size_t buffer_size)
{
	if (buffer_size < PNGSIGSIZE) return nullptr;
	png_byte  header[PNGSIGSIZE] = { 0 };

	// check the data is png or not
	memcpy(header, buffer, PNGSIGSIZE);
	if (png_sig_cmp(header, 0, PNGSIGSIZE))
		return nullptr;

	// init png_struct
	png_structp  png_ptr = nullptr;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) return nullptr;

	// init png_info
	png_infop  info_ptr = nullptr;
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (info_ptr) ? &info_ptr : 0, 0);
		return nullptr;
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{	
		png_destroy_read_struct(&png_ptr, (info_ptr) ? &info_ptr : 0, 0);
		return nullptr;
	}

	// set the read call back function
	tImageSource imageSource;
	imageSource.data = (unsigned char*)buffer;
	imageSource.size = buffer_size;
	imageSource.offset = 0;
	png_set_read_fn(png_ptr, &imageSource, png_read_callback);

	// read png header info

	// read png file info
	png_read_info(png_ptr, info_ptr);

	uint32_t width = png_get_image_width(png_ptr, info_ptr);
	uint32_t height = png_get_image_height(png_ptr, info_ptr);
	png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);

	// force palette images to be expanded to 24-bit RGB
	// it may include alpha channel
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
	}

	// low-bit-depth grayscale images are to be expanded to 8 bits
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	{
		bit_depth = 8;
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}
	// expand any tRNS chunk data into a full alpha channel
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
	}
	// reduce images with 16-bit samples to 8 bits
	if (bit_depth == 16)
	{
		png_set_strip_16(png_ptr);
	}

	// Expanded earlier for grayscale, now take care of palette and rgb
	if (bit_depth < 8)
	{
		png_set_packing(png_ptr);
	}
	// update info
	png_read_update_info(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);


	Image::EImagePixelFormat image_format = Image::IPF_R8G8B8A8;

	switch (color_type)
	{
	case PNG_COLOR_TYPE_GRAY:
		image_format = Image::IPF_L8;
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		image_format = Image::IPF_A8L8;
		break;
	case PNG_COLOR_TYPE_RGB:
		image_format = Image::IPF_R8G8B8;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		image_format = Image::IPF_R8G8B8A8;
		break;
	default:
		break;
	}

	png_size_t rowbytes;
	png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

	rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	uint32_t dataLen = rowbytes * height;
	unsigned char* _data = static_cast<unsigned char*>(malloc(dataLen * sizeof(unsigned char)));
	if (!_data)
	{
		if (row_pointers != nullptr)
		{
			free(row_pointers);
		}

		png_destroy_read_struct(&png_ptr, (info_ptr) ? &info_ptr : 0, 0);
		return nullptr;
	}
	// flip vertical
	for (unsigned short i = 0; i < height; ++i)
	{
		row_pointers[i] = _data + i * rowbytes;
	}
	png_read_image(png_ptr, row_pointers);

	png_read_end(png_ptr, nullptr);

	if (row_pointers != nullptr)
	{
		free(row_pointers);
	}

	auto image = std::make_shared<Image>();
	image->width = width;
	image->height = height;
	image->Format = image_format;
	image->data = _data;
	image->data_size = dataLen;
	return image;
}

