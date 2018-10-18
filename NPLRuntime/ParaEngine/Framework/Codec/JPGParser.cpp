#include "JPGParser.h"
#include "jpeglib.h"
#include <setjmp.h>
using namespace ParaEngine;
using namespace IParaEngine;


struct MyErrorMgr
{
	struct jpeg_error_mgr pub;  /* "public" fields */
	jmp_buf setjmp_buffer;  /* for return to caller */
};

typedef struct MyErrorMgr * MyErrorPtr;

/*
* Here's the routine that will replace the standard error_exit method:
*/

METHODDEF(void)
myErrorExit(j_common_ptr cinfo)
{
	/* cinfo->err really points to a MyErrorMgr struct, so coerce pointer */
	MyErrorPtr myerr = (MyErrorPtr)cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	/* internal message function can't show error message in some platforms, so we rewrite it here.
	* edit it if has version conflict.
	*/
	//(*cinfo->err->output_message) (cinfo);
	char buffer[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message) (cinfo, buffer);
	//OUTPUT_LOG("jpeg error: %s", buffer);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}


ImagePtr JPGParser::Parse(const unsigned char* buffer, size_t buffer_size)
{

	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct MyErrorMgr jerr;
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1] = { 0 };
	unsigned long location = 0;
	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = myErrorExit;
	/* Establish the setjmp return context for MyErrorExit to use. */
	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		jpeg_destroy_decompress(&cinfo);
		return nullptr;
	}

	/* setup decompression process and source, then read JPEG header */
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, const_cast<unsigned char*>(buffer), buffer_size);


	/* reading the image header which contains image information */
#if (JPEG_LIB_VERSION >= 90)
	// libjpeg 0.9 adds stricter types.
	jpeg_read_header(&cinfo, (boolean)1);
#else
	jpeg_read_header(&cinfo, TRUE);
#endif

	Image::EImagePixelFormat img_format;
	// we only support RGB or grayscale
	if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
	{
		img_format = Image::IPF_L8;
	}
	else
	{
		cinfo.out_color_space = JCS_RGB;
		img_format = Image::IPF_R8G8B8;
	}

	/* Start decompression jpeg here */
	jpeg_start_decompress(&cinfo);


	/* init image info */
	uint32_t img_width = cinfo.output_width;
	uint32_t img_height = cinfo.output_height;
	uint32_t img_data_size = cinfo.output_width*cinfo.output_height*cinfo.output_components;;
	uint8_t* img_data = new uint8_t[img_data_size];

	/* now actually read the jpeg into the raw buffer */
	/* read one scan line at a time */
	while (cinfo.output_scanline < cinfo.output_height)
	{
		row_pointer[0] = img_data + location;
		location += cinfo.output_width*cinfo.output_components;
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
	}

	/* When read image file with broken data, jpeg_finish_decompress() may cause error.
	* Besides, jpeg_destroy_decompress() shall deallocate and release all memory associated
	* with the decompression object.
	* So it doesn't need to call jpeg_finish_decompress().
	*/
	//jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress(&cinfo);


	auto img = std::make_shared<Image>();
	img->data = img_data;
	img->data_size = img_data_size;
	img->Format = img_format;

	ImageMipmap mm;
	mm.width = img_width;
	mm.height = img_height;
	mm.offset = 0;
	img->mipmaps.push_back(mm);


	return img;
}

