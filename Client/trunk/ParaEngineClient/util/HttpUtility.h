#pragma once
#include <string>
#include <vector>
using namespace std;

namespace ParaEngine
{
	/** build a url string. It keeps the name value paire structure internally. */
	class CUrlBuilder
	{
	public:
		CUrlBuilder():m_bNeedRebuild(false){};
		
		/** get the final string. this can be called multiple times efficiently. it caches the result internally. 
		It ensures that name and value are converted using URLEncode
		*/
		const string& ToString();

		/** the url without parameter part */
		void SetBaseURL(const string & baseurl);

		/** the url without parameter part */
		const string& GetBaseUrl(){return m_baseurl;}

		/** append a param name and value pair */
		void AppendParam(const string& name, const string& value);

		/** insert a given url at the given index. it will replace the one if there already one at the index. 
		* one can insert at nIndex in any order, and the function ensures that the output params are ordered from low to high. 
		* @param name: if NULL, it will not update the name portion. 
		* @param value: if NULL, it will not update the name portion. 
		*/
		void InsertParam(int nIndex, const char* name, const char* value);

		/** get param by it name. return NULL if not found. */
		const char* GetParam(const string& name);

		/** return true if it has at least one parameter. */
		bool HasParams();
	private:
		/** rebuild m_url according to base url and url params */
		void RebuildUrl();
	private:
		/** name value pairs.*/
		class CNameValuePair
		{
		public:
			CNameValuePair(const string& name, const string& value):m_name(name), m_value(value){};
			CNameValuePair(){};

			string m_name;
			string m_value;
		};

		/// the final version of url
		string m_url;
		/// the url without parameter part. 
		string m_baseurl;
		/// whether the m_url needs rebuild. 
		bool m_bNeedRebuild;
		vector<CNameValuePair> m_params;



	};

	/**
	* http utility
	*/
	class CHttpUtility
	{
	public:
		static std::string UrlDecode(const std::string & encoded);
		static std::string UrlEncode(const std::string & decoded);
		/** encode a byte */
		static std::string UrlEncode(const char* sz_decoded, int nSize);

		/** get the 16 bytes (128 bits) md5 of a src string. we usually use it to hash downloaded file to save to disk cache
		* Note: there is a still a low possibility of two different src generating the same hash. However, I just ignored it. 
		* @param src: the src string. 
		* @return: 16 bytes
		*/
		static string HashStringMD5(const char* input);

		/** same as above. */
		static void HashStringMD5(std::string& out, const char* input);
	};
}