//-----------------------------------------------------------------------------
// Class:	NPL Compiler
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.10.10
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_NPLCompiler
#include "NPLCompiler.h"

extern "C"
{
#define luac_c
#define LUA_CORE

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "npl_compiler.h"
}

#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif

using namespace NPL;

#ifdef WIN32
int CompilerDump()
{
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int CNPLCompiler::Compile(int argc, char* argv[])
{
	HRESULT hr = S_OK;
#ifdef WIN32
	__try
#else
	try
#endif
	{
		hr = npl_compile(argc, argv);
	}
#ifdef WIN32
	__except(CompilerDump())
#else
	catch(...)
#endif
	{
		hr = E_FAIL;
	}
	return hr;
}

int CNPLCompiler::Compile(const char* args)
{
	CommandLineParams cmdLine(args);
	return Compile(cmdLine.GetArgumentCount(), cmdLine.GetArgumentV());
}

//////////////////////////////////////////////////////////////////////////
//
// CommandLineParams
//
//////////////////////////////////////////////////////////////////////////
void NPL::CommandLineParams::DoParseCommandLine(const char *cmdLine, char **argv,int*argc)
{
	char *buffer[256], *ptr; //256 parameters limitation!!!
	int quote, ch;

	char *bufcmdline= (char*) calloc (strlen(cmdLine)+1, sizeof(char));
	char * pt_bufcmdline = bufcmdline;
	buffer[0]=bufcmdline;
	strcpy(bufcmdline,cmdLine);
	int i=0;
	for(quote = 0, (buffer)[0] = ptr = (char*)bufcmdline;;) 
	{
		if((ch = *bufcmdline++) == '"') 
		{
			quote ^= 1;
		} 
		else if(ch && (quote || (ch != ' ' && ch != '\t')))
		{
			*ptr++ = (char) ch;
		}
		else 
		{
			*ptr++ = '\0';
			buffer[++i] = ptr;

			if(ch == '\0') 
			{
				break;
			}
		}
	}
	int nCount = 0;
	for (int j=0;j<i;++j)
	{
		// fixed:skip empty params
		if(buffer[j][0] != '\0')
		{
			strcpy((argv)[nCount],buffer[j]);
			++nCount;
		}
	}
	(*argc) = nCount;

	free(pt_bufcmdline);
}


char* NPL::CommandLineParams::GetCommandParam( char* cmd, char* parm )
{
	*parm = 0;

	while (isspace(*cmd))
		cmd++;

	if (!*cmd)
		return cmd;

	if (*cmd == '"') 
	{
		cmd++;
		while (*cmd && (*cmd != '"')) 
		{
			if (*cmd == '\\') 
			{
				if ((cmd[1] == '"') || (cmd[1] == '\\'))
					cmd++;
				*parm++ = *cmd++;
			}
			else
				*parm++ = *cmd++;
		}
		while (*cmd == '"')
			cmd++;
	}
	else 
	{
		while (*cmd && !isspace(*cmd))
			*parm++ = *cmd++;
	}
	*parm = 0;

	while (isspace(*cmd))
		cmd++;

	return cmd;
}

void NPL::CommandLineParams::ParseCommandLineInit( char*** argv, int max_parameter_number, int max_parameter_size )
{
	if (!(*argv))
		*argv=(char**) calloc(max_parameter_number, sizeof(char*) );
	for ( int i=0; i<max_parameter_number; ++i) 
		(*argv)[i]= (char*)calloc(max_parameter_size,sizeof(char));
}

void NPL::CommandLineParams::ParseCommandLineFree( char** argv, int max_parameter_number, int destroy_me )
{
	for ( int i=0; i<max_parameter_number; ++i) 
		free(argv[i]);
	if (destroy_me)
		free(argv);
}

NPL::CommandLineParams::CommandLineParams( const char* commandLine, int nMaxCapcity, int nMaxParamSize )
:m_nMaxCapcity(nMaxCapcity), m_nMaxParamSize(nMaxParamSize ), m_argc(0), m_argv(0)
{
	ParseCommandLineInit(&m_argv, m_nMaxCapcity, m_nMaxParamSize);
	Reset(commandLine);
}

NPL::CommandLineParams::CommandLineParams( int nMaxCapcity/*=20*/, int nMaxParamSize/*=1024*/ )
:m_nMaxCapcity(nMaxCapcity), m_nMaxParamSize(nMaxParamSize ), m_argc(0), m_argv(0)
{
	ParseCommandLineInit(&m_argv, m_nMaxCapcity, m_nMaxParamSize);
}

NPL::CommandLineParams::~CommandLineParams()
{
	ParseCommandLineFree(m_argv, m_nMaxCapcity, 1);
}

int NPL::CommandLineParams::Reset( const char* commandLine )
{
	DoParseCommandLine(commandLine, m_argv, &m_argc);
	return S_OK;
}
#endif