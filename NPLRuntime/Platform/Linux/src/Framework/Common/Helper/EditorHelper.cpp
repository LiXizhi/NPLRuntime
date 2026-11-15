#include "ParaEngine.h"
#undef ShellExecute
#undef CreateProcess
#include "Globals.h"
#include "ParaEngineSettings.h"
#include "Framework/Common/Helper/EditorHelper.h"

#include <cstdlib>
#include <cstring>
#include <vector>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <boost/filesystem.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/pipe.hpp>
#include <boost/process/v1/search_path.hpp>
#include <boost/process/v1/start_dir.hpp>
#include <boost/process/v1/args.hpp>

namespace ParaEngine {

	bool CEditorHelper::CreateEmptyCharacterEventFile(const char* scriptFile, const char* sName)
	{
		return CreateEmptyCharacterEventFileShared(scriptFile, sName);
	}

	bool CEditorHelper::OpenWithDefaultEditor(const char* sScriptFile, bool bWaitOnReturn)
	{
		if (!sScriptFile || sScriptFile[0] == '\0')
			return false;

		namespace bp = boost::process::v1;
		std::string editor = CGlobals::GetSettings().GetScriptEditor();
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
		return false;
	}

	bool CEditorHelper::CreateProcess(const char* lpApplicationName, const char* lpCommandLine, bool bWaitOnReturn)
	{
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
				bp::child c("/bin/sh", bp::args({ std::string("-c"), fullCmd }), bp::std_out > outStream, bp::std_err > outStream);
				std::string line;
				while (std::getline(outStream, line))
				{
					OUTPUT_LOG("%s\n", line.c_str());
				}
				c.wait();
			}
			else
			{
				bp::child c("/bin/sh", bp::args({ std::string("-c"), fullCmd }));
				c.detach();
			}
			return true;
		}
		catch (const std::exception& e)
		{
			OUTPUT_LOG("error: CreateProcess failed: %s\n", e.what());
		}
		return false;
	}

	bool CEditorHelper::ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{
		if (!lpFile || lpFile[0] == '\0')
			return false;

		namespace bp = boost::process::v1;
		std::string op = (lpOperation && lpOperation[0] != '\0') ? lpOperation : "open";
		std::string fileStr = lpFile;
		std::string paramsStr = lpParameters ? lpParameters : "";
		std::string dirStr = lpDirectory ? lpDirectory : "";
		bool isWait = (op == "wait");

		try
		{
			std::string cmd = "\"" + fileStr + "\"";
			if (!paramsStr.empty())
				cmd += " " + paramsStr;
			if (fileStr.size() > 3 && fileStr.substr(fileStr.size() - 3) == ".sh")
				cmd = "/bin/sh " + cmd;

			bp::child c = dirStr.empty()
				? bp::child("/bin/sh", bp::args({ std::string("-c"), cmd }))
				: bp::child("/bin/sh", bp::args({ std::string("-c"), cmd }), bp::start_dir(dirStr));
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
		return false;
	}

} // end namespace
