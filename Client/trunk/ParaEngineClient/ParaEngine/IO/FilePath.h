#pragma once
#include <list>
#include <vector>
#include <string>
#include "IAttributeFields.h"

namespace ParaEngine
{
	/** a single path variable */
	class CPathVariable
	{
	public:
		CPathVariable();
		CPathVariable(const std::string& sVarName, const std::string& sVarValue);
		CPathVariable(const std::string& sVarName, const std::string& sVarValue, bool bEditable, bool bEnableEncoding = true);
	public:
		/** variable name */
		std::string m_sVarName;
		/** to which string value this variable is mapped to.  */
		std::string m_sVarValue;
		/** whether this variable can be replaced or overridden from scripting interface. */
		bool m_bEditable;
		/** whether this is used during encoding. see CPathReplaceables::EncodePath() */
		bool m_bEnableEncoding;
	};

	/**
	* replacing some known variables in string path.
	* This is usually used as a singleton.
	* e.g. %ROOT% maps to game engine root dir, and %W% maps to the current world directory, %USERID% maps to user id.
	* for security reasons, most replaceables are not editable or overridable via scripting interface.
	*/
	class CPathReplaceables
	{
	public:
		CPathReplaceables();
		~CPathReplaceables();
		static CPathReplaceables& GetSingleton();
	public:
		/**
		* replace variables in input path and return the result path.
		* @param output: the output string, can be same as input string.
		* @param input: a path with or without replaceable. make sure you have called ToCanonicalFilePath() to canonicalize the input before calling this function
		* @return the resulting path. Please note that the return value is the input itself if nothing is replaced.
		* otherwise, it is a static string reference containing the result. therefore the result is NOT thread safe.
		*/
		void DecodePath(std::string& output, const std::string& input);

		/**
		* this does reverse of DecodePath.
		* @param output: the output string, can be same as input string.
		* @param input: a path with or without replaceable. make sure you have called ToCanonicalFilePath() to canonicalize the input before calling this function
		*/
		void EncodePath(std::string& output, const std::string& input);

		/** same as EncodePath, except that it will only replace variables who name appears in varNames. varNames is a comma separated list of variable names.
		* @param output: the output string, can be same as input string.
		* @param varNames: a comma separated list of variable names. like "WORLD,USERID", etc.
		*/
		void EncodePath(std::string& output, const std::string& input, const std::string& varNames);

		/** add a new variable to the replaceable pool
		* @param variable: the variable to add.
		* @param bCheckCanEdit: default to true. if true, the function will fail if the pool already contain
		* a variable with the same name but it not editable
		* @return: true if succeed.
		*/
		bool AddVariable(const CPathVariable& variable, bool bCheckCanEdit = true);

		/** get an variable object by its name if it exist. it will return NULL if not exist*/
		CPathVariable * GetVariable(const char* sVarName);

		/** return a variable by its name.
		* @param sVarName: variable name to remove.
		* @param bCheckCanEdit: default to true. if true, the function will fail if the pool already contain
		* a variable with the same name but it not editable
		* @return: true if succeed.
		*/
		bool RemoveVariable(const char* sVarName, bool bCheckCanEdit);

		/** clear all variables. */
		bool Clear();

	private:
		std::vector <CPathVariable> m_variables;
	};

	/** a search path. */
	struct SearchPath
	{
	public:
		SearchPath() :m_nPriority(0){};
		SearchPath(const char* searchpath) : m_searchpath(searchpath), m_nPriority(0){};
		SearchPath(const char* searchpath, int nPriority) : m_searchpath(searchpath), m_nPriority(nPriority){};

		bool operator==(const SearchPath& left){ return m_searchpath == left.m_searchpath; }
		bool operator==(const std::string& left){ return m_searchpath == left; }

		std::string& GetPath(){ return m_searchpath; }
	public:
		std::string m_searchpath;
		int m_nPriority;
	};

	/** search path manager interface and implementation. */
	class ISearchPathManager : public IAttributeFields
	{
	protected:
		/** a list of all search paths */
		std::list <SearchPath> m_searchpaths;
	public:
		/** this is low level, use with care. Once added, can not be removed. */
		bool AddDiskSearchPath(const std::string& sFile, bool nFront = false);

		/**
		* add a search path to the search path pool. It will internally normalize the path and check for duplicates
		* @security note: this function shall not be called by an untrusted client, since it will secretly swap files.
		* @TODO: shall we support remote http zip file as a valid search path?
		*/
		PE_CORE_DECL bool AddSearchPath(const char* sFile, int nPriority = 0);
		/**
		* remove a search path from the search path pool.
		*/
		PE_CORE_DECL bool RemoveSearchPath(const char* sFile);
		/**
		* clear all search paths.
		*/
		PE_CORE_DECL bool ClearAllSearchPath();

		/**
		* this function is used mostly internally.
		*/
		std::list <SearchPath>& GetSearchPaths(){ return m_searchpaths; }
	};

}