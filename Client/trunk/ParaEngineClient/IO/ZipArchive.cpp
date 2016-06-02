//-----------------------------------------------------------------------------
// Class: CZipArchive
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.10
// Revised: 2006.11.27
// Notes: 
// Index building for 20000 files in a zip archive is 0.10 seconds
// random file access using this index for 20000 files is 0.000025 seconds
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ZipArchive.h"
#include "ZipWriter.h"
#include "ReadFile.h"
#include "MemReadFile.h"
#include <algorithm>
#include "FileManager.h"
#include "util/StringHelper.h"
#include "util/regularexpression.h"

/**@def define this macro to compile with ZLIB. */
#define COMPILE_WITH_ZLIB
#ifdef COMPILE_WITH_ZLIB
#include <zlib.h>
#endif

/** keys used when encoding the pkg file. A pkg package muse be encoded and decoded with the same key, or decoding will fail.*/
#define PKG_KEY1	1
#define PKG_KEY2	2
#define PKG_KEY3	3
#define PKG_KEY4	4

/** @def define this macro to cache read all header information to memory from the central directory. 
this will reduce disk IO counts. However, there does not seem to be a performance penalty even with 20000+ IO read.
so there is no need to use it. */
//#define ENABLE_INDEX_CACHE

using namespace ParaEngine;

CZipArchive::CZipArchive(void)
:m_pFile(NULL),m_bIgnoreCase(true),m_zipComment(NULL),m_pEntries(NULL),m_bRelativePath(false)
{

}

CZipArchive::CZipArchive(bool bIgnoreCase)
:m_pFile(NULL),m_bIgnoreCase(bIgnoreCase),m_zipComment(NULL),m_pEntries(NULL),m_bRelativePath(false)
{
}

CZipArchive::~CZipArchive(void)
{
	SAFE_DELETE(m_pFile);
	SAFE_DELETE_ARRAY(m_zipComment);
	SAFE_DELETE_ARRAY(m_pEntries);
}

/*
Load Archive File Logics
========================
if(sArchiveName is "[filename].pkg") then
Load "[filename].pkg"
return
end
Suppose the loaded sArchiveName is "[filename].zip", 
if(At debug mode) then
if "[filename].zip" does not exist then
if "[filename].pkg" exists then
Load "[filename].pkg"
else
No file loaded.
end
else
if "[filename].pkg" exists and its modified date is newer than the "[filename].zip" then
Load "[filename].pkg"
else
Load "[filename].zip"
generate a new file called "[filename].pkg" based on "[filename].zip"
end
end
else 
if "[filename].zip" exist then
Load "[filename].zip"
else if "[filename].pkg" exist then
Load "[filename].pkg"
else
No file loaded.
end
end

pkg File Format
========================
char[4] <file type id> ".pkg"
byte[4] <pkg file version> 0.0.0.1
[int] <length of string>
[string] "ParaEngine Tech Studio Package File Format. Please note that content in 
this file is meant to be protected and copyrighted by their author(s). Decoding this pkg file is illegal."
[int] reserved1
[int] reserved2
[int] reserved3
[int] reserved4
[int] Central Directory Size in Bytes
[int] number of directory record
[item 1]: LenOfFilename, filename, compression method, compressed size,uncompressed size, fileDataPosition(this should be encoded using a key, current file address and compressed file size, etc)
...
[item n]
[data 1]: header+data
...
[data n]
*/
bool CZipArchive::Open(const string& sArchiveName, int nPriority)
{	
	Close();
	CArchive::Open(sArchiveName, nPriority);

	string tempStr = m_filename;
#ifdef WIN32
	int nSize = (int)tempStr.size();
	for(int i =0; i<nSize;i++)
	{
		if(tempStr[i] == '/')
			tempStr[i] = '\\';
	}
#endif

	const string sFileExt = CParaFile::GetFileExtension(tempStr);
	if(sFileExt == "pkg")
	{
		return OpenPkgFile(tempStr);
	}

	if( CParaFile::DoesFileExist(tempStr.c_str(), false, true))
	{
		m_bOpened =  OpenZipFile(tempStr);
	}
	else
	{
		string pkgFile = CParaFile::ChangeFileExtension(tempStr, "pkg");
		if (CParaFile::DoesFileExist(pkgFile.c_str(), false, true))
		{
			m_bOpened =  OpenPkgFile(pkgFile);
		}
	}	

	return m_bOpened;
}

void CZipArchive::SetRootDirectory( const string& filename )
{
	char tmp[256];
	int nLastPos = 0;
	int nSize = (int)filename.size();
	if(nSize<256)
	{
		for (int i=0;i<nSize;++i)
		{
			tmp[i] = filename[i];
			if(tmp[i] == '\\' || tmp[i] == '/')
			{
				tmp[i] =  '/';
				nLastPos = i+1;
			}
		}
		tmp[nLastPos] = '\0';
		if(nLastPos == 0)
		{
			m_bRelativePath = false;
			m_sRootPath.clear();
		}
		else
		{
			m_bRelativePath = true;
			m_sRootPath = tmp;
		}
	}
	else
	{
		m_bRelativePath = false;
		OUTPUT_LOG("zip file name is too long: %s \r\n", filename.c_str());
	}
}

bool CZipArchive::OpenZipFile(const string& filename)
{
	m_pFile = new CReadFile(filename);
	m_bOpened = m_pFile->isOpen();
	if (!m_bOpened)
	{
		SAFE_DELETE(m_pFile);
		m_pFile = new CMemReadFile(filename.c_str());
		m_bOpened = m_pFile->isOpen();
		if (!m_bOpened)
			SAFE_DELETE(m_pFile);
	}
	if(m_bOpened)
	{
		// scan local headers
		// it does not sequentially transverse the file, 
		// instead it uses central directory records at the end of the zip file

		//PERF_BEGIN(sArchiveName.c_str());
		m_bOpened = ReadEntries();
		//PERF_END(sArchiveName.c_str());

		// prepare file index for binary search
		m_FileList.sort();

		if(m_bOpened)
		{
			OUTPUT_LOG("Archive: %s is opened and read %d entries\n", filename.c_str(), (int)(m_FileList.size()));
		}
		else
		{
			OUTPUT_LOG("Warning: can not read archive: %s, perhaps file is corrupted\n", filename.c_str());
		}
	}
	return m_bOpened;
}

bool CZipArchive::OpenPkgFile(const string& filename)
{
	ParaEngine::Lock lock_(m_mutex);

	m_pFile = new CReadFile(filename);
	m_bOpened = m_pFile->isOpen();
	if (!m_bOpened)
	{
		SAFE_DELETE(m_pFile);
		m_pFile = new CMemReadFile(filename.c_str());
		m_bOpened = m_pFile->isOpen();
		if (!m_bOpened)
			SAFE_DELETE(m_pFile);
	}

	if(m_bOpened)
	{
		m_bOpened = ReadEntries_pkg();

		// no need to sort since already sorted in file. 
		// m_FileList.sort();
		if(m_bOpened)
		{
			OUTPUT_LOG("Archive: %s is opened and read %d entries\n", filename.c_str(), (int)(m_FileList.size()));
		}
	}
	return m_bOpened;
}

bool CZipArchive::OpenMemFile(const char* buffer, DWORD nSize, bool bDeleteBuffer)
{
	m_pFile = new CMemReadFile((byte*)buffer, nSize, bDeleteBuffer);
	m_bOpened = m_pFile->isOpen();
	if(m_bOpened)
	{
		m_bOpened = ReadEntries();
		m_FileList.sort();
	}
	return m_bOpened;
}

int CZipArchive::GetFileCount()
{
	return (int) (m_FileList.size());
}

bool CZipArchive::GeneratePkgFile( const char* filename )
{
	CParaFile file;
	if(!file.CreateNewFile(filename))
		return false;

	ParaEngine::Lock lock_(m_mutex);

	// pkg header
	file.WriteString(".pkg",4);
#define PKG_FILE_VERSION	1
	// version number
	file.WriteDWORD(PKG_FILE_VERSION);
	// reserved bytes
	file.WriteDWORD(0);file.WriteDWORD(0);file.WriteDWORD(0);file.WriteDWORD(0);
	// comment
	const char* comments = "ParaEngine Tech Studio Package File.";
	int nLen = (int)strlen(comments);
	file.WriteDWORD(nLen);
	file.WriteString(comments, nLen);
	// center directory
	int nEntryNum = (int)(m_FileList.size());
	int i=0;
	file.WriteDWORD(nEntryNum);
	DWORD nDirSize = 0;
	{
		// get dir section size
		for (i=0;i<nEntryNum;++i)
		{
			SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
			int nFileNameLen = (int)entry.zipFileName.size();
			nDirSize+=sizeof(WORD)+nFileNameLen+sizeof(WORD)+sizeof(DWORD)*3;
		}
	}
	DWORD dataPos = (DWORD)file.getPos()+nDirSize;
	for (i=0;i<nEntryNum;++i)
	{
		SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
		WORD nFileNameLen = (WORD)entry.zipFileName.size();
		file.write(&nFileNameLen, sizeof(WORD));
		file.WriteString(entry.zipFileName.c_str());
		file.write(&entry.CompressionMethod, sizeof(WORD));
		file.WriteDWORD(entry.CompressedSize);
		file.WriteDWORD(entry.UncompressedSize);
		// encode this size. 
		DWORD encodedSize = dataPos; 
		if (nFileNameLen >= 4)
		{
			// take the file name into consideration. 
			encodedSize += entry.zipFileName[0]*PKG_KEY1+entry.zipFileName[1]*PKG_KEY2+entry.zipFileName[2]*PKG_KEY3+entry.zipFileName[3]*PKG_KEY4;
		}
		file.WriteDWORD(encodedSize); 
		dataPos+=entry.CompressedSize;
	}
	// write data
	{
		vector<byte> cData;

		for (i=0;i<nEntryNum;++i)
		{
			SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
			if(cData.size()<entry.CompressedSize)
				cData.resize(entry.CompressedSize);
			if(entry.CompressedSize>0)
			{
				m_pFile->seek(entry.fileDataPosition);
				m_pFile->read(&(cData[0]), entry.CompressedSize);
				file.write(&(cData[0]), entry.CompressedSize);
			}
		}
		cData.clear();
	}
	return true;
}

bool CZipArchive::ReadEntries_pkg()
{
	char tmp[1024];
	// header
	m_pFile->read(&tmp, 4);
	if( (tmp[0]=='.') && (tmp[1]=='p') && (tmp[2]=='k') && (tmp[3]=='g') )
	{
	}
	else
	{
		OUTPUT_LOG("ParaEngine PKG archiver file header is incorrect.\r\n");
		return false;
	}

	// version
	DWORD fileVersion = 0;
	m_pFile->read(&fileVersion, sizeof(DWORD));
	if(fileVersion > PKG_FILE_VERSION)
	{
		OUTPUT_LOG("ParaEngine PKG archiver version is newer than supported \r\n");
		return false;
	}
	// reserved words
	m_pFile->seek(sizeof(DWORD)*4, true);

	// comments
	int nLen=0;
	m_pFile->read(&nLen, sizeof(DWORD));
	m_pFile->seek(nLen,true);

	// number of files
	int nEntryNum=0;
	m_pFile->read(&nEntryNum, sizeof(DWORD));

	// OUTPUT_LOG("len: %d, nEntry: %d, sizeof(DWORD)%d, sizeof(int)%d\n", nLen, nEntryNum, sizeof(DWORD), sizeof(int));

	m_FileList.set_used(nEntryNum);
	m_FileList.set_sorted(true);
	SAFE_DELETE_ARRAY(m_pEntries);
	m_pEntries = new SZipFileEntry[nEntryNum];
	for (int i = 0; i < nEntryNum; ++i) 
	{
		m_FileList[i].m_pEntry = &m_pEntries[i];
		SZipFileEntry& entry = *(m_FileList[i].m_pEntry);

		// read filename
		WORD nNameSize = 0;
		m_pFile->read(&nNameSize, sizeof(WORD));
		m_pFile->read(tmp, nNameSize);
		tmp[nNameSize] = 0x0;
		entry.zipFileName = tmp;
		if (m_bIgnoreCase)
			StringHelper::make_lower(entry.zipFileName);

		m_pFile->read(&entry.CompressionMethod, sizeof(WORD));
		m_pFile->read(&entry.CompressedSize, sizeof(DWORD));
		m_pFile->read(&entry.UncompressedSize, sizeof(DWORD));
		int nEncodedDataPos=0;
		m_pFile->read(&nEncodedDataPos, sizeof(DWORD));
		// decode the data pos
		int nDataPos = nEncodedDataPos;
		if (nNameSize >= 4)
		{
			// take the file name into consideration. 
			nDataPos -= entry.zipFileName[0]*PKG_KEY1+entry.zipFileName[1]*PKG_KEY2+entry.zipFileName[2]*PKG_KEY3+entry.zipFileName[3]*PKG_KEY4;
		}
		entry.fileDataPosition = nDataPos;
	}
	return true;
}

void CZipArchive::Close()
{	
	if(m_pEntries != 0)
	{
		SAFE_DELETE_ARRAY(m_pEntries);
	}
	else
	{
		int nLen = m_FileList.size();
		for(int i=0;i<nLen;++i)
			delete m_FileList[i].m_pEntry;
	}
	m_FileList.clear();

	if(m_bOpened)
	{
		SAFE_DELETE(m_pFile);
	}
	SAFE_DELETE_ARRAY(m_zipComment);
	m_bOpened = false;
}

bool CZipArchive::DoesFileExist(const string& filename)
{	
	return findFile(filename)>=0;
}


int CZipArchive::findFile(const string& sFilename)
{
	SZipFileEntry entry;

	if(!m_bRelativePath)
	{
		entry.zipFileName = sFilename;
	}
	else
	{
		int nSize = (int)m_sRootPath.size();
		int i=0;
		for (;i<nSize&&(m_sRootPath[i] == sFilename[i]);++i)
		{
		}
		if(i==nSize)
		{
			entry.zipFileName = sFilename.substr(i);
		}
		else
			return -1; // return file not found
	}

	if (m_bIgnoreCase)
		StringHelper::make_lower(entry.zipFileName);

	int res = m_FileList.binary_search(SZipFileEntryPtr(&entry));

	return res;
}

bool CZipArchive::OpenFile(const char* filename, FileHandle& handle)
{	
	string tempStr = filename;
	int nSize = (int)tempStr.size();
	for(int i =0; i<nSize;i++)
	{
		if(tempStr[i] == '\\')
			tempStr[i] = '/';
	}
	handle.m_index = findFile(tempStr);

	bool bRes = (handle.m_index != -1);
	if(bRes)
		handle.m_pArchive = this;
	return bRes;
}

DWORD CZipArchive::GetFileSize(FileHandle& handle)
{	
	if(handle.m_index!=-1)
	{
		return m_FileList[handle.m_index].m_pEntry->UncompressedSize;
	}
	return 0;
}

bool CZipArchive::ReadFileRaw(FileHandle& handle,LPVOID* lppBuffer,LPDWORD pnCompressedSize, LPDWORD pnUncompressedSize)
{
	ParaEngine::Lock lock_(m_mutex);

	DWORD nBytesRead=0;
	int index = handle.m_index;

	(*lppBuffer) = 0;
	bool res = false;
	WORD compressMethod = m_FileList[index].m_pEntry->CompressionMethod;
	switch(compressMethod)
	{
	case 0: // no compression
	case 8: // zip compressed data
		{
			DWORD uncompressedSize = m_FileList[index].m_pEntry->UncompressedSize;			
			DWORD compressedSize = m_FileList[index].m_pEntry->CompressedSize;
			if(pnCompressedSize)
				*pnCompressedSize = compressedSize;
			if(pnUncompressedSize)
				*pnUncompressedSize = uncompressedSize;
			if(uncompressedSize == 0)
			{
				return true;
			}
			(*lppBuffer) = new byte[ compressedSize ];
			if (!(*lppBuffer))
			{
				OUTPUT_LOG("Not enough memory for reading archive file %s\n", m_FileList[index].m_pEntry->zipFileName.c_str());
				return false;
			}

			m_pFile->seek(m_FileList[index].m_pEntry->fileDataPosition);
			nBytesRead = m_pFile->read(*lppBuffer, compressedSize);
			res = true;
			if(compressMethod == 0)
				*pnUncompressedSize = 0;
			break;
		}
	default:
		OUTPUT_LOG("warning: file %s has unsupported compression method: \n", m_FileList[index].m_pEntry->zipFileName.c_str());
		break;
	};
	return res; 
}

bool CZipArchive::Decompress( LPVOID lpCompressedBuffer, DWORD nCompressedSize, LPVOID lpUnCompressedBuffer, DWORD nUncompressedSize )
{
#ifdef COMPILE_WITH_ZLIB
	// Setup the inflate stream.
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)lpCompressedBuffer;
	stream.avail_in = (uInt)nCompressedSize;
	stream.next_out = (Bytef*)lpUnCompressedBuffer;
	stream.avail_out = (uInt)nUncompressedSize;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	// Perform inflation. wbits < 0 indicates no zlib header inside the data.
	err = inflateInit2(&stream, -MAX_WBITS);
	if (err == Z_OK)
	{
		err = inflate(&stream, Z_FINISH);
		inflateEnd(&stream);
		if (err == Z_STREAM_END)
			err = Z_OK;

		err = Z_OK;
		inflateEnd(&stream);
		return true;
	}
#endif
	return false;
}

/*
this function is based on Nikolaus Gebhardt's CZipReader in the "Irrlicht Engine".
*/
bool CZipArchive::ReadFile(FileHandle& handle,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead)
{
	ParaEngine::Lock lock_(m_mutex);

	//0 - The file is stored (no compression)
	//1 - The file is Shrunk
	//2 - The file is Reduced with compression factor 1
	//3 - The file is Reduced with compression factor 2
	//4 - The file is Reduced with compression factor 3
	//5 - The file is Reduced with compression factor 4
	//6 - The file is Imploded
	//7 - Reserved for Tokenizing compression algorithm
	//8 - The file is Deflated
	//9 - Reserved for enhanced Deflating
	//10 - PKWARE Date Compression Library Imploding
	DWORD nBytesRead=0;
	int index = handle.m_index;
	switch(m_FileList[index].m_pEntry->CompressionMethod)
	{
	case 0: // no compression
		{
			m_pFile->seek(m_FileList[index].m_pEntry->fileDataPosition);
			nBytesRead = m_pFile->read(lpBuffer, nNumberOfBytesToRead);
			if(lpNumberOfBytesRead)
				*lpNumberOfBytesRead = nBytesRead;
		}
	case 8:
		{
#ifdef COMPILE_WITH_ZLIB

			DWORD uncompressedSize = m_FileList[index].m_pEntry->UncompressedSize;			
			DWORD compressedSize = m_FileList[index].m_pEntry->CompressedSize;

			void* pBuf = lpBuffer;
			bool bCopyBuffer = false;
			if(nNumberOfBytesToRead < uncompressedSize)
			{
				pBuf = new byte[ uncompressedSize ];
				if (!pBuf)
				{
					OUTPUT_LOG("Not enough memory for decompressing %s\n", m_FileList[index].m_pEntry->zipFileName.c_str());
					return false;
				}
				bCopyBuffer = true;
			}

			byte *pcData = new byte[ compressedSize ];
			if (pcData==0)
			{
				OUTPUT_LOG("Not enough memory for decompressing %s\n", m_FileList[index].m_pEntry->zipFileName.c_str());
				return false;
			}

			//memset(pcData, 0, compressedSize );
			m_pFile->seek(m_FileList[index].m_pEntry->fileDataPosition);
			m_pFile->read(pcData, compressedSize );

			// Setup the inflate stream.
			z_stream stream;
			int err;

			stream.next_in = (Bytef*)pcData;
			stream.avail_in = (uInt)compressedSize;
			stream.next_out = (Bytef*)pBuf;
			stream.avail_out = uncompressedSize;
			stream.zalloc = (alloc_func)0;
			stream.zfree = (free_func)0;

			// Perform inflation. wbits < 0 indicates no zlib header inside the data.
			err = inflateInit2(&stream, -MAX_WBITS);
			if (err == Z_OK)
			{
				err = inflate(&stream, Z_FINISH);
				inflateEnd(&stream);
				if (err == Z_STREAM_END)
					err = Z_OK;

				err = Z_OK;
				inflateEnd(&stream);
			}
			delete [] pcData;

			if (err == Z_OK)
			{
				if(lpNumberOfBytesRead)
					(*lpNumberOfBytesRead) = nNumberOfBytesToRead;
				if(bCopyBuffer)
				{
					memcpy(lpBuffer, pBuf, nNumberOfBytesToRead);
					delete [] (byte*)pBuf;
				}
				return true;
			}
			else
			{
				OUTPUT_LOG("Error decompressing %s\n", m_FileList[index].m_pEntry->zipFileName.c_str());
				if(bCopyBuffer)
					delete [] (byte*)pBuf;
				return false;
			}

#else
			return 0; // zlib not compiled, we cannot decompress the data.
#endif
		}
		break;
	default:
		OUTPUT_LOG("file %s has unsupported compression method: \n", m_FileList[index].m_pEntry->zipFileName.c_str());
		return 0;
	};
	return false; // SFileReadFile(handle.m_handle, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, 0);
}

bool CZipArchive::CloseFile(FileHandle& hFile)
{
	// Zip file does not need to be closed. 
	hFile.m_index = -1;
	return true;
}

#ifdef LocalHeader // unused code for scanning the local header.
bool CZipArchive::scanLocalHeader()
{
	char tmp[1024];
	SZipFileEntryPtr entryPtr(new SZipFileEntry());
	if(entryPtr.m_pEntry == 0)
		return false;
	SZipFileEntry& entry = *entryPtr.m_pEntry;

	m_pFile->read(&entry.header, sizeof(SZIPFileHeader));

	if (entry.header.Sig != 0x04034b50)
		return false; // local file headers end here.

	// read filename
	entry.zipFileName.reserve(entry.header.FilenameLength+2);
	m_pFile->read(tmp, entry.header.FilenameLength);
	tmp[entry.header.FilenameLength] = 0x0;
	entry.zipFileName = tmp;
	if (m_bIgnoreCase)
		StringHelper::make_lower(entry.zipFileName);

	// extractFilename(&entry);

	// move forward length of extra field.

	if (entry.header.ExtraFieldLength)
		m_pFile->seek(entry.header.ExtraFieldLength, true);

	// if bit 3 was set, read DataDescriptor, following after the compressed data
	if (entry.header.GeneralBitFlag & ZIP_INFO_IN_DATA_DESCRITOR)
	{
		// read data descriptor
		m_pFile->read(&entry.header.DataDescriptor, sizeof(entry.header.DataDescriptor));
	}

	// store position in file
	entry.fileDataPosition = m_pFile->getPos();

	// move forward length of data
	m_pFile->seek(entry.header.DataDescriptor.CompressedSize, true);

	m_FileList.push_back(entryPtr);
	return true;
}
#endif

int CZipArchive::LocateBlockWithSignature(DWORD signature, long endLocation, int minimumBlockSize, int maximumVariableData)
{
	int pos = endLocation - minimumBlockSize;
	if (pos < 0) {
		return -1;
	}

	int giveUpMarker = max(pos - maximumVariableData, 0);

	// read the entire block from giveUpMarker to pos. 
	m_pFile->seek(giveUpMarker, false);
	int nTempBufSize = pos - giveUpMarker;
	if(nTempBufSize<4)
		return -1;
	pos = -1;
	byte * pBuf = new byte[nTempBufSize];
	if( m_pFile->read(pBuf, nTempBufSize) == nTempBufSize)
	{
		for(int i=nTempBufSize-4; i>=0; --i)
		{
			DWORD nSig_LittleEndian = *(DWORD*)(pBuf+i);
			if(nSig_LittleEndian == signature)
			{
				pos = giveUpMarker+(i+4);
				m_pFile->seek(pos, false);
				break;
			}
		} 
		delete []pBuf;
	}
	return pos;
}

bool CZipArchive::ReadEntries()
{
	char tmp[1024];
	// Search for the End Of Central Directory.  When a zip comment is
	// present the directory may start earlier.
	// 
	// TODO: The search is limited to 64K which is the maximum size of a trailing comment field to aid speed.
	// This should be compatible with both SFX and ZIP files but has only been tested for Zip files
	// Need to confirm this is valid in all cases.
	// Could also speed this up by reading memory in larger blocks.			

	// search 4 bytes, if failed search 64KB
	long locatedCentralDirOffset = LocateBlockWithSignature(ZIP_CONST_ENDSIG, m_pFile->getSize(), sizeof(ZIP_EndOfCentralDirectory), 0x4);
	if (locatedCentralDirOffset < 0) {
		locatedCentralDirOffset = LocateBlockWithSignature(ZIP_CONST_ENDSIG, m_pFile->getSize(), sizeof(ZIP_EndOfCentralDirectory), 0xffff);
		if (locatedCentralDirOffset < 0)
			return false;
	}

	ZIP_EndOfCentralDirectory EndOfCentralDir;
	m_pFile->read(&EndOfCentralDir, sizeof(ZIP_EndOfCentralDirectory));

	if(EndOfCentralDir.commentSize>0)
	{
		SAFE_DELETE_ARRAY(m_zipComment);
		m_zipComment = new char[EndOfCentralDir.commentSize]; 
		m_pFile->read(m_zipComment, EndOfCentralDir.commentSize); 
	}
	int offsetOfFirstEntry = 0;
	// SFX support, find the offset of the first entry vis the start of the stream
	// This applies to Zip files that are appended to the end of the SFX stub.
	// Zip files created by some archivers have the offsets altered to reflect the true offsets
	// and so do not require any adjustment here...
	if (EndOfCentralDir.offsetOfCentralDir < locatedCentralDirOffset - (4 + EndOfCentralDir.centralDirSize)) {
		offsetOfFirstEntry = locatedCentralDirOffset - (4 + EndOfCentralDir.centralDirSize + EndOfCentralDir.offsetOfCentralDir);
		if (offsetOfFirstEntry <= 0) {
			return false;//throw new ZipException("Invalid SFX file");
		}
	}

	m_pFile->seek(offsetOfFirstEntry + EndOfCentralDir.offsetOfCentralDir, false);

	IReadFile* pReader = NULL;

#ifdef ENABLE_INDEX_CACHE
	CMemReadFile memfile(m_pFile, locatedCentralDirOffset - m_pFile->getPos());
	pReader = &memfile;
#else
	pReader = m_pFile;
#endif

	int nEntryNum = EndOfCentralDir.entriesForThisDisk;
	m_FileList.set_used(nEntryNum);
	m_FileList.set_sorted(false);
	SAFE_DELETE_ARRAY(m_pEntries);
	m_pEntries = new SZipFileEntry[nEntryNum];
	for (int i = 0; i < nEntryNum; ++i) 
	{
		ZIP_CentralDirectory CentralDir;
		pReader->read(&CentralDir, sizeof(ZIP_CentralDirectory));
		if(CentralDir.Sig!=ZIP_CONST_CENSIG)
		{
			return false; // throw new ZipException("Wrong Central Directory signature");
		}

		m_FileList[i].m_pEntry = &m_pEntries[i];
		if(m_FileList[i].m_pEntry == 0)
			return false;
		SZipFileEntry& entry = *(m_FileList[i].m_pEntry);

		// read filename
		entry.zipFileName.reserve(CentralDir.NameSize+2);
		pReader->read(tmp, CentralDir.NameSize);
		tmp[CentralDir.NameSize] = 0x0;
		entry.zipFileName = tmp;
		if (m_bIgnoreCase)
			StringHelper::make_lower(entry.zipFileName);

#ifdef SAVE_ZIP_HEADER
		/// fill header data
		entry.header.FilenameLength = CentralDir.NameSize;
		entry.header.CompressionMethod = CentralDir.CompressionMethod;
		entry.header.DataDescriptor.CRC32 = CentralDir.FileCRC;
		entry.header.DataDescriptor.UncompressedSize = CentralDir.UnPackSize;
		entry.header.DataDescriptor.CompressedSize = CentralDir.PackSize;
		entry.header.GeneralBitFlag = CentralDir.Flags;
		// entry.header.LastModFileTime = CentralDir.Time;
		entry.header.ExtraFieldLength = CentralDir.ExtraSize;
#endif
		entry.CompressionMethod = CentralDir.CompressionMethod;
		entry.UncompressedSize = CentralDir.UnPackSize;
		entry.CompressedSize = CentralDir.PackSize;

		int nExtraLength = 0;
		if (CentralDir.ExtraSize > 0) {
			pReader->seek(CentralDir.ExtraSize, true);

			/**
			* extra field length fixed by LiXizhi 2007.6.20: this still not exactly matches the zip definition.
			* by definition: the following code block must be run for each zip entity. 
			* However, I know that winzip does not generate extra field and it is always 0. 
			* the zip utility I used, will write some extra data such as time to the extra field. 
			* The following code is run under the assumption that if the CentralDir.ExtraSize is not 0, then we will fetch the extra size from the file list chunk.
			*/
			{
				DWORD oldPos = pReader->getPos();
				pReader->seek(CentralDir.LocalHeaderOffset, false);
				SZIPFileHeader header;
				pReader->read(&header, sizeof(SZIPFileHeader));
				nExtraLength = header.ExtraFieldLength;
				pReader->seek(oldPos, false);
			}

		}

		if (CentralDir.CommentSize > 0) {
			pReader->seek(CentralDir.CommentSize, true);
		}
		// calculate data pos offset.
		int nDataPos = CentralDir.LocalHeaderOffset + sizeof(SZIPFileHeader) + CentralDir.NameSize + nExtraLength;

		entry.fileDataPosition = nDataPos;
		//entry.LocalHeaderOffset = CentralDir.LocalHeaderOffset;
	}
	return true;
}


void CZipArchive::FindFiles(CSearchResult& result, const string& sRootPath, const string& sPattern, int nSubLevel)
{

	if(sPattern.size()>1 && (sPattern[0] == ':'))
	{
		// search using regular expression if the pattern begins with ":", things after ":" is treated like a regular expression. 
		string sFilePattern = sRootPath;
		if(sRootPath.size() >0)
		{
			char lastChar = sRootPath[sRootPath.size()-1];
			if(lastChar != '\\' &&  lastChar != '/')
			{
				sFilePattern += "/";
			}
		}
		sFilePattern += sPattern.substr(1);

		regex re(sFilePattern);

		int nSize = m_FileList.size();
		for (int index=0;index<nSize;++index)
		{
			const SZipFileEntryPtr& entry = m_FileList[index];
			if(regex_search(entry.m_pEntry->zipFileName, re))
			{
				result.AddResult(entry.m_pEntry->zipFileName, entry.m_pEntry->CompressedSize);
			}
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// it performs wild cards search by default, where /\ matches to direction. * matches to anything except /\. and . matches to . itself. 
		// the following code just hard-coded to support search patterns like. "*.*", "*.", "worlds/*.abc", "*/*.jpg", etc
		// It now only support a single sub folder, and uses a linear search.

		string sFilePattern = sRootPath;
		if(sRootPath.size() >0  )
		{
			char lastChar = sRootPath[sRootPath.size()-1];
			if(lastChar != '\\' &&  lastChar != '/')
			{
				sFilePattern += "/";
			}
		}

		int remove_head_len = 0;
		if(m_bRelativePath)
		{
			int nSize = (int)m_sRootPath.size();
			int i=0;
			for (;i<nSize&&(m_sRootPath[i] == sFilePattern[i]);++i)
			{
			}
			if(i==nSize)
			{
				if(sFilePattern.size() == nSize)
					sFilePattern.clear();
				else
				{
					sFilePattern = sFilePattern.substr(i);
					remove_head_len = (int)sFilePattern.size();
				}
			}
			else
			{
				// we will continue to try to match without relative path. 
			}
		}

		if (sPattern != "*.*")
			sFilePattern += sPattern;
		else
			sFilePattern += "*";
		int nDestCount = (int)sFilePattern.size();
		if(nDestCount==0)
			return;
		int nSize = m_FileList.size();

		if(nSubLevel >0)
		{
			OUTPUT_LOG("warning: CZipArchive::FindFiles only support nSubLevel=0 when using wild card search. Hence sub folders are ignored. One can also use regular expression search just begin the search pattern with \":\" like \":.*jpg\" \n");
		}

		if (m_bIgnoreCase)
			StringHelper::make_lower(sFilePattern);

		for (int index=0;index<nSize;++index)
		{
			const SZipFileEntryPtr& entry = m_FileList[index];

			int nCount = (int)entry.m_pEntry->zipFileName.size();
			int j=0;
			int i=0;
			for (i=0; i<nCount;)
			{
				char srcChar = entry.m_pEntry->zipFileName[i];
				char destChar = sFilePattern[j];
				if(destChar==srcChar)
				{
					++i;
					++j;
					if(j==nDestCount)
					{
						break;
					}
				}
				else if(destChar=='*')
				{
					if(srcChar == '/' || srcChar == '\\')
					{
						++j;
						if(j==nDestCount)
						{
							if ((i+1) < nCount)
							{
								// this is a folder match, add folder with 0 size.  
								std::string filename = entry.m_pEntry->zipFileName.substr(remove_head_len, i - remove_head_len);
								result.AddResult(filename, 0);
							}
							break; 
						}
					}
					else if (srcChar == '.')
					{
						if ((j + 1) == nDestCount)
							i++;
						else
						{
							++j;
							if (j == nDestCount)
							{
								break;
							}
						}
					}
					else
					{
						++i;
						if(i==nCount && j == (nDestCount-1))
						{ // a perfect match for * at the end
							++j;
							break;
						}
					}
				}
				else
				{
					if(destChar == '.')
					{
						++j;
						++i;
					}
					break;
				}
			}
			if(i==nCount && j == nDestCount)
			{
				if(remove_head_len!=0)
				{
					std::string filename = entry.m_pEntry->zipFileName.substr(remove_head_len);	
					result.AddResult(filename, entry.m_pEntry->CompressedSize);
				}
				else
					result.AddResult(entry.m_pEntry->zipFileName, entry.m_pEntry->CompressedSize);
			}
		}
	}

}

void ParaEngine::CZipArchive::SetBaseDirectory(const char * sBaseDir_)
{
	std::string sBaseDir = sBaseDir_;
	if (!sBaseDir.empty())
	{
		ParaEngine::Lock lock_(m_mutex);

		if (m_bIgnoreCase)
			StringHelper::make_lower(sBaseDir);
		if (sBaseDir[sBaseDir.size() - 1] != '/')
		{
			sBaseDir = sBaseDir + "/";
		}
		int nBaseSize = (int)sBaseDir.size();

		int nEntryNum = (int)(m_FileList.size());
		// get dir section size
		for (int i = 0; i < nEntryNum; ++i)
		{
			SZipFileEntry* pEntry = m_FileList[i].m_pEntry;

			// check if zipFileName begins with sBaseDir
			if ((int)pEntry->zipFileName.size() > nBaseSize &&
				pEntry->zipFileName.compare(0, nBaseSize, sBaseDir) == 0)
			{
				pEntry->zipFileName = pEntry->zipFileName.substr(nBaseSize);
			}
		}
	}
	
}

int ParaEngine::CZipArchive::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CArchive::InstallFields(pClass, bOverride);

	pClass->AddField("SetBaseDirectory", FieldType_String, (void*)SetBaseDirectory_s, NULL, NULL, NULL, bOverride);
	return S_OK;
}
