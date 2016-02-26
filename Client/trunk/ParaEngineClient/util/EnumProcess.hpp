#pragma once
////////////////////////////////////////////////////////////////
// MSDN Magazine -- July 2002
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio 6.0 and Visual Studio .NET
// Runs in Windows XP and probably Windows 2000 too.
//

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <malloc.h>
#include <crtdbg.h>

#include <Windows.h>
#include <psapi.h>

namespace ParaEngine
{
	//////////////////
	// Iterate the top-level windows. Encapsulates ::EnumWindows.
	//
	class CWindowIterator {
	protected:
		HWND*	m_hwnds;				// array of hwnds for this PID
		DWORD m_nAlloc;			// size of array
		DWORD m_count;				// number of HWNDs found
		DWORD m_current;			// current HWND

		static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lp){
			return ((CWindowIterator*)lp)->OnEnumProc(hwnd);
		}

		// virtual enumerator
		virtual BOOL OnEnumProc(HWND hwnd){
			if (OnWindow(hwnd)) {
				if (m_count < m_nAlloc)
					m_hwnds[m_count++] = hwnd;
			}
			return TRUE; // keep looking
		}

		// override to filter different kinds of windows
		virtual BOOL OnWindow(HWND hwnd) {
			return TRUE;
		}

	public:
		CWindowIterator(DWORD nAlloc=1024)
		{
			//ASSERT(nAlloc>0);
			m_current = m_count = 0;
			m_hwnds = new HWND [nAlloc];
			m_nAlloc = nAlloc;
		}

		~CWindowIterator()
		{
			delete [] m_hwnds;
		}

		DWORD GetCount() { return m_count; }
		HWND First(){
			::EnumWindows(EnumProc, (LPARAM)this);
			m_current = 0;
			return Next();
		}
		HWND Next() {
			return m_hwnds && m_current < m_count ? m_hwnds[m_current++] : NULL;
		}
	};

	//////////////////
	// Iterate the top-level windows in a process.
	//
	class CMainWindowIterator : public CWindowIterator  {
	protected:
		DWORD m_pid;							// process id
		virtual BOOL OnWindow(HWND hwnd){
			if (GetWindowLong(hwnd,GWL_STYLE) & WS_VISIBLE) {
				DWORD pidwin;
				GetWindowThreadProcessId(hwnd, &pidwin);
				if (pidwin==m_pid)
					return TRUE;
			}
			return FALSE;
		}
	public:
		CMainWindowIterator(DWORD pid, DWORD nAlloc=1024){
			m_pid = pid;
		}
		~CMainWindowIterator(){}
	};

	//////////////////
	// Process iterator -- iterator over all system processes
	// Always skips the first (IDLE) process with PID=0.
	//
	class CProcessIterator {
	protected:
		DWORD*	m_pids;			// array of procssor IDs
		DWORD		m_count;			// size of array
		DWORD		m_current;		// next array item
	public:
		CProcessIterator(){
			m_pids = NULL;
		}
		~CProcessIterator()
		{
			delete [] m_pids;
		}


		DWORD GetCount() { return m_count; }
		DWORD First(){
			m_current = (DWORD)-1;
			m_count = 0;
			DWORD nalloc = 1024;
			do {
				delete [] m_pids;
				m_pids = new DWORD [nalloc];
				if (EnumProcesses(m_pids, nalloc*sizeof(DWORD), &m_count)) {
					m_count /= sizeof(DWORD);
					m_current = 1; // important: skip IDLE process with pid=0.
				}
			} while (nalloc <= m_count);

			return Next();
		}
		DWORD Next() {
			return m_pids && m_current < m_count ? m_pids[m_current++] : 0;
		}
	};



	//////////////////
	// Iterate the modules in a process. Note that the first module is the
	// main EXE that started the process.
	//
	class CProcessModuleIterator {
	protected:
		HANDLE	m_hProcess;			// process handle
		HMODULE*	m_hModules;			// array of module handles
		DWORD		m_count;				// size of array
		DWORD		m_current;			// next module handle
	public:
		CProcessModuleIterator(DWORD pid)
		{
			// open the process
			m_hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, pid);
		}

		~CProcessModuleIterator()
		{
			CloseHandle(m_hProcess);
			delete [] m_hModules;
		}

		HANDLE GetProcessHandle()	{ return m_hProcess; }
		DWORD GetCount()				{ return m_count; }
		HMODULE First()
		{
			m_count = 0;
			m_current = (DWORD)-1; 
			m_hModules = NULL;
			if (m_hProcess) {
				DWORD nalloc = 1024;
				do {
					delete [] m_hModules;
					m_hModules = new HMODULE [nalloc];
					if (EnumProcessModules(m_hProcess, m_hModules,
						nalloc*sizeof(DWORD), &m_count)) {
							m_count /= sizeof(HMODULE);
							m_current = 0;
					}
				} while (nalloc <= m_count);
			}
			return Next();
		}
		HMODULE Next() {
			return m_hProcess && m_current < m_count ? m_hModules[m_current++] : 0;
		}
	};

	//////////////////
	// Handy class to facilitate finding and killing a process by name.
	class CFindKillProcess 
	{
	public:
		CFindKillProcess(){}
		~CFindKillProcess(){}
		DWORD FindProcess(LPCTSTR modname, BOOL bAddExe=TRUE, BOOL bSkipCurrentProcess = TRUE)
		{
			DWORD dwCurrentPID = GetCurrentProcessId();
			CProcessIterator   itp;   
			for   (DWORD   pid=itp.First();   pid;   pid=itp.Next())   {   
				TCHAR   name[_MAX_PATH];   
				CProcessModuleIterator   itm(pid);   
				HMODULE   hModule   =   itm.First();   //   .EXE   
				if   (hModule && (dwCurrentPID != pid || !bSkipCurrentProcess) )   
				{   
					GetModuleBaseName(itm.GetProcessHandle(),   
						hModule,   name,   _MAX_PATH);   

					std::string   sModName   =   modname;   
					if   (_strcmpi(sModName.c_str(),name)==0)   
						return   pid;   
					sModName   +=   ".exe";   
					if   (bAddExe   &&   _strcmpi(sModName.c_str(),name)==0)   
						return   pid;   
				}   
			}   
			return   0;   
		}

		bool FindAndKillProcess(LPCTSTR modname, BOOL bAddExe=TRUE, BOOL bSkipCurrentProcess = TRUE)
		{   
			DWORD dwCurrentPID = GetCurrentProcessId();
			CProcessIterator   itp;   
			for   (DWORD   pid=itp.First();   pid;   pid=itp.Next())   {   
				TCHAR   name[_MAX_PATH];   
				CProcessModuleIterator   itm(pid);   
				HMODULE   hModule   =   itm.First();   //   .EXE   
				if   (hModule && (dwCurrentPID != pid || !bSkipCurrentProcess))   
				{   
					GetModuleBaseName(itm.GetProcessHandle(),   
						hModule,   name,   _MAX_PATH);   

					std::string   sModName   =   modname;   
					if   (_strcmpi(sModName.c_str(),name)==0)
					{
						if(!KillProcess(pid,true))
						{
							return false;
						}
					}
					sModName   +=   ".exe";   
					if   (bAddExe   &&   _strcmpi(sModName.c_str(),name)==0)   
					{
						if(!KillProcess(pid,true))
						{
							return false;
						}
					}
				}   
			}   
			return   true;   
		}
		BOOL KillProcess(DWORD pid, BOOL bZap){   
			CMainWindowIterator   itw(pid);   
			for   (HWND   hwnd=itw.First();   hwnd;   hwnd=itw.Next())   {   
				DWORD_PTR   bOKToKill = FALSE;
				SendMessageTimeout(hwnd,   WM_QUERYENDSESSION,   0,   0,   
					SMTO_ABORTIFHUNG|SMTO_NOTIMEOUTIFNOTHUNG,   5000,   &bOKToKill);   
				if   (!bOKToKill)   
				{
					printf("program do not exit!");
					break;
				}
				PostMessage(hwnd,   WM_CLOSE,   0,   0);   
			}   

			BOOL   bKilled   =   TRUE;   
			HANDLE   hp=OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE,FALSE,pid);   
			if   (hp)   {   
				if   (WaitForSingleObject(hp,   5000)   !=   WAIT_OBJECT_0)   {   
					if   (bZap)   {   
						OutputDebugStringA("Terminate Process normally!");
						TerminateProcess(hp,0);   
					}   else   {   
						OutputDebugStringA("bzap is false!");
						bKilled   =   FALSE;   
					}   
				}   
				else
				{
					Sleep(1000);
					OutputDebugStringA("wait single object return false");
					if   (bZap)   {   
						OutputDebugStringA("Terminate Process!");
						TerminateProcess(hp,0);   
					} 
				}
				CloseHandle(hp);   
			}
			else
			{
				OutputDebugStringA("hp is null");
			}
			return   bKilled;   
		}  
	};
}
