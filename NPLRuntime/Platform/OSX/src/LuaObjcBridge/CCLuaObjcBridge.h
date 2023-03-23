//-----------------------------------------------------------------------------
// Class: CCLuaObjcBridge.h
// Authors: big
// Emails: onedou@126.com
// CreateDate: 2023.3.23
//-----------------------------------------------------------------------------

#include <string>
#include <map>
#include <iostream>

namespace ParaEngine
{
    /**
     * Build a bridge between ObjectC and Lua script.
     * This mechanism make Lua and ObjectC call each other easily.
     */
    class LuaObjcBridge
    {
    public:
        static void nplActivate(const std::string &msg, const std::string &strNPLFileName);
    };
}
