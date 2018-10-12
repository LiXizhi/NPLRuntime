#include "ParaEngine.h"

#include "TextureOpenGL.h"
#include "GLWrapper.h"
#include <algorithm>
using namespace ParaEngine;
using namespace IParaEngine;







struct DXTColBlock {
	uint16_t col0;
	uint16_t col1;

	uint8_t row[4];
};

struct DXT3AlphaBlock {
	uint16_t row[4];
};


struct DXT5AlphaBlock {
	uint8_t alpha0;
	uint8_t alpha1;
	uint8_t row[6];
};



///////////////////////////////////////////////////////////////////////////////
// flip a DXT1 color block
void flip_blocks_dxtc1(DXTColBlock *line, unsigned int numBlocks) {
	DXTColBlock *curblock = line;

	for (unsigned int i = 0; i < numBlocks; i++) {
		std::swap(curblock->row[0], curblock->row[3]);
		std::swap(curblock->row[1], curblock->row[2]);

		curblock++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT3 color block
void flip_blocks_dxtc3(DXTColBlock *line, unsigned int numBlocks) {
	DXTColBlock *curblock = line;
	DXT3AlphaBlock *alphablock;

	for (unsigned int i = 0; i < numBlocks; i++) {
		alphablock = (DXT3AlphaBlock*)curblock;

		std::swap(alphablock->row[0], alphablock->row[3]);
		std::swap(alphablock->row[1], alphablock->row[2]);

		curblock++;

		std::swap(curblock->row[0], curblock->row[3]);
		std::swap(curblock->row[1], curblock->row[2]);

		curblock++;
	}
}


///////////////////////////////////////////////////////////////////////////////
// flip a DXT5 alpha block
void flip_dxt5_alpha(DXT5AlphaBlock *block) {
	uint8_t gBits[4][4];

	const uint32_t mask = 0x00000007;          // bits = 00 00 01 11
	uint32_t bits = 0;
	memcpy(&bits, &block->row[0], sizeof(uint8_t) * 3);

	gBits[0][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[0][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[0][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[0][3] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[1][3] = (uint8_t)(bits & mask);

	bits = 0;
	memcpy(&bits, &block->row[3], sizeof(uint8_t) * 3);

	gBits[2][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[2][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[2][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[2][3] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][0] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][1] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][2] = (uint8_t)(bits & mask);
	bits >>= 3;
	gBits[3][3] = (uint8_t)(bits & mask);

	uint32_t *pBits = ((uint32_t*) &(block->row[0]));

	*pBits = *pBits | (gBits[3][0] << 0);
	*pBits = *pBits | (gBits[3][1] << 3);
	*pBits = *pBits | (gBits[3][2] << 6);
	*pBits = *pBits | (gBits[3][3] << 9);

	*pBits = *pBits | (gBits[2][0] << 12);
	*pBits = *pBits | (gBits[2][1] << 15);
	*pBits = *pBits | (gBits[2][2] << 18);
	*pBits = *pBits | (gBits[2][3] << 21);

	pBits = ((uint32_t*) &(block->row[3]));

#ifdef MACOS
	*pBits &= 0x000000ff;
#else
	*pBits &= 0xff000000;
#endif

	*pBits = *pBits | (gBits[1][0] << 0);
	*pBits = *pBits | (gBits[1][1] << 3);
	*pBits = *pBits | (gBits[1][2] << 6);
	*pBits = *pBits | (gBits[1][3] << 9);

	*pBits = *pBits | (gBits[0][0] << 12);
	*pBits = *pBits | (gBits[0][1] << 15);
	*pBits = *pBits | (gBits[0][2] << 18);
	*pBits = *pBits | (gBits[0][3] << 21);
}


///////////////////////////////////////////////////////////////////////////////
// flip a DXT5 color block
void flip_blocks_dxtc5(DXTColBlock *line, unsigned int numBlocks) {
	DXTColBlock *curblock = line;
	DXT5AlphaBlock *alphablock;

	for (unsigned int i = 0; i < numBlocks; i++) {
		alphablock = (DXT5AlphaBlock*)curblock;

		flip_dxt5_alpha(alphablock);

		curblock++;

		std::swap(curblock->row[0], curblock->row[3]);
		std::swap(curblock->row[1], curblock->row[2]);

		curblock++;
	}
}



uint8_t* flip_dxt_image(const uint8_t* pSrc, uint32_t width, uint32_t height,uint32_t size, Image::EImagePixelFormat format)
{

	uint32_t blockSize = 0;
	void(*flipblocks)(DXTColBlock*, unsigned int);
	switch (format)
	{
	case Image::IPF_COMPRESSED_DXT1:
		blockSize = 8;
		flipblocks = flip_blocks_dxtc1;
		break;
	case Image::IPF_COMPRESSED_DXT3:
		flipblocks = flip_blocks_dxtc3;
		blockSize = 16;
		break;
	case Image::IPF_COMPRESSED_DXT5:
		flipblocks = flip_blocks_dxtc5;
		blockSize = 16;
		break;
	}

	unsigned int xblocks = width / 4;
	unsigned int yblocks = height / 4;
	uint32_t linesize = xblocks * blockSize;
	DXTColBlock *top;
	DXTColBlock *bottom;
	uint8_t *tmp = new uint8_t[linesize];
	uint8_t* pDest = new uint8_t[size];
	memcpy(pDest,pSrc,size);
	for (unsigned int j = 0; j < (yblocks >> 1); j++) {
		top = (DXTColBlock*)((uint8_t*)pDest + j * linesize);
		bottom = (DXTColBlock*)((uint8_t*)pDest + (((yblocks - j) - 1) * linesize));

		flipblocks(top, xblocks);
		flipblocks(bottom, xblocks);

		// swap
		memcpy(tmp, bottom, linesize);
		memcpy(bottom, top, linesize);
		memcpy(top, tmp, linesize);
	}
	delete[] tmp;
	return pDest;
}





bool IsComressedFormat(EPixelFormat format)
{

	if (format == EPixelFormat::DXT1 || format == EPixelFormat::DXT3 || format == EPixelFormat::DXT5)
	{
		return true;
	}
	else {
		return false;
	}
}


ParaEngine::TextureOpenGL::TextureOpenGL()
	:m_TextureID(0)
	, m_Width(0)
	, m_Height(0)
	, m_Format(EPixelFormat::Unkonwn)
	, m_GLFormat(0)
	, m_GLDataType(0)
	, m_GLPixelFomat(0)
	, m_MagFilter(ETextureFilter::Point)
	, m_MinFilter(ETextureFilter::Point)
	, m_MipFilter(ETextureFilter::None)
	, m_AddressU(ETextureWrapMode::Clamp)
	, m_AddressV(ETextureWrapMode::Clamp)
	, m_Usage(ETextureUsage::Default)
{

}

ParaEngine::TextureOpenGL::~TextureOpenGL()
{

}


bool ParaEngine::TextureOpenGL::CopyTo(ITexture* target)
{
	return false;
}

bool ParaEngine::TextureOpenGL::StretchRect(ITexture* target, const ParaEngine::Rect* srcRect, const ParaEngine::Rect* targetRect, const ParaEngine::ETextureFilter filter)
{
	return false;
}

uint32_t ParaEngine::TextureOpenGL::GetWidth() const
{
	return m_Width;
}

uint32_t ParaEngine::TextureOpenGL::GetHeight() const
{
	return m_Height;
}

GLuint ParaEngine::TextureOpenGL::GetTextureID() const
{
	return m_TextureID;
}


bool ParaEngine::TextureOpenGL::UpdateImage(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels)
{
	if (IsComressedFormat(m_Format))
	{
		return UpdateImageComressed(level, xoffset, yoffset, width, height, pixels);
	}
	else {
		return UpdateImageUncomressed(level, xoffset, yoffset, width, height, pixels);
	}
}

ParaEngine::ImagePtr ParaEngine::TextureOpenGL::GetImage(uint32_t level)
{

	uint32_t pitch = 0;
	Image::EImagePixelFormat format = Image::IPF_A8;

	GLint width = 0;
	GLint height = 0;
	LibGL::BindTexture2D(0, m_TextureID);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &height);
	switch (m_GLFormat)
	{
	case GL_RGB:
	{
		pitch = 3 * sizeof(unsigned char) * width;
		format = Image::IPF_R8G8B8;
	}
	break;
	case GL_RGBA:
	{
		pitch = 4 * sizeof(unsigned char) * width;
		format = Image::IPF_R8G8B8A8;
	}
	break;
	case GL_BGRA:
	{
		pitch = 4 * sizeof(unsigned char) * width;
		format = Image::IPF_B8G8R8A8;
	}
	break;
	case GL_ALPHA:
	{
		pitch = 1 * sizeof(unsigned char) * width;
		format = Image::IPF_A8;

	}
	break;
	case GL_LUMINANCE:
	{
		pitch = 1 * sizeof(unsigned char) * width;
		format = Image::IPF_L8;

	}
	break;
	case GL_LUMINANCE_ALPHA:
	{
		pitch = 2 * sizeof(unsigned char) * width;
		format = Image::IPF_A8L8;

	}
	break;
	default:
		break;
	}

	size_t buffer_size = pitch * height;
	unsigned char* buffer = new unsigned char[buffer_size];
	memset(buffer, 0, buffer_size);
	
	
	glGetTexImage(GL_TEXTURE_2D, level, m_GLFormat, GL_UNSIGNED_BYTE,buffer);


	auto img = std::make_shared<Image>();
	ImageMipmap mipmap;
	mipmap.width = width;
	mipmap.height = height;
	mipmap.offset = 0;
	mipmap.size = buffer_size;
	img->data = buffer;
	img->data_size = buffer_size;
	img->Format = format;
	img->mipmaps.push_back(mipmap);
	return img;
}

ParaEngine::ETextureFilter ParaEngine::TextureOpenGL::GetMinFilter() const
{
	return m_MinFilter;
}

ParaEngine::ETextureFilter ParaEngine::TextureOpenGL::GetMagFilter() const
{
	return m_MagFilter;
}


bool ParaEngine::TextureOpenGL::SetMinFilter(ParaEngine::ETextureFilter type)
{
	if (type == ETextureFilter::None) return false;
	if (m_MipFilter != ETextureFilter::None) {
		m_MinFilter = type;
		return true;
	}
	if (type == m_MinFilter) return true;
	LibGL::BindTexture2D(0,m_TextureID);
	switch (type)
	{
	case ParaEngine::ETextureFilter::Point:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case ParaEngine::ETextureFilter::Linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	default:
		break;
	}
	m_MinFilter = type;
	return true;
}


bool ParaEngine::TextureOpenGL::SetMagFilter(ParaEngine::ETextureFilter type)
{
	if (type == m_MagFilter) return true;
	m_MagFilter = type;
	LibGL::BindTexture2D(0,m_TextureID);
	switch (m_MagFilter)
	{
	case ParaEngine::ETextureFilter::Point:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case ParaEngine::ETextureFilter::Linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	default:
		break;
	}

	return true;
}

ParaEngine::ETextureWrapMode ParaEngine::TextureOpenGL::GetAddressU() const
{
	return m_AddressU;
}

ParaEngine::ETextureWrapMode ParaEngine::TextureOpenGL::GetAddressV() const
{
	return m_AddressV;
}


bool ParaEngine::TextureOpenGL::SetAddressU(ParaEngine::ETextureWrapMode mode)
{
	if (mode == m_AddressU) return true;
    if(mode == ETextureWrapMode::Border && !CGlobals::GetRenderDevice()->GetCaps().BorderClamp) return false;
	m_AddressU = mode;
	LibGL::BindTexture2D(0,m_TextureID);
	switch (m_AddressU)
	{
	case ETextureWrapMode::Clamp:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		break;
	case ETextureWrapMode::Repeat:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		break;
	case ETextureWrapMode::Mirror:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		break;
	case ETextureWrapMode::Border:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		break;
	default:
		break;
	}
	return true;
}

bool ParaEngine::TextureOpenGL::SetAddressV(ParaEngine::ETextureWrapMode mode)
{
	if (mode == m_AddressV) return true;
    if(mode == ETextureWrapMode::Border && !CGlobals::GetRenderDevice()->GetCaps().BorderClamp) return false;
	m_AddressV = mode;
	LibGL::BindTexture2D(0,m_TextureID);
	switch (m_AddressV)
	{
	case ParaEngine::ETextureWrapMode::Clamp:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	case ParaEngine::ETextureWrapMode::Repeat:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		break;
	case ETextureWrapMode::Mirror:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		break;
	case ETextureWrapMode::Border:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		break;
	default:
		break;
	}
	return true;
}


ParaEngine::ETextureFilter ParaEngine::TextureOpenGL::GetMipFilter() const
{
	return m_MipFilter;
}


bool ParaEngine::TextureOpenGL::SetMipFilter(ParaEngine::ETextureFilter type)
{
	if (type == m_MipFilter) return true;
	LibGL::BindTexture2D(0,m_TextureID);

	
	switch (type)
	{
	case ParaEngine::ETextureFilter::Point:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		break;
	case ParaEngine::ETextureFilter::Linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		break;
	case ETextureFilter::None:
	{
		if (m_MinFilter == ETextureFilter::Linear)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		}else if (m_MinFilter == ETextureFilter::Point)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else {
			return false;
		}
	}
	break;
	default:
		break;
	}
	m_MipFilter = type;
	return true;
}


bool ParaEngine::TextureOpenGL::SetBorderColor(const ParaEngine::Color4f& color)
{
    if (!CGlobals::GetRenderDevice()->GetCaps().BorderClamp)
    {
        return false;
    }
    
	if (color.r == m_BorderColor.r && color.g == m_BorderColor.g && color.b == m_BorderColor.b && color.a == m_BorderColor.a)
	{
		return true;
	}

	LibGL::BindTexture2D(0,m_TextureID);
	GLfloat col[] = {color.r,color.g,color.b,color.a};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, col);
	m_BorderColor = color;
	return true;
}


ParaEngine::Color4f ParaEngine::TextureOpenGL::GetBorderColor() const
{
	return m_BorderColor;
}

TextureOpenGL* TextureOpenGL::Create(uint32_t width, uint32_t height, EPixelFormat format, ETextureUsage usage)
{

	if (format == EPixelFormat::R32F && !CGlobals::GetRenderDevice()->GetCaps().Texture_R32F)
	{
		return nullptr;
	}


	if (format == EPixelFormat::A16B16G16R16F && !CGlobals::GetRenderDevice()->GetCaps().Texture_RGBA16F)
	{
		return nullptr;
	}


	GLenum glFormat = 0;
	GLenum glDataType = 0;
	GLenum glPixelFormat = 0;
	switch (format)
	{
	case EPixelFormat::R8G8B8:
		glFormat = GL_RGB;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_RGB;
		break;
	case EPixelFormat::A8R8G8B8:
 		glFormat = GL_RGBA;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_RGBA;
		break;
	case EPixelFormat::A8B8G8R8:
		glFormat = GL_BGRA;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_BGRA;
		break;
	case EPixelFormat::A8:
		glFormat = GL_ALPHA;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_ALPHA;
		break;
	case EPixelFormat::L8:
		glFormat = GL_LUMINANCE;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_RED;
		break;
	case EPixelFormat::A8L8:
		glFormat = GL_LUMINANCE_ALPHA;
		glDataType = GL_UNSIGNED_BYTE;
		glPixelFormat = GL_RG;
	case EPixelFormat::D24S8:
		glFormat = GL_DEPTH_STENCIL;
		glDataType = GL_UNSIGNED_INT_24_8;
		glPixelFormat = GL_DEPTH_STENCIL;
		break;
	case EPixelFormat::R32F:
		glFormat = GL_R32F;
		glDataType = GL_FLOAT;
		glPixelFormat = GL_RED;
		break;
	case EPixelFormat::A16B16G16R16F:
		glFormat = GL_RGBA16F;
		glDataType = GL_FLOAT;
		glPixelFormat = GL_RGBA;
		break;
	default:
		return nullptr;
		break;
	}
	if (glFormat == 0) return nullptr;

	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	LibGL::BindTexture2D(0,textureID);


	glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glPixelFormat, glDataType, nullptr);


	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		OUTPUT_LOG("Create texture failed error:%x\n", error);
		glDeleteTextures(GL_TEXTURE_2D,&textureID);
		return nullptr;
	}


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



    

	TextureOpenGL* tex = new TextureOpenGL();

	tex->m_TextureID = textureID;
	tex->m_Width = width;
	tex->m_Height = height;
	tex->m_GLFormat = glFormat;
    tex->m_GLDataType = glDataType;
    tex->m_GLPixelFomat = glPixelFormat;
	tex->m_Usage = usage;
	tex->m_Format = format;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	return tex;
}

ParaEngine::TextureOpenGL * ParaEngine::TextureOpenGL::CreateWithImage(ImagePtr image)
{
	if (image == nullptr) return nullptr;

	if (image->Format == Image::IPF_COMPRESSED_DXT1 ||
		image->Format == Image::IPF_COMPRESSED_DXT3 ||
		image->Format == Image::IPF_COMPRESSED_DXT5)
	{
		return CreateComressedTextureWithImage(image);
	}
	else {
		return CreateUnCompressedTextureWithImage(image);
	}

}

void ParaEngine::TextureOpenGL::OnRelease()
{
	glDeleteTextures(1, &m_TextureID);
	m_TextureID = 0;
	m_Width = 0;
	m_Height = 0;
	m_Format = EPixelFormat::Unkonwn;
}
inline uint32_t GetBPP(GLenum format)
{
    uint32_t bpp = 0;
    switch (format)
    {
        case GL_RGB:
            bpp = 3;
            break;
        case GL_RGBA:
        case GL_BGRA:
            bpp = 4;
            break;
        case GL_ALPHA:
        case GL_LUMINANCE:
            bpp = 1;
            break;
        case GL_LUMINANCE_ALPHA:
            bpp = 2;
            break;
        default:
            break;
    }
    return bpp;
}
inline unsigned char* flipImageData(const unsigned char* pSrcData,GLenum format,uint32_t width,uint32_t height)
{
    // flip vectical
    uint32_t bpp = GetBPP(format);
    uint32_t pitch = width * bpp;
    unsigned char* pDest = new unsigned  char[height * pitch];
    for (int y = 0; y < height; y++)
    {
        memcpy(pDest + y * pitch, pSrcData + (height - 1 - y)*pitch, pitch);
    }
    return pDest;
}

ParaEngine::TextureOpenGL* ParaEngine::TextureOpenGL::CreateUnCompressedTextureWithImage(ImagePtr image)
{
	EPixelFormat format = EPixelFormat::Unkonwn;
	switch (image->Format)
	{
	case  Image::IPF_L8:
	{
		format = EPixelFormat::L8;
	}
	break;
	case  Image::IPF_A8:
	{
		format = EPixelFormat::A8;
	}
	break;
	case  Image::IPF_A8L8:
	{
		format = EPixelFormat::A8L8;
	}
	break;
	case  Image::IPF_R8G8B8:
	{
		format = EPixelFormat::R8G8B8;
	}
	break;
	case  Image::IPF_R8G8B8A8:
	{
		format = EPixelFormat::A8R8G8B8;
	}
	break;
	case Image::IPF_COMPRESSED_DXT1:
	{
		format = EPixelFormat::DXT1;
	}
	break;
	case Image::IPF_COMPRESSED_DXT3:
	{
		format = EPixelFormat::DXT3;
	}
	break;
	case Image::IPF_COMPRESSED_DXT5:
	{
		format = EPixelFormat::DXT5;
	}
	break;
	default:
		break;
	}

    
    
    GLenum glFormat = 0;
    GLenum glDataType = 0;
    GLenum glPixelFormat = 0;
    switch (format)
    {
        case EPixelFormat::R8G8B8:
            glFormat = GL_RGB;
            glDataType = GL_UNSIGNED_BYTE;
            glPixelFormat = GL_RGB;
            break;
        case EPixelFormat::A8R8G8B8:
            glFormat = GL_RGBA;
            glDataType = GL_UNSIGNED_BYTE;
            glPixelFormat = GL_RGBA;
            break;
        case EPixelFormat::A8B8G8R8:
            glFormat = GL_BGRA;
            glDataType = GL_UNSIGNED_BYTE;
            glPixelFormat = GL_BGRA;
            break;
        case EPixelFormat::A8:
            glFormat = GL_ALPHA;
            glDataType = GL_UNSIGNED_BYTE;
            glPixelFormat = GL_ALPHA;
            break;
        case EPixelFormat::L8:
            glFormat = GL_LUMINANCE;
            glDataType = GL_UNSIGNED_BYTE;
            glPixelFormat = GL_RED;
            break;
        case EPixelFormat::A8L8:
            glFormat = GL_LUMINANCE_ALPHA;
            glDataType = GL_UNSIGNED_BYTE;
            glPixelFormat = GL_RG;
        case EPixelFormat::D24S8:
            glFormat = GL_DEPTH_STENCIL;
            glDataType = GL_UNSIGNED_INT_24_8;
            glPixelFormat = GL_DEPTH_STENCIL;
            break;
        case EPixelFormat::R32F:
            glFormat = GL_R32F;
            glDataType = GL_FLOAT;
            glPixelFormat = GL_RED;
            break;
        case EPixelFormat::A16B16G16R16F:
            glFormat = GL_RGBA16F;
            glDataType = GL_FLOAT;
            glPixelFormat = GL_RGBA;
            break;
        default:
            return nullptr;
            break;
    }
    if (glFormat == 0) return nullptr;
    
    GLuint textureID = 0;
    glGenTextures(1, &textureID);
	LibGL::BindTexture2D(0,textureID);
    
    
    for (int i = 0; i < image->mipmaps.size(); i++)
    {
        auto img = image->mipmaps[i];
       // tex->UpdateImage(i, 0, 0, image->mipmaps[i].width, image->mipmaps[i].height, ((unsigned char*)image->data) + image->mipmaps[i].offset);
        
        const unsigned char* pSrc= ((unsigned char*)image->data) + image->mipmaps[i].offset;
        unsigned char* pDest = flipImageData(pSrc, glFormat, img.width, img.height);
        glTexImage2D(GL_TEXTURE_2D, 0, glFormat, img.width, img.height, 0, glPixelFormat, glDataType,pDest);
        delete [] pDest;
    }
    
    
    

    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        OUTPUT_LOG("Create texture failed error:%x\n", error);
        glDeleteTextures(GL_TEXTURE_2D,&textureID);
        return nullptr;
    }
    
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    

    
    
    TextureOpenGL* tex = new TextureOpenGL();
    
    tex->m_TextureID = textureID;
    tex->m_Width = image->mipmaps[0].width;
    tex->m_Height = image->mipmaps[0].height;
    tex->m_GLFormat = glFormat;
    tex->m_GLDataType = glDataType;
    tex->m_GLPixelFomat = glPixelFormat;
    tex->m_Usage = ETextureUsage::Default;
    tex->m_Format = format;

    

	return tex;
}

bool TextureOpenGL::UpdateImageComressed(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels)
{
	
	uint32_t blockSize = 0;
	Image::EImagePixelFormat imgFormat;
	switch (m_Format)
	{
	case ParaEngine::EPixelFormat::DXT1:
		blockSize = 8;
		imgFormat = Image::IPF_COMPRESSED_DXT1;
		break;
	case ParaEngine::EPixelFormat::DXT3:
		blockSize = 16;
		imgFormat = Image::IPF_COMPRESSED_DXT3;
		break;
	case ParaEngine::EPixelFormat::DXT5:
		blockSize = 16;
		imgFormat = Image::IPF_COMPRESSED_DXT5;
		break;
	default:
		return false;
	}


	GLuint size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
	uint32_t offy = m_Height - yoffset - height;

	uint8_t* pDest = flip_dxt_image(pixels, width, height, size, imgFormat);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCompressedTexSubImage2D(GL_TEXTURE_2D, level, xoffset, offy, width, height, m_GLFormat, size, pixels);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	delete[] pDest;
	return true;
}





bool TextureOpenGL::UpdateImageUncomressed(uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height, const unsigned char* pixels)
{

    unsigned char* pDest = flipImageData(pixels, m_GLFormat, width, height);

	LibGL::BindTexture2D(0,m_TextureID);
    
    
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	uint32_t offy = m_Height - yoffset - height;

	glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, offy, width, height, m_GLFormat, m_GLDataType, pDest);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	delete[] pDest;
	return true;
}

TextureOpenGL* TextureOpenGL::CreateComressedTextureWithImage(ImagePtr image)
{

	GLenum glFormat = 0;
	EPixelFormat format;
	switch (image->Format)
	{
	case Image::IPF_COMPRESSED_DXT1:
		glFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		format = EPixelFormat::DXT1;
		break;
	case Image::IPF_COMPRESSED_DXT3:
		glFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		format = EPixelFormat::DXT3;
		break;
	case Image::IPF_COMPRESSED_DXT5:
		glFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		format = EPixelFormat::DXT5;
		break;
	default:
		return nullptr;
		break;
	}
	if (glFormat == 0) return nullptr;

	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	LibGL::BindTexture2D(0,textureID);


	for (int i =0;i<image->mipmaps.size();i++)
	{

		ImageMipmap& mp = image->mipmaps[i];
		uint8_t* pDest = flip_dxt_image((uint8_t*)image->data + mp.offset, mp.width, mp.height, mp.size, image->Format);
		glCompressedTexImage2D(GL_TEXTURE_2D, i, glFormat, mp.width, mp.height, 0, mp.size,pDest);
		delete[] pDest;
	}


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	TextureOpenGL* tex = new TextureOpenGL();
	tex->m_TextureID = textureID;
	tex->m_Width = image->mipmaps[0].width;
	tex->m_Height = image->mipmaps[0].height;
	tex->m_GLFormat = glFormat;
	tex->m_Usage = ETextureUsage::Default;
	tex->m_Format = format;
	return tex;
}

