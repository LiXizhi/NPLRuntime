//----------------------------------------------------------------------
// Class:	CICMapTrace
// Authors:	Liu Weili
// Date:	2005.11.13
//
// desc: 
// Basic element of Information Center
//
//
//-----------------------------------------------------------------------
#ifdef USE_ICMAPTRACE
#include "ParaEngine.h"
#include "ICMapTrace.h"
#include "ICGenManager.h"
#include "ICGen.h"
#include "ICDBManager.h"
#include "ICRecordSet.h"

#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif

using namespace ParaInfoCenter;
//////////////////////////////////////////////////////////////////////////
// CICGenMapTrace
//////////////////////////////////////////////////////////////////////////
CICGenMapTrace::CICGenMapTrace()
:CICGen()
{
	m_type = MAPTRACE;
}

void CICGenMapTrace::GetBlock(int &x, int &y, int &z)
{
	x = (int)m_x / GEN_MAPTRACE_SIZE;
	y = (int)m_y / GEN_MAPTRACE_SIZE;
	z = (int)m_z / GEN_MAPTRACE_SIZE;
}
void CICGenMapTrace::FromDB(CICRecordSet *rs)
{
	if (rs->ColumnCount() != 5 || rs->IsEOF()) {
		OUTPUT_LOG("\nData of the map trace info is wrong.");
		return;
	}
	m_time = (*rs)[0];
	m_x = (*rs)[1];
	m_y = (*rs)[2];
	m_z = (*rs)[3];
	WCHAR* data = ((*rs)[4]);
	m_id = data;
}

void CICGenMapTrace::ToDB()
{
	try{

		DBEntity *dbManager = CICGenManager::Instance()->GetDB();
		CICRecordSet *rs = dbManager->CreateRecordSet(L"INSERT INTO memDB.maptrace(time,x,y,z,id) VALUES(%d,%f,%f,%f,?)", m_time, m_x, m_y, m_z);
		rs->DataBinding(1, m_id.c_str());
		rs->Execute();

	}
	catch (CICSQLException &err) {
		OUTPUT_LOG("%s", err.errmsg.c_str());
	}
}
//////////////////////////////////////////////////////////////////////////
// CICMapTrace
//////////////////////////////////////////////////////////////////////////

CICMapTrace::CICMapTrace()
{
	m_lasttrace = NULL;
	m_nCursor = -1;
	m_recordSet = NULL;
}
CICMapTrace::~CICMapTrace()
{
	if (m_recordSet) {
		EndGettingTrace();
	}
	SAFE_DELETE(m_lasttrace);
}
void CICMapTrace::AddTrace(double x, double y, double z, const WCHAR* id)
{
	//TODO: should change
	CICGenMapTrace *newtrace = new CICGenMapTrace();
	newtrace->m_x = x; newtrace->m_y = y; newtrace->m_z = z;
	newtrace->m_id = id; newtrace->m_time = GetTickCount();
	int x1 = 0, x2 = 0, y1 = 0, y2 = 0, z1 = 0, z2 = 0;
	if (m_lasttrace != NULL) {
		m_lasttrace->GetBlock(x1, y1, z1);
		newtrace->GetBlock(x2, y2, z2);

	}
	if (x1 != x2 || y1 != y2 || z1 != z2 || !m_lasttrace) {
		//add the new trace to db
		newtrace->ToDB();
	}

	SAFE_DELETE(m_lasttrace);
	m_lasttrace = newtrace;
}
void CICMapTrace::DelTrace(DWORD nTime)
{
	//TODO: should change
	DWORD beforeTime = GetTickCount() - nTime;
	CICGenManager *manager = CICGenManager::Instance();
	manager->GetDB()->prepare_sql(L"DELETE FROM memDB.maptrace WHERE time<%d", beforeTime);
	manager->GetDB()->exec_sql16();

}
void CICMapTrace::DisplayTrace()
{

}
void CICMapTrace::BeginGettingTrace(bool bBegin/* =false */)
{
	DBEntity *manager = CICDBManager::GetDB(L"IC");
	if (manager == NULL) {
		OUTPUT_DEBUG("IC database not open");
		return;
	}
	wstring sqlstr = L"SELECT * FROM memDB.maptrace ORDER BY time ";
	if (bBegin) {
		m_bASC = true;
		sqlstr += L"ASC";
	}
	else{
		sqlstr += L"DESC";
		m_bASC = false;
	}
	sqlstr += L" LIMIT 1000;";
	if (m_recordSet) {
		EndGettingTrace();
	}
	m_recordSet = manager->CreateRecordSet(sqlstr.c_str());
	if (!m_recordSet) {
		OUTPUT_DEBUG("Creating recordset failed");
		return;
	}
	int errcode = m_recordSet->NextRow();
	if (errcode != 0) {
		return;
	}
	m_nCursor = 0;
	m_nLowerWindow = 0;
}
void CICMapTrace::EndGettingTrace()
{
	DBEntity *manager = CICDBManager::GetDB(L"IC");
	if (manager == NULL) {
		//		OUTPUT_DEBUG("IC database not open");
		return;
	}
	if (m_recordSet) {
		manager->DeleteRecordSet(m_recordSet);
		m_recordSet = NULL;
	}
	m_nCursor = -1;
}

int CICMapTrace::GetAllTrace(CICGenMapTrace *returndata, bool bDesc/* =true */, int nLimit/* =0 */)
{
	DBEntity *manager = CICDBManager::GetDB(L"IC");
	if (manager == NULL) {
		OUTPUT_DEBUG("IC database not open");
		return 0;
	}
	int nCount = 0;
	wstring sqlstr;
	WCHAR *tempwchar;
	CICRecordSet *tempRS;
	if (returndata == NULL) {
		tempRS = manager->CreateRecordSet(L"SELECT count(*) FROM memDB.maptrace");
		if (tempRS->NextRow() == 0) {
			nCount = (*tempRS)[0];
		}

	}
	else{
		CICGenMapTrace* data = returndata;
		sqlstr = L"SELECT * FROM memDB.maptrace ORDER BY time ";
		if (bDesc) {
			sqlstr += L"DESC";
		}
		else{
			sqlstr += L"ASC";
		}
		if (nLimit > 0) {
			sqlstr += L" LIMIT ";
			WCHAR tempchar[20];
			_itow(nLimit, tempchar, 10);
			tempchar[19] = L'\0';
			sqlstr += tempchar;
		}
		tempRS = manager->CreateRecordSet(sqlstr.c_str());
		while (tempRS->NextRow() == 0) {
			data->m_time = (*tempRS)[0];
			data->m_x = (*tempRS)[1];
			data->m_y = (*tempRS)[2];
			data->m_z = (*tempRS)[3];
			tempwchar = (*tempRS)[4];
			data->m_id = tempwchar;
			data++; nCount++;
		}

	}
	manager->DeleteRecordSet(tempRS);
	return nCount;
}

//we return 1000 records at a time, so the memory will not be consumed
//TODO: but if the game runs too long, the memDB will become very big, so we need to delete the record at that time
bool CICMapTrace::GetTrace(CICGenMapTrace &maptrace)
{
	if (m_nCursor < 0 || !m_recordSet) {
		return false;
	}
	if (m_recordSet->IsEOF()) {
		if (m_nCursor == 0) {
			//no item returns
			return false;
		}
		//otherwise, it may be moving to the next thousand.
		wstring sqlstr = L"SELECT * FROM memDB.maptrace ORDER BY time ";
		if (m_bASC) {
			sqlstr += L"ASC";
		}
		else
			sqlstr += L"DESC";

		sqlstr += L" LIMIT ";
		WCHAR tempchar[20];
		_itow(m_nCursor, tempchar, 10);
		tempchar[19] = L'\0';
		sqlstr += tempchar;
		sqlstr += L",1000";
		m_recordSet->Initialize(sqlstr.c_str());
		if (m_recordSet->NextRow() != 0) {
			//reaches the end
			return false;
		}
		m_nLowerWindow = m_nCursor;
	}
	maptrace.m_time = (*m_recordSet)[0];
	maptrace.m_x = (*m_recordSet)[1];
	maptrace.m_y = (*m_recordSet)[2];
	maptrace.m_z = (*m_recordSet)[3];
	WCHAR *tempwchar = (*m_recordSet)[4];
	maptrace.m_id = tempwchar;
	m_recordSet->NextRow();
	m_nCursor++;
	return true;
}

bool CICMapTrace::SeekTrace(int index)
{
	if (m_nCursor < 0 || !m_recordSet || index < 0) {
		return false;
	}
	int a = 0;
	//the index is in the current window
	if (index >= m_nLowerWindow&&index < m_nLowerWindow + 1000) {
		m_recordSet->Reset();
		for (; a <= index - m_nLowerWindow; a++) {
			if (0 != m_recordSet->NextRow()) {
				return false;
			};
		}
	}
	else{
		int tempwindow = index - index % 1000;
		//the index is in other window
		wstring sqlstr = L"SELECT * FROM memDB.maptrace ORDER BY time ";
		if (m_bASC) {
			sqlstr += L"ASC";
		}
		else
			sqlstr += L"DESC";

		sqlstr += L" LIMIT ";
		WCHAR tempchar[20];
		_itow(tempwindow, tempchar, 10);
		tempchar[19] = L'\0';
		sqlstr += tempchar;
		sqlstr += L",1000";
		m_recordSet->Initialize(sqlstr.c_str());
		for (; a <= index - tempwindow; a++) {
			if (0 != m_recordSet->NextRow()) {
				return false;
			};
		}
		m_nLowerWindow = tempwindow;
	}
	m_nCursor = index;
	return true;
}
#endif