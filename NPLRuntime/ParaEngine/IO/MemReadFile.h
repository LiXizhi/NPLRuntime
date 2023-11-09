#pragma once
#include "IFile.h"

namespace ParaEngine
{
	class CMemReadFile :
		public IReadFile
	{
	public:
		CMemReadFile(void);
		CMemReadFile(const char * filename);
		CMemReadFile(IReadFile* pFile, uint32_t nSize);
		/* zip buffer
		* @param buffer:
		* @param nSize: size in unsigned char of the buffer
		* @param bDeleteBuffer: true if the zip file will take the ownership of the buffer and will delete it on exit.
		*/
		CMemReadFile(unsigned char* buffer, uint32_t nSize, bool bDeleteBuffer = true);

		virtual ~CMemReadFile(void);

		/// returns how much was read
		virtual uint32_t read(void* buffer, uint32_t sizeToRead);

		/// changes position in file, returns true if successful
		/// if relativeMovement==true, the pos is changed relative to current pos,
		/// otherwise from begin of file
		virtual bool seek(uint32_t finalPos, bool relativeMovement = false);

		/// returns size of file
		virtual uint32_t getSize() { return m_CacheEndPos - m_CacheStartPos; };

		/// returns if file is open
		virtual bool isOpen() { return m_CacheData != 0; };

		/// returns where in the file we are.
		virtual uint32_t getPos() { return m_curPos; };

		/// returns name of file
		virtual const char* getFileName() { return "MemReadFile"; };

		/// delete this
		virtual void Release() { delete this; };

		/** get the buffer at current location*/
		unsigned char* getBuffer();
	private:
		uint32_t	m_CacheStartPos;
		uint32_t	m_CacheEndPos;
		uint32_t	m_curPos;
		unsigned char*	m_CacheData;
		bool	m_bOwnBuffer;

		void Unload();
	};

}