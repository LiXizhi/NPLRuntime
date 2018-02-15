//-----------------------------------------------------------------------------
// Authors: Liu, Weili
// Date:	2006.5.10
// desc:
// This 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BaseTable.h"
#include "memdebug.h"
using namespace ParaEngine;
//////////////////////////////////////////////////////////////////////////
// CBaseRowColumn
//////////////////////////////////////////////////////////////////////////
/**
* Clone never clones the m_nIndex, m_parent, m_bIsFixedSize and m_bIsValid
* new object is: m_nIndex=-1;m_parent=NULL;m_bIsFixedSize=false;m_bIsValid=false;
*/
IObject* CBaseRowColumn::Clone()const
{
	CBaseRowColumn* newOne=new CBaseRowColumn();
	Clone(newOne);
	return newOne;
}
void CBaseRowColumn::Clone(IObject *pobj)const
{
	assert(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}
	CBaseRowColumn* pRC=(CBaseRowColumn*)pobj;
	pRC->Resize(Size());
	for (int a=0;a<Size();a++) {
		pRC->InternalSetAt((*(IObject*)GetAt(a)),a);
	}
}

void CBaseRowColumn::Clear()
{
	m_data.Clear();
}

bool CBaseRowColumn::Equals(const IObject*obj)const
{
	if (obj==NULL) {
		return false;
	}
	if (((CBaseRowColumn*)obj)->Size()!=this->Size()) {
		return false;
	}
	for (int a=0;a<Size();a++) {
		if (!GetAt(a)->Equals(((CBaseRowColumn*)obj)->GetAt(a))) {
			return false;
		}
	}
	return true;
}
/**
* Add an object to the row or column, will failed if m_bIsFixedSize==true
* @return: new size of the row or column, -1 if failed 
*/
//call mdata's Add()
int CBaseRowColumn::Add(IObject& obj)
{
	if (!m_bIsFixedSize&&!IsValid()) {
		return -1;
	}
	m_data.Add(&obj);
	return m_data.Size()-1;	
}

/**
* Set the object at a given position in a row or column
* This will change the corresponding records in the CBaseTable
* @return: the index of the inserted object. -1 if failed.
*/
//call CBaseTable::SetAt();
int CBaseRowColumn::SetAt(IObject& obj, int index)
{
	if (!IsValid()) {
		return -1;
	}
	return InternalSetAt(obj,index);
}
int CBaseRowColumn::InternalSetAt(IObject& obj, int index)
{
	return m_data.SetAt(&obj,index);
}
/**
* Resize the current row or column, if m_IsFixedSize==true, it will fail
* @return: new size if successes, -1 if failed
*/
int CBaseRowColumn::Resize(int newsize)
{
	if (m_bIsFixedSize) {
		return -1;
	}
	return m_data.Resize(newsize);
}

/**
* insert an object at a given position,, will failed if m_bIsFixedSize==true
* @return: the index of the inserted object. -1 if failed.
*/
//call m_data's Insert()
int CBaseRowColumn::Insert(IObject& obj, int index)
{
	if (!m_bIsFixedSize&&!IsValid()) {
		return -1;
	}
	return m_data.Insert(&obj,index);
}

/**
* get the IObject at a given position
* @return: NULL if not exists.
*/
//call corresponding CBaseTable::GetAt
const IObject* CBaseRowColumn::GetAt(int index)const
{
	if (!IsValid()){
		return NULL;
	}
	if(m_parent){
		if (m_etype==Table_Column){
			return m_parent->GetAt(index,m_nIndex);
		}else if (m_etype==Table_Row){
			return m_parent->GetAt(m_nIndex,index);
		}
	}
	return m_data.GetAt(index);
}
IObject* CBaseRowColumn::GetAt(int index)
{
	if (!IsValid()){
		return NULL;
	}
	return m_data.GetAt(index);
}

bool CBaseRowColumn::IsValid()const
{
	if (m_parent==NULL) {
		return true;
	}
	return true;
}

int CBaseRowColumn::Remove(int index)
{
	if (!m_bIsFixedSize&&!IsValid()) {
		return -1;
	}
	return m_data.Remove(index);
}
bool CBaseRowColumn::Update()
{
	if (m_parent){
		return true;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
// CBaseTable
//////////////////////////////////////////////////////////////////////////
// *
// * Release all resource, output error if some of the rows and columns are not properly release via ReleaseXX() functions
// CBaseTable::~CBaseTable()
// {
// 	Clear();
// }
// 
// CBaseTable::CBaseTable(int rowsize,int columnsize):m_bRowValid(true),m_bColumnValid(true)
// {
// 	
// 	m_nColumnSize=columnsize>0?columnsize:0;
// 	m_nRowSize=rowsize>0?rowsize:0;
// 	m_data.Resize(rowsize*columnsize);
// 	m_Rows.Resize(rowsize);
// 	m_Columns.Resize(columnsize);
// }
// *
// * insert a new row at a given position, return the newly inserted row (it is an empty row)
// * the row is filled with null (using Resize())
// * will invalidate the rows which are larger than index and all columns.
// int CBaseTable::InsertRow(int index)
// {
// 	//do not set row valid here because it calls another insertrow();
// 	if ((DWORD)index>(DWORD)m_Rows.Size()) {
// 		return -1;
// 	}
// 	CBaseRowColumn* newOne=new CBaseRowColumn(index,Table_Row,false);
// 	int re=InsertRow(*newOne,index);
// 	SAFE_RELEASE(newOne);
// 	return re;
// }
// *
// * insert an existing row at a given position
// * if the size of the input object does not match the CBaseTable's, the input object's 
// *  Resize() will be called to set the correct size;
// * will invalidate the rows which are larger than index and all columns.
// //assign correct type of the input row, check if index valid,
// int CBaseTable::InsertRow(CBaseRowColumn& row, int index)
// {
// 	if ((DWORD)index>(DWORD)m_Rows.Size()) {
// 		return -1;
// 	}
// 	int start,a;
// 	start=index*m_nColumnSize;
// 	for (a=0;a<m_nColumnSize;a++) {
// 
// 		if (m_data.Insert((row.GetAt(a)),start+a)!=start+a) {
// 			//error
// 			OUTPUT_LOG("CBaseTable::InsertRow error\n");
// 			return -1;
// 		}
// 	}
// 	m_nRowSize++;
// 	return index;
// }
// *
// * get a row
// * @return: NULL if not exists.
// CBaseRowColumn* CBaseTable::GetRow(int index)
// {
// 	if ((DWORD)index>=(DWORD)m_nRowSize){
// 		return NULL;
// 	}
// 	if (!m_bRowValid){
// 		Update();
// 	}
// 	CBaseRowColumn* re=m_Rows.GetAt(index);
// 	if (re==NULL){
// 		re=new CBaseRowColumn(index,Table_Row,false);
// 		for (int a=0,b=index*m_nColumnSize;a<m_nColumnSize;a++,b++){
// 			re->m_data.Add(m_data.GetAt(b));
// 		}
// 		re->m_bIsFixedSize=true;
// 		re->m_parent=this;
// 		m_Rows.SetAt(re,index);
// 	}else{
// 		re->addref();
// 	}
// 	return re;
// }
// // const CBaseRowColumn* CBaseTable::GetRow(int index)const
// // {
// // 	if ((DWORD)index>=(DWORD)m_nRowSize){
// // 		return NULL;
// // 	}
// // 	const CBaseRowColumn* re=m_Rows.GetAt(index);
// // 	CBaseRowColumn* re1;
// // 	if (re==NULL){
// // 
// // 		re1=new CBaseRowColumn(index,Table_Row,false);
// // 		for (int a=0,b=index*m_nColumnSize;a<m_nColumnSize;a++,b++){
// // 			re1->m_data.Add(m_data.GetAt(b));
// // 		}
// // 		m_Rows.SetAt(re,index);
// // 		re->m_bIsFixedSize=true;		
// // 	}else{
// // 		re->addref();
// // 	}
// // 	return re;
// // }
// *
// * insert a new column at a given position, return the newly inserted column (it is an empty column)
// * the column is filled with null(using Resize())
// * will invalidate the columns which are larger than index and all rows.
// int CBaseTable::InsertColumn(int index)
// {
// 	//do not set row valid here because it calls another insertrow();
// 	if ((DWORD)index>(DWORD)m_nColumnSize) {
// 		return -1;
// 	}
// 	CBaseRowColumn* newOne=new CBaseRowColumn(index,Table_Column,false);
// 	int re=InsertColumn(*newOne,index);
// 	SAFE_RELEASE(newOne);
// 	return re;
// }
// *
// * insert an existing column at a given position
// * if the size of the input object does not match the CBaseTable's, the input object's 
// *  Resize() will be called to set the correct size;
// * will invalidate the columns which are larger than index and all rows.
// //assign correct type of the input column, check if index valid,
// int CBaseTable::InsertColumn(CBaseRowColumn& column, int index)
// {
// 	if ((DWORD)index>(DWORD)m_nColumnSize) {
// 		return -1;
// 	}
// 	int a,b;
// 	for (a=index+(m_nRowSize-1)*m_nColumnSize,b=m_nRowSize-1;b>=0;a-=(m_nColumnSize),b--) {
// 		if (m_data.Insert((column.GetAt(b)),a)!=a) {
// 			//error
// 			OUTPUT_LOG("CBaseTable::InsertColumn error\n");
// 			return -1;
// 		}
// 	}
// 	m_bRowValid=false;
// 	m_nColumnSize++;
// 	return index;
// }
// *
// * get a column
// * @return: NULL if not exists.
// CBaseRowColumn* CBaseTable::GetColumn(int index)
// {
// 	if ((DWORD)index>=(DWORD)m_nColumnSize){
// 		return NULL;
// 	}
// 	if (!m_bColumnValid){
// 		Update();
// 	}
// 	CBaseRowColumn* re=m_Columns.GetAt(index);
// 	if (re==NULL){
// 		re=new CBaseRowColumn(index,Table_Column,false);
// 		for (int a=0,b=index;a<m_nRowSize;a++,b+=m_nColumnSize){
// 			re->m_data.Add(m_data.GetAt(b));
// 		}
// 		re->m_bIsFixedSize=true;		
// 		re->m_parent=this;
// 		m_Columns.SetAt(re,index);
// 	}else{
// 		re->addref();
// 	}
// 	return re;
// }
// // const CBaseRowColumn* CBaseTable::GetColumn(int index)const
// // {
// // 	if ((DWORD)index>=(DWORD)m_nColumnSize){
// // 		return NULL;
// // 	}
// // 	cosnt CBaseRowColumn* re=m_Columns.GetAt(index);
// // 	if (re==NULL){
// // 		re=new CBaseRowColumn(index,Table_Column,false);
// // 		for (int a=0,b=index;a<m_nRowSize;a++,b+=m_nColumnSize){
// // 			re->m_data.Add(m_data.GetAt(b));
// // 		}
// // 		m_Columns.SetAt(re,index);
// // 		re->m_bIsFixedSize=true;		
// // 	}else{
// // 		re->addref();
// // 	}
// // 	return re;
// // }
// *
// * get the IObject at a given position
// * @return: NULL if not exists.
// const IObject* CBaseTable::GetAt(int row, int column)const
// {
// 	if ((DWORD)row>=(DWORD)m_nRowSize||(DWORD)column>=(DWORD)m_nColumnSize){
// 		return NULL;
// 	}
// 	return m_data.GetAt(row*m_nColumnSize+column);
// }
// IObject* CBaseTable::GetAt(int row, int column)
// {
// 	if ((DWORD)row>=(DWORD)m_nRowSize||(DWORD)column>=(DWORD)m_nColumnSize){
// 		return NULL;
// 	}
// 	return m_data.GetAt(row*m_nColumnSize+column);
// }
// 
// *
// * Set the IObject at a given position
// * @return: return row*number_of_columns+column, -1 if failed
// int CBaseTable::SetAt(IObject& obj, int row, int column)
// {
// 	if ((DWORD)row>=(DWORD)m_nRowSize||(DWORD)column>=(DWORD)m_nColumnSize){
// 		return -1;
// 	}
// 	return m_data.SetAt(&obj,row*m_nColumnSize+column);
// }
// 
// //delete the column and its reference in the CBaseTable
// void CBaseTable::DeleteColumn(int index)
// {
// 	if (m_Columns.GetAt(index)){
// 		m_Columns.Remove(index);
// 	}
// 	int a,b;
// 	for (a=index+(m_nRowSize-1)*m_nColumnSize,b=0; b<m_nRowSize ; a-=m_nColumnSize,b++){
// 		m_data.Remove(a);
// 	}
// }
// //delete the row and its reference in the CBaseTable
// void CBaseTable::DeleteRow(int index){
// 	if (m_Rows.GetAt(index)){
// 		m_Rows.Remove(index);
// 	}
// 	int a,b;
// 	for (a=(index+1)*m_nColumnSize-1,b=0; b<m_nColumnSize ; a--,b++){
// 		m_data.Remove(a);
// 	}
// 
// }
// 
// //delete all rows and columns
// void CBaseTable::Clear(){
// 	m_Rows.Clear();
// 	m_Columns.Clear();
// 	m_data.Clear();
// 	m_bRowValid=true;
// 	m_bColumnValid=true;
// }
// 
// bool CBaseTable::Update()
// {
// 	CBaseRowColumn *temp;
// 	if (!m_bColumnValid){
// 		for (int a=0;a<m_nColumnSize;a++){
// 			if ((temp=m_Columns.GetAt(a))!=NULL){
// 				for (int b=0,c=temp->m_nIndex;b<m_nRowSize;b++,c+=m_nColumnSize){
// 					temp->m_data.SetAt(m_data.GetAt(c),b);
// 				}
// 			}
// 		}
// 		m_bColumnValid=true;
// 	}
// 	if (!m_bRowValid){
// 		for (int a=0;a<m_nRowSize;a++){
// 			if ((temp=m_Rows.GetAt(a))!=NULL){
// 				for (int b=0,c=temp->m_nIndex*m_nColumnSize;b<m_nColumnSize;b++,c++){
// 					temp->m_data.SetAt(m_data.GetAt(c),b);
// 				}
// 			}
// 		}
// 		m_bRowValid=true;
// 	}
// 	return true;
// }
// 
// void CBaseTable::CleanCache()
// {
// 	if (!m_bRowValid){
// 		m_Rows.Clear();
// 		m_bRowValid=true;
// 	}
// 	if (!m_bColumnValid){
// 		m_Columns.Clear();
// 		m_bColumnValid=true;
// 	}
// }

/**
* Release all resource, output error if some of the rows and columns are not properly release via ReleaseXX() functions
*/
CBaseTable::~CBaseTable()
{
	Clear();
}

CBaseTable::CBaseTable(int rowsize,int columnsize)
{

	m_nColumnSize=columnsize>0?columnsize:0;
	m_nRowSize=rowsize>0?rowsize:0;
	for (int a = 0; a < m_nRowSize ; a++)
	{
		CBaseRowColumn* row=new CBaseRowColumn(a,Table_Row,false);
		row->Resize(m_nColumnSize);
		row->m_parent=this;
		row->m_bIsFixedSize=true;
		m_Rows.Add(row);
		SAFE_RELEASE(row);
	}
}
/**
* insert a new row at a given position, return the newly inserted row (it is an empty row)
* the row is filled with null (using Resize())
* will invalidate the rows which are larger than index and all columns.
*/
int CBaseTable::InsertRow(int index)
{
	//do not set row valid here because it calls another insertrow();
	if ((DWORD)index>(DWORD)m_Rows.Size()) {
		return -1;
	}
	CBaseRowColumn* newOne=new CBaseRowColumn(index,Table_Row,false);
	int re=InsertRow(*newOne,index);
	SAFE_RELEASE(newOne);
	return re;
}
/**
* insert an existing row at a given position
* if the size of the input object does not match the CBaseTable's, the input object's 
*  Resize() will be called to set the correct size;
* will invalidate the rows which are larger than index and all columns.
*/
//assign correct type of the input row, check if index valid,
int CBaseTable::InsertRow(CBaseRowColumn& row, int index)
{
	if ((DWORD)index>(DWORD)m_Rows.Size()) {
		return -1;
	}
	row.Resize(m_nColumnSize);
	if (m_Rows.Insert(&row,index)!=index){
		//error
		OUTPUT_LOG("CBaseTable::InsertRow error\n");
		return -1;
	}
	m_nRowSize++;
	return index;
}
/**
* get a row
* @return: NULL if not exists.
*/
CBaseRowColumn* CBaseTable::GetRow(int index)
{
	if ((DWORD)index>=(DWORD)m_nRowSize){
		return NULL;
	}
	CBaseRowColumn* re=m_Rows.GetAt(index);
	re->addref();
	return re;
}

/**
* insert a new column at a given position, return the newly inserted column (it is an empty column)
* the column is filled with null(using Resize())
* will invalidate the columns which are larger than index and all rows.
*/
int CBaseTable::InsertColumn(int index)
{
	//do not set row valid here because it calls another insertrow();
	if ((DWORD)index>(DWORD)m_nColumnSize) {
		return -1;
	}
	CBaseRowColumn* newOne=new CBaseRowColumn(index,Table_Column,false);
	int re=InsertColumn(*newOne,index);
	SAFE_RELEASE(newOne);
	return re;
}
/**
* insert an existing column at a given position
* if the size of the input object does not match the CBaseTable's, the input object's 
*  Resize() will be called to set the correct size;
* will invalidate the columns which are larger than index and all rows.
*/
//assign correct type of the input column, check if index valid,
int CBaseTable::InsertColumn(CBaseRowColumn& column, int index)
{
	if ((DWORD)index>(DWORD)m_nColumnSize) {
		return -1;
	}
	int a;
	for (a=0;a<m_nRowSize;a++){
		if (m_Rows.GetAt(a)->Insert(*column.GetAt(a),index)!=index) {
			//error
			OUTPUT_LOG("CBaseTable::InsertColumn error\n");
			return -1;
		}
	}
	m_nColumnSize++;
	return index;
}

/**
* get the IObject at a given position
* @return: NULL if not exists.
*/
const IObject* CBaseTable::GetAt(int row, int column)const
{
	if ((DWORD)row>=(DWORD)m_nRowSize||(DWORD)column>=(DWORD)m_nColumnSize){
		return NULL;
	}
	return m_Rows.GetAt(row)->GetAt(column);
}
IObject* CBaseTable::GetAt(int row, int column)
{
	if ((DWORD)row>=(DWORD)m_nRowSize||(DWORD)column>=(DWORD)m_nColumnSize){
		return NULL;
	}
	return m_Rows.GetAt(row)->GetAt(column);
}

/**
* Set the IObject at a given position
* @return: return row*number_of_columns+column, -1 if failed
*/
int CBaseTable::SetAt(IObject& obj, int row, int column)
{
	if ((DWORD)row>=(DWORD)m_nRowSize||(DWORD)column>=(DWORD)m_nColumnSize){
		return -1;
	}
	return m_Rows.GetAt(row)->SetAt(obj,column);

}

//delete the column and its reference in the CBaseTable
void CBaseTable::DeleteColumn(int index)
{
	if ((DWORD)index>=(DWORD)m_nColumnSize){
		return;
	}
	int a;
	for (a=0;a<m_nRowSize;a++){
		m_Rows.GetAt(a)->Remove(index);
	}
}
//delete the row and its reference in the CBaseTable
void CBaseTable::DeleteRow(int index){
	if (m_Rows.GetAt(index)){
		m_Rows.Remove(index);
	}
}

//delete all rows and columns
void CBaseTable::Clear(){
	m_Rows.Clear();
}


#ifdef _DEBUG
class testtable:public IObject
{
public:
	virtual bool Equals(const IObject *obj){return true;}
	virtual IObject* Clone(){testtable* temp= new testtable();temp->a=this->a;return temp;}
	virtual void Clone(const IObject* obj){((testtable*)obj)->a=this->a;}
	int a;
};
void CBaseTable::test()
{
	//test 1: base new and delete of CBaseTable
	CBaseTable *t1;
	// 	t1=new CBaseTable();
	// 	delete t1;
	// 	t1=new CBaseTable(1,1);
	// 	delete t1;
	// 	t1=new CBaseTable(-10,0);
	// 	delete t1;
	//test 2: independent row and column functionality
	CBaseRowColumn* r1=new CBaseRowColumn();
	if (r1->m_etype!=Table_Neutral||r1->m_bIsFixedSize!=false){
		OUTPUT_LOG("\nCBaseTable Test 2 failed\n");
	}
	SAFE_RELEASE(r1);
	//test add, get and change
	r1=new CBaseRowColumn();
	testtable *temp1=new testtable();
	temp1->a=1;	
	if (r1->Add(*temp1)<0){
		OUTPUT_LOG("\nCBaseTable Test 2 failed\n");
	}
	SAFE_RELEASE(temp1);
	temp1=new testtable();
	temp1->a=2;
	r1->Add(*temp1);
	r1->Add(*temp1);
	OUTPUT_LOG("CBaseTable Test 2:\n output 1--before change: ");
	for (int a=0;a<r1->Size();a++){
		OUTPUT_LOG("%d ",((testtable*)r1->GetAt(a))->a);
		//output should be "1 2 2"
	}
	//change value
	((testtable*)r1->GetAt(0))->a=3;
	((testtable*)r1->GetAt(1))->a=4;
	OUTPUT_LOG("\n output 2--after change: ");
	for (int a=0;a<r1->Size();a++){
		OUTPUT_LOG("%d ",((testtable*)r1->GetAt(a))->a);
		//output should be "3 4 4"
	}
	SAFE_RELEASE(temp1);
	//test insert and remove
	temp1=new testtable();
	temp1->a=5;
	r1->Insert(*temp1,1);
	if (r1->Insert(*temp1,4)!=4||r1->Insert(*temp1,6)!=-1||r1->Insert(*temp1,-1)!=-1){
		OUTPUT_LOG("\nCBaseTable Test 2 failed\n");
	}
	SAFE_RELEASE(temp1);
	r1->Remove(2);
	if (r1->Remove(-1)!=-1){
		OUTPUT_LOG("\nCBaseTable Test 2 failed\n");
	}
	OUTPUT_LOG("\n output 3--insert and remove: ");
	for (int a=0;a<r1->Size();a++){
		OUTPUT_LOG("%d ",((testtable*)r1->GetAt(a))->a);
		//output should be "3 5 4 5"
	}
	temp1=new testtable();
	temp1->a=-1;
	if (r1->SetAt(*temp1,2)!=2||r1->SetAt(*temp1,-1)!=-1||r1->SetAt(*temp1,5)!=-1){
		OUTPUT_LOG("\nCBaseTable Test 2 failed\n");
	}
	SAFE_RELEASE(temp1);
	OUTPUT_LOG("\n output 4--setat: ");
	for (int a=0;a<r1->Size();a++){
		OUTPUT_LOG("%d ",((testtable*)r1->GetAt(a))->a);
		//output should be "3 5 -1 5"
	}
	SAFE_RELEASE(r1);

	//test 3: create and modify table
	t1=new CBaseTable(2,2);
	temp1=(testtable*)t1->GetAt(0,0);
	if (temp1!=NULL){
		OUTPUT_LOG("\nCBaseTable Test 3 failed\n");
	}
	temp1=new testtable();
	temp1->a=1;
	if (t1->SetAt(*temp1,0,0)!=0||t1->SetAt(*temp1,-1,0)!=-1||t1->SetAt(*temp1,1,-1)!=-1||t1->SetAt(*temp1,2,2)!=-1||t1->SetAt(*temp1,2,-1)!=-1){
		OUTPUT_LOG("\nCBaseTable Test 3 failed\n");
	}
	SAFE_RELEASE(temp1);
	for (int a = 0; a < t1->NumberOfRows() ; a++)	{
		for (int b = 0; b < t1->NumberOfColumns() ; b++){
			temp1=new testtable();
			temp1->a = a * t1->NumberOfColumns() + b;
			t1->SetAt(*temp1,a,b);
			SAFE_RELEASE(temp1);			
		}      		
	}
	OUTPUT_LOG("\nCBaseTable Test 3: \n output 1--init:\n ");
	for (int a = 0; a < t1->NumberOfRows() ; a++)	{
		for (int b = 0; b < t1->NumberOfColumns() ; b++){
			OUTPUT_LOG("%d ",((testtable*)t1->GetAt(a,b))->a);
			//output should be "0 1 \n 2 3"
		}      		
		OUTPUT_LOG("\n");
	}
	if (t1->InsertRow(0)!=0||t1->InsertRow(-1)!=-1||t1->InsertRow(4)!=-1){
		OUTPUT_LOG("CBaseTable Test 3 failed\n");
	}
	for (int a = 0; a < t1->NumberOfRows() ; a++)	{
		for (int b = 0; b < t1->NumberOfColumns() ; b++){
			temp1=new testtable();
			temp1->a = a * t1->NumberOfColumns() + b;
			t1->SetAt(*temp1,a,b);
			SAFE_RELEASE(temp1);			
		}      		
	}
	OUTPUT_LOG("\n output 2--insert a row at 0:\n ");
	for (int a = 0; a < t1->NumberOfRows() ; a++)	{
		for (int b = 0; b < t1->NumberOfColumns() ; b++){
			OUTPUT_LOG("%d ",((testtable*)t1->GetAt(a,b))->a);
			//output should be "0 1\n 2 3\n 4 5"
		}      		
		OUTPUT_LOG("\n");
	}
	//change a row
	r1=t1->GetRow(0);
	temp1=new testtable();
	temp1->a=-1;
	r1->SetAt(*temp1,1);
	SAFE_RELEASE(temp1);
	SAFE_RELEASE(r1);
	OUTPUT_LOG("\n output 3--change the just inserted row:\n ");
	for (int a = 0; a < t1->NumberOfRows() ; a++)	{
		for (int b = 0; b < t1->NumberOfColumns() ; b++){
			OUTPUT_LOG("%d ",((testtable*)t1->GetAt(a,b))->a);
			//output should be "0 -1\n 2 3\n 4 5"
		}      		
		OUTPUT_LOG("\n");
	}
	r1=new CBaseRowColumn();
	temp1=new testtable();
	temp1->a=8;
	r1->Add(*temp1);
	SAFE_RELEASE(temp1);
	temp1=new testtable();
	temp1->a=9;
	r1->Add(*temp1);
	SAFE_RELEASE(temp1);
	temp1=new testtable();
	temp1->a=10;
	r1->Add(*temp1);
	SAFE_RELEASE(temp1);
	if (t1->InsertColumn(*r1,2)!=2){
		OUTPUT_LOG("CBaseTable Test 3 failed\n");
	}
	OUTPUT_LOG("\n output 4--insert column at 2:\n");
	for (int a = 0; a < t1->NumberOfRows() ; a++)	{
		for (int b = 0; b < t1->NumberOfColumns() ; b++){
			OUTPUT_LOG("%d ",((testtable*)t1->GetAt(a,b))->a);
			//output should be "0 -1 8\n 2 3 9\n 4 5 10"
		}      		
		OUTPUT_LOG("\n");
	}
	SAFE_RELEASE(r1);
	// 	r1=t1->GetRow(0);
	// 	if (r1->SetAt(*temp1,0)!=0){
	// 		OUTPUT_LOG("CBaseTable Test 3 failed\n");
	// 	}
	// 	r1->Update();
	// 	SAFE_RELEASE(r1);
	r1=t1->GetRow(0);
	temp1=new testtable();
	temp1->a=-2;
	if (r1->SetAt(*temp1,0)==-1){
		OUTPUT_LOG("CBaseTable Test 3 failed\n");
	}
	SAFE_RELEASE(r1);
	SAFE_RELEASE(temp1);
	OUTPUT_LOG("\n output 5--update and change:\n");
	for (int a = 0; a < t1->NumberOfRows() ; a++)	{
		for (int b = 0; b < t1->NumberOfColumns() ; b++){
			OUTPUT_LOG("%d ",((testtable*)t1->GetAt(a,b))->a);
			//output should be "-2 -1 8\n 2 3 9\n 4 5 10"
		}      		
		OUTPUT_LOG("\n");
	}

	delete t1;
}
#endif
