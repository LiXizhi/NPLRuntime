#pragma once

///////////////////////////////////////////////////////////////////////////
// forward declaration of some database class alias.

namespace ParaInfoCenter{
	class CICDBManager;
	class DBEntity;
	class CICRecordSet;
}
namespace ParaEngine{
	/** database entity*/
	typedef ParaInfoCenter::DBEntity DBEntity;
	/** database manager: please use static method */
	typedef ParaInfoCenter::CICDBManager CDBManager;
	/** a record set and stored procedure */
	typedef ParaInfoCenter::CICRecordSet CDBRecordSet;
}
