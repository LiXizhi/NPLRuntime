#include "ParaEngine.h"
#include "GLTexture2D.h"

#include "GLImage.h"
#include "ParaImage.h"
#include "GLProgram.h"
#include "GLProgramCache.h"
#include "RHISoftwareS3TC.h"
#include <string.h>
#include <memory>

// Some core-profile headers (including macOS gl3.h) omit these extension
// constants even when the device reports BC support. Keep the format table
// available; initWithImage selects BC only when the device supports it.
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

using namespace ParaEngine;


namespace {
	typedef GLTexture2D::PixelFormatInfoMap::value_type PixelFormatInfoMapValue;
	static const PixelFormatInfoMapValue TexturePixelFormatInfoTablesValue[] =
	{
		PixelFormatInfoMapValue(PixelFormat::A8B8G8R8, GLTexture2D::PixelFormatInfo(GL_BGRA, GL_BGRA, GL_UNSIGNED_BYTE, 32, false, true)),
		PixelFormatInfoMapValue(PixelFormat::A8R8G8B8, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, true)),
		PixelFormatInfoMapValue(PixelFormat::A4R4G4B4, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 16, false, true)),
		PixelFormatInfoMapValue(PixelFormat::A1R5G5B5, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 16, false, true)),
		PixelFormatInfoMapValue(PixelFormat::R5G6B5, GLTexture2D::PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 16, false, false)),
		PixelFormatInfoMapValue(PixelFormat::R8G8B8, GLTexture2D::PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 24, false, false)),
#if defined(__APPLE__) && !defined(PARAENGINE_MOBILE)
		PixelFormatInfoMapValue(PixelFormat::A8, GLTexture2D::PixelFormatInfo(GL_R8, GL_RED, GL_UNSIGNED_BYTE, 8, false, false)),
		PixelFormatInfoMapValue(PixelFormat::L8, GLTexture2D::PixelFormatInfo(GL_R8, GL_RED, GL_UNSIGNED_BYTE, 8, false, false)),
		PixelFormatInfoMapValue(PixelFormat::A8L8, GLTexture2D::PixelFormatInfo(GL_RG8, GL_RG, GL_UNSIGNED_BYTE, 16, false, true)),
#else
		PixelFormatInfoMapValue(PixelFormat::A8, GLTexture2D::PixelFormatInfo(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 8, false, false)),
		PixelFormatInfoMapValue(PixelFormat::L8, GLTexture2D::PixelFormatInfo(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 8, false, false)),
		PixelFormatInfoMapValue(PixelFormat::A8L8, GLTexture2D::PixelFormatInfo(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 16, false, true)),
#endif
		PixelFormatInfoMapValue(PixelFormat::X8R8G8B8, GLTexture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, false)),
#ifdef GL_R32F
		PixelFormatInfoMapValue(PixelFormat::R32F, GLTexture2D::PixelFormatInfo(GL_R32F, GL_RED, GL_FLOAT, 32, false, false)),
		PixelFormatInfoMapValue(PixelFormat::R16F, GLTexture2D::PixelFormatInfo(GL_R16F, GL_RED, GL_HALF_FLOAT, 16, false, false)),
		PixelFormatInfoMapValue(PixelFormat::G16R16F, GLTexture2D::PixelFormatInfo(GL_RG16F, GL_RG, GL_HALF_FLOAT, 32, false, false)),
		PixelFormatInfoMapValue(PixelFormat::A16B16G16R16F, GLTexture2D::PixelFormatInfo(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, 64, false, true)),
		PixelFormatInfoMapValue(PixelFormat::A32B32G32R32F, GLTexture2D::PixelFormatInfo(GL_RGBA32F, GL_RGBA, GL_FLOAT, 128, false, true)),
#endif

#ifdef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
		PixelFormatInfoMapValue(PixelFormat::PVRTC2, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, false)),
		PixelFormatInfoMapValue(PixelFormat::PVRTC2A, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, true)),
		PixelFormatInfoMapValue(PixelFormat::PVRTC4, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
		PixelFormatInfoMapValue(PixelFormat::PVRTC4A, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, true)),
#endif

#ifdef GL_ETC1_RGB8_OES
		PixelFormatInfoMapValue(PixelFormat::ETC1, GLTexture2D::PixelFormatInfo(GL_ETC1_RGB8_OES, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
		PixelFormatInfoMapValue(PixelFormat::DXT1, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
		PixelFormatInfoMapValue(PixelFormat::DXT3, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
		PixelFormatInfoMapValue(PixelFormat::DXT5, GLTexture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif

#ifdef GL_ATC_RGB_AMD
		PixelFormatInfoMapValue(PixelFormat::ATC_RGB, GLTexture2D::PixelFormatInfo(GL_ATC_RGB_AMD,
		0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif

#ifdef GL_ATC_RGBA_EXPLICIT_ALPHA_AMD
		PixelFormatInfoMapValue(PixelFormat::ATC_EXPLICIT_ALPHA, GLTexture2D::PixelFormatInfo(GL_ATC_RGBA_EXPLICIT_ALPHA_AMD,
		0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif

#ifdef GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD
		PixelFormatInfoMapValue(PixelFormat::ATC_INTERPOLATED_ALPHA, GLTexture2D::PixelFormatInfo(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD,
		0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
	};
}

//The PixpelFormat corresponding information
const GLTexture2D::PixelFormatInfoMap GLTexture2D::_pixelFormatInfoTables(TexturePixelFormatInfoTablesValue,
	TexturePixelFormatInfoTablesValue + sizeof(TexturePixelFormatInfoTablesValue) / sizeof(TexturePixelFormatInfoTablesValue[0]));

// If the image has alpha, you can create RGBA8 (32-bit) or RGBA4 (16-bit) or RGB5A1 (16-bit)
// Unknown selects DXT1/DXT5 for color images, with a capability-aware fallback.
PixelFormat GLTexture2D::g_defaultAlphaPixelFormat = PixelFormat::Unknown;

const GLTexture2D::PixelFormatInfoMap& GLTexture2D::getPixelFormatInfoMap()
{
	return _pixelFormatInfoTables;
}

int GLTexture2D::ccNextPOT(int x)
{
	x = x - 1;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return x + 1;
}

#define MAX(a,b) ((a>b)?a:b)


GLTexture2D::GLTexture2D()
	: _pixelFormat(PixelFormat::Unknown)
	, _pixelsWide(0)
	, _pixelsHigh(0)
	, _name(0)
	, _maxS(0.0)
	, _maxT(0.0)
	, _hasPremultipliedAlpha(false)
	, _hasMipmaps(false)
	, _shaderProgram(nullptr)
	, _antialiasEnabled(true)
	, _valid(true)
	, _alphaTexture(nullptr)
	, _rhiTexture(nullptr)
	, _ownsRHITexture(true)
{
}

GLTexture2D::~GLTexture2D()
{

#if CC_ENABLE_CACHE_TEXTURE_DATA
	VolatileTextureMgr::removeTexture(this);
#endif
	SAFE_RELEASE(_alphaTexture); // ETC1 ALPHA support.

	SAFE_RELEASE(_shaderProgram);

	if (_rhiTexture)
	{
		if (_ownsRHITexture)
			_rhiTexture->ReleaseRHIResource();
		_rhiTexture = nullptr;
	}

	if (_ownsRHITexture && _name && CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
	{
		GL::deleteTexture(_name);
	}
}

bool GLTexture2D::initWithImage(const ParaImage *image)
{
	return initWithImage(image, g_defaultAlphaPixelFormat);
}

bool GLTexture2D::initWithImage(const ParaImage* image, PixelFormat format)
{
	return initWithImage(image, format, false);
}

#ifdef EMSCRIPTEN_SINGLE_THREAD
namespace ParaEngine
{
	struct GuiAtlasPage
	{
		static constexpr int Size = 1024;
		GLTexture2D texture;
		int x = 0, y = 0, rowHeight = 0;
	};
}

// Borrow the already converted upload bytes. No readback, re-decode or retained CPU copy.
void GLTexture2D::cacheGuiImage(const MipmapInfo& mip)
{
	if (_pixelsWide > 256 || _pixelsHigh > 256 || !_name || _hasMipmaps)
		return;
	const auto& info = _pixelFormatInfoTables.at(_pixelFormat);
	if (_pixelFormat != PixelFormat::A8R8G8B8 && _pixelFormat != PixelFormat::A4R4G4B4 &&
		_pixelFormat != PixelFormat::R5G6B5 && _pixelFormat != PixelFormat::R8G8B8)
		return;
	const int bytes = info.bpp / 8;
	if (mip.len < _pixelsWide * _pixelsHigh * bytes)
		return;

	// At most two live pages across all formats (8 MiB total at RGBA8). A page
	// retires with its last source image, including on world or device reload.
	static std::weak_ptr<GuiAtlasPage> pages[2];
	std::weak_ptr<GuiAtlasPage>* vacant = nullptr;
	std::shared_ptr<GuiAtlasPage> page;
	const int width = _pixelsWide + 2, height = _pixelsHigh + 2;
	int x = 0, y = 0;
	for (auto& slot : pages)
	{
		auto candidate = slot.lock();
		if (!candidate || !candidate->texture.getName())
		{
			vacant = &slot;
			continue;
		}
		if (candidate->texture.GetFormat() != _pixelFormat)
			continue;
		x = candidate->x;
		y = candidate->y;
		if (x + width > GuiAtlasPage::Size)
		{
			x = 0;
			y += candidate->rowHeight;
		}
		if (y + height <= GuiAtlasPage::Size)
		{
			page = candidate;
			break;
		}
	}
	if (!page)
	{
		if (!vacant)
			return;
		page = std::make_shared<GuiAtlasPage>();
		const int size = GuiAtlasPage::Size;
		if (!page->texture.initWithData(nullptr, size * size * bytes, _pixelFormat, size, size, Size(size, size)))
			return;
		*vacant = page;
		x = y = 0;
	}

	// Match the WebGL file loader's REPEAT for power-of-two images and CLAMP
	// otherwise, including bilinear footprints crossing an outer edge.
	const bool repeat = ccNextPOT(_pixelsWide) == _pixelsWide && ccNextPOT(_pixelsHigh) == _pixelsHigh;
	std::vector<unsigned char> padded(width * height * bytes);
	for (int row = 0; row < height; ++row)
	{
		int sy = row - 1;
		if (row == 0) sy = repeat ? _pixelsHigh - 1 : 0;
		if (row == height - 1) sy = repeat ? 0 : _pixelsHigh - 1;
		const auto* src = mip.address + sy * _pixelsWide * bytes;
		auto* dst = padded.data() + row * width * bytes;
		memcpy(dst + bytes, src, _pixelsWide * bytes);
		memcpy(dst, src + (repeat ? _pixelsWide - 1 : 0) * bytes, bytes);
		memcpy(dst + (width - 1) * bytes, src + (repeat ? 0 : _pixelsWide - 1) * bytes, bytes);
	}
	if (!page->texture.updateWithData(padded.data(), x, y, width, height))
		return;
	page->rowHeight = x == 0 ? height : (std::max)(height, page->rowHeight);
	page->x = x + width;
	page->y = y;
	_guiX = x + 1;
	_guiY = y + 1;
	_guiPage = page;
}

IRHITexture* GLTexture2D::getGuiAtlasRegion(RECT& source) const
{
	if (!_guiPage || !_guiPage->texture.getName() || source.left < 0 || source.right < 0 || source.top < 0 || source.bottom < 0 ||
		source.left > _pixelsWide || source.right > _pixelsWide || source.top > _pixelsHigh || source.bottom > _pixelsHigh)
		return nullptr;
	source.left += _guiX;
	source.right += _guiX;
	source.top += _guiY;
	source.bottom += _guiY;
	return &_guiPage->texture;
}
#endif

bool GLTexture2D::initWithImage(const ParaImage* image, PixelFormat format, bool guiImage)
{
	if (!image || !image->getData())
		return false;
	const int width = static_cast<int>(image->getWidth());
	const int height = static_cast<int>(image->getHeight());
	if (width <= 0 || height <= 0 || width > ParaImage::MaxTextureSize || height > ParaImage::MaxTextureSize)
		return false;

	IRenderDevice* device = CGlobals::GetRenderDevice();
	const PixelFormat sourceFormat = image->getRenderFormat();
	const bool automatic = format == PixelFormat::Unknown || format == PixelFormat::COUNT;
	const bool sourceBC = RHISoftwareS3TC::IsS3TCFormat(sourceFormat);
	const bool colorImage = sourceFormat == PixelFormat::A8R8G8B8 || sourceFormat == PixelFormat::R8G8B8;
	bool hasAlpha = sourceFormat != PixelFormat::R8G8B8;
	if (sourceFormat == PixelFormat::A8R8G8B8)
	{
		if (static_cast<size_t>(image->getDataLen()) < static_cast<size_t>(width) * height * 4)
			return false;
		hasAlpha = false;
		for (size_t i = 3, end = static_cast<size_t>(width) * height * 4; i < end; i += 4)
			if (image->getData()[i] != 255) { hasAlpha = true; break; }
	}
	// BC uploads require block-aligned base dimensions on WebGL and modern RHIs.
	const bool canUseBC = device && device->GetCaps().supportsBCCompression && width % 4 == 0 && height % 4 == 0;
	const bool canUsePacked16 = device && device->GetBackendType() == ERHIBackend::OpenGL;
	PixelFormat targetFormat = automatic ? sourceFormat : format;
	if (automatic && colorImage)
		targetFormat = hasAlpha ? PixelFormat::DXT5 : PixelFormat::DXT1;
	if (RHISoftwareS3TC::IsS3TCFormat(targetFormat) && !canUseBC)
	{
		// Do not encode blocks only to have the backend expand them to RGBA8 again.
		targetFormat = canUsePacked16 ? (hasAlpha ? PixelFormat::A4R4G4B4 : PixelFormat::R5G6B5) :
			(sourceBC ? PixelFormat::A8R8G8B8 : sourceFormat);
	}

	const int mipCount = (std::max)(1, image->getNumberOfMipmaps());
	std::vector<MipmapInfo> mips(static_cast<size_t>(mipCount));
	std::vector<std::vector<uint8_t>> storage(static_cast<size_t>(mipCount));
	struct FreePixels { void operator()(unsigned char* pixels) const { free(pixels); } };
	std::vector<std::unique_ptr<unsigned char, FreePixels>> convertedStorage(static_cast<size_t>(mipCount));
	int mipWidth = width, mipHeight = height;
	for (int level = 0; level < mipCount; ++level)
	{
		const unsigned char* input = mipCount > 1 ? image->getMipmaps()[level].address : image->getData();
		const size_t inputSize = mipCount > 1 ? image->getMipmaps()[level].len : image->getDataLen();
		if (!input || !inputSize)
			return false;
		if (sourceFormat == targetFormat)
		{
			mips[level].address = const_cast<unsigned char*>(input);
			mips[level].len = static_cast<int>(inputSize);
		}
		else
		{
			std::vector<uint8_t> decoded;
			PixelFormat inputFormat = sourceFormat;
			size_t byteSize = inputSize;
			if (sourceBC)
			{
				if (!RHISoftwareS3TC::DecodeToRGBA8(sourceFormat, input, inputSize, mipWidth, mipHeight, decoded))
					return false;
				input = decoded.data();
				byteSize = decoded.size();
				inputFormat = PixelFormat::A8R8G8B8;
			}
			const bool encode = RHISoftwareS3TC::IsS3TCFormat(targetFormat);
			// Encode RGB directly in 4x4 blocks instead of allocating a full RGBA expansion.
			const PixelFormat conversionFormat = encode ?
				(inputFormat == PixelFormat::R8G8B8 ? inputFormat : PixelFormat::A8R8G8B8) : targetFormat;
			unsigned char* converted = nullptr;
			size_t convertedSize = 0;
			const PixelFormat convertedFormat = ParaImage::convertDataToFormat(input, byteSize, inputFormat,
				conversionFormat, &converted, &convertedSize);
			bool ok = converted && convertedFormat == conversionFormat;
			if (ok)
			{
				if (encode)
				{
					ok = RHISoftwareS3TC::EncodeFromPixels(targetFormat, convertedFormat, converted, convertedSize, mipWidth, mipHeight, storage[level]);
					mips[level].address = storage[level].data();
					mips[level].len = static_cast<int>(storage[level].size());
				}
				else
				{
					if (converted == decoded.data())
						storage[level] = std::move(decoded);
					mips[level].address = converted;
					mips[level].len = static_cast<int>(convertedSize);
				}
			}
			if (converted != input)
			{
				if (encode) free(converted);
				else convertedStorage[level].reset(converted);
			}
			if (!ok)
				return false;
		}
		mipWidth = (std::max)(1, mipWidth >> 1);
		mipHeight = (std::max)(1, mipHeight >> 1);
	}
	_filePath = image->getFilePath();
	const bool uploaded = initWithMipmaps(mips.data(), mipCount, targetFormat, width, height);
#ifdef EMSCRIPTEN_SINGLE_THREAD
	if (guiImage && uploaded && mipCount == 1 && device && device->GetBackendType() == ERHIBackend::OpenGL)
		cacheGuiImage(mips[0]);
#endif
	_hasPremultipliedAlpha = image->hasPremultipliedAlpha();
	return uploaded;
}

bool GLTexture2D::updateWithData(const void *data, int offsetX, int offsetY, int width, int height)
{
#ifdef EMSCRIPTEN_SINGLE_THREAD
	_guiPage.reset();
#endif
	if (width <= 0 || height <= 0 || !data)
		return true;
	IRenderDevice* pDev = CGlobals::GetRenderDevice();
	if (!pDev)
		return false;
	if (pDev->GetBackendType() != ERHIBackend::OpenGL)
	{
		if (_rhiTexture)
			return _rhiTexture->UpdateSubresource(0, 0, offsetX, offsetY, static_cast<uint32_t>(width), static_cast<uint32_t>(height), data, 0);
		return pDev->GetBackendType() == ERHIBackend::Null;
	}
#ifdef PARA_ENABLE_OPENGL
	if (_name)
	{
		auto it = _pixelFormatInfoTables.find(_pixelFormat);
		if (it == _pixelFormatInfoTables.end())
			return false;
		const PixelFormatInfo& info = it->second;
		GL::bindTexture2D(_name);
		unsigned int bytesPerRow = static_cast<unsigned int>(width) * info.bpp / 8;
		glPixelStorei(GL_UNPACK_ALIGNMENT, (bytesPerRow % 4 == 0) ? 4 : ((bytesPerRow % 2 == 0) ? 2 : 1));
		glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, width, height, info.format, info.type, data);
		PE_CHECK_GL_ERROR_DEBUG();

		return true;
	}
#endif
	return false;
}

void GLTexture2D::setAntiAliasTexParameters()
{
	if (_antialiasEnabled)
	{
		return;
	}

	_antialiasEnabled = true;

#ifdef PARA_ENABLE_OPENGL
	if (_name == 0 || !CGlobals::GetRenderDevice() || CGlobals::GetRenderDevice()->GetBackendType() != ERHIBackend::OpenGL)
	{
		return;
	}

	GL::bindTexture2D(_name);

	if (!_hasMipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if CC_ENABLE_CACHE_TEXTURE_DATA
	TexParams texParams = { (GLuint)(_hasMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR),GL_LINEAR,GL_NONE,GL_NONE };
	VolatileTextureMgr::setTexParameters(this, texParams);
#endif
#endif
}


int GLTexture2D::getNumberOfMipmaps()
{
	return 0;
}

 /** Gets the width of the texture in pixels. */
int GLTexture2D::getPixelsWide() const
{

	return _pixelsWide;

}

/** Gets the height of the texture in pixels. */
int  GLTexture2D::getPixelsHigh() const
{

	return _pixelsHigh;

}



bool GLTexture2D::initWithData(const void *data, size_t dataLen, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const Size& contentSize)
{
	PE_ASSERT2(dataLen>0 && pixelsWide>0 && pixelsHigh>0, "Invalid size");

	//if data has no mipmaps, we will consider it has only one mipmap
	MipmapInfo mipmap;
	mipmap.address = (unsigned char*)data;
	mipmap.len = static_cast<int>(dataLen);
	return initWithMipmaps(&mipmap, 1, pixelFormat, pixelsWide, pixelsHigh);
}

bool GLTexture2D::initWithRHITexture(IRHITexture* rhiTex, bool takeOwnership)
{
#ifdef EMSCRIPTEN_SINGLE_THREAD
	_guiPage.reset();
#endif
	if (_rhiTexture)
	{
		if (_ownsRHITexture)
			_rhiTexture->ReleaseRHIResource();
		_rhiTexture = nullptr;
	}
	if (_ownsRHITexture && _name != 0 && CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
	{
		GL::deleteTexture(_name);
	}
	_name = 0;

	if (!rhiTex)
	{
		_ownsRHITexture = true;
		_pixelsWide = 0;
		_pixelsHigh = 0;
		_pixelFormat = PixelFormat::Unknown;
		_contentSize = Size(0.0f, 0.0f);
		return false;
	}

	_rhiTexture = rhiTex;
	_ownsRHITexture = takeOwnership;
	_pixelsWide = static_cast<int>(rhiTex->GetWidth());
	_pixelsHigh = static_cast<int>(rhiTex->GetHeight());
	_pixelFormat = rhiTex->GetFormat();
	_contentSize = Size(static_cast<float>(_pixelsWide), static_cast<float>(_pixelsHigh));
	_maxS = 1.0f;
	_maxT = 1.0f;
	_hasPremultipliedAlpha = false;
	_hasMipmaps = rhiTex->GetMipLevels() > 1;
	if (CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
	{
		_name = static_cast<GLuint>(reinterpret_cast<uintptr_t>(rhiTex->GetNativeHandle()));
	}
	return true;
}

void GLTexture2D::detachRHITexture()
{
	if (!_ownsRHITexture)
	{
		_rhiTexture = nullptr;
		_name = 0;
		_ownsRHITexture = true;
	}
}

bool GLTexture2D::initWithMipmaps(const MipmapInfo* mipmaps, int mipmapsNum, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh)
{
#ifdef EMSCRIPTEN_SINGLE_THREAD
	_guiPage.reset();
#endif
	IRenderDevice* pDev = CGlobals::GetRenderDevice();
	if (!pDev || pDev->GetBackendType() != ERHIBackend::OpenGL)
	{
		if (_rhiTexture)
		{
			if (_ownsRHITexture)
				_rhiTexture->ReleaseRHIResource();
			_rhiTexture = nullptr;
		}
		_ownsRHITexture = true;
		_pixelsWide = pixelsWide;
		_pixelsHigh = pixelsHigh;
		_pixelFormat = pixelFormat;
		_contentSize = Size((float)pixelsWide, (float)pixelsHigh);
		_maxS = 1.0f;
		_maxT = 1.0f;
		_hasPremultipliedAlpha = false;
		_hasMipmaps = mipmapsNum > 1;

		if (pDev && mipmaps && mipmapsNum > 0 && pixelsWide > 0 && pixelsHigh > 0)
		{
			std::vector<RHIMipData> rhiMips(static_cast<size_t>(mipmapsNum));
			for (int i = 0; i < mipmapsNum; ++i)
			{
				rhiMips[i].data = mipmaps[i].address;
				rhiMips[i].dataSize = static_cast<uint32_t>(mipmaps[i].len > 0 ? mipmaps[i].len : 0);
				rhiMips[i].rowPitch = 0;
			}
			_rhiTexture = pDev->CreateTexture(
				static_cast<uint32_t>(pixelsWide),
				static_cast<uint32_t>(pixelsHigh),
				pixelFormat,
				static_cast<uint32_t>(mipmapsNum),
				ERHITextureUsage::ShaderResource | ERHITextureUsage::Dynamic,
				rhiMips.data(),
				static_cast<uint32_t>(rhiMips.size()));
			return _rhiTexture != nullptr || pDev->GetBackendType() == ERHIBackend::Null;
		}
		return true;
	}

#ifdef PARA_ENABLE_OPENGL
	//the pixelFormat must be a certain value
	PE_ASSERT2(pixelFormat != PixelFormat::COUNT && pixelFormat != PixelFormat::Unknown, "the \"pixelFormat\" param must be a certain value!");
	PE_ASSERT2(pixelsWide>0 && pixelsHigh>0, "Invalid size");

	if (mipmapsNum <= 0)
	{
		OUTPUT_LOG("WARNING: mipmap number is less than 1");
		return false;
	}


	if (_pixelFormatInfoTables.find(pixelFormat) == _pixelFormatInfoTables.end())
	{
		OUTPUT_LOG("WARNING: unsupported pixelformat: %lx", (unsigned long)pixelFormat);
		return false;
	}

	const PixelFormatInfo& info = _pixelFormatInfoTables.at(pixelFormat);

	//Set the row align only when mipmapsNum == 1 and the data is uncompressed
	if (mipmapsNum == 1 && !info.compressed)
	{
		unsigned int bytesPerRow = pixelsWide * info.bpp / 8;

		if (bytesPerRow % 8 == 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
		}
		else if (bytesPerRow % 4 == 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
		else if (bytesPerRow % 2 == 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		}
		else
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
	}
	else
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	if (_rhiTexture)
	{
		if (_ownsRHITexture)
			_rhiTexture->ReleaseRHIResource();
		_rhiTexture = nullptr;
	}
	if (_name != 0)
	{
		if (_ownsRHITexture)
			GL::deleteTexture(_name);
		_name = 0;
	}
	_ownsRHITexture = true;

	glGenTextures(1, &_name);
	GL::bindTexture2D(_name);

	if (mipmapsNum == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST);
	}
	else
	{
		// note: GL_LINEAR_MIPMAP_NEAREST is the only GL_LINEAR that makes sense to use with mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(__APPLE__) && !defined(PARAENGINE_MOBILE)
	if (pixelFormat == PixelFormat::A8)
	{
		const GLint swizzle[4] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	}
	else if (pixelFormat == PixelFormat::L8)
	{
		const GLint swizzle[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	}
	else if (pixelFormat == PixelFormat::A8L8)
	{
		const GLint swizzle[4] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	}
#endif

#if CC_ENABLE_CACHE_TEXTURE_DATA
	if (_antialiasEnabled)
	{
		TexParams texParams = { (GLuint)(_hasMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR),GL_LINEAR,GL_NONE,GL_NONE };
		VolatileTextureMgr::setTexParameters(this, texParams);
	}
	else
	{
		TexParams texParams = { (GLuint)(_hasMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST),GL_NEAREST,GL_NONE,GL_NONE };
		VolatileTextureMgr::setTexParameters(this, texParams);
	}
#endif

	// clean possible GL error
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		OUTPUT_LOG("OpenGL error 0x%04X in %s %s %d\n", err, __FILE__, __FUNCTION__, __LINE__);
	}

	// Specify OpenGL texture image
	int width = pixelsWide;
	int height = pixelsHigh;

	for (int i = 0; i < mipmapsNum; ++i)
	{
		unsigned char *data = mipmaps[i].address;
		GLsizei datalen = mipmaps[i].len;

		if (info.compressed)
		{
			bool hwUploaded = false;
			const auto* pDev = CGlobals::GetRenderDevice();
			const bool supportsBC = pDev ? pDev->GetCaps().supportsBCCompression : false;
			if (!RHISoftwareS3TC::IsS3TCFormat(pixelFormat) || supportsBC)
			{
				glCompressedTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, datalen, data);
				if (glGetError() == GL_NO_ERROR)
				{
					hwUploaded = true;
				}
			}

			if (!hwUploaded && RHISoftwareS3TC::IsS3TCFormat(pixelFormat))
			{
				std::vector<uint8_t> rgbaDecoded;
				if (RHISoftwareS3TC::DecodeToRGBA8(pixelFormat, data, static_cast<size_t>(datalen), static_cast<uint32_t>(width), static_cast<uint32_t>(height), rgbaDecoded))
				{
					glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
					glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaDecoded.data());
					hwUploaded = true;
				}
			}

			if (!hwUploaded)
			{
				OUTPUT_LOG("Texture2D. WARNING: Compressed texture format %d could not be uploaded or decoded\n", (int)pixelFormat);
				return false;
			}
		}
		else
		{
			GLint internalFormat = info.internalFormat;
#if defined(__EMSCRIPTEN__) || defined(PARAENGINE_MOBILE)
			if (pDev->GetCaps().featureLevel >= 300)
			{
				if (pixelFormat == PixelFormat::R5G6B5) internalFormat = GL_RGB565;
				else if (pixelFormat == PixelFormat::A4R4G4B4) internalFormat = GL_RGBA4;
				else if (pixelFormat == PixelFormat::A1R5G5B5) internalFormat = GL_RGB5_A1;
			}
#else
			if (pixelFormat == PixelFormat::R5G6B5)
				internalFormat = pDev->GetCaps().featureLevel >= 420 ? GL_RGB565 : GL_RGB5;
			else if (pixelFormat == PixelFormat::A4R4G4B4) internalFormat = GL_RGBA4;
			else if (pixelFormat == PixelFormat::A1R5G5B5) internalFormat = GL_RGB5_A1;
#endif
			glTexImage2D(GL_TEXTURE_2D, i, internalFormat, (GLsizei)width, (GLsizei)height, 0, info.format, info.type, data);
		}

		if (i > 0 && (width != height || ccNextPOT(width) != width))
		{
			OUTPUT_LOG("Texture2D. WARNING. Mipmap level %u is not squared. Texture won't render correctly. width=%d != height=%d", i, width, height);
		}

		err = glGetError();
		if (err != GL_NO_ERROR)
		{
			OUTPUT_LOG("Texture2D: Error uploading compressed texture level: %u . glError: 0x%04X", i, err);
			return false;
		}

		width = MAX(width >> 1, 1);
		height = MAX(height >> 1, 1);
	}

	_contentSize = Size((float)pixelsWide, (float)pixelsHigh);
	_pixelsWide = pixelsWide;
	_pixelsHigh = pixelsHigh;
	_pixelFormat = pixelFormat;
	_maxS = 1;
	_maxT = 1;

	_hasPremultipliedAlpha = false;
	_hasMipmaps = mipmapsNum > 1;

	// shader
	setGLProgram(GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE));
	return true;
#else
	return false;
#endif
}

void GLTexture2D::setGLProgram(GLProgram* shaderProgram)
{
	if(shaderProgram)
		shaderProgram->addref();
	SAFE_RELEASE(_shaderProgram);
	_shaderProgram = shaderProgram;
}


GLuint GLTexture2D::getName() const
{
	return _name;
}

void GLTexture2D::bind()
{
	if (CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
		GL::bindTexture2D(this->_name);
}
 
void GLTexture2D::bindN(GLuint textureUnit)
{
	if (CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
		GL::bindTexture2DN(textureUnit, this->_name);
}


void GLTexture2D::bind(GLTexture2D* pTex)
{
	if (CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
		GL::bindTexture2D(pTex == nullptr ? 0 : pTex->_name);
}

void GLTexture2D::bindN(GLTexture2D* pTex, GLuint textureUnit)
{
	if (CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
		GL::bindTexture2DN(textureUnit, pTex == nullptr ? 0 : pTex->_name);
}

GLuint GLTexture2D::getName(const GLTexture2D* pTex)
{
	return pTex == nullptr ? 0 : pTex->getName();
}

void GLTexture2D::setTexParameters(const TexParams& texParams)
{
#ifdef EMSCRIPTEN_SINGLE_THREAD
	_guiPage.reset();
#endif
#ifdef PARA_ENABLE_OPENGL
	if (_name == 0 || !CGlobals::GetRenderDevice() || CGlobals::GetRenderDevice()->GetBackendType() != ERHIBackend::OpenGL)
		return;

	GL::bindTexture2D(_name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT);

	PE_CHECK_GL_ERROR_DEBUG();

#if CC_ENABLE_CACHE_TEXTURE_DATA
	VolatileTextureMgr::setTexParameters(this, texParams);
#endif
#else
	(void)texParams;
#endif
}

void GLTexture2D::setAliasTexParameters()
{
#ifdef EMSCRIPTEN_SINGLE_THREAD
	_guiPage.reset();
#endif
	if (!_antialiasEnabled)
	{
		return;
	}

	_antialiasEnabled = false;

#ifdef PARA_ENABLE_OPENGL
	if (_name == 0 || !CGlobals::GetRenderDevice() || CGlobals::GetRenderDevice()->GetBackendType() != ERHIBackend::OpenGL)
	{
		return;
	}

	GL::bindTexture2D(_name);

	if (!_hasMipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	PE_CHECK_GL_ERROR_DEBUG();
#endif
}



void GLTexture2D::RendererRecreated()
{
#ifdef EMSCRIPTEN_SINGLE_THREAD
	if (_guiPage)
	{
		_guiPage->texture.RendererRecreated();
		_guiPage.reset();
	}
#endif
	if (_rhiTexture)
	{
		if (_ownsRHITexture)
			_rhiTexture->ReleaseRHIResource();
		_rhiTexture = nullptr;
	}
	_name = 0;
	_ownsRHITexture = true;
}

// converter function end
//////////////////////////////////////////////////////////////////////////
