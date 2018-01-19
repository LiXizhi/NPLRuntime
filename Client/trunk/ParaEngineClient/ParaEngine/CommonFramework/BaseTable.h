#include <map>
#include <vector>
#include "ICollection.h"
namespace ParaEngine
{
	class CBaseTable;

	enum Table_Row_Column_Type{
		Table_Neutral,
		Table_Column,
		Table_Row
	};

	/**
	* Although this class inherits the CRefCounted interface, I don't manage it in GC
	*/
	class CBaseRowColumn:public IObject
	{
	public:
		friend class CBaseTable;
		friend class CDataRow;

		/**
		* Clone never clones the m_nIndex, m_parent, m_bIsFixedSize 
		* new object is: m_nIndex=-1;m_parent=NULL;m_bIsFixedSize=false
		*/
		virtual IObject* Clone()const;
		virtual void Clone(IObject *obj)const;

		virtual void Clear();
		/**
		* Size and corresponding members of the two objects must equal to return true;
		* It uses the Equals() member function of the object to compare.
		*/
		virtual bool Equals(const IObject*obj)const;

		bool Update();//update the row or column from the CBaseTable

		/**
		* Add an object to the row or column, will failed if m_bIsFixedSize==true
		* Only the 
		* @return: new size of the row or column, -1 if failed 
		*/
		//call mdata's Add()
		virtual int Add(IObject& obj);

		/**
		* Set the object at a given position in a row or column
		* This will change the corresponding records in the CBaseTable
		* @return: the index of the inserted object. -1 if failed.
		*/
		//call CBaseTable::SetAt();
		virtual int SetAt(IObject& obj, int index);

		/**
		* insert an object at a given position,, will failed if m_bIsFixedSize==true
		* @return: the index of the inserted object. -1 if failed.
		*/
		//call m_data's Insert()
		virtual int Insert(IObject& obj, int index);

		/**
		* get the IObject at a given position
		* @return: NULL if not exists.
		*/
		//call corresponding CBaseTable::GetAt
		virtual const IObject* GetAt(int index)const;
		virtual IObject* GetAt(int index);

		/**
		* Resize the current row or column, if m_IsFixedSize==true, it will fail
		* @return: new size if successes, -1 if failed
		*/
		virtual int Resize(int newsize);
		virtual int Size()const{return m_data.Size();}

		/**
		* Check if the data in the row or column is valid
		* if not valid, the row or column's Update() must be called to make it valid.
		*/
		virtual bool IsValid()const;

		/**
		* Remove an object, if m_IsFixedSize==true, it will fail
		* @return: new size of the row or column, -1 if failed
		*/
		virtual int Remove(int index);
	protected:
		CBaseRowColumn():m_bIsFixedSize(false),m_etype(Table_Neutral),m_nIndex(-1),m_parent(NULL){}
		CBaseRowColumn(int index, int etype):m_bIsFixedSize(false),m_etype(etype),m_nIndex(index),m_parent(NULL){}
		CBaseRowColumn(int index, int etype,bool fixedsize):m_bIsFixedSize(fixedsize),m_etype(etype),m_nIndex(index),m_parent(NULL){}
		CBaseRowColumn(int index, bool fixedsize):m_bIsFixedSize(fixedsize),m_etype(Table_Neutral),m_nIndex(index),m_parent(NULL){}
		virtual ~CBaseRowColumn(){
			m_data.Clear();
		}
		/**
		* Internal function. Only set the object itself
		*/
		virtual int InternalSetAt(IObject& obj, int index);
		//only the row or column that is not related to a CBaseTable is m_bIsFixedSize==false;
		bool m_bIsFixedSize;
		int m_nIndex;//the row or column index of this object in the CBaseTable
		CBaseTable* m_parent;
		Collection_Vector<IObject> m_data;
		int m_etype;//identify if this is a row, column or neutral. enumeration Table_Row_Column_Type

	};



	//any change in this table will make all CBaseRow and CBaseColumn object of it Invalid
	/**
	*/
	class CBaseTable
	{
	public:
		CBaseTable(){}
		CBaseTable(int rowsize,int columnsize);
		/**
		* Release all resource, output error if some of the rows and columns are not properly release via ReleaseXX() functions
		*/
		virtual ~CBaseTable();
		/**
		* insert a new row at a given position, return the newly inserted row (it is an empty row)
		* the row is filled with null (using Resize())
		* will invalidate the rows which are larger than index and all columns.
		* One shall delete or release the input CBaseRowColumn. 
		*/
		virtual int InsertRow(int index);
		/**
		* insert an existing row at a given position
		* if the size of the input object does not match the CBaseTable's, the input object's 
		*  Resize() will be called to set the correct size;
		* will invalidate the rows which are larger than index and all columns.
		* One shall delete or release the input CBaseRowColumn. 
		*/
		//assign correct type of the input row, check if index valid,
		virtual int InsertRow(CBaseRowColumn& row, int index);
		/**
		* get a row, you should release it after use
		* @return: NULL if not exists.
		*/
		virtual CBaseRowColumn* GetRow(int index);
		//		virtual const CBaseRowColumn* GetRow(int index)const;
		/**
		* insert a new column at a given position, return the newly inserted column (it is an empty column)
		* the column is filled with null(using Resize())
		* will invalidate the columns which are larger than index and all rows.
		* One shall delete or release the input CBaseRowColumn. 
		*/
		virtual int InsertColumn(int index);
		/**
		* insert an existing column at a given position
		* if the size of the input object does not match the CBaseTable's, the input object's 
		*  Resize() will be called to set the correct size;
		* will invalidate the columns which are larger than index and all rows.
		* One shall delete or release the input CBaseRowColumn. 
		*/
		//assign correct type of the input column, check if index valid,
		virtual int InsertColumn(CBaseRowColumn& column, int index);
		/**
		* get the IObject at a given position
		* @return: NULL if not exists.
		*/
		virtual const IObject* GetAt(int row, int column)const;
		virtual IObject* GetAt(int row, int column);

		/**
		* Set the IObject at a given position
		* @return: -1 if failed
		*/
		virtual int SetAt(IObject& obj, int row, int column);

		//delete the column and its reference in the CBaseTable
		virtual void DeleteColumn(int index);
		//delete the row and its reference in the CBaseTable
		virtual void DeleteRow(int index);

		//delete all rows and columns
		virtual void Clear();

		virtual int NumberOfRows(){return m_nRowSize;}
		virtual int NumberOfColumns(){return m_nColumnSize;}
#ifdef _DEBUG
		static void test();
#endif
		friend class CBaseRowColumn;
	protected:
		Collection_Vector<CBaseRowColumn> m_Rows;//the rows generated from this table
		int m_nRowSize,m_nColumnSize;
	};

}

//any change in this table will make all CBaseRow and CBaseColumn object of it Invalid
/**
* Inserting rows and columns are very time consuming. Approximately O(n/2) for initial insert and followed by a O(n^2) Update operation.
* The best is to create the table with an initial size.
* This solution provides fast Get and Set operation, but slow Insert and Delete  operation
*/
// 	class CBaseTable
// 	{
// 	public:
// 		CBaseTable():m_bRowValid(true),m_bColumnValid(true){}
// 		CBaseTable(int rowsize,int columnsize);
// 		/**
// 		* Release all resource, output error if some of the rows and columns are not properly release via ReleaseXX() functions
// 		*/
// 		virtual ~CBaseTable();
// 		/**
// 		* insert a new row at a given position, return the newly inserted row (it is an empty row)
// 		* the row is filled with null (using Resize())
// 		* will invalidate the rows which are larger than index and all columns.
// 		* One shall delete or release the input CBaseRowColumn. 
// 		*/
// 		virtual int InsertRow(int index);
// 		/**
// 		* insert an existing row at a given position
// 		* if the size of the input object does not match the CBaseTable's, the input object's 
// 		*  Resize() will be called to set the correct size;
// 		* will invalidate the rows which are larger than index and all columns.
// 		* One shall delete or release the input CBaseRowColumn. 
// 		*/
// 		//assign correct type of the input row, check if index valid,
// 		virtual int InsertRow(CBaseRowColumn& row, int index);
// 		/**
// 		* get a row, you should release it after use
// 		* @return: NULL if not exists.
// 		*/
// 		virtual CBaseRowColumn* GetRow(int index);
// //		virtual const CBaseRowColumn* GetRow(int index)const;
// 		/**
// 		* insert a new column at a given position, return the newly inserted column (it is an empty column)
// 		* the column is filled with null(using Resize())
// 		* will invalidate the columns which are larger than index and all rows.
// 		* One shall delete or release the input CBaseRowColumn. 
// 		*/
// 		virtual int InsertColumn(int index);
// 		/**
// 		* insert an existing column at a given position
// 		* if the size of the input object does not match the CBaseTable's, the input object's 
// 		*  Resize() will be called to set the correct size;
// 		* will invalidate the columns which are larger than index and all rows.
// 		* One shall delete or release the input CBaseRowColumn. 
// 		*/
// 		//assign correct type of the input column, check if index valid,
// 		virtual int InsertColumn(CBaseRowColumn& column, int index);
// 		/**
// 		* get a column, you should release it after use
// 		* @return: NULL if not exists.
// 		*/
// 		virtual CBaseRowColumn* GetColumn(int index);
// //		virtual const CBaseRowColumn* GetColumn(int index)const;
// 		/**
// 		* get the IObject at a given position
// 		* @return: NULL if not exists.
// 		*/
// 		virtual const IObject* GetAt(int row, int column)const;
// 		virtual IObject* GetAt(int row, int column);
// 
// 		/**
// 		* Set the IObject at a given position
// 		* @return: -1 if failed
// 		*/
// 		virtual int SetAt(IObject& obj, int row, int column);
// 
// 		//delete the column and its reference in the CBaseTable
// 		virtual void DeleteColumn(int index);
// 		//delete the row and its reference in the CBaseTable
// 		virtual void DeleteRow(int index);
// 
// 		//delete all rows and columns
// 		virtual void Clear();
// 
// 		/**
// 		* Clean the row and column cache. It will make Update operation faster
// 		*/
// 		virtual void CleanCache();
// 		//update rows or columns
// 		virtual bool Update();
// 
// 		virtual int NumberOfRows(){return m_nRowSize;}
// 		virtual int NumberOfColumns(){return m_nColumnSize;}
// #ifdef _DEBUG
// 		static void test();
// #endif
// 		friend class CBaseRowColumn;
// 	protected:
// 		Collection_Map<CBaseRowColumn> m_Rows;//the rows generated from this table
// 		Collection_Map<CBaseRowColumn> m_Columns;//the columns generated from this table
// 		Collection_Vector<IObject> m_data;//pos=row*columnsize+column
// 		int m_nRowSize,m_nColumnSize;
// 		bool m_bRowValid;//marks all the rows as valid or invalid
// 		bool m_bColumnValid;//marks all the columns as valid or invalid
// 	};
