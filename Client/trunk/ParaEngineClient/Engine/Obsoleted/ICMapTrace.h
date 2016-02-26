#pragma once

#include <string>
#include "ICGen.h"

namespace ParaInfoCenter
{
	// the recorded map trace in runtime
	class CICGenMapTrace:public CICGen
	{
	public:
		CICGenMapTrace();
		DWORD m_time; //time when this gen generated, in tick count
		/**
		* Coordinates of this gen in the current map
		* The means are different from the coordinates of the CBaseObject;
		* x, y are coordinates of the horizontal plane where the player is on. z is the height of this plane
		* In CBaseObject, x, z are coordinates of the horizontal plane where the player is on. y is the height of this plane
		*/
		double m_x,m_y,m_z; 
		wstring m_id; //whose trace is this

		///Get the logic block coordinates of the GEN
		void GetBlock(int &x, int &y, int &z);
		virtual void FromDB(CICRecordSet *rs); //get the GEN data from a sql result
		virtual void ToDB();//insert the GEN data to the db
	};	//this class should attach to each object we wish to trace
	class CICMapTrace
	{
	public:
		/**
		Get all trace.
		User should new enough instances of CICGenMapTrace before calling this function and delete them afterwards
		@note: IMPORTANT, make sure destructor of each item is called. Do not just free the memory block.
		@param: bDesc tells how the return data arranges, in ascending or descending order, default is descending order which means the latest one comes first
		@param: nLimit tells how many items at most is return, 0 means all items will be returned. This parameter does not affect the return if returndata is NULL.
		@return: return the number of CICGenMapTrace if returndata is null,return the actually get number of CICGenMapTrace 
			if returndata is not null, and all CICGenMapTrace items are stored in returndata
		*/
		int GetAllTrace(CICGenMapTrace *returndata, bool bDesc=true, int nLimit=0);

		/**
		* begin getting trace, call before you decide to get trace, 
		* if this is called, the getting trace operation will reset
		* if it's not called before GetTrace, These function will return nothing
		* @param bBegin: If bBegin is true, the current cursor will set to the first trace. if false, it will set to the last trace
		*/
		void BeginGettingTrace(bool bBegin=false); 

		/**
		* call if you want to end the operation, but it is not compulsory.
		*/
		void EndGettingTrace();

		/**
		* Data at the cursor are written to parameter maptrace and the cursor will move according to bForward to the next one or previous one
		* @param maptrace: the CICGenMapTrace item the data will be written to. 
		* @return: return true if success, false if failed due to BOF, EOF, DB error or not called BeginTrace yet
		*/
		bool GetTrace(CICGenMapTrace &maptrace);

		/**
		* Move the cursor to a given item.
		* @param index: where the cursor will move to
		* @return: return true, if we can successfully seek to the item. return false, if not
		*/
		bool SeekTrace(int index);

		void AddTrace(double x,double y, double z, const WCHAR* id);
		void DisplayTrace();
		///deletes the trace which is longer than certain time, in milliseconds.
		void DelTrace(DWORD nTime);
		CICMapTrace();
		~CICMapTrace();
	protected:
		CICGenMapTrace *m_lasttrace;
		int m_nCursor;
		bool m_bASC;
		int m_nLowerWindow;
		CICRecordSet *m_recordSet;
	private:
	};
}