#pragma once

#include <string>
#include <sqlite3.h>
#include <map>
#include <bitset>
#include "ICGen.h"
#define GEN_TRACEDATA_SIZE 30//should change to 30000
namespace ParaInfoCenter
{
	/**
	* Direction bits meaning
	* 0 1 2
	* 7 * 3
	* 6 5 4
	* Range bits meaning
	* 2^(range)
	*/
	struct ICTraceElement {
		byte direction:3;
		byte range:3;
		byte undefined:2;
		DWORD count;
	};	
	//the analyzed trace data from database
	class CICGenTraceData:public CICGen
	{
	public:
		CICGenTraceData();
		//here m_x and m_y are in logic units
		DWORD m_datasize;//in bytes
		bitset<8> m_usableRange;//a bitset tells which range is usable. it is usable if the bit is 1
		ICTraceElement m_data[8][8];
		///Get the logic block coordinates of the GEN
		int GetX(){return m_x;}
		int GetY(){return m_y;}
		//changes the absolute coordinates to logic units
		void SetXY(double x,double y);
		virtual void FromDB(CICRecordSet *rs); //get the GEN data from a sql result
		virtual void ToDB();//insert the GEN data to the db

	protected:
		int m_x;
		int m_y;
	};

	//the class stores the gens
	class CICTraceData
	{
	public:
		CICGenTraceData* GetTraceData(double x,double y);
		
	protected:
		multimap<int,CICGenTraceData*> x_index;
		multimap<int,CICGenTraceData*> y_index;
		vector<CICGenTraceData> m_items;
	private:
	};
}