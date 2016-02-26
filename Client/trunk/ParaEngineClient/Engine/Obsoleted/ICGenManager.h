#pragma once


#define MAX_GEN_TRACEDATA_NUMBER 30000
namespace ParaInfoCenter
{
	class CICGen;
	class CICRecordSet;
	class DBEntity;
	//the CICGenManager is a singleton 
	class CICGenManager
	{
	public:
		static CICGenManager *Instance();
		static wstring QuoteReplace(const WCHAR* input);
		void Finalize();
		void Open();
		void Close();
		//gets all trace 
		CICRecordSet *GetMapTrace();
		//gets the where defined trace
		CICRecordSet *GetMapTrace(const WCHAR* where);
		//gets data at (x,y)'s block
		CICRecordSet *GetTraceData(double x, double y);
		//gets data at all blocks in rectangle (x1,y1)-(x2,y2), x1<x2, y1<y2
		CICRecordSet *GetTraceData(double x1,double y1,double x2, double y2);
		~CICGenManager();
		DBEntity* GetDB(){
			return m_db.get();
		};
	protected:
		CICGenManager();
		void CreateMemTables();
		void CreateFileTables();
		ParaEngine::asset_ptr<DBEntity> m_db;
	};
}
