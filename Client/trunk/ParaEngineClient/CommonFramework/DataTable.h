#pragma once
#include <map>
#include <vector>
#include <string>
#include "Type.h"
using namespace std;
namespace ParaEngine
{
	enum DATAROW_STATE{
		DataRow_Detached=1,
		DataRow_Unchanged=2,
		DataRow_Added=4,
		DataRow_Deleted=8,
		DataRow_Modified=16
	};
	class CDataColumnInfo:public IObject
	{
	public:
		CDataColumnInfo():m_bReadonly(false),m_bAllowNull(true),m_nMaxLength(-1),m_parent(NULL),m_datatype(IType::GetType("string")){m_type=IType::GetType("void");}
		CDataColumnInfo(IType* ptype):m_bReadonly(false),m_bAllowNull(true),m_nMaxLength(-1),m_parent(NULL),m_datatype(ptype){m_type=IType::GetType("void");}
		CDataColumnInfo(const string&szColumnName);
		CDataColumnInfo(const string&szColumnName,IType* ptype);

		virtual const IType* GetType()const { return m_type; };
		//memcpy data and assign the string data
		virtual IObject* Clone()const;
		virtual void Clone(IObject *obj)const;

		/**
		* Get or set caption of the column. Caption is what is displayed in the header of the column
		* If m_szCaption's length==0, return ColumnName instead
		*/
		virtual const char* GetCaption()const;
		//if szText==null or length==0, the m_szCaption will be cleared.
		virtual void SetCaption(const char* szText);

		virtual const char* GetColumnName()const{return m_szColumnName.c_str();}
		virtual void SetColumnName(const char* szText){m_szColumnName=szText;}

		virtual bool GetReadOnly()const{return m_bReadonly;}
		virtual void SetReadOnly(bool readonly){m_bReadonly=readonly;}

		virtual int GetMaxLength()const{return m_nMaxLength;}
		virtual void SetMaxLength(int maxlength){m_nMaxLength=maxlength;}//may need update something in its parent

		virtual bool GetAllowNull()const {return m_bAllowNull;}
		virtual void SetAllowNull(bool allownull){m_bAllowNull=allownull;}

		virtual const IType* GetDataType()const{return m_datatype;}
		friend class CDataTable;
		friend class CDataRow;
		friend class CColumnCollection;
	protected:
		IType* m_datatype;
		CBaseTable* m_parent;
		string m_szCaption;
		string m_szColumnName;
		bool m_bReadonly;
		bool m_bAllowNull;
		int m_nMaxLength;//-1 means unbound
		IType* m_type;
	};
	class CDataRow;
	class CDataCellItem:public IObject
	{
	public:
		CDataCellItem(IObject* data,CDataRow* parent,CDataColumnInfo* columninfo);
		virtual ~CDataCellItem();
		virtual const IType* GetType()const { return m_type; };
		/**
		 * clone every thing, but the data only adds reference instead of calling data's Clone()
		 **/
		IObject* Clone()const;
		void Clone(IObject*)const ;
		/**
		* Assigning different value to the cell. If type not match, nothing will happen;
		*/
		//use the different type's parse function to convert the string to the type, only the value types can be converted
		virtual CDataCellItem& operator =(const string& value);
		//convert input to correct type, only the value types can be converted
		virtual CDataCellItem& operator =(int value);
		//convert input to correct type, only the value types can be converted
		virtual CDataCellItem& operator =(bool value);
		//convert input to correct type, only the value types can be converted
		virtual CDataCellItem& operator =(float value);
		//convert input to correct type, only the value types can be converted
		virtual CDataCellItem& operator =(double value);
		//convert input to correct type, if not value type, the value's refcount is increased.
		virtual CDataCellItem& operator =(IObject& value);
		//if value==null, call setnull
		//convert input to correct type, if not value type, the value's refcount is increased.
		virtual CDataCellItem& operator =(CDataCellItem* value);
		virtual operator string();
		virtual operator int();
		virtual operator bool();
		virtual operator float();
		virtual operator double();
		virtual operator const char*();
		virtual operator IObject*();
		bool IsNull(){return m_data==NULL;}
		//if column allow null, return true and release the m_data, else return false;
		bool SetNull();
		bool IsValid()const{return m_bIsValid;}
	protected:
		IObject* m_data;
		CDataRow* m_parent;
		CDataColumnInfo* m_columninfo;
		bool m_bIsValid;
		IType* m_type;
	};

	class CDataColumnInfo;
	class CColumnCollection:public ICollection<CDataColumnInfo>
	{
	public:
		enum ColumnCollectionAction
		{
			ColumnAction_Add,//add one
			ColumnAction_Remove,//remove one
			ColumnAction_Refresh//complete change
		};
		CColumnCollection(CDataTable* parent):m_parent(parent),m_row(NULL){}
		CColumnCollection(CDataRow* row):m_parent(NULL),m_row(row){}
		virtual CDataColumnInfo* GetAt(int index);
		virtual const CDataColumnInfo* GetAt(int index)const;
		virtual CDataColumnInfo* GetAt(const string& szName);
		virtual const CDataColumnInfo* GetAt(const string& szName)const;
		virtual void Clear();
		/**
		* delete a column
		*/
		virtual int Remove(int index);
		virtual int IndexOf(const string& szName)const;
		virtual void Add();
		virtual void Add(CDataColumnInfo *obj);
		/**
		* Default type is string
		* 
		*/
		virtual void Add(const char* szName);
		virtual void Add(const char* szName,IType* pType);
		virtual int Size()const{return m_data.Size();}
		virtual int Resize(int newsize){return m_data.Resize(newsize);}
// 		virtual void SetOnChangeMethod(void(*newmethod)(int,CDataColumnInfo*) ){m_onchange=newmethod;}
	protected:
		Collection_Vector<CDataColumnInfo> m_data;
		map<string,int> m_map;
		CDataTable *m_parent;
		CDataRow *m_row;
// 		void(*m_onchange)(int eAction,CDataColumnInfo* obj) ;
	private:
		virtual int Insert(CDataColumnInfo* obj, int index){return -1;}
		virtual int SetAt(CDataColumnInfo* obj, int index){return -1;}
		virtual int Remove(){return -1;};
		virtual int IndexOf(const CDataColumnInfo *obj)const{return -1;};
		virtual int CopySetAt(const CDataColumnInfo *obj, int index){return -1;};
		virtual void CopyAdd(const CDataColumnInfo *obj){};
		virtual int CopyInsert(CDataColumnInfo* obj, int index){return -1;};
		static int ColumnNameCounter;
	};
	/**
	* children of data row is some addition data rows. they do not attach to any table
	* TODO: add undo support of the rows, so changes only take place when EndEdit() is executed

	*/
	class CDataRow:public CBaseRowColumn
	{
	public:
		CDataRow();
		virtual ~CDataRow();
		//also clone children rows recursively
		virtual IObject* Clone()const;
		virtual void Clone(IObject *obj)const;

		//return the editable children rows collection
		virtual ICollection<CDataRow>* GetChildRows();

		//check input data, encapsulate it into a CDataCellItem
		virtual int SetAt(IObject&obj, int index);

		virtual const CDataCellItem* GetAt(int index)const;
		virtual CDataCellItem* GetAt(int index);

		///delete the row, 
		virtual void Delete();
		virtual int Resize(int newsize);

		/**
		 * Begin the edit transaction. Operation such as SetAt(), Delete, operator [] will not actually take place until EndEdit() is executed
		 **/
		virtual void BeingEdit(){m_bEditing=true;}

		virtual void CancelEdit(){}
		/**
		 * validates all modified data and set m_bEditing =false
		 * 
		 **/
		virtual int EndEdit();

		virtual int GetRowState(){return m_eRowState;}

		//Gets a readonly data
		const CDataCellItem& operator [](const string& szName)const;
		const CDataCellItem& operator [](int index)const{return *GetAt(index);}
		//Get a writable data
		CDataCellItem& operator[](const string& szName);
		CDataCellItem& operator[](int index){return *GetAt(index);}

		virtual CColumnCollection* ColumnInfos(){return m_ColumnInfo;}

	protected:

		Collection_Vector<CDataRow> m_children;
		//this info has higher priority over the table's columninfo
		//if set, the functions will use this instead of the table's
		CColumnCollection* m_ColumnInfo;

		bool m_bEditing;//if the row is being edited. if true, the validation will not trigger automatically
		int m_eRowState;//DATAROW_STATE
	};

	//support adding and removing undo
	class CDataTable:public CBaseTable
	{
	public:
		CDataTable();
		virtual ~CDataTable();
		virtual CColumnCollection* GetColumnInfos(){return m_ColumnInfo;}
	protected:
		CColumnCollection* m_ColumnInfo;
		string m_szTableName;

	};

}