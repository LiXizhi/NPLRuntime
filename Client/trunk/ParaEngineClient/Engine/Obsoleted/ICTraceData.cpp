//----------------------------------------------------------------------
// Class:	CICTraceAnalyze
// Authors:	Liu Weili
// Date:	2005.11.22
//
// desc: 
// Analyze the trace of the players. Forming statical trace information about the routes and
// the player's tendance to use the routes. This process also bases on previous analyze.
// 
//
//-----------------------------------------------------------------------
#ifdef USE_ICGEN_MANAGER
#include "ParaEngine.h"
#include "ICRecordSet.h"
#include "ICDBManager.h"
#include "ICGenManager.h"
#include "ICGen.h"
#include "ICTraceData.h"
#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif
using namespace ParaInfoCenter;
//////////////////////////////////////////////////////////////////////////
// CICGenTraceData
//////////////////////////////////////////////////////////////////////////
CICGenTraceData::CICGenTraceData()
:CICGen()
{
	m_type=TRACEDATA;
}
void CICGenTraceData::SetXY(double x,double y)
{
	m_x=(int)x/GEN_TRACEDATA_SIZE;
	m_y=(int)y/GEN_TRACEDATA_SIZE;
}

void CICGenTraceData::FromDB(CICRecordSet *rs)
{
	if (rs->ColumnCount()!=3||rs->IsEOF()) {
		OUTPUT_LOG("\nData of the trace data is wrong.");
		return;
	}
	m_x=(*rs)[0];
	m_y=(*rs)[1];
	unsigned char* data;
	void *ptemp=(*rs)[2];
	data=(unsigned char*)ptemp;
	m_datasize=(*rs)[2].GetDataLength();
	//if the data is corrupted
	if (m_datasize%sizeof(ICTraceElement)!=0) {
		OUTPUT_LOG("\nData of the trace data is wrong.");
		return;
	}
	memset(m_data,0, 64*sizeof(ICTraceElement));
	DWORD a=0;
	ICTraceElement *temp,*temp1;
	for (a=0;a<m_datasize;a+=sizeof(ICTraceElement)) {
		temp=(ICTraceElement*)(data+a);
		temp1=&m_data[temp->range][temp->direction];
		memcpy(temp1,temp,sizeof(ICTraceElement));
		m_usableRange.set(temp->range);
	}

	delete data;
}

void CICGenTraceData::ToDB()
{
	int range,direction;
	m_datasize=0;
	DWORD tempstoresize=1000;
	unsigned char* data=new unsigned char[tempstoresize];
	try{

		DBEntity *dbManager=CICGenManager::Instance()->GetDB();
		CICRecordSet *rs=dbManager->CreateRecordSet(L"INSERT INTO fileDB.tracedata(x,y,data) VALUES(%f,%f,?)",m_x,m_y);
		for (range=0;range<8;range++) {
			if (m_usableRange[range]) {
				for (direction=0;direction<8;direction++) {
					if (m_data[range][direction].count>0) {
						m_data[range][direction].direction=direction;
						m_data[range][direction].range=range;
						if (m_datasize+sizeof(ICTraceElement)>=tempstoresize) {
							tempstoresize=tempstoresize<<2;
							data=(unsigned char *)realloc(data,tempstoresize);
						}
						memcpy(data+m_datasize,&m_data[range][direction],sizeof(ICTraceElement));
						m_datasize+=sizeof(ICTraceElement);

					}
				}
			}
		}
		rs->DataBinding(1,(void*)data,m_datasize);
		rs->Execute();
		delete data;
	}
	catch (CICSQLException &err) {
		delete data;
		OUTPUT_LOG("%s", err.errmsg.c_str());
	}
}
//////////////////////////////////////////////////////////////////////////
// CICTraceData
//////////////////////////////////////////////////////////////////////////
#endif