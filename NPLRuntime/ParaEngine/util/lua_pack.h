#pragma once

struct lua_State;

extern "C"
{
	/**
	from: https://github.com/Mashape/lua-pack

	string.pack is called as follows: pack(F,x1,x2,...), 
	where F is a string describing how the values x1, x2, ... are to be interpreted and formatted. 
	Each letter in the format string F consumes one of the given values. Only values of type number 
	or string are accepted. pack returns a (binary) string containing the values packed as described in F. 
	The letter codes understood by pack are listed in lpack.c (they are inspired by Perl's codes 
	but are not the same). Numbers following letter codes in F indicate repetitions.

	string.unpack is called as follows: unpack(s,F,[init]), 
	where s is a (binary) string containing data packed as if by pack, 
	F is a format string describing what is to be read from s, and the optional init marks 
	where in s to begin reading the values. unpack returns one value per letter in F 
	until F or s is exhausted (the letters codes are the same as for pack, except that numbers 
	following `A' are interpreted as the number of characters to read into the string, not as repetitions).

	The first value returned by unpack is the next unread position in s, which can be used as 
	the init position in a subsequent call to unpack. This allows you to unpack values in a 
	loop or in several steps. If the position returned by unpack is beyond the end of s, 
	then s has been exhausted; any calls to unpack starting beyond the end of s will always return nil values.

	*/
	int luaopen_lua_pack(lua_State *L);
}