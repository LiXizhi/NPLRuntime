#pragma once
#include "IAttributeFields.h"	

namespace ParaEngine
{
	void dosdatetime2filetime(WORD dosdate, WORD dostime, time_t *ft);
#ifdef WIN32
	void standardtime2osfiletime(time_t t, LPFILETIME pft);
#else
	void standardtime2osfiletime(time_t source_t, time_t* target_t);
#endif
	void filetime2dosdatetime(const time_t& ft, WORD *dosdate, WORD *dostime);

	class ZipArchiveEntry;

	/**
	* creating zip files
	* 
	* e.g.
	*  (1) Traditional use, creating a zipfile from existing files
	* CZipWriter* writer = CZipWriter::CreateZip("c:\\simple1.zip","");
	* writer->ZipAdd("znsimple.bmp", "c:\\simple.bmp");
	* writer->ZipAdd("znsimple.txt", "c:\\simple.txt");
	* writer->close();
	*/
	class CZipWriter : public IAttributeFields
	{
	public:
		CZipWriter();
		~CZipWriter();

		ATTRIBUTE_DEFINE_CLASS(CZipWriter);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CZipWriter);

		enum ZipResult {
			ZIP_OK = 0,
			ZIP_NOFILE = 0x00000200,
		};
	public:
		/** 
		* call this to start the creation of a zip file.
		* one need to call Release() or use ref_ptr<CZipWriter>
		*/
		static CZipWriter* CreateZip(const char *filename, const char *password = NULL);

		/** whether the writer is valid.*/
		bool IsValid();

		/** create a new zip file*/
		void InitNewZip(const char *filename, const char *password = NULL);

		/**
		* add a zip file to the zip. file call this for each file to be added to the zip.
		* It does not check for duplicates
		* @param destFilename: destination filename as appeared in the zip file
		* @param filename: the local disk file name to add to the zip file.
		* @return: ZipResult enum. 0 if succeed. 
		*/
		DWORD ZipAdd(const char* destFilename, const char* filename);

		/**
		* add a zip folder to the zip file. call this for each folder to be added to the zip.
		* It does not check for duplicates
		* @param destFilename: destination filename as appeared in the zip file
		* @return: ZipResult enum. 0 if succeed. 
		*/
		DWORD ZipAddFolder(const char* destFilename);

		/**
		* add everything in side a directory to the zip. 
		* e.g. AddDirectory("myworld/", "worlds/myworld/ *.*", 10);
		* @param dstzn: all files in fn will be appended with this string to be saved in the zip file.
		* @param filepattern: file patterns, which can include wild characters in the file portion.
		* @param nSubLevel: sub directory levels. 0 means only files at parent directory.
		*/
		DWORD AddDirectory(const char* dstzn, const char* filepattern, int nSubLevel=0);

		/**
		* call this when you have finished adding files and folders to the zip file.
		* Note: you can't add any more after calling this.
		*/
		DWORD close();

	protected:
		int SaveAndClose();
		void removeAllEntries();

	protected:
		std::vector<ZipArchiveEntry*>  m_entries;
		std::string m_filename;
		std::string m_password;
	};
}
