#pragma once
/** 
* Author: LiXizhi
* Date: 2015.1.27
* by the time of writing: android NDK(still gcc 4.8) does not fully support std::regex, all other platform seems fine. 
* so boost::regex is used instead. When gcc is upgraded in android NDK, I need to switch to std::regex.  
* in either case, I removed PCRE support. 
*/
#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_ANDROID) || (PARA_TARGET_PLATFORM == PARA_PLATFORM_LINUX)
#include <boost/regex.hpp>
#define USE_BOOST_REGEX
using namespace boost;
#else
#include <regex>
#endif

/**
* Title: ParaEngine uses regular expression from pcre.org. A c++ wrapper of it called "pcrecpp" is used, which is contributed by Google Inc.
* Author: LiXizhi
* Date: 2008.1.7
*/

/** 
#include <pcrecpp.h> // test pcre lib

more doc can be found in /pcre/contributes/xxx.chm, below are some examples. 

The "FullMatch" operation checks that supplied text matches a supplied pattern exactly. If pointer arguments are supplied, it copies matched sub-strings that match sub-patterns into them. 
------------------------------

Example: successful match
pcrecpp::RE re("h.*o");
re.FullMatch("hello");

Example: unsuccessful match (requires full match):
pcrecpp::RE re("e");
!re.FullMatch("hello");

Example: creating a temporary RE object:
pcrecpp::RE("h.*o").FullMatch("hello");

PARTIAL MATCHES
------------------------------

You can use the "PartialMatch" operation when you want the pattern to match any substring of the text. 

Example: simple search for a string:
pcrecpp::RE("ell").PartialMatch("hello");

Example: find first number in a string:
int number;
pcrecpp::RE re("(\\d+)");
re.PartialMatch("x*100 + 20", &number);
assert(number == 100);

RE(pattern,
RE_Options().set_caseless(true).set_multiline(true))
.PartialMatch(str);

RE(" ^ xyz \\s+ .* blah$",
RE_Options()
.set_caseless(true)
.set_extended(true)
.set_multiline(true)).PartialMatch(sometext);

SCANNING TEXT INCREMENTALLY
------------------------------
The "Consume" operation may be useful if you want to repeatedly match regular expressions at the front of a string and skip over them as they match. This requires use of the "StringPiece" type, which represents a sub-range of a real string. Like RE, StringPiece is defined in the pcrecpp namespace. 

Example: read lines of the form "var = value" from a string.
string contents = ...;                 // Fill string somehow
pcrecpp::StringPiece input(contents);  // Wrap in a StringPiece



string var;
int value;
pcrecpp::RE re("(\\w+) = (\\d+)\n");
while (re.Consume(&input, &var, &value)) {
...;
}

REPLACING PARTS OF STRINGS
-------------------------------

You can replace the first match of "pattern" in "str" with "rewrite". Within "rewrite", backslash-escaped digits (\1 to \9) can be used to insert text matching corresponding parenthesized group from the pattern. \0 in "rewrite" refers to the entire matching text. For example: 

string s = "yabba dabba doo";
pcrecpp::RE("b+").Replace("d", &s);

will leave "s" containing "yada dabba doo".  The result is true if  the pattern matches and a replacement occurs, or false otherwise.
GlobalReplace() is like Replace(), except that it replaces all occurrences of the pattern in the string with the rewrite. Replacements are not subject to re-matching.  E.g.,

string s = "yabba dabba doo";
pcrecpp::RE("b+").GlobalReplace("d", &s);

*/