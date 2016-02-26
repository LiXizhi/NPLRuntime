#pragma once
#include <string>

namespace ParaEngine
{
	/** get time in nano seconds. this ensures that time is always forward. */
	int64 GetTimeUS( void );
	int64 GetTimeMS( void );	

	/**
	* get the date in string
	* [thread safe]
	* @param sFormat: can be NULL to use default.e.g. "ddd',' MMM dd yy"
	- d Day of month as digits with no leading zero for single-digit days. 
	- dd Day of month as digits with leading zero for single-digit days. 
	- ddd Day of week as a three-letter abbreviation. The function uses the LOCALE_SABBREVDAYNAME value associated with the specified locale. 
	- dddd Day of week as its full name. The function uses the LOCALE_SDAYNAME value associated with the specified locale. 
	- M Month as digits with no leading zero for single-digit months. 
	- MM Month as digits with leading zero for single-digit months. 
	- MMM Month as a three-letter abbreviation. The function uses the LOCALE_SABBREVMONTHNAME value associated with the specified locale. 
	- MMMM Month as its full name. The function uses the LOCALE_SMONTHNAME value associated with the specified locale. 
	- y Year as last two digits, but with no leading zero for years less than 10. 
	- yy Year as last two digits, but with leading zero for years less than 10. 
	- yyyy Year represented by full four digits. 
	- gg Period/era string. The function uses the CAL_SERASTRING value associated with the specified locale. This element is ignored if the date to be formatted does not have an associated era or period string. 
	*/
	std::string GetDateFormat(const char*  sFormat);

	/**
	* get the time in string
	* [thread safe]
	* @param sFormat: can be NULL to use default.
	* e.g. "hh':'mm':'ss tt"
	- h Hours with no leading zero for single-digit hours; 12-hour clock. 
	- hh Hours with leading zero for single-digit hours; 12-hour clock. 
	- H Hours with no leading zero for single-digit hours; 24-hour clock. 
	- HH Hours with leading zero for single-digit hours; 24-hour clock. 
	- m Minutes with no leading zero for single-digit minutes. 
	- mm Minutes with leading zero for single-digit minutes. 
	- s Seconds with no leading zero for single-digit seconds. 
	- ss Seconds with leading zero for single-digit seconds. 
	- t One character time-marker string, such as A or P. 
	- tt Multicharacter time-marker string, such as AM or PM. 
	*/
	std::string GetTimeFormat(const char* sFormat);

	/** generate a unique id using current universal time.
	* [thread safe]
	*/
	std::string GenerateUniqueIDByTime();

	/**
	* Get the system date and time in seconds. The system time is expressed in Coordinated Universal Time (UTC). 
	* Note: there is some trick to make the returned value a valid number in NPL. Only compare time with time returned by the same function. 
	* TODO: in the long run, a true unsigned int64 should be returned. 
	* [thread safe]
	*/
	double GetSysDateTime(); 
	
	/* this function can be used to print code execution time. */
	class CParaTimeInterval
	{
	public:
		CParaTimeInterval(bool bStartNow=true);
		void Begin();
		/** output to log */
		void Print(const char* sMsg);
		/** get current interval in uSeconds. */
		int32 GetIntervalUS();
	public:
		int64 m_nFromTime;

	};
}