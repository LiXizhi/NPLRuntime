#pragma once

namespace ParaScripting
{
	using namespace ParaEngine;

	/**
	* @ingroup ParaUI
	* a HTML browser control and texture
	*/
	class PE_CORE_DECL ParaHTMLBrowser
	{
	public:
		ParaHTMLBrowser(){ m_nID = -1; };
		ParaHTMLBrowser(int nID){ m_nID = nID; };

		int m_nID;

		/**
		* check if the object is valid
		*/
		bool IsValid();

		/** get the window name. */
		const char* GetName();

		/** the last url when navigateTo() is called. default is ""*/
		const char* GetLastNavURL();

		/** something like delete this*/
		void Release();

		bool setSize(int widthIn, int heightIn);
		// current browser width (can vary slightly after page is rendered)
		int getBrowserWidth();
		// current height
		int getBrowserHeight();
		bool scrollByLines(int linesIn);
		bool setBackgroundColor(int red, int green, int blue);
		bool setEnabled(bool enabled);

		// navigation - self explanatory
		bool navigateTo(const char* url);
		bool navigateStop();
		bool canNavigateBack();
		bool navigateBack();
		bool canNavigateForward();
		bool navigateForward();

		// mouse/keyboard interaction
		bool mouseDown(int xPos, int yPos);		// send a mouse down event to a browser window at given XY in browser space
		bool mouseUp(int xPos, int yPos);			// send a mouse up event to a browser window at given XY in browser space
		bool mouseMove(int xPos, int yPos);		// send a mouse move event to a browser window at given XY in browser space
		bool keyPress(int keyCode);				// send a key press event to a browser window 
		bool focusBrowser(bool focusBrowser);		// set/remove focus to given browser window
	};
}
