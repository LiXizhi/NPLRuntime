#pragma once

struct lua_State;

extern "C"
{
	/** 
	The library provides the constant bit.bits that gives the number of
	bits that can be used in bitwise operations, and the following
	functions:

	bit.cast(a)        cast a to the internally-used integer type
	bit.bnot(a)        returns the one's complement of a
	bit.band(w1, ...)  returns the bitwise and of the w's
	bit.bor(w1, ...)   returns the bitwise or of the w's
	bit.bxor(w1, ...)  returns the bitwise exclusive or of the w's
	bit.lshift(a, b)   returns a shifted logically left b places
	bit.rshift(a, b)   returns a shifted logically right b places
	bit.alshift(a, b)  returns a shifted arithmetically left b places
	bit.arshift(a, b)  returns a shifted arithmetically right b places
	*/
	int luaopen_bit_local(lua_State *L);
}