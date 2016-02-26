#ifndef LUADEBUGDLL_H
#define LUADEBUGDLL_H

#include <windows.h>

struct lua_State;
#define VSLUA_EXPORT extern "C" __declspec(dllexport)

// error codes are defined in the enumeration below
enum VSLua_ErrorCodes
{
    VSL_NO_ERROR = 0,           // No error
    VSL_DLL_FUNC_MISSING,       // Some function in dll missing. see VSLua_GetLastErrorDesc()
    VSL_DEBUGGER_INIT_FAILED,   // Debugger initialization failed. see VSLua_GetLastErrorDesc() for more information
    VSL_DEBUGGER_NOT_ENABLED,   // VSLua->Enable/Disable menu option is set to Disable
    VSL_DEBUGGER_NOT_INITIALIZED,// Debugger from Visual Studio has not been initialized as yet
    VSL_LOAD_LIBRARY_FAILED,    // LoadLibrary(...) on one of the supplied custom/default lua dlls failed
    VSL_COM_LINK_FAILED,        // Could not establish a communication link with Visual Studio

    VSL_NUM_CODES,              // number of error codes
};


//////////////////////////////////////////////////////////////////////////
/// New addition for luaL_loadbuffer
//////////////////////////////////////////////////////////////////////////
VSLUA_EXPORT int VSLua_LoadBuffer(lua_State *ls, const char *buffer, 
                                  size_t size, const char *name);

//////////////////////////////////////////////////////////////////////////
/// Initialization related
//////////////////////////////////////////////////////////////////////////
// exported methods

VSLUA_EXPORT int VSLua_GetLastError();
VSLUA_EXPORT const char *VSLua_GetLastErrorDesc();

/************************************************************************/
/* 
    Name:           Init_VSLuaDebugger

    Parameters:     lua_State - Pointer to lua virtual machine instance that needs to be debugged
                    reserved - Reserved, must be 0

    Returns:        int - A value below or equal to 0 (<=0) means error, 
                    greater than 0 (>0) means success

    Description:    Initializes the lua debugger using the dlls specified in 
                    Tools->Options->VSLua->Compiler settings dialog. Note that
                    the user MUST take care that the debugged lua_State (1st parameter)
                    also belongs to the same DLL. Inconsistency can cause weird errors
                    that are hard to detect. 
*/
/************************************************************************/
VSLUA_EXPORT int Init_VSLuaDebugger(lua_State *ls, __int64 reserved);

/************************************************************************/
/* 
    Name:           Init_VSLuaDebugger_FromFile

    Parameters:     luaDLLPath - Path to the Lua DLL. 
                    lualibDLLPath - Path to the LuaLib DLL. If this is NULL then the 
                    Lua DLL is used for lib functions as well (as is the case with Lua 5.1).
                    lua_State - Pointer to lua virtual machine instance that needs to be debugged
                    reserved - Reserved, must be 0

    Returns:        int - A value below or equal to 0 (<=0) means error, 
                    greater than 0 (>0) means success

    Description:    Initializes the lua debugger using the dlls passed as a parameter. 
                    Note that the user MUST take care that the debugged lua_State (3rd parameter)
                    also belongs to the same DLL. Inconsistency can cause weird errors
                    that are hard to detect. 
*/
/************************************************************************/
VSLUA_EXPORT int Init_VSLuaDebugger_FromFile(const TCHAR *luaDLLPath, const TCHAR *lualibDLLPath, 
                                             lua_State *ls, __int64 reserved);

/************************************************************************/
/* 
    Name:           Init_VSLuaDebugger_FromModule

    Parameters:     hLuaDLL - Handle to the Lua DLL module.
                    hLuaLibDLL - Handle to the LuaLib DLL module. If this is NULL then the 
                    Lua DLL is used for lib functions as well (as is the case with Lua 5.1).
                    lua_State - Pointer to lua virtual machine instance that needs to be debugged
                    reserved - Reserved, must be 0

    Returns:        int - A value below or equal to 0 (<=0) means error, 
                    greater than 0 (>0) means success

    Description:    Initializes the lua debugger using the module handles passed as a parameter. 
                    Note that the user MUST take care that the debugged lua_State (3rd parameter)
                    also belongs to the same DLL. Inconsistency can cause weird errors
                    that are hard to detect. 
*/
/************************************************************************/
VSLUA_EXPORT int Init_VSLuaDebugger_FromModule(HMODULE hLuaDLL, HMODULE hLuaLibDLL, 
                                               lua_State *ls, __int64 reserved);

// Convenience macros
#ifdef USE_VSLUA

#   define vsl_LoadBuffer       VSLua_LoadBuffer
#   define vsl_Init             Init_VSLuaDebugger
#   define vsl_Init_FromFile    Init_VSLuaDebugger_FromFile
#   define vsl_Init_FromModule  Init_VSLuaDebugger_FromFile

#else

#   define vsl_LoadBuffer       luaL_loadbuffer
#   define vsl_Init         
#   define vsl_Init_FromFile 
#   define vsl_Init_FromModule  

#endif

#endif // LUADEBUGDLL_H

