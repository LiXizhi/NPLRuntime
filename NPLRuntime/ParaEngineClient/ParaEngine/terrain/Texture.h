#pragma once
#include "TerrainCommon.h"
#include "Terrain.h"
namespace ParaEngine
{
	struct TextureEntity;
}
namespace ParaTerrain
{
	using namespace ParaEngine;
	/// \brief Texture represents a 2D texture image.

	/// Instances of this class are used for the overall terrain texture, for common textures, and for detail textures.
	/// See the Documentation at terrainengine.com for details on each of these different uses of textures. Most users
	/// simply let TextureLoaders create instances of this class for them, but you can use
	/// the static SetXXXTextureFormat methods of this class to specify which OpenGL texture formats Demeter should create
	/// even if you aren't directly creating your own instances of this class. You can create your own instances of this class
	/// for advanced texturing, such as when implementing a TextureFactory or doing detail texture painting.
	class Texture
	{
	public:
		/// \brief Creates a new 2D texture image from a raw buffer of bytes.
		/// 
		/// This constructor assumes that the buffer contains RGB byte triplets or RGBA byte quartets. It makes a copy of the buffer, so the Texture object does not attempt to own the buffer for later deleting.
		/// \param pBuffer A pointer to the RGB or RGBA bytes describing the image.
		/// \param width The width of the image in pixels.
		/// \param height The height of the image in pixels.
		/// \param stride The number of triplets or quartets in each row. For simple buffers, this will be equal to the width parameter. It will only be different if the rows in your image buffer contain padding before the next row begins.
		/// \param borderSize The border size to use when this image is converted into an OpenGL texture. This value should be zero unless you have built border pixels into your image.
		/// \param bClamp Specifies whether or not to set texture clamping when the image is converted into an OpenGL texture.
		/// \param useCompression Specifies whether or not to use OpenGL compression extensions when this image is converted into an OpenGL texture.
		/// \param useAlpha Specifies whether or not the buffer passed in the pBuffer parameter contains RGBA quartets (where the 4th byte of each pixel is alpha) or not (if not, the buffer contains only RGB triplets.)
		Texture(const uint8 * pBuffer, int width, int height, int stride, int borderSize, bool bClamp, bool useCompression, bool useAlpha = false);
		/// \brief Creates a new 2D texture image from raw bytes.
		/// 
		/// This constructor assumes that the buffer contains RGB byte triplets or RGBA byte quartets. It makes a copy of the buffer, so the Texture object does not attempt to own the buffer for later deleting.
		/// Texture constructor detailed description.
		/// \param pBuffer A pointer to the RGB or RGBA bytes describing the image.
		/// \param width The width of the image in pixels.
		/// \param height The height of the image in pixels.
		/// \param stride The number of triplets or quartets in each row. For simple buffers, this will be equal to the width parameter. It will only be different if the rows in your image buffer contain padding before the next row begins.
		/// \param borderSize The border size to use when this image is converted into an OpenGL texture. This value should be zero unless you have built border pixels into your image.
		/// \param bClamp Specifies whether or not to set texture clamping when the image is converted into an OpenGL texture.
		/// \param useCompression Specifies whether or not to use OpenGL compression extensions when this image is converted into an OpenGL texture.
		/// \param bytesPerPixel The number of bytes per pixel in the image buffer. Currently, only values of 3 or 4 are legal.
		/// \param textureFormat The OpenGL texture format of the texture to be created from the image data.
		Texture(const uint8 * pBuffer, int width, int height, int stride, int borderSize, bool bClamp, bool useCompression, int bytesPerPixel, int textureFormat);
		
		/** texture from ParaEngine's texture entity.*/
		Texture(TextureEntity* pEntity);

		/// \brief Creates an empty 2D texture image that is not yet suitable for use by Demeter.
		/// This constructor is only useful if you need to create an instance of Texture but must defer the loading of its image data until a later time.
		  Texture();
		  
		/// \brief Destroys the texture, including its image data (if still held by this time) and OpenGL texture.
		 ~Texture();
		/// \brief Binds the 2D image data to an OpenGL texture.

		/// If SetBufferPersistent() has been called on this Texture object, then the associated 2D image data is maintained in system RAM for later calls to UploadTexture(). Otherwise, the image data is freed from system RAM and only the OpenGL texture remains (hopefully residing in video RAM.)
		/// \return The texture id of the OpenGL texture object.
		 DeviceTexturePtr_type UploadTexture();
		/// \brief Unbinds the OpenGL texture object, releasing texture memory. 

		/// If SetBufferPersistent() has been called on this Texture object, it can be rebound by calling UploadTexture().
		/// UnloadTexture function detailed description.
		void UnloadTexture();
		/// \brief Gets a pointer to the 2D image data represented by this Texture object.
		/// \return A pointer to the RGB or RGBA bytes representing the 2D image buffer.
		uint8 *GetBuffer();
		/// \brief Gets the width of the 2D image in pixels.
		int GetWidth();
		/// \brief Gets the height of the 2D image in pixels.
		int GetHeight();
		/** set the image width. height is also changed.  it will resize the buffer (shrink or enlarge the image) if necessary. */
		void SetWidth(int nWidth);

		/// \brief Marks this Texture as belonging to a Terrain's pool of textures to be used for detail texture painting.

		/// Generally, external code has little reason to ever call this method. This method is used internally by Demeter to manage texture pools.
		/// \param index The new unique ID of this Texture. You must guarantee that the ID is unique relative to all other Textures in the pool.
		/// SetSharedIndex function detailed description.
		void SetSharedIndex(int index);
		/// \brief Gets the index of this Texture in the managing Terrain's pool of detail textures.

		/// Gives you the unique ID of this Texture. This ID can then be used to call methods such as Terrain::Paint() to paint the Texture on the Terrain's surface.
		/// \return The unique ID of this Texture within the owning Terrain's pool of detail textures.
		int GetSharedIndex();
		/// \brief Simply tags the Texture as having come from the specified file.

		/// This tag is only used by Demeter when saving files from the Demeter Texture Editor. Outside of that context, you may freely use this value for your own purposes.
		/// \param szFilename Any null-terminated string.
		void SetFilename(const char *szFilename);
		/// \brief Gets the filename tag for this Texture.
		/// \return A null-terminated string of the filename. It is the caller's responsibility to free this string.
		const char *GetFilename();
		/// \brief Flips the 2D image data of this Texture vertically.

		/// If the Texture has already been bound, then it must be rebound by calling UploadTexture() before this change will be visible.
		void FlipVertical();
		/// \brief Flips the 2D image data of this Texture horizontally.

		/// If the Texture has already been bound, then it must be rebound by calling UploadTexture() before this change will be visible.
		void FlipHorizontal();
		/// \brief Indicates whether or not this Texture has already been bound to an OpenGL texture object.
		bool IsBound();
		/// \brief Gets the color depth of this Texture.
		int GetBitsPerPixel();
		int GetBytesPerPixel();
		/// \brief Indicates whether or not this Texture uses clamp extensions when bound to an OpenGL texture object.
		bool IsClamped();
		/// \brief Indicates whether or not this Texture uses compression extensions when bound to an OpenGL texture object.
		bool UseCompression();
		/// \brief Gets the OpenGL border size of this Texture.
		int GetBorderSize();
		/// \brief Gets the OpenGL texture format of this Texture. For example, GL_RGBA, etc.
		int GetFormat();
		/// \brief Specified whether or not this Texture's 2D image data should be retained after it is bound to an OpenGL texture object.

		/// By default, when a Texture is bound by calling UploadTexture(), its 2D image buffer is deleted. If you need to dynamically modify
		/// the texture at runtime, then you must change this behavior by calling SetBufferPersisten(true). In this mode, when UploadTexture()
		/// is called, the Texture object still has its original 2D image buffer, so you can make changes to this buffer and call UploadTexture() again
		/// to make those changes visible. Of course, this makes each Texture consume approximately twice the memory.
		void SetBufferPersistent(bool persist);
		/// \brief Specifies the default OpenGL texture format that textures should be created as when they are bound to OpenGL texture objects. 

		/// This setting is used by Demeter whenever it creates OpenGL texture objects from the 2D image data provided by your application (or when Texture Loaders are used.) 
		/// Therefore, this setting allows you to specify what the format of textures will be, even if you aren't directly creating Texture objects yourself. The default value is GL_RGB8.
		static void SetDefaultTextureFormat(int GLTextureFormat);
		/// \brief Specifies the default OpenGL texture format that textures containing ONLY alpha values should be created as when they are bound to OpenGL texture objects.

		/// This setting is used by Demeter whenever it creates OpenGL texture objects from the 2D image data provided by your application (or when Texture Loaders are used.) 
		/// Therefore, this setting allows you to specify what the format of textures will be, even if you aren't directly creating Texture objects yourself.
		static void SetAlphaTextureFormat(int GLTextureFormat);
		/// \brief Specifies the default OpenGL texture format that textures should be created as when they are bound to OpenGL texture objects. The default is GL_RGBA.

		/// This setting is used by Demeter whenever it creates OpenGL texture objects from the 2D image data provided by your application (or when Texture Loaders are used.) 
		/// Therefore, this setting allows you to specify what the format of textures will be, even if you aren't directly creating Texture objects yourself.
		static void SetRgbaTextureFormat(int GLTextureFormat);
		/// \brief Specifies the default OpenGL texture format that textures should be created as when they are bound to OpenGL texture objects.

		/// This setting is used by Demeter whenever it creates OpenGL texture objects from the 2D image data provided by your application (or when Texture Loaders are used.) 
		/// Therefore, this setting allows you to specify what the format of textures will be, even if you aren't directly creating Texture objects yourself. The default value is COMPRESSED_RGB_S3TC_DXT1_EXT.
		static void SetCompressedTextureFormat(int GLTextureFormat);
		/** get the TextureEntity associated with this texture object. if the texture type is ParaX texture entity*/
		TextureEntity* GetTextureEntity();
		/** set the TextureEnity. The function only succeed, if the texture type is ParaX texture entity. 
		* this function can be called multiple times to replace old textures.*/
		bool SetTextureEntity(TextureEntity* texture);

		/// get Texture device pointer
		DeviceTexturePtr_type GetDevicePointer();
		bool IsEqual(ParaEngine::TextureEntity* pEntity);
	private:
		void Init(const uint8 * pBuffer, int width, int height, int stride, int borderSize, bool bClamp, bool useCompression, bool useAlpha = false);
		uint8 *m_pBuffer;
		int m_nWidth;
		int m_nHeight;
		int m_RowLength;
		int m_BorderSize;
		bool m_UseCompression:1;
		bool m_bClamp:1;
		bool m_BufferPersistent:1;
		bool m_bIsParaEngineEntity:1;
		ref_ptr<TextureEntity> m_TextureID;
		int m_SharedIndex;
		char *m_szFilename;
		int m_BytesPerPixel;
		int m_TextureFormat;
		static int m_DefaultTextureFormat;
		static int m_CompressedTextureFormat;
		static int m_AlphaTextureFormat;
		static int m_RgbaTextureFormat;

		friend class DetailTexture;
	};
}
