//-----------------------------------------------------------------------------
// Class: ZipWriter
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.10
// Revised: 2016.4.24
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ZipWriter.h"
#include "MemReadFile.h"
#include "FileUtils.h"
#include "ZipArchive.h"
#include "NPLCodec.h"
#include "zlib.h"

#ifdef PARAENGINE_CLIENT
#include "zlib/zip.h"
#endif

using namespace ParaEngine;

namespace ParaEngine 
{
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
#ifdef PARAENGINE_CLIENT
	return m_handle != 0;
#else
	return true;
#endif
}

DWORD CZipWriter::close()
{
#ifdef PARAENGINE_CLIENT
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
#ifdef PARAENGINE_CLIENT
	if(m_handle == 0)
		m_handle = new CZipWriter(::CreateZip(fn, password));
#else
	m_filename = filename;
	m_password = password ? password : "";
#endif
}

DWORD CZipWriter::ZipAdd(const char* destFilename, const char* filename)
{
#ifdef PARAENGINE_CLIENT
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
#ifdef PARAENGINE_CLIENT
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
