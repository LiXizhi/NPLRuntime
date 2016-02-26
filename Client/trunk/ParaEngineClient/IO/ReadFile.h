#pragma once
#include "IFile.h"
#include <string>
namespace ParaEngine
{
	using namespace std;
	class CMemReadFile;

	/**	reading a disk file.
	*/
	class CReadFile : public IReadFile
	{
	public:
		CReadFile(const string& fileName);
		
		virtual ~CReadFile();

		/// returns how much was read
		virtual DWORD read(void* buffer, DWORD sizeToRead);

		/// changes position in file, returns true if successful
		/// if relativeMovement==true, the pos is changed relative to current pos,
		/// otherwise from begin of file
		virtual bool seek(DWORD finalPos, bool relativeMovement = false);

		/// returns size of file
		virtual DWORD getSize();

		/// returns if file is open
		virtual bool isOpen();

		/// returns where in the file we are.
		virtual DWORD getPos();

		/// returns name of file
		virtual const char* getFileName();

	private:

		/// opens the file
		void openFile();	

		string  m_Filename;
		FILE*	m_pFile;
		DWORD	m_FileSize;
	};
}
