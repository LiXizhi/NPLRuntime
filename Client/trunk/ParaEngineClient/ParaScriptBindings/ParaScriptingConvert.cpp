#include "ParaEngine.h"
#include "ParaScriptingCommon.h"
#include "ParaScriptingGlobal.h"

extern "C"
{
#include <lua.h>
}

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include "Physics/PhysicsShape.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsDynamicsWorld.h"
#include "Physics/PhysicsConstraint.h"

#include <ParaScriptingConvert.h>

namespace ParaScripting
{
	_convert::_ReadLuaValue<bool, LUA_TBOOLEAN> ReadLuaBoolean;
	_convert::_ReadLuaValue<int, LUA_TNUMBER> ReadLuaInt;
	_convert::_ReadLuaValue<unsigned int, LUA_TNUMBER> ReadLuaUInt;
	_convert::_ReadLuaValue<short, LUA_TNUMBER> ReadLuaShort;
	_convert::_ReadLuaValue<float, LUA_TNUMBER> ReadLuaFloat;
	_convert::_ReadLuaValue<double, LUA_TNUMBER> ReadLuaDouble;
	_convert::_ReadLuaValue<DWORD, LUA_TNUMBER> ReadLuaDWORD;
	_convert::_ReadLuaValue<const char*, LUA_TSTRING> ReadLuaString;

	_convert::_ReadLuaArray<ParaEngine::PARAVECTOR3, _convert::_Object2Vector3> ReadLuaVector3Array;

	_convert::_ReadLuaTable<bool, _convert::_ReadLuaValue<bool, LUA_TBOOLEAN>> ReadLuaTableBoolean;
	_convert::_ReadLuaTable<int, _convert::_ReadLuaValue<int, LUA_TNUMBER>> ReadLuaTableInt;
	_convert::_ReadLuaTable<unsigned int, _convert::_ReadLuaValue<unsigned int, LUA_TNUMBER>> ReadLuaTableUInt;
	_convert::_ReadLuaTable<short, _convert::_ReadLuaValue<short, LUA_TNUMBER>> ReadLuaTableShort;
	_convert::_ReadLuaTable<float, _convert::_ReadLuaValue<float, LUA_TNUMBER>> ReadLuaTableFloat;
	_convert::_ReadLuaTable<double, _convert::_ReadLuaValue<double, LUA_TNUMBER>> ReadLuaTableDouble;
	_convert::_ReadLuaTable<unsigned long, _convert::_ReadLuaValue<unsigned long, LUA_TNUMBER>> ReadLuaTableDWORD;
	_convert::_ReadLuaTable<const char*, _convert::_ReadLuaValue<const char*, LUA_TSTRING>> ReadLuaTableString;
	_convert::_ReadLuaTable<ParaEngine::PARAVECTOR3, _convert::_Object2Vector3> ReadLuaTableVector3;
	_convert::_ReadLuaTable<ParaEngine::PARAMATRIX3x3, _convert::_Object2Matrix3x3> ReadLuaTableMatrix3x3;
	_convert::_ReadLuaTable<ParaEngine::CPhysicsShape*, _convert::_ReadLuaPhysicsShape> ReadLuaTablePhysicsShape;
	_convert::_ReadLuaTable<ParaEngine::CPhysicsTriangleMeshShape*, _convert::_ReadLuaPhysicsTriangleMeshShape> ReadLuaTablePhysicsTriangleMeshShape;
	_convert::_ReadLuaTable<ParaEngine::CPhysicsRigidBody*, _convert::_ReadLuaPhysicsRigidBody> ReadLuaTablePhysicsRigidBody;
	_convert::_ReadLuaTable<ParaEngine::CPhysicsConstraint*, _convert::_ReadLuaPhysicsConstraint> ReadLuaTablePhysicsConstraint;
	_convert::_ReadLuaTable<std::vector<ParaEngine::PARAVECTOR3>, _convert::_ReadLuaArray<ParaEngine::PARAVECTOR3, _convert::_Object2Vector3>> ReadLuaTableVector3Array;
	_convert::_ReadLuaTable<std::vector<WORD>, _convert::_ReadLuaArray<WORD, _convert::_ReadLuaValue<WORD, LUA_TNUMBER>>> ReadLuaTableWordArray;
}