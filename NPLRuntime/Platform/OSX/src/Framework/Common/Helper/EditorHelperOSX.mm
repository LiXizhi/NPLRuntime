#import <Cocoa/Cocoa.h>
#include "ParaEngine.h"
#undef ShellExecute
#undef CreateProcess
#include "Globals.h"
#include "ParaEngineSettings.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "WebView/WebView.h"
#include <cstdlib>
#include <cstring>

namespace ParaEngine {

	bool CEditorHelper::CreateEmptyCharacterEventFile(const char* scriptFile, const char* sName)
	{
		return CreateEmptyCharacterEventFileShared(scriptFile, sName);
	}

	bool CEditorHelper::OpenWithDefaultEditor(const char* sScriptFile, bool bWaitOnReturn)
	{
		if (!sScriptFile || sScriptFile[0] == '\0')
			return false;

		NSString* nsFile = [NSString stringWithUTF8String:sScriptFile];
		if (!nsFile)
			return false;

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
				editor.clear();
		}

		@try
		{
			NSTask* task = [[NSTask alloc] init];
			if (!editor.empty())
			{
				[task setLaunchPath:@"/usr/bin/env"];
				NSString* nsEditor = [NSString stringWithUTF8String:editor.c_str()];
				[task setArguments:@[nsEditor, nsFile]];
			}
			else
			{
				[task setLaunchPath:@"/usr/bin/open"];
				if (bWaitOnReturn)
					[task setArguments:@[@"-W", @"-t", nsFile]];
				else
					[task setArguments:@[@"-t", nsFile]];
			}
			[task launch];
			if (bWaitOnReturn)
				[task waitUntilExit];
			return true;
		}
		@catch (NSException* ex)
		{
			OUTPUT_LOG("warning: OpenWithDefaultEditor failed on macOS: %s\n", [[ex reason] UTF8String]);
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

		std::string fullCmd;
		if (!app.empty() && cmdLine.empty())
			fullCmd = "\"" + app + "\"";
		else if (!app.empty() && cmdLine.find(app) != 0 && cmdLine.find("\"" + app + "\"") != 0)
			fullCmd = "\"" + app + "\" " + cmdLine;
		else
			fullCmd = cmdLine;

		NSString* nsCmd = [NSString stringWithUTF8String:fullCmd.c_str()];
		if (!nsCmd)
			return false;

		@try
		{
			NSTask* task = [[NSTask alloc] init];
			[task setLaunchPath:@"/bin/sh"];
			[task setArguments:@[@"-c", nsCmd]];

			if (bWaitOnReturn)
			{
				NSPipe* pipe = [NSPipe pipe];
				[task setStandardOutput:pipe];
				[task setStandardError:pipe];
				NSFileHandle* readHandle = [pipe fileHandleForReading];
				[task launch];
				NSData* data = [readHandle readDataToEndOfFile];
				[task waitUntilExit];
				if (data && [data length] > 0)
				{
					NSString* output = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
					if (output)
						OUTPUT_LOG("%s", [output UTF8String]);
				}
			}
			else
			{
				[task launch];
			}
			return true;
		}
		@catch (NSException* ex)
		{
			OUTPUT_LOG("error: CreateProcess failed on macOS: %s\n", [[ex reason] UTF8String]);
		}
		return false;
	}

	static bool executeOnOSX(const char* lpFile, const char* lpParameters, const char* lpDirectory, bool isWait)
	{
		if (!lpFile || lpFile[0] == '\0')
			return false;

		std::string fileStr = lpFile;
		std::string paramsStr = lpParameters ? lpParameters : "";
		std::string dirStr = lpDirectory ? lpDirectory : "";

		if (fileStr == "explorer.exe" || fileStr == "explorer")
		{
			std::string target = paramsStr.empty() ? (dirStr.empty() ? "." : dirStr) : paramsStr;
			bool selectMode = false;
			if (target.find("/select,") == 0)
			{
				selectMode = true;
				target = target.substr(8);
			}
			if (target.size() >= 2 && target.front() == '"' && target.back() == '"')
				target = target.substr(1, target.size() - 2);
			NSString* nsTarget = [NSString stringWithUTF8String:target.c_str()];
			if (!nsTarget)
				return false;
			if (selectMode)
				return [[NSWorkspace sharedWorkspace] selectFile:nsTarget inFileViewerRootedAtPath:@""];
			return [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:nsTarget]];
		}

		bool isShellScript = (fileStr.size() > 3 && fileStr.substr(fileStr.size() - 3) == ".sh");
		NSString* nsFile = [NSString stringWithUTF8String:fileStr.c_str()];
		if (!isWait && !isShellScript && paramsStr.empty() && nsFile)
		{
			BOOL isDir = NO;
			if ([[NSFileManager defaultManager] fileExistsAtPath:nsFile isDirectory:&isDir])
			{
				if (isDir || ![[NSFileManager defaultManager] isExecutableFileAtPath:nsFile])
				{
					return [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:nsFile]];
				}
			}
		}

		std::string cmd = "\"" + fileStr + "\"";
		if (!paramsStr.empty())
			cmd += " " + paramsStr;
		if (isShellScript)
			cmd = "/bin/sh " + cmd;

		NSString* nsCmd = [NSString stringWithUTF8String:cmd.c_str()];
		if (!nsCmd)
			return false;

		@try
		{
			NSTask* task = [[NSTask alloc] init];
			[task setLaunchPath:@"/bin/sh"];
			[task setArguments:@[@"-c", nsCmd]];
			if (!dirStr.empty())
			{
				NSString* nsDir = [NSString stringWithUTF8String:dirStr.c_str()];
				if (nsDir)
					[task setCurrentDirectoryPath:nsDir];
			}
			[task launch];
			if (isWait)
				[task waitUntilExit];
			return true;
		}
		@catch (NSException* ex)
		{
			OUTPUT_LOG("warning: ShellExecute failed on macOS: %s\n", [[ex reason] UTF8String]);
		}
		return false;
	}

	bool CEditorHelper::ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{
		if (!lpFile || lpFile[0] == '\0')
			return false;
		std::string op = (lpOperation && lpOperation[0] != '\0') ? lpOperation : "open";

		if (op == "open" || op == "explore" || op == "edit")
		{
			auto len = strlen(lpFile);
			bool isHttpUrl = (len > 7 && strncmp(lpFile, "http://", 7) == 0)
				|| (len > 8 && strncmp(lpFile, "https://", 8) == 0);

			if (isHttpUrl && op == "open")
			{
				// The opt-out applies only to this open of the shared fullscreen view.
				bool hideReturnButton = lpParameters && strcmp(lpParameters, "HideCloseButton=true") == 0;
				ParaEngineWebView::openWebView(lpFile, hideReturnButton);
				return true;
			}
			return executeOnOSX(lpFile, lpParameters, lpDirectory, false);
		}
		else if (op == "wait" || op == "runas")
		{
			return executeOnOSX(lpFile, lpParameters, lpDirectory, op == "wait");
		}
		else if (op == "openExternalBrowser")
		{
			NSString* sUrl = [NSString stringWithUTF8String:lpFile];
			if (!sUrl)
				sUrl = [NSString stringWithCString:lpFile encoding:[NSString defaultCStringEncoding]];
			if (sUrl)
			{
				NSURL* nsUrl = [NSURL URLWithString:sUrl];
				if (!nsUrl)
				{
					NSString* encoded = [sUrl stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLFragmentAllowedCharacterSet]];
					if (encoded)
						nsUrl = [NSURL URLWithString:encoded];
				}
				if (nsUrl)
					return [[NSWorkspace sharedWorkspace] openURL:nsUrl];
			}
			return false;
		}

		return false;
	}
} // end namespace
