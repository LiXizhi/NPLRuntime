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
#include "IO/FileUtils.h"
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

#define PKG_FILE_VERSION	1
#define PKG_FILE_VERSION2	2

/** @def define this macro to cache read all header information to memory from the central directory.
this will reduce disk IO counts. However, there does not seem to be a performance penalty even with 20000+ IO read.
so there is no need to use it. */
//#define ENABLE_INDEX_CACHE

using namespace ParaEngine;

// check if it is ".pkg" file
static bool IsPkgData(const char* src)
{
	return ((src[0] == '.') && (src[1] == 'p') && (src[2] == 'k') && (src[3] == 'g'));
}

// return true if data is a zip file or pkg file. 
bool ParaEngine::IsZipData(const char* src, size_t size)
{
	if (size < sizeof(ZIP_EndOfCentralDirectoryBlock))
		return false;

	if (IsPkgData(src))
		return true;

	src += size - sizeof(ZIP_EndOfCentralDirectoryBlock);

	ZIP_EndOfCentralDirectoryBlock* p = (ZIP_EndOfCentralDirectoryBlock*)src;

	return p->sig == ZIP_CONST_ENDSIG;
}

const SZIPFileHeader* ParaEngine::GetFirstFileInfo(const char* src, std::string* filename)
{
	const SZIPFileHeader* p = (const SZIPFileHeader*)src;
	if (p->Sig == ZIP_CONST_LOCALHEADERSIG)
	{
		if (filename != nullptr)
		{
			filename->operator=(std::string(src + sizeof(SZIPFileHeader), p->FilenameLength));
			filename->operator+=('\0');
		}
		return p;
	}
	else
		return nullptr;
}

bool ParaEngine::GetFirstFileData(const char* src, size_t size, std::string& out)
{
	if (IsPkgData(src))
	{
		CMemReadFile file((byte*)src, size, false);
		// sig + version(only support PKG version 2)
		file.seek(8, true);

		// reserved words
		file.seek(sizeof(DWORD) * 4, true);

		// comments
		DWORD nLen = 0;
		file.read(&nLen, sizeof(DWORD));
		file.seek(nLen, true);

		// Ignore Case
		uint8 ignoreCase;
		file.read(&ignoreCase, sizeof(uint8));

		// number of files
		DWORD nEntryNum = 0;
		file.read(&nEntryNum, sizeof(DWORD));
		// name block size
		DWORD nameBuffSize = 0;
		file.read(&nameBuffSize, sizeof(DWORD));
		// name block CompressedSize
		DWORD nameCompressedSize = 0;
		file.read(&nameCompressedSize, sizeof(DWORD));

		// read name block
		std::vector<char> nameBlock;
		nameBlock.resize(nameBuffSize);
		if (nameCompressedSize == 0)
			file.read(&nameBlock[0], nameBuffSize);
		else
		{
			vector<uint8> compressedData;
			compressedData.resize(nameCompressedSize);
			file.read(&compressedData[0], nameCompressedSize);

			if (!CZipArchive::Decompress(&compressedData[0], nameCompressedSize, &nameBlock[0], nameBuffSize))
			{
				OUTPUT_LOG("warning: unable to Decompress name block \n");
				return false;
			}
		}

		if (nEntryNum > 0)
		{
			SZipFileEntry entry;
			// read filename
			file.read(&entry.fileNameLen, sizeof(WORD));
			DWORD nameOffset;
			file.read(&nameOffset, sizeof(DWORD));
			entry.zipFileName = &nameBlock[nameOffset];

			// read hash
			file.read(&entry.hashValue, sizeof(uint32));
			file.read(&entry.CompressionMethod, sizeof(WORD));
			file.read(&entry.CompressedSize, sizeof(DWORD));
			file.read(&entry.UncompressedSize, sizeof(DWORD));
			int nEncodedDataPos = 0;
			file.read(&nEncodedDataPos, sizeof(DWORD));
			// decode the data pos
			int nDataPos = nEncodedDataPos;
			if (entry.fileNameLen >= 4)
			{
				// take the file name into consideration. 
				nDataPos -= entry.zipFileName[0] * PKG_KEY1 + entry.zipFileName[1] * PKG_KEY2 + entry.zipFileName[2] * PKG_KEY3 + entry.zipFileName[3] * PKG_KEY4;
			}
			entry.fileDataPosition = nDataPos;

			DWORD nBytesRead = 0;
			int index = 0;
			switch (entry.CompressionMethod)
			{
			case 0: // no compression
			{
				file.seek(entry.fileDataPosition);
				out.resize(entry.UncompressedSize);
				nBytesRead = file.read(&out[0], entry.UncompressedSize);
				return true;
			}
			case 8:
			{
				out.resize(entry.UncompressedSize);
				char* pBuf = &out[0];
				DWORD uncompressedSize = entry.UncompressedSize;
				DWORD compressedSize = entry.CompressedSize;

				byte* pcData = new byte[compressedSize];
				if (pcData == 0)
				{
					OUTPUT_LOG("Not enough memory for decompressing %s\n", entry.zipFileName);
					return false;
				}

				file.seek(entry.fileDataPosition);
				file.read(pcData, compressedSize);

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
				delete[] pcData;
				return (err == Z_OK);
			}
			}
		}
		return false;
	}
	else
	{
		const SZIPFileHeader* info = (const SZIPFileHeader*)src;
		if (info->Sig != ZIP_CONST_LOCALHEADERSIG)
			return false;

		auto filedata = src + sizeof(SZIPFileHeader) + info->FilenameLength + info->ExtraFieldLength;
		if (info->CompressionMethod == 0)  // 8 for zip, 0 for no compression.
		{
			out = std::string(filedata, info->DataDescriptor.UncompressedSize);
			return true;
		}
		else if (info->CompressionMethod == 8)
		{
			out.resize(info->DataDescriptor.UncompressedSize);

			z_stream stream;
			int err;

			stream.next_in = (Bytef*)filedata;
			stream.avail_in = (uInt)info->DataDescriptor.CompressedSize;
			stream.next_out = (Bytef*)(&*out.begin());
			stream.avail_out = info->DataDescriptor.UncompressedSize;
			stream.zalloc = (alloc_func)0;
			stream.zfree = (free_func)0;

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


			if (err == Z_OK)
			{
				return true;
			}
			else
			{
				std::string filename;
				filename = (std::string(src + sizeof(SZIPFileHeader), info->FilenameLength));
				filename += '\0';
				OUTPUT_LOG("Error decompressing %s\n", filename.c_str());
				return false;
			}
		}
		else
		{
			return false;
		}
	}
}

CZipArchive::CZipArchive(void)
	:m_pFile(NULL), m_bIgnoreCase(true), m_zipComment(NULL), m_pEntries(NULL), m_bRelativePath(false)
	, m_bDirty(true)
{

}

CZipArchive::CZipArchive(bool bIgnoreCase)
	:m_pFile(NULL), m_bIgnoreCase(bIgnoreCase), m_zipComment(NULL), m_pEntries(NULL), m_bRelativePath(false)
	, m_bDirty(true)
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

	const string sFileExt = CParaFile::GetFileExtension(tempStr);
	if (sFileExt == "pkg")
	{
		return OpenPkgFile(tempStr);
	}
	else if (sFileExt == "p3d")
	{
		return OpenPkgFile(tempStr) || OpenZipFile(tempStr);
	}

	DWORD dwFound = CParaFile::DoesFileExist2(tempStr.c_str(), FILE_ON_DISK | FILE_ON_SEARCH_PATH | FILE_ON_EXECUTABLE);
	if (dwFound)
	{
		if (dwFound == FILE_ON_EXECUTABLE)
		{
			FileData data = CFileUtils::GetResDataFromFile(tempStr.c_str());
			if (!data.isNull())
			{
				m_bOpened = OpenMemFile(data.GetBytes(), data.GetSize(), false);
				data.ReleaseOwnership();
			}
		}
		else
		{
			m_bOpened = OpenZipFile(tempStr);
		}
	}
	else
	{
		string pkgFile = CParaFile::ChangeFileExtension(tempStr, "pkg");
		dwFound = CParaFile::DoesFileExist2(pkgFile.c_str(), FILE_ON_DISK | FILE_ON_SEARCH_PATH | FILE_ON_EXECUTABLE);
		if (dwFound == FILE_ON_EXECUTABLE)
		{
			FileData data = CFileUtils::GetResDataFromFile(pkgFile.c_str());
			if (!data.isNull())
			{
				m_bOpened = OpenMemFile(data.GetBytes(), data.GetSize(), false);
				data.ReleaseOwnership();
			}
		}
		else
		{
			m_bOpened = OpenPkgFile(pkgFile);
		}
	}

	return m_bOpened;
}

void CZipArchive::SetRootDirectory(const string& filename)
{
	char tmp[1024];
	int nLastPos = 0;
	int nSize = (int)filename.size();
	if (nSize < 1024)
	{
		for (int i = 0; i < nSize; ++i)
		{
			tmp[i] = filename[i];
			if (tmp[i] == '\\' || tmp[i] == '/')
			{
				tmp[i] = '/';
				nLastPos = i + 1;
			}
		}
		tmp[nLastPos] = '\0';
		if (nLastPos == 0)
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

void CZipArchive::ReBuild()
{
	if (m_bDirty)
	{

		std::sort(m_FileList.begin(), m_FileList.end(), [](const SZipFileEntryPtr& a, const SZipFileEntryPtr& b)
		{
			return a.m_pEntry->hashValue < b.m_pEntry->hashValue;
		});

		m_bDirty = false;
	}
}

bool CZipArchive::OpenZipFile(const string& filename)
{
	SAFE_DELETE(m_pFile);
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
	if (m_bOpened)
	{
		// scan local headers
		// it does not sequentially transverse the file, 
		// instead it uses central directory records at the end of the zip file

		//PERF_BEGIN(sArchiveName.c_str());
		m_bOpened = ReadEntries();
		//PERF_END(sArchiveName.c_str());

		// prepare file index for binary search
		//Sort();

		if (m_bOpened)
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

	SAFE_DELETE(m_pFile);
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

	if (m_bOpened)
	{
		m_bOpened = _ReadEntries_pkg();

		// no need to sort since already sorted in file. 
		// m_FileList.sort();
		if (m_bOpened)
		{
			OUTPUT_LOG("Archive: %s is opened and read %d entries\n", filename.c_str(), (int)(m_FileList.size()));
		}
	}
	return m_bOpened;
}

bool CZipArchive::OpenMemFile(const char* buffer, DWORD nSize, bool bDeleteBuffer)
{
	SAFE_DELETE(m_pFile);
	m_pFile = new CMemReadFile((byte*)buffer, nSize, bDeleteBuffer);
	m_bOpened = m_pFile->isOpen();
	if (m_bOpened)
	{
		m_bOpened = ReadEntries();
		ReBuild();
	}
	return m_bOpened;
}

int CZipArchive::GetFileCount()
{
	return (int)(m_FileList.size());
}

bool CZipArchive::GeneratePkgFile2(const char* filename)
{
	CParaFile file;
	if (!file.CreateNewFile(filename))
		return false;

	ParaEngine::Lock lock_(m_mutex);

	ReBuild();

	// pkg header
	file.WriteString(".pkg", 4);
	// version number
	file.WriteDWORD(PKG_FILE_VERSION2);
	// reserved bytes
	file.WriteDWORD(0); file.WriteDWORD(0); file.WriteDWORD(0); file.WriteDWORD(0);
	// comment
	const char* comments = "ParaEngine Tech Studio Package File.";
	int nLen = (int)strlen(comments);
	file.WriteDWORD(nLen);
	file.WriteString(comments, nLen);

	// IgnoreCase
	file.WriteByte(m_bIgnoreCase ? 1 : 0);

	// center directory
	int nEntryNum = (int)(m_FileList.size());
	file.WriteDWORD(nEntryNum);

	int i = 0;

	DWORD nameBlockSize = 0;
	{
		// get name buff size
		for (; i < nEntryNum; i++)
		{
			SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
			nameBlockSize += entry.fileNameLen + 1;
		}
	}
	file.WriteDWORD(nameBlockSize);

	// write all filename;
	const size_t needCompressSize = 100 * 1024;
	if (nameBlockSize >= needCompressSize)
	{
		std::string input;
		input.reserve(nameBlockSize);
		for (i = 0; i < nEntryNum; i++)
		{
			SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
			input += entry.zipFileName;
			input.push_back(0);
		}

		std::string output;
		if (CZipWriter::Compress(output, (const char*)input.c_str(), input.size()) != 1)
			return false;


		file.WriteDWORD(output.size());
		file.write(output.c_str(), output.size());
	}
	else
	{
		file.WriteDWORD(0);

		for (i = 0; i < nEntryNum; i++)
		{
			SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
			file.write(entry.zipFileName, entry.fileNameLen);
			file.WriteByte(0);
		}
	}


	const DWORD entrySize = sizeof(WORD) + sizeof(DWORD) + sizeof(uint32) + sizeof(WORD) + sizeof(DWORD) * 3;
	DWORD dataPos = (DWORD)file.getPos() + (entrySize * nEntryNum);
	DWORD nameOffset = 0;
	for (i = 0; i < nEntryNum; ++i)
	{
		if (m_FileList[i].m_pEntry == nullptr)
			continue;

		SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
		// name len
		auto nFileNameLen = entry.fileNameLen;
		file.write(&nFileNameLen, sizeof(WORD));
		// name offset
		file.write(&nameOffset, sizeof(DWORD));
		nameOffset += nFileNameLen + 1;
		// name hash
		file.write(&entry.hashValue, sizeof(entry.hashValue));


		file.write(&entry.CompressionMethod, sizeof(WORD));
		file.WriteDWORD(entry.CompressedSize);
		file.WriteDWORD(entry.UncompressedSize);
		// encode this size. 
		DWORD encodedSize = dataPos;
		if (nFileNameLen >= 4)
		{
			// take the file name into consideration. 
			encodedSize += entry.zipFileName[0] * PKG_KEY1 + entry.zipFileName[1] * PKG_KEY2 + entry.zipFileName[2] * PKG_KEY3 + entry.zipFileName[3] * PKG_KEY4;
		}
		file.WriteDWORD(encodedSize);
		dataPos += entry.CompressedSize;
	}


	// write data
	{
		vector<byte> cData;

		for (i = 0; i < nEntryNum; ++i)
		{
			if (m_FileList[i].m_pEntry == nullptr)
				continue;

			SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
			if (cData.size() < entry.CompressedSize)
				cData.resize(entry.CompressedSize);
			if (entry.CompressedSize > 0)
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

bool CZipArchive::GeneratePkgFile(const char* filename)
{
	CParaFile file;
	if (!file.CreateNewFile(filename))
		return false;

	ParaEngine::Lock lock_(m_mutex);

	ReBuild();

	// pkg header
	file.WriteString(".pkg", 4);
	// version number
	file.WriteDWORD(PKG_FILE_VERSION);
	// reserved bytes
	file.WriteDWORD(0); file.WriteDWORD(0); file.WriteDWORD(0); file.WriteDWORD(0);
	// comment
	const char* comments = "ParaEngine Tech Studio Package File.";
	int nLen = (int)strlen(comments);
	file.WriteDWORD(nLen);
	file.WriteString(comments, nLen);
	// center directory
	int nEntryNum = (int)(m_FileList.size());
	int i = 0;
	file.WriteDWORD(nEntryNum);
	DWORD nDirSize = 0;
	{
		// get dir section size
		for (i = 0; i < nEntryNum; ++i)
		{
			SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
			int nFileNameLen = (int)entry.fileNameLen;
			nDirSize += sizeof(WORD) + nFileNameLen + sizeof(WORD) + sizeof(DWORD) * 3;
		}
	}
	DWORD dataPos = (DWORD)file.getPos() + nDirSize;
	for (i = 0; i < nEntryNum; ++i)
	{
		if (m_FileList[i].m_pEntry == nullptr)
			continue;

		SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
		WORD nFileNameLen = (WORD)entry.fileNameLen;
		file.write(&nFileNameLen, sizeof(WORD));
		file.WriteString(entry.zipFileName);
		file.write(&entry.CompressionMethod, sizeof(WORD));
		file.WriteDWORD(entry.CompressedSize);
		file.WriteDWORD(entry.UncompressedSize);
		// encode this size. 
		DWORD encodedSize = dataPos;
		if (nFileNameLen >= 4)
		{
			// take the file name into consideration. 
			encodedSize += entry.zipFileName[0] * PKG_KEY1 + entry.zipFileName[1] * PKG_KEY2 + entry.zipFileName[2] * PKG_KEY3 + entry.zipFileName[3] * PKG_KEY4;
		}
		file.WriteDWORD(encodedSize);
		dataPos += entry.CompressedSize;
	}
	// write data
	{
		vector<byte> cData;

		for (i = 0; i < nEntryNum; ++i)
		{
			if (m_FileList[i].m_pEntry == nullptr)
				continue;

			SZipFileEntry& entry = *(m_FileList[i].m_pEntry);
			if (cData.size() < entry.CompressedSize)
				cData.resize(entry.CompressedSize);
			if (entry.CompressedSize > 0)
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

bool CZipArchive::_ReadEntries_pkg()
{
	char sig[4];
	// header
	m_pFile->read(&sig, 4);
	if ((sig[0] == '.') && (sig[1] == 'p') && (sig[2] == 'k') && (sig[3] == 'g'))
	{
	}
	else
	{
		return false;
	}

	// version
	DWORD fileVersion = 0;
	m_pFile->read(&fileVersion, sizeof(DWORD));
	if (fileVersion == PKG_FILE_VERSION)
	{
		return ReadEntries_pkg();
	}
	else if (fileVersion == PKG_FILE_VERSION2)
	{
		return ReadEntries_pkg2();
	}
	else
	{
		OUTPUT_LOG("ParaEngine PKG archiver file header is incorrect.\r\n");
		return false;
	}
}


bool CZipArchive::ReadEntries_pkg2()
{
	// reserved words
	m_pFile->seek(sizeof(DWORD) * 4, true);

	// comments
	DWORD nLen = 0;
	m_pFile->read(&nLen, sizeof(DWORD));
	m_pFile->seek(nLen, true);

	// Ignore Case
	uint8 ignoreCase;
	m_pFile->read(&ignoreCase, sizeof(uint8));
	m_bIgnoreCase = ignoreCase != 0;

	// number of files
	DWORD nEntryNum = 0;
	m_pFile->read(&nEntryNum, sizeof(DWORD));

	// name block size
	DWORD nameBuffSize = 0;
	m_pFile->read(&nameBuffSize, sizeof(DWORD));
	// name block CompressedSize
	DWORD nameCompressedSize = 0;
	m_pFile->read(&nameCompressedSize, sizeof(DWORD));

	m_FileList.resize(nEntryNum);

	SAFE_DELETE_ARRAY(m_pEntries);
	m_nameBlock.clear();
	m_nameBlock.resize(nameBuffSize);
	m_pEntries = new SZipFileEntry[nEntryNum];

	// read name block
	{
		if (nameCompressedSize == 0)
		{
			m_pFile->read(&m_nameBlock[0], nameBuffSize);
		}
		else
		{
			vector<uint8> compressedData;
			compressedData.resize(nameCompressedSize);
			m_pFile->read(&compressedData[0], nameCompressedSize);

			if (!Decompress(&compressedData[0], nameCompressedSize, &m_nameBlock[0], nameBuffSize))
			{
				OUTPUT_LOG("warning: unable to Decompress name block \n");
				return false;
			}
		}
	}

	for (DWORD i = 0; i < nEntryNum; i++)
	{
		m_FileList[i].m_pEntry = &m_pEntries[i];
		SZipFileEntry& entry = *(m_FileList[i].m_pEntry);

		// read filename
		m_pFile->read(&entry.fileNameLen, sizeof(WORD));

		DWORD nameOffset;
		m_pFile->read(&nameOffset, sizeof(DWORD));
		entry.zipFileName = &m_nameBlock[nameOffset];

		// read hash
		m_pFile->read(&entry.hashValue, sizeof(uint32));


		m_pFile->read(&entry.CompressionMethod, sizeof(WORD));
		m_pFile->read(&entry.CompressedSize, sizeof(DWORD));
		m_pFile->read(&entry.UncompressedSize, sizeof(DWORD));
		int nEncodedDataPos = 0;
		m_pFile->read(&nEncodedDataPos, sizeof(DWORD));
		// decode the data pos
		int nDataPos = nEncodedDataPos;
		if (entry.fileNameLen >= 4)
		{
			// take the file name into consideration. 
			nDataPos -= entry.zipFileName[0] * PKG_KEY1 + entry.zipFileName[1] * PKG_KEY2 + entry.zipFileName[2] * PKG_KEY3 + entry.zipFileName[3] * PKG_KEY4;
		}
		entry.fileDataPosition = nDataPos;
	}

	// pkg already sorted
	m_bDirty = false;

	return true;
}

bool CZipArchive::ReadEntries_pkg()
{
	// reserved words
	m_pFile->seek(sizeof(DWORD) * 4, true);

	// comments
	int nLen = 0;
	m_pFile->read(&nLen, sizeof(DWORD));
	m_pFile->seek(nLen, true);

	// number of files
	int nEntryNum = 0;
	m_pFile->read(&nEntryNum, sizeof(DWORD));

	// OUTPUT_LOG("len: %d, nEntry: %d, sizeof(DWORD)%d, sizeof(int)%d\n", nLen, nEntryNum, sizeof(DWORD), sizeof(int));

	m_FileList.resize(nEntryNum);
	m_bDirty = true;

	SAFE_DELETE_ARRAY(m_pEntries);
	m_nameBlock.clear();
	m_pEntries = new SZipFileEntry[nEntryNum];
	DWORD nameOffset = 0;
	DWORD nameBlockSize = 0;

	// get name block size
	if (nEntryNum > 0)
	{
		m_FileList[0].m_pEntry = &m_pEntries[0];
		SZipFileEntry& entry = *(m_FileList[0].m_pEntry);

		auto pos = m_pFile->getPos();

		WORD nNameSize = 0;
		m_pFile->read(&nNameSize, sizeof(WORD));
		m_pFile->seek(nNameSize + sizeof(WORD) + sizeof(DWORD) * 2, true);

		int nEncodedDataPos = 0;
		m_pFile->read(&nEncodedDataPos, sizeof(DWORD));

		if (nNameSize >= 4)
		{
			m_pFile->seek(pos + sizeof(WORD), false);
			char tmp[4];
			m_pFile->read(tmp, 4);
			nEncodedDataPos -= tmp[0] * PKG_KEY1 + tmp[1] * PKG_KEY2 + tmp[2] * PKG_KEY3 + tmp[3] * PKG_KEY4;
		}
		m_pFile->seek(pos, false);
		const DWORD entrySize = sizeof(WORD) + sizeof(WORD) + sizeof(DWORD) * 3;
		const DWORD headSize = 4 + sizeof(DWORD) + sizeof(DWORD) * 4 + sizeof(DWORD) + nLen + sizeof(DWORD);

		DWORD size = nEncodedDataPos - headSize - (entrySize - 1) * nEntryNum;
		m_nameBlock.resize(size);
	}


	for (int i = 0; i < nEntryNum; ++i)
	{
		m_FileList[i].m_pEntry = &m_pEntries[i];
		SZipFileEntry& entry = *(m_FileList[i].m_pEntry);

		// read filename
		WORD nNameSize = 0;
		m_pFile->read(&nNameSize, sizeof(WORD));

		nameBlockSize = nameOffset + nNameSize + 1;
		m_pFile->read(&m_nameBlock[nameOffset], nNameSize);
		m_nameBlock[nameBlockSize - 1] = 0;

		entry.zipFileName = &m_nameBlock[nameOffset];
		entry.fileNameLen = nNameSize;
		entry.RefreshHash(m_bIgnoreCase);
		nameOffset += nNameSize + 1;

		m_pFile->read(&entry.CompressionMethod, sizeof(WORD));
		m_pFile->read(&entry.CompressedSize, sizeof(DWORD));
		m_pFile->read(&entry.UncompressedSize, sizeof(DWORD));
		int nEncodedDataPos = 0;
		m_pFile->read(&nEncodedDataPos, sizeof(DWORD));
		// decode the data pos
		int nDataPos = nEncodedDataPos;
		if (nNameSize >= 4)
		{
			// take the file name into consideration. 
			nDataPos -= entry.zipFileName[0] * PKG_KEY1 + entry.zipFileName[1] * PKG_KEY2 + entry.zipFileName[2] * PKG_KEY3 + entry.zipFileName[3] * PKG_KEY4;
		}
		entry.fileDataPosition = nDataPos;
	}


	return true;
}

void CZipArchive::Close()
{
	if (m_pEntries != 0)
	{
		SAFE_DELETE_ARRAY(m_pEntries);
	}
	else
	{
		int nLen = m_FileList.size();
		for (int i = 0; i < nLen; ++i)
			delete m_FileList[i].m_pEntry;
	}
	m_FileList.clear();
	m_nameBlock.clear();

	if (m_bOpened)
	{
		SAFE_DELETE(m_pFile);
	}
	SAFE_DELETE_ARRAY(m_zipComment);
	m_bOpened = false;
}

bool CZipArchive::DoesFileExist(const string& filename)
{
	ArchiveFileFindItem item(filename.c_str());
	return findFile(&item) >= 0;
}

int CZipArchive::findFile(const ArchiveFileFindItem* item)
{
	const char* filename = item->filename;
	bool bRefreshHash = false;

	if (!m_bRelativePath)
	{

	}
	else
	{
		int nSize = (int)m_sRootPath.size();
		int i = 0;
		for (; i < nSize && (m_sRootPath[i] == item->filename[i]); ++i)
		{
		}

		if (i == nSize)
		{
			filename += i;
			bRefreshHash = true;
		}
		else
			return -1; // return file not found
	}


	char tmp[MAX_PATH_LENGTH + 1];
	if (m_bIgnoreCase)
	{
		size_t i = 0;
		for (; filename[i] != 0 && i < MAX_PATH_LENGTH; i++)
		{
			tmp[i] = filename[i];
			if (tmp[i] >= 'A' && tmp[i] <= 'Z')
				tmp[i] += 'a' - 'A';
		}
		tmp[i] = 0;

		filename = tmp;
	}

	ReBuild();

	int nIndex = findFileImp(item, filename, bRefreshHash);
	if (nIndex < 0 && !m_fileAliasMap.empty())
	{
		const std::string* aliasFilename = GetAlias(filename);
		if (aliasFilename != nullptr)
		{
			nIndex = findFileImp(item, aliasFilename->c_str(), true);
		}
	}
	return nIndex;
}

int ParaEngine::CZipArchive::findFileImp(const ArchiveFileFindItem* item, const char* filename, bool bRefreshHash)
{
	//auto hash = (hashValue == nullptr) ? (SZipFileEntry::Hash(filename, false)): *hashValue;
	uint32 hash;
	if (bRefreshHash)
		hash = SZipFileEntry::Hash(filename, false);
	else
	{
		if (m_bIgnoreCase)
		{
			hash = item->hashlower ? *item->hashlower : SZipFileEntry::Hash(filename, false);
		}
		else
		{
			hash = item->hashValue ? *item->hashValue : SZipFileEntry::Hash(filename, false);
		}
	}

	auto it = std::lower_bound(m_FileList.begin(), m_FileList.end(), hash, [](const SZipFileEntryPtr& a, const uint32& hash)
	{
		return a.m_pEntry->hashValue < hash;
	});

	if (m_bIgnoreCase)
	{
		for (; it != m_FileList.end() && it->m_pEntry->hashValue == hash; it++)
		{
			auto& entry = *(it->m_pEntry);
			bool eq = true;
			for (size_t i = 0; i < entry.fileNameLen && filename[i] != 0; i++)
			{
				auto c = entry.zipFileName[i];
				if (c >= 'A' && c <= 'Z')
					c += 'a' - 'A';

				if (filename[i] != c)
				{
					eq = false;
					break;
				}
			}

			if (eq)
			{
				return it - m_FileList.begin();
			}
		}
	}
	else
	{
		for (; it != m_FileList.end() && it->m_pEntry->hashValue == hash; it++)
		{
			if (strcmp(it->m_pEntry->zipFileName, filename) == 0)
			{
				return it - m_FileList.begin();
			}
		}
	}

	return -1;
}



bool CZipArchive::OpenFile(const ArchiveFileFindItem* item, FileHandle& handle)
{
	handle.m_index = findFile(item);

	bool bRes = (handle.m_index != -1);
	if (bRes)
		handle.m_pArchive = this;
	return bRes;
}

bool CZipArchive::OpenFile(const char* filename, FileHandle& handle)
{
	ArchiveFileFindItem item(filename);
	return OpenFile(&item, handle);
}

DWORD CZipArchive::GetFileSize(FileHandle& handle)
{
	if (handle.m_index != -1)
	{
		return m_FileList[handle.m_index].m_pEntry->UncompressedSize;
	}
	return 0;
}

string CZipArchive::GetNameInArchive(FileHandle& handle)
{
	if (handle.m_index != -1)
	{
		return m_FileList[handle.m_index].m_pEntry->zipFileName;
	}
	return 0;
}

string CZipArchive::GetOriginalNameInArchive(FileHandle& handle)
{
	if (handle.m_index != -1)
	{
		return m_FileList[handle.m_index].m_pEntry->zipFileNameOriginal;
	}
	return 0;
}

bool CZipArchive::ReadFileRaw(FileHandle& handle, LPVOID* lppBuffer, LPDWORD pnCompressedSize, LPDWORD pnUncompressedSize)
{
	ParaEngine::Lock lock_(m_mutex);

	DWORD nBytesRead = 0;
	int index = handle.m_index;

	(*lppBuffer) = 0;
	bool res = false;
	WORD compressMethod = m_FileList[index].m_pEntry->CompressionMethod;
	switch (compressMethod)
	{
	case 0: // no compression
	case 8: // zip compressed data
	{
		DWORD uncompressedSize = m_FileList[index].m_pEntry->UncompressedSize;
		DWORD compressedSize = m_FileList[index].m_pEntry->CompressedSize;
		if (pnCompressedSize)
			*pnCompressedSize = compressedSize;
		if (pnUncompressedSize)
			*pnUncompressedSize = uncompressedSize;
		if (uncompressedSize == 0)
		{
			return true;
		}
		(*lppBuffer) = new byte[compressedSize];
		if (!(*lppBuffer))
		{
			OUTPUT_LOG("Not enough memory for reading archive file %s\n", m_FileList[index].m_pEntry->zipFileName);
			return false;
		}

		m_pFile->seek(m_FileList[index].m_pEntry->fileDataPosition);
		nBytesRead = m_pFile->read(*lppBuffer, compressedSize);
		res = true;
		if (compressMethod == 0)
			*pnUncompressedSize = 0;
		break;
	}
	default:
		OUTPUT_LOG("warning: file %s has unsupported compression method: \n", m_FileList[index].m_pEntry->zipFileName);
		break;
	};
	return res;
}

bool CZipArchive::Decompress(LPVOID lpCompressedBuffer, DWORD nCompressedSize, LPVOID lpUnCompressedBuffer, DWORD nUncompressedSize)
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
bool CZipArchive::ReadFile(FileHandle& handle, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPDWORD lpLastWriteTime)
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
	DWORD nBytesRead = 0;
	int index = handle.m_index;
	switch (m_FileList[index].m_pEntry->CompressionMethod)
	{
	case 0: // no compression
	{
		m_pFile->seek(m_FileList[index].m_pEntry->fileDataPosition);
		nBytesRead = m_pFile->read(lpBuffer, nNumberOfBytesToRead);
		if (lpNumberOfBytesRead)
			*lpNumberOfBytesRead = nBytesRead;

		//break;
		return true;
	}
	case 8:
	{
#ifdef COMPILE_WITH_ZLIB

		DWORD uncompressedSize = m_FileList[index].m_pEntry->UncompressedSize;
		DWORD compressedSize = m_FileList[index].m_pEntry->CompressedSize;
		DWORD lastWriteTime = m_FileList[index].m_pEntry->LastModifiedTime;

		void* pBuf = lpBuffer;
		bool bCopyBuffer = false;
		if (nNumberOfBytesToRead < uncompressedSize)
		{
			pBuf = new byte[uncompressedSize];
			if (!pBuf)
			{
				OUTPUT_LOG("Not enough memory for decompressing %s\n", m_FileList[index].m_pEntry->zipFileName);
				return false;
			}
			bCopyBuffer = true;
		}

		byte* pcData = new byte[compressedSize];
		if (pcData == 0)
		{
			OUTPUT_LOG("Not enough memory for decompressing %s\n", m_FileList[index].m_pEntry->zipFileName);
			return false;
		}

		//memset(pcData, 0, compressedSize );
		m_pFile->seek(m_FileList[index].m_pEntry->fileDataPosition);
		m_pFile->read(pcData, compressedSize);

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
		delete[] pcData;

		if (err == Z_OK)
		{
			if (lpNumberOfBytesRead)
				(*lpNumberOfBytesRead) = nNumberOfBytesToRead;
			if (lpLastWriteTime)
				(*lpLastWriteTime) = lastWriteTime;
			if (bCopyBuffer)
			{
				memcpy(lpBuffer, pBuf, nNumberOfBytesToRead);
				delete[](byte*)pBuf;
			}
			return true;
		}
		else
		{
			OUTPUT_LOG("Error decompressing %s\n", m_FileList[index].m_pEntry->zipFileName);
			if (bCopyBuffer)
				delete[](byte*)pBuf;
			return false;
		}

#else
		return 0; // zlib not compiled, we cannot decompress the data.
#endif
	}
	break;
	default:
		OUTPUT_LOG("file %s has unsupported compression method: \n", m_FileList[index].m_pEntry->zipFileName);
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
	if (entryPtr.m_pEntry == 0)
		return false;
	SZipFileEntry& entry = *entryPtr.m_pEntry;

	m_pFile->read(&entry.header, sizeof(SZIPFileHeader));

	if (entry.header.Sig != 0x04034b50)
		return false; // local file headers end here.

	// read filename
	entry.zipFileName.reserve(entry.header.FilenameLength + 2);
	m_pFile->read(tmp, entry.header.FilenameLength);
	tmp[entry.header.FilenameLength] = 0x0;

	entry.zipFileName = tmp;
	//if (m_bIgnoreCase)
	//	StringHelper::make_lower(entry.zipFileName);
	entry.RefreshHash(m_bIgnoreCase);

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
	if (nTempBufSize < 4)
		return -1;
	pos = -1;
	byte* pBuf = new byte[nTempBufSize];
	if (m_pFile->read(pBuf, nTempBufSize) == nTempBufSize)
	{
		for (int i = nTempBufSize - 4; i >= 0; --i)
		{
			DWORD nSig_LittleEndian = *(DWORD*)(pBuf + i);
			if (nSig_LittleEndian == signature)
			{
				pos = giveUpMarker + (i + 4);
				m_pFile->seek(pos, false);
				break;
			}
		}
		delete[]pBuf;
	}
	return pos;
}

bool CZipArchive::ReadEntries()
{
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

	auto EndOfCentralDirPos = m_pFile->getPos();

	ZIP_EndOfCentralDirectory EndOfCentralDir;
	m_pFile->read(&EndOfCentralDir, sizeof(ZIP_EndOfCentralDirectory));

	if (EndOfCentralDir.commentSize > 0)
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

	auto CentralDirPos = offsetOfFirstEntry + EndOfCentralDir.offsetOfCentralDir;

	m_pFile->seek(CentralDirPos, false);

	IReadFile* pReader = NULL;

#ifdef ENABLE_INDEX_CACHE
	CMemReadFile memfile(m_pFile, locatedCentralDirOffset - m_pFile->getPos());
	pReader = &memfile;
#else
	pReader = m_pFile;
#endif

	int nEntryNum = EndOfCentralDir.entriesForThisDisk;
	m_FileList.clear();


	m_FileList.resize(nEntryNum);
	m_bDirty = true;

	SAFE_DELETE_ARRAY(m_pEntries);
	m_nameBlock.clear();

	// get name block size
	{
		auto nameBlockSize = EndOfCentralDirPos - CentralDirPos - sizeof(ZIP_CentralDirectory) * nEntryNum + nEntryNum;
		m_nameBlock.resize(nameBlockSize);
	}
	m_pEntries = new SZipFileEntry[nEntryNum];
	DWORD nameOffset = 0;
	DWORD nameBlockSize = 0;


	for (int i = 0; i < nEntryNum; ++i)
	{
		ZIP_CentralDirectory CentralDir;
		pReader->read(&CentralDir, sizeof(ZIP_CentralDirectory));
		if (CentralDir.Sig != ZIP_CONST_CENSIG)
		{
			return false; // throw new ZipException("Wrong Central Directory signature");
		}

		// read filename
		nameBlockSize = nameOffset + CentralDir.NameSize + 1;
		pReader->read(&m_nameBlock[nameOffset], CentralDir.NameSize);
		m_nameBlock[nameBlockSize - 1] = 0;

		m_FileList[i].m_pEntry = &m_pEntries[i];
		SZipFileEntry& entry = *(m_FileList[i].m_pEntry);

		entry.zipFileName = &m_nameBlock[nameOffset];
		entry.zipFileNameOriginal = &m_nameBlock[nameOffset];
		entry.fileNameLen = CentralDir.NameSize;
		entry.RefreshHash(m_bIgnoreCase);
		nameOffset += CentralDir.NameSize + 1;


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
		entry.LastModifiedTime = (CentralDir.LastModFileDate << 16) + (CentralDir.LastModFileTime & 0xffff);

		int nExtraLength = 0;

		/**
		* zip align ensures that all uncompressed files in the archive are aligned relative to the start of the file. This allows those files to be accessed directly via mmap(2), removing the need to copy this data in RAM and reducing your app's memory usage.
		* The alignment is achieved by altering the size of the "extra" field in the zip Local File Header sections. Existing data in the "extra" fields may be altered by this process.
		*/
		//if (CentralDir.ExtraSize > 0) 
		{
			pReader->seek(CentralDir.ExtraSize, true);
			DWORD oldPos = pReader->getPos();
			pReader->seek(CentralDir.LocalHeaderOffset, false);
			SZIPFileHeader header;
			pReader->read(&header, sizeof(SZIPFileHeader));
			nExtraLength = header.ExtraFieldLength;
			pReader->seek(oldPos, false);
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

	if (sPattern.size() > 1 && (sPattern[0] == ':'))
	{
		// search using regular expression if the pattern begins with ":", things after ":" is treated like a regular expression. 
		string sFilePattern = sRootPath;
		if (sRootPath.size() > 0)
		{
			char lastChar = sRootPath[sRootPath.size() - 1];
			if (lastChar != '\\' && lastChar != '/')
			{
				sFilePattern += "/";
			}
		}
		sFilePattern += sPattern.substr(1);

		regex re(sFilePattern);

		int nSize = m_FileList.size();
		for (int index = 0; index < nSize; ++index)
		{
			const SZipFileEntryPtr& entry = m_FileList[index];
			if (regex_search(entry.m_pEntry->zipFileName, re))
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
		if (sRootPath.size() > 0)
		{
			char lastChar = sRootPath[sRootPath.size() - 1];
			if (lastChar != '\\' && lastChar != '/')
			{
				sFilePattern += "/";
			}
		}

		int remove_head_len = 0;
		if (m_bRelativePath)
		{
			int nSize = (int)m_sRootPath.size();
			int i = 0;
			for (; i < nSize && (m_sRootPath[i] == sFilePattern[i]); ++i)
			{
			}
			if (i == nSize)
			{
				if (sFilePattern.size() == nSize)
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
		if (nDestCount == 0)
			return;
		int nSize = m_FileList.size();

		if (nSubLevel > 0)
		{
			OUTPUT_LOG("warning: CZipArchive::FindFiles only support nSubLevel=0 when using wild card search. Hence sub folders are ignored. One can also use regular expression search just begin the search pattern with \":\" like \":.*jpg\" \n");
		}

		if (m_bIgnoreCase)
			StringHelper::make_lower(sFilePattern);

		for (int index = 0; index < nSize; ++index)
		{
			const SZipFileEntryPtr& entry = m_FileList[index];
			std::string filename = entry.m_pEntry->zipFileName;

			if (m_bIgnoreCase)
				StringHelper::make_lower(filename);

			int nCount = (int)filename.size();
			int j = 0;
			int i = 0;
			for (i = 0; i < nCount;)
			{
				char srcChar = filename[i];
				char destChar = sFilePattern[j];
				if (destChar == srcChar)
				{
					++i;
					++j;
					if (j == nDestCount)
					{
						break;
					}
				}
				else if (destChar == '*')
				{
					if (srcChar == '/' || srcChar == '\\')
					{
						++j;
						if (j == nDestCount)
						{
							if ((i + 1) < nCount)
							{
								// this is a folder match, add folder with 0 size.  
								//std::string filename = entry.m_pEntry->zipFileName.substr(remove_head_len, i - remove_head_len);
								std::string filename(entry.m_pEntry->zipFileName + remove_head_len, i - remove_head_len);
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
						if (i == nCount && j == (nDestCount - 1))
						{ // a perfect match for * at the end
							++j;
							break;
						}
					}
				}
				else
				{
					if (destChar == '.')
					{
						++j;
						++i;
					}
					break;
				}
			}
			if (i == nCount && j == nDestCount)
			{
				if (remove_head_len != 0)
				{
					//std::string filename = entry.m_pEntry->zipFileName.substr(remove_head_len);	
					const char* filename = entry.m_pEntry->zipFileName + remove_head_len;
					result.AddResult(filename, entry.m_pEntry->CompressedSize);
				}
				else
					result.AddResult(entry.m_pEntry->zipFileName, entry.m_pEntry->CompressedSize);
			}
		}
	}

}

const std::string& ParaEngine::CZipArchive::GetRootDirectory()
{
	return m_sRootPath;
}

void ParaEngine::CZipArchive::SetBaseDirectory(const char* sBaseDir_)
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
			std::string filename = pEntry->zipFileName;
			if (m_bIgnoreCase)
				StringHelper::make_lower(filename);

			if ((int)filename.size() > nBaseSize &&
				filename.compare(0, nBaseSize, sBaseDir) == 0)
			{
				//pEntry->zipFileName = pEntry->zipFileName.substr(nBaseSize);
				pEntry->zipFileName += nBaseSize;
				pEntry->fileNameLen -= nBaseSize;
				pEntry->RefreshHash(m_bIgnoreCase);
			}
		}

		m_bDirty = true;
	}

}

static std::string tmp_alias_from;
void ParaEngine::CZipArchive::AddAliasFrom(const char* from)
{
	tmp_alias_from = from;
}

void ParaEngine::CZipArchive::AddAliasTo(const char* to)
{
	if (!tmp_alias_from.empty() && to)
	{
		AddAlias(tmp_alias_from, to);
	}
}

void ParaEngine::CZipArchive::AddAlias(const std::string& from, const std::string& to)
{
	m_fileAliasMap[from] = to;
}

const std::string* ParaEngine::CZipArchive::GetAlias(const std::string& from)
{
	if (!m_fileAliasMap.empty()) {
		auto it = m_fileAliasMap.find(from);
		if (it != m_fileAliasMap.end())
		{
			return &(it->second);
		}
	}
	return nullptr;
}

int ParaEngine::CZipArchive::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CArchive::InstallFields(pClass, bOverride);

	pClass->AddField("SetBaseDirectory", FieldType_String, (void*)SetBaseDirectory_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("RootDirectory", FieldType_String, (void*)SetRootDirectory_s, (void*)GetRootDirectory_s, NULL, NULL, bOverride);
	pClass->AddField("GeneratePkgFileV1", FieldType_String, (void*)GeneratePkgFileV1_s, (void*)0, NULL, NULL, bOverride);
	pClass->AddField("GeneratePkgFileV2", FieldType_String, (void*)GeneratePkgFileV2_s, (void*)0, NULL, NULL, bOverride);
	pClass->AddField("FileCount", FieldType_Int, (void*)0, (void*)GetFileCount_s, NULL, NULL, bOverride);
	pClass->AddField("IsIgnoreCase", FieldType_Bool, (void*)0, (void*)IsIgnoreCase_s, NULL, NULL, bOverride);
	pClass->AddField("AddAliasFrom", FieldType_String, (void*)AddAliasFrom_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("AddAliasTo", FieldType_String, (void*)AddAliasTo_s, NULL, NULL, NULL, bOverride);
	return S_OK;
}
