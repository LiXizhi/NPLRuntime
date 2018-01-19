#pragma once

namespace ParaEngine
{
	/**
	* command line parameters. 
	*/
	class CCommandLineParams
	{
	public:
		CCommandLineParams();
		CCommandLineParams(const char* lpCmdLine);

		/** get string specifying the command line for the application, excluding the program name.*/
		const char* GetAppCommandLine();

		/**
		* return a specified parameter value in the command line of the application. If the parameter does not exist, the return value is NULL. 
		* @param pParam: key to get
		* @param defaultValue: if the key does not exist, this value will be added and returned. This can be NULL.
		*/
		const char* GetAppCommandLineByParam(const char* pParam, const char* defaultValue);

		/** set string specifying the command line for the application, excluding the program name.
		* calling this function multiple times with different command line is permitted. The latter settings will merge and override the previous ones.
		* such as key="value" key2="value2"
		*/
		void SetAppCommandLine(const char* pCommandLine);

	private:
		/** string specifying the command line for the application, excluding the program name.
		* such as XX="YY" aa="pp"
		*/
		string m_sAppCmdLine;

		/** parameters from m_sAppCmdLine */
		std::map<string, string> m_commandParams;
	};
	
}
