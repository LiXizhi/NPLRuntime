#pragma once
#include "AssetEntity.h"

struct sqlite3;

namespace ParaEngine
{
	//--------------------------------------------------------
	/// DataBaseEntity distinguish one template from other
	/// TODO: I should wrap the SQLite3 interface in DatabaseEntity.
	//--------------------------------------------------------
	struct DatabaseEntity : public AssetEntity
	{
	private:
		/// if this is NULL, it means that the database is closed, otherwise it is open
		sqlite3 * m_pDB;

	public:
		virtual AssetEntity::AssetType GetType(){ return AssetEntity::database; };
		/// name of the file name (*.db)
		string  sFileName;

	public:

		DatabaseEntity(const AssetKey& key)
			:AssetEntity(key)
		{
			m_pDB = NULL;
		}

		DatabaseEntity()
		{
			m_pDB = NULL;
		};

		virtual ~DatabaseEntity(){};

		virtual void Cleanup();

		/** open the database file, if it is not already open.
		* call this function whenever the database is needed.*/
		sqlite3* OpenDB();
		/** close the database file. */
		void CloseDB();

	};

	typedef AssetManager<DatabaseEntity> DatabaseAssetManager;
}