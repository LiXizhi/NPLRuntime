#pragma once

#include <vector>

namespace NPL
{
	using namespace std;

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
		Zio(const char *input, size_t nSize):p(input), n(nSize){};
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
		static const int TOKEN_LEN	= (sizeof("function")/sizeof(char));
		
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
		static const int NUM_RESERVED = (int)TK_WHILE-FIRST_RESERVED+1;
		
	private:
		LexState m_lexState;
		Zio m_zio;
	public:

		/**
		* set the input stream. and clean up all states
		* @param input code to be parsed
		* @param nLen length of the code
		* @return the state
		*/
		LexState * SetInput(const char* input, int nLen);
		static int luaX_lex (LexState *LS, SemInfo *seminfo);
		LexState * GetState(){return &m_lexState;};

		static const char* FormatString(const char * zFormat,...);
		static void luaX_errorline (LexState *ls, const char *s, const char *token, int line);
		static void luaX_error (LexState *ls, const char *s, const char *token);
		static void luaX_syntaxerror (LexState *ls, const char *msg);
		static void luaX_checklimit (LexState *ls, int val, int limit, const char *msg);
		static void luaX_lexerror (LexState *ls, const char *s, int token);
		static const char *luaX_token2str (LexState *ls, int token);


	private:
		//////////////////////////////////////////////////////////////////////////
		// static lex functions
		//////////////////////////////////////////////////////////////////////////
		static inline int next(LexState *LS)
		{
			return LS->current = ((LS->z)->n--)>0 ?  (int)((unsigned char)(*(LS->z)->p++)) : EOZ;
		}
	
		static void checkbuffer(LexState *LS, int len)
		{
			if (((len)+MAXNOCHECK)*sizeof(char) > LS->buff.size())
			{
				if (len < 500)
					LS->buff.resize(len+EXTRABUFF);
				else
				{
					// added by LiXizhi 2007.6.20: in case the file contains super large string, such as a base64 encoded file of 2MB, we will double the size instead using a fixed length.
					LS->buff.resize(len*2);
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

		static void ThrowError(LexState *ls, const char* errorMsg);

		static void inclinenumber (LexState *LS);
		static void read_long_string (LexState *LS, SemInfo *seminfo);
		static void read_string (LexState *LS, int del, SemInfo *seminfo);
		static int readname (LexState *LS);

		/* LUA_NUMBER */
		static void read_numeral (LexState *LS, int comma, SemInfo *seminfo);

	};

	/** parser for NPL files.
	* Internally it uses NPLLexer
	*/
	class NPLParser
	{
	public:
		NPLParser(void);
		~NPLParser(void);


		/*
		** maximum number of syntactical nested non-terminals: Not too big,
		** or may overflow the C stack...
		*/
		static const int LUA_MAXPARSERLEVEL	= 200;

	public:

		/**
		* whether the NPL data is a msg data or not. msg data is a pure data or data table that is usually received via network.
		* @param input code to be parsed
		* @param nLen length of the code
		* @return 
		*/
		static bool IsMsgData(const char* input, int nLen);

		/** it will return true if input string is "false", "true", NUMBER, STRING, and {table} */
		static bool IsPureData(const char* input, int nLen);

		/** it will return true if input string is a {table} containing only "false", "true", NUMBER, STRING, and other such {table} */
		static bool IsPureTable(const char* input, int nLen);

		/** it consumes, "false", "true", NUMBER, STRING, and {table} */
		static bool CheckPureDataBlock(LexState *ls);

		/** whether the string is an identifier. 
		* Identifiers in npl can be any string of letters, digits, and underscores, not beginning with a digit. This coincides with the definition of identifiers in most languages. (The definition of letter depends on the current locale: any character considered alphabetic by the current locale can be used in an identifier.)
		* @param str: string
		* @param nLength: it must be a valid length.
		*/
		static bool IsIdentifier(const char* str, int nLength);
	
		static void enterlevel(LexState *ls)
		{
			if (++(ls)->nestlevel > LUA_MAXPARSERLEVEL) 
				NPLLex::luaX_syntaxerror(ls, "too many syntax levels");
		}

		static void leavelevel(LexState *ls)
		{
			((ls)->nestlevel--);
		}

		static void next (LexState *ls);
		static void lookahead (LexState *ls);
		static void error_expected (LexState *ls, int token);
		static int testnext (LexState *ls, int c);
		static void check (LexState *ls, int c);
		static void check_condition(LexState *ls,void* c,const char* msg);
		static void check_match (LexState *ls, int what, int who, int where);
	};
}
