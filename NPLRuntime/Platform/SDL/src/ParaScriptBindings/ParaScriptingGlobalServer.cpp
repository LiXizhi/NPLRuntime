//-----------------------------------------------------------------------------
// Class: For ParaGlobal namespace (SDL2 / Linux / Web).
// Authors: Big
// Emails: onedous@gmail.com
// Date: 2019.8.12
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaScriptingGlobal.h"
#include "NPL/NPLScriptingState.h"

#if defined(__linux__) && !defined(ANDROID) && !defined(EMSCRIPTEN) && !defined(NPLRUNTIME_SERVER)
#include <cstdio>
#include <string>
#endif

bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
#if defined(__linux__) && !defined(ANDROID) && !defined(EMSCRIPTEN) && !defined(NPLRUNTIME_SERVER)
	if (luabind::type(inout) != LUA_TTABLE)
		return false;

	std::string title;
	std::string initialdir;
	std::string filter;
	bool isSave = false;

	if (luabind::type(inout["title"]) == LUA_TSTRING)
	{
		const char* s = luabind::object_cast<const char*>(inout["title"]);
		if (s) title = s;
	}
	if (luabind::type(inout["initialdir"]) == LUA_TSTRING)
	{
		const char* s = luabind::object_cast<const char*>(inout["initialdir"]);
		if (s) initialdir = s;
	}
	if (luabind::type(inout["filter"]) == LUA_TSTRING)
	{
		const char* s = luabind::object_cast<const char*>(inout["filter"]);
		if (s) filter = s;
	}
	if (luabind::type(inout["save"]) == LUA_TBOOLEAN)
	{
		isSave = luabind::object_cast<bool>(inout["save"]);
	}
	else if (luabind::type(inout["issave"]) == LUA_TBOOLEAN)
	{
		isSave = luabind::object_cast<bool>(inout["issave"]);
	}

	auto shellQuote = [](const std::string& s) -> std::string {
		std::string out = "'";
		for (char c : s)
		{
			if (c == '\'')
				out += "'\\''";
			else
				out += c;
		}
		out += "'";
		return out;
	};

	std::string cmd = "zenity --file-selection";
	if (isSave)
		cmd += " --save --confirm-overwrite";
	if (!title.empty())
		cmd += " --title=" + shellQuote(title);
	if (!initialdir.empty())
	{
		std::string dir = initialdir;
		if (dir.back() != '/')
			dir += '/';
		cmd += " --filename=" + shellQuote(dir);
	}
	if (filter == "image/*")
	{
		cmd += " --file-filter='Images | *.png *.jpg *.jpeg *.bmp *.webp *.gif'";
	}
	cmd += " 2>/dev/null";

	std::string selected;
	FILE* fp = popen(cmd.c_str(), "r");
	if (fp)
	{
		char buf[1024];
		while (fgets(buf, sizeof(buf), fp) != nullptr)
			selected.append(buf);
		if (pclose(fp) != 0)
			selected.clear();
	}

	if (selected.empty())
	{
		cmd = isSave ? "kdialog --getsavefilename" : "kdialog --getopenfilename";
		if (!initialdir.empty())
			cmd += " " + shellQuote(initialdir);
		else
			cmd += " .";
		if (!title.empty())
			cmd += " --title " + shellQuote(title);
		cmd += " 2>/dev/null";

		fp = popen(cmd.c_str(), "r");
		if (fp)
		{
			char buf[1024];
			while (fgets(buf, sizeof(buf), fp) != nullptr)
				selected.append(buf);
			if (pclose(fp) != 0)
				selected.clear();
		}
	}

	while (!selected.empty() && (selected.back() == '\n' || selected.back() == '\r'))
		selected.pop_back();

	if (!selected.empty())
	{
		inout["result"] = true;
		inout["filename"] = selected.c_str();
		return true;
	}
#endif
	return false;
}

void ParaScripting::CNPLScriptingState::LoadHAPI_Platform()
{
}
