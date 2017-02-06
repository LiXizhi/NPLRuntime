#pragma once
//-----------------------------------------------------------------------------
// Class:	NPLInterface
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.2.23
// Desc: This file can be used by a ParaEngine plugin to parse NPL messages without the need to link with the ParaEngine library. 
//-----------------------------------------------------------------------------
/** if this is defined outside this file, we will never use any static variables that contains destructor in this class.
* this is usually required by C++/CLI project, where no static variable can be used.
*/
// #define DISABLE_STATIC_VARIABLE
#include <boost/intrusive_ptr.hpp>
#include "PEtypes.h"
#include "NPLTypes.h"
#include "INPL.h"
#include <vector>
#include <map>

#pragma region CommonHeaders
#ifndef MAX_TABLE_STRING_LENGTH
#define MAX_TABLE_STRING_LENGTH 512
#endif

#ifndef MAX_DEBUG_STRING_LENGTH
#define MAX_DEBUG_STRING_LENGTH 1024
#endif

#ifndef OUTPUT_LOG
#define OUTPUT_LOG	printf
#endif

#ifndef LUA_NUMBER_FMT
#define LUA_NUMBER_FMT		"%.14g"
#endif

#ifdef WIN32
#define snprintf _snprintf
#endif


namespace NPLInterface
{
	/** single-threaded reference counted base class for boost::intrusive_ptr
	* all boost::intrusive_ptr<T>, should derive from this class.
	* this is NOT thead safe, use a lock when used by multiple thread.
	e.g.
	Class Foo : public intrusive_ptr_single_thread_base
	{
	// members
	}
	// so now Foo_ptr is the same as boost::shared_ptr
	typedef boost::intrusive_ptr<Foo> Foo_ptr;
	*/
	class intrusive_ptr_single_thread_base
	{
	public:
		mutable long m_ref_count;

		intrusive_ptr_single_thread_base() : m_ref_count(0){};
		virtual ~intrusive_ptr_single_thread_base(){};
	};

	/** Here we defined these two functions in NPLInterface namespace other than boost namespace, 
	this requires a modern compiler with dependency name lookup*/

	template <typename T>
	void intrusive_ptr_add_ref(T* ref)
	{
		// increment reference count of object *ref
		++(ref->m_ref_count);
	}

	template <typename T>
	void intrusive_ptr_release(T* ref)
	{
		// decrement reference count, and delete object when reference count reaches 0
		if (--(ref->m_ref_count) == 0)
			delete ref;
	}
}
#pragma endregion CommonHeaders

namespace NPLInterface
{
	using std::vector;

	struct LexState;
	class NPLObjectBase;
	class NPLNumberObject;
	class NPLBoolObject;
	class NPLTable;
	class NPLStringObject;
	typedef boost::intrusive_ptr<NPLObjectBase> NPLObjectBase_ptr;
	typedef boost::intrusive_ptr<NPLTable> NPLTable_ptr;
	typedef boost::intrusive_ptr<NPLNumberObject> NPLNumberObject_ptr;
	typedef boost::intrusive_ptr<NPLBoolObject> NPLBoolObject_ptr;
	typedef boost::intrusive_ptr<NPLStringObject> NPLStringObject_ptr;

#pragma region NPLParser
	/* semantics information */
	struct SemInfo
	{
	public:
		double r;
		string ts;
	};

	struct Token
	{
		int token;
		SemInfo seminfo;
	};

	struct  Zio {
		size_t n;			/* bytes still unread */
		const char *p;		/* current position in buffer */

		Zio(){};
		Zio(const char *input, size_t nSize) :p(input), n(nSize){};
	};

	/** lex state */
	struct LexState
	{
		int current;  /* current character (charint) */
		int linenumber;  /* input line counter */
		int lastline;  /* line of last token `consumed' */
		Token t;  /* current token */
		Token lookahead;  /* look ahead token */
		Zio* z;  /* input stream */
		vector<char> buff;  /* buffer for tokens */
		int nestlevel;  /* level of nested non-terminals */
		/** true if no error*/
		bool bSucceed;
	};

#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')

	/* ORDER RESERVED */
	static const char * token2string[] = {
		"and", "break", "do", "else", "elseif",
		"end", "false", "for", "function", "if",
		"in", "local", "nil", "not", "or", "repeat",
		"return", "then", "true", "until", "while", "*name",
		"..", "...", "==", ">=", "<=", "~=",
		"*number", "*string", "<eof>"
	};

	/** for lexer for NPL files */
	class NPLLex
	{
	public:
		/* end of stream */
		static const int EOZ = -1;
		static const int MAX_INT = 65530;
		static const int FIRST_RESERVED = 257;

		/* maximum number of chars that can be read without checking buffer size */
		static const int MAXNOCHECK = 5;
		/* extra space to allocate when growing buffer */
		static const int EXTRABUFF = 32;

		/* maximum length of a reserved word */
		static const int TOKEN_LEN = (sizeof("function") / sizeof(char));

		enum RESERVED {
			/* terminal symbols denoted by reserved words */
			TK_AND = FIRST_RESERVED, TK_BREAK,
			TK_DO, TK_ELSE, TK_ELSEIF, TK_END, TK_FALSE, TK_FOR, TK_FUNCTION,
			TK_IF, TK_IN, TK_LOCAL, TK_NIL, TK_NOT, TK_OR, TK_REPEAT,
			TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,
			/* other terminal symbols */
			TK_NAME, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE, TK_NUMBER,
			TK_STRING, TK_EOS
		};
		/* number of reserved words */
		static const int NUM_RESERVED = (int)TK_WHILE - FIRST_RESERVED + 1;

	private:
		LexState m_lexState;
		Zio m_zio;
	public:
		LexState * GetState(){ return &m_lexState; };

		LexState * SetInput(const char* input, int nLen)
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

		static void ThrowError(LexState *ls, const char* errorMsg)
		{
			ls->bSucceed = false;
			throw errorMsg;
		}

		static const char* FormatString(const char * zFormat, ...)
		{
			static char buf_[MAX_DEBUG_STRING_LENGTH + 1];
			va_list args;
			va_start(args, zFormat);
			vsnprintf(buf_, MAX_DEBUG_STRING_LENGTH, zFormat, args);
			va_end(args);
			return buf_;
		}


		static const char * luaX_token2str(LexState *ls, int token)
		{
			if (token < FIRST_RESERVED) {
				assert(token == (unsigned char)token);
				return FormatString("%c", token);
			}
			else
				return token2string[token - FIRST_RESERVED];
		}


		static void luaX_lexerror(LexState *ls, const char *s, int token)
		{
			if (token == TK_EOS)
				luaX_error(ls, s, luaX_token2str(ls, token));
			else
				luaX_error(ls, s, &(ls->buff[0]));
		}

		static void luaX_errorline(LexState *ls, const char *s, const char *token, int line)
		{
			ThrowError(ls, FormatString("%d: %s near `%s'", line, s, token));
		}

		static void luaX_error(LexState *ls, const char *s, const char *token)
		{
			luaX_errorline(ls, s, token, ls->linenumber);
		}

		static void luaX_syntaxerror(LexState *ls, const char *msg)
		{
			const char *lasttoken;
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

		static void luaX_checklimit(LexState *ls, int val, int limit, const char *msg)
		{
			if (val > limit) {
				msg = FormatString("too many %s (limit=%d)", msg, limit);
				luaX_syntaxerror(ls, msg);
			}
		}

		static void inclinenumber(LexState *LS)
		{
			next(LS);  /* skip `\n' */
			++LS->linenumber;
			luaX_checklimit(LS, LS->linenumber, MAX_INT, "lines in a chunk");
		}

		static void read_long_string(LexState *LS, SemInfo *seminfo) {
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
			save_and_next(LS, l);  /* skip the second `]' */
			save(LS, '\0', l);
			if (seminfo)
			{
				seminfo->ts.clear();
				seminfo->ts.append(&(LS->buff[2]), l - 5);
			}
		}

		static void read_string(LexState *LS, int del, SemInfo *seminfo)
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
							} while (++i<3 && isdigit(LS->current));
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

		static int readname(LexState *LS)
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

		static void read_numeral(LexState *LS, int comma, SemInfo *seminfo)
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

		static int luaX_lex(LexState *LS, SemInfo *seminfo)
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

	private:
		//////////////////////////////////////////////////////////////////////////
		// static lex functions
		//////////////////////////////////////////////////////////////////////////
		static inline int next(LexState *LS)
		{
			return LS->current = ((LS->z)->n--) > 0 ? (int)((unsigned char)(*(LS->z)->p++)) : EOZ;
		}

		static void checkbuffer(LexState *LS, int len)
		{
			if (((len)+MAXNOCHECK)*sizeof(char) > LS->buff.size())
			{
				if (len < 500)
					LS->buff.resize(len + EXTRABUFF);
				else
				{
					// added by LiXizhi 2007.6.20: in case the file contains super large string, such as a base64 encoded file of 2MB, we will double the size instead using a fixed length.
					LS->buff.resize(len * 2);
				}
			}
		}
		static void save(LexState *LS, char c, int& l)
		{
			LS->buff[l++] = (char)c;
		}
		static void save_and_next(LexState *LS, int& l)
		{
			save(LS, LS->current, l);
			next(LS);
		}


	};

	/** parser for NPL files.
	* Internally it uses NPLLexer
	*/
	class NPLParser
	{
	public:
		/*
		** maximum number of syntactical nested non-terminals: Not too big,
		** or may overflow the C stack...
		*/
		static const int LUA_MAXPARSERLEVEL = 200;

	public:

		NPLParser(void)
		{
		}

		~NPLParser(void)
		{
		}

		static void next(LexState *ls)
		{
			ls->lastline = ls->linenumber;
			if (ls->lookahead.token != NPLLex::TK_EOS) {  /* is there a look-ahead token? */
				ls->t = ls->lookahead;  /* use this one */
				ls->lookahead.token = NPLLex::TK_EOS;  /* and discharge it */
			}
			else
				ls->t.token = NPLLex::luaX_lex(ls, &ls->t.seminfo);  /* read next token */
		}

		static void lookahead(LexState *ls)
		{
			assert(ls->lookahead.token == NPLLex::TK_EOS);
			ls->lookahead.token = NPLLex::luaX_lex(ls, &ls->lookahead.seminfo);
		}

		static void error_expected(LexState *ls, int token)
		{
			NPLLex::luaX_syntaxerror(ls,
				NPLLex::FormatString("`%s' expected", NPLLex::luaX_token2str(ls, token)));
		}

		static int testnext(LexState *ls, int c)
		{
			if (ls->t.token == c) {
				next(ls);
				return 1;
			}
			else return 0;
		}

		static void check_condition(LexState *ls, void* c, const char* msg)
		{
			if (!(c))
				NPLLex::luaX_syntaxerror(ls, msg);
		}

		static void check_match(LexState *ls, int what, int who, int where)
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

		static void check(LexState *ls, int c)
		{
			if (!testnext(ls, c))
				error_expected(ls, c);
		}

		static bool CheckPureDataBlock(LexState *ls)
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

		static bool IsPureData(const char* input, int nLen)
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
				OUTPUT_LOG(err);
				OUTPUT_LOG("\r\n");
				return false;
			}
			catch (...)
			{
				OUTPUT_LOG("error: unknown error in NPLParser::IsPureData()\r\n");
				return false;
			}
			return false;
		}

		static bool IsPureTable(const char* input, int nLen)
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
				OUTPUT_LOG(err);
				OUTPUT_LOG("\r\n");
				return false;
			}
			catch (...)
			{
				OUTPUT_LOG("error: unknown error in NPLParser::IsPureTable()\r\n");
				return false;
			}
			return false;
		}

		static bool IsMsgData(const char* input, int nLen)
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
				OUTPUT_LOG(err);
				OUTPUT_LOG("\r\n");
				return false;
			}
			catch (...)
			{
				OUTPUT_LOG("error: unknown error in NPLParser::IsMsgData()\r\n");
				return false;
			}
			return false;
		}

		static bool IsIdentifier(const char* str, int nLength)
		{
			bool bIsIdentifier = !isdigit(str[0]);
			for (int i = 0; i < nLength && bIsIdentifier; ++i)
			{
				char c = str[i];
				bIsIdentifier = (isalnum(c) || c == '_');
			}
			return bIsIdentifier;
		}

		static void enterlevel(LexState *ls)
		{
			if (++(ls)->nestlevel > LUA_MAXPARSERLEVEL)
				NPLLex::luaX_syntaxerror(ls, "too many syntax levels");
		}

		static void leavelevel(LexState *ls)
		{
			((ls)->nestlevel--);
		}


	};



#pragma endregion NPLParser

#pragma region NPLObjectProxy

	/** base class for all NPL date members. Do not use this class directly. Use NPLObjectProxy*/
	class NPLObjectBase : public intrusive_ptr_single_thread_base
	{
	public:
		enum NPLObjectType
		{
			NPLObjectType_Nil,
			NPLObjectType_Table,
			NPLObjectType_Number,
			NPLObjectType_String,
			NPLObjectType_Bool,
		};
		NPLObjectBase() :m_type(NPLObjectType_Nil){};
		virtual ~NPLObjectBase(){};

		inline NPLObjectType GetType(){ return m_type; }

	protected:
		NPLObjectType m_type;
	};

	/** a floating point number */
	class NPLNumberObject : public NPLObjectBase
	{
	public:
		NPLNumberObject() :m_value(0){ m_type = NPLObjectType_Number; };
		/** create the table from a serialized string. */
		NPLNumberObject(double value) :m_value(value){ m_type = NPLObjectType_Number; };

		virtual ~NPLNumberObject(){};

		void SetValue(double value){ m_value = value; }
		double GetValue() { return m_value; }

		NPLNumberObject& operator = (double value) { SetValue(value); return *this; }

	private:
		double m_value;
	};


	/** a boolean */
	class NPLBoolObject : public NPLObjectBase
	{
	public:
		NPLBoolObject() :m_value(false){ m_type = NPLObjectType_Bool; };
		/** create the table from a serialized string. */
		NPLBoolObject(bool value) :m_value(value){ m_type = NPLObjectType_Bool; };

		virtual ~NPLBoolObject(){};

		inline void SetValue(bool value){ m_value = value; }
		bool GetValue() { return m_value; }

		NPLBoolObject& operator = (bool value) { SetValue(value); return *this; }

	private:
		bool m_value;
	};


	/** a floating point number */
	class NPLStringObject : public NPLObjectBase
	{
	public:
		NPLStringObject(){ m_type = NPLObjectType_String; };
		/** create the table from a serialized string. */
		NPLStringObject(const std::string& value) :m_value(value){ m_type = NPLObjectType_String; };

		virtual ~NPLStringObject(){};

		void SetValue(const std::string& value){ m_value = value; }
		std::string& GetValue(){ return m_value; }

		NPLStringObject& operator = (const std::string& value) { SetValue(value); return *this; }

	private:
		std::string m_value;
	};

	/** NPL object proxy. This is both a smart pointer and accessors
	// Example 1: Create NPLTable and serialize to string
	NPLObjectProxy msg;
	msg["nid"] = (double)10;
	msg["name"] = "value";
	msg["tab"]["name1"] = "value1";
	NPLHelper::SerializeNPLTableToString(NULL, msg, strOutput);

	// Example 2: deserialize NPLTable from string.

	NPLObjectProxy tabMsg = NPLHelper::StringToNPLTable("{nid=10, name=\"value\", tab={name1=\"value1\"}}");
	assert((double)tabMsg["nid"] == 10);
	assert((tabMsg["name"]) == "value");
	assert((tabMsg["tab"]["name1"]) == "value1");
	*/
	template <class T>
	class NPLObjectProxyT : public NPLObjectBase_ptr
	{
	public:
		NPLObjectProxyT() {};
		NPLObjectProxyT(NPLObjectBase* pObject) : NPLObjectBase_ptr(pObject)
		{

		}

		operator double()
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Number)
				*this = NPLObjectProxyT(new NPLNumberObject());
			return ((NPLNumberObject*)get())->GetValue();
		}

		void operator = (double value)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Number)
				*this = NPLObjectProxyT(new NPLNumberObject());
			((NPLNumberObject*)get())->SetValue(value);
		}

		operator bool()
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Bool)
				*this = NPLObjectProxyT(new NPLBoolObject());
			return (((NPLBoolObject*)get())->GetValue());
		}

		void operator = (bool value)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Bool)
				*this = NPLObjectProxyT(new NPLBoolObject());
			((NPLBoolObject*)get())->SetValue(value);

		}

		operator const string& ()
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_String)
				*this = NPLObjectProxyT(new NPLStringObject());
			return (((NPLStringObject*)get())->GetValue());
		}

		bool operator == (const string& value)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_String)
				return false;
			else
				return ((NPLStringObject*)get())->GetValue() == value;
		}
		bool operator == (const char* value)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_String)
				return false;
			else
				return ((NPLStringObject*)get())->GetValue() == value;
		}

		void operator = (const std::string& value)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_String)
				*this = NPLObjectProxyT(new NPLStringObject());
			((NPLStringObject*)get())->SetValue(value);
		}

		void operator = (const char* value)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_String)
				*this = NPLObjectProxyT(new NPLStringObject());
			((NPLStringObject*)get())->SetValue(value);
		}

		NPLObjectProxyT& operator [] (const string& sName)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
				*this = NPLObjectProxyT(new T());
			return ((T*)get())->CreateGetField(sName);
		};

		NPLObjectProxyT& operator [] (const char* sName)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
				*this = NPLObjectProxyT(new T());
			return ((T*)get())->CreateGetField(sName);

		};

		NPLObjectProxyT& operator [](int nIndex)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
				*this = NPLObjectProxyT(new T());
			return ((T*)get())->CreateGetField(nIndex);
		};

		NPLObjectProxyT GetField(const string& sName)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
			{
				return NPLObjectProxyT();
			}
			else
			{
				return ((T*)get())->GetField(sName);
			}
		}


		NPLObjectProxyT GetField(const char* sName)
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
			{
				return NPLObjectProxyT();
			}
			else
			{
				return ((T*)get())->GetField(sName);
			}
		}

		void MakeNil()
		{
			reset();
		}

		typename T::Iterator_Type begin()
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
				*this = NPLObjectProxyT(new T());
			return ((T*)get())->begin();
		};

		typename T::Iterator_Type end()
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
				*this = NPLObjectProxyT(new T());
			return ((T*)get())->end();
		};

		typename T::IndexIterator_Type index_begin()
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
				*this = NPLObjectProxyT(new T());
			return ((T*)get())->index_begin();
		};

		typename T::IndexIterator_Type index_end()
		{
			if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
				*this = NPLObjectProxyT(new T());
			return ((T*)get())->index_end();
		};

		/** get the type */
		NPLObjectBase::NPLObjectType  GetType()
		{
			return (get() != 0) ? get()->GetType() : NPLObjectBase::NPLObjectType_Nil;
		}
	};


	typedef NPLObjectProxyT<NPLTable> NPLObjectProxy;

	/** this is a pure c++ implementation of lua table. it can convert back and force from string.
	* only data members are supported. This class is mostly used by C++ plugin modules.
	* @remark: Use NPLObjectProxy instead of this class.
	* Example:
	// Example 1: Create NPLTable and serialize to string
	NPLObjectProxy msg;
	msg["nid"] = (double)10;
	msg["name"] = "value";
	msg["tab"]["name1"] = "value1";
	NPLHelper::SerializeNPLTableToString(NULL, msg, strOutput);

	// Example 2: deserialize NPLTable from string.

	NPLObjectProxy tabMsg = NPLHelper::StringToNPLTable("{nid=10, name=\"value\", tab={name1=\"value1\"}}");
	assert((double)tabMsg["nid"] == 10);
	assert((tabMsg["name"]) == "value");
	assert((tabMsg["tab"]["name1"]) == "value1");

	* @note: no cyclic link is allowed.
	*/
	class NPLTable : public NPLObjectBase
	{
	public:
		typedef std::map<std::string, NPLObjectProxy>	TableFieldMap_Type;
		typedef std::map<int, NPLObjectProxy>			TableIntFieldMap_Type;
		typedef TableFieldMap_Type::iterator			Iterator_Type;
		typedef TableIntFieldMap_Type::iterator			IndexIterator_Type;
		

		/** this is an empty table*/
		NPLTable(){ m_type = NPLObjectType_Table; };

		virtual ~NPLTable(){
			Clear();
		}

	public:

		void ToString(std::string& str)
		{
		}

		void Clear()
		{
			m_fields.clear();
		}

		void SetField(const string& sName, const NPLObjectProxy& pObject)
		{
			TableFieldMap_Type::iterator iter = m_fields.find(sName);
			if (iter == m_fields.end())
			{
				if (pObject.get() != 0)
				{
					m_fields[sName] = pObject;
				}
			}
			else
			{
				if (pObject.get() != 0)
				{
					iter->second = pObject;
				}
				else
				{
					m_fields.erase(iter);
				}
			}
		}

		void SetField(int nIndex, const NPLObjectProxy& pObject)
		{
			TableIntFieldMap_Type::iterator iter = m_index_fields.find(nIndex);
			if (iter == m_index_fields.end())
			{
				if (pObject.get() != 0)
				{
					m_index_fields[nIndex] = pObject;
				}
			}
			else
			{
				if (pObject.get() != 0)
				{
					iter->second = pObject;
				}
				else
				{
					m_index_fields.erase(iter);
				}
			}
		}

		NPLObjectProxy GetField(int nIndex)
		{
			TableIntFieldMap_Type::iterator iter = m_index_fields.find(nIndex);
			return (iter != m_index_fields.end()) ? iter->second : NPLObjectProxy();
		}

		NPLObjectProxy GetField(const string& sName)
		{
			TableFieldMap_Type::iterator iter = m_fields.find(sName);
			return (iter != m_fields.end()) ? iter->second : NPLObjectProxy();
		}

		NPLObjectProxy& CreateGetField(int nIndex)
		{
			return m_index_fields[nIndex];
		}

		NPLObjectProxy& CreateGetField(const string& sName)
		{
			return m_fields[sName];
		}

		Iterator_Type begin() { return m_fields.begin(); };
		Iterator_Type end() { return m_fields.end(); };

		IndexIterator_Type index_begin() { return m_index_fields.begin(); };
		IndexIterator_Type index_end() { return m_index_fields.end(); };

		/** this will create get field. */
		NPLObjectProxy& operator [](const string& sName) { return CreateGetField(sName); };
		NPLObjectProxy& operator [](const char* sName) { return CreateGetField(sName); };
		NPLObjectProxy& operator [](int nIndex) { return CreateGetField(nIndex); };
	private:
		TableFieldMap_Type	m_fields;
		TableIntFieldMap_Type m_index_fields;
	};


#pragma endregion NPLObjectProxy

#pragma region NPLHelper

	/** NPL helper class */
	class NPLHelper
	{
	public:
		static bool DeserializePureNPLDataBlock(LexState *ls, NPLObjectProxy& objProxy)
		{
			// data
			int c = ls->t.token;
			switch (c)
			{
			case NPLLex::TK_TRUE:
				objProxy = true;
				NPLParser::next(ls);
				return true;
			case NPLLex::TK_FALSE:
				objProxy = false;
				NPLParser::next(ls);
				return true;
			case NPLLex::TK_NIL:
				objProxy.MakeNil();
				NPLParser::next(ls);
				return true;
			case NPLLex::TK_NUMBER:
				objProxy = ls->t.seminfo.r;
				NPLParser::next(ls);
				return true;
			case NPLLex::TK_STRING:
				objProxy = ls->t.seminfo.ts;
				NPLParser::next(ls);
				return true;
			case '-':
			{
				// negative number
				NPLParser::next(ls);
				if (ls->t.token == NPLLex::TK_NUMBER)
				{
					objProxy = -ls->t.seminfo.r;
					NPLParser::next(ls);
					return true;
				}
				else
					return false;
			}
			case '{':
			{
				// table object
				NPLObjectProxy tabMsg;

				NPLParser::enterlevel(ls);
				bool bBreak = false;
				NPLParser::next(ls);
				// auto table index that begins with 1. 
				int nTableAutoIndex = 1;
				while (!bBreak)
				{
					c = ls->t.token;
					if (c == '}')
					{
						// end of table
						NPLParser::leavelevel(ls);
						NPLParser::next(ls);
						bBreak = true;
						objProxy = tabMsg;
					}
					else if (c == NPLLex::TK_NAME)
					{
						NPLObjectProxy& proxy_ = tabMsg[ls->t.seminfo.ts];
						// by name assignment, such as name = data|table
						NPLParser::next(ls);
						if (ls->t.token == '=')
						{
							NPLParser::next(ls);
							if (!DeserializePureNPLDataBlock(ls, proxy_))
								return false;
							NPLParser::testnext(ls, ',');
						}
						else
							return false;
					}
					else if (c == '[')
					{
						// by integer or string key assignment, such as [number|string] = data|table
						NPLParser::next(ls);
						if (ls->t.token == NPLLex::TK_NUMBER)
						{
							// verify that it is an integer, instead of a floating value.
							NPLObjectProxy& proxy_ = tabMsg[(int)(ls->t.seminfo.r)];

							NPLParser::next(ls);
							if (ls->t.token == ']')
							{
								NPLParser::next(ls);
								if (ls->t.token == '=')
								{
									NPLParser::next(ls);
									if (!DeserializePureNPLDataBlock(ls, proxy_))
										return false;
									NPLParser::testnext(ls, ',');
								}
								else
									return false;
							}
						}
						else if (ls->t.token == NPLLex::TK_STRING)
						{
							// verify that the string is a value key(non-empty);
							if (ls->t.seminfo.ts.empty())
								return false;

							NPLObjectProxy& proxy_ = tabMsg[ls->t.seminfo.ts];
							NPLParser::next(ls);
							if (ls->t.token == ']')
							{
								NPLParser::next(ls);
								if (ls->t.token == '=')
								{
									NPLParser::next(ls);
									if (!DeserializePureNPLDataBlock(ls, proxy_))
										return false;
									NPLParser::testnext(ls, ',');
								}
								else
									return false;
							}
						}
						else
							return false;
					}
					/// Fixed: 2008.6.3 LiXizhi
					/// the following is for auto indexed table items {"string1", "string2\r\n", 213, nil,["A"]="B", true, false, {"another table", "field1"}}
					else
					{
						NPLObjectProxy& proxy_ = tabMsg[nTableAutoIndex++];
						if (!DeserializePureNPLDataBlock(ls, proxy_))
							return false;
						NPLParser::testnext(ls, ',');
					}
				}
				return true;
			}
			default:
				break;
			}
			return false;
		}

		/** verify the script code. it returns true if the script code contains pure msg data or table.
		* this function is used to verify scode received from the network. So that the execution of a pure data in the local runtime is harmless.
		* @param sCode: the input string
		* @param nCodeSize: number of bytes in string. if -1, strlen() is used to determine the size.
		*/
		static bool IsSCodePureData(const char * sCode, int nCodeSize = -1)
		{
			if (nCodeSize < 0)
				nCodeSize = strlen(sCode);
			return NPLParser::IsMsgData(sCode, nCodeSize);
		}

		/** it will return true if input string is "false", "true", NUMBER, STRING, and {table}
		* @param sCode: the input string
		* @param nCodeSize: number of bytes in string. if -1, strlen() is used to determine the size.
		*/
		static bool IsPureData(const char * sCode, int nCodeSize = -1){
			if (nCodeSize < 0)
				nCodeSize = strlen(sCode);
			return NPLParser::IsPureData(sCode, nCodeSize);
		}

		/** it will return true if input string is a {table} containing only "false", "true", NUMBER, STRING, and other such {table}
		* @param sCode: the input string
		* @param nCodeSize: number of bytes in string. if -1, strlen() is used to determine the size.
		*/
		static bool IsPureTable(const char * sCode, int nCodeSize = -1){
			if (nCodeSize < 0)
				nCodeSize = strlen(sCode);
			return NPLParser::IsPureTable(sCode, nCodeSize);
		}

		/** converting string to NPL table object
		* @param input: such as "{nid=10, name=\"value\", tab={name1=\"value1\"}}"
		*/
		static NPLObjectProxy StringToNPLTable(const char* input, int nLen = -1){
			NPLLex lex;
			LexState* ls = lex.SetInput(input, nLen);
			ls->nestlevel = 0;

			try
			{
				NPLParser::next(ls);  /* read first token */

				if (ls->t.token == '{')
				{
					NPLObjectProxy output;
					if (DeserializePureNPLDataBlock(ls, output))
					{
						NPLParser::testnext(ls, ';');
						if (ls->t.token == NPLLex::TK_EOS)
						{
							return output;
						}
					}
				}
			}
			catch (const char* err)
			{
				OUTPUT_LOG("error: %s in NPLHelper::StringToNPLTable()\n", err);
				return NPLObjectProxy();
			}
			catch (...)
			{
				OUTPUT_LOG("error: unknown error in NPLHelper::StringToNPLTable()\n");
				return NPLObjectProxy();
			}
			return NPLObjectProxy();
		}

		/** same as StringToNPLTable(), except that it begins with "msg={...}"
		* @param input: such as "msg={nid=10, name=\"value\", tab={name1=\"value1\"}}"
		*/
		static NPLObjectProxy MsgStringToNPLTable(const char* input, int nLen = -1){
			NPLLex lex;
			LexState* ls = lex.SetInput(input, nLen);
			ls->nestlevel = 0;

			try
			{
				NPLParser::next(ls);  /* read first token */

				if (ls->t.token == NPLLex::TK_NAME && ls->t.seminfo.ts == "msg")
				{
					NPLParser::next(ls);
					if (ls->t.token == '=')
					{
						NPLParser::next(ls);
						NPLObjectProxy output;
						if (DeserializePureNPLDataBlock(ls, output))
						{
							NPLParser::testnext(ls, ';');
							if (ls->t.token == NPLLex::TK_EOS)
							{
								return output;
							}
						}
					}
				}
			}
			catch (const char* err)
			{
				OUTPUT_LOG("error: %s in NPLHelper::StringToNPLTable()\n", err);
				return NPLObjectProxy();
			}
			catch (...)
			{
				OUTPUT_LOG("error: unknown error in NPLHelper::StringToNPLTable()\n");
				return NPLObjectProxy();
			}
			return NPLObjectProxy();
		}

		template <typename StringType>
		static bool SerializeNPLTableToString(const char* sStorageVar, NPLObjectProxy& input, StringType& sCode, int nCodeOffset)
		{
			sCode.resize(nCodeOffset);

			int nStorageVarLen = 0;
			if (sStorageVar != NULL)
			{
				nStorageVarLen = strlen(sStorageVar);
				if (nStorageVarLen > 0)
				{
					sCode.append(sStorageVar, nStorageVarLen);
					sCode.append("=");
				}
			}

			NPLObjectBase::NPLObjectType nType = input.GetType();
			switch (nType)
			{
			case NPLObjectBase::NPLObjectType_Number:
			{
				double value = input;
				char buff[40];
				int nLen = snprintf(buff, 40, LUA_NUMBER_FMT, value);
				sCode.append(buff, nLen);
				break;
			}
			case NPLObjectBase::NPLObjectType_Bool:
			{
				bool bValue = input;
				sCode.append(bValue ? "true" : "false");
				break;
			}
			case NPLObjectBase::NPLObjectType_String:
			{
				// this is something like string.format("%q") in NPL.
				const string& str = input;
				EncodeStringInQuotation(sCode, (int)(sCode.size()), str.c_str(), (int)(str.size()));
				break;
			}
			case NPLObjectBase::NPLObjectType_Table:
			{
				sCode.append("{");
				int nNextIndex = 1;
				// serialize item with number key
				for (NPLTable::IndexIterator_Type itCur = input.index_begin(), itEnd = input.index_end(); itCur != itEnd; ++itCur)
				{
					int nIndex = itCur->first;
					NPLObjectProxy& value = itCur->second;
					int nOldSize = (int)(sCode.size());
					if (nIndex != nNextIndex)
					{
						sCode.append("[");
						char buff[40];
						int nLen = snprintf(buff, 40, "%d", nIndex);
						sCode.append(buff, nLen);
						sCode.append("]=");
					}
					if (SerializeNPLTableToString(NULL, value, sCode, (int)(sCode.size())))
						sCode.append(",");
					else
						sCode.resize(nOldSize);
					nNextIndex = nIndex + 1;
				}
				// serialize item with a string key
				for (NPLTable::Iterator_Type itCur = input.begin(), itEnd = input.end(); itCur != itEnd; ++itCur)
				{
					const string& key = itCur->first;
					const char* sKey = key.c_str();
					NPLObjectProxy& value = itCur->second;
					int nOldSize = (int)(sCode.size());
					// if sKey contains only alphabetic letters, we will use sKey=data,otherwise, we go the safer one ["sKey"]=data.
					// the first is more efficient in disk space. 
					int nSKeyCount = (int)(key.size());
					bool bIsIdentifier = NPLParser::IsIdentifier(sKey, nSKeyCount);
					if (bIsIdentifier)
					{
						sCode.append(sKey, nSKeyCount);
						sCode.append("=");
					}
					else
					{
						sCode.append("[");
						EncodeStringInQuotation(sCode, (int)(sCode.size()), sKey, nSKeyCount);
						sCode.append("]=");
					}
					if (SerializeNPLTableToString(NULL, value, sCode, (int)(sCode.size())))
					{
						sCode.append(",");
					}
					else
					{
						sCode.resize(nOldSize);
					}
				}
				sCode.append("}");
				break;
			}
			default:
				// we will escape any functions or nil, etc. 
				if (nStorageVarLen > 0)
				{
					sCode.resize(nCodeOffset);
				}
				return false;
				break;
			}
			return true;
		}

		// Thread-safe version
		template <typename StringType>
		static void EncodeStringInQuotation(StringType& buff, int nOutputOffset, const char* str, int nSize)
		{
			// this is something like string.format("%q") in NPL.
			// estimate the size. 
			if (nSize < 0)
				nSize = strlen(str);
			int nFinalSize = nOutputOffset + nSize + 2;
			buff.resize(nFinalSize);

			// replace quotation mark in string. 
			int nPos = nOutputOffset;
			buff[nPos++] = '"';
			for (int i = 0; i < nSize; ++i)
			{
				char c = str[i];
				switch (c)
				{
				case '"': case '\\': {
					nFinalSize += 1;
					buff.resize(nFinalSize);

					buff[nPos++] = '\\';
					buff[nPos++] = c;

					break;
				}
				case '\n': {
					nFinalSize += 1;
					buff.resize(nFinalSize);

					buff[nPos++] = '\\';
					buff[nPos++] = 'n';
					break;
				}
				case '\r': {
					nFinalSize += 1;
					buff.resize(nFinalSize);

					buff[nPos++] = '\\';
					buff[nPos++] = 'r';
					break;
				}
				case '\0': {
					nFinalSize += 3;
					buff.resize(nFinalSize);

					buff[nPos++] = '\\';
					buff[nPos++] = '0';
					buff[nPos++] = '0';
					buff[nPos++] = '0';
					break;
				}
				default: {
					buff[nPos++] = c;
					break;
				}
				}
			}
			buff[nPos++] = '"';
			assert(nPos == nFinalSize && (int)(buff.size()) == nFinalSize);
		}

		/** same as SerializeNPLTableToString. Except that it is used via DLL interface. */
		static bool NPLTableToString(const char* sStorageVar, NPLObjectProxy& input, std::string& sCode, int nCodeOffset = 0){
			return SerializeNPLTableToString(sStorageVar, input, sCode, nCodeOffset);
		}

		template <typename StringType>
		static void EncodeStringInQuotation(StringType& output, int nOutputOffset, const std::string& input)
		{
			EncodeStringInQuotation(output, nOutputOffset, input.c_str(), (int)input.size());
		}
		template <typename StringType>
		static void EncodeStringInQuotation(StringType& output, int nOutputOffset, const char* input)
		{
			EncodeStringInQuotation(output, nOutputOffset, input, strlen(input));
		}
	};

	// Note: Instantiate function with the explicitly specified template. 
	// This allows us to put template implementation code in cpp file. 
	template void NPLHelper::EncodeStringInQuotation(std::string& output, int nOutputOffset, const char* input, int nInputSize);
	template bool NPLHelper::SerializeNPLTableToString(const char* sStorageVar, NPLObjectProxy& input, std::string& sCode, int nCodeOffset);

#pragma endregion NPLHelper

#pragma region NPLWriter
	template <typename StringBufferType = std::string>
	class CNPLWriterT;

	/** using std::string for buffering */
	typedef CNPLWriterT< std::string >  CNPLWriter;

	/** a simple class for creating NPL script code, especially data table code.
	this class is reentrant (thread-safe). Please note that this class does not ensure that the code is a pure table.
	See Example:
	// to generate the string : msg={name=1,2,{"3"="4",},};
	CNPLWriter writer; // if you can estimate the buffer size, use CNPLWriter writer(nReservedSize)
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("name");
	writer.WriteValue(1);
	writer.WriteValue(2);
	writer.BeginTable();
	writer.WriteName("3", true);
	writer.WriteValue("4");
	writer.EndTable();
	writer.EndTable();
	writer.WriteParamDelimiter();
	log(writer.ToString().c_str());

	One can also provide their own string buffer to write to, like below.
	std::string buff;
	buff.reserve(100);
	CNPLWriter writer(buff);
	*/
	template <typename StringBufferType>
	class CNPLWriterT
	{
	public:
		typedef StringBufferType  Buffer_Type;

		/** the internal buffer reserved size. */
		CNPLWriterT(int nReservedSize = -1);
		/** into which buff_ to write the */
		CNPLWriterT(Buffer_Type& buff_, int nReservedSize = -1);
		~CNPLWriterT();

		/** reset the writer */
		void Reset(int nReservedSize = -1);

		/** write begin table "{" */
		void BeginTable();
		/** write a parameter name
		@param bUseBrackets: if false, one has to make sure that the name is a valid NPL name string, without special characters. */
		void WriteName(const char* name, bool bUseBrackets = false);

		/** if bInQuotation is true, it writes a parameter text value. Otherwise it will just append the value without encoding it with quatation marks. */
		void WriteValue(const char* value, bool bInQuotation = true);
		/** if bInQuotation is true, it writes a parameter text value. Otherwise it will just append the value without encoding it with quatation marks. */
		void WriteValue(const char* buffer, int nSize, bool bInQuotation = true);
		void WriteValue(const string& sStr, bool bInQuotation = true);
		/** write a parameter value */
		void WriteValue(double value);
		/** write a parameter nil*/
		void WriteNil();

		/** write end table "}" */
		void EndTable();

		/** append any text */
		void Append(const char* text);
		void Append(const char* pData, int nSize);
		void Append(const string& sText);
		/** add a mem block of size nSize and return the address of the block.
		* we may fill the block at a latter time.
		* @param nSize: size in bytes.
		*/
		char* AddMemBlock(int nSize);

		/** write ";" */
		void WriteParamDelimiter(){ Append(";"); };

		/** get the current NPL code. */
		const Buffer_Type& ToString() { return m_sCode; };
	public:
		// static strings. 

#ifndef DISABLE_STATIC_VARIABLE
		/** return "msg=nil;"*/
		static const Buffer_Type& GetNilMessage();

		/** return "msg={};"*/
		static const Buffer_Type& GetEmptyMessage();
#endif

	private:
		Buffer_Type& m_sCode;
		Buffer_Type m_buf;
		bool m_bBeginAssignment;
		int m_nTableLevel;
	};

	template <typename StringBufferType>
	CNPLWriterT<StringBufferType>::CNPLWriterT(int nReservedSize)
		: m_sCode(m_buf), m_bBeginAssignment(false), m_nTableLevel(0)
	{
		if (nReservedSize > 0)
			m_sCode.reserve(nReservedSize);
	}

	template <typename StringBufferType>
	CNPLWriterT<StringBufferType>::CNPLWriterT(StringBufferType& buff_, int nReservedSize)
		: m_sCode(buff_), m_bBeginAssignment(false), m_nTableLevel(0)
	{
		if (nReservedSize > 0)
			m_sCode.reserve(nReservedSize);
	}

	template <typename StringBufferType>
	CNPLWriterT<StringBufferType>::~CNPLWriterT()
	{
	}


	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::Reset(int nReservedSize /*= -1*/)
	{
		m_sCode.clear();
		if (nReservedSize > 0)
			m_sCode.reserve(nReservedSize);
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::WriteName(const char* name, bool bUseBrackets /*= false*/)
	{
		if (name)
		{
			m_bBeginAssignment = true;
			if (!bUseBrackets)
			{
				m_sCode += name;
			}
			else
			{
				m_sCode += "[\"";
				m_sCode += name;
				m_sCode += "\"]";
			}
		}
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::WriteValue(const char* value, bool bInQuotation/*=true*/)
	{
		if (value == NULL)
		{
			return WriteNil();
		}
		if (m_bBeginAssignment)
		{
			m_sCode += "=";
		}
		if (bInQuotation)
		{
			NPLHelper::EncodeStringInQuotation(m_sCode, (int)m_sCode.size(), value);
		}
		else
		{
			m_sCode += value;
		}
		if (m_nTableLevel > 0)
			m_sCode += ",";
		m_bBeginAssignment = false;
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::WriteValue(const char* buffer, int nSize, bool bInQuotation/*=true*/)
	{
		if (buffer == NULL)
		{
			return WriteNil();
		}
		if (m_bBeginAssignment)
		{
			m_sCode += "=";
		}
		if (bInQuotation)
		{
			NPLHelper::EncodeStringInQuotation(m_sCode, (int)m_sCode.size(), buffer, nSize);
		}
		else
		{
			size_t nOldSize = m_sCode.size();
			m_sCode.resize(nOldSize + nSize);
			memcpy((void*)(m_sCode.c_str() + nOldSize), buffer, nSize);
		}
		if (m_nTableLevel > 0)
			m_sCode += ",";
		m_bBeginAssignment = false;
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::WriteValue(double value)
	{
		char buff[40];
		snprintf(buff, 40, LUA_NUMBER_FMT, value);
		WriteValue(buff, false);
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::WriteValue(const string& sStr, bool bInQuotation/*=true*/)
	{
		WriteValue(sStr.c_str(), (int)sStr.size(), bInQuotation);
	}
	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::WriteNil()
	{
		WriteValue("nil", false);
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::BeginTable()
	{
		m_sCode += m_bBeginAssignment ? "={" : "{";
		m_nTableLevel++;
		m_bBeginAssignment = false;
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::EndTable()
	{
		m_sCode += "}";
		if ((--m_nTableLevel) > 0)
			m_sCode += ",";
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::Append(const char* text)
	{
		if (text)
			m_sCode += text;
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::Append(const char* pData, int nSize)
	{
		m_sCode.append(pData, nSize);
	}

	template <typename StringBufferType>
	void CNPLWriterT<StringBufferType>::Append(const string& text)
	{
		m_sCode += text;
	}

	template <typename StringBufferType>
	char* CNPLWriterT<StringBufferType>::AddMemBlock(int nSize)
	{
		if (nSize > 0)
		{
			m_sCode.resize(m_sCode.size() + nSize);
			return &(m_sCode[m_sCode.size() - nSize]);
		}
		else
			return NULL;
	}

#ifndef DISABLE_STATIC_VARIABLE
	template <typename StringBufferType>
	const StringBufferType& CNPLWriterT<StringBufferType>::GetNilMessage()
	{
		static const StringBufferType g_str = "msg=nil;";
		return g_str;
	}

	template <typename StringBufferType>
	const StringBufferType& CNPLWriterT<StringBufferType>::GetEmptyMessage()
	{
		static const StringBufferType g_str = "msg={};";
		return g_str;
	}
#endif

	// instantiate class so that no link errors when separating template implementation to hpp file. 
	template class CNPLWriterT < std::string > ;
#pragma endregion NPLWriter
}
