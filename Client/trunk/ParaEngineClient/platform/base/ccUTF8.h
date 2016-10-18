
#ifndef __cocos2dx__ccUTF8__
#define __cocos2dx__ccUTF8__


#include "../mac/CCType.h"

#include <vector>
#include <string>
#include <sstream>


#define CC_FORMAT_PRINTF(a,b)

NS_CC_BEGIN

namespace StringUtils {

template<typename T>
std::string toString(T arg)
{
    std::stringstream ss;
    ss << arg;
    return ss.str();
}

std::string format(const char* format, ...) CC_FORMAT_PRINTF(1, 2);

/**
 *  @brief Converts from UTF8 string to UTF16 string.
 *
 *  This function resizes \p outUtf16 to required size and
 *  fill its contents with result UTF16 string if conversion success.
 *  If conversion fails it guarantees not to change \p outUtf16.
 *
 *  @param inUtf8 The source UTF8 string to be converted from.
 *  @param outUtf16 The output string to hold the result UTF16s.
 *  @return True if succeed, otherwise false.
 *  @note Please check the return value before using \p outUtf16
 *  e.g.
 *  @code
 *    std::u16string utf16;
 *    bool ret = StringUtils::UTF8ToUTF16("你好hello", utf16);
 *    if (ret) {
 *        do_some_thing_with_utf16(utf16);
 *    }
 *  @endcode
 */
bool UTF8ToUTF16(const std::string& inUtf8, std::u16string& outUtf16);

/**
 *  @brief Same as \a UTF8ToUTF16 but converts form UTF8 to UTF32.
 *
 *  @see UTF8ToUTF16
 */
bool UTF8ToUTF32(const std::string& inUtf8, std::u32string& outUtf32);

/**
 *  @brief Same as \a UTF8ToUTF16 but converts form UTF16 to UTF8.
 *
 *  @see UTF8ToUTF16
 */
bool UTF16ToUTF8(const std::u16string& inUtf16, std::string& outUtf8);
    
/**
 *  @brief Same as \a UTF8ToUTF16 but converts form UTF16 to UTF32.
 *
 *  @see UTF8ToUTF16
 */
bool UTF16ToUTF32(const std::u16string& inUtf16, std::u32string& outUtf32);

/**
 *  @brief Same as \a UTF8ToUTF16 but converts form UTF32 to UTF8.
 *
 *  @see UTF8ToUTF16
 */
bool UTF32ToUTF8(const std::u32string& inUtf32, std::string& outUtf8);
    
/**
 *  @brief Same as \a UTF8ToUTF16 but converts form UTF32 to UTF16.
 *
 *  @see UTF8ToUTF16
 */
bool UTF32ToUTF16(const std::u32string& inUtf32, std::u16string& outUtf16);



/**
 *  @brief Trims the unicode spaces at the end of char16_t vector.
 */
void trimUTF16Vector(std::vector<char16_t>& str);

/**
 *  @brief Whether the character is a whitespace character.
 *  @param ch    The unicode character.
 *  @returns     Whether the character is a white space character.
 *
 *  @see http://en.wikipedia.org/wiki/Whitespace_character#Unicode
 *
 */
bool isUnicodeSpace(char16_t ch);

/**
 *  @brief Whether the character is a Chinese/Japanese/Korean character.
 *  @param ch    The unicode character.
 *  @returns     Whether the character is a Chinese character.
 *
 *  @see http://www.searchtb.com/2012/04/chinese_encode.html
 *  @see http://tieba.baidu.com/p/748765987
 *
 */
bool isCJKUnicode(char16_t ch);

/**
 *  @brief Returns the length of the string in characters.
 *  @param utf8 An UTF-8 encoded string.
 *  @returns The length of the string in characters.
 */
long getCharacterCountInUTF8String(const std::string& utf8);

/**
 *  @brief Gets the index of the last character that is not equal to the character given.
 *  @param str   The string to be searched.
 *  @param c     The character to be searched for.
 *  @returns The index of the last character that is not \p c.
 */
unsigned int getIndexOfLastNotChar16(const std::vector<char16_t>& str, char16_t c);

/**
 *  @brief Gets char16_t vector from a given utf16 string.
 */
std::vector<char16_t> getChar16VectorFromUTF16String(const std::u16string& utf16);



/**
* Utf8 sequence
* Store all utf8 chars as std::string
* Build from std::string
*/
class StringUTF8
{
public:
    struct CharUTF8
    {
        std::string _char;
        bool isAnsi() { return _char.size() == 1; }
    };
    typedef std::vector<CharUTF8> CharUTF8Store;

    StringUTF8();
    StringUTF8(const std::string& newStr);
    ~StringUTF8();

    std::size_t length() const;
    void replace(const std::string& newStr);

    std::string getAsCharSequence() const;

    bool deleteChar(std::size_t pos);
    bool insert(std::size_t pos, const std::string& insertStr);
    bool insert(std::size_t pos, const StringUTF8& insertStr);

    CharUTF8Store& getString() { return _str; }

private:
    CharUTF8Store _str;
};

} // namespace StringUtils {

/**
 * Returns the character count in UTF16 string.
 * @param str Pointer to the start of a UTF-16 encoded string. It must be an NULL terminal UTF8 string.
 * @deprecated Please use c++11 `std::u16string::length` instead, don't use `unsigned short*` directly.
 */
int cc_wcslen(const unsigned short* str);

/** Trims the space characters at the end of UTF8 string.
 *  @deprecated Please use `StringUtils::trimUTF16Vector` instead.
 */

void cc_utf8_trim_ws(std::vector<unsigned short>* str);

/**
 * Whether the character is a whitespace character.
 *
 * @param ch    The unicode character.
 * @returns     Whether the character is a white space character.
 * @deprecated Please use `StringUtils::isUnicodeSpace` instead.
 *
 * @see http://en.wikipedia.org/wiki/Whitespace_character#Unicode
 */
bool isspace_unicode(unsigned short ch);

/**
 * Whether the character is a Chinese/Japanese/Korean character.
 *
 * @param ch    The unicode character
 * @returns     Whether the character is a Chinese character.
 * @deprecated Please use `StringUtils::isCJKUnicode` instead.
 *
 * @see http://www.searchtb.com/2012/04/chinese_encode.html
 * @see http://tieba.baidu.com/p/748765987
 */
bool iscjk_unicode(unsigned short ch);

/**
 * Returns the length of the string in characters.
 *
 * @param p Pointer to the start of a UTF-8 encoded string. It must be an NULL terminal UTF8 string.
 * @param max Not used from 3.1, just keep it for backward compatibility.
 * @deprecated Please use `StringUtils::getCharacterCountInUTF8String` instead.
 * @returns The length of the string in characters.
 **/
long cc_utf8_strlen (const char * p, int max = -1);

/**
 * Find the last character that is not equal to the character given.
 *
 * @param str   The string to be searched.
 * @param c     The character to be searched for.
 * @deprecated Please use `StringUtils::getIndexOfLastNotChar16` instead.
 * @returns The index of the last character that is not \p c.
 */
unsigned int cc_utf8_find_last_not_char(const std::vector<unsigned short>& str, unsigned short c);

/**
 *  @brief Gets `unsigned short` vector from a given utf16 string.
 *  @param str A given utf16 string.
 *  @deprecated Please use `StringUtils::getChar16VectorFromUTF16String` instead.
 */
std::vector<unsigned short> cc_utf16_vec_from_utf16_str(const unsigned short* str);

/**
 * Creates an utf8 string from a c string. The result will be null terminated.
 *
 * @param str_old Pointer to the start of a C string. It must be an NULL terminal UTF8 string.
 * @param length  Not used from 3.1, keep it just for backward compatibility.
 * @param rUtf16Size The character count in the return UTF16 string.
 * @deprecated Please use `StringUtils::UTF8ToUTF16` instead.
 * @returns The newly created utf16 string, it must be released with `delete[]`,
 *          If an error occurs, %NULL will be returned.
 */
unsigned short* cc_utf8_to_utf16(const char* str_old, int length = -1, int* rUtf16Size = nullptr);

/**
 * Converts a string from UTF-16 to UTF-8. The result will be null terminated.
 *
 * @param utf16 An UTF-16 encoded string, It must be an NULL terminal UTF16 string.
 * @param len Not used from 3.1, keep it just for backward compatibility.
 * @param items_read     Not used from 3.1, keep it just for backward compatibility.
 * @param items_written  Not used from 3.1, keep it just for backward compatibility.
 * @deprecated Please use `StringUtils::UTF16ToUTF8` instead.
 * @returns A pointer to a newly allocated UTF-8 string. This value must be
 *          released with `delete[]`. If an error occurs, %NULL will be returned.
 */
char * cc_utf16_to_utf8 (const unsigned short  *str,
                  int             len = -1,
                  long            *items_read = nullptr,
                  long            *items_written = nullptr);


NS_CC_END

#endif /** defined(__cocos2dx__ccUTF8__) */
