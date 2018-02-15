#pragma once

#include <string>

namespace ParaEngine
{
	using namespace std;
	class CParaFile;
	class CShapeAABB;
	/**
	* writing the scene to script file.
	*/
	class CSceneWriter
	{
	public:
		/** Initializes a new instance of the scene writer class for the specified file, 
		* using the default encoding and buffer size. It will write after the current file pointer.*/
		CSceneWriter(CParaFile& file);
		~CSceneWriter(void);

		/** encoding, one can combine them with bit and operation.*/
		enum ENCODING{
			NPL_TEXT,		// text file encoding
			NPL_BINARY,		// binary file encoding
			NPL_COMPRESSED, // compressed file encoding.
			XML_TEXT,		// XML text
		};
	
	public:
		/** Closes the current writer and the underlying file stream. */
		void Close();

		/** Clears all buffers for the current writer and causes any buffered data to be written to the underlying stream. */
		void Flush();

		/** Gets the Encoding in which the output is written. @see ENCODING */
		DWORD GetEncoding();

		/** Sets the Encoding in which the output is written. @see ENCODING */
		void SetEncoding(DWORD dwEncoding);

		/** Writes some data as specified by the overloaded parameters, followed by a line terminator. */
		void WriteLine();
		void WriteLine(const string& str);
		void WriteLine(double dValue);

		/** Writes some data as specified by the overloaded parameters. */
		void Write(const string& str);
		void Write(const char* pData, int nSize);
		void Write(double dValue);

		/** write the begin managed loader */
		void BeginManagedLoader(const string& sLoaderName);

		/** write the end managed loader */
		void EndManagedLoader();

		/** 
		* write all static scene objects in a rectangular region to the current file. 
		* if the world position is inside the region, then the object is exported.
		* @param sParent: to which NPL variable the data should be attached.*/
		void WriteRegion(const CShapeAABB& aabb);

	private:
		CParaFile&	m_file;
		char*		m_buffer;
		int			m_nbufferSize;
		int			m_nMaxBufferSize;
		DWORD		m_nEncoding;
		bool		m_bInsideManagedLoader;
	};

}
