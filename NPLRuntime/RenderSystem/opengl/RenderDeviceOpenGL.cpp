#include <stdexcept>
#include <stack>
#include <vector>
#include <cstring>
#include <cstdio>
#include "ParaEngine.h"
#include "renderer/VertexDeclarationOpenGL.h"
#include "math/ParaViewport.h"
#include "RenderDeviceOpenGL.h"
#include "ViewportManager.h"
#include "OpenGLWrapper/GLType.h"
#include "OpenGLWrapper/GLProgram.h"
#include "OpenGLWrapper/GLTexture2D.h"
#include "RHISoftwareS3TC.h"

using namespace ParaEngine;

#ifdef EMSCRIPTEN
#define USE_USER_POINTER_VBO
#endif

#ifndef GL_MIN
#ifdef GL_MIN_EXT
#define GL_MIN GL_MIN_EXT
#else
#define GL_MIN 0x8007
#endif
#endif

#ifndef GL_MAX
#ifdef GL_MAX_EXT
#define GL_MAX GL_MAX_EXT
#else
#define GL_MAX 0x8008
#endif
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
#ifndef GL_DEPTH24_STENCIL8_DEFAULT
#ifdef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8_DEFAULT GL_DEPTH24_STENCIL8
#elif defined(GL_DEPTH24_STENCIL8_OES)
#define GL_DEPTH24_STENCIL8_DEFAULT GL_DEPTH24_STENCIL8_OES
#else
#define GL_DEPTH24_STENCIL8_DEFAULT 0x88F0
#endif
#endif

namespace ParaEngine
{
	namespace
	{
		class CDynamicBufferObject
		{
		public:
			explicit CDynamicBufferObject(GLenum nBufferType = GL_ARRAY_BUFFER)
				: m_nBufferType(nBufferType)
				, m_nCurrentSize(0)
				, m_nNextFreeIndex(0)
				, m_devicePointer(0)
				, m_bUseRingBuffer(true)
				, m_nMinBufferSize(1024 * 100)
			{
			}

			void GenerateBuffer()
			{
				glGenBuffers(1, &m_devicePointer);
				m_nCurrentSize = 0;
				m_nNextFreeIndex = 0;
			}

			void UploadUserPointer(int nCountBytes, const void* pData, int nFrom = 0)
			{
				GLuint id = GetDevicePointer();
				glBindBuffer(m_nBufferType, id);
				int nSize = nCountBytes + nFrom;
				if (m_nCurrentSize < nSize)
				{
					int nTotalSize = (m_bUseRingBuffer && (m_nMinBufferSize > nSize)) ? m_nMinBufferSize : nSize;
					if (nFrom == 0 && nTotalSize == nSize)
					{
						glBufferData(m_nBufferType, nSize, pData, m_bUseRingBuffer ? GL_STREAM_DRAW : GL_DYNAMIC_DRAW);
					}
					else
					{
						glBufferData(m_nBufferType, nTotalSize, nullptr, m_bUseRingBuffer ? GL_STREAM_DRAW : GL_DYNAMIC_DRAW);
						glBufferSubData(m_nBufferType, nFrom, nCountBytes, pData);
					}
					m_nCurrentSize = nTotalSize;
				}
				else
				{
					glBufferSubData(m_nBufferType, nFrom, nCountBytes, pData);
				}
				PE_CHECK_GL_ERROR_DEBUG();
				m_nNextFreeIndex = m_bUseRingBuffer ? nSize : 0;
			}

			int UploadUserPointerRingBuffer(int nCount, const void* pData, int VertexStreamZeroStride)
			{
				if (VertexStreamZeroStride <= 0)
					return 0;
				int nOffsetIndex = (m_nNextFreeIndex == 0) ? 0 : ((m_nNextFreeIndex / VertexStreamZeroStride) + 1);
				int nFromBytes = nOffsetIndex * VertexStreamZeroStride;
				int nTotalBytes = nCount * VertexStreamZeroStride;
				int nSize = nFromBytes + nTotalBytes;
				if (nSize < m_nCurrentSize)
				{
					UploadUserPointer(nTotalBytes, pData, nFromBytes);
				}
				else
				{
					nOffsetIndex = 0;
					UploadUserPointer(nTotalBytes, pData, 0);
				}
				return nOffsetIndex;
			}

			GLuint GetDevicePointer()
			{
				if (m_devicePointer == 0)
				{
					GenerateBuffer();
				}
				return m_devicePointer;
			}

		private:
			GLenum m_nBufferType;
			int m_nCurrentSize;
			int m_nNextFreeIndex;
			GLuint m_devicePointer;
			bool m_bUseRingBuffer;
			int m_nMinBufferSize;
		};

		CDynamicBufferObject g_userPointerVertexBuffer(GL_ARRAY_BUFFER);
		CDynamicBufferObject g_userPointerIndexBuffer(GL_ELEMENT_ARRAY_BUFFER);

		class GLBuffer : public IRHIBuffer
		{
		public:
			GLBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage, const void* initialData)
				: m_bufferId(0)
				, m_target(isIndexBuffer ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER)
				, m_byteSize(byteSize)
				, m_isIndexBuffer(isIndexBuffer)
				, m_glUsage(GL_STATIC_DRAW)
				, m_mapOffset(0)
				, m_mapSize(0)
				, m_mapMode(ERHIMapMode::Write)
				, m_isMapped(false)
			{
				if (HasFlag(usage, ERHIBufferUsage::Stream))
					m_glUsage = GL_STREAM_DRAW;
				else if (HasFlag(usage, ERHIBufferUsage::Dynamic))
					m_glUsage = GL_DYNAMIC_DRAW;

				glGenBuffers(1, &m_bufferId);
				GLint prevBinding = 0;
				glGetIntegerv(m_isIndexBuffer ? GL_ELEMENT_ARRAY_BUFFER_BINDING : GL_ARRAY_BUFFER_BINDING, &prevBinding);
				glBindBuffer(m_target, m_bufferId);
				glBufferData(m_target, static_cast<GLsizeiptr>(m_byteSize), initialData, m_glUsage);
				glBindBuffer(m_target, static_cast<GLuint>(prevBinding));

				if (initialData && m_byteSize > 0 && !HasFlag(usage, ERHIBufferUsage::WriteOnly))
				{
					m_cpuShadow.resize(m_byteSize);
					std::memcpy(m_cpuShadow.data(), initialData, m_byteSize);
				}
			}

			~GLBuffer() override
			{
				if (m_bufferId != 0)
				{
					glDeleteBuffers(1, &m_bufferId);
					m_bufferId = 0;
				}
			}

			uint32_t GetSize() const override { return m_byteSize; }
			bool IsIndexBuffer() const override { return m_isIndexBuffer; }
			void* GetNativeHandle() const override { return reinterpret_cast<void*>(static_cast<uintptr_t>(m_bufferId)); }

			void* Map(uint32_t offset, uint32_t size, ERHIMapMode mode) override
			{
				if (offset >= m_byteSize)
					return nullptr;
				uint32_t actualSize = (size == 0 || offset + size > m_byteSize) ? (m_byteSize - offset) : size;
				if (m_cpuShadow.size() < m_byteSize)
					m_cpuShadow.resize(m_byteSize, 0);
				m_mapOffset = offset;
				m_mapSize = actualSize;
				m_mapMode = mode;
				m_isMapped = true;
				return m_cpuShadow.data() + offset;
			}

			void Unmap() override
			{
				if (!m_isMapped)
					return;
				m_isMapped = false;
				if (m_mapMode == ERHIMapMode::Read || m_bufferId == 0 || m_mapSize == 0)
					return;

				GLint prevBinding = 0;
				glGetIntegerv(m_isIndexBuffer ? GL_ELEMENT_ARRAY_BUFFER_BINDING : GL_ARRAY_BUFFER_BINDING, &prevBinding);
				glBindBuffer(m_target, m_bufferId);
				if (m_mapMode == ERHIMapMode::WriteDiscard && m_mapOffset == 0 && m_mapSize == m_byteSize)
				{
					glBufferData(m_target, static_cast<GLsizeiptr>(m_byteSize), nullptr, m_glUsage);
					glBufferSubData(m_target, 0, static_cast<GLsizeiptr>(m_mapSize), m_cpuShadow.data());
				}
				else
				{
					glBufferSubData(m_target, static_cast<GLintptr>(m_mapOffset), static_cast<GLsizeiptr>(m_mapSize), m_cpuShadow.data() + m_mapOffset);
				}
				glBindBuffer(m_target, static_cast<GLuint>(prevBinding));
			}

			bool UpdateData(uint32_t offset, uint32_t size, const void* data) override
			{
				if (!data || size == 0 || offset + size > m_byteSize || m_bufferId == 0)
					return false;
				if (!m_cpuShadow.empty())
				{
					if (m_cpuShadow.size() < m_byteSize)
						m_cpuShadow.resize(m_byteSize, 0);
					std::memcpy(m_cpuShadow.data() + offset, data, size);
				}

				GLint prevBinding = 0;
				glGetIntegerv(m_isIndexBuffer ? GL_ELEMENT_ARRAY_BUFFER_BINDING : GL_ARRAY_BUFFER_BINDING, &prevBinding);
				glBindBuffer(m_target, m_bufferId);
				if (offset == 0 && size == m_byteSize && m_glUsage != GL_STATIC_DRAW)
				{
					glBufferData(m_target, static_cast<GLsizeiptr>(m_byteSize), nullptr, m_glUsage);
				}
				glBufferSubData(m_target, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
				glBindBuffer(m_target, static_cast<GLuint>(prevBinding));
				return true;
			}

		private:
			GLuint m_bufferId;
			GLenum m_target;
			uint32_t m_byteSize;
			bool m_isIndexBuffer;
			GLenum m_glUsage;
			std::vector<uint8_t> m_cpuShadow;
			uint32_t m_mapOffset;
			uint32_t m_mapSize;
			ERHIMapMode m_mapMode;
			bool m_isMapped;
		};

		class GLRawTexture : public IRHITexture
		{
		public:
			GLRawTexture(GLuint texId, uint32_t width, uint32_t height, uint32_t mipLevels, PixelFormat format, GLenum glFormat, GLenum glType)
				: m_texId(texId)
				, m_width(width)
				, m_height(height)
				, m_mipLevels(mipLevels > 0 ? mipLevels : 1)
				, m_format(format)
				, m_glFormat(glFormat)
				, m_glType(glType)
			{
			}

			~GLRawTexture() override
			{
				if (m_texId != 0)
				{
					GL::deleteTexture(m_texId);
					m_texId = 0;
				}
			}

			uint32_t GetWidth() const override { return m_width; }
			uint32_t GetHeight() const override { return m_height; }
			uint32_t GetMipLevels() const override { return m_mipLevels; }
			PixelFormat GetFormat() const override { return m_format; }
			void* GetNativeHandle() const override { return reinterpret_cast<void*>(static_cast<uintptr_t>(m_texId)); }

			bool UpdateSubresource(uint32_t mipLevel, uint32_t arraySlice, int x, int y, uint32_t width, uint32_t height, const void* data, uint32_t /*rowPitch*/) override
			{
				if (!data || arraySlice != 0 || m_texId == 0 || m_glFormat == 0)
					return false;
				GL::bindTexture2D(m_texId);
				glTexSubImage2D(GL_TEXTURE_2D, static_cast<GLint>(mipLevel), x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height), m_glFormat, m_glType, data);
				return true;
			}

		private:
			GLuint m_texId;
			uint32_t m_width;
			uint32_t m_height;
			uint32_t m_mipLevels;
			PixelFormat m_format;
			GLenum m_glFormat;
			GLenum m_glType;
		};

		bool MapPixelFormatToGL(PixelFormat fmt, GLenum& outInternal, GLenum& outFormat, GLenum& outType, uint32_t& outBpp)
		{
			switch (fmt)
			{
			case PixelFormat::A8R8G8B8:
			case PixelFormat::A8B8G8R8:
			case PixelFormat::X8R8G8B8:
			case PixelFormat::X8B8G8R8:
				outInternal = GL_RGBA;
				outFormat = GL_RGBA;
				outType = GL_UNSIGNED_BYTE;
				outBpp = 32;
				return true;
			case PixelFormat::R8G8B8:
				outInternal = GL_RGB;
				outFormat = GL_RGB;
				outType = GL_UNSIGNED_BYTE;
				outBpp = 24;
				return true;
			case PixelFormat::A8:
				outInternal = GL_ALPHA;
				outFormat = GL_ALPHA;
				outType = GL_UNSIGNED_BYTE;
				outBpp = 8;
				return true;
			case PixelFormat::L8:
				outInternal = GL_LUMINANCE;
				outFormat = GL_LUMINANCE;
				outType = GL_UNSIGNED_BYTE;
				outBpp = 8;
				return true;
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
			case PixelFormat::R16F:
				outInternal = GL_R16F;
				outFormat = GL_RED;
				outType = GL_HALF_FLOAT;
				outBpp = 16;
				return true;
			case PixelFormat::R32F:
				outInternal = GL_R32F;
				outFormat = GL_RED;
				outType = GL_FLOAT;
				outBpp = 32;
				return true;
			case PixelFormat::G16R16F:
				outInternal = GL_RG16F;
				outFormat = GL_RG;
				outType = GL_HALF_FLOAT;
				outBpp = 32;
				return true;
			case PixelFormat::G32R32F:
				outInternal = GL_RG32F;
				outFormat = GL_RG;
				outType = GL_FLOAT;
				outBpp = 64;
				return true;
			case PixelFormat::A16B16G16R16F:
				outInternal = GL_RGBA16F;
				outFormat = GL_RGBA;
				outType = GL_HALF_FLOAT;
				outBpp = 64;
				return true;
			case PixelFormat::A32B32G32R32F:
				outInternal = GL_RGBA32F;
				outFormat = GL_RGBA;
				outType = GL_FLOAT;
				outBpp = 128;
				return true;
			case PixelFormat::D24S8:
			case PixelFormat::D24FS8:
				outInternal = GL_DEPTH24_STENCIL8;
				outFormat = GL_DEPTH_STENCIL;
				outType = GL_UNSIGNED_INT_24_8;
				outBpp = 32;
				return true;
			case PixelFormat::D32:
			case PixelFormat::D32F_LOCKABLE:
				outInternal = GL_DEPTH_COMPONENT32F;
				outFormat = GL_DEPTH_COMPONENT;
				outType = GL_FLOAT;
				outBpp = 32;
				return true;
#endif
			case PixelFormat::D16:
			case PixelFormat::D16_LOCKABLE:
				outInternal = GL_DEPTH_COMPONENT16;
				outFormat = GL_DEPTH_COMPONENT;
				outType = GL_UNSIGNED_SHORT;
				outBpp = 16;
				return true;
			default:
				return false;
			}
		}

		class GLRenderTarget : public IRHIRenderTarget
		{
		public:
			GLRenderTarget(uint32_t width, uint32_t height, PixelFormat colorFormat, PixelFormat depthFormat, uint32_t colorTargetCount)
				: m_fbo(0)
				, m_depthRbo(0)
				, m_width(width)
				, m_height(height)
				, m_colorFormat(colorFormat)
				, m_depthTexture(nullptr)
				, m_valid(false)
			{
				GLint oldFbo = 0;
				glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFbo);

				glGenFramebuffers(1, &m_fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

				uint32_t rtCount = colorTargetCount;
				if (rtCount > 4)
					rtCount = 4;

				for (uint32_t i = 0; i < rtCount; ++i)
				{
					GLenum internalFmt = GL_RGBA;
					GLenum fmt = GL_RGBA;
					GLenum type = GL_UNSIGNED_BYTE;
					uint32_t bpp = 32;
					if (!MapPixelFormatToGL(colorFormat, internalFmt, fmt, type, bpp))
					{
						internalFmt = GL_RGBA;
						fmt = GL_RGBA;
						type = GL_UNSIGNED_BYTE;
					}

					GLuint texId = 0;
					glGenTextures(1, &texId);
					GL::bindTexture2D(texId);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, fmt, type, nullptr);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texId, 0);

					m_colorTextures.push_back(new GLRawTexture(texId, width, height, 1, colorFormat, fmt, type));
				}

#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
				if (rtCount > 1 && glad_glDrawBuffers != nullptr)
				{
					GLenum drawBufs[4] = {
						GL_COLOR_ATTACHMENT0,
						GL_COLOR_ATTACHMENT1,
						GL_COLOR_ATTACHMENT2,
						GL_COLOR_ATTACHMENT3
					};
					glDrawBuffers(static_cast<GLsizei>(rtCount), drawBufs);
				}
				else if (rtCount == 0 && glad_glDrawBuffer != nullptr)
				{
					glDrawBuffer(GL_NONE);
					if (glad_glReadBuffer != nullptr)
						glReadBuffer(GL_NONE);
				}
#endif

				if (depthFormat != PixelFormat::Unknown)
				{
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
					if (depthFormat == PixelFormat::D32 || depthFormat == PixelFormat::D32F_LOCKABLE || rtCount == 0)
					{
						GLuint depthTex = 0;
						glGenTextures(1, &depthTex);
						GL::bindTexture2D(depthTex);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
						m_depthTexture = new GLRawTexture(depthTex, width, height, 1, depthFormat, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
					}
					else
#endif
					{
						glGenRenderbuffers(1, &m_depthRbo);
						glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
						if (depthFormat == PixelFormat::D16 || depthFormat == PixelFormat::D16_LOCKABLE)
						{
							glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
							glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);
						}
						else
						{
							glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_DEFAULT, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
							glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);
							glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);
						}
						glBindRenderbuffer(GL_RENDERBUFFER, 0);
					}
				}

				m_valid = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
				glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(oldFbo));
			}

			~GLRenderTarget() override
			{
				for (auto* tex : m_colorTextures)
				{
					if (tex)
						tex->ReleaseRHIResource();
				}
				m_colorTextures.clear();

				if (m_depthTexture)
				{
					m_depthTexture->ReleaseRHIResource();
					m_depthTexture = nullptr;
				}
				if (m_depthRbo != 0)
				{
					glDeleteRenderbuffers(1, &m_depthRbo);
					m_depthRbo = 0;
				}
				if (m_fbo != 0)
				{
					glDeleteFramebuffers(1, &m_fbo);
					m_fbo = 0;
				}
			}

			bool IsValid() const { return m_valid; }
			GLuint GetFBO() const { return m_fbo; }
			uint32_t GetColorCount() const { return static_cast<uint32_t>(m_colorTextures.size()); }
			uint32_t GetWidth() const override { return m_width; }
			uint32_t GetHeight() const override { return m_height; }
			PixelFormat GetFormat() const override { return m_colorFormat; }
			IRHITexture* GetColorTexture(uint32_t index = 0) const override
			{
				return (index < m_colorTextures.size()) ? m_colorTextures[index] : nullptr;
			}
			IRHITexture* GetDepthTexture() const override { return m_depthTexture; }
			void* GetNativeHandle() const override { return reinterpret_cast<void*>(static_cast<uintptr_t>(m_fbo)); }

		private:
			GLuint m_fbo;
			GLuint m_depthRbo;
			uint32_t m_width;
			uint32_t m_height;
			PixelFormat m_colorFormat;
			std::vector<IRHITexture*> m_colorTextures;
			IRHITexture* m_depthTexture;
			bool m_valid;
		};

#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
		class GLOcclusionQuery : public IRHIOcclusionQuery
		{
		public:
			GLOcclusionQuery() : m_queryId(0), m_active(false)
			{
				if (glad_glGenQueries != nullptr)
					glGenQueries(1, &m_queryId);
			}

			~GLOcclusionQuery() override
			{
				if (m_queryId != 0 && glad_glDeleteQueries != nullptr)
				{
					glDeleteQueries(1, &m_queryId);
					m_queryId = 0;
				}
			}

			bool Begin() override
			{
				if (m_queryId == 0 || glad_glBeginQuery == nullptr)
					return false;
				glBeginQuery(GL_SAMPLES_PASSED, m_queryId);
				m_active = true;
				return true;
			}

			void End() override
			{
				if (m_active && glad_glEndQuery != nullptr)
				{
					glEndQuery(GL_SAMPLES_PASSED);
					m_active = false;
				}
			}

			bool GetResult(uint64_t& outSamplesPassed, bool flush) override
			{
				if (m_queryId == 0 || glad_glGetQueryObjectuiv == nullptr)
				{
					outSamplesPassed = 1;
					return true;
				}
				if (!flush)
				{
					GLuint available = 0;
					glGetQueryObjectuiv(m_queryId, GL_QUERY_RESULT_AVAILABLE, &available);
					if (!available)
						return false;
				}
				GLuint count = 0;
				glGetQueryObjectuiv(m_queryId, GL_QUERY_RESULT, &count);
				outSamplesPassed = static_cast<uint64_t>(count);
				return true;
			}

		private:
			GLuint m_queryId;
			bool m_active;
		};
#endif

		uint32_t ToGLBlendValue(uint32_t value)
		{
			switch (value)
			{
			case RSV_BLEND_ZERO:         return GL_ZERO;
			case RSV_BLEND_ONE:          return GL_ONE;
			case RSV_BLEND_SRCCOLOR:     return GL_SRC_COLOR;
			case RSV_BLEND_INVSRCCOLOR:  return GL_ONE_MINUS_SRC_COLOR;
			case RSV_BLEND_SRCALPHA:     return GL_SRC_ALPHA;
			case RSV_BLEND_INVSRCALPHA:  return GL_ONE_MINUS_SRC_ALPHA;
			case RSV_BLEND_DESTALPHA:    return GL_DST_ALPHA;
			case RSV_BLEND_INVDESTALPHA: return GL_ONE_MINUS_DST_ALPHA;
			case RSV_BLEND_DESTCOLOR:    return GL_DST_COLOR;
			case RSV_BLEND_INVDESTCOLOR: return GL_ONE_MINUS_DST_COLOR;
			case RSV_BLEND_SRCALPHASAT:  return GL_SRC_ALPHA_SATURATE;
			case RSV_BLEND_BLENDFACTOR:  return GL_CONSTANT_COLOR;
			case RSV_BLEND_INVBLENDFACTOR: return GL_ONE_MINUS_CONSTANT_COLOR;
			default:                     return GL_ONE;
			}
		}

		GLenum ToGLCompareFunc(uint32_t value)
		{
			switch (value)
			{
			case RSV_CMP_NEVER:        return GL_NEVER;
			case RSV_CMP_LESS:         return GL_LESS;
			case RSV_CMP_EQUAL:        return GL_EQUAL;
			case RSV_CMP_LESSEQUAL:    return GL_LEQUAL;
			case RSV_CMP_GREATER:      return GL_GREATER;
			case RSV_CMP_NOTEQUAL:     return GL_NOTEQUAL;
			case RSV_CMP_GREATEREQUAL: return GL_GEQUAL;
			case RSV_CMP_ALWAYS:
			default:                   return GL_ALWAYS;
			}
		}

		GLenum ToGLStencilOp(uint32_t value)
		{
			switch (value)
			{
			case RSV_STENCILOP_ZERO:    return GL_ZERO;
			case RSV_STENCILOP_REPLACE: return GL_REPLACE;
			case RSV_STENCILOP_INCRSAT: return GL_INCR;
			case RSV_STENCILOP_DECRSAT: return GL_DECR;
			case RSV_STENCILOP_INVERT:  return GL_INVERT;
			case RSV_STENCILOP_INCR:    return GL_INCR_WRAP;
			case RSV_STENCILOP_DECR:    return GL_DECR_WRAP;
			case RSV_STENCILOP_KEEP:
			default:                    return GL_KEEP;
			}
		}

		bool ToGLPrimitiveModeAndCount(EPrimitiveType primType, uint32_t primCount, GLenum& outMode, GLsizei& outVertCount)
		{
			if (primCount == 0)
				return false;
			switch (primType)
			{
			case EPrimitiveType::POINTLIST:
				outMode = GL_POINTS;
				outVertCount = static_cast<GLsizei>(primCount);
				return true;
			case EPrimitiveType::LINELIST:
				outMode = GL_LINES;
				outVertCount = static_cast<GLsizei>(primCount * 2);
				return true;
			case EPrimitiveType::LINESTRIP:
				outMode = GL_LINE_STRIP;
				outVertCount = static_cast<GLsizei>(primCount + 1);
				return true;
			case EPrimitiveType::TRIANGLELIST:
				outMode = GL_TRIANGLES;
				outVertCount = static_cast<GLsizei>(primCount * 3);
				return true;
			case EPrimitiveType::TRIANGLESTRIP:
				outMode = GL_TRIANGLE_STRIP;
				outVertCount = static_cast<GLsizei>(primCount + 2);
				return true;
			case EPrimitiveType::TRIANGLEFAN:
				outMode = GL_TRIANGLE_FAN;
				outVertCount = static_cast<GLsizei>(primCount + 2);
				return true;
			default:
				return false;
			}
		}

		uint32_t MapGLAttribSlot(const VertexElement& elem, int& inoutTexIndex, int& inoutColorIndex)
		{
			switch (elem.Usage)
			{
			case D3DDECLUSAGE_POSITION:
			case D3DDECLUSAGE_POSITIONT:
				return GLProgram::VERTEX_ATTRIB_POSITION;
			case D3DDECLUSAGE_COLOR:
			{
				uint32_t slot = (inoutColorIndex <= 1)
					? static_cast<uint32_t>(GLProgram::VERTEX_ATTRIB_COLOR + inoutColorIndex)
					: static_cast<uint32_t>(GLProgram::VERTEX_ATTRIB_MAX + 0);
				++inoutColorIndex;
				return slot;
			}
			case D3DDECLUSAGE_TEXCOORD:
			{
				uint32_t idx = (elem.UsageIndex > 0) ? elem.UsageIndex : static_cast<uint32_t>(inoutTexIndex);
				++inoutTexIndex;
				if (idx < 4u)
					return static_cast<uint32_t>(GLProgram::VERTEX_ATTRIB_TEX_COORD) + idx;
				return 13u + ((idx - 4u) < 3u ? (idx - 4u) : 2u);
			}
			case D3DDECLUSAGE_NORMAL:
				return GLProgram::VERTEX_ATTRIB_NORMAL;
			case D3DDECLUSAGE_BLENDWEIGHT:
				return GLProgram::VERTEX_ATTRIB_BLEND_WEIGHT;
			case D3DDECLUSAGE_BLENDINDICES:
				return GLProgram::VERTEX_ATTRIB_BLEND_INDEX;
			case D3DDECLUSAGE_TANGENT:
				return 11u;
			case D3DDECLUSAGE_BINORMAL:
				return 12u;
			default:
				return GLProgram::VERTEX_ATTRIB_POSITION;
			}
		}

		bool MapVertexElemTypeToGL(uint8_t declType, GLint& outComponents, GLenum& outType, GLboolean& outNormalized)
		{
			switch (declType)
			{
			case D3DDECLTYPE_FLOAT1:
				outComponents = 1; outType = GL_FLOAT; outNormalized = GL_FALSE; return true;
			case D3DDECLTYPE_FLOAT2:
				outComponents = 2; outType = GL_FLOAT; outNormalized = GL_FALSE; return true;
			case D3DDECLTYPE_FLOAT3:
				outComponents = 3; outType = GL_FLOAT; outNormalized = GL_FALSE; return true;
			case D3DDECLTYPE_FLOAT4:
				outComponents = 4; outType = GL_FLOAT; outNormalized = GL_FALSE; return true;
			case D3DDECLTYPE_D3DCOLOR:
			case D3DDECLTYPE_UBYTE4N:
				outComponents = 4; outType = GL_UNSIGNED_BYTE; outNormalized = GL_TRUE; return true;
			case D3DDECLTYPE_UBYTE4:
				outComponents = 4; outType = GL_UNSIGNED_BYTE; outNormalized = GL_FALSE; return true;
			case D3DDECLTYPE_SHORT2:
				outComponents = 2; outType = GL_SHORT; outNormalized = GL_FALSE; return true;
			case D3DDECLTYPE_SHORT4:
				outComponents = 4; outType = GL_SHORT; outNormalized = GL_FALSE; return true;
			case D3DDECLTYPE_SHORT2N:
				outComponents = 2; outType = GL_SHORT; outNormalized = GL_TRUE; return true;
			case D3DDECLTYPE_SHORT4N:
				outComponents = 4; outType = GL_SHORT; outNormalized = GL_TRUE; return true;
			case D3DDECLTYPE_USHORT2N:
				outComponents = 2; outType = GL_UNSIGNED_SHORT; outNormalized = GL_TRUE; return true;
			case D3DDECLTYPE_USHORT4N:
				outComponents = 4; outType = GL_UNSIGNED_SHORT; outNormalized = GL_TRUE; return true;
			default:
				return false;
			}
		}

		std::stack<std::pair<uint32_t, uint32_t>> g_stackRenderTargetSizes;
	}

	RenderDeviceOpenGL::RenderDeviceOpenGL()
		: m_AlphaBlendingChanged(true)
		, m_BlendingChanged(true)
		, m_EnableBlending(false)
		, m_EnableSeparateAlphaBlending(false)
		, m_BlendingSource(RSV_BLEND_SRCALPHA)
		, m_BlendingDest(RSV_BLEND_INVSRCALPHA)
		, m_BlendingAlphaSource(RSV_BLEND_SRCALPHA)
		, m_BlendingAlphaDest(RSV_BLEND_INVSRCALPHA)
		, m_StencilRefValue(0)
		, m_StencilMask(0xFFFFFFFFu)
		, m_StencilFunc(RSV_CMP_ALWAYS)
		, m_StencilFail(RSV_STENCILOP_KEEP)
		, m_StencilZFail(RSV_STENCILOP_KEEP)
		, m_StencilPass(RSV_STENCILOP_KEEP)
		, m_StencilDirty(false)
		, m_CurrentVertexDeclaration(nullptr)
		, m_CurrentVertexBuffer(nullptr)
		, m_CurrentIndexBuffer(nullptr)
	{
	}

	RenderDeviceOpenGL::~RenderDeviceOpenGL()
	{
		GL::ClearCache();
	}

	void RenderDeviceOpenGL::InitCaps() const
	{
		if (m_CapsInitialized)
			return;

		m_Caps.backend = ERHIBackend::OpenGL;
		m_Caps.backendName = "OpenGL";
		const GLubyte* rendererStr = glGetString(GL_RENDERER);
		m_AdapterName = rendererStr ? reinterpret_cast<const char*>(rendererStr) : "OpenGL Adapter";
		m_Caps.adapterName = m_AdapterName.c_str();

		int major = 2;
		int minor = 0;
		const GLubyte* verStr = glGetString(GL_VERSION);
		if (verStr)
		{
			const char* s = reinterpret_cast<const char*>(verStr);
			while (*s && (*s < '0' || *s > '9'))
				++s;
			if (*s)
				std::sscanf(s, "%d.%d", &major, &minor);
		}
		m_Caps.featureLevel = static_cast<uint32_t>(major * 100 + minor * 10);

		GLint maxTexUnits = 8;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);
		m_Caps.maxTextureStages = static_cast<uint32_t>(maxTexUnits > 0 ? maxTexUnits : 8);

		GLint maxTexSize = 4096;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
		m_Caps.maxTextureSize = static_cast<uint32_t>(maxTexSize > 0 ? maxTexSize : 4096);

#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
		GLint maxDrawBufs = 1;
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBufs);
		m_Caps.maxSimultaneousRTs = static_cast<uint32_t>(maxDrawBufs > 0 ? maxDrawBufs : 1);
		m_Caps.supportsFloatRenderTargets = (m_Caps.featureLevel >= 300);
		m_Caps.supportsDepthTextures = (m_Caps.featureLevel >= 300);
		m_Caps.supportsInstancing = (glad_glDrawElementsInstanced != nullptr && glad_glVertexAttribDivisor != nullptr);
		m_Caps.supportsOcclusionQuery = (glad_glGenQueries != nullptr && glad_glGetQueryObjectuiv != nullptr);
		m_Caps.supportsTwoSidedStencil = (glad_glStencilFuncSeparate != nullptr && glad_glStencilOpSeparate != nullptr);
		m_Caps.supportsUserClipPlanes = true;
#else
		m_Caps.maxSimultaneousRTs = 1;
		m_Caps.supportsTwoSidedStencil = true;
#endif

		GLint numCompressedFormats = 0;
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedFormats);
		if (numCompressedFormats > 0 && numCompressedFormats < 256)
		{
			std::vector<GLint> formats(static_cast<size_t>(numCompressedFormats), 0);
			glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats.data());
			for (GLint f : formats)
			{
				if (f == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
					f == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
					f == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
				{
					m_Caps.supportsBCCompression = true;
					break;
				}
			}
		}

		// Drain any benign GL error from extension/format probing
		while (glGetError() != GL_NO_ERROR) {}

		m_Caps.supportsRuntimeShaderCompile = true;
		m_Caps.topLeftScreenOrigin = false;
		m_Caps.clipSpaceZeroToOne = false;
		m_CapsInitialized = true;
	}

	const RHIDeviceCaps& RenderDeviceOpenGL::GetCaps() const
	{
		if (!m_CapsInitialized)
			InitCaps();
		return m_Caps;
	}

	uint32_t RenderDeviceOpenGL::GetBackBufferWidth() const
	{
		if (m_BackBufferWidth > 0)
			return m_BackBufferWidth;
		auto* pApp = CGlobals::GetApp();
		auto* pWin = pApp ? pApp->GetRenderWindow() : nullptr;
		if (pWin)
			return pApp->IsRotateScreen() ? pWin->GetHeight() : pWin->GetWidth();
		return m_CurrentViewPort.Width;
	}

	uint32_t RenderDeviceOpenGL::GetBackBufferHeight() const
	{
		if (m_BackBufferHeight > 0)
			return m_BackBufferHeight;
		auto* pApp = CGlobals::GetApp();
		auto* pWin = pApp ? pApp->GetRenderWindow() : nullptr;
		if (pWin)
			return pApp->IsRotateScreen() ? pWin->GetWidth() : pWin->GetHeight();
		return m_CurrentViewPort.Height;
	}

	bool RenderDeviceOpenGL::CheckDeviceError(const char* filename, const char* func, int nLine)
	{
		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
		{
			CLogger::GetSingleton().WriteFormated("OpenGL error 0x%04X in %s %s %d\n", err,
				filename ? filename : "", func ? func : "", nLine);
			return false;
		}
		return true;
	}

	IRHITexture* RenderDeviceOpenGL::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels, ERHITextureUsage /*usage*/, const RHIMipData* initialMips, uint32_t initialMipCount)
	{
		if (width == 0 || height == 0)
			return nullptr;

		// Use GLTexture2D when compatible with single-mip standard/compressed formats
		const auto& infoMap = GLTexture2D::getPixelFormatInfoMap();
		if (mipLevels <= 1 && infoMap.find(format) != infoMap.end())
		{
			GLTexture2D* tex = new GLTexture2D();
			const auto& info = infoMap.at(format);
			const void* data = (initialMips && initialMipCount > 0) ? initialMips[0].data : nullptr;
			size_t dataLen = (initialMips && initialMipCount > 0 && initialMips[0].dataSize > 0)
				? static_cast<size_t>(initialMips[0].dataSize)
				: static_cast<size_t>(width) * static_cast<size_t>(height) * (info.bpp > 0 ? static_cast<size_t>((info.bpp + 7) / 8) : 4u);
			if (dataLen == 0)
				dataLen = 4;
			if (tex->initWithData(data, dataLen, format, static_cast<int>(width), static_cast<int>(height), Size(static_cast<float>(width), static_cast<float>(height))))
			{
				return tex;
			}
			tex->Release();
		}

		GLenum internalFmt = GL_RGBA;
		GLenum glFmt = GL_RGBA;
		GLenum glType = GL_UNSIGNED_BYTE;
		uint32_t bpp = 32;
		if (!MapPixelFormatToGL(format, internalFmt, glFmt, glType, bpp))
			return nullptr;

		GLuint texId = 0;
		glGenTextures(1, &texId);
		GL::bindTexture2D(texId);
		uint32_t numMips = (mipLevels > 0) ? mipLevels : 1;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (numMips > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		uint32_t mipW = width;
		uint32_t mipH = height;
		for (uint32_t m = 0; m < numMips; ++m)
		{
			const void* mipData = (initialMips && m < initialMipCount) ? initialMips[m].data : nullptr;
			glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(m), internalFmt, static_cast<GLsizei>(mipW), static_cast<GLsizei>(mipH), 0, glFmt, glType, mipData);
			mipW = (mipW > 1) ? (mipW >> 1) : 1;
			mipH = (mipH > 1) ? (mipH >> 1) : 1;
		}
		return new GLRawTexture(texId, width, height, numMips, format, glFmt, glType);
	}

	IRHIBuffer* RenderDeviceOpenGL::CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage usage, const void* initialData)
	{
		if (byteSize == 0)
			return nullptr;
		return new GLBuffer(byteSize, isIndexBuffer, usage, initialData);
	}

	IRHIRenderTarget* RenderDeviceOpenGL::CreateRenderTarget(uint32_t width, uint32_t height, PixelFormat colorFormat, PixelFormat depthFormat, uint32_t colorTargetCount, uint32_t /*msaaSamples*/)
	{
		if (width == 0 || height == 0)
			return nullptr;
		auto* rt = new GLRenderTarget(width, height, colorFormat, depthFormat, colorTargetCount);
		if (!rt->IsValid())
		{
			rt->ReleaseRHIResource();
			return nullptr;
		}
		return rt;
	}

	IRHIOcclusionQuery* RenderDeviceOpenGL::CreateOcclusionQuery()
	{
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
		if (GetCaps().supportsOcclusionQuery)
			return new GLOcclusionQuery();
#endif
		return nullptr;
	}

	bool RenderDeviceOpenGL::SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride)
	{
		if (count == 0 || targets == nullptr || targets[0] == nullptr)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_DefaultFBO);
			m_RenderTargetWidth = m_BackBufferWidth;
			m_RenderTargetHeight = m_BackBufferHeight;
			m_isBeginRenderTarget = false;
			NotifyRenderTargetsBound(0, nullptr, depthStencilOverride);
			return true;
		}

		auto* primaryRT = static_cast<GLRenderTarget*>(targets[0]);
		glBindFramebuffer(GL_FRAMEBUFFER, primaryRT->GetFBO());
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
		if (count > 1 && glad_glDrawBuffers != nullptr)
		{
			uint32_t rtCount = count > 4u ? 4u : count;
			GLenum drawBufs[4] = { GL_NONE, GL_NONE, GL_NONE, GL_NONE };
			for (uint32_t i = 0; i < rtCount; ++i)
			{
				auto* rt = static_cast<GLRenderTarget*>(targets[i]);
				IRHITexture* colorTex = rt ? rt->GetColorTexture(0) : nullptr;
				GLuint texId = colorTex ? static_cast<GLuint>(reinterpret_cast<uintptr_t>(colorTex->GetNativeHandle())) : 0;
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texId, 0);
				drawBufs[i] = texId != 0 ? (GL_COLOR_ATTACHMENT0 + i) : GL_NONE;
			}
			glDrawBuffers(static_cast<GLsizei>(rtCount), drawBufs);
		}
		else if (count == 1 && primaryRT->GetColorCount() == 1 && glad_glDrawBuffers != nullptr)
		{
			IRHITexture* colorTex0 = primaryRT->GetColorTexture(0);
			GLuint texId0 = colorTex0 ? static_cast<GLuint>(reinterpret_cast<uintptr_t>(colorTex0->GetNativeHandle())) : 0;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId0, 0);
			for (uint32_t i = 1; i < 4; ++i)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
			const GLenum att0 = GL_COLOR_ATTACHMENT0;
			glDrawBuffers(1, &att0);
		}
#endif
		m_RenderTargetWidth = primaryRT->GetWidth();
		m_RenderTargetHeight = primaryRT->GetHeight();
		m_isBeginRenderTarget = true;
		NotifyRenderTargetsBound(count, targets, depthStencilOverride);
		return true;
	}

	bool RenderDeviceOpenGL::BlitRenderTarget(IRHIRenderTarget* src, IRHIRenderTarget* dst, const RECT* srcRect, const RECT* dstRect, bool linearFilter)
	{
		GLint prevFbo = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);

		GLuint srcFbo = src ? static_cast<GLRenderTarget*>(src)->GetFBO() : m_DefaultFBO;
		GLuint dstFbo = dst ? static_cast<GLRenderTarget*>(dst)->GetFBO() : m_DefaultFBO;
		uint32_t srcW = src ? src->GetWidth() : GetBackBufferWidth();
		uint32_t srcH = src ? src->GetHeight() : GetBackBufferHeight();
		uint32_t dstW = dst ? dst->GetWidth() : GetBackBufferWidth();
		uint32_t dstH = dst ? dst->GetHeight() : GetBackBufferHeight();

#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
		if (glad_glBlitFramebuffer != nullptr)
		{
			GLint sx0 = srcRect ? srcRect->left : 0;
			GLint sy0 = srcRect ? static_cast<GLint>(srcH) - srcRect->bottom : 0;
			GLint sx1 = srcRect ? srcRect->right : static_cast<GLint>(srcW);
			GLint sy1 = srcRect ? static_cast<GLint>(srcH) - srcRect->top : static_cast<GLint>(srcH);

			GLint dx0 = dstRect ? dstRect->left : 0;
			GLint dy0 = dstRect ? static_cast<GLint>(dstH) - dstRect->bottom : 0;
			GLint dx1 = dstRect ? dstRect->right : static_cast<GLint>(dstW);
			GLint dy1 = dstRect ? static_cast<GLint>(dstH) - dstRect->top : static_cast<GLint>(dstH);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);
			glBlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, GL_COLOR_BUFFER_BIT, linearFilter ? GL_LINEAR : GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));
			return true;
		}
#endif
		// Fallback for GLES 2.0 / WebGL 1.0 when copying an equal-sized region into an offscreen color texture
		IRHITexture* dstTex = dst ? dst->GetColorTexture(0) : nullptr;
		if (dstTex != nullptr && srcW == dstW && srcH == dstH && !srcRect && !dstRect)
		{
			GLuint dstTexId = static_cast<GLuint>(reinterpret_cast<uintptr_t>(dstTex->GetNativeHandle()));
			if (dstTexId != 0)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, srcFbo);
				GL::bindTexture2D(dstTexId);
				glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, static_cast<GLsizei>(srcW), static_cast<GLsizei>(srcH));
				glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));
				return true;
			}
		}

		(void)linearFilter;
		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));
		return false;
	}

	bool RenderDeviceOpenGL::ImportExternalFramebuffer(uintptr_t nativeFboOrTexHandle, uint32_t width, uint32_t height)
	{
		m_DefaultFBO = static_cast<GLuint>(nativeFboOrTexHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DefaultFBO);
		m_BackBufferWidth = width;
		m_BackBufferHeight = height;
		m_RenderTargetWidth = width;
		m_RenderTargetHeight = height;
		return true;
	}

	bool RenderDeviceOpenGL::SetStreamSourceFreq(uint32_t streamNumber, uint32_t divider)
	{
		if (streamNumber >= CVertexDeclaration::MAX_STREAMS)
			return false;
		if (m_Streams[streamNumber].frequencyDivider != divider)
		{
			m_Streams[streamNumber].frequencyDivider = divider;
			if (m_CurrentVertexDeclaration && streamNumber > 0)
				ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr);
		}
		return true;
	}

	bool RenderDeviceOpenGL::DrawIndexedInstanced(EPrimitiveType primitiveType, int baseVertexIndex, uint32_t indexStart, uint32_t primitiveCount, uint32_t instanceCount)
	{
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
		if (instanceCount == 0 || glad_glDrawElementsInstanced == nullptr)
			return false;

		GLenum mode = GL_TRIANGLES;
		GLsizei count = 0;
		if (!ToGLPrimitiveModeAndCount(primitiveType, primitiveCount, mode, count))
			return false;

		ApplyBlendingModeChange();
		ApplyStencilStateChange();
		if (m_CurrentVertexDeclaration)
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr);

		const void* indexOffset = reinterpret_cast<const void*>(static_cast<uintptr_t>(indexStart * sizeof(uint16_t)));
		if (baseVertexIndex != 0 && glad_glDrawElementsInstancedBaseVertex != nullptr)
		{
			glDrawElementsInstancedBaseVertex(mode, count, GL_UNSIGNED_SHORT, indexOffset, static_cast<GLsizei>(instanceCount), baseVertexIndex);
		}
		else
		{
			glDrawElementsInstanced(mode, count, GL_UNSIGNED_SHORT, indexOffset, static_cast<GLsizei>(instanceCount));
		}
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
#else
		(void)primitiveType; (void)baseVertexIndex; (void)indexStart; (void)primitiveCount; (void)instanceCount;
		return false;
#endif
	}

	bool RenderDeviceOpenGL::ReadDepthPixel(int x, int y, float& outDepth)
	{
		float depth = 1.0f;
		int glY = static_cast<int>(m_RenderTargetHeight) - y - 1;
		if (glY < 0)
			glY = 0;
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
		glReadPixels(x, glY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		outDepth = depth;
		return true;
#else
		outDepth = depth;
		return false;
#endif
	}

	bool RenderDeviceOpenGL::ReadTexturePixels(IRHITexture* texture, void* pDataOut)
	{
		if (!texture || !pDataOut)
			return false;
		const GLuint texId = static_cast<GLuint>(reinterpret_cast<uintptr_t>(texture->GetNativeHandle()));
		const GLsizei width = static_cast<GLsizei>(texture->GetWidth());
		const GLsizei height = static_cast<GLsizei>(texture->GetHeight());
		if (texId == 0 || width <= 0 || height <= 0)
			return false;

		GLint prevFbo = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);

		GLuint fbo = 0;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pDataOut);
		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));
		glDeleteFramebuffers(1, &fbo);
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	uint32_t RenderDeviceOpenGL::GetRenderState(const ERenderState& State)
	{
		return m_StateTracker.GetRenderState(State);
	}

	bool RenderDeviceOpenGL::SetRenderState(const ERenderState State, const uint32_t Value)
	{
		m_StateTracker.SetRenderState(State, Value);

		switch (State)
		{
		case ERenderState::CULLMODE:
		{
			if (Value == RSV_CULL_CW)
			{
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CCW);
			}
			else if (Value == RSV_CULL_CCW)
			{
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CW);
			}
			else if (Value == RSV_CULL_NONE)
			{
				glDisable(GL_CULL_FACE);
			}
			break;
		}
		case ERenderState::ZENABLE:
		{
			if (Value)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);
			break;
		}
		case ERenderState::ZFUNC:
		{
			glDepthFunc(ToGLCompareFunc(Value));
			break;
		}
		case ERenderState::ZWRITEENABLE:
		{
			glDepthMask(Value ? GL_TRUE : GL_FALSE);
			break;
		}
		case ERenderState::SEPARATEALPHABLENDENABLE:
		{
			bool bEnabled = Value ? true : false;
			if (bEnabled == m_EnableSeparateAlphaBlending)
				break;
			m_EnableSeparateAlphaBlending = bEnabled;
			m_AlphaBlendingChanged = true;
			if (bEnabled && !m_EnableBlending)
				SetRenderState(ERenderState::ALPHABLENDENABLE, 1);
			break;
		}
		case ERenderState::ALPHABLENDENABLE:
		{
			bool bEnabled = Value ? true : false;
			if (bEnabled == m_EnableBlending)
				break;
			m_EnableBlending = bEnabled;
			if (bEnabled)
				glEnable(GL_BLEND);
			else
				glDisable(GL_BLEND);
			break;
		}
		case ERenderState::SRCBLENDALPHA:
		{
			if (Value != m_BlendingAlphaSource)
			{
				m_BlendingAlphaSource = Value;
				m_AlphaBlendingChanged = true;
			}
			break;
		}
		case ERenderState::DESTBLENDALPHA:
		{
			if (Value != m_BlendingAlphaDest)
			{
				m_BlendingAlphaDest = Value;
				m_AlphaBlendingChanged = true;
			}
			break;
		}
		case ERenderState::SRCBLEND:
		{
			if (Value != m_BlendingSource)
			{
				m_BlendingSource = Value;
				m_BlendingChanged = true;
			}
			break;
		}
		case ERenderState::DESTBLEND:
		{
			if (Value != m_BlendingDest)
			{
				m_BlendingDest = Value;
				m_BlendingChanged = true;
			}
			break;
		}
		case ERenderState::STENCILENABLE:
		{
			if (Value)
				glEnable(GL_STENCIL_TEST);
			else
				glDisable(GL_STENCIL_TEST);
			break;
		}
		case ERenderState::STENCILFUNC:
			m_StencilFunc = Value;
			m_StencilDirty = true;
			break;
		case ERenderState::STENCILREF:
			m_StencilRefValue = Value;
			m_StencilDirty = true;
			break;
		case ERenderState::STENCILMASK:
			m_StencilMask = Value;
			m_StencilDirty = true;
			break;
		case ERenderState::STENCILFAIL:
			m_StencilFail = Value;
			m_StencilDirty = true;
			break;
		case ERenderState::STENCILZFAIL:
			m_StencilZFail = Value;
			m_StencilDirty = true;
			break;
		case ERenderState::STENCILPASS:
			m_StencilPass = Value;
			m_StencilDirty = true;
			break;
		case ERenderState::STENCILWRITEMASK:
		{
			glStencilMask(Value);
			break;
		}
		case ERenderState::SCISSORTESTENABLE:
		{
			if (Value)
				glEnable(GL_SCISSOR_TEST);
			else
				glDisable(GL_SCISSOR_TEST);
			break;
		}
		case ERenderState::COLORWRITEENABLE:
		{
			glColorMask(
				(Value & 0x1) ? GL_TRUE : GL_FALSE,
				(Value & 0x2) ? GL_TRUE : GL_FALSE,
				(Value & 0x4) ? GL_TRUE : GL_FALSE,
				(Value & 0x8) ? GL_TRUE : GL_FALSE);
			break;
		}
		case ERenderState::BLENDOP:
		case ERenderState::BLENDOPALPHA:
		{
			uint32_t glValue = GL_FUNC_ADD;
			if (Value == D3DBLENDOP_ADD)
				glValue = GL_FUNC_ADD;
			else if (Value == D3DBLENDOP_SUBTRACT)
				glValue = GL_FUNC_SUBTRACT;
			else if (Value == D3DBLENDOP_REVSUBTRACT)
				glValue = GL_FUNC_REVERSE_SUBTRACT;
			else if (Value == D3DBLENDOP_MIN)
				glValue = GL_MIN;
			else if (Value == D3DBLENDOP_MAX)
				glValue = GL_MAX;
			if (State == ERenderState::BLENDOP)
				glBlendEquation(glValue);
			else
				glBlendEquationSeparate(GL_FUNC_ADD, glValue);
			break;
		}
		case ERenderState::DEPTHBIAS:
		case ERenderState::SLOPESCALEDEPTHBIAS:
		{
			uint32_t biasBits = m_StateTracker.GetRenderState(ERenderState::DEPTHBIAS);
			uint32_t slopeBits = m_StateTracker.GetRenderState(ERenderState::SLOPESCALEDEPTHBIAS);
			float bias = 0.0f;
			float slope = 0.0f;
			std::memcpy(&bias, &biasBits, sizeof(float));
			std::memcpy(&slope, &slopeBits, sizeof(float));
			if (bias != 0.0f || slope != 0.0f)
			{
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(slope, bias * 65535.0f);
			}
			else
			{
				glDisable(GL_POLYGON_OFFSET_FILL);
			}
			break;
		}
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
		case ERenderState::FILLMODE:
		{
			if (Value == RSV_FILL_WIREFRAME)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else if (Value == RSV_FILL_POINT)
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
		}
		case ERenderState::CLIPPLANEENABLE:
		{
			for (uint32_t i = 0; i < RHIStateTracker::MAX_CLIP_PLANES; ++i)
			{
				if (Value & (1u << i))
					glEnable(GL_CLIP_DISTANCE0 + i);
				else
					glDisable(GL_CLIP_DISTANCE0 + i);
			}
			break;
		}
#endif
		default:
			break;
		}

		return true;
	}

	bool RenderDeviceOpenGL::SetClipPlane(uint32_t Index, const float* pPlane)
	{
		m_StateTracker.SetClipPlane(Index, pPlane);
		return true;
	}

	bool RenderDeviceOpenGL::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t /*nDataFormat*/, uint32_t /*nDataType*/)
	{
		if (!pDataOut || nWidth <= 0 || nHeight <= 0)
			return false;
		glReadPixels(nLeft, static_cast<GLint>(m_RenderTargetHeight) - nTop - nHeight, nWidth, nHeight, GL_RGBA, GL_UNSIGNED_BYTE, pDataOut);
		return true;
	}

	int RenderDeviceOpenGL::GetMaxSimultaneousTextures()
	{
		return static_cast<int>(GetCaps().maxTextureStages);
	}

	bool RenderDeviceOpenGL::SetTexture(uint32_t stage, DeviceTexturePtr_type texture)
	{
		GLuint texId = texture ? static_cast<GLuint>(reinterpret_cast<uintptr_t>(texture->GetNativeHandle())) : 0u;
		GL::bindTexture2DN(stage, texId);
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	void RenderDeviceOpenGL::ApplyGLVertexDeclaration(CVertexDeclaration* pDecl, const void* streamZeroUserPtr, uint32_t streamZeroStrideOverride)
	{
		if (!pDecl)
			return;

		GL::enableVertexAttribs(pDecl->GetAttributeMask());

		int nTextureIndex = 0;
		int nColorIndex = 0;
		GLuint lastBoundVbo = 0xFFFFFFFFu;

		for (const auto& elem : pDecl->GetElements())
		{
			uint32_t slot = MapGLAttribSlot(elem, nTextureIndex, nColorIndex);
			if (slot >= 16u)
				continue;

			GLint comps = 0;
			GLenum glType = GL_FLOAT;
			GLboolean normalized = GL_FALSE;
			if (!MapVertexElemTypeToGL(elem.Type, comps, glType, normalized))
				continue;

			GLuint targetVbo = 0;
			uint32_t stride = 0;
			const void* pointer = nullptr;
			uint32_t divisor = 0;

			if (elem.Stream == 0 && streamZeroUserPtr != nullptr)
			{
				targetVbo = 0;
				stride = streamZeroStrideOverride ? streamZeroStrideOverride : pDecl->GetStride(0);
				pointer = static_cast<const uint8_t*>(streamZeroUserPtr) + elem.Offset;
			}
			else if (elem.Stream < CVertexDeclaration::MAX_STREAMS)
			{
				const auto& sb = m_Streams[elem.Stream];
				targetVbo = sb.buffer ? static_cast<GLuint>(reinterpret_cast<uintptr_t>(sb.buffer->GetNativeHandle())) : 0u;
#ifdef USE_USER_POINTER_VBO
				if (targetVbo == 0 && elem.Stream == 0)
					targetVbo = g_userPointerVertexBuffer.GetDevicePointer();
#endif
				if (targetVbo == 0)
					continue;
				stride = sb.stride ? sb.stride : pDecl->GetStride(elem.Stream);
				pointer = reinterpret_cast<const void*>(static_cast<uintptr_t>(sb.offsetInBytes + elem.Offset));
				if (elem.Stream > 0 && sb.frequencyDivider != 0)
				{
					divisor = (sb.frequencyDivider & 0xC0000000u) ? (sb.frequencyDivider & 0x3FFFFFFFu) : sb.frequencyDivider;
				}
				else
				{
					divisor = 0;
				}
			}
			else
			{
				continue;
			}

			if (targetVbo != lastBoundVbo)
			{
				glBindBuffer(GL_ARRAY_BUFFER, targetVbo);
				lastBoundVbo = targetVbo;
			}

			glVertexAttribPointer(slot, comps, glType, normalized, static_cast<GLsizei>(stride), pointer);

#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
			if (glad_glVertexAttribDivisor != nullptr && m_AttribDivisors[slot] != divisor)
			{
				glVertexAttribDivisor(slot, divisor);
				m_AttribDivisors[slot] = divisor;
			}
#endif
		}
	}

	bool RenderDeviceOpenGL::DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void* pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
	{
		GLenum mode = GL_TRIANGLES;
		GLsizei indexCount = 0;
		if (!ToGLPrimitiveModeAndCount(PrimitiveType, PrimitiveCount, mode, indexCount))
			return false;

		GLenum glIndexType = (IndexDataFormat == PixelFormat::INDEX32) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
		uint32_t indexElemBytes = (IndexDataFormat == PixelFormat::INDEX32) ? 4u : 2u;

#ifdef USE_USER_POINTER_VBO
		g_userPointerIndexBuffer.UploadUserPointer(static_cast<int>(indexCount * indexElemBytes), pIndexData);
		g_userPointerVertexBuffer.UploadUserPointer(static_cast<int>(NumVertices * VertexStreamZeroStride), pVertexStreamZeroData, static_cast<int>(MinVertexIndex * VertexStreamZeroStride));
		m_CurrentIndexBuffer = nullptr;
		m_CurrentVertexBuffer = nullptr;
		m_Streams[0].buffer = nullptr;
		m_Streams[0].offsetInBytes = 0;
		m_Streams[0].stride = VertexStreamZeroStride;
		if (m_CurrentVertexDeclaration)
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr, VertexStreamZeroStride);
		pIndexData = nullptr;
#else
		(void)MinVertexIndex;
		(void)NumVertices;
		(void)indexElemBytes;
		if (m_CurrentIndexBuffer != nullptr)
			SetIndices(nullptr);
		if (m_CurrentVertexBuffer != nullptr)
			SetStreamSource(0, nullptr, 0, 0);
		if (m_CurrentVertexDeclaration)
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, pVertexStreamZeroData, VertexStreamZeroStride);
#endif

		ApplyBlendingModeChange();
		ApplyStencilStateChange();
		glDrawElements(mode, indexCount, glIndexType, pIndexData);
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	bool RenderDeviceOpenGL::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
	{
		GLenum mode = GL_TRIANGLES;
		GLsizei vertCount = 0;
		if (!ToGLPrimitiveModeAndCount(PrimitiveType, PrimitiveCount, mode, vertCount))
			return false;

		ApplyBlendingModeChange();
		ApplyStencilStateChange();
		glDrawArrays(mode, static_cast<GLint>(StartVertex), vertCount);
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	bool RenderDeviceOpenGL::DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
	{
		GLenum mode = GL_TRIANGLES;
		GLsizei vertCount = 0;
		if (!ToGLPrimitiveModeAndCount(PrimitiveType, PrimitiveCount, mode, vertCount))
			return false;

		ApplyBlendingModeChange();
		ApplyStencilStateChange();
		int nOffset = 0;
#ifdef USE_USER_POINTER_VBO
		nOffset = g_userPointerVertexBuffer.UploadUserPointerRingBuffer(vertCount, pVertexStreamZeroData, static_cast<int>(VertexStreamZeroStride));
		m_CurrentVertexBuffer = nullptr;
		m_Streams[0].buffer = nullptr;
		m_Streams[0].offsetInBytes = 0;
		m_Streams[0].stride = VertexStreamZeroStride;
		if (m_CurrentVertexDeclaration)
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr, VertexStreamZeroStride);
#else
		if (m_CurrentIndexBuffer != nullptr)
			SetIndices(nullptr);
		if (m_CurrentVertexBuffer != nullptr)
			SetStreamSource(0, nullptr, 0, 0);
		if (m_CurrentVertexDeclaration)
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, pVertexStreamZeroData, VertexStreamZeroStride);
#endif
		glDrawArrays(mode, nOffset, vertCount);
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	bool RenderDeviceOpenGL::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
	{
		m_StateTracker.SetSamplerState(stage, type, value);
		return true;
	}

	bool RenderDeviceOpenGL::SetVertexDeclaration(CVertexDeclaration* pVertexDeclaration)
	{
		m_CurrentVertexDeclaration = pVertexDeclaration;
		if (pVertexDeclaration)
		{
			GL::bindVAO(0);
			ApplyGLVertexDeclaration(pVertexDeclaration, nullptr);
			PE_CHECK_GL_ERROR_DEBUG();
		}
		return true;
	}

	bool RenderDeviceOpenGL::CreateVertexDeclaration(VertexElement* pVertexElements, CVertexDeclaration** ppDecl)
	{
		if (!ppDecl)
			return false;
		*ppDecl = new CVertexDeclaration(pVertexElements);
		return true;
	}

	bool RenderDeviceOpenGL::SetIndices(IndexBufferDevicePtr_type pIndexData)
	{
		if (pIndexData != m_CurrentIndexBuffer)
		{
			m_CurrentIndexBuffer = pIndexData;
			GLuint ibo = pIndexData ? static_cast<GLuint>(reinterpret_cast<uintptr_t>(pIndexData->GetNativeHandle())) : 0u;
#ifdef USE_USER_POINTER_VBO
			if (ibo == 0)
				ibo = g_userPointerIndexBuffer.GetDevicePointer();
#endif
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			PE_CHECK_GL_ERROR_DEBUG();
		}
		return true;
	}

	bool RenderDeviceOpenGL::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
	{
		if (StreamNumber >= CVertexDeclaration::MAX_STREAMS)
			return false;

		auto& sb = m_Streams[StreamNumber];
		const bool changed = (sb.buffer != pStreamData) || (sb.offsetInBytes != OffsetInBytes) || (sb.stride != Stride);
		sb.buffer = pStreamData;
		sb.offsetInBytes = OffsetInBytes;
		sb.stride = Stride;

		if (StreamNumber == 0)
			m_CurrentVertexBuffer = pStreamData;

		if (changed)
		{
			GLuint vbo = pStreamData ? static_cast<GLuint>(reinterpret_cast<uintptr_t>(pStreamData->GetNativeHandle())) : 0u;
#ifdef USE_USER_POINTER_VBO
			if (vbo == 0 && StreamNumber == 0)
				vbo = g_userPointerVertexBuffer.GetDevicePointer();
#endif
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			if (m_CurrentVertexDeclaration && vbo != 0)
			{
				ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr);
			}
			PE_CHECK_GL_ERROR_DEBUG();
		}
		return true;
	}

	void RenderDeviceOpenGL::BeginRenderTarget(uint32_t width, uint32_t height)
	{
		g_stackRenderTargetSizes.push(std::make_pair(m_RenderTargetWidth, m_RenderTargetHeight));
		m_RenderTargetWidth = width;
		m_RenderTargetHeight = height;
		m_isBeginRenderTarget = true;
	}

	void RenderDeviceOpenGL::EndRenderTarget()
	{
		m_isBeginRenderTarget = false;
		if (!g_stackRenderTargetSizes.empty())
		{
			m_RenderTargetWidth = g_stackRenderTargetSizes.top().first;
			m_RenderTargetHeight = g_stackRenderTargetSizes.top().second;
			g_stackRenderTargetSizes.pop();
		}
	}

	bool RenderDeviceOpenGL::BeginScene()
	{
		if (!m_CapsInitialized)
			InitCaps();

		auto* pApp = CGlobals::GetApp();
		auto* pWin = pApp ? pApp->GetRenderWindow() : nullptr;
		if (pWin)
		{
			if (pApp->IsRotateScreen())
			{
				m_BackBufferWidth = pWin->GetHeight();
				m_BackBufferHeight = pWin->GetWidth();
			}
			else
			{
				m_BackBufferWidth = pWin->GetWidth();
				m_BackBufferHeight = pWin->GetHeight();
			}
			m_RenderTargetWidth = m_BackBufferWidth;
			m_RenderTargetHeight = m_BackBufferHeight;
		}

		if (!g_stackRenderTargetSizes.empty())
		{
			std::stack<std::pair<uint32_t, uint32_t>> emptyStack;
			g_stackRenderTargetSizes.swap(emptyStack);
		}
		return true;
	}

	bool RenderDeviceOpenGL::EndScene()
	{
		return true;
	}

	void RenderDeviceOpenGL::Flush()
	{
		glFinish();
	}

	int RenderDeviceOpenGL::GetStencilBits()
	{
		static bool once = true;
		static GLint g_sStencilBits = -1;
		if (once)
		{
			glGetIntegerv(GL_STENCIL_BITS, &g_sStencilBits);
			once = false;
		}
		return g_sStencilBits;
	}

	bool RenderDeviceOpenGL::DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t /*MinIndex*/, uint32_t /*NumVertices*/, uint32_t indexStart, uint32_t PrimitiveCount)
	{
		if ((m_Streams[0].frequencyDivider & 0x40000000u) != 0)
		{
			uint32_t instanceCount = m_Streams[0].frequencyDivider & 0x3FFFFFFFu;
			if (instanceCount > 0)
				return DrawIndexedInstanced(Type, BaseVertexIndex, indexStart, PrimitiveCount, instanceCount);
		}

		GLenum mode = GL_TRIANGLES;
		GLsizei indexCount = 0;
		if (!ToGLPrimitiveModeAndCount(Type, PrimitiveCount, mode, indexCount))
			return false;

		ApplyBlendingModeChange();
		ApplyStencilStateChange();

		const void* indexOffset = reinterpret_cast<const void*>(static_cast<uintptr_t>(sizeof(uint16_t) * indexStart));
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)
		if (BaseVertexIndex != 0 && glad_glDrawElementsBaseVertex != nullptr)
		{
			glDrawElementsBaseVertex(mode, indexCount, GL_UNSIGNED_SHORT, indexOffset, BaseVertexIndex);
			PE_CHECK_GL_ERROR_DEBUG();
			return true;
		}
#endif
		if (BaseVertexIndex != 0 && m_CurrentVertexDeclaration && m_Streams[0].buffer != nullptr)
		{
			uint32_t origOffset = m_Streams[0].offsetInBytes;
			uint32_t stride = m_Streams[0].stride ? m_Streams[0].stride : m_CurrentVertexDeclaration->GetStride(0);
			m_Streams[0].offsetInBytes = static_cast<uint32_t>(static_cast<int32_t>(origOffset) + BaseVertexIndex * static_cast<int32_t>(stride));
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr);
			glDrawElements(mode, indexCount, GL_UNSIGNED_SHORT, indexOffset);
			m_Streams[0].offsetInBytes = origOffset;
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr);
		}
		else
		{
			glDrawElements(mode, indexCount, GL_UNSIGNED_SHORT, indexOffset);
		}

		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	bool RenderDeviceOpenGL::SetTransform(ETransformsStateType /*State*/, DeviceMatrix_ptr /*pMatrix*/)
	{
		return true;
	}

	bool RenderDeviceOpenGL::SetFVF(uint32_t /*FVF*/)
	{
		return true;
	}

	void RenderDeviceOpenGL::SetCursorPosition(int /*X*/, int /*Y*/, uint32_t /*Flags*/)
	{
	}

	bool RenderDeviceOpenGL::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
	{
		if (value)
			*value = m_StateTracker.GetSamplerState(stage, type);
		return true;
	}

	ParaViewport RenderDeviceOpenGL::GetViewport()
	{
		return m_CurrentViewPort;
	}

	bool RenderDeviceOpenGL::SetViewport(const ParaViewport& viewport)
	{
		m_CurrentViewPort = viewport;
		m_StateTracker.SetViewport(viewport);
		if (m_RenderTargetHeight == 0 && viewport.Height > 0)
		{
			m_RenderTargetWidth = viewport.Width;
			m_RenderTargetHeight = viewport.Height;
			m_BackBufferWidth = viewport.Width;
			m_BackBufferHeight = viewport.Height;
		}
		glViewport(
			static_cast<GLint>(viewport.X),
			static_cast<GLint>(m_RenderTargetHeight - viewport.Y - viewport.Height),
			static_cast<GLsizei>(viewport.Width),
			static_cast<GLsizei>(viewport.Height));
		return true;
	}

	bool RenderDeviceOpenGL::Clear(bool color, bool depth, bool stencil)
	{
		uint32_t fields = 0;
		if (depth)
			fields |= GL_DEPTH_BUFFER_BIT;
		if (color)
			fields |= GL_COLOR_BUFFER_BIT;
		if (stencil)
			fields |= GL_STENCIL_BUFFER_BIT;
		glClear(fields);
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	bool RenderDeviceOpenGL::SetScissorRect(RECT* pRect)
	{
		m_StateTracker.SetScissorRect(pRect);
		if (!pRect || (pRect->left <= 0 && pRect->top <= 0 && pRect->right <= 0 && pRect->bottom <= 0))
		{
			glDisable(GL_SCISSOR_TEST);
		}
		else
		{
			int nScreenHeight = static_cast<int>(m_RenderTargetHeight);
			glScissor(
				static_cast<GLint>(pRect->left),
				static_cast<GLint>(nScreenHeight - pRect->bottom),
				static_cast<GLsizei>(pRect->right - pRect->left),
				static_cast<GLsizei>(pRect->bottom - pRect->top));
		}
		return true;
	}

	bool RenderDeviceOpenGL::GetScissorRect(RECT* pRect)
	{
		if (pRect)
			*pRect = m_StateTracker.GetScissorRect();
		return true;
	}

	bool RenderDeviceOpenGL::SetClearColor(const Color4f& color)
	{
		m_StateTracker.SetClearColor(color);
		glClearColor(color.r, color.g, color.b, color.a);
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	bool RenderDeviceOpenGL::SetClearDepth(const float depth)
	{
		m_StateTracker.SetClearDepth(depth);
#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN)
		glClearDepthf(depth);
#else
		glClearDepth(depth);
#endif
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	bool RenderDeviceOpenGL::SetClearStencil(const int stencil)
	{
		m_StateTracker.SetClearStencil(stencil);
		glClearStencil(stencil);
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	void RenderDeviceOpenGL::ApplyBlendingModeChange()
	{
		if (m_EnableSeparateAlphaBlending && (m_AlphaBlendingChanged || m_BlendingChanged))
		{
			uint32_t asrc = ToGLBlendValue(m_BlendingAlphaSource);
			uint32_t adst = ToGLBlendValue(m_BlendingAlphaDest);
			uint32_t src = ToGLBlendValue(m_BlendingSource);
			uint32_t dst = ToGLBlendValue(m_BlendingDest);
			glBlendFuncSeparate(src, dst, asrc, adst);
			m_AlphaBlendingChanged = false;
			m_BlendingChanged = false;
		}
		else if (m_BlendingChanged || m_AlphaBlendingChanged)
		{
			uint32_t src = ToGLBlendValue(m_BlendingSource);
			uint32_t dst = ToGLBlendValue(m_BlendingDest);
			glBlendFunc(src, dst);
			m_BlendingChanged = false;
			m_AlphaBlendingChanged = false;
		}
	}

	void RenderDeviceOpenGL::ApplyStencilStateChange()
	{
		if (!m_StencilDirty)
			return;
		m_StencilDirty = false;
		glStencilFunc(ToGLCompareFunc(m_StencilFunc), static_cast<GLint>(m_StencilRefValue), m_StencilMask);
		glStencilOp(ToGLStencilOp(m_StencilFail), ToGLStencilOp(m_StencilZFail), ToGLStencilOp(m_StencilPass));
	}
}
