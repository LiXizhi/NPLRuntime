#pragma once
#include "ParaScriptingHTMLBrowser.h"

namespace ParaScripting
{
	using namespace luabind;
	using namespace ParaEngine;

	/**
	* @ingroup ParaUI
	* managing HTML browsers
	*/
	class PE_CORE_DECL ParaBrowserManager
	{
	public:
		/**
		* get ParaHTMLBrowser by name. this function does not create any player if there is no browser with the given name.
		*/
		static ParaHTMLBrowser GetBrowserWindow(const char* sFileName);

		/**
		* get ParaHTMLBrowser by nWindowID. this function does not create any player if there is no browser at the given index.
		*/
		static ParaHTMLBrowser GetBrowserWindow1(int nWindowID);

		/**
		* create a new browser window with the given name and dimension in pixels.
		*/
		static ParaHTMLBrowser createBrowserWindow(const char* sFileName, int browserWindowWidth, int browserWindowHeight);

		//////////////////////////////////////////////////////////////////////////
		// house keeping
		static bool reset();
		static bool clearCache();
		static int getLastError();
		static const char* getVersion();

		// how many browsers can be opened at the same time
		static int GetMaxWindowNum();
		static void SetMaxWindowNum(int nNum);

		//////////////////////////////////////////////////////////////////////////
		// event

		/**
		* set event handler. the scripting interface will receive a msg table for the following type
		*	msg={windowid=number, value=[ValueInt|ValueString|nil]}
		* where value is nil
		* @param strScriptName: format is "[neuronfile];sCode";
		*/
		static void onPageChanged(const object& strScriptName);
		/**
		* set event handler. the scripting interface will receive a msg table for the following type
		*	msg={windowid=number, value=[ValueInt|ValueString|nil]}
		* where value is a string
		* @param strScriptName: format is "[neuronfile];sCode";
		*/
		static void onNavigateBegin(const object& strScriptName);
		/**
		* set event handler. the scripting interface will receive a msg table for the following type
		*	msg={windowid=number, value=[ValueInt|ValueString|nil]}
		* where value is a string
		* @param strScriptName: format is "[neuronfile];sCode";
		*/
		static void onNavigateComplete(const object& strScriptName);
		/**
		* set event handler. the scripting interface will receive a msg table for the following type
		*	msg={windowid=number, value=[ValueInt|ValueString|nil]}
		* where value is a int of [0-100]
		* @param strScriptName: format is "[neuronfile];sCode";
		*/
		static void onUpdateProgress(const object& strScriptName);
		/**
		* set event handler. the scripting interface will receive a msg table for the following type
		*	msg={windowid=number, value=[ValueInt|ValueString|nil]}
		* where value is a string of status text
		* @param strScriptName: format is "[neuronfile];sCode";
		*/
		static void onStatusTextChange(const object& strScriptName);
		/**
		* set event handler. the scripting interface will receive a msg table for the following type
		*	msg={windowid=number, value=[ValueInt|ValueString|nil]}
		* where value is a string
		* @param strScriptName: format is "[neuronfile];sCode";
		*/
		static void onLocationChange(const object& strScriptName);
		/**
		* set event handler. the scripting interface will receive a msg table for the following type
		*	msg={windowid=number, value=[ValueInt|ValueString|nil]}
		* where value is a string of HRef
		* @param strScriptName: format is "[neuronfile];sCode";
		*/
		static void onClickLinkHref(const object& strScriptName);
	};
}