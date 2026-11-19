#include <stdexcept>
#include <stack>
#include <algorithm>
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

#ifdef __EMSCRIPTEN__
#include <emscripten/html5_webgl.h>
#define USE_USER_POINTER_VBO
#elif defined(__APPLE__) && !defined(PARAENGINE_MOBILE)
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
				// A wrapped ring (or another indexed UP draw) must not overwrite
				// storage still referenced by queued draws. Orphan it so the GPU
				// can finish using the old storage without stalling this upload.
				if (m_nCurrentSize < nSize || nFrom < m_nNextFreeIndex)
				{
					int nTotalSize = (m_bUseRingBuffer && (m_nMinBufferSize > nSize)) ? m_nMinBufferSize : nSize;
					nTotalSize = (std::max)(nTotalSize, m_nCurrentSize);
					if (nFrom == 0 && nTotalSize == nSize)
					{
						glBufferData(m_nBufferType, nSize, pData, m_bUseRingBuffer ? GL_STREAM_DRAW : GL_DYNAMIC_DRAW);
					}
					else
					{
						glBufferData(m_nBufferType, nTotalSize, nullptr, m_bUseRingBuffer ? GL_STREAM_DRAW : GL_DYNAMIC_DRAW);
						UploadRange(nFrom, nCountBytes, pData);
					}
					m_nCurrentSize = nTotalSize;
				}
				else
				{
					UploadRange(nFrom, nCountBytes, pData);
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
			void UploadRange(int offset, int size, const void* data)
			{
#if defined(__APPLE__) && !defined(PARAENGINE_MOBILE)
				// Apple's GL driver synchronizes glBufferSubData against the whole
				// resource, even for unused ranges. Appends are disjoint and wraps
				// orphan the storage above, so an unsynchronized map is safe here.
				void* mapped = glMapBufferRange(m_nBufferType, offset, size,
					GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
				if (mapped)
				{
					std::memcpy(mapped, data, size);
					glUnmapBuffer(m_nBufferType);
					return;
				}
#endif
				glBufferSubData(m_nBufferType, offset, size, data);
			}

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

		inline bool HasGLDrawBuffers(uint32_t featureLevel)
		{
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__)
			return featureLevel >= 300;
#elif defined(__APPLE__)
			return featureLevel >= 300;
#else
			(void)featureLevel;
			return glad_glDrawBuffers != nullptr;
#endif
		}

		inline bool HasGLBlitFramebuffer(uint32_t featureLevel)
		{
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__)
			return featureLevel >= 300;
#elif defined(__APPLE__)
			return featureLevel >= 300;
#else
			(void)featureLevel;
			return glad_glBlitFramebuffer != nullptr;
#endif
		}

		inline bool HasGLInstancing(uint32_t featureLevel)
		{
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__)
			return featureLevel >= 300;
#elif defined(__APPLE__)
			return featureLevel >= 330;
#else
			(void)featureLevel;
			return glad_glDrawElementsInstanced != nullptr && glad_glVertexAttribDivisor != nullptr;
#endif
		}

		inline bool HasGLTwoSidedStencil()
		{
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__) || defined(__APPLE__)
			return true;
#else
			return glad_glStencilFuncSeparate != nullptr && glad_glStencilOpSeparate != nullptr;
#endif
		}

		bool MapPixelFormatToGL(PixelFormat fmt, uint32_t featureLevel, GLenum& outInternal, GLenum& outFormat, GLenum& outType, uint32_t& outBpp)
		{
			switch (fmt)
			{
			case PixelFormat::A8R8G8B8:
			case PixelFormat::A8B8G8R8:
			case PixelFormat::X8R8G8B8:
			case PixelFormat::X8B8G8R8:
				outInternal = (featureLevel >= 300) ? GL_RGBA8 : GL_RGBA;
				outFormat = GL_RGBA;
				outType = GL_UNSIGNED_BYTE;
				outBpp = 32;
				return true;
			case PixelFormat::R8G8B8:
				outInternal = (featureLevel >= 300) ? GL_RGB8 : GL_RGB;
				outFormat = GL_RGB;
				outType = GL_UNSIGNED_BYTE;
				outBpp = 24;
				return true;
			case PixelFormat::A8:
#if defined(__APPLE__) && !defined(PARAENGINE_MOBILE)
				outInternal = GL_R8;
				outFormat = GL_RED;
#else
				outInternal = GL_ALPHA;
				outFormat = GL_ALPHA;
#endif
				outType = GL_UNSIGNED_BYTE;
				outBpp = 8;
				return true;
			case PixelFormat::L8:
#if defined(__APPLE__) && !defined(PARAENGINE_MOBILE)
				outInternal = GL_R8;
				outFormat = GL_RED;
#else
				outInternal = GL_LUMINANCE;
				outFormat = GL_LUMINANCE;
#endif
				outType = GL_UNSIGNED_BYTE;
				outBpp = 8;
				return true;
			case PixelFormat::R16F:
				if (featureLevel < 300) return false;
				outInternal = GL_R16F;
				outFormat = GL_RED;
				outType = GL_HALF_FLOAT;
				outBpp = 16;
				return true;
			case PixelFormat::R32F:
				if (featureLevel < 300) return false;
				outInternal = GL_R32F;
				outFormat = GL_RED;
				outType = GL_FLOAT;
				outBpp = 32;
				return true;
			case PixelFormat::G16R16F:
				if (featureLevel < 300) return false;
				outInternal = GL_RG16F;
				outFormat = GL_RG;
				outType = GL_HALF_FLOAT;
				outBpp = 32;
				return true;
			case PixelFormat::G32R32F:
				if (featureLevel < 300) return false;
				outInternal = GL_RG32F;
				outFormat = GL_RG;
				outType = GL_FLOAT;
				outBpp = 64;
				return true;
			case PixelFormat::A16B16G16R16F:
				if (featureLevel < 300) return false;
				outInternal = GL_RGBA16F;
				outFormat = GL_RGBA;
				outType = GL_HALF_FLOAT;
				outBpp = 64;
				return true;
			case PixelFormat::A32B32G32R32F:
				if (featureLevel < 300) return false;
				outInternal = GL_RGBA32F;
				outFormat = GL_RGBA;
				outType = GL_FLOAT;
				outBpp = 128;
				return true;
			case PixelFormat::D24S8:
			case PixelFormat::D24FS8:
				if (featureLevel < 300) return false;
				outInternal = GL_DEPTH24_STENCIL8;
				outFormat = GL_DEPTH_STENCIL;
				outType = GL_UNSIGNED_INT_24_8;
				outBpp = 32;
				return true;
			case PixelFormat::D32:
			case PixelFormat::D32F_LOCKABLE:
				if (featureLevel < 300) return false;
				outInternal = GL_DEPTH_COMPONENT32F;
				outFormat = GL_DEPTH_COMPONENT;
				outType = GL_FLOAT;
				outBpp = 32;
				return true;
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
			GLRenderTarget(RenderDeviceOpenGL* owner, uint32_t width, uint32_t height, PixelFormat colorFormat, PixelFormat depthFormat, uint32_t colorTargetCount)
				: m_owner(owner)
				, m_selfId(++s_nextRtSerial)
				, m_attachedDepthOwnerId(0)
				, m_fbo(0)
				, m_depthRbo(0)
				, m_width(width)
				, m_height(height)
				, m_colorFormat(colorFormat)
				, m_depthTexture(nullptr)
				, m_depthHasStencil(false)
				, m_valid(false)
			{
				const uint32_t fl = owner ? owner->GetCaps().featureLevel : 200u;
				const bool supportsFloatCB = owner ? owner->SupportsFloatColorBuffer() : false;
				const bool supportsFloatLinear = owner ? owner->SupportsFloatLinearFilter() : false;

				// On GLES 3.0 / WebGL 2.0, if 32-bit float color rendering or linear filtering is unavailable,
				// transparently promote 32-bit float RTs to 16-bit half-float (which is core linear-filterable in ES 3.0).
				PixelFormat effectiveColorFormat = colorFormat;
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__)
				if (!supportsFloatCB || !supportsFloatLinear)
				{
					if (effectiveColorFormat == PixelFormat::R32F)
						effectiveColorFormat = PixelFormat::R16F;
					else if (effectiveColorFormat == PixelFormat::G32R32F)
						effectiveColorFormat = PixelFormat::G16R16F;
					else if (effectiveColorFormat == PixelFormat::A32B32G32R32F)
						effectiveColorFormat = PixelFormat::A16B16G16R16F;
				}
#endif
				m_colorFormat = effectiveColorFormat;

				GLint oldFbo = 0;
				glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFbo);

				glGenFramebuffers(1, &m_fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

				uint32_t rtCount = colorTargetCount;
				if (rtCount > 4)
					rtCount = 4;

				for (uint32_t i = 0; i < rtCount; ++i)
				{
					GLenum internalFmt = (fl >= 300) ? GL_RGBA8 : GL_RGBA;
					GLenum fmt = GL_RGBA;
					GLenum type = GL_UNSIGNED_BYTE;
					uint32_t bpp = 32;
					if (!MapPixelFormatToGL(effectiveColorFormat, fl, internalFmt, fmt, type, bpp))
					{
						internalFmt = (fl >= 300) ? GL_RGBA8 : GL_RGBA;
						fmt = GL_RGBA;
						type = GL_UNSIGNED_BYTE;
					}

					const bool is32BitFloatTex = (type == GL_FLOAT);
					const GLint filterMode = (is32BitFloatTex && !supportsFloatLinear) ? GL_NEAREST : GL_LINEAR;

					GLuint texId = 0;
					glGenTextures(1, &texId);
					GL::bindTexture2D(texId);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, fmt, type, nullptr);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texId, 0);

					m_colorTextures.push_back(new GLRawTexture(texId, width, height, 1, effectiveColorFormat, fmt, type));
				}

				if (rtCount > 1 && HasGLDrawBuffers(fl))
				{
					GLenum drawBufs[4] = {
						GL_COLOR_ATTACHMENT0,
						GL_COLOR_ATTACHMENT1,
						GL_COLOR_ATTACHMENT2,
						GL_COLOR_ATTACHMENT3
					};
					glDrawBuffers(static_cast<GLsizei>(rtCount), drawBufs);
				}
				else if (rtCount == 0 && HasGLDrawBuffers(fl))
				{
					const GLenum noneBuf = GL_NONE;
					glDrawBuffers(1, &noneBuf);
#if !defined(PARAENGINE_MOBILE) && !defined(__EMSCRIPTEN__)
					if (glad_glReadBuffer != nullptr)
						glReadBuffer(GL_NONE);
#else
					if (fl >= 300)
						glReadBuffer(GL_NONE);
#endif
				}

				if (depthFormat != PixelFormat::Unknown)
				{
					if (fl >= 300 && (depthFormat == PixelFormat::D32 || depthFormat == PixelFormat::D32F_LOCKABLE || rtCount == 0))
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
						m_depthHasStencil = false;
					}
					else
					{
						glGenRenderbuffers(1, &m_depthRbo);
						glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
						if (depthFormat == PixelFormat::D16 || depthFormat == PixelFormat::D16_LOCKABLE)
						{
							glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
							glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);
							m_depthHasStencil = false;
						}
						else
						{
							glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_DEFAULT, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
#ifdef GL_DEPTH_STENCIL_ATTACHMENT
							if (fl >= 300)
							{
								glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);
							}
							else
#endif
							{
								glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);
								glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);
							}
							m_depthHasStencil = true;
						}
						glBindRenderbuffer(GL_RENDERBUFFER, 0);
					}
				}
				m_attachedDepthOwnerId = m_selfId;

				m_valid = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
				while (glGetError() != GL_NO_ERROR) {}
				glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(oldFbo));
			}

			~GLRenderTarget() override
			{
				if (m_owner)
				{
					m_owner->NotifyRenderTargetDestroyed(this);
				}
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

			bool HasDepthAttachment() const { return m_depthTexture != nullptr || m_depthRbo != 0; }

			// Bind either dsOverride's depth/stencil (when sharing scene depth across HDR postfx RTs) or restore this RT's own depth/stencil.
			void ApplyDepthStencilAttachment(const GLRenderTarget* dsOverride, uint32_t fl)
			{
				const GLRenderTarget* effective = (dsOverride != nullptr && dsOverride->m_width == m_width && dsOverride->m_height == m_height && dsOverride->HasDepthAttachment())
					? dsOverride
					: this;
				if (m_attachedDepthOwnerId == effective->m_selfId)
					return;

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
#ifdef GL_DEPTH_STENCIL_ATTACHMENT
				if (fl >= 300)
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
				}
				else
#endif
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
				}

				if (effective->m_depthTexture != nullptr)
				{
					const GLuint dTex = static_cast<GLuint>(reinterpret_cast<uintptr_t>(effective->m_depthTexture->GetNativeHandle()));
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dTex, 0);
				}
				else if (effective->m_depthRbo != 0)
				{
					if (!effective->m_depthHasStencil)
					{
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, effective->m_depthRbo);
					}
					else
					{
#ifdef GL_DEPTH_STENCIL_ATTACHMENT
						if (fl >= 300)
						{
							glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, effective->m_depthRbo);
						}
						else
#endif
						{
							glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, effective->m_depthRbo);
							glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, effective->m_depthRbo);
						}
					}
				}
				m_attachedDepthOwnerId = effective->m_selfId;
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
			static inline uint64_t s_nextRtSerial = 0;
			RenderDeviceOpenGL* m_owner = nullptr;
			uint64_t m_selfId;
			uint64_t m_attachedDepthOwnerId;
			GLuint m_fbo;
			GLuint m_depthRbo;
			uint32_t m_width;
			uint32_t m_height;
			PixelFormat m_colorFormat;
			std::vector<IRHITexture*> m_colorTextures;
			IRHITexture* m_depthTexture;
			bool m_depthHasStencil;
			bool m_valid;
		};

#if !defined(PARAENGINE_MOBILE) && !defined(__EMSCRIPTEN__)
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
		, m_TwoSidedStencil(false)
		, m_StencilRefValue(0)
		, m_StencilMask(0xFFFFFFFFu)
		, m_StencilFunc(RSV_CMP_ALWAYS)
		, m_StencilFail(RSV_STENCILOP_KEEP)
		, m_StencilZFail(RSV_STENCILOP_KEEP)
		, m_StencilPass(RSV_STENCILOP_KEEP)
		, m_CCWStencilFunc(RSV_CMP_ALWAYS)
		, m_CCWStencilFail(RSV_STENCILOP_KEEP)
		, m_CCWStencilZFail(RSV_STENCILOP_KEEP)
		, m_CCWStencilPass(RSV_STENCILOP_KEEP)
		, m_StencilDirty(false)
		, m_CurrentVertexDeclaration(nullptr)
		, m_CurrentVertexBuffer(nullptr)
		, m_CurrentIndexBuffer(nullptr)
	{
	}

	RenderDeviceOpenGL::~RenderDeviceOpenGL()
	{
		m_isShuttingDown = true;
		if (m_BlitProgram != nullptr)
		{
			m_BlitProgram->Release();
			m_BlitProgram = nullptr;
		}
		GL::ClearCache();
	}

	void RenderDeviceOpenGL::NotifyRenderTargetDestroyed(IRHIRenderTarget* rt)
	{
		if (m_isShuttingDown || !rt)
			return;

		bool changed = false;
		IRHIRenderTarget* newTargets[4] = {
			m_currentRTBinding.targets[0],
			m_currentRTBinding.targets[1],
			m_currentRTBinding.targets[2],
			m_currentRTBinding.targets[3]
		};
		for (uint32_t i = 0; i < 4; ++i)
		{
			if (newTargets[i] == rt)
			{
				newTargets[i] = nullptr;
				changed = true;
			}
		}
		IRHIRenderTarget* dsOverride = (m_currentRTBinding.depthStencilOverride == rt)
			? nullptr
			: m_currentRTBinding.depthStencilOverride;
		if (dsOverride != m_currentRTBinding.depthStencilOverride)
			changed = true;
		if (changed)
		{
			uint32_t newCount = 0;
			for (uint32_t i = 0; i < 4; ++i)
			{
				if (newTargets[i] != nullptr)
					newCount = i + 1;
			}
			SetRenderTargets(newCount, newCount > 0 ? newTargets : nullptr, dsOverride);
		}
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
			const char* esPos = std::strstr(s, "OpenGL ES");
			const char* webglPos = std::strstr(s, "WebGL");
			if (esPos)
			{
				// Handles "OpenGL ES 3.2 ..." and "WebGL 2.0 (OpenGL ES 3.0 Chromium)"
				const char* p = esPos + 9;
				while (*p && (*p < '0' || *p > '9'))
					++p;
				if (*p)
					std::sscanf(p, "%d.%d", &major, &minor);
			}
			else if (webglPos)
			{
				// Handles bare "WebGL 2.0" / "WebGL 1.0" strings (e.g. Firefox)
				int wglMajor = 1;
				int wglMinor = 0;
				const char* p = webglPos + 5;
				while (*p && (*p < '0' || *p > '9'))
					++p;
				if (*p && std::sscanf(p, "%d.%d", &wglMajor, &wglMinor) >= 1)
				{
					major = (wglMajor >= 2) ? 3 : 2;
					minor = 0;
				}
			}
			else
			{
				while (*s && (*s < '0' || *s > '9'))
					++s;
				if (*s)
					std::sscanf(s, "%d.%d", &major, &minor);
			}
		}
#ifdef __EMSCRIPTEN__
		{
			EMSCRIPTEN_WEBGL_CONTEXT_HANDLE wglCtx = emscripten_webgl_get_current_context();
			EmscriptenWebGLContextAttributes wglAttrs;
			if (wglCtx > 0 && emscripten_webgl_get_context_attributes(wglCtx, &wglAttrs) == EMSCRIPTEN_RESULT_SUCCESS)
			{
				if (wglAttrs.majorVersion >= 2 && major < 3)
				{
					major = 3;
					minor = 0;
				}
			}
		}
#endif
		m_Caps.featureLevel = static_cast<uint32_t>(major * 100 + minor * 10);

		GLint maxTexUnits = 8;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);
		m_Caps.maxTextureStages = static_cast<uint32_t>(maxTexUnits > 0 ? maxTexUnits : 8);

		GLint maxTexSize = 4096;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
		m_Caps.maxTextureSize = static_cast<uint32_t>(maxTexSize > 0 ? maxTexSize : 4096);

#if !defined(PARAENGINE_MOBILE) && !defined(__EMSCRIPTEN__)
		GLint maxDrawBufs = 1;
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBufs);
		m_Caps.maxSimultaneousRTs = static_cast<uint32_t>(maxDrawBufs > 0 ? maxDrawBufs : 1);
		GLint maxVsComps = 1024;
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVsComps);
		m_Caps.maxVertexUniformVectors = static_cast<uint32_t>(maxVsComps > 0 ? (maxVsComps / 4) : 256);
		m_SupportsFloatColorBuffer = (m_Caps.featureLevel >= 300);
		m_SupportsHalfFloatColorBuffer = (m_Caps.featureLevel >= 300);
		m_SupportsFloatLinearFilter = (m_Caps.featureLevel >= 300);
		m_Caps.supportsFloatRenderTargets = (m_Caps.featureLevel >= 300);
		m_Caps.supportsDepthTextures = (m_Caps.featureLevel >= 300);
		m_Caps.supportsInstancing = HasGLInstancing(m_Caps.featureLevel);
		m_Caps.supportsOcclusionQuery = (glad_glGenQueries != nullptr && glad_glGetQueryObjectuiv != nullptr);
		m_Caps.supportsTwoSidedStencil = HasGLTwoSidedStencil();
		m_Caps.supportsUserClipPlanes = true;
		m_Caps.supports32BitIndices = true;
#else
#ifdef __EMSCRIPTEN__
		{
			EMSCRIPTEN_WEBGL_CONTEXT_HANDLE wglCtx = emscripten_webgl_get_current_context();
			if (wglCtx > 0)
			{
				if (emscripten_webgl_enable_extension(wglCtx, "EXT_color_buffer_float"))
					m_SupportsFloatColorBuffer = true;
				if (emscripten_webgl_enable_extension(wglCtx, "EXT_color_buffer_half_float"))
					m_SupportsHalfFloatColorBuffer = true;
				if (emscripten_webgl_enable_extension(wglCtx, "OES_texture_float_linear"))
					m_SupportsFloatLinearFilter = true;
				emscripten_webgl_enable_extension(wglCtx, "OES_texture_half_float_linear");
				if (emscripten_webgl_enable_extension(wglCtx, "WEBGL_compressed_texture_s3tc") ||
					emscripten_webgl_enable_extension(wglCtx, "WEBKIT_WEBGL_compressed_texture_s3tc") ||
					emscripten_webgl_enable_extension(wglCtx, "MOZ_WEBGL_compressed_texture_s3tc"))
				{
					m_Caps.supportsBCCompression = true;
				}
			}
		}
#endif
		auto hasGLExt = [&](const char* extName) -> bool
		{
			if (!extName || !extName[0])
				return false;
			if (m_Caps.featureLevel >= 300)
			{
				GLint numExts = 0;
				glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
				for (GLint i = 0; i < numExts; ++i)
				{
					const GLubyte* e = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i));
					if (e && std::strcmp(reinterpret_cast<const char*>(e), extName) == 0)
						return true;
				}
				return false;
			}
			const GLubyte* extStr = glGetString(GL_EXTENSIONS);
			if (extStr)
			{
				const char* all = reinterpret_cast<const char*>(extStr);
				const size_t len = std::strlen(extName);
				const char* p = all;
				while ((p = std::strstr(p, extName)) != nullptr)
				{
					const bool startOk = (p == all || p[-1] == ' ');
					const bool endOk = (p[len] == '\0' || p[len] == ' ');
					if (startOk && endOk)
						return true;
					p += len;
				}
			}
			return false;
		};

		if (m_Caps.featureLevel >= 320 || hasGLExt("GL_EXT_color_buffer_float") || hasGLExt("EXT_color_buffer_float"))
			m_SupportsFloatColorBuffer = true;
		if (m_SupportsFloatColorBuffer || hasGLExt("GL_EXT_color_buffer_half_float") || hasGLExt("EXT_color_buffer_half_float"))
			m_SupportsHalfFloatColorBuffer = true;
		if (hasGLExt("GL_OES_texture_float_linear") || hasGLExt("OES_texture_float_linear"))
			m_SupportsFloatLinearFilter = true;

		if (m_Caps.featureLevel >= 300)
		{
			GLint maxDrawBufs = 4;
			glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBufs);
			m_Caps.maxSimultaneousRTs = static_cast<uint32_t>(maxDrawBufs > 0 ? maxDrawBufs : 1);
			m_Caps.supportsFloatRenderTargets = (m_SupportsFloatColorBuffer || m_SupportsHalfFloatColorBuffer);
			m_Caps.supportsDepthTextures = true;
			m_Caps.supportsInstancing = true;
		}
		else
		{
			m_Caps.maxSimultaneousRTs = 1;
			m_Caps.supportsFloatRenderTargets = false;
			m_Caps.supportsDepthTextures = false;
			m_Caps.supportsInstancing = false;
		}
		GLint maxVsVecs = 256;
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVsVecs);
		m_Caps.maxVertexUniformVectors = static_cast<uint32_t>(maxVsVecs > 0 ? maxVsVecs : 256);
		m_Caps.supportsTwoSidedStencil = true;
		m_Caps.supports32BitIndices = (m_Caps.featureLevel >= 300 || hasGLExt("GL_OES_element_index_uint") || hasGLExt("OES_element_index_uint"));
#endif
		{
			std::string lowerAdapter = m_AdapterName;
			std::transform(lowerAdapter.begin(), lowerAdapter.end(), lowerAdapter.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			m_Caps.isSoftwareAdapter =
				(lowerAdapter.find("gdi generic") != std::string::npos) ||
				(lowerAdapter.find("llvmpipe") != std::string::npos) ||
				(lowerAdapter.find("softpipe") != std::string::npos) ||
				(lowerAdapter.find("swiftshader") != std::string::npos) ||
				(lowerAdapter.find("software") != std::string::npos);
		}

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
		const uint32_t fl = GetCaps().featureLevel;
		if (!MapPixelFormatToGL(format, fl, internalFmt, glFmt, glType, bpp))
			return nullptr;

		GLuint texId = 0;
		glGenTextures(1, &texId);
		GL::bindTexture2D(texId);
		uint32_t numMips = (mipLevels > 0) ? mipLevels : 1;
		const bool nonFilterableFloat = (glType == GL_FLOAT && !SupportsFloatLinearFilter());
		const GLint minFilter = nonFilterableFloat ? GL_NEAREST : ((numMips > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		const GLint magFilter = nonFilterableFloat ? GL_NEAREST : GL_LINEAR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(__APPLE__) && !defined(PARAENGINE_MOBILE)
		if (format == PixelFormat::A8)
		{
			const GLint swizzle[4] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
		}
		else if (format == PixelFormat::L8)
		{
			const GLint swizzle[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
		}
#endif
		if (bpp == 8 || bpp == 16 || bpp == 24)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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
		auto* rt = new GLRenderTarget(this, width, height, colorFormat, depthFormat, colorTargetCount);
		if (!rt->IsValid())
		{
			rt->ReleaseRHIResource();
			return nullptr;
		}
		return rt;
	}

	IRHIOcclusionQuery* RenderDeviceOpenGL::CreateOcclusionQuery()
	{
#if !defined(PARAENGINE_MOBILE) && !defined(__EMSCRIPTEN__)
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
			m_BoundMrtCount = 0;
			m_ValidMrtMask = 0;
			m_AppliedDrawBufferMask = 0;
			NotifyRenderTargetsBound(0, nullptr, depthStencilOverride);
			return true;
		}

		auto* primaryRT = static_cast<GLRenderTarget*>(targets[0]);
		glBindFramebuffer(GL_FRAMEBUFFER, primaryRT->GetFBO());
		m_BoundMrtCount = 1;
		m_ValidMrtMask = 0x1u;
		m_AppliedDrawBufferMask = 0x1u;
		const uint32_t fl = GetCaps().featureLevel;
		primaryRT->ApplyDepthStencilAttachment(static_cast<const GLRenderTarget*>(depthStencilOverride), fl);
		if (count > 1 && HasGLDrawBuffers(fl))
		{
			uint32_t rtCount = count > 4u ? 4u : count;
			GLenum drawBufs[4] = { GL_NONE, GL_NONE, GL_NONE, GL_NONE };
			uint8_t validMask = 0;
			for (uint32_t i = 0; i < rtCount; ++i)
			{
				auto* rt = static_cast<GLRenderTarget*>(targets[i]);
				IRHITexture* colorTex = rt ? rt->GetColorTexture(0) : nullptr;
				GLuint texId = colorTex ? static_cast<GLuint>(reinterpret_cast<uintptr_t>(colorTex->GetNativeHandle())) : 0;
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texId, 0);
				if (texId != 0)
				{
					drawBufs[i] = GL_COLOR_ATTACHMENT0 + i;
					validMask |= static_cast<uint8_t>(1u << i);
				}
			}
			for (uint32_t i = rtCount; i < 4; ++i)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
			}
			glDrawBuffers(static_cast<GLsizei>(rtCount), drawBufs);
			m_BoundMrtCount = rtCount;
			m_ValidMrtMask = validMask;
			m_AppliedDrawBufferMask = validMask;
		}
		else if (count == 1 && primaryRT->GetColorCount() == 1 && HasGLDrawBuffers(fl))
		{
			IRHITexture* colorTex0 = primaryRT->GetColorTexture(0);
			GLuint texId0 = colorTex0 ? static_cast<GLuint>(reinterpret_cast<uintptr_t>(colorTex0->GetNativeHandle())) : 0;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId0, 0);
			for (uint32_t i = 1; i < 4; ++i)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
			const GLenum att0 = GL_COLOR_ATTACHMENT0;
			glDrawBuffers(1, &att0);
		}
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
		while (glGetError() != GL_NO_ERROR) {}

		GLuint srcFbo = src ? static_cast<GLRenderTarget*>(src)->GetFBO() : m_DefaultFBO;
		GLuint dstFbo = dst ? static_cast<GLRenderTarget*>(dst)->GetFBO() : m_DefaultFBO;
		uint32_t srcW = src ? src->GetWidth() : GetBackBufferWidth();
		uint32_t srcH = src ? src->GetHeight() : GetBackBufferHeight();
		uint32_t dstW = dst ? dst->GetWidth() : GetBackBufferWidth();
		uint32_t dstH = dst ? dst->GetHeight() : GetBackBufferHeight();
		if (srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));
			return false;
		}

		GLint sx0 = srcRect ? srcRect->left : 0;
		GLint sy0 = srcRect ? static_cast<GLint>(srcH) - srcRect->bottom : 0;
		GLint sx1 = srcRect ? srcRect->right : static_cast<GLint>(srcW);
		GLint sy1 = srcRect ? static_cast<GLint>(srcH) - srcRect->top : static_cast<GLint>(srcH);

		GLint dx0 = dstRect ? dstRect->left : 0;
		GLint dy0 = dstRect ? static_cast<GLint>(dstH) - dstRect->bottom : 0;
		GLint dx1 = dstRect ? dstRect->right : static_cast<GLint>(dstW);
		GLint dy1 = dstRect ? (dstRect->top == dstRect->bottom ? 0 : (static_cast<GLint>(dstH) - dstRect->top)) : static_cast<GLint>(dstH);

		if (HasGLBlitFramebuffer(GetCaps().featureLevel))
		{
			bool canHardwareBlit = true;
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__)
			if (dst == nullptr && dstFbo == 0)
			{
				// OpenGL ES 3.0 / WebGL 2.0 forbids glBlitFramebuffer into a multisampled draw FBO.
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);
				GLint dstSampleBuffers = 0;
				glGetIntegerv(GL_SAMPLE_BUFFERS, &dstSampleBuffers);
				if (dstSampleBuffers > 0)
					canHardwareBlit = false;
			}
			if (canHardwareBlit && src == nullptr && srcFbo == 0 && m_DefaultFboReadBlitSupported == 0)
			{
				canHardwareBlit = false;
			}
#endif
			if (canHardwareBlit)
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFbo);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);
				glBlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, GL_COLOR_BUFFER_BIT, linearFilter ? GL_LINEAR : GL_NEAREST);
				const GLenum blitErr = glGetError();
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__)
				if (src == nullptr && srcFbo == 0)
					m_DefaultFboReadBlitSupported = (blitErr == GL_NO_ERROR) ? 1 : 0;
#endif
				if (blitErr == GL_NO_ERROR)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));
					return true;
				}
			}
		}

		// Textured fullscreen/subrect quad fallback when blitting from an offscreen RT (e.g. into a multisampled WebGL2 default FBO 0)
		IRHITexture* srcTex = src ? src->GetColorTexture(0) : nullptr;
		if (srcTex != nullptr && dx1 > dx0 && dy1 > dy0)
		{
			GLuint srcTexId = static_cast<GLuint>(reinterpret_cast<uintptr_t>(srcTex->GetNativeHandle()));
			if (srcTexId != 0)
			{
				GLint prevProgram = 0;
				glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);

				if (m_BlitProgram == nullptr)
				{
					static const char* kBlitVS =
						"attribute vec4 a_position;\n"
						"attribute vec2 a_texCoord;\n"
						"varying vec2 v_texCoord;\n"
						"void main()\n"
						"{\n"
						"    gl_Position = a_position;\n"
						"    v_texCoord = a_texCoord;\n"
						"}\n";
					static const char* kBlitFS =
						"uniform sampler2D tex0Sampler;\n"
						"varying vec2 v_texCoord;\n"
						"void main()\n"
						"{\n"
						"    gl_FragColor = texture2D(tex0Sampler, v_texCoord);\n"
						"}\n";
					GLProgram* prog = new GLProgram();
					if (prog->initWithByteArrays(kBlitVS, kBlitFS) && prog->link())
					{
						prog->updateUniforms();
						m_BlitProgram = prog;
					}
					else
					{
						prog->Release();
						GL::useProgram(static_cast<GLuint>(prevProgram));
					}
				}

				if (m_BlitProgram != nullptr)
				{
					GLint prevArrayBuf = 0;
					GLint prevViewport[4] = { 0, 0, 0, 0 };
					GLboolean prevColorMask[4] = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
					GLboolean prevDepthMask = GL_TRUE;
					glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevArrayBuf);
					glGetIntegerv(GL_VIEWPORT, prevViewport);
					glGetBooleanv(GL_COLOR_WRITEMASK, prevColorMask);
					glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthMask);
					const GLboolean wasDepthTest = glIsEnabled(GL_DEPTH_TEST);
					const GLboolean wasStencilTest = glIsEnabled(GL_STENCIL_TEST);
					const GLboolean wasBlend = glIsEnabled(GL_BLEND);
					const GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
					const GLboolean wasScissor = glIsEnabled(GL_SCISSOR_TEST);

					glBindFramebuffer(GL_FRAMEBUFFER, dstFbo);
					if (dst != nullptr && HasGLDrawBuffers(GetCaps().featureLevel))
					{
						const GLenum att0 = GL_COLOR_ATTACHMENT0;
						glDrawBuffers(1, &att0);
					}

					if (wasDepthTest) glDisable(GL_DEPTH_TEST);
					if (wasStencilTest) glDisable(GL_STENCIL_TEST);
					if (wasBlend) glDisable(GL_BLEND);
					if (wasCull) glDisable(GL_CULL_FACE);
					if (wasScissor) glDisable(GL_SCISSOR_TEST);
					if (prevDepthMask) glDepthMask(GL_FALSE);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glViewport(dx0, dy0, dx1 - dx0, dy1 - dy0);

					m_BlitProgram->use();
					const GLint samplerLoc = m_BlitProgram->getUniformLocationForName("tex0Sampler");
					if (samplerLoc >= 0)
						glUniform1i(samplerLoc, 0);

					GL::bindTexture2DN(0, srcTexId);
					const PixelFormat srcFmt = srcTex->GetFormat();
					const bool is32BitFloat = (srcFmt == PixelFormat::R32F || srcFmt == PixelFormat::G32R32F || srcFmt == PixelFormat::A32B32G32R32F);
					const GLint desiredFilter = (linearFilter && (!is32BitFloat || SupportsFloatLinearFilter())) ? GL_LINEAR : GL_NEAREST;
					GLint prevMinFilter = GL_LINEAR;
					GLint prevMagFilter = GL_LINEAR;
					glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &prevMinFilter);
					glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &prevMagFilter);
					if (prevMinFilter != desiredFilter)
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, desiredFilter);
					if (prevMagFilter != desiredFilter)
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, desiredFilter);

					const float invSrcW = 1.0f / static_cast<float>(srcW);
					const float invSrcH = 1.0f / static_cast<float>(srcH);
					const float u0 = static_cast<float>(sx0) * invSrcW;
					const float v0 = static_cast<float>(sy0) * invSrcH;
					const float u1 = static_cast<float>(sx1) * invSrcW;
					const float v1 = static_cast<float>(sy1) * invSrcH;
					const float quadVerts[16] = {
						-1.0f, -1.0f, u0, v0,
						 1.0f, -1.0f, u1, v0,
						-1.0f,  1.0f, u0, v1,
						 1.0f,  1.0f, u1, v1,
					};

					const int vertOffset = g_userPointerVertexBuffer.UploadUserPointerRingBuffer(4, quadVerts, 4 * static_cast<int>(sizeof(float)));
					GL::bindVAO(0);
					glBindBuffer(GL_ARRAY_BUFFER, g_userPointerVertexBuffer.GetDevicePointer());
					GL::enableVertexAttribs((1u << GLProgram::VERTEX_ATTRIB_POSITION) | (1u << GLProgram::VERTEX_ATTRIB_TEX_COORD));
					if (HasGLInstancing(GetCaps().featureLevel))
					{
						if (m_AttribDivisors[GLProgram::VERTEX_ATTRIB_POSITION] != 0)
						{
							glVertexAttribDivisor(GLProgram::VERTEX_ATTRIB_POSITION, 0);
							m_AttribDivisors[GLProgram::VERTEX_ATTRIB_POSITION] = 0;
						}
						if (m_AttribDivisors[GLProgram::VERTEX_ATTRIB_TEX_COORD] != 0)
						{
							glVertexAttribDivisor(GLProgram::VERTEX_ATTRIB_TEX_COORD, 0);
							m_AttribDivisors[GLProgram::VERTEX_ATTRIB_TEX_COORD] = 0;
						}
					}
					glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<const void*>(0));
					glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<const void*>(2 * sizeof(float)));
					glDrawArrays(GL_TRIANGLE_STRIP, vertOffset, 4);

					if (prevMinFilter != desiredFilter)
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, prevMinFilter);
					if (prevMagFilter != desiredFilter)
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, prevMagFilter);

					glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
					glColorMask(prevColorMask[0], prevColorMask[1], prevColorMask[2], prevColorMask[3]);
					if (prevDepthMask) glDepthMask(GL_TRUE);
					if (wasDepthTest) glEnable(GL_DEPTH_TEST);
					if (wasStencilTest) glEnable(GL_STENCIL_TEST);
					if (wasBlend) glEnable(GL_BLEND);
					if (wasCull) glEnable(GL_CULL_FACE);
					if (wasScissor) glEnable(GL_SCISSOR_TEST);

					GL::useProgram(static_cast<GLuint>(prevProgram));
					glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(prevArrayBuf));
					if (m_CurrentVertexDeclaration)
						ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr);

					const bool ok = (glGetError() == GL_NO_ERROR);
					glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));
					return ok;
				}
			}
		}

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
				const bool copyOk = (glGetError() == GL_NO_ERROR);
				glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));
				return copyOk;
			}
		}

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
		if (instanceCount == 0 || !HasGLInstancing(GetCaps().featureLevel))
			return false;

		GLenum mode = GL_TRIANGLES;
		GLsizei count = 0;
		if (!ToGLPrimitiveModeAndCount(primitiveType, primitiveCount, mode, count))
			return false;

		ApplyBlendingModeChange();
		ApplyStencilStateChange();
		ApplyDrawBuffersForCurrentProgram();
		if (m_CurrentVertexDeclaration)
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr);

		const void* indexOffset = reinterpret_cast<const void*>(static_cast<uintptr_t>(indexStart * sizeof(uint16_t)));
#if !defined(PARAENGINE_MOBILE) && !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
		if (baseVertexIndex != 0 && glad_glDrawElementsInstancedBaseVertex != nullptr)
		{
			glDrawElementsInstancedBaseVertex(mode, count, GL_UNSIGNED_SHORT, indexOffset, static_cast<GLsizei>(instanceCount), baseVertexIndex);
		}
		else
#endif
		if (baseVertexIndex != 0 && m_CurrentVertexDeclaration && m_Streams[0].buffer != nullptr)
		{
			uint32_t origOffset = m_Streams[0].offsetInBytes;
			uint32_t stride = m_Streams[0].stride ? m_Streams[0].stride : m_CurrentVertexDeclaration->GetStride(0);
			m_Streams[0].offsetInBytes = static_cast<uint32_t>(static_cast<int32_t>(origOffset) + baseVertexIndex * static_cast<int32_t>(stride));
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr);
			glDrawElementsInstanced(mode, count, GL_UNSIGNED_SHORT, indexOffset, static_cast<GLsizei>(instanceCount));
			m_Streams[0].offsetInBytes = origOffset;
			ApplyGLVertexDeclaration(m_CurrentVertexDeclaration, nullptr);
		}
		else
		{
			glDrawElementsInstanced(mode, count, GL_UNSIGNED_SHORT, indexOffset, static_cast<GLsizei>(instanceCount));
		}
		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	bool RenderDeviceOpenGL::ReadDepthPixel(int x, int y, float& outDepth)
	{
		float depth = 1.0f;
		int glY = static_cast<int>(m_RenderTargetHeight) - y - 1;
		if (glY < 0)
			glY = 0;
#if !defined(PARAENGINE_MOBILE) && !defined(__EMSCRIPTEN__)
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
				glFrontFace(GL_CW);
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
		case ERenderState::TWOSIDEDSTENCILMODE:
			m_TwoSidedStencil = (Value != 0);
			m_StencilDirty = true;
			break;
		case ERenderState::CCW_STENCILFUNC:
			m_CCWStencilFunc = Value;
			m_StencilDirty = true;
			break;
		case ERenderState::CCW_STENCILFAIL:
			m_CCWStencilFail = Value;
			m_StencilDirty = true;
			break;
		case ERenderState::CCW_STENCILZFAIL:
			m_CCWStencilZFail = Value;
			m_StencilDirty = true;
			break;
		case ERenderState::CCW_STENCILPASS:
			m_CCWStencilPass = Value;
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
#if !defined(PARAENGINE_MOBILE) && !defined(__EMSCRIPTEN__)
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
				if ((sb.frequencyDivider & 0x80000000u) != 0)
					divisor = sb.frequencyDivider & 0x3FFFFFFFu;
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

			if (HasGLInstancing(GetCaps().featureLevel) && m_AttribDivisors[slot] != divisor)
			{
				glVertexAttribDivisor(slot, divisor);
				m_AttribDivisors[slot] = divisor;
			}
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
		ApplyDrawBuffersForCurrentProgram();
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
		ApplyDrawBuffersForCurrentProgram();
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
		ApplyDrawBuffersForCurrentProgram();
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
		if (m_isBeginRenderTarget || m_BoundMrtCount > 0)
			return 8;

		static bool once = true;
		static GLint g_sStencilBits = -1;
		if (once)
		{
			GLint bits = 0;
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__)
			glGetIntegerv(GL_STENCIL_BITS, &bits);
			if (glGetError() != GL_NO_ERROR)
				bits = 8;
#else
			glGetIntegerv(GL_STENCIL_BITS, &bits);
			if (glGetError() != GL_NO_ERROR || bits == 0)
			{
				glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &bits);
				if (glGetError() != GL_NO_ERROR)
					bits = 8;
			}
#endif
			g_sStencilBits = bits;
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
		ApplyDrawBuffersForCurrentProgram();

		const void* indexOffset = reinterpret_cast<const void*>(static_cast<uintptr_t>(sizeof(uint16_t) * indexStart));
#if !defined(PARAENGINE_MOBILE) && !defined(__EMSCRIPTEN__)
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
		if (color && m_BoundMrtCount > 1 && m_AppliedDrawBufferMask != m_ValidMrtMask && HasGLDrawBuffers(GetCaps().featureLevel))
		{
			GLenum drawBufs[4] = { GL_NONE, GL_NONE, GL_NONE, GL_NONE };
			for (uint32_t i = 0; i < m_BoundMrtCount; ++i)
			{
				if (m_ValidMrtMask & (1u << i))
					drawBufs[i] = GL_COLOR_ATTACHMENT0 + i;
			}
			glDrawBuffers(static_cast<GLsizei>(m_BoundMrtCount), drawBufs);
			m_AppliedDrawBufferMask = m_ValidMrtMask;
		}

		// OpenGL glClear obeys write masks; temporarily unmask requested channels to match D3D/ParaRHI Clear semantics.
		const uint32_t prevColorMask = m_StateTracker.GetRenderState(ERenderState::COLORWRITEENABLE);
		const uint32_t prevZWrite = m_StateTracker.GetRenderState(ERenderState::ZWRITEENABLE);
		const uint32_t prevStencilMask = m_StateTracker.GetRenderState(ERenderState::STENCILWRITEMASK);

		uint32_t fields = 0;
		if (depth)
		{
			fields |= GL_DEPTH_BUFFER_BIT;
			if (prevZWrite == 0)
				glDepthMask(GL_TRUE);
		}
		if (color)
		{
			fields |= GL_COLOR_BUFFER_BIT;
			if ((prevColorMask & 0xFu) != 0xFu)
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
		if (stencil)
		{
			fields |= GL_STENCIL_BUFFER_BIT;
			if (prevStencilMask != 0xFFFFFFFFu)
				glStencilMask(0xFFFFFFFFu);
		}

		if (fields != 0)
		{
			// OpenGL glClear obeys GL_SCISSOR_TEST; temporarily disable it so Clear() matches D3D/ParaRHI full-RT clear semantics.
			const GLboolean wasScissor = glIsEnabled(GL_SCISSOR_TEST);
			if (wasScissor)
				glDisable(GL_SCISSOR_TEST);
			glClear(fields);
			if (wasScissor)
				glEnable(GL_SCISSOR_TEST);
		}

		if (depth && prevZWrite == 0)
			glDepthMask(GL_FALSE);
		if (color && (prevColorMask & 0xFu) != 0xFu)
		{
			glColorMask(
				(prevColorMask & 0x1u) ? GL_TRUE : GL_FALSE,
				(prevColorMask & 0x2u) ? GL_TRUE : GL_FALSE,
				(prevColorMask & 0x4u) ? GL_TRUE : GL_FALSE,
				(prevColorMask & 0x8u) ? GL_TRUE : GL_FALSE);
		}
		if (stencil && prevStencilMask != 0xFFFFFFFFu)
			glStencilMask(prevStencilMask);

		PE_CHECK_GL_ERROR_DEBUG();
		return true;
	}

	bool RenderDeviceOpenGL::SetScissorRect(RECT* pRect)
	{
		m_StateTracker.SetScissorRect(pRect);
		if (!pRect)
		{
			glScissor(0, 0, static_cast<GLsizei>(m_RenderTargetWidth), static_cast<GLsizei>(m_RenderTargetHeight));
		}
		else
		{
			const int nScreenHeight = static_cast<int>(m_RenderTargetHeight);
			const GLsizei w = pRect->right > pRect->left ? static_cast<GLsizei>(pRect->right - pRect->left) : 0;
			const GLsizei h = pRect->bottom > pRect->top ? static_cast<GLsizei>(pRect->bottom - pRect->top) : 0;
			glScissor(
				static_cast<GLint>(pRect->left),
				static_cast<GLint>(nScreenHeight - pRect->bottom),
				w,
				h);
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
#if defined(PARAENGINE_MOBILE) || defined(__EMSCRIPTEN__)
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
		if (m_TwoSidedStencil && HasGLTwoSidedStencil())
		{
			glStencilFuncSeparate(GL_FRONT, ToGLCompareFunc(m_StencilFunc), static_cast<GLint>(m_StencilRefValue), m_StencilMask);
			glStencilOpSeparate(GL_FRONT, ToGLStencilOp(m_StencilFail), ToGLStencilOp(m_StencilZFail), ToGLStencilOp(m_StencilPass));
			glStencilFuncSeparate(GL_BACK, ToGLCompareFunc(m_CCWStencilFunc), static_cast<GLint>(m_StencilRefValue), m_StencilMask);
			glStencilOpSeparate(GL_BACK, ToGLStencilOp(m_CCWStencilFail), ToGLStencilOp(m_CCWStencilZFail), ToGLStencilOp(m_CCWStencilPass));
		}
		else
		{
			glStencilFunc(ToGLCompareFunc(m_StencilFunc), static_cast<GLint>(m_StencilRefValue), m_StencilMask);
			glStencilOp(ToGLStencilOp(m_StencilFail), ToGLStencilOp(m_StencilZFail), ToGLStencilOp(m_StencilPass));
		}
	}

	void RenderDeviceOpenGL::ApplyDrawBuffersForCurrentProgram()
	{
		if (m_BoundMrtCount <= 1 || !HasGLDrawBuffers(GetCaps().featureLevel))
			return;
		const uint8_t desiredMask = static_cast<uint8_t>(m_ValidMrtMask & GLProgram::getCurrentColorOutputMask());
		if (desiredMask == m_AppliedDrawBufferMask)
			return;
		GLenum drawBufs[4] = { GL_NONE, GL_NONE, GL_NONE, GL_NONE };
		for (uint32_t i = 0; i < m_BoundMrtCount; ++i)
		{
			if (desiredMask & (1u << i))
				drawBufs[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		glDrawBuffers(static_cast<GLsizei>(m_BoundMrtCount), drawBufs);
		m_AppliedDrawBufferMask = desiredMask;
	}
}
