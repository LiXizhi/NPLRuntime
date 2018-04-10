package com.tatfook.paracraft;

public class ParaEngineLuaJavaBridge
{
	public static native int callLuaFunctionWithString(int luaFunctionId, String value);
    public static native int retainLuaFunction(int luaFunctionId);
    public static native int releaseLuaFunction(int luaFunctionId);
}