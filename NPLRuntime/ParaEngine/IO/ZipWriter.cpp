//-----------------------------------------------------------------------------
// Class: ZipWriter
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.10
// Revised: 2016.4.24 removed dependency on ziplib
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ZipWriter.h"
#include "MemReadFile.h"
#include "FileUtils.h"
#include "ZipArchive.h"
#include "NPLCodec.h"
#include <boost/filesystem.hpp>
#include "zlib.h"


using namespace ParaEngine;

namespace ParaEngine 
{
	void dosdatetime2filetime(WORD dosdate, WORD dostime, time_t *ft)
	{
		tm st;
		st.tm_year = (WORD)((dosdate >> 9) + 1980 - 1900);
		st.tm_mon = (WORD)(((dosdate >> 5) & 0xf) - 1);
		st.tm_mday = (WORD)(dosdate & 0x1f);
		st.tm_hour = (WORD)(dostime >> 11);
		st.tm_min = (WORD)((dostime >> 5) & 0x3f);
		st.tm_sec = (WORD)((dostime & 0x1f) * 2);
		*ft = mktime( &st );
	}

#ifdef WIN32
	void standardtime2osfiletime(time_t t, LPFILETIME pft)
	{
		// This function comes from MSDN:
		// https://msdn.microsoft.com/zh-tw/library/windows/desktop/ms724228(v=vs.85).aspx
		// Note that LONGLONG is a 64-bit value
		LONGLONG ll;

		ll = Int32x32To64(t, 10000000) + 116444736000000000;
		pft->dwLowDateTime = (DWORD)ll;
		pft->dwHighDateTime = ll >> 32;
	}
#else
	void standardtime2osfiletime(time_t source_t, time_t* target_t)
	{
		if(target_t)
			*target_t = source_t;
	}    
#endif

	void filetime2dosdatetime(const time_t& ft, WORD *dosdate, WORD *dostime)
	{
		// struct tm *st = gmtime(&ft); // convert to UTC time
		struct tm *st = localtime(&ft); // convert to Local time
		
		*dosdate = (uint16_t)(((st->tm_year + 1900 - 1980) & 0x7f) << 9);
		*dosdate |= (uint16_t)(((st->tm_mon+1) & 0xf) << 5);
		*dosdate |= (uint16_t)((st->tm_mday & 0x1f));
		*dostime = (uint16_t)((st->tm_hour & 0x1f) << 11);
		*dostime |= (uint16_t)((st->tm_min & 0x3f) << 5);
		*dostime |= (uint16_t)((st->tm_sec / 2) & 0x1f);
	}

	void GetFileTime(const std::string& filename, WORD *dosdate, WORD *dostime)
	{
		using namespace boost::filesystem;
		boost::filesystem::path filePath(filename);
		if (boost::filesystem::exists(filePath))
		{
			time_t lastWriteTime = boost::filesystem::last_write_time(filePath);
			if (lastWriteTime != (time_t)(-1))
			{
				filetime2dosdatetime(lastWriteTime, dosdate, dostime);
			}
		}
	}

	/** a single file in zip archive to be written to disk */
	class ZipArchiveEntry : public CRefCounted 
	{
	public:
		ZipArchiveEntry() : m_offsetOfCompressedData(0), m_offsetOfSerializedLocalFileHeader(0), m_pFile(nullptr)
		{
			memset(&m_localFileHeader, 0, sizeof(SZIPFileHeader));
			m_localFileHeader.Sig = ZIP_CONST_LOCALHEADERSIG;
		}

		virtual ~ZipArchiveEntry() 
		{
			if (m_pFile)
				delete m_pFile;
		}
	public:

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

		/**
		* @param destFilename: filename in zip file
		* @param pFile: file data reader.
		*/
		void Init(const char* destFilename, CParaFile* pFile)
		{
			m_destFilename = destFilename;
			m_localFileHeader.FilenameLength = static_cast<uint16_t>(m_destFilename.length());
			m_filename = m_destFilename;
			m_pFile = pFile;
		}

		void SerializeLocalFileHeader(CParaFile& file) 
		{
			// save offset of stream here
			m_offsetOfSerializedLocalFileHeader = file.getPos();

			m_localFileHeader.DataDescriptor.CompressedSize = 0;
			m_localFileHeader.DataDescriptor.UncompressedSize = 0;
			m_localFileHeader.DataDescriptor.CRC32 = 0;
			
			// serialize header
			file.write(&m_localFileHeader, sizeof(SZIPFileHeader));
			file.WriteString(m_destFilename);

			m_offsetOfCompressedData = file.getPos();
			// serialize body of compressed file
			if (!IsDirectory()) 
			{
				std::string sExt = CParaFile::GetFileExtension(m_destFilename);
				int CompressionLevel = -1;
				if (sExt == "pkg" || sExt == "zip" || sExt == "p3d" || sExt == "ogg" || sExt == "png" || sExt == "jpg" || sExt == "mp3")
				{
					// no compression
					if (m_pFile)
					{
						m_localFileHeader.CompressionMethod = 0; // 8 for zip, 0 for no compression.
						m_localFileHeader.DataDescriptor.UncompressedSize = m_pFile->getSize();
						m_localFileHeader.DataDescriptor.CompressedSize = m_pFile->getSize();

						m_localFileHeader.LastModFileDate = m_localFileHeader.LastModFileTime = 0;

						uint32_t crc = 0;
						m_localFileHeader.DataDescriptor.CRC32 = crc32(crc, (const Bytef*)m_pFile->getBuffer(), m_pFile->getSize());
						file.WriteString((const char*)m_pFile->getBuffer(), m_pFile->getSize());

						// actualize local file header
						auto currentPos = file.getPos();
						file.seek(m_offsetOfSerializedLocalFileHeader);
						file.write(&m_localFileHeader, sizeof(SZIPFileHeader));
						file.seek(currentPos);
					}
					else 
					{
						CMemReadFile input(m_filename.c_str());
						if (input.isOpen())
						{
							m_localFileHeader.CompressionMethod = 0; // 8 for zip, 0 for no compression.
							m_localFileHeader.DataDescriptor.UncompressedSize = input.getSize();
							m_localFileHeader.DataDescriptor.CompressedSize = input.getSize();

							m_localFileHeader.LastModFileDate = m_localFileHeader.LastModFileTime = 0;

							uint32_t crc = 0;
							m_localFileHeader.DataDescriptor.CRC32 = crc32(crc, (const Bytef*)input.getBuffer(), input.getSize());
							file.WriteString((const char*)input.getBuffer(), input.getSize());

							// actualize local file header
							auto currentPos = file.getPos();
							file.seek(m_offsetOfSerializedLocalFileHeader);
							file.write(&m_localFileHeader, sizeof(SZIPFileHeader));
							file.seek(currentPos);
						}
						else
						{
							OUTPUT_LOG("warning: failed to add file: %s to zip archive\n", m_filename.c_str());
						}
					}
					return;
				}

				if (m_pFile)
				{
					{
						std::string output;
						output.reserve(m_pFile->getSize());
						if (CZipWriter::Compress(output, (const char*)m_pFile->getBuffer(), m_pFile->getSize()) == 1)
						{
							m_localFileHeader.CompressionMethod = 8; // 8 for zip, 0 for no compression.
							m_localFileHeader.DataDescriptor.UncompressedSize = m_pFile->getSize();
							m_localFileHeader.DataDescriptor.CompressedSize = output.size();

							m_localFileHeader.LastModFileDate = m_localFileHeader.LastModFileTime = 0;

							uint32_t crc = 0;
							m_localFileHeader.DataDescriptor.CRC32 = crc32(crc, (const Bytef*)m_pFile->getBuffer(), m_pFile->getSize());
							file.WriteString(output);

							// actualize local file header
							auto currentPos = file.getPos();
							file.seek(m_offsetOfSerializedLocalFileHeader);
							file.write(&m_localFileHeader, sizeof(SZIPFileHeader));
							file.seek(currentPos);
						}
					}
				}
				else
				{
					CMemReadFile input(m_filename.c_str());
					if (input.isOpen())
					{
						std::string output;
						output.reserve(input.getSize());
						if (CZipWriter::Compress(output, (const char*)input.getBuffer(), input.getSize()) == 1)
						{
							m_localFileHeader.CompressionMethod = 8; // 8 for zip, 0 for no compression.
							m_localFileHeader.DataDescriptor.UncompressedSize = input.getSize();
							m_localFileHeader.DataDescriptor.CompressedSize = output.size();

							GetFileTime(m_filename, &m_localFileHeader.LastModFileDate, &m_localFileHeader.LastModFileTime);

							uint32_t crc = 0;
							m_localFileHeader.DataDescriptor.CRC32 = crc32(crc, input.getBuffer(), input.getSize());
							file.WriteString(output);

							// actualize local file header
							auto currentPos = file.getPos();
							file.seek(m_offsetOfSerializedLocalFileHeader);
							file.write(&m_localFileHeader, sizeof(SZIPFileHeader));
							file.seek(currentPos);
						}
					}
					else
					{
						OUTPUT_LOG("warning: failed to add file: %s to zip archive\n", m_filename.c_str());
					}
				}
			}
		};
		void SerializeCentralDirectoryFileHeader(CParaFile& file)
		{
			ZIP_CentralDirectory _centralDirectoryFileHeader;
			memset(&_centralDirectoryFileHeader, 0, sizeof(ZIP_CentralDirectory));
			_centralDirectoryFileHeader.Sig = ZIP_CONST_CENSIG;
			_centralDirectoryFileHeader.LastModFileTime = m_localFileHeader.LastModFileTime;
			_centralDirectoryFileHeader.LastModFileDate = m_localFileHeader.LastModFileDate;
			_centralDirectoryFileHeader.FileCRC = m_localFileHeader.DataDescriptor.CRC32;
			_centralDirectoryFileHeader.CompressionMethod = m_localFileHeader.CompressionMethod;
			_centralDirectoryFileHeader.PackSize = m_localFileHeader.DataDescriptor.CompressedSize;
			_centralDirectoryFileHeader.UnPackSize = m_localFileHeader.DataDescriptor.UncompressedSize;
			_centralDirectoryFileHeader.NameSize = m_localFileHeader.FilenameLength;

			_centralDirectoryFileHeader.LocalHeaderOffset = static_cast<int32_t>(m_offsetOfSerializedLocalFileHeader);

			file.write(&_centralDirectoryFileHeader, sizeof(ZIP_CentralDirectory));
			file.WriteString(m_destFilename);
		};

		bool IsDirectory() { return m_filename.empty(); };
	public:
		SZIPFileHeader m_localFileHeader;
		size_t m_offsetOfCompressedData;
		size_t m_offsetOfSerializedLocalFileHeader;

		std::string m_destFilename;
		std::string m_filename;
		CParaFile* m_pFile;
	};
}

/////////////////////////////////////////////
///
/// CZipWriter
///
/////////////////////////////////////////////

/** compress without zip header*/
int CZipWriter::Compress(std::string& outstring, const char* src, int nSrcSize, int compressionlevel)
{
	/**@def CHUNK is simply the buffer size for feeding data to and pulling data from the zlib routines.
	Larger buffer sizes would be more efficient, especially for inflate(). If the memory is available,
	buffers sizes on the order of 128K or 256K bytes should be used. */
	static const int NPL_ZLIB_CHUNK = 32768;

	z_stream zs;
	memset(&zs, 0, sizeof(z_stream));

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


CZipWriter::CZipWriter()
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
	return true;
}

DWORD CZipWriter::close()
{
	SaveAndClose();
	return 0;
}

void ParaEngine::CZipWriter::InitNewZip(const char * filename, const char * password)
{
	m_filename = filename;
	m_password = password ? password : "";
}


DWORD CZipWriter::ZipAdd(const char* destFilename, CParaFile* pFile)
{
	auto* pEntry = new ZipArchiveEntry();
	pEntry->Init(destFilename, pFile);
	m_entries.push_back(pEntry);
	return 0;
}

DWORD CZipWriter::ZipAdd(const char* destFilename, const char* filename)
{
	auto* pEntry = new ZipArchiveEntry();
	pEntry->Init(destFilename, filename);
	m_entries.push_back(pEntry);
	return 0;
}

DWORD CZipWriter::ZipAddFolder(const char* destFilename)
{
	auto* pEntry = new ZipArchiveEntry();
	pEntry->Init(destFilename);
	m_entries.push_back(pEntry);
	return 0;
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
			auto itemData = result->GetItemData(i);
			const string& item = itemData->m_sFileName;

			if (!itemData->IsDirectory())
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
					auto path = sDestFolder + item;
					if (item[item.size() - 1] != '/')
						path += "/";

					if (ZipAddFolder(path.c_str()) != ZIP_OK)
					{
						OUTPUT_LOG("warning: unable to add folder %s to zip. \n", path.c_str());
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
		// make file to 0 size
		file.SetFilePointer(0, FILE_BEGIN);
		file.SetEndOfFile();
		file.SetFilePointer(0, FILE_BEGIN);

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
		file.write(&_endOfCentralDirectoryBlock, sizeof(ZIP_EndOfCentralDirectoryBlock));

		file.close();
	}

	removeAllEntries();
	return 0;
}
