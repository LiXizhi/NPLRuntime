#pragma once
#include <string>
#include <map>
#include <float.h>
using namespace std;

namespace ParaEngine
{
	/**
	* string manipulation helpers for ParaEngine
	*/
	class StringHelper
	{
	public:
		/**
		*  @brief Converts utf8 string to utf16 string
		*  @param utf8 The utf8 string to be converted
		*  @param outUtf16 The output utf16 string
		*  @return true if succeed, otherwise false
		*  @note Please check the return value before using \p outUtf16
		*  e.g.
		*  @code
		*    std::u16string utf16;
		*    bool ret = StringHelper::UTF8ToUTF16("ÄãºÃhello", utf16);
		*    if (ret) {
		*        do_some_thing_with_utf16(utf16);
		*    }
		*  @endcode
		*/
		static bool UTF8ToUTF16(const std::string& utf8, std::u16string& outUtf16);
		/** similar to UTF8ToUTF16. except that if UTF8ToUTF16 returns false, it will replace invalid utf8 character with ?. and then return the converted string. */
		static bool UTF8ToUTF16_Safe(const std::string& utf8, std::u16string& outUtf16);

		/*
		* @str:    the string to trim
		* @index:    the index to start trimming from.
		*
		* Trims str st str=[0, index) after the operation.
		*
		* Return value: the trimmed string.
		* */
		static void TrimUTF16VectorFromIndex(std::vector<char16_t>& str, int index);

		/*
		* @ch is the unicode character whitespace?
		*
		* Reference: http://en.wikipedia.org/wiki/Whitespace_character#Unicode
		*
		* Return value: weather the character is a whitespace character.
		* */
		static bool IsUnicodeSpace(char16_t ch);

		static bool IsCJKUnicode(char16_t ch);

		static void TrimUTF16Vector(std::vector<char16_t>& str);


		/*
		* @str:    the string to search through.
		* @c:        the character to not look for.
		*
		* Return value: the index of the last character that is not c.
		* */
		static unsigned int GetIndexOfLastNotChar16(const std::vector<char16_t>& str, char16_t c);

		/**
		*  @brief Converts utf16 string to utf8 string
		*  @param utf16 The utf16 string to be converted
		*  @param outUtf8 The output utf8 string
		*  @return true if succeed, otherwise false
		*  @note Please check the return value before using \p outUtf8
		*  e.g.
		*  @code
		*    std::string utf8;
		*    bool ret = StringHelper::UTF16ToUTF8(u"\u4f60\u597d", utf8);
		*    if (ret) {
		*        do_some_thing_with_utf8(utf8);
		*    }
		*  @endcode
		*/
		static bool UTF16ToUTF8(const std::u16string& utf16, std::string& outUtf8);

		static const WCHAR* MultiByteToWideChar(const char* name, unsigned int nCodePage = 0, size_t* outLen = nullptr);
		static const char* WideCharToMultiByte(const WCHAR* name, unsigned int nCodePage = 0, size_t* outLen = nullptr);
		/**
		* get the text of the control
		* @param szText [out] buffer to receive the text
		* @param nLength size of the input buffer
		* @return return the number of bytes written to the buffer. If nLength <= 0, this function returns the total number of characters of the text
		*/
		static int WideCharToMultiByte(const WCHAR* name, char* szText, int nLength, unsigned int nCodePage = 0);

		static const WCHAR* AnsiToWideChar(const char* name);
		static const char* WideCharToAnsi(const WCHAR* name);

		static const char* UTF8ToAnsi(const char* name);
		static const char* AnsiToUTF8(const char* name);

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
		static string SimpleEncode(const string& source);

		/** decode a string using really simple algorithm.
		* str = SimpleDecode(SimpleEncode(str))
		* @return: it may return NULL if input invalid
		*/
		static string SimpleDecode(const string& source);

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
		static const std::string& EncodingConvert(const std::string& srcEncoding, const std::string& dstEncoding, const std::string& bytes);

		/** copy text to clipboard. Input is GUI UTF8 encoding */
		static bool CopyTextToClipboard(const string& text);

		/** get text from clipboard. text is converted to ANSI code page when returned.*/
		static const char* GetTextFromClipboard();

		/** divide string */
		static void DevideString(const string& input, string& str1, string& str2, char separator = ';');

		/** splite string by token */
		static void split(const std::string& src, const std::string& token, std::vector<std::string>& vect);

		/// Makes the string lower case.
		static void make_lower(string& str);

		/// Makes the string upper case.
		static void make_upper(string& str);

		/**
		* Checks whether the given input is valid UTF-8.
		* @param data The data to check for validity.
		* @return @@b True if the input is valid UTF-8, @b false otherwise.
		*/
		static bool checkValidXMLChars(const std::string& data);

		/**
		* remove invalid characters in the given input.
		* @param data The data to check for validity.
		* @return @@b True if the input is valid UTF-8, @b false otherwise. If false, invalid chars in input will be removed.
		*/
		static bool removeInValidXMLChars(std::string& data);

		static bool IsNumber(const char* str);
		static bool IsLetter(const char* str);
		static int StrToInt(const char* str);
		static double StrToFloat(const char* str);
		static bool RegularMatch(const char* input, const char* expression);
		//replace all "\" in a string to "\\"
		static std::string ToCString(const char* input);
		static std::string StrReplace(const char* inputstring, char srcchar, char destchar);
		static RECT* GetImageAndRect(const std::string& str, std::string& imagefile, RECT* pOut = NULL);

		/** convert the md5 of the input source string.
		* @param bBinary: if false (default), result is 32 hex number chars. if true, result is 16 bytes binary string.
		*/
		static std::string md5(const std::string& source, bool bBinary = false);

		/*
			convert the sha1 of the input source string.
			* @param bBinary: if false (default), result is 32 hex number chars. if true, result is 16 bytes binary string.
		*/
		static std::string sha1(const std::string& source, bool bBinary = false);

		/** convert the base64 of the input source string. */
		static std::string base64(const std::string& source);
		/** return unbase64 of the input source string. */
		static std::string unbase64(const std::string& source);



		/** a fast printf function that support limited functions.
		The formats supported by this implementation are: 'd' 'u' 'c' 's' 'x' 'X' 'f'.
		Zero padding and field width are also supported.
		One very important feature is that %f behaves like %d if float is integer
		*/
		static void fast_sprintf(char* s, const char* fmt, ...);
		static void fast_snprintf(char* s, int nMaxCount, const char* fmt, ...);

		/** fast itoa modified from http://code.google.com/p/maxmods/
		@param value: the value to be converted.
		@param result: the array to hold the result.
		@param buf_size: the result buffer size.
		@param base: can only be [2,16]
		@return the number of characters(not including the trailing '\0') in result.
		*/
		static int fast_itoa(int64 value, char* result, int buf_size, int base = 10);

		/** doing a fast (may not be accurate) double to string conversion.
		* it is similar to "%.xxf", except that it will remove all trailing zero, and ensures that nBuffSize is not exceed for out of range numbers, etc.
		@param num: Value to be converted to a string. NaN, +Inf,-Inf will generate "0"
		@param str: output string, array in memory where to store the resulting null-terminated string.
		@param nBuffSize: the output buff size.
		@param max_decimal: max number of numbers after the decimal point. It will remove trailing '0'
		@param radix: the radix.
		@return the number of characters(not including the trailing '\0') in result.
		*/
		static int fast_dtoa(double num, char* str, int nBuffSize = 40, int max_decimal = 5, int radix = 10);

		/** check if the input string matches the wild card pattern
		* @param sWildcardPattern: such as "*.*" or "*" or "main_*.pkg", etc.
		*/
		static bool MatchWildcard(const std::string& str, const std::string& sWildcardPattern);

		/** check whether str ends with sequence
		@param sequence: it may contain ?, which matches to 1 any character.
		*/
		static bool StrEndsWith(const string& str, const string& sequence);

		/** check whether str ends with sequence without spaces in front of it.
		@param sequence: it may contain ?, which matches to 1 any character.
		*/
		static bool StrEndsWithWord(const string& str, const string& sequence);

		static std::string GetDefaultCPName() { return defaultCPName.get(); }

		// replace " with \" in text
		std::string EncodeStringInQuotation(const std::string& text);

		/** // and \\ are replaced with / and \ */
		void RemoveDoubleSlashesInString(std::string& sFilePath);

		/** fast check isdigit without considering locale */
		static inline bool isdigit(char c) {
			return ((c >= '0') && (c <= '9'));
		}

		/** fast check isalnum without considering locale */
		static inline bool isalnum(char c)
		{
			return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') );
		}

	private:
		class _CodePageName
		{
		public:
			_CodePageName()
			{
#ifdef DEFAULT_FILE_ENCODING
				name = "utf-8";
#else
#ifdef WIN32
				auto cp = GetACP();
				char tmp[30];
				ParaEngine::StringHelper::fast_itoa((int)cp, tmp, 30);

				name = "CP";
				name += tmp;
#else
				name = "utf-8";
#endif
#endif
			}

			const std::string& get() const
			{
				return name;
			}

		private:
			std::string name;

		};

		static _CodePageName defaultCPName;

	};
}