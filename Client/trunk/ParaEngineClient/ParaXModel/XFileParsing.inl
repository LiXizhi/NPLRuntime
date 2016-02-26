//-----------------------------------------------------------------------------
// Class:	XFile parsing related functions
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.6
// Desc: ported from WINE 1.7.2 directX source code(parsing.c dxdxof.c  dxfile.h xfile.c)
//-----------------------------------------------------------------------------
#pragma once

using namespace ParaEngine;

enum XFileToken
{
	TOKEN_NONE = 0,
	TOKEN_NAME = 1,
	TOKEN_STRING = 2,
	TOKEN_INTEGER = 3,
	TOKEN_GUID = 5,
	TOKEN_INTEGER_LIST = 6,
	TOKEN_FLOAT_LIST = 7,
	TOKEN_OBRACE = 10,
	TOKEN_CBRACE = 11,
	TOKEN_OPAREN = 12,
	TOKEN_CPAREN = 13,
	TOKEN_OBRACKET = 14,
	TOKEN_CBRACKET = 15,
	TOKEN_OANGLE = 16,
	TOKEN_CANGLE = 17,
	TOKEN_DOT = 18,
	TOKEN_COMMA = 19,
	TOKEN_SEMICOLON = 20,
	TOKEN_TEMPLATE = 31,
	TOKEN_WORD = 40,
	TOKEN_DWORD = 41,
	TOKEN_FLOAT = 42,
	TOKEN_DOUBLE = 43,
	TOKEN_CHAR = 44,
	TOKEN_UCHAR = 45,
	TOKEN_SWORD = 46,
	TOKEN_SDWORD = 47,
	TOKEN_VOID = 48,
	TOKEN_LPSTR = 49,
	TOKEN_UNICODE = 50,
	TOKEN_CSTRING = 51,
	TOKEN_ARRAY = 52,
	TOKEN_ERROR = 0xffff,
};

static const char* get_primitive_string(DWORD token)
{
	switch (token)
	{
	case TOKEN_WORD:
		return "WORD";
	case TOKEN_DWORD:
		return "DWORD";
	case TOKEN_FLOAT:
		return "FLOAT";
	case TOKEN_DOUBLE:
		return "DOUBLE";
	case TOKEN_CHAR:
		return "CHAR";
	case TOKEN_UCHAR:
		return "UCHAR";
	case TOKEN_SWORD:
		return "SWORD";
	case TOKEN_SDWORD:
		return "SDWORD";
	case TOKEN_VOID:
		return "VOID";
	case TOKEN_LPSTR:
		return "STRING";
	case TOKEN_UNICODE:
		return "UNICODE";
	case TOKEN_CSTRING:
		return "CSTRING ";
	default:
		break;
	}
	return NULL;
}

static BOOL is_space(char c)
{
	switch (c)
	{
	case 0x00:
	case 0x0D:
	case 0x0A:
	case ' ':
	case '\t':
		return TRUE;
	}
	return FALSE;
}

static BOOL is_operator(char c)
{
	switch (c)
	{
	case '{':
	case '}':
	case '[':
	case ']':
	case '(':
	case ')':
	case '<':
	case '>':
	case ',':
	case ';':
		return TRUE;
	}
	return FALSE;
}

static inline BOOL is_separator(char c)
{
	return is_space(c) || is_operator(c);
}

static WORD get_operator_token(char c)
{
	switch (c)
	{
	case '{':
		return TOKEN_OBRACE;
	case '}':
		return TOKEN_CBRACE;
	case '[':
		return TOKEN_OBRACKET;
	case ']':
		return TOKEN_CBRACKET;
	case '(':
		return TOKEN_OPAREN;
	case ')':
		return TOKEN_CPAREN;
	case '<':
		return TOKEN_OANGLE;
	case '>':
		return TOKEN_CANGLE;
	case ',':
		return TOKEN_COMMA;
	case ';':
		return TOKEN_SEMICOLON;
	}
	return 0;
}