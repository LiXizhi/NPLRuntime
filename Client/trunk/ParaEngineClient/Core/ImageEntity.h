#pragma once
#include "AssetEntity.h"

namespace ParaEngine
{
	/** Unlike TextureEntity, ImageEntity is a device independent entity in memory. 
	* Hence, it is thread safe by itself. 
	*/
	class ImageEntity : public AssetEntity
	{
	public:
		ImageEntity();
		virtual ~ImageEntity();

		virtual AssetEntity::AssetType GetType(){ return AssetEntity::image; };
	public:
		/**
		@brief Load the image from the specified path.
		@param path   the absolute file path.
		@return true if loaded correctly.
		*/
		bool LoadFromFile(const std::string& path);

		/**
		@brief Load image from stream buffer.
		@param data  stream buffer which holds the image data.
		@param dataLen  data length expressed in (number of) bytes.
		@param bOwnData: if true, we will copy the data. if false, the caller needs to ensure that input data is valid for longer than this object. 
		@return true if loaded correctly.
		*/
		bool LoadFromMemory(const unsigned char * data, size_t dataLen, bool bOwnData = true);

		/** currently only support RGBA8 32bits data, such as from render target */
		bool LoadFromRawData(const unsigned char * data, size_t dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);

		// Getters
		inline unsigned char *   getData()              { return _data; }
		inline size_t           getDataLen()            { return _dataLen; }
		inline DWORD            getFileType()           { return _fileType; }
		inline D3DFORMAT		 getRenderFormat()					{ return _renderFormat; }
		inline int               getWidth()              { return _width; }
		inline int               getHeight()             { return _height; }
		inline int               getNumberOfMipmaps()    { return _numberOfMipmaps; }
		inline bool              hasPremultipliedAlpha() { return false; }

		int                      getBitPerPixel();
		bool                     hasAlpha();
		bool                     isCompressed();

		/**
		@brief    Save Image data to the specified file, with specified format.
		@param    filePath        the file's absolute path, including file suffix.
		@param    isToRGB        whether the image is saved as RGB format.
		*/
		bool SaveToFile(const std::string &filename, bool isToRGB = true);

	protected:
		/** whether we own the data buffer */
		bool m_bIsOwnData;
		unsigned char *_data;
		size_t _dataLen;
		int _width;
		int _height;
		bool _unpack;
		DWORD _fileType;
		D3DFORMAT _renderFormat;
		int _numberOfMipmaps;
		bool _hasPremultipliedAlpha;
		std::string _filePath;
	};
}


