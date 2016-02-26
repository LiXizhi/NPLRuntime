#pragma once
#ifdef USE_NPLCompiler
namespace NPL
{
	/** parse command line string to argc and argv as the main() entry file. 
	*/
	class CommandLineParams
	{
	public:
		CommandLineParams(const char* commandLine, int nMaxCapcity=20, int nMaxParamSize=1024);
		CommandLineParams(int nMaxCapcity=20, int nMaxParamSize=1024);
		~CommandLineParams();

		/** re parse command line */
		int Reset(const char* commandLine);
		/** get count argc*/
		int GetArgumentCount(){return m_argc;}
		/** get argv*/
		char ** GetArgumentV(){return m_argv;}
		/** get argument at given index, return NULL if there is none. */
		char * GetArgumentAtIndex(int i){return (i>=0 && i<m_argc) ? m_argv[i] : NULL;}

	public:
		/* Note: I took this source from Joe D. 's HPIPack.
		Put the next parameter of command line in parm
		@param cmd : command line to process
		@param parm : will contain new parameter

		@param return : return command line to proceed. Use return value to
		call GetCommandParam in cmd parameter for next 
		command line parameter.
		*/
		static char* GetCommandParam(char* cmd, char* parm);

		/* 
		* "Cut" the command line into the argv table
		* @param cmdLine: full command line
		* @param argv a two dimension table (use for exemple a char * argv[1024])
		* @param argc will receive the number of argument
		*/
		static void DoParseCommandLine(const char *cmdLine, char **argv,int*argc);

		/*
		* Initialise memory for argv table
		* @param argv : reference of pointer to two dimension table argv
		*     if argv == NULL, it will be allocated
		*     if != NULL, it's supposed to be already allocated in memory
		* @param max_parameter_number : maximum number of parameter
		* @param max_parameter_size : maximum size of each parameter
		*/
		static void ParseCommandLineInit(char*** argv, int max_parameter_number, int max_parameter_size);

		/* 
		* Free memory for argv table
		* @param argv : pointer to two dimension table argv
		* @param max_parameter_number : maximum number of parameter
		* @param destoy_me : 0 : will not destroy argv (must be a char **)
		*              1 : will destroy argv (if it's a char * [???])
		*/
		static void ParseCommandLineFree(char** argv, int max_parameter_number, int destroy_me);

	private:
		int m_argc;
		int m_nMaxCapcity;
		int m_nMaxParamSize;
		char ** m_argv;
	};

	class CNPLCompiler
	{
	public:
		/**
		* compile by giving a command line. 
		*/
		static int Compile(const char* args);
		static int Compile(int argc, char* argv[]);
	};
}
#endif