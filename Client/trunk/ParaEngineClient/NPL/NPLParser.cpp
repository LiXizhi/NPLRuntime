//-----------------------------------------------------------------------------
// Class:	NPL Parser
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine tech Studio
// Date:	2007.3.1
// Desc: cross-platformed 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "NPLParser.h"

#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif

using namespace NPL;
using namespace std;
//////////////////////////////////////////////////////////////////////////
//
// NPLLex
//
//////////////////////////////////////////////////////////////////////////

#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')

/* ORDER RESERVED */
static const char* token2string[] = {
	"and", "break", "do", "else", "elseif",
	"end", "false", "for", "function", "if",
	"in", "local", "nil", "not", "or", "repeat",
	"return", "then", "true", "until", "while", "*name",
	"..", "...", "==", ">=", "<=", "~=",
	"*number", "*string", "<eof>"
};

LexState* NPLLex::SetInput(const char* input, int nLen)
{
	m_zio.p = input;
	m_zio.n = nLen;
	if (nLen <= 0)
	{
		m_zio.n = strlen(input);
	}

	m_lexState.bSucceed = true;
	m_lexState.lookahead.token = TK_EOS;  /* no look-ahead token */
	m_lexState.z = &m_zio;
	m_lexState.linenumber = 1;
	m_lexState.lastline = 1;
	m_lexState.buff.clear();

	next(&m_lexState);  /* read first char */
	if (m_lexState.current == '#') {
		do {  /* skip first line */
			next(&m_lexState);
		} while (m_lexState.current != '\n' && m_lexState.current != EOZ);
	}

	return &m_lexState;
}

void NPLLex::ThrowError(LexState* ls, const char* errorMsg)
{
	ls->bSucceed = false;
	throw errorMsg;
}

const char* NPLLex::FormatString(const char* zFormat, ...)
{
	static char buf_[MAX_DEBUG_STRING_LENGTH + 1];
	va_list args;
	va_start(args, zFormat);
	vsnprintf(buf_, MAX_DEBUG_STRING_LENGTH, zFormat, args);
	va_end(args);
	return buf_;
}


const char* NPLLex::luaX_token2str(LexState* ls, int token)
{
	if (token < FIRST_RESERVED) {
		PE_ASSERT(token == (unsigned char)token);
		return FormatString("%c", token);
	}
	else
		return token2string[token - FIRST_RESERVED];
}


void NPLLex::luaX_lexerror(LexState* ls, const char* s, int token)
{
	if (token == TK_EOS)
		luaX_error(ls, s, luaX_token2str(ls, token));
	else
		luaX_error(ls, s, &(ls->buff[0]));
}

void NPLLex::luaX_errorline(LexState* ls, const char* s, const char* token, int line)
{
	ThrowError(ls, FormatString("%d: %s near `%s'", line, s, token));
}

void NPLLex::luaX_error(LexState* ls, const char* s, const char* token)
{
	luaX_errorline(ls, s, token, ls->linenumber);
}

void NPLLex::luaX_syntaxerror(LexState* ls, const char* msg)
{
	const char* lasttoken;
	switch (ls->t.token) {
	case TK_NAME:
		lasttoken = ls->t.seminfo.ts.c_str();
		break;
	case TK_STRING:
	case TK_NUMBER:
		lasttoken = &(ls->buff[0]);
		break;
	default:
		lasttoken = luaX_token2str(ls, ls->t.token);
		break;
	}
	luaX_error(ls, msg, lasttoken);
}

void NPLLex::luaX_checklimit(LexState* ls, int val, int limit, const char* msg)
{
	if (val > limit) {
		msg = FormatString("too many %s (limit=%d)", msg, limit);
		luaX_syntaxerror(ls, msg);
	}
}

void NPLLex::inclinenumber(LexState* LS)
{
	next(LS);  /* skip `\n' */
	++LS->linenumber;
	luaX_checklimit(LS, LS->linenumber, MAX_INT, "lines in a chunk");
}

void NPLLex::read_long_string(LexState* LS, SemInfo* seminfo) {
	int cont = 0;
	int l = 0;
	checkbuffer(LS, l);
	save(LS, '[', l);  /* save first `[' */
	save_and_next(LS, l);  /* pass the second `[' */
	if (currIsNewline(LS))  /* string starts with a newline? */
		inclinenumber(LS);  /* skip it */
	bool bBreak = false;
	for (; !bBreak;) {
		checkbuffer(LS, l);
		switch (LS->current) {
		case EOZ:
			save(LS, '\0', l);
			luaX_lexerror(LS, (seminfo) ? "unfinished long string" :
				"unfinished long comment", TK_EOS);
			break;  /* to avoid warnings */
		case '[':
			save_and_next(LS, l);
			if (LS->current == '[') {
				cont++;
				save_and_next(LS, l);
			}
			continue;
		case ']':
			save_and_next(LS, l);
			if (LS->current == ']') {
				if (cont == 0) bBreak = true;
				cont--;
				save_and_next(LS, l);
			}
			continue;
		case '\n':
		case '\r': // lua 5.1 syntax fix
			save(LS, '\n', l);
			inclinenumber(LS);
			if (!seminfo) l = 0;  /* reset buffer to avoid wasting space */
			continue;
		default:
			save_and_next(LS, l);
			break;
		}
	}
	save(LS, '\0', l);
	if (seminfo)
	{
		seminfo->ts.clear();
		seminfo->ts.append(&(LS->buff[2]), l - 5);
	}
}

void NPLLex::read_string(LexState* LS, int del, SemInfo* seminfo)
{
	int l = 0;
	checkbuffer(LS, l);
	save_and_next(LS, l);
	while (LS->current != del) {
		checkbuffer(LS, l);
		switch (LS->current) {
		case EOZ:
			save(LS, '\0', l);
			luaX_lexerror(LS, "unfinished string", TK_EOS);
			break;  /* to avoid warnings */
		case '\n':
		case '\r': // lua 5.1 syntax fix
			save(LS, '\0', l);
			luaX_lexerror(LS, "unfinished string", TK_STRING);
			break;  /* to avoid warnings */
		case '\\':
			next(LS);  /* do not save the `\' */
			switch (LS->current) {
			case 'a': save(LS, '\a', l); next(LS); break;
			case 'b': save(LS, '\b', l); next(LS); break;
			case 'f': save(LS, '\f', l); next(LS); break;
			case 'n': save(LS, '\n', l); next(LS); break;
			case 'r': save(LS, '\r', l); next(LS); break;
			case 't': save(LS, '\t', l); next(LS); break;
			case 'v': save(LS, '\v', l); next(LS); break;
			case '\n':
			case '\r': // lua 5.1 syntax fix
				save(LS, '\n', l); inclinenumber(LS); break;
			case EOZ: break;  /* will raise an error next loop */
			default: {
				if (!isdigit(LS->current))
					save_and_next(LS, l);  /* handles \\, \", \', and \? */
				else {  /* \xxx */
					int c = 0;
					int i = 0;
					do {
						c = 10 * c + (LS->current - '0');
						next(LS);
					} while (++i < 3 && isdigit(LS->current));
					if (c > UCHAR_MAX) {
						save(LS, '\0', l);
						luaX_lexerror(LS, "escape sequence too large", TK_STRING);
					}
					save(LS, c, l);
				}
				break;
			}
			}
			break;
		default:
			save_and_next(LS, l);
			break;
		}
	}
	save_and_next(LS, l);  /* skip delimiter */
	save(LS, '\0', l);
	if (seminfo)
	{
		seminfo->ts.clear();
		seminfo->ts.append(&(LS->buff[1]), l - 3);
	}
}

int NPLLex::readname(LexState* LS)
{
	int l = 0;
	checkbuffer(LS, l);
	do {
		checkbuffer(LS, l);
		save_and_next(LS, l);
	} while (isalnum(LS->current) || LS->current == '_');
	save(LS, '\0', l);
	return l - 1;
}

void NPLLex::read_numeral(LexState* LS, int comma, SemInfo* seminfo)
{
	int l = 0;
	checkbuffer(LS, l);
	if (comma) save(LS, '.', l);
	while (isdigit(LS->current)) {
		checkbuffer(LS, l);
		save_and_next(LS, l);
	}
	if (LS->current == '.') {
		save_and_next(LS, l);
		if (LS->current == '.') {
			save_and_next(LS, l);
			save(LS, '\0', l);
			luaX_lexerror(LS,
				"ambiguous syntax (decimal point x string concatenation)",
				TK_NUMBER);
		}
	}
	while (isdigit(LS->current)) {
		checkbuffer(LS, l);
		save_and_next(LS, l);
	}
	if (LS->current == 'e' || LS->current == 'E') {
		save_and_next(LS, l);  /* read `E' */
		if (LS->current == '+' || LS->current == '-')
			save_and_next(LS, l);  /* optional exponent sign */
		while (isdigit(LS->current)) {
			checkbuffer(LS, l);
			save_and_next(LS, l);
		}
	}
	save(LS, '\0', l);
	try
	{
		seminfo->r = atof(&LS->buff[0]);
	}
	catch (...)
	{
		luaX_lexerror(LS, "malformed number", TK_NUMBER);
	}
}

int NPLLex::luaX_lex(LexState* LS, SemInfo* seminfo)
{
	for (;;) {
		switch (LS->current) {

		case '\n':
		case '\r': {
			inclinenumber(LS);
			continue;
		}
		case '-': {
			next(LS);
			if (LS->current != '-') return '-';
			/* else is a comment */
			next(LS);
			if (LS->current == '[' && (next(LS) == '['))
				read_long_string(LS, NULL);  /* long comment */
			else  /* short comment */
				while (!currIsNewline(LS) && LS->current != EOZ)
					next(LS);
			continue;
		}
		case '[': {
			next(LS);
			if (LS->current != '[') return '[';
			else {
				read_long_string(LS, seminfo);
				return TK_STRING;
			}
		}
		case '=': {
			next(LS);
			if (LS->current != '=') return '=';
			else { next(LS); return TK_EQ; }
		}
		case '<': {
			next(LS);
			if (LS->current != '=') return '<';
			else { next(LS); return TK_LE; }
		}
		case '>': {
			next(LS);
			if (LS->current != '=') return '>';
			else { next(LS); return TK_GE; }
		}
		case '~': {
			next(LS);
			if (LS->current != '=') return '~';
			else { next(LS); return TK_NE; }
		}
		case '"':
		case '\'': {
			read_string(LS, LS->current, seminfo);
			return TK_STRING;
		}
		case '.': {
			next(LS);
			if (LS->current == '.') {
				next(LS);
				if (LS->current == '.') {
					next(LS);
					return TK_DOTS;   /* ... */
				}
				else return TK_CONCAT;   /* .. */
			}
			else if (!isdigit(LS->current)) return '.';
			else {
				read_numeral(LS, 1, seminfo);
				return TK_NUMBER;
			}
		}
		case EOZ: {
			return TK_EOS;
		}
		default: {
			if (isspace(LS->current)) {
				next(LS);
				continue;
			}
			else if (isdigit(LS->current)) {
				read_numeral(LS, 0, seminfo);
				return TK_NUMBER;
			}
			else if (isalpha(LS->current) || LS->current == '_') {
				/* identifier or reserved word */
				int l = readname(LS);
				string ts;
				ts.append(&(LS->buff[0]), l);

				// The following code implemented below
				//if (ts->tsv.reserved > 0)  /* reserved word? */
				// 	return ts->tsv.reserved - 1 + FIRST_RESERVED;

				/* reserved word? */
				for (int i = 0; i < NUM_RESERVED; ++i)
				{
					if (ts == token2string[i])
						return i + FIRST_RESERVED;
				}

				seminfo->ts = ts;
				return TK_NAME;
			}
			else {
				int c = LS->current;
				if (iscntrl(c))
					luaX_error(LS, "invalid control char",
						FormatString("char(%d)", c));
				next(LS);
				return c;  /* single-char tokens (+ - / ...) */
			}
			break;
		}
		}
	}//for (;;) {
}

//////////////////////////////////////////////////////////////////////////
//
// NPLParser
//
//////////////////////////////////////////////////////////////////////////

NPLParser::NPLParser(void)
{
}

NPLParser::~NPLParser(void)
{
}

void NPLParser::next(LexState* ls)
{
	ls->lastline = ls->linenumber;
	if (ls->lookahead.token != NPLLex::TK_EOS) {  /* is there a look-ahead token? */
		ls->t = ls->lookahead;  /* use this one */
		ls->lookahead.token = NPLLex::TK_EOS;  /* and discharge it */
	}
	else
		ls->t.token = NPLLex::luaX_lex(ls, &ls->t.seminfo);  /* read next token */
}

void NPLParser::lookahead(LexState* ls)
{
	PE_ASSERT(ls->lookahead.token == NPLLex::TK_EOS);
	ls->lookahead.token = NPLLex::luaX_lex(ls, &ls->lookahead.seminfo);
}

void NPLParser::error_expected(LexState* ls, int token)
{
	NPLLex::luaX_syntaxerror(ls,
		NPLLex::FormatString("`%s' expected", NPLLex::luaX_token2str(ls, token)));
}

int NPLParser::testnext(LexState* ls, int c)
{
	if (ls->t.token == c) {
		next(ls);
		return 1;
	}
	else return 0;
}

void NPLParser::check_condition(LexState* ls, void* c, const char* msg)
{
	if (!(c))
		NPLLex::luaX_syntaxerror(ls, msg);
}

void NPLParser::check_match(LexState* ls, int what, int who, int where)
{
	if (!testnext(ls, what)) {
		if (where == ls->linenumber)
			error_expected(ls, what);
		else {
			NPLLex::luaX_syntaxerror(ls, NPLLex::FormatString("`%s' expected (to close `%s' at line %d)",
				NPLLex::luaX_token2str(ls, what), NPLLex::luaX_token2str(ls, who), where));
		}
	}
}

void NPLParser::check(LexState* ls, int c)
{
	if (!testnext(ls, c))
		error_expected(ls, c);
}

bool NPLParser::CheckPureDataBlock(LexState* ls)
{
	// data
	int c = ls->t.token;
	if (c == NPLLex::TK_TRUE || c == NPLLex::TK_NIL || c == NPLLex::TK_FALSE || c == NPLLex::TK_NUMBER || c == NPLLex::TK_STRING)
	{
		next(ls);
		return true;
	}
	else if (c == '-')
	{
		// negative number
		next(ls);
		if (ls->t.token == NPLLex::TK_NUMBER)
		{
			next(ls);
			return true;
		}
		else
			return false;
	}
	else if (c == '{')
	{
		enterlevel(ls);
		bool bBreak = false;
		next(ls);
		while (!bBreak)
		{
			c = ls->t.token;
			if (c == '}')
			{
				// end of table
				leavelevel(ls);
				next(ls);
				bBreak = true;
			}
			else if (c == NPLLex::TK_NAME)
			{
				// by name assignment, such as name = data|table
				next(ls);
				if (ls->t.token == '=')
				{
					next(ls);
					if (!CheckPureDataBlock(ls))
						return false;
					testnext(ls, ',');
				}
				else
					return false;
			}
			else if (c == '[')
			{
				// by integer or string key assignment, such as [number|string] = data|table
				next(ls);
				if (ls->t.token == NPLLex::TK_NUMBER)
				{
					// TODO: verify that it is an integer, instead of a floating value.
				}
				else if (ls->t.token == NPLLex::TK_STRING)
				{
					// verify that the string is a value key(non-empty);
					if (ls->t.seminfo.ts.empty())
						return false;
				}
				else
					return false;
				next(ls);
				if (ls->t.token == ']')
				{
					next(ls);
					if (ls->t.token == '=')
					{
						next(ls);
						if (!CheckPureDataBlock(ls))
							return false;
						testnext(ls, ',');
					}
					else
						return false;
				}
			}
			/// Fixed: 2008.6.3 LiXizhi
			/// the following is for auto indexed table items {"string1", "string2\r\n", 213, nil,["A"]="B", true, false, {"another table", "field1"}}
			else if (c == NPLLex::TK_STRING || c == NPLLex::TK_NUMBER || c == NPLLex::TK_NIL || c == NPLLex::TK_FALSE || c == NPLLex::TK_TRUE)
			{
				next(ls);
				testnext(ls, ',');
			}
			else if (c == '{')
			{
				if (!CheckPureDataBlock(ls))
					return false;
				testnext(ls, ',');
			}
			else
			{
				return false;
			}
		};
		return true;
	}
	else
	{
		return false;
	}
}

bool NPLParser::IsPureData(const char* input, int nLen)
{
	NPLLex lex;
	LexState* ls = lex.SetInput(input, nLen);
	ls->nestlevel = 0;
	try
	{
		next(ls);  /* read first token */
		if (CheckPureDataBlock(ls))
		{
			testnext(ls, ';');
			if (ls->t.token == NPLLex::TK_EOS)
			{
				return true;
			}
		}
	}
	catch (const char* err)
	{
		OUTPUT_DEBUG(err);
		OUTPUT_DEBUG("\r\n");
		return false;
	}
	catch (...)
	{
		OUTPUT_DEBUG("error: unknown error in NPLParser::IsPureData()\r\n");
		return false;
	}
	return false;
}

bool NPLParser::IsPureTable(const char* input, int nLen)
{
	NPLLex lex;
	LexState* ls = lex.SetInput(input, nLen);
	ls->nestlevel = 0;
	try
	{
		next(ls);  /* read first token */
		int c = ls->t.token;
		if (c == '{')
		{
			if (CheckPureDataBlock(ls))
			{
				testnext(ls, ';');
				if (ls->t.token == NPLLex::TK_EOS)
				{
					return true;
				}
			}
		}
	}
	catch (const char* err)
	{
		OUTPUT_DEBUG(err);
		OUTPUT_DEBUG("\r\n");
		return false;
	}
	catch (...)
	{
		OUTPUT_DEBUG("error: unknown error in NPLParser::IsPureTable()\r\n");
		return false;
	}
	return false;
}

bool NPLParser::IsMsgData(const char* input, int nLen)
{
	NPLLex lex;
	LexState* ls = lex.SetInput(input, nLen);
	ls->nestlevel = 0;

	try
	{
		next(ls);  /* read first token */

		if (ls->t.token == NPLLex::TK_NAME && ls->t.seminfo.ts == "msg")
		{
			next(ls);
			if (ls->t.token == '=')
			{
				next(ls);
				if (CheckPureDataBlock(ls))
				{
					testnext(ls, ';');
					if (ls->t.token == NPLLex::TK_EOS)
					{
						return true;
					}
				}
			}
		}
	}
	catch (const char* err)
	{
		OUTPUT_DEBUG(err);
		OUTPUT_DEBUG("\r\n");
		return false;
	}
	catch (...)
	{
		OUTPUT_DEBUG("error: unknown error in NPLParser::IsMsgData()\r\n");
		return false;
	}
	return false;
}

bool NPLParser::IsIdentifier(const char* str, int nLength)
{
	bool bIsIdentifier = !ParaEngine::StringHelper::isdigit(str[0]);
	for (int i = 0; i < nLength && bIsIdentifier; ++i)
	{
		char c = str[i];
		bIsIdentifier = (ParaEngine::StringHelper::isalnum(c) || c == '_');
	}
	return bIsIdentifier;
}
