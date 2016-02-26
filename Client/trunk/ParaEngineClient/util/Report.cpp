//-----------------------------------------------------------------------------
// Class:	CReport
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Report.h"
using namespace ParaEngine;


string CReport::ReportItem::GetString()
{
	char tmp[50];
	if(itemtype == ReportItem::ReportValue)
	{
		snprintf(tmp, 50, "%.3f", dValue);
		return string(tmp);
	}
	else
		return str;
}

CReport::CReport(void)
{
	m_items.clear();
}

CReport::~CReport(void)
{
	// clear all items
	m_items.clear();
}

double CReport::GetValue (const char * strItemName)
{
	map<string, ReportItem>::iterator iter = m_items.find(string(strItemName));
	if(iter != m_items.end())
	{
		return (*iter).second.dValue;
	}
	return 0;
}

void CReport::SetValue (const char * strItemName, double dValue)
{
	map<string, ReportItem>::iterator iter = m_items.find(string(strItemName));
	if(iter != m_items.end())
	{
		(*iter).second = ReportItem(dValue);
	}
	else
	{
		m_items.insert(pair<string, ReportItem>(string(strItemName), ReportItem(dValue)));
	}
}

void CReport::SetString (const char * strItemName, const char * str)
{
	map<string, ReportItem>::iterator iter = m_items.find(string(strItemName));
	if(iter != m_items.end())
	{
		(*iter).second = ReportItem(str);
	}
	else
	{
		m_items.insert(pair<string, ReportItem>(string(strItemName), ReportItem(str)));
	}
}
void CReport::GetItemReport (const char * strItemName, char* pReport)
{
	map<string, ReportItem>::iterator iter = m_items.find(string(strItemName));
	if(iter != m_items.end())
	{
		sprintf(pReport, "%s=%s", strItemName, (*iter).second.GetString().c_str());
		return;
	}
	pReport[0] = '\0';
}

void CReport::GetAllReport (string& sReport)
{
	sReport.clear();

	map< string, ReportItem>::iterator itCurCP, itEndCP = m_items.end();

	for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		sReport+=(*itCurCP).first;
		sReport+="=";
		sReport+=(*itCurCP).second.GetString();
		sReport+="\n";
	}
}
