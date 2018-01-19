#pragma once

#include <list>
#include <string>
#include <map>
using namespace std;

namespace ParaEngine
{
/**
* this is a default report class. 
* for simplicity, it uses a list to store items. So do not
* save too many items inside a single report instance.
*/
class CReport
{
public:
	CReport(void);
	~CReport(void);

private:

	struct ReportItem
	{
		enum ItemType{
			ReportString,
			ReportValue
		} itemtype;
		string str;
		double dValue;
		ReportItem(const char* str){
			this->str = str;
			itemtype = ReportString;
		}
		ReportItem(double v){
			dValue = v;
			itemtype = ReportValue;
		}
		string GetString();
		
	};
	// report items
	map <string, ReportItem> m_items;

public:
	/** 
	* get the value of a named item. If the item does not exist, 0 is returned.
	*/
	double GetValue (const char * strItemName);
	/** 
	* set the value of a named item. If the item does not exist, it shall be created and then saved.
	*/
	void SetValue (const char * strItemName, double dValue);
	/** 
	* set the string of a named item. If the item does not exist, it shall be created and then saved.
	*/
	void SetString (const char * strItemName, const char * str);

	/*
	* create a new item using a user defined format.
	* the format will be passed to printf(report, format, dValue) to generate report string.
	* @params format: the format string such as "FPS is %d".
	*/
	//void CreateItem(const char * strItemName, ReportItem& item);

	/** 
	* get report string for a specified item.
	* if nothing is found, return ""
	* @params: [out] pReport contains the report
	*/
	void  GetItemReport (const char * strItemName, char* pReport);

	/** 
	* get report string for all items.
	* @params: [out] pReport contains the report
	*/
	void GetAllReport (string & sReport);
};
}