#pragma once

namespace ParaScripting
{
	namespace _convert
	{
		typedef std::function<bool(ParaEngine::ATTRIBUTE_CLASSID_TABLE classType)> CheckClassFunc;
	}

	inline IAttributeFields* Object2AttributeFields(const object& o, _convert::CheckClassFunc f)
	{
		if (type(o) != LUA_TUSERDATA)
			return nullptr;

		auto pAttObj = object_cast<ParaAttributeObject*>(o);
		if (!pAttObj || !pAttObj->IsValid())
			return nullptr;

		auto classId = pAttObj->GetClassID();
		if (f((ParaEngine::ATTRIBUTE_CLASSID_TABLE)classId))
			return pAttObj->m_pAttribute.get();
		else
			return nullptr;

	}

	inline IAttributeFields* Object2AttributeFields(const object& o, ParaEngine::ATTRIBUTE_CLASSID_TABLE classType)
	{
		return Object2AttributeFields(o, [classType](ParaEngine::ATTRIBUTE_CLASSID_TABLE thisType)->bool
		{
			return classType == thisType;
		});
	}

	template<class T>
	bool Object2PhysicsObject(const object& o, T*& out, _convert::CheckClassFunc f)
	{
		auto p = Object2AttributeFields(o, f);
		if (!p)
			return false;

		out = static_cast<T*>(p);
		return true;
	}


	namespace _convert
	{
		struct _Object2Vector3
		{
			bool operator() (const luabind::object& o, ParaEngine::PARAVECTOR3& output)
			{
				if (luabind::type(o) == LUA_TTABLE
					&& luabind::type(o[1]) == LUA_TNUMBER
					&& luabind::type(o[2]) == LUA_TNUMBER
					&& luabind::type(o[3]) == LUA_TNUMBER)
				{
					output.x = luabind::object_cast<float>(o[1]);
					output.y = luabind::object_cast<float>(o[2]);
					output.z = luabind::object_cast<float>(o[3]);

					return true;
				}
				else
				{
					return false;
				}
			}
		};

		struct _Object2Matrix3x3
		{
			bool operator() (const luabind::object& o, ParaEngine::PARAMATRIX3x3& output)
			{
				if (luabind::type(o) != LUA_TTABLE)
					return false;

				for (int x = 0; x < 3; x++)
					for (int y = 0; y < 3; y++)
					{
						int index = x * 3 + y + 1;
						if (luabind::type(o[index]) != LUA_TNUMBER)
							return false;

						output.m[x][y] = luabind::object_cast<float>(o[index]);
					}

				return true;
			}
		};


		struct _ReadLuaPhysicsShape
		{
			bool operator() (const object& o, ParaEngine::CPhysicsShape*& out)
			{
				return Object2PhysicsObject(o, out, [](ParaEngine::ATTRIBUTE_CLASSID_TABLE classType)
				{
					return (classType >= ParaEngine::ATTRIBUTE_CLASSID_CPhysicsShapeMin
						&& classType <= ParaEngine::ATTRIBUTE_CLASSID_CPhysicsShapeMax);
				});
			}
		};

		struct _ReadLuaPhysicsTriangleMeshShape
		{
			bool operator() (const object& o, ParaEngine::CPhysicsTriangleMeshShape*& out)
			{
				return Object2PhysicsObject(o, out, [](ParaEngine::ATTRIBUTE_CLASSID_TABLE classType)
				{
					return (classType >= ParaEngine::ATTRIBUTE_CLASSID_CPhysicsTriangleMeshShape);
				});
			}
		};

		struct _ReadLuaPhysicsRigidBody
		{
			bool operator() (const object& o, ParaEngine::CPhysicsRigidBody*& out)
			{
				return Object2PhysicsObject(o, out, [](ParaEngine::ATTRIBUTE_CLASSID_TABLE classType)
				{
					return classType == ParaEngine::ATTRIBUTE_CLASSID_CPhysicsRigidBody;
				});
			}
		};

		struct _ReadLuaPhysicsConstraint
		{
			bool operator() (const object& o, ParaEngine::CPhysicsConstraint*& out)
			{
				return Object2PhysicsObject(o, out, [](ParaEngine::ATTRIBUTE_CLASSID_TABLE classType)
				{
					return (classType >= ParaEngine::ATTRIBUTE_CLASSID_CPhysicsConstraintMin
						&& classType <= ParaEngine::ATTRIBUTE_CLASSID_CPhysicsConstraintMax);
				});
			}
		};

		template<class T, int luaType>
		struct _ReadLuaValue
		{
			bool operator()(const luabind::object& o, T& out)
			{
				if (luabind::type(o) != luaType)
					return false;

				out = luabind::object_cast<T>(o);

				return true;
			}
		};

		template<class T, class ConvertFunc>
		struct _ReadLuaArray
		{
			bool operator()(const luabind::object& o, std::vector<T>& out)
			{
				if (luabind::type(o) != LUA_TTABLE)
					return false;

				int index = 1;
				luabind::object v;
				while (luabind::type(v = o[index++]) != LUA_TNIL)
				{
					T value;
					if (!ConvertFunc()(v, value))
						return false;
					out.push_back(value);
				}

				return true;
			}
		};

		template<class T, class FuncType>
		struct _ReadLuaTable
		{
			bool operator()(const luabind::object& o
				, const char* key
				, T& out)
			{
				auto& sub = o[key];
				return FuncType()(sub, out);
			}
		};
	}


	inline bool Object2Vector3(const luabind::object& o, ParaEngine::PARAVECTOR3& output)
	{
		return _convert::_Object2Vector3()(o, output);
	}

	
	inline bool Object2Matrix3x3(const luabind::object& o, ParaEngine::PARAMATRIX3x3& output)
	{
		return _convert::_Object2Matrix3x3()(o, output);
	}
	

	inline void Vector32Object(const ParaEngine::PARAVECTOR3& v, luabind::object& o)
	{
		(o)[1] = (v).x; (o)[2] = (v).y; (o)[3] = (v).z;
	}

	inline void Matrix3x32Object(const ParaEngine::PARAMATRIX3x3& matrix, luabind::object& o)
	{
		for (int x = 0; x < 3; x++)
			for (int y = 0; y < 3; y++)
			{
				int index = x * 3 + y + 1;
				(o)[index] = (matrix).m[x][y];
			}
	}

	inline void Object2RigidBody(const luabind::object& o, ParaAttributeObject*& rb)
	{
		(rb) = nullptr;
		if (luabind::type((o)) == LUA_TUSERDATA)
		{
			auto f = luabind::object_cast<ParaAttributeObject*>((o));
			if (f->IsValid())
			{
				auto classId = f->GetClassID();
				if (classId == ATTRIBUTE_CLASSID_CPhysicsRigidBody)
				{
					(rb) = f;
				}
			}
		}
	}

	
	inline bool ReadLuaPhysicsShape(const object& o, ParaEngine::CPhysicsShape*& out)
	{
		return _convert::_ReadLuaPhysicsShape()(o, out);
	}

	inline bool ReadLuaPhysicsTriangleMeshShape(const object& o, ParaEngine::CPhysicsTriangleMeshShape*& out)
	{
		return _convert::_ReadLuaPhysicsTriangleMeshShape()(o, out);
	}


	inline bool ReadLuaPhysicsRigidBody(const object& o, ParaEngine::CPhysicsRigidBody*& out)
	{
		return _convert::_ReadLuaPhysicsRigidBody()(o, out);
	}

	inline bool ReadLuaPhysicsConstraint(const object& o, ParaEngine::CPhysicsConstraint*& out)
	{
		return _convert::_ReadLuaPhysicsConstraint()(o, out);
	}

	
	template<class T, int luaType>
	bool ReadLuaValue(const luabind::object& o, T& out)
	{
		return _convert::_ReadLuaValue<T, luaType>()(o, out);
	}

	template<class T, class ConvertFunc>
	bool ReadLuaArray(const luabind::object& o
		, std::vector<T>& out)
	{
		return _convert::_ReadLuaArray<T, ConvertFunc>()(o, out);
	}

	template<class T, class FuncType>
	bool ReadLuaTable(const luabind::object& o
		, const char* key
		, T& out)
	{
		return _convert::_ReadLuaTable<T, FuncType>()(o, key, out);
	}

	extern _convert::_ReadLuaValue<bool, LUA_TBOOLEAN> ReadLuaBoolean;
	extern _convert::_ReadLuaValue<int, LUA_TNUMBER> ReadLuaInt;
	extern _convert::_ReadLuaValue<unsigned int, LUA_TNUMBER> ReadLuaUInt;
	extern _convert::_ReadLuaValue<short, LUA_TNUMBER> ReadLuaShort;
	extern _convert::_ReadLuaValue<float, LUA_TNUMBER> ReadLuaFloat;
	extern _convert::_ReadLuaValue<double, LUA_TNUMBER> ReadLuaDouble;
	extern _convert::_ReadLuaValue<DWORD, LUA_TNUMBER> ReadLuaDWORD;
	extern _convert::_ReadLuaValue<const char*, LUA_TSTRING> ReadLuaString;

	extern _convert::_ReadLuaArray<ParaEngine::PARAVECTOR3, _convert::_Object2Vector3> ReadLuaVector3Array;

	extern _convert::_ReadLuaTable<bool, _convert::_ReadLuaValue<bool, LUA_TBOOLEAN>> ReadLuaTableBoolean;
	extern _convert::_ReadLuaTable<int, _convert::_ReadLuaValue<int, LUA_TNUMBER>> ReadLuaTableInt;
	extern _convert::_ReadLuaTable<unsigned int, _convert::_ReadLuaValue<unsigned int, LUA_TNUMBER>> ReadLuaTableUInt;
	extern _convert::_ReadLuaTable<short, _convert::_ReadLuaValue<short, LUA_TNUMBER>> ReadLuaTableShort;
	extern _convert::_ReadLuaTable<float, _convert::_ReadLuaValue<float, LUA_TNUMBER>> ReadLuaTableFloat;
	extern _convert::_ReadLuaTable<double, _convert::_ReadLuaValue<double, LUA_TNUMBER>> ReadLuaTableDouble;
	extern _convert::_ReadLuaTable<unsigned long, _convert::_ReadLuaValue<unsigned long, LUA_TNUMBER>> ReadLuaTableDWORD;
	extern _convert::_ReadLuaTable<const char*, _convert::_ReadLuaValue<const char*, LUA_TSTRING>> ReadLuaTableString;
	extern _convert::_ReadLuaTable<ParaEngine::PARAVECTOR3, _convert::_Object2Vector3> ReadLuaTableVector3;
	extern _convert::_ReadLuaTable<ParaEngine::PARAMATRIX3x3, _convert::_Object2Matrix3x3> ReadLuaTableMatrix3x3;
	extern _convert::_ReadLuaTable<ParaEngine::CPhysicsShape*, _convert::_ReadLuaPhysicsShape> ReadLuaTablePhysicsShape;
	extern _convert::_ReadLuaTable<ParaEngine::CPhysicsTriangleMeshShape*, _convert::_ReadLuaPhysicsTriangleMeshShape> ReadLuaTablePhysicsTriangleMeshShape;
	extern _convert::_ReadLuaTable<ParaEngine::CPhysicsRigidBody*, _convert::_ReadLuaPhysicsRigidBody> ReadLuaTablePhysicsRigidBody;
	extern _convert::_ReadLuaTable<ParaEngine::CPhysicsConstraint*, _convert::_ReadLuaPhysicsConstraint> ReadLuaTablePhysicsConstraint;

	extern _convert::_ReadLuaTable<std::vector<ParaEngine::PARAVECTOR3>, _convert::_ReadLuaArray<ParaEngine::PARAVECTOR3, _convert::_Object2Vector3>> ReadLuaTableVector3Array;
	extern _convert::_ReadLuaTable<std::vector<WORD>, _convert::_ReadLuaArray<WORD, _convert::_ReadLuaValue<WORD, LUA_TNUMBER>>> ReadLuaTableWordArray;
}