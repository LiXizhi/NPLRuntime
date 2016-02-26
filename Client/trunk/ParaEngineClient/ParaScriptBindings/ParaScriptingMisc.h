//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2007 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2005.10
// Description:	API for misc functions and ParaEngine namespace.
//-----------------------------------------------------------------------------
#pragma once
#include "ParaScriptingGlobal.h"
#include "ParaScriptingScene.h"
#include <string>
namespace ParaScripting
{
	class ParaAssetObject;
	using namespace std;
	using namespace luabind;

	/**
	* @ingroup ParaMisc
	* Contains miscellaneous functions
	*/
	class PE_CORE_DECL ParaMisc 
	{
	public:
		/** get the number of characters in str. Str is assumed to be in ANSI code page. 
		* it is converted to Unicode and return the character count. */
		static int GetUnicodeCharNum(const char* str);

		/** 
		* same as LUA string.sub(), except that the index is character.
		* get a sub string of a ANSI Code page string. However, the index are unicode characters. 
		* @param str: the string to use
		* @param nFrom: character index beginning from 1. 
		*/
		static string UniSubString(const char* str, int nFrom, int nTo);

		/** encode a string using really simple algorithm. it just makes the source ineligible. It is still not immune to crackers.
		* str = SimpleDecode(SimpleEncode(str)) 
		* @return: it may return NULL if input invalid
		*/
		static const char* SimpleEncode(const char* source);

		/** decode a string using really simple algorithm.  
		* str = SimpleDecode(SimpleEncode(str)) 
		* @return: it may return NULL if input invalid
		*/
		static const char* SimpleDecode(const char* source);

		/** convert the md5 of the input source string. */
		static string md5(const char* source);
		static string md5_(const char* source, bool bBinary);

		/** Generating [-MAX, MAX] long integer
		* @param seedTable: nil or a table containing {_seed=integer} 
		* when the function returned the seedTable._seed will be modified. 
		*/
		static long RandomLong(const object& seedTable);
		/** generating [0,1] double value */
		static double RandomDouble(const object& seedTable);
		
		/**
		* Converts an entire byte array from one encoding to another.
		* @param srcEncoding: any encoding name. If nil or "", it is the default coding in NPL. 
		* see Encoding.GetEncoding(). Below are some commonly used field
		  | *Code Page* | *Name* |
		  | 950   |  big5   |
		  | 936   |  gb2312 |
		  | 65001 |  utf-8  |
		  | 65005 |  utf-32  |
		* There is one special code name called "HTML", which contains HTML special characters in ascii code page. This is usually true for most "iso8859-15" encoding in western worlds. 
		* It just writes the unicode number\U+XXXX in ascii character "&#XXXX;" where & is optional. 
		* @param dstEncoding: save as above. If nil or "", it will be converted to default coding in NPL. 
		* @bytes: the source bytes.
		* e.g. The most common use of this function is to create HTML special character to NPL string, like below
		* local text = ParaMisc.EncodingConvert("HTML", "", "Chinese characters: &#24320;&#21457;")
		* log(text);
		*/
		static const char* EncodingConvert(const object& srcEncoding, const object& dstEncoding, const object& bytes);

		/** copy text to clipboard. Input is ANSI code page */
		static bool CopyTextToClipboard(const char* text);

		/** get text from clipboard. text is converted to ANSI code page when returned.*/
		static const char* GetTextFromClipboard();
	};

	/** 
	* @ingroup ParaEngine
	* global game engine related functions, such as ParaEngineCore interface, copy right information, simple water marking 
	*/
	class PE_CORE_DECL CParaEngine
	{
	public:
		/** get the attribute object of ParaEngine settings */
		static ParaAttributeObject GetAttributeObject();
		/** for API exportation.*/
		static void GetAttributeObject_(ParaAttributeObject& output);

		/** get ParaEngine version */
		static string GetVersion();

		// this prevent functions to be generated in the documentation.
		/// @cond
		static string GetCopyright();
		static string GetAuthorTo();
		static string GetWatermark();
		/// @endcond

		/** get attribute object */
		static ParaAttributeObject GetViewportAttributeObject(int nViewportIndex);

		/** render the current frame and does not return until everything is presented to screen. 
		* this function is usually used to draw the animated loading screen. */
		static bool ForceRender();
		/** cause the main thread to sleep for the specified seconds.*/
		static bool Sleep(float fSeconds);
		/**
		* save an existing mesh to file.
		* @param filename: file to be saved to. if this is "", the xmesh entity's file name will be used and appended with ".x"
		* @param xmesh: ParaX mesh object to export.
		* @param bBinaryEncoding: true to use binary encoding. 
		* @return: return true if succeeds.
		*/
		static bool SaveParaXMesh(const char* filename, ParaAssetObject& xmesh, bool bBinaryEncoding);
		

		/////////////////////////////////////////////////
		//
		// some d3d functions.
		//
		/////////////////////////////////////////////////

		/** return the current render target. calling this function a second time will make the returned object from the previous call invalid.*/
		static ParaAssetObject GetRenderTarget();
		
		/** set the render target of the object.*/
		static bool SetRenderTarget(ParaAssetObject& pRenderTarget);
		static bool SetRenderTarget2(int nIndex, const char* render_targetname);
		
		/** Copy render target content from one surface to another. they may be of different resolution */
		static bool StretchRect(ParaAssetObject& pSrcRenderTarget, ParaAssetObject& pDestRenderTarget);
		
		/** draw a full screen quad. 
		* @note: one need to set the vertex declaration to be S0_POS_TEX0 prior to calling this function */
		static bool DrawQuad();

		/** Set declaration by id
		enum VERTEX_DECLARATION
		{
			S0_POS_TEX0, // all data in stream 0: position and tex0
			S0_POS_NORM_TEX0, // all data in stream 0: position, normal and tex0
			S0_POS_NORM_TEX0_INSTANCED, // all data in stream 0: position, normal and tex0, stream1:instanced data
			S0_POS_TEX0_COLOR, // all data in stream 0: position, tex0 and color
			S0_S1_S2_OCEAN_FFT, // for FFT ocean
			S0_S1_S2_S3_OCEAN_FFT, // for FFT ocean with terrain height field
			S0_POS_NORM_TEX0_TEX1, // all data in stream 0: position, normal tex0 and tex1
			MAX_DECLARATIONS_NUM,
		};
		@param nIndex: value is in @see VERTEX_DECLARATION 
		@return: return true if successful.
		*/
		static bool SetVertexDeclaration(int nIndex);
	};
}