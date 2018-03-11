#pragma once
#include "IFile.h"
#include <string>
namespace ParaEngine
{
	/**	reading a disk file.
	*/
	class CReadFileOSX : public IReadFile
	{
	public:
		CReadFileOSX(const std::string& fileName);
		
		virtual ~CReadFileOSX();

		/// returns how much was read
		virtual uint32_t read(void* buffer, uint32_t sizeToRead);

		/// changes position in file, returns true if successful
		/// if relativeMovement==true, the pos is changed relative to current pos,
		/// otherwise from begin of file
		virtual bool seek(uint32_t finalPos, bool relativeMovement = false);

		/// returns size of file
		virtual uint32_t getSize();

		/// returns if file is open
		virtual bool isOpen();

		/// returns where in the file we are.
		virtual uint32_t getPos();

		/// returns name of file
		virtual const char* getFileName();

	private:

		/// opens the file
		void openFile();	

		std::string  m_Filename;
		FILE*	m_pFile;
		uint32_t	m_FileSize;
	};
}
