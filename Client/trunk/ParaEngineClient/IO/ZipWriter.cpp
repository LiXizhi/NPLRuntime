//-----------------------------------------------------------------------------
// Class: ZipWriter
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.10
// Revised: 2016.4.24
// Notes: 
//-----------------------------------------------------------------------------
#ifdef ZIP_STD
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <windows.h>
#include <tchar.h>
#include <ctype.h>
#include <stdio.h>
#endif

#include "ParaEngine.h"
#include "ZipWriter.h"
#include "MemReadFile.h"
#include "FileUtils.h"
#include "ZipArchive.h"
#include "NPLCodec.h"
#include "zlib.h"

#ifdef USE_ZIPLIB
#include "zlib/zip.h"
#endif

using namespace ParaEngine;

namespace ParaEngine 
{

	// ----------------------------------------------------------------------
	// some windows<->linux portability things
#ifdef ZIP_STD
	void filetime2dosdatetime(const FILETIME ft, WORD *dosdate, WORD *dostime)
	{
		struct tm *st = gmtime(&ft);
		*dosdate = (ush)(((st->tm_year + 1900 - 1980) & 0x7f) << 9);
		*dosdate |= (ush)((st->tm_mon & 0xf) << 5);
		*dosdate |= (ush)((st->tm_mday & 0x1f));
		*dostime = (ush)((st->tm_hour & 0x1f) << 11);
		*dostime |= (ush)((st->tm_min & 0x3f) << 5);
		*dostime |= (ush)((st->tm_sec * 2) & 0x1f);
	}

	void GetNow(lutime_t *ft, WORD *dosdate, WORD *dostime)
	{
		time_t tm = time(0);
		filetime2dosdatetime(tm, dosdate, dostime);
		*ft = (lutime_t)tm;
	}

	DWORD GetFilePosZ(HANDLE hfout)
	{
		struct stat st; fstat(fileno(hfout), &st);
		if ((st.st_mode&S_IFREG) == 0) return 0xFFFFFFFF;
		return ftell(hfout);
	}

	ZRESULT GetFileInfo(FILE *hf, ulg *attr, long *size, iztimes *times, ulg *timestamp)
	{ // The handle must be a handle to a file
		// The date and time is returned in a long with the date most significant to allow
		// unsigned integer comparison of absolute times. The attributes have two
		// high bytes unix attr, and two low bytes a mapping of that to DOS attr.
		struct stat bhi; int res = fstat(fileno(hf), &bhi); if (res == -1) return ZR_NOFILE;
		ulg fa = bhi.st_mode; ulg a = 0;
		// Zip uses the lower word for its interpretation of windows stuff
		if ((fa&S_IWUSR) == 0) a |= 0x01;
		if (S_ISDIR(fa)) a |= 0x10;
		// It uses the upper word for standard unix attr
		a |= ((fa & 0xFFFF) << 16);
		//
		if (attr != NULL) *attr = a;
		if (size != NULL) *size = bhi.st_size;
		if (times != NULL)
		{
			times->atime = (lutime_t)bhi.st_atime;
			times->mtime = (lutime_t)bhi.st_mtime;
			times->ctime = (lutime_t)bhi.st_ctime;
		}
		if (timestamp != NULL)
		{
			ush dosdate, dostime;
			filetime2dosdatetime(bhi.st_mtime, &dosdate, &dostime);
			*timestamp = (ush)dostime | (((ulg)dosdate) << 16);
		}
		return ZR_OK;
	}


	// ----------------------------------------------------------------------
#else
	void filetime2dosdatetime(const FILETIME ft, WORD *dosdate, WORD *dostime)
	{ // date: bits 0-4 are day of month 1-31. Bits 5-8 are month 1..12. Bits 9-15 are year-1980
		// time: bits 0-4 are seconds/2, bits 5-10 are minute 0..59. Bits 11-15 are hour 0..23
		SYSTEMTIME st; FileTimeToSystemTime(&ft, &st);
		*dosdate = (WORD)(((st.wYear - 1980) & 0x7f) << 9);
		*dosdate |= (WORD)((st.wMonth & 0xf) << 5);
		*dosdate |= (WORD)((st.wDay & 0x1f));
		*dostime = (WORD)((st.wHour & 0x1f) << 11);
		*dostime |= (WORD)((st.wMinute & 0x3f) << 5);
		*dostime |= (WORD)((st.wSecond * 2) & 0x1f);
	}

	lutime_t filetime2timet(const FILETIME ft)
	{
		LONGLONG i = *(LONGLONG*)&ft;
		return (lutime_t)((i - 116444736000000000LL) / 10000000LL);
	}

	void GetNow(lutime_t *pft, WORD *dosdate, WORD *dostime)
	{
		SYSTEMTIME st; GetLocalTime(&st);
		FILETIME ft;   SystemTimeToFileTime(&st, &ft);
		filetime2dosdatetime(ft, dosdate, dostime);
		*pft = filetime2timet(ft);
	}

	DWORD GetFilePosZ(HANDLE hfout)
	{
		return SetFilePointer(hfout, 0, 0, FILE_CURRENT);
	}


	ZRESULT GetFileInfo(HANDLE hf, ulg *attr, long *size, iztimes *times, ulg *timestamp)
	{ // The handle must be a handle to a file
		// The date and time is returned in a long with the date most significant to allow
		// unsigned integer comparison of absolute times. The attributes have two
		// high bytes unix attr, and two low bytes a mapping of that to DOS attr.
		//struct stat s; int res=stat(fn,&s); if (res!=0) return false;
		// translate windows file attributes into zip ones.
		BY_HANDLE_FILE_INFORMATION bhi; BOOL res = GetFileInformationByHandle(hf, &bhi);
		if (!res) return ZR_NOFILE;
		DWORD fa = bhi.dwFileAttributes; ulg a = 0;
		// Zip uses the lower word for its interpretation of windows stuff
		if (fa&FILE_ATTRIBUTE_READONLY) a |= 0x01;
		if (fa&FILE_ATTRIBUTE_HIDDEN)   a |= 0x02;
		if (fa&FILE_ATTRIBUTE_SYSTEM)   a |= 0x04;
		if (fa&FILE_ATTRIBUTE_DIRECTORY)a |= 0x10;
		if (fa&FILE_ATTRIBUTE_ARCHIVE)  a |= 0x20;
		// It uses the upper word for standard unix attr, which we manually construct
		if (fa&FILE_ATTRIBUTE_DIRECTORY)a |= 0x40000000;  // directory
		else a |= 0x80000000;  // normal file
		a |= 0x01000000;      // readable
		if (fa&FILE_ATTRIBUTE_READONLY) {}
		else a |= 0x00800000; // writeable
		// now just a small heuristic to check if it's an executable:
		DWORD red, hsize = GetFileSize(hf, NULL); if (hsize > 40)
		{
			SetFilePointer(hf, 0, NULL, FILE_BEGIN); unsigned short magic; ReadFile(hf, &magic, sizeof(magic), &red, NULL);
			SetFilePointer(hf, 36, NULL, FILE_BEGIN); unsigned long hpos;  ReadFile(hf, &hpos, sizeof(hpos), &red, NULL);
			if (magic == 0x54AD && hsize > hpos + 4 + 20 + 28)
			{
				SetFilePointer(hf, hpos, NULL, FILE_BEGIN); unsigned long signature; ReadFile(hf, &signature, sizeof(signature), &red, NULL);
				if (signature == IMAGE_DOS_SIGNATURE || signature == IMAGE_OS2_SIGNATURE
					|| signature == IMAGE_OS2_SIGNATURE_LE || signature == IMAGE_NT_SIGNATURE)
				{
					a |= 0x00400000; // executable
				}
			}
		}
		//
		if (attr != NULL) *attr = a;
		if (size != NULL) *size = hsize;
		if (times != NULL)
		{ // lutime_t is 32bit number of seconds elapsed since 0:0:0GMT, Jan1, 1970.
			// but FILETIME is 64bit number of 100-nanosecs since Jan1, 1601
			times->atime = filetime2timet(bhi.ftLastAccessTime);
			times->mtime = filetime2timet(bhi.ftLastWriteTime);
			times->ctime = filetime2timet(bhi.ftCreationTime);
		}
		if (timestamp != NULL)
		{
			WORD dosdate, dostime;
			filetime2dosdatetime(bhi.ftLastWriteTime, &dosdate, &dostime);
			*timestamp = (WORD)dostime | (((DWORD)dosdate) << 16);
		}
		return ZR_OK;
	}
#endif

	/** a single file in zip archive to be written to disk */
	class ZipArchiveEntry : public CRefCounted 
	{
	public:
		ZipArchiveEntry() : m_offsetOfCompressedData(0), m_offsetOfSerializedLocalFileHeader(0)
		{
			memset(&m_localFileHeader, 0, sizeof(m_localFileHeader));
			//SignatureConstant = 0x04034b50,
			//DataDescriptorSignature = 0x08074b50
			// m_localFileHeader.Sig = 0x08074b50;
			m_localFileHeader.Sig = ZIP_CONST_LOCALHEADERSIG;
		}

		virtual ~ZipArchiveEntry() 
		{
		}
	public:

		/**@def CHUNK is simply the buffer size for feeding data to and pulling data from the zlib routines.
		Larger buffer sizes would be more efficient, especially for inflate(). If the memory is available,
		buffers sizes on the order of 128K or 256K bytes should be used. */
		static const int NPL_ZLIB_CHUNK = 32768;

		/** compress without zip header*/
		static int Compress(std::string& outstring, const char* src, int nSrcSize, int compressionlevel = Z_DEFAULT_COMPRESSION)
		{
			z_stream zs;
			memset(&zs, 0, sizeof(zs));

			if (deflateInit2(&zs, compressionlevel, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK)
			{
				OUTPUT_LOG("warning: NPLCodec::Compress deflateInit failed while compressing.\n");
				return -1;
			}

			zs.next_in = (Bytef*)src;
			// set the z_stream's input
			zs.avail_in = nSrcSize;

			int ret;
			char outbuffer[NPL_ZLIB_CHUNK];
			// retrieve the compressed bytes blockwise
			do {
				zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
				zs.avail_out = sizeof(outbuffer);

				ret = deflate(&zs, Z_FINISH);

				if (outstring.size() < zs.total_out) {
					// append the block to the output string
					outstring.append(outbuffer,
						zs.total_out - outstring.size());
				}
			} while (ret == Z_OK);

			deflateEnd(&zs);

			if (ret != Z_STREAM_END) {
				OUTPUT_LOG("warning: NPLCodec::Compress failed an error occurred that was not EOF.\n");
				return -1;
			}
			return 1;
		}

		/**
		* @param destFilename: filename in zip file
		* @param filename: filename in system. if empty, it means destFilename is a directory. 
		*/
		void Init(const char* destFilename, const char* filename = NULL)
		{
			m_destFilename = destFilename;
			if (filename) {
				m_filename = filename;
			}
			m_localFileHeader.FilenameLength = static_cast<uint16_t>(m_destFilename.length());
		}

		void SerializeLocalFileHeader(CParaFile& file) 
		{
			// save offset of stream here
			m_offsetOfSerializedLocalFileHeader = file.getPos();

			m_localFileHeader.DataDescriptor.CompressedSize = 0;
			m_localFileHeader.DataDescriptor.UncompressedSize = 0;
			m_localFileHeader.DataDescriptor.CRC32 = 0;
			
			// serialize header
			file.write(&m_localFileHeader, sizeof(m_localFileHeader));
			file.WriteString(m_destFilename);

			m_offsetOfCompressedData = file.getPos();
			// serialize body of compressed file
			if (!IsDirectory()) 
			{
				CMemReadFile input(m_filename.c_str());
				if (input.isOpen())
				{
					std::string output;
					output.reserve(input.getSize());
					if (Compress(output, (const char*)input.getBuffer(), input.getSize()) == 1) 
					{
						m_localFileHeader.CompressionMethod = 8; // 8 for zip, 0 for no compression.
						m_localFileHeader.DataDescriptor.UncompressedSize = input.getSize();
						m_localFileHeader.DataDescriptor.CompressedSize = output.size();
						uint32_t crc = 0;
						m_localFileHeader.DataDescriptor.CRC32 = crc32(crc, input.getBuffer(), input.getSize());
						file.WriteString(output);

						// actualize local file header
						auto currentPos = file.getPos();
						file.seek(m_offsetOfSerializedLocalFileHeader);
						file.write(&m_localFileHeader, sizeof(m_localFileHeader));
						file.seek(currentPos);
					}
				}
				else 
				{
					OUTPUT_LOG("warning: failed to add file: %s to zip archive\n", m_filename.c_str());
				}
			}
		};
		void SerializeCentralDirectoryFileHeader(CParaFile& file)
		{
			ZIP_CentralDirectory _centralDirectoryFileHeader;
			memset(&_centralDirectoryFileHeader, 0, sizeof(_centralDirectoryFileHeader));
			_centralDirectoryFileHeader.Sig = ZIP_CONST_CENSIG;
			_centralDirectoryFileHeader.LastModFileTime = m_localFileHeader.LastModFileTime;
			_centralDirectoryFileHeader.LastModFileDate = m_localFileHeader.LastModFileDate;
			_centralDirectoryFileHeader.FileCRC = m_localFileHeader.DataDescriptor.CRC32;
			_centralDirectoryFileHeader.CompressionMethod = m_localFileHeader.CompressionMethod;
			_centralDirectoryFileHeader.PackSize = m_localFileHeader.DataDescriptor.CompressedSize;
			_centralDirectoryFileHeader.UnPackSize = m_localFileHeader.DataDescriptor.UncompressedSize;
			_centralDirectoryFileHeader.NameSize = m_localFileHeader.FilenameLength;

			_centralDirectoryFileHeader.LocalHeaderOffset = static_cast<int32_t>(m_offsetOfSerializedLocalFileHeader);

			file.write(&_centralDirectoryFileHeader, sizeof(_centralDirectoryFileHeader));
			file.WriteString(m_destFilename);
		};

		bool IsDirectory() { return m_filename.empty(); };
	public:
		SZIPFileHeader m_localFileHeader;
		size_t m_offsetOfCompressedData;
		size_t m_offsetOfSerializedLocalFileHeader;
		std::string m_destFilename;
		std::string m_filename;
	};
}

/////////////////////////////////////////////
///
/// CZipWriter
///
/////////////////////////////////////////////

CZipWriter::CZipWriter()
	:m_handle(NULL)
{

}

CZipWriter::CZipWriter(void* handle)
	: m_handle(handle)
{
}

ParaEngine::CZipWriter::~CZipWriter()
{
	close();
}

CZipWriter* CZipWriter::CreateZip(const char *fn, const char *password)
{
	CZipWriter* zipWriter = new CZipWriter();
	zipWriter->InitNewZip(fn, password);
	return zipWriter;
}

bool CZipWriter::IsValid() 
{
#ifdef USE_ZIPLIB
	return m_handle != 0;
#else
	return true;
#endif
}

DWORD CZipWriter::close()
{
#ifdef USE_ZIPLIB
	DWORD result = ::CloseZip((HZIP)m_handle);
	m_handle = 0;
	return result;
#else
	SaveAndClose();
	return 0;
#endif
}

void ParaEngine::CZipWriter::InitNewZip(const char * filename, const char * password)
{
#ifdef USE_ZIPLIB
	if(m_handle == 0)
		m_handle = new CZipWriter(::CreateZip(filename, password));
#else
	m_filename = filename;
	m_password = password ? password : "";
#endif
}

DWORD CZipWriter::ZipAdd(const char* destFilename, const char* filename)
{
#ifdef USE_ZIPLIB
	return ::ZipAdd((HZIP)m_handle, destFilename, filename);
#else
	auto* pEntry = new ZipArchiveEntry();
	pEntry->Init(destFilename, filename);
	m_entries.push_back(pEntry);
	return 0;
#endif
}

DWORD CZipWriter::ZipAddFolder(const char* destFilename)
{
#ifdef USE_ZIPLIB
	return ::ZipAddFolder((HZIP)m_handle, destFilename);
#else
	auto* pEntry = new ZipArchiveEntry();
	pEntry->Init(destFilename);
	m_entries.push_back(pEntry);
	return 0;
#endif
}

DWORD CZipWriter::AddDirectory(const char* dstzn, const char* filepattern, int nSubLevel)
{
	string sDestFolder = dstzn;
	if (sDestFolder.size() > 0)
	{
		char lastChar = sDestFolder[sDestFolder.size() - 1];
		if (lastChar != '\\' &&  lastChar != '/')
		{
			sDestFolder += "/";
		}
	}

	string rootpath = CParaFile::GetParentDirectoryFromPath(filepattern);

	//////////////////////////////////////////////////////////////////////////
	// add the dstzn folder itself
	if (ZipAddFolder((sDestFolder).c_str()) != ZIP_OK)
	{
		OUTPUT_LOG("warning: unable to add folder %s to zip. \n", (sDestFolder).c_str());
	}

	CSearchResult* result = CFileManager::GetInstance()->SearchFiles(rootpath, CParaFile::GetFileName(filepattern), "", nSubLevel, 10000000, 0);
	if (result != 0)
	{
		int nNum = result->GetNumOfResult();
		for (int i = 0; i < nNum; ++i)
		{
			string item = result->GetItem(i);
			if (CParaFile::GetFileExtension(item) != "")
			{
				DWORD result = ZipAdd((sDestFolder + item).c_str(), (rootpath + item).c_str());

				if (result == ZIP_NOFILE)
				{
					// ZipAdd internally uses synchronous IO, however some database files may be opened with asynchronous flag,
					// please see CreateFile(..., FILE_FLAG_OVERLAPPED,...). In such situations, I will :
					// - copy the file using asynchronous IO to a temp location
					// - add zip from the temp location with synchronous IO
					// - delete the temp zip at the temp location. 
					string diskfile = (rootpath + item).c_str();
					string tempfile = diskfile + ".temp";
					
					if (CFileUtils::CopyFile(diskfile.c_str(), tempfile.c_str(), false))
					{
						result = ZipAdd((sDestFolder + item).c_str(), tempfile.c_str());
						if (!CFileUtils::DeleteFile(tempfile.c_str()))
						{
							OUTPUT_LOG("warning: unable to delete temp file %s during zipping \n", tempfile.c_str());
						}
					}
					else
					{
						OUTPUT_LOG("warning: unable to add file %s to zip. It may be used by another application.\n", diskfile.c_str());
					}
				}

				if (result != ZIP_OK)
				{
					OUTPUT_LOG("warning: unable to add file %s to zip. \n", (rootpath + item).c_str());
				}
			}
			else
			{
				if (!item.empty())
				{
					if (ZipAddFolder((sDestFolder + item).c_str()) != ZIP_OK)
					{
						OUTPUT_LOG("warning: unable to add folder %s to zip. \n", (sDestFolder + item).c_str());
					}
				}
			}
		}
		SAFE_RELEASE(result);
	}
	return 0;
}

void ParaEngine::CZipWriter::removeAllEntries()
{
	for (ZipArchiveEntry* entry : m_entries) {
		SAFE_DELETE(entry);
	}
	m_entries.clear();
}

int ParaEngine::CZipWriter::SaveAndClose()
{
	CParaFile file;
	if (file.OpenFile(m_filename.c_str(), false))
	{
		auto startPosition = file.getPos();

		for (auto* entry : m_entries)
		{
			entry->SerializeLocalFileHeader(file);
		}

		auto offsetOfStartOfCDFH = file.getPos() - startPosition;
		for (auto* entry : m_entries)
		{
			entry->SerializeCentralDirectoryFileHeader(file);
		}

		ZIP_EndOfCentralDirectoryBlock _endOfCentralDirectoryBlock;
		_endOfCentralDirectoryBlock.sig = ZIP_CONST_ENDSIG;
		_endOfCentralDirectoryBlock.thisDiskNumber = 0;
		_endOfCentralDirectoryBlock.startCentralDirDisk = 0;

		_endOfCentralDirectoryBlock.entriesForThisDisk = static_cast<uint16_t>(m_entries.size());
		_endOfCentralDirectoryBlock.entriesForWholeCentralDir = static_cast<uint16_t>(m_entries.size());

		_endOfCentralDirectoryBlock.centralDirSize = static_cast<uint32_t>(file.getPos() - offsetOfStartOfCDFH);
		_endOfCentralDirectoryBlock.offsetOfCentralDir = static_cast<uint32_t>(offsetOfStartOfCDFH);
		_endOfCentralDirectoryBlock.commentSize = 0;
		file.write(&_endOfCentralDirectoryBlock, sizeof(_endOfCentralDirectoryBlock));

		file.close();
	}

	removeAllEntries();
	return 0;
}
