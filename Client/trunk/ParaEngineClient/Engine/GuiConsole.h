#pragma once

namespace ParaEngine
{
	/** allocate a console with redirected stdin/stdout/stderr, mostly for script debugging. 
	* This function can be called many times, but only the first time takes effect. 
	*/
	void RedirectIOToConsole();

	/**	Sets the attributes of characters written to the console screen buffer by the WriteFile  or WriteConsole  function, or echoed by the ReadFile  or ReadConsole  function. 
	* This function affects text written after the function call. 
	* @param wAttributes: BitWise fields. please see SetConsoleTextAttribute for a list of font color and styles. 
		#define FOREGROUND_BLUE      0x0001 // text color contains blue.
		#define FOREGROUND_GREEN     0x0002 // text color contains green.
		#define FOREGROUND_RED       0x0004 // text color contains red.
		#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
		#define BACKGROUND_BLUE      0x0010 // background color contains blue.
		#define BACKGROUND_GREEN     0x0020 // background color contains green.
		#define BACKGROUND_RED       0x0040 // background color contains red.
		#define BACKGROUND_INTENSITY 0x0080 // background color is intensified.
		#define COMMON_LVB_LEADING_BYTE    0x0100 // Leading Byte of DBCS
		#define COMMON_LVB_TRAILING_BYTE   0x0200 // Trailing Byte of DBCS
		#define COMMON_LVB_GRID_HORIZONTAL 0x0400 // DBCS: Grid attribute: top horizontal.
		#define COMMON_LVB_GRID_LVERTICAL  0x0800 // DBCS: Grid attribute: left vertical.
		#define COMMON_LVB_GRID_RVERTICAL  0x1000 // DBCS: Grid attribute: right vertical.
		#define COMMON_LVB_REVERSE_VIDEO   0x4000 // DBCS: Reverse fore/back ground attribute.
		#define COMMON_LVB_UNDERSCORE      0x8000 // DBCS: Underscore.
		#define COMMON_LVB_SBCSDBCS        0x0300 // SBCS or DBCS flag
	*/
	bool SetOutputConsoleTextAttribute(WORD wAttributes);
}
