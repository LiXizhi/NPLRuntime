#pragma once

#ifdef USE_FLASH_MANAGER
namespace ParaScripting
{
	using namespace ParaEngine;
	/**
	* @ingroup ParaUI
	* a flash player
	*/
	class PE_CORE_DECL ParaFlashPlayer 
	{
	public:
		ParaFlashPlayer(){m_nFlashPlayerIndex=-1;};
		ParaFlashPlayer(int nFlashPlayerIndex){m_nFlashPlayerIndex=nFlashPlayerIndex;};

		int m_nFlashPlayerIndex;

		/**
		* check if the object is valid
		*/
		bool IsValid();

		/** get texture */
		const char* GetFileName();

		/** get index from */
		int GetIndex() {return m_nFlashPlayerIndex;};

		/** whether the current player is free to load a new movie. */
		bool IsFree();

		/** load a movie by file name, it can be URL, a virtual file, a file, or a resource */
		void LoadMovie(const char* filename);

		/** unload current movie. However d3d texture and the flash player window are not released for reuse.*/
		void UnloadMovie();

		/** put flash variables. This function must be loaded before a flash movie is loaded. 
		* @param args: such as "company=http://paraengine.com"
		*/
		void PutFlashVars(const char* args);

		/**
		[ ActionScript ] 
		import flash.external.*; 
		ExternalInterface.addCallback("CallMeFromApplication", this, InternalFunction); 

		function InternalFunction(str: string): string { 
		TextArea1.text = str; 
		return "The function was called successfully"; 
		} 
		[ C++ ] 
		string res = CallFlashFunction("<invoke name=\"CallMeFromApplication\" returntype=\"xml\"><arguments><string>Some text for FlashPlayerControl</string></arguments></invoke>"); 
		@param res: result is returned as a string. 
		@note: the max return string length of the flash function can not exceed 2048 bytes.
		@remark: one can also execute NPL script from flash script. Such as below. 
		[ ActionScript ]
		on (click) {   
		_root.TextArea1.text = flash.external.ExternalInterface.call("SomeNPLFunction(parameters)");
		}

		Please note that the return value must be explicitly set in the NPL script, by calling SetFlashReturnValue() function. 
		Otherwise empty string is returned. 

		[Security alert]: must be run in sand-box environment or there is a security here. Plus this function must be called from the same thread as ParaEngine main thread.  
		*/
		const char* CallFlashFunction(const char* sFunc);

		/** This function is called from NPL script to set the return value of an NPL function which can be called by the flash external interface call. 
		* @param sResult: xml encoded string. such as "<string>Result from NPL</string>". if NULL, it is set to "<string></string>"
		*/
		void SetFlashReturnValue(const char* sResult);

		/** release d3d texture.*/
		void InvalidateDeviceObjects();

		/**
		* get texture info
		* in NPL, it is as below
		*	local width, height;
		*	self:GetTextureInfo(width, height);
		* @param width [in|out] 
		* @param height [in|out]
		* @return 
		*/
		void GetTextureInfo(int widthIn, int heightIn, int& width, int& height);

		/** whether this player is using a real win32 window for display. */
		bool IsWindowMode();

		/** set whether this player is using a real win32 window for display. Default to false. */
		void SetWindowMode(bool bIsWindowMode);

		/** move or resize the window. */
		void MoveWindow(int x, int y, int width, int height);

		/**
		* send a windows message to the inner flash player window.
		* in most cases: we will forward mouse and key messages to the flash player using this function.
		* @param Msg: 
		*	Msg = 0: single click, wParam contains the relative X position and lParam contains the relative Y position.
		*		it also set the key focus window to this flash window. 
		*		Because the game engine app always captures the mouse, the single click function will actually 
		*		send a pair of Left button down and up message to the flash control. 
		*		The first click set the mouse focus and the second performs the actual click. Then the flash control will soon lose mouse capture to the game engine.
		*/
		int SendMessage( unsigned int Msg,DWORD wParam,DWORD lParam );
	};
}
#endif