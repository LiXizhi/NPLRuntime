//----------------------------------------------------------------------
// Class:	ParaEngine X model global functions
// Authors:	Li,Xizhi
// Emails:	lxz1982@hotmail.com or LiXizhi@yeah.net
// Date: 2005/03
// original class  based on David GRIMBICHLER (theprophet@wanadoo.Fr) in its War3 Viewer
/**
Some global functions used by the model loader
*/
#include "DxStdAfx.h"
BOOL g_showmeshes[] = {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE};
float myEPSILON = 0.001f;

#ifdef MPQ_ARCHIVE


#include "math.h"
#include "mdx.h"
#include "blp.h"
#include "MDXModel.h"

#include "SFmpq_static.h"
#include "memdebug.h"
using namespace ParaEngine;

/**
* Turn-on this macro when you want extract MPQ file to disk, this will accelerate loading.
*/
/// #define EXTRACT_MPQ_FILE_TO_DISK

#pragma comment(lib, "SFmpq_static.lib")
#pragma comment(lib, "MyJPEGLib.lib")

// LiXizhi: extern(s) scope removed
DWORD FromMPQ(char*filename, char*& buffer);

extern void *malloc_func(const size_t size);
extern void free_func(void* ptr);

/// only try loading from MPQ file, it will return 0, if file is not in the specified MPQ file
DWORD FromMPQ(char*szFichier, char*& buffer)
{
	MPQHANDLE hFile;
	DWORD dwsize, r;

	if (SFileOpenFile(szFichier, &hFile)==FALSE) return 0;
	if (hFile==(HANDLE)0xcccccccc)
		return 0;
	dwsize = SFileGetFileSize(hFile, NULL);
	buffer = new char[dwsize];
	SFileReadFile(hFile, buffer, dwsize, &r, NULL); 
	SFileCloseFile(hFile);
	return dwsize;
}


/// load from disk, then try loading from the MPQ file.
DWORD FromMPQ1(char*szFichier, char*& buffer)
{
	MPQHANDLE hFile;
	DWORD dwsize, r;

	/// first try loading from disk
	HANDLE hf = CreateFile(szFichier, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hf==INVALID_HANDLE_VALUE)
	{
		if (SFileOpenFile(szFichier, &hFile)==FALSE) return 0;
		if (hFile==(HANDLE)0xcccccccc)
			return 0;
		dwsize = SFileGetFileSize(hFile, NULL);
		buffer = new char[dwsize];
		SFileReadFile(hFile, buffer, dwsize, &r, NULL); 
		SFileCloseFile(hFile);

//#define EXTRACT_MPQ_FILE_TO_DISK
#ifdef EXTRACT_MPQ_FILE_TO_DISK	
		/// we will extract to disk
		char filepath[200];
		
		for(int i=0; szFichier[i]!='\0'; i++)
		{
			filepath[i]=szFichier[i];
			if(filepath[i] == '\\')
			{
				filepath[i+1] = '\0';
				CreateDirectory(filepath, NULL);
			}
		}
		hf = CreateFile(szFichier, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
		if(hf)
		{
			WriteFile(hf, buffer, dwsize, &r, NULL);		
			CloseHandle(hf);
		}
#endif

		return dwsize;
	}

	dwsize = GetFileSize(hf, NULL);
	buffer = new char[dwsize];
	ReadFile(hf, buffer, dwsize, &r, NULL);		
	CloseHandle(hf);
	return dwsize;
}
#endif