#include "ParaEngine.h"
#include "Globals.h"
#include "ParaEngineSettings.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "StringHelper.h"

#include <Shellapi.h>
#include <vector>
#undef ShellExecute
namespace ParaEngine {
	
	bool CEditorHelper::OpenWithDefaultEditor(const char* sScriptFile, bool bWaitOnReturn)
	{
		if (!sScriptFile)
			return false;

		//////////////////////////////////////////////////////////////////////////
		// open the editor.
		STARTUPINFOW si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		const std::string& sEditor = CGlobals::GetSettings().GetScriptEditor();
		string sCommands = "\"" + sEditor;
		sCommands.append("\" ");
		sCommands.append(sScriptFile);

		const WCHAR* pWideEditor = !sEditor.empty() ? StringHelper::MultiByteToWideChar(sEditor.c_str(), DEFAULT_FILE_ENCODING) : nullptr;
		std::wstring wEditor = pWideEditor ? pWideEditor : L"";
		const WCHAR* pWideCmd = StringHelper::MultiByteToWideChar(sCommands.c_str(), DEFAULT_FILE_ENCODING);
		std::wstring wCmdStr = pWideCmd ? pWideCmd : L"";
		std::vector<WCHAR> wCmdBuf(wCmdStr.begin(), wCmdStr.end());
		wCmdBuf.push_back(L'\0');

		// Start the child process. 
		if (!::CreateProcessW(wEditor.empty() ? NULL : wEditor.c_str(),
			wCmdBuf.data(),   // Command line. 
			NULL,             // Process handle not inheritable. 
			NULL,             // Thread handle not inheritable. 
			FALSE,            // Set handle inheritance to FALSE. 
			0,                // No creation flags. 
			NULL,             // Use parent's environment block. 
			NULL,             // Use parent's starting directory. 
			&si,              // Pointer to STARTUPINFO structure.
			&pi)              // Pointer to PROCESS_INFORMATION structure.
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
		if (!scriptFile || !sName)
			return false;

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

		HANDLE hChildStdinRd = NULL, hChildStdinWr = NULL,
			hChildStdoutRd = NULL, hChildStdoutWr = NULL;

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
				CloseHandle(hChildStdoutRd);
				CloseHandle(hChildStdoutWr);
				OUTPUT_LOG("error: Stdin pipe creation failed\n");
				return false;
			}

			// Ensure that the write handle to the child process's pipe for STDIN is not inherited. 
			SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);
		}

		//////////////////////////////////////////////////////////////////////////
		// open the child process.
		STARTUPINFOW si;
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

		const bool hasAppName = (lpApplicationName != nullptr && lpApplicationName[0] != '\0');
		const WCHAR* pWideApp = hasAppName ? StringHelper::MultiByteToWideChar(lpApplicationName, DEFAULT_FILE_ENCODING) : nullptr;
		std::wstring wAppName = pWideApp ? pWideApp : L"";

		const WCHAR* pWideCmd = (lpCommandLine && lpCommandLine[0] != '\0') ? StringHelper::MultiByteToWideChar(lpCommandLine, DEFAULT_FILE_ENCODING) : nullptr;
		std::wstring wCmdStr = pWideCmd ? pWideCmd : L"";
		std::vector<WCHAR> wCmdBuf(wCmdStr.begin(), wCmdStr.end());
		wCmdBuf.push_back(L'\0');

		// Start the child process. 
		if (!::CreateProcessW(hasAppName ? wAppName.c_str() : NULL,
			wCmdBuf.data(),                   // Command line. 
			NULL,                             // Process handle not inheritable. 
			NULL,                             // Thread handle not inheritable. 
			bWaitOnReturn ? TRUE : FALSE,     // Inherit stdio handles when waiting on return. 
			0,                                // No creation flags. 
			NULL,                             // Use parent's environment block. 
			NULL,                             // Use parent's starting directory. 
			&si,                              // Pointer to STARTUPINFO structure.
			&pi)                              // Pointer to PROCESS_INFORMATION structure.
			)
		{
			if (bWaitOnReturn)
			{
				if (hChildStdinRd) CloseHandle(hChildStdinRd);
				if (hChildStdinWr) CloseHandle(hChildStdinWr);
				if (hChildStdoutRd) CloseHandle(hChildStdoutRd);
				if (hChildStdoutWr) CloseHandle(hChildStdoutWr);
			}
			OUTPUT_LOG("error: failed running application: %s, error code(%d).\r\n", lpApplicationName ? lpApplicationName : "", GetLastError());
			return false;
		}

		// Wait until child process exits.
		if (bWaitOnReturn)
		{
			// Close child-side ends in parent immediately so ReadFile does not deadlock on large output
			if (hChildStdoutWr)
			{
				CloseHandle(hChildStdoutWr);
				hChildStdoutWr = NULL;
			}
			if (hChildStdinRd)
			{
				CloseHandle(hChildStdinRd);
				hChildStdinRd = NULL;
			}
			if (hChildStdinWr)
			{
				CloseHandle(hChildStdinWr);
				hChildStdinWr = NULL;
			}

			// ReadFromPipe while or after child runs
			{
				DWORD dwRead = 0;
				char chBuf[BUFSIZE];

				for (;;)
				{
					if (!ReadFile(hChildStdoutRd, chBuf, BUFSIZE, &dwRead, NULL) || dwRead == 0)
						break;
					CLogger::GetSingleton().Write(chBuf, dwRead);
				}
			}

			if (hChildStdoutRd)
			{
				CloseHandle(hChildStdoutRd);
				hChildStdoutRd = NULL;
			}

			WaitForSingleObject(pi.hProcess, INFINITE);
		}

		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return true;
	}

	bool CEditorHelper::ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{
		const bool hasDir = (lpDirectory != nullptr && lpDirectory[0] != '\0');
		// OUTPUT_LOG("warning: security alert. ShellExecute function should only allow explore folder and text only files in formal release\n");
		if (lpOperation && strcmp(lpOperation, "wait") == 0)
		{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
			const WCHAR* wFile = lpFile ? StringHelper::MultiByteToWideChar(lpFile, DEFAULT_FILE_ENCODING) : nullptr;
			std::wstring str_file = wFile ? wFile : L"";
			const WCHAR* wParam = lpParameters ? StringHelper::MultiByteToWideChar(lpParameters, DEFAULT_FILE_ENCODING) : nullptr;
			std::wstring str_param = wParam ? wParam : L"";
			const WCHAR* wDir = hasDir ? StringHelper::MultiByteToWideChar(lpDirectory, DEFAULT_FILE_ENCODING) : nullptr;
			std::wstring str_dir = wDir ? wDir : L"";

			SHELLEXECUTEINFOW ShExecInfo = { 0 };
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
			ShExecInfo.lpParameters = lpParameters ? str_param.c_str() : NULL;
			ShExecInfo.lpDirectory = hasDir ? str_dir.c_str() : NULL;
			ShExecInfo.lpFile = lpFile ? str_file.c_str() : NULL;
#else 
			SHELLEXECUTEINFO ShExecInfo = { 0 };
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			ShExecInfo.lpParameters = lpParameters;
			ShExecInfo.lpDirectory = hasDir ? lpDirectory : NULL;
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
				if (ShExecInfo.hProcess)
				{
					WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
					CloseHandle(ShExecInfo.hProcess);
				}
				return true;
			}
			else
				return false;
		}
		else
		{
			const char* effectiveOp = (lpOperation && strcmp(lpOperation, "openExternalBrowser") == 0) ? "open" : lpOperation;
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
			const WCHAR* wOp = (effectiveOp && effectiveOp[0] != '\0') ? StringHelper::MultiByteToWideChar(effectiveOp, DEFAULT_FILE_ENCODING) : nullptr;
			std::wstring str_op = wOp ? wOp : L"";
			const WCHAR* wFile = lpFile ? StringHelper::MultiByteToWideChar(lpFile, DEFAULT_FILE_ENCODING) : nullptr;
			std::wstring str_file = wFile ? wFile : L"";
			const WCHAR* wParam = lpParameters ? StringHelper::MultiByteToWideChar(lpParameters, DEFAULT_FILE_ENCODING) : nullptr;
			std::wstring str_param = wParam ? wParam : L"";
			const WCHAR* wDir = hasDir ? StringHelper::MultiByteToWideChar(lpDirectory, DEFAULT_FILE_ENCODING) : nullptr;
			std::wstring str_dir = wDir ? wDir : L"";
			if ((INT_PTR)::ShellExecuteW(
				(HWND)NULL,
				(effectiveOp && effectiveOp[0] != '\0') ? str_op.c_str() : NULL,
				lpFile ? str_file.c_str() : NULL,
				lpParameters ? str_param.c_str() : NULL,
				hasDir ? str_dir.c_str() : NULL,
				nShowCmd) > 32)
#else
			if ((INT_PTR)::ShellExecute((HWND)NULL, (effectiveOp && effectiveOp[0] != '\0') ? effectiveOp : NULL, lpFile, lpParameters, hasDir ? lpDirectory : NULL, nShowCmd) > 32)
#endif

				return true;
			else
				return false;
		}
	}
	
} // end namespace

