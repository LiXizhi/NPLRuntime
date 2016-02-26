#pragma once

#include <string>
namespace ParaEngine
{
	using namespace std;
	/**
	* helper functions for editing. It is a group of static helper functions for scene editing.  
	*/
	class PE_CORE_DECL CEditorHelper
	{
	public:
		/**
		* return the file name from a given script text. 
		* It searches "NPL.load(\"" in the script text, and returns its first string parameter. 
		* @param output: output file name
		* @param sScript: script text. if the file name is not found within the first MAX_PATH=260 characters, "" is returned. 
		* @param bRelativePath: if true, relative path is returned. if false, the complete NPL path in the script is returned, which may contain (gl) prefix.
		* @return: true if found. 
		*/
		static bool SearchFileNameInScript(string & output, const char* sScript, bool bRelativePath = true);
		/** same as SearchFileNameInScript(). only used for API exportation. Not thread-safe*/
		static const char* SearchFileNameInScript_(const char* sScript, bool bRelativePath);

		/**
		* Open a given file with the default registered editor in the game engine. 
		* @param sFileName: file name to be opened by the default editor.
		* @param bWaitOnReturn: if false, the function returns immediately; otherwise it will wait for the editor to return. 
		* @return true if opened. 
		*/
		static bool OpenWithDefaultEditor( const char* sFilename, bool bWaitOnReturn = false);

		/**
		* create an empty character event file with empty event handlers
		* @param sScriptFile: the script file to be created. 
		* @param sName: the character name. 
		* @return: return true if file is created. return false, if the file already exists, or failed creating the new file. 
		*/
		static bool CreateEmptyCharacterEventFile(const char* sScriptFile, const char* sName);

		/**
		* run an external application. creates a new process and its primary thread. The new process runs the specified executable file in the security context of the calling process.
		* @remark: One can also use ParaEngine C++ or .Net API to write application plug-ins for the game engine, which can be loaded like any other script files.
		* e.g. To open a file in an external notepad editor use  ParaGlobal.CreateProcess("c:\\notepad.exe", "\"c:\\notepad.exe\" c:\\test.txt", true);
		* @param lpApplicationName:Pointer to a null-terminated string that specifies the module to execute. The specified module can be a Windows-based application. 
		* The string can specify the full path and file name of the module to execute or it can specify a partial name. In the case of a partial name, the function 
		* uses the current drive and current directory to complete the specification. The function will not use the search path. If the file name does not contain an extension, .exe is assumed. 
		* If the executable module is a 16-bit application, lpApplicationName should be NULL, and the string pointed to by lpCommandLine should specify the executable module as well as its arguments.
		* @param lpCommandLine:Pointer to a null-terminated string that specifies the command line to execute.
		* @param bWaitOnReturn: if false, the function returns immediately; otherwise it will wait for the editor to return. 
		* if this is true, the Child Process will have Redirected Input and Output to current log file. 
		* @return true if opened. 
		*/
		static bool CreateProcess(const char* lpApplicationName, const char* lpCommandLine, bool bWaitOnReturn = false);

		/**
		* Performs an operation on a specified file.
		* e.g. ShellExecute("open", "iexplore.exe", "http://www.paraengine.com", NULL, 1);
		* 
		* @param lpOperation:[in] Pointer to a null-terminated string, 
		*  - "wait" this is a special one that uses ShellExecuteEx to wait on the process to terminate before return
		*  - "edit" Launches an editor and opens the document for editing. If lpFile is not a document file, the function will fail.
		*  - "explore"	Explores the folder specified by lpFile.
		*  - "find"		Initiates a search starting from the specified directory.
		*  - "open"		Opens the file specified by the lpFile parameter. The file can be an executable file, a document file, or a folder.
		*  - "print"		Prints the document file specified by lpFile. If lpFile is not a document file, the function will fail.
		*  - NULL		For systems prior to Microsoft Windows 2000, the default verb is used if it is valid and available in the registry. If not, the "open" verb is used.
		* @param lpFile [in] Pointer to a null-terminated string that specifies the file or object on which to execute the specified verb. To specify a Shell namespace object, pass the fully qualified parse name. Note that not all verbs are supported on all objects. For example, not all document types support the "print" verb.
		* @param lpParameters[in] If the lpFile parameter specifies an executable file, lpParameters is a pointer to a null-terminated string that specifies the parameters to be passed to the application. The format of this string is determined by the verb that is to be invoked. If lpFile specifies a document file, lpParameters should be NULL.
		* @param lpDirectory [in] Pointer to a null-terminated string that specifies the default directory.
		* @param nShowCmd: 	[in] Flags that specify how an application is to be displayed when it is opened. If lpFile specifies a document file, the flag is simply passed to the associated application. It is up to the application to decide how to handle it.
		*  - 	#define SW_HIDE             0
		*  - 	#define SW_NORMAL           1
		*  - 	#define SW_MAXIMIZE         3
		*  - 	#define SW_SHOW             5
		*  - 	#define SW_MINIMIZE         6
		*  - 	#define SW_RESTORE          9
		*/
		static bool ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd);
		
	};
}
