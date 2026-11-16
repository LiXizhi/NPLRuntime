#include "ParaEngine.h"
#undef ShellExecute
#undef CreateProcess
#include "Globals.h"
#include "ParaEngineSettings.h"
#include "Framework/Common/Helper/EditorHelper.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#elif !defined(PARAENGINE_MOBILE)
#include <cstdlib>
#include <cstring>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/pipe.hpp>
#include <boost/process/v1/search_path.hpp>
#include <boost/process/v1/start_dir.hpp>
#include <boost/process/v1/args.hpp>
#if !defined(WIN32)
#include <unistd.h>
#endif
#endif

namespace ParaEngine {

	bool CEditorHelper::CreateEmptyCharacterEventFile(const char* scriptFile, const char* sName)
	{
		return CreateEmptyCharacterEventFileShared(scriptFile, sName);
	}

	bool CEditorHelper::OpenWithDefaultEditor(const char* sScriptFile, bool bWaitOnReturn)
	{
#if !defined(__EMSCRIPTEN__) && !defined(PARAENGINE_MOBILE)
		if (!sScriptFile || sScriptFile[0] == '\0')
			return false;

		namespace bp = boost::process::v1;
		std::string editor = CGlobals::GetSettings().GetScriptEditor();
#if !defined(WIN32)
		if (editor.empty() || editor.find(".exe") != std::string::npos)
		{
			const char* visualEnv = std::getenv("VISUAL");
			const char* editorEnv = std::getenv("EDITOR");
			if (visualEnv && visualEnv[0] != '\0')
				editor = visualEnv;
			else if (editorEnv && editorEnv[0] != '\0')
				editor = editorEnv;
			else
				editor = "xdg-open";
		}
#endif
		try
		{
			boost::filesystem::path editorPath = editor;
			if (!editorPath.is_absolute())
			{
				auto found = bp::search_path(editor);
				if (!found.empty())
					editorPath = found;
			}
			if (editorPath.empty())
				return false;

			bp::child c(editorPath, bp::args({ std::string(sScriptFile) }));
			if (bWaitOnReturn)
				c.wait();
			else
				c.detach();
			return true;
		}
		catch (const std::exception& e)
		{
			OUTPUT_LOG("warning: OpenWithDefaultEditor failed for %s: %s\n", sScriptFile, e.what());
		}
#endif
		return false;
	}

	bool CEditorHelper::CreateProcess(const char* lpApplicationName, const char* lpCommandLine, bool bWaitOnReturn)
	{
#if !defined(__EMSCRIPTEN__) && !defined(PARAENGINE_MOBILE)
		OUTPUT_LOG("warning: security alert. CreateProcess function should be deprecated in formal release\n");
		std::string app = lpApplicationName ? lpApplicationName : "";
		std::string cmdLine = lpCommandLine ? lpCommandLine : "";
		if (app.empty() && cmdLine.empty())
			return false;

		namespace bp = boost::process::v1;
		try
		{
			std::string fullCmd;
			if (!app.empty() && cmdLine.empty())
				fullCmd = "\"" + app + "\"";
			else if (!app.empty() && cmdLine.find(app) != 0 && cmdLine.find("\"" + app + "\"") != 0)
				fullCmd = "\"" + app + "\" " + cmdLine;
			else
				fullCmd = cmdLine;

			if (bWaitOnReturn)
			{
				bp::ipstream outStream;
#if defined(WIN32)
				bp::child c(fullCmd, bp::std_out > outStream, bp::std_err > outStream);
#else
				bp::child c("/bin/sh", bp::args({ std::string("-c"), fullCmd }), bp::std_out > outStream, bp::std_err > outStream);
#endif
				std::string line;
				while (std::getline(outStream, line))
				{
					OUTPUT_LOG("%s\n", line.c_str());
				}
				c.wait();
			}
			else
			{
#if defined(WIN32)
				bp::child c(fullCmd);
#else
				bp::child c("/bin/sh", bp::args({ std::string("-c"), fullCmd }));
#endif
				c.detach();
			}
			return true;
		}
		catch (const std::exception& e)
		{
			OUTPUT_LOG("error: CreateProcess failed: %s\n", e.what());
		}
#endif
		return false;
	}

	bool CEditorHelper::ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{
		if (!lpFile || lpFile[0] == '\0')
			return false;
		std::string op = (lpOperation && lpOperation[0] != '\0') ? lpOperation : "open";

#ifdef __EMSCRIPTEN__
		if (op == "open" || op == "openExternalBrowser")
		{
			EM_ASM({
				const url = UTF8ToString($0);
				console.log('open ' + url);
				window.open(url);
			}, lpFile);
			return true;
		}
		return false;
#elif !defined(PARAENGINE_MOBILE)
		namespace bp = boost::process::v1;
		std::string fileStr = lpFile;
		std::string paramsStr = lpParameters ? lpParameters : "";
		std::string dirStr = lpDirectory ? lpDirectory : "";

		bool isWait = (op == "wait");
		bool isRunAs = (op == "runas");
		bool isUrl = (fileStr.find("http://") == 0 || fileStr.find("https://") == 0 || fileStr.find("://") != std::string::npos);

#if !defined(WIN32)
		if (fileStr == "explorer.exe" || fileStr == "explorer")
		{
			auto opener = bp::search_path("xdg-open");
			if (opener.empty())
				return false;
			std::string target = paramsStr.empty() ? (dirStr.empty() ? "." : dirStr) : paramsStr;
			// Strip "/select," prefix if passed from Windows-style caller
			if (target.find("/select,") == 0)
				target = target.substr(8);
			if (target.size() >= 2 && target.front() == '"' && target.back() == '"')
				target = target.substr(1, target.size() - 2);
			try
			{
				bp::child c(opener, bp::args({ target }));
				c.detach();
				return true;
			}
			catch (const std::exception& e)
			{
				OUTPUT_LOG("warning: ShellExecute explorer->xdg-open failed: %s\n", e.what());
				return false;
			}
		}

		if (op == "openExternalBrowser" || ((op == "open" || op == "explore") && isUrl))
		{
			auto opener = bp::search_path("xdg-open");
			if (opener.empty())
				opener = bp::search_path("sensible-browser");
			if (opener.empty())
				return false;
			try
			{
				bp::child c(opener, bp::args({ fileStr }));
				c.detach();
				return true;
			}
			catch (const std::exception& e)
			{
				OUTPUT_LOG("warning: ShellExecute URL open failed: %s\n", e.what());
				return false;
			}
		}
#endif

		try
		{
			bool isShellScript = (fileStr.size() > 3 && fileStr.substr(fileStr.size() - 3) == ".sh");
			boost::filesystem::path targetPath = fileStr;
			if (!dirStr.empty() && !targetPath.is_absolute() && boost::filesystem::exists(boost::filesystem::path(dirStr) / targetPath))
			{
				targetPath = boost::filesystem::path(dirStr) / targetPath;
			}

#if !defined(WIN32)
			// If opening a folder or non-executable document with "open" / "explore" / "edit", delegate to xdg-open
			if (!isWait && !isRunAs && !isShellScript && paramsStr.empty() &&
				(boost::filesystem::is_directory(targetPath) ||
				 (boost::filesystem::exists(targetPath) && ::access(targetPath.c_str(), X_OK) != 0)))
			{
				auto opener = bp::search_path("xdg-open");
				if (!opener.empty())
				{
					bp::child c(opener, bp::args({ targetPath.string() }));
					c.detach();
					return true;
				}
			}
#endif

			std::string cmd = "\"" + targetPath.string() + "\"";
			if (!paramsStr.empty())
				cmd += " " + paramsStr;
#if !defined(WIN32)
			if (isShellScript)
				cmd = "/bin/sh " + cmd;
			if (isRunAs && ::geteuid() != 0)
			{
				auto pkexec = bp::search_path("pkexec");
				if (!pkexec.empty())
					cmd = "\"" + pkexec.string() + "\" /bin/sh -c " + "'" + cmd + "'";
			}
			bp::child c = dirStr.empty()
				? bp::child("/bin/sh", bp::args({ std::string("-c"), cmd }))
				: bp::child("/bin/sh", bp::args({ std::string("-c"), cmd }), bp::start_dir(dirStr));
#else
			bp::child c = dirStr.empty()
				? bp::child(cmd)
				: bp::child(cmd, bp::start_dir(dirStr));
#endif
			if (isWait)
				c.wait();
			else
				c.detach();
			return true;
		}
		catch (const std::exception& e)
		{
			OUTPUT_LOG("warning: ShellExecute failed for %s: %s\n", lpFile, e.what());
		}
#endif
		return false;
	}

} // end namespace
