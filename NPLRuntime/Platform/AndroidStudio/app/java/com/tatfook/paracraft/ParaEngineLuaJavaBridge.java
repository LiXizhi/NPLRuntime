//-----------------------------------------------------------------------------
// ParaEngineLuaJavaBridge.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import java.lang.Thread;

public class ParaEngineLuaJavaBridge
{
	public static native int callLuaFunctionWithString(int luaFunctionId, String value);
    public static native int retainLuaFunction(int luaFunctionId);
    public static native int releaseLuaFunction(int luaFunctionId);
	public static native int nplActivate(String filePath, String msg);
}
