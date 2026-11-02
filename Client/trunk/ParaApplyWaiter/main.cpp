//-----------------------------------------------------------------------------
// Class:	ParaApplyWaiter
// Authors:	hyz, Kimi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2026.11.2
// Desc:	A tiny native launcher helper for 430/431 channel force update.
//   The game process exits slowly after asking the launcher to apply the
//   update, so the launcher may fail to overwrite files (e.g. main*.pkg)
//   that are still opened by the exiting game process. This helper replaces
//   the old PowerShell waiter script:
//     1. waits for the game process (waitPid) to fully exit (native
//        OpenProcess + WaitForSingleObject),
//     2. kills residual engine processes inside the install directory,
//     3. clears read-only file attributes under the install directory,
//     4. reads the launcher command line from a cmd file written by the game
//        (default caches/430applyCmd.txt) and starts the real launcher.
//   It can also run in cleanMode (kill residual + clear read-only only),
//   which is used by the game at startup to self-heal a previously failed
//   apply.
//
// Usage:
//   ParaApplyWaiter.exe waitPid=1234 launcherExe="ParaCraft.exe" [cmdFile="caches/430applyCmd.txt"] [waitTimeoutSec=180]
//   ParaApplyWaiter.exe cleanMode=true excludePid=1234
//-----------------------------------------------------------------------------
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <set>
#include <cstdio>
#include <cstdlib>

namespace ParaApplyWaiter
{
	static std::string g_rootDir;

	static void Log(const char* fmt, ...)
	{
		char msg[2048];
		va_list args;
		va_start(args, fmt);
		vsnprintf(msg, sizeof(msg) - 1, fmt, args);
		va_end(args);
		msg[sizeof(msg) - 1] = '\0';

		SYSTEMTIME st;
		GetLocalTime(&st);
		char line[2200];
		snprintf(line, sizeof(line) - 1, "[%04d-%02d-%02d %02d:%02d:%02d] [ParaApplyWaiter] %s\n",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, msg);
		line[sizeof(line) - 1] = '\0';

		OutputDebugStringA(line);

		std::string logPath = g_rootDir + "auto_update_log.txt";
		FILE* fp = fopen(logPath.c_str(), "ab");
		if (fp)
		{
			fwrite(line, 1, strlen(line), fp);
			fclose(fp);
		}
	}

	static std::string ToLower(const std::string& s)
	{
		std::string r = s;
		for (size_t i = 0; i < r.size(); ++i)
			r[i] = (char)tolower((unsigned char)r[i]);
		return r;
	}

	static std::string Trim(const std::string& s)
	{
		size_t b = s.find_first_not_of(" \t\r\n");
		if (b == std::string::npos) return "";
		size_t e = s.find_last_not_of(" \t\r\n");
		return s.substr(b, e - b + 1);
	}

	static std::string Unquote(const std::string& s)
	{
		if (s.size() >= 2 && s[0] == '"' && s[s.size() - 1] == '"')
			return s.substr(1, s.size() - 2);
		return s;
	}

	// parse "key=value" command line, values may be double quoted
	struct CmdParams
	{
		static std::string Get(const std::string& cmdLine, const std::string& key, const std::string& def = "")
		{
			std::string lower = ToLower(cmdLine);
			std::string k = ToLower(key) + "=";
			size_t pos = lower.find(k);
			if (pos == std::string::npos)
				return def;
			pos += k.size();
			if (pos < cmdLine.size() && cmdLine[pos] == '"')
			{
				size_t end = cmdLine.find('"', pos + 1);
				if (end != std::string::npos)
					return cmdLine.substr(pos + 1, end - pos - 1);
				return cmdLine.substr(pos + 1);
			}
			size_t end = cmdLine.find(' ', pos);
			if (end == std::string::npos)
				end = cmdLine.size();
			return cmdLine.substr(pos, end - pos);
		}
	};

	static std::string GetModuleDir()
	{
		char path[MAX_PATH * 2] = { 0 };
		GetModuleFileNameA(NULL, path, sizeof(path) - 1);
		std::string dir = path;
		size_t pos = dir.find_last_of("\\/");
		if (pos != std::string::npos)
			dir = dir.substr(0, pos + 1);
		return dir;
	}

	static bool IsAbsolutePath(const std::string& p)
	{
		return (p.size() >= 2 && p[1] == ':') || (p.size() >= 2 && p[0] == '\\' && p[1] == '\\');
	}

	/** wait for the given process to exit, up to timeoutSec seconds. */
	static void WaitForProcessExit(DWORD pid, int timeoutSec)
	{
		if (pid == 0)
			return;
		HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, pid);
		if (h == NULL)
		{
			Log("waitPid=%d already exited (OpenProcess err=%d)", pid, GetLastError());
			return;
		}
		Log("waiting for game process %d to exit (timeout %ds)...", pid, timeoutSec);
		DWORD waitMs = timeoutSec > 0 ? (DWORD)timeoutSec * 1000 : INFINITE;
		DWORD r = WaitForSingleObject(h, waitMs);
		if (r == WAIT_OBJECT_0)
			Log("process %d exited", pid);
		else if (r == WAIT_TIMEOUT)
			Log("warning: timed out waiting for process %d, continue anyway", pid);
		else
			Log("warning: WaitForSingleObject returned %d (err=%d)", r, GetLastError());
		CloseHandle(h);
		// file handles may be released slightly after process exit
		Sleep(500);
	}

	static bool IsResidualEngineName(const std::string& exeNameLower)
	{
		return exeNameLower == "paraengineclient.exe" || exeNameLower == "paracraft.mem.exe";
	}

	/** kill leftover engine processes whose image is inside the install dir. */
	static int KillResidualEngines(const std::set<DWORD>& excludePids, const char* selfExeName, const char* launcherExeName)
	{
		int killed = 0;
		std::string selfLower = ToLower(selfExeName ? selfExeName : "");
		std::string launcherLower = ToLower(launcherExeName ? launcherExeName : "");
		std::string rootLower = ToLower(g_rootDir);

		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snap == INVALID_HANDLE_VALUE)
		{
			Log("warning: CreateToolhelp32Snapshot failed err=%d", GetLastError());
			return 0;
		}
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(pe);
		if (Process32First(snap, &pe))
		{
			do
			{
				DWORD pid = pe.th32ProcessID;
				if (excludePids.count(pid) > 0)
					continue;
				std::string exeLower = ToLower(pe.szExeFile);
				if (exeLower == selfLower || exeLower == launcherLower)
					continue;
				if (!IsResidualEngineName(exeLower))
					continue;
				// verify the process image is inside our install dir, so we never
				// kill a game instance of another channel/install on the same PC
				std::string imgLower;
				HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE, FALSE, pid);
				if (hProc)
				{
					char img[MAX_PATH * 2] = { 0 };
					DWORD size = sizeof(img) - 1;
					if (QueryFullProcessImageNameA(hProc, 0, img, &size))
						imgLower = ToLower(img);
					if (imgLower.empty() || imgLower.compare(0, rootLower.size(), rootLower) != 0)
					{
						CloseHandle(hProc);
						continue;
					}
					if (TerminateProcess(hProc, 0))
					{
						Log("killed residual engine process %d (%s)", pid, pe.szExeFile);
						++killed;
					}
					else
					{
						Log("warning: failed to kill process %d err=%d", pid, GetLastError());
					}
					CloseHandle(hProc);
				}
			} while (Process32Next(snap, &pe));
		}
		CloseHandle(snap);
		return killed;
	}

	/** recursively clear the read-only attribute of every file under dir. */
	static int ClearReadOnlyRecursive(const std::string& dir, int depth = 0)
	{
		if (depth > 32)
			return 0;
		int count = 0;
		std::string pattern = dir + "*";
		WIN32_FIND_DATAA fd;
		HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
		if (h == INVALID_HANDLE_VALUE)
			return 0;
		do
		{
			if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
				continue;
			std::string full = dir + fd.cFileName;
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				count += ClearReadOnlyRecursive(full + "\\", depth + 1);
			}
			else if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			{
				if (SetFileAttributesA(full.c_str(), fd.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY))
					++count;
			}
		} while (FindNextFileA(h, &fd));
		FindClose(h);
		return count;
	}

	static std::string ReadTextFile(const std::string& path)
	{
		FILE* fp = fopen(path.c_str(), "rb");
		if (!fp)
			return "";
		std::string content;
		char buf[4096];
		size_t n;
		while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
			content.append(buf, n);
		fclose(fp);
		return content;
	}

	static const char* GetSelfExeName()
	{
		static char name[MAX_PATH] = { 0 };
		if (name[0] == 0)
		{
			char path[MAX_PATH * 2] = { 0 };
			GetModuleFileNameA(NULL, path, sizeof(path) - 1);
			std::string p = path;
			size_t pos = p.find_last_of("\\/");
			snprintf(name, sizeof(name) - 1, "%s", (pos != std::string::npos) ? p.c_str() + pos + 1 : p.c_str());
		}
		return name;
	}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
{
	using namespace ParaApplyWaiter;

	g_rootDir = GetModuleDir();
	SetCurrentDirectoryA(g_rootDir.c_str());

	std::string cmdLine = lpCmdLine ? lpCmdLine : "";
	std::string launcherExe = CmdParams::Get(cmdLine, "launcherExe", "ParaCraft.exe");
	bool cleanMode = ToLower(CmdParams::Get(cmdLine, "cleanMode", "false")) == "true";
	DWORD waitPid = (DWORD)atoi(CmdParams::Get(cmdLine, "waitPid", "0").c_str());
	DWORD excludePid = (DWORD)atoi(CmdParams::Get(cmdLine, "excludePid", "0").c_str());
	int waitTimeoutSec = atoi(CmdParams::Get(cmdLine, "waitTimeoutSec", "180").c_str());
	std::string cmdFile = CmdParams::Get(cmdLine, "cmdFile", "caches/430applyCmd.txt");

	Log("started, cmdline: %s", cmdLine.c_str());

	std::set<DWORD> excludePids;
	excludePids.insert(GetCurrentProcessId());
	if (excludePid != 0)
		excludePids.insert(excludePid);

	if (cleanMode)
	{
		// startup self-heal mode: kill residual engine processes and clear
		// read-only attributes, then exit. The calling game process is excluded.
		int killed = KillResidualEngines(excludePids, GetSelfExeName(), launcherExe.c_str());
		int cleared = ClearReadOnlyRecursive(g_rootDir);
		Log("cleanMode: killed %d residual process(es), cleared read-only on %d file(s)", killed, cleared);
		return 0;
	}

	// normal mode: wait for the game process to fully exit first
	if (waitPid != 0)
		WaitForProcessExit(waitPid, waitTimeoutSec);

	// as a safeguard, kill any leftover engine processes still holding files
	int killed = KillResidualEngines(excludePids, GetSelfExeName(), launcherExe.c_str());
	if (killed > 0)
		Sleep(500);

	// clear read-only so the launcher can always overwrite
	int cleared = ClearReadOnlyRecursive(g_rootDir);
	if (cleared > 0)
		Log("cleared read-only on %d file(s)", cleared);

	// read the launcher argument string prepared by the game
	if (!IsAbsolutePath(cmdFile))
		cmdFile = g_rootDir + cmdFile;
	std::string args = Trim(ReadTextFile(cmdFile));
	if (args.empty())
		Log("warning: cmd file %s missing or empty, launching with empty args", cmdFile.c_str());

	std::string launcherPath = IsAbsolutePath(launcherExe) ? launcherExe : (g_rootDir + launcherExe);

	std::string fullCmd = "\"" + launcherPath + "\"";
	if (!args.empty())
		fullCmd += " " + args;

	Log("starting launcher: %s", fullCmd.c_str());

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	std::vector<char> cmdBuf(fullCmd.begin(), fullCmd.end());
	cmdBuf.push_back('\0');

	if (!CreateProcessA(NULL, cmdBuf.data(), NULL, NULL, FALSE, 0, NULL, g_rootDir.c_str(), &si, &pi))
	{
		DWORD err = GetLastError();
		Log("error: CreateProcess failed err=%d", err);
		char msg[1024];
		snprintf(msg, sizeof(msg) - 1, "Failed to start %s (error %d). Please restart the application.", launcherPath.c_str(), err);
		MessageBoxA(NULL, msg, "ParaCraft Update", MB_OK | MB_ICONERROR);
		return 2;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	Log("launcher started, waiter exits");
	return 0;
}
