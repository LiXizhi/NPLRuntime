/********************************************************************
*********************************************************************
**  Utility
**  This file contain soem importants functions I use in all my 
**  progs
**
**
**
**  Enjoy to use it like you want ;)
**
**  Gaetan SEMET
**  
**  gaetan.semet@wanadoo.fr
**
*********************************************************************
*********************************************************************/

/* List of function:
LPVOID utilGetMem(int size, BOOL zero);
void utilFreeMem(LPVOID x);
PVOID utilReAllocMem(LPVOID x, int newsize, int zero);
BOOL utilReDim(void ***pointer, UINT NewCount, int size, int count);

LPSTR utilGetCommandParameter(LPSTR cmd, LPSTR parm);
void utilParseCommandLine(const char *cmdLine, char **argv,int	*argc);
void utilParseCommandLineInit(char** argv, int max_parameter_number, int max_parameter_size)
void utilParseCommandLineFree(char** argv, int max_parameter_number, int destroy_me)

BOOL utilRemBkSlash(LPSTR path, LPSTR buf, int BufLen);
BOOL utilAddBkSlash(LPSTR path, LPSTR buf, int BufLen);
int utilExtractFileName(LPSTR path, LPSTR buf, int BufLen);
int utilExtractFilePath(LPSTR path, LPSTR buf, int BufLen);
BOOL utilIsSubFolder(LPSTR SubPath, LPSTR Path);
int utilChangeFileExt(LPSTR FilePath, LPSTR NewExt, int BufLen);
int utilCopy (LPCSTR Str,int BegPos,int Len, LPSTR buf, int BufLen);
int utilRight(LPCSTR Str, int Len, LPSTR buf, int BufLen);

BOOL utilAnsiToUnicode(char *szA, WCHAR *szW);
BOOL utilUnicodeToAnsi(WCHAR *szW, char *szA);

BOOL utilIsSubFolder(LPSTR SubPath, LPSTR Path);
BOOL utilFileExists(LPSTR File);
BOOL utilDirExists(LPSTR Dir);
BOOL utilGetProcessDirectory(char *buf, int buflen);

void utilDoEvents ( HWND hWnd );

BOOL utilLogToFile (LPCTSTR text, ...);
BOOL utilChangeFileToLog(LPSTR);

BOOL utilCenterWindow (HWND hwndChild, HWND hwndParent);
LPTSTR   utilGetStringRes (int id);
int axtoi(char *hexStg) 
int utilaxtoi(char *hexStg) 
void utilFormatHexData(void * lpUnknow,DWORD dwUnknowSize, 
				   DWORD dwMaxReturnChar, char * szReturnBuffer, 
				   int offset, //int offset = -1,
				   unsigned int colunm_number, //unsigned int colunm_number=8
				   char *tab_str //char *tab_str=0
				   );



*/
#ifndef __HEADER_UTILITY_H_
#define __HEADER_UTILITY_H_

#ifndef true
	#define true 1
#endif

#ifndef false
	#define false 0
#endif

// assume backward compatibility (old names did not have "util" suffix)
#define GetMem(a,b)					utilGetMem(a,b)
#define FreeMem(a)					utilFreeMem(a)
#define ReAllocMem(a, b, c)			utilReAllocMem(a,b,c)
#define ReDim(a,b,c,d)				utilReDim(a,b,c,d)
#define GetCommandParameter(a,b)	utilGetCommandParameter(a,b)
#define ParseCommandLine(a,b,c)		utilParseCommandLine(a,b,c)
#define RemBkSlash(a,b,c)			utilRemBkSlash(a,b,c)
#define AddBkSlash(a,b,c)			utilAddBkSlash(a,b,c)
#define ExtractFileName(a,b,c)		utilExtractFileName(a,b,c)
#define ExtractFilePath(a,b,c)		utilExtractFilePath(a,b,c)
#define ChangeFileExt(a,b,c)		utilChangeFileExt(a,b,c)
#define uCopy(a,b,c,d,e)				utilCopy(a,b,c,d,e)
#define Right(a,b,c,d)				utilRight(a,b,c,d)
#define IsSubFolder(a,b)			utilIsSubFolder(a,b)
#define FileExists(a)				utilFileExists(a)
#define DirExists(a)				utilDirExists(a)
#define CreateAllSubDirs(a)			utilCreateAllSubDirs(a)
#define DoEvents(a)					utilDoEvents(a)
#define ChangeFileToLog(a)			utilChangeFileToLog(a)
#define LogToFile					utilLogToFile
#define AnsiToUnicode(a,b)			utilAnsiToUnicode(a,b)
#define UnicodeToAnsi(a,b)			utilUnicodeToAnsi(a,b)
#define CenterWindow(a,b)			utilCenterWindow(a,b)
#define GetStringRes(a)				utilGetStringRes(a)
#define axtoi(a)					utilaxtoi(a)
#define FormatHexData				utilFormatHexData
#define ShortenPath(a,b,c)			utilShortenPath(a,b,c)


/******************************************************************\
					Memory management section
\******************************************************************/

/*
 This function create a buffer in memoy.
 size : the size of the buffer to create
 zero : fill with zero or not?
 heap : Heap stack to use. Optional parameter. By default, it's
        GetProcessHeap()

 Return : if success, return a pointer to the new buffer, if fail, 
 it return an HeapAlloc errror
 */
LPVOID utilGetMem(int size, BOOL zero);

/*
 Free the memory allocated by GetMem
 x : the buffer to free
 heap : Heap stack to use. Optional parameter. By default, it's
        GetProcessHeap()

 Return : Nothing
 */
void utilFreeMem(LPVOID x);

/*
 Change the size for the buffer
 x : buffer to resize
 newsize : new size of the buffer
 zero : fill with zero extra data (if newsize if > previous size) ?
 heap : Heap stack to use. Optional parameter. By default, it's
        GetProcessHeap()

 Return: if success, return a pointer to new resized buffer. If fail
 return an HeapReAlloc error.
 */
PVOID utilReAllocMem(LPVOID x, int newsize, BOOL zero);



/* ReDim : ReDimention a dynamic table of pointer...
 pointer  : pointer to redimentionne
 NewCount : New count of entry there will have....
            If minor than previous, datas can be loosed
 size     : size of BASE value (if pointer is char **, size must be
            sizeof (char) = 1
 count    : count to size to create in BASE value (for exemple 50 to 
            create a 50 chars string iftypesize is char **)
Notes:  1- pointer must be equal to 0 if you want to use it the first time,
        to destroy him, but in this case size or count must be set 
		to '-1', and NewCount must have the real number of entry 
		there is in...
		2- To Redimention an BASE string, you can use MemRealloc.
		But to resize to table of pointers, yo should use ReDim

Sample of call:
void MyFunc()
{
	char **lpt;
	lpt = NULL;	// initialize pointer to 0 to be able to create it
	ReDim((void ***) &lpt,5,sizeof(char),50);
		// create 5 strings which have 50 chars in
	lstrcpy(lpt[0],"First string");
	lstrcpy(lpt[1],"Second string");
	lstrcpy(lpt[2],"Third string");
	lstrcpy(lpt[3],"Fourth string");
	lstrcpy(lpt[4],"Fifth string");
	ReDim((void ***) &lpt,6,sizeof(char),50);
		// Resize to 6 strings which have 50 chars in
	lstrcpy(lpt[5],"Sixth string");
	ReDim((void ***) &lpt, 6,-1,-1); // -1 = destroy, 6 is the number of entry in
}
*/
BOOL utilReDim(void ***pointer, UINT EntryCount, int sizeofvalue, int EntrySize);

// Note: Not fully operational

/******************************************************************\
				Command line functions section
\******************************************************************/


/*
 Put the next parameter of command line in parm
 cmd : command line to process
 parm : will contain new parameter

 Return : return command line to proceed. Use return value to
 call GetCommandParameter in cmd parameter for next 
 command line parameter.
 */
LPSTR utilGetCommandParameter(LPSTR cmd, LPSTR parm);

/* utilParseCommandLineInit
 * Initialise memory for argv table
 *  argv : pointer to two dimension table argv
 *     if argv == NULL, it will be allocated
 *     if != NULL, it's supposed to be already allocated in memory
 *  max_parameter_number : maximum number of parameter
 *  max_parameter_size : maximum size of each parameter
 */
void utilParseCommandLineInit(char** argv, int max_parameter_number, int max_parameter_size);

/* utilParseCommandLineFree
 * Free memory for argv table
 *  argv : pointer to two dimension table argv
 *  max_parameter_number : maximum number of parameter
 *  destoy_me : 0 : will not destroy argv (must be a char **)
 *              1 : will destroy argv (if it's a char * [???])
 */
void utilParseCommandLineFree(char** argv, int max_parameter_number, int destroy_me=false);

/* ParseCommandLine
 * "Cut" the command line into the argv table
 * cmdLine: full command line
 * argv a pointer to a table (use a char ** initialised with utilParseCommandLineInit)
 * argc will receive the number of argument
 */
void utilParseCommandLine(const char *cmdLine, char **argv,int	*argc);

/******************************************************************\
				Strings functions section
\******************************************************************/

/* utilGetProcessDirectory
 * Retreive the directory where the process was launched
 * buf : string buffer. Will receive the datas
 * buflen : max data lenght to copy
 */
BOOL utilGetProcessDirectory(char *buf, int buflen);

/*
 Remove Back Slash from path and put the result into buf.
 path : path to remove '\'
 buf : will receive the new string without '\'
 BufLen : length in byte of buf

 Return true if success, false if fail
 */
BOOL utilRemBkSlash(LPSTR path, LPSTR buf, int BufLen);

/*
 Add Back Slash from path and put the result into buf.
 path : path to add '\'
 buf : will receive the new string with '\'
 BufLen : length in byte of buf

 Return true if success, false if fail
 */
BOOL utilAddBkSlash(LPSTR path, LPSTR buf, int BufLen);


/*
 Extract filename from path and put result into buf.
 path : file path to extract the name
 buf : will receive the filename
 BufLen : length in byte of buf

 Return 0 if fail, or the length of buf if success
 */

int utilExtractFileName(LPCSTR path, LPSTR buf, int BufLen);


/*
 Extract filepath from path and put result into buf.
 path : file path to extract the path
 buf : will receive the path
 BufLen : length in byte of buf

 Return 0 if fail, or the length of buf if success
 */
int utilExtractFilePath(LPCSTR path, LPSTR buf, int BufLen);

/*
 Return true if SubPath is a sub-path of path, false if not
 */
BOOL utilIsSubFolder(LPSTR SubPath, LPSTR Path);

/*
 Change file extenstion from FilePath.
 FilePath : file path to change the extension
 NewExt : new extension ( "EXE");
 BufLen : length in byte of FilePath
 if NewExt = "", remove extension
 Return 0 if fail, or the length of buf if success
 */
int utilChangeFileExt(LPSTR FilePath, LPSTR NewExt, int BufLen);


/* Copy Len chars from BegPos position in Str to buf strings.
	Str: source string
	BegPos : begin position to copy in Str
	Len : number of char to copy
	buf : destination string buffer
	BufLen : buffer lenght.
 */
int utilCopy (LPCSTR Str,int BegPos,int Len, LPSTR buf, int BufLen);

/*
 Copy Len characters from the right of the Str string to buf string
	Str: source string
	Len: number of character to copy from the right
	buf: string buffer who will recieve the copied string
	BufLen : lenght of buf (in characters).
 */
int utilRight(LPCSTR Str, int Len, LPSTR buf, int BufLen);


/*
 AnsiToUnicode : convert an ANSI string to a Widechar (UNICODE) string
 szA : string to convert
 szW : pointer to a buffer which will receive the UNICODE version of
       szA
 Return true if success, false if failure.
 Be sure the buffer is large enough!
 */
BOOL utilAnsiToUnicode(char *szA, WCHAR *szW);

/*
 UnicodeToAnsi : convert an ANSI string to a Widechar (UNICODE) string
 szW : string to convert
 szA : pointer to a buffer which will receive the ANSI version of
       szA
 Return true if success, false if failure.
 Be sure the buffer is large enough!
 */
BOOL utilUnicodeToAnsi(WCHAR *szW, char *szA);

/******************************************************************\
				Files management functions section
\******************************************************************/
/*
 Return true if SubPath is a sub-path of path, false if not
 */
BOOL utilIsSubFolder(LPSTR SubPath, LPSTR Path);
/*
 Return true if file exists or false if not
 */
BOOL utilFileExists(LPCSTR File);

BOOL utilDirExists(LPSTR Dir);


/******************************************************************\
				Miscs functions section
\******************************************************************/

/*
 Refresh the messages list of a window
 */
void utilDoEvents ( HWND hWnd );



/******************************************************************\
				LogToFile functions section
\******************************************************************/
/* 
 Change file path to log to. If newLogFile is empty, LogToFile will
 be disabled.
*/
BOOL utilLogToFile (LPCTSTR text, ...);

/*
 Log a line to a file
 Same syntaxe as printf
 */
BOOL utilChangeFileToLog(LPSTR); 

/******************************************************************\
				      Other functions
\******************************************************************/

//   FUNCTION: CenterWindow(HWND, HWND)
//
//   PURPOSE: Centers one window over another.
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
//       This function will center one window over another ensuring that
//    the placement of the window is within the 'working area', meaning
//    that it is both within the display limits of the screen, and not
//    obscured by the tray or other framing elements of the desktop.
BOOL utilCenterWindow (HWND hwndChild, HWND hwndParent);

//
// FUNCTION:    GetStringRes (int id INPUT ONLY)
//
// COMMENTS:    Load the resource string with the ID given, and return a
//              pointer to it.  Notice that the buffer is common memory so
//              the string must be used before this call is made a second time.
//
LPTSTR   utilGetStringRes (int id);

/* utilaxtoi
 * Convert hexadecimal string to integer
 * hexStg must me a string containing the following chars:
 * '1' to '9', 'a' to 'f', or 'A' to 'B'.
 * The conversion stop when a wrong char occurs ('g', 'w',...)
 */
int utilaxtoi(char *hexStg);


/* FormatHexData()
 * Fill a string buffer with the content of an unknown buffer:
 * Ex : 0x00400000   4D 5A 90 00 03 00 00 00    MZ?....
 * Argument:
 * lpUnkwown : pointer to the buffer to "display"
 * dwUnknownSize : size of the lpUnknown memory
 * dwMaxReturnChar : maximum number of character to write to string buffer
 * szReturnBuffer : must point to an allocated memory space to be fill
 * offset (optional) : offset to start count (-1 => no offset colunm) (default value : -1)
 * colunm_number (optional) : number of column to display in the main hexadecimal colunm  (default value : 8)
 * tab_str (optional) : pointer to a string to write on the left of each line (0=nothing) (default value : 0)
 * Exemple
 * Call : 	FormatHexData(hInstance,75, 1024, buffer, (int) hInstance,8,"\t");
 	0x00400000   4D 5A 90 00 03 00 00 00    MZ?....
	0x00400008   04 00 00 00 FF FF 00 00    ....ÿÿ..
	0x00400010   B8 00 00 00 00 00 00 00    ?......
	0x00400018   40 00 00 00 00 00 00 00    @.......
	0x00400020   00 00 00 00 00 00 00 00    ........
	0x00400028   00 00 00 00 00 00 00 00    ........
	0x00400030   00 00 00 00 00 00 00 00    ........
	0x00400038   00 00 00 00 F8 00 00 00    ....?..
	0x00400040   0E 1F BA 0E 00 B4 09 CD    ..?.??
	0x00400048   21 B8 01                   !?
 */
void utilFormatHexData(void * lpUnknow,DWORD dwUnknowSize, 
				   DWORD dwMaxReturnChar, char * szReturnBuffer, int offset = -1,
				   unsigned int colunm_number=8, char *tab_str=0);

/* utilShortenPath
 * Shorten the given path to the size of maxpathsize
 * c:\program files\Internet Explorer => c:\prog...\Inte...\
 * path : input string
 * rebuf : buffer to write to
 * maxpathsize : maximum size of rebuf string
 */

BOOL utilShortenPath(char *path, char *retbuf, int maxpathsize);

BOOL utilCreateAllSubDirs(LPSTR spath);

#endif // __HEADER_UTILITY_H_

