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
		CMemReadFile(IReadFile* pFile, DWORD nSize);
		/* zip buffer
		* @param buffer: 
		* @param nSize: size in byte of the buffer
		* @param bDeleteBuffer: true if the zip file will take the ownership of the buffer and will delete it on exit.
		*/
		CMemReadFile(byte* buffer, DWORD nSize, bool bDeleteBuffer=true);

		virtual ~CMemReadFile(void);

		/// returns how much was read
		virtual DWORD read(void* buffer, DWORD sizeToRead);

		/// changes position in file, returns true if successful
		/// if relativeMovement==true, the pos is changed relative to current pos,
		/// otherwise from begin of file
		virtual bool seek(DWORD finalPos, bool relativeMovement = false);

		/// returns size of file
		virtual DWORD getSize(){return m_CacheEndPos - m_CacheStartPos;};

		/// returns if file is open
		virtual bool isOpen(){return m_CacheData!=0;};

		/// returns where in the file we are.
		virtual DWORD getPos(){return m_curPos;};

		/// returns name of file
		virtual const char* getFileName(){return "MemReadFile";};

		/// delete this
		virtual void Release(){delete this;};

		/** get the buffer at current location*/
		byte* getBuffer();
	private:
		DWORD	m_CacheStartPos;
		DWORD	m_CacheEndPos;
		DWORD	m_curPos;
		byte*	m_CacheData;
		bool	m_bOwnBuffer;

		void Unload();
	};

}

