#pragma once

#include "sqlite3.h"
#define GEN_MAPTRACE_SIZE 5
namespace ParaInfoCenter
{
	enum IC_GEN_TYPE{
		BASE=0, MAPTRACE=1, TRACEDATA=2
	};
	class CICRecordSet;
	class CICGen {
	public:
		int m_type; //type of the GEN
		int m_size; //data size of the GEN
		void *m_data; //data of the GEN
		
		CICGen(){m_data=NULL;m_type=BASE;}
		virtual void FromDB(CICRecordSet *rs){}; //get the GEN data from a sql result
		virtual void ToDB(){};//insert the GEN data to the db
	};




}
