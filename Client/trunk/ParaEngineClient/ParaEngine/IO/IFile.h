#pragma once

namespace ParaEngine
{
	/**
	* Interface for read access to a file.
	*/
	class IReadFile
	{
	public:

		virtual ~IReadFile() {};

		/// Reads an amount of bytes from the file.
		/// @param buffer: Pointer to buffer where to read bytes will be written to.
		/// @param sizeToRead: Amount of bytes to read from the file.
		/// @return Returns how much bytes were read.
		virtual DWORD read(void* buffer, DWORD sizeToRead) = 0;

		/// Changes position in file, returns true if successful.
		/// @param finalPos: Destination position in the file.
		/// @param relativeMovement: If set to true, the position in the file is
		/// changed relative to current position. Otherwise the position is changed 
		/// from begin of file.		
		/// @return Returns true if successful, otherwise false.
		virtual bool seek(DWORD finalPos, bool relativeMovement = false) = 0;

		/// Returns size of file.
		/// @return Returns the size of the file in bytes.
		virtual DWORD getSize() = 0;

		/// returns if file is open
		virtual bool isOpen() = 0;

		/// Returns the current position in the file.
		/// @return Returns the current position in the file in bytes.
		virtual DWORD getPos() = 0;

		/// Returns name of file.
		/// @return Returns the file name as zero terminated character string.
		virtual const char* getFileName() = 0;

		/// delete this
		virtual void Release(){};
	};

	/**
	* Interface providing write access to a file.
	*/
	class IWriteFile 
	{
	public:

		virtual ~IWriteFile() {};

		/// Reads an amount of bytes from the file.
		/// @param buffer: Pointer to buffer of bytes to write.
		/// @param sizeToWrite: Amount of bytes to wrtie to the file.
		/// @return Returns how much bytes were written.
		virtual int write(const void* buffer, int sizeToWrite) = 0;

		/// Changes position in file, returns true if successful.
		/// @param finalPos: Destination position in the file.
		/// @param relativeMovement: If set to true, the position in the file is
		/// changed relative to current position. Otherwise the position is changed 
		/// from begin of file.		
		/// @return Returns true if successful, otherwise false.
		virtual bool seek(int finalPos, bool relativeMovement = false) = 0;

		/// Returns the current position in the file.
		/// @return Returns the current position in the file in bytes.
		virtual int getPos() = 0;

		/// Returns name of file.
		/// @return Returns the file name as zero terminated character string.
		virtual const char* getFileName() = 0;
	};
}