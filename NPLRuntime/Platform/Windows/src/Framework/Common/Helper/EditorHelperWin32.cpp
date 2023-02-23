#include "ParaEngine.h"
#include "Globals.h"
#include "ParaEngineSettings.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "StringHelper.h"

#include <Shellapi.h>
#undef ShellExecute
namespace ParaEngine {
	
	bool CEditorHelper::OpenWithDefaultEditor(const char* sScriptFile, bool bWaitOnReturn)
	{
		//////////////////////////////////////////////////////////////////////////
		// open the editor.
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));


		string sCommands = "\"" + CGlobals::GetSettings().GetScriptEditor();
		sCommands.append("\" ");
		sCommands.append(sScriptFile);

		char cmd[MAX_PATH + 1];
		strncpy(cmd, sCommands.c_str(), MAX_PATH);
		cmd[MAX_PATH] = '\0';

		// Start the child process. 
		if (!::CreateProcess(CGlobals::GetSettings().GetScriptEditor().c_str(),   // No module name (use command line). 
			cmd, // Command line. 
			NULL,             // Process handle not inheritable. 
			NULL,             // Thread handle not inheritable. 
			FALSE,            // Set handle inheritance to FALSE. 
			0,                // No creation flags. 
			NULL,             // Use parent's environment block. 
			NULL,             // Use parent's starting directory. 
			&si,              // Pointer to STARTUPINFO structure.
			&pi)             // Pointer to PROCESS_INFORMATION structure.
			)
		{
			OUTPUT_LOG("failed running script editor. error code(%d).\r\n", GetLastError());
			return false;
		}

		// Wait until child process exits.
		if (bWaitOnReturn)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
		}

		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return true;
	}
	
	bool CEditorHelper::CreateEmptyCharacterEventFile(const char* scriptFile, const char* sName)
	{
		string sScriptFile;
		CPathReplaceables::GetSingleton().DecodePath(sScriptFile, scriptFile);
		if (!CParaFile::DoesFileExist(sScriptFile.c_str(), false))
		{
			// create a blank file if not exists
			CParaFile file;
			if (file.CreateNewFile(sScriptFile.c_str(), true))
			{
				// make valid name, any non ascii words are replaced by "_", if the name begins by any number, "_" is appended to it. 
				int nSize = (int)strlen(sName); // unsafe
				char name[MAX_PATH];
				if (nSize >= MAX_PATH)
					nSize = MAX_PATH - 1;

				int j = 0;
				for (int i = 0; i < nSize; ++i, ++j)
				{
					char c = sName[i];
					if ((c >= 'a' &&  c <= 'z') || (c >= 'A' &&  c <= 'Z'))
					{
						name[j] = sName[i];
					}
					else if (c >= '0' &&  c <= '9')
					{
						if (i == 0)
						{
							name[j] = '_';
							j++;
						}
						name[j] = sName[i];
					}
					else
						name[j] = '_';
				}
				nSize = j;
				name[nSize] = '\0';

				// replace name from template
				CParaFile cTemplate(":IDR_NPL_CHARACTER_EVENTS");
				nSize = (int)(cTemplate.getSize() + nSize * 20);
				char* buffer = new char[nSize];
				memset(buffer, 0, nSize);

				__time64_t ltime;
				_time64(&ltime);

				snprintf(buffer, nSize, cTemplate.getBuffer(), name, _ctime64(&ltime),
					name, name, name, name, name, name, name, name, name);
				file.write(buffer, (int)strlen(buffer));
				delete[] buffer;
				return true;
			}
		}
		return false;
	}
	
	bool CEditorHelper::CreateProcess(const char* lpApplicationName, const char* lpCommandLine, bool bWaitOnReturn)
	{
		OUTPUT_LOG("warning: security alert. CreateProcess function should be deprecated in formal release\n");
#define BUFSIZE 4096 

		HANDLE hChildStdinRd, hChildStdinWr,
			hChildStdoutRd, hChildStdoutWr;

		if (bWaitOnReturn)
		{
			// Creating a Child Process with Redirected Input and Output
			SECURITY_ATTRIBUTES saAttr;

			// Set the bInheritHandle flag so pipe handles are inherited. 
			saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;

			// Create a pipe for the child process's STDOUT. 
			if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
			{
				OUTPUT_LOG("error: Stdout pipe creation failed\n");
				return false;
			}

			// Ensure that the read handle to the child process's pipe for STDOUT is not inherited.

			SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);

			// Create a pipe for the child process's STDIN. 

			if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
			{
				OUTPUT_LOG("error: Stdin pipe creation failed\n");
				return false;
			}

			// Ensure that the write handle to the child process's pipe for STDIN is not inherited. 

			SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);

			// Now create the child process. 
		}

		//////////////////////////////////////////////////////////////////////////
		// open the editor.
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		// Set up members of the STARTUPINFO structure. 
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		if (bWaitOnReturn)
		{
			si.hStdError = hChildStdoutWr;
			si.hStdOutput = hChildStdoutWr;
			si.hStdInput = hChildStdinRd;
			si.dwFlags |= STARTF_USESTDHANDLES;
		}
		// Set up members of the PROCESS_INFORMATION structure. 
		ZeroMemory(&pi, sizeof(pi));

		char cmd[MAX_PATH + 1];
		strncpy(cmd, lpCommandLine, MAX_PATH);
		cmd[MAX_PATH] = '\0';

		// Start the child process. 
		if (!::CreateProcess(lpApplicationName,   // No module name (use command line). 
			cmd, // Command line. 
			NULL,             // Process handle not inheritable. 
			NULL,             // Thread handle not inheritable. 
			FALSE,            // Set handle inheritance to FALSE. 
			0,                // No creation flags. 
			NULL,             // Use parent's environment block. 
			NULL,             // Use parent's starting directory. 
			&si,              // Pointer to STARTUPINFO structure.
			&pi)             // Pointer to PROCESS_INFORMATION structure.
			)
		{
			OUTPUT_LOG("error: failed running application: %s, error code(%d).\r\n", lpApplicationName, GetLastError());
			return false;
		}

		// Wait until child process exits.
		if (bWaitOnReturn)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);

			// ReadFromPipe
			{
				DWORD dwRead;
				char chBuf[BUFSIZE];

				// Close the write end of the pipe before reading from the 
				// read end of the pipe. 

				if (!CloseHandle(hChildStdoutWr))
				{
					OUTPUT_LOG("error: Closing handle failed\n");
					return false;
				}

				// Read output from the child process, and write to parent's STDOUT. 

				for (;;)
				{
					if (!ReadFile(hChildStdoutRd, chBuf, BUFSIZE, &dwRead,
						NULL) || dwRead == 0) break;
					if (CLogger::GetSingleton().Write(chBuf, dwRead) >= 0)
						break;
				}
			}

		}

		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return true;
	}

	bool CEditorHelper::ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{
		// OUTPUT_LOG("warning: security alert. ShellExecute function should only allow explore folder and text only files in formal release\n");
		if (lpOperation && strcmp(lpOperation, "wait") == 0)
		{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
			std::wstring str_file = StringHelper::MultiByteToWideChar(lpFile, DEFAULT_FILE_ENCODING);
			std::wstring str_param = StringHelper::MultiByteToWideChar(lpParameters, DEFAULT_FILE_ENCODING);
			std::wstring str_dir = StringHelper::MultiByteToWideChar(lpDirectory, DEFAULT_FILE_ENCODING);

			SHELLEXECUTEINFOW ShExecInfo = { 0 };
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
			ShExecInfo.lpParameters = str_param.c_str();
			ShExecInfo.lpDirectory = str_dir.c_str();
			ShExecInfo.lpFile = str_file.c_str();
#else 
			SHELLEXECUTEINFO ShExecInfo = { 0 };
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			ShExecInfo.lpParameters = lpParameters;
			ShExecInfo.lpDirectory = lpDirectory;
			ShExecInfo.lpFile = lpFile;
#endif
			ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
			ShExecInfo.hwnd = NULL;
			ShExecInfo.lpVerb = NULL;


			ShExecInfo.nShow = nShowCmd;
			ShExecInfo.hInstApp = NULL;
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
			if (ShellExecuteExW(&ShExecInfo))
#else
			if (ShellExecuteEx(&ShExecInfo))
#endif

			{
				WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
				CloseHandle(ShExecInfo.hProcess);
				return true;
			}
			else
				return false;
		}
		else
		{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
			std::wstring str_op = StringHelper::MultiByteToWideChar(lpOperation, DEFAULT_FILE_ENCODING);
			std::wstring str_file = StringHelper::MultiByteToWideChar(lpFile, DEFAULT_FILE_ENCODING);
			std::wstring str_param = StringHelper::MultiByteToWideChar(lpParameters, DEFAULT_FILE_ENCODING);
			std::wstring str_dir = StringHelper::MultiByteToWideChar(lpDirectory, DEFAULT_FILE_ENCODING);
			if (::ShellExecuteW((HWND)NULL, str_op.c_str(), str_file.c_str(), str_param.c_str(), str_dir.c_str(), nShowCmd) > ((HINSTANCE)32))
#else
			if (::ShellExecute((HWND)NULL, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd) > ((HINSTANCE)32))
#endif

				return true;
			else
				return false;
		}
	}
	
} // end namespace

