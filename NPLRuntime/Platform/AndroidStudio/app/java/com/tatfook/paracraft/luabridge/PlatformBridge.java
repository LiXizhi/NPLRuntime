package com.tatfook.paracraft.luabridge;

public class PlatformBridge {
    public static native void onNativeCallback(long luaCbPtr,final String result);

    public static String call_native(final String key,final String jsonParam) {
        String ret = "";
        if(key.equals("test")){
            ret = "aaa_111";
        }
        return ret;
    }

    public static void call_native_withCB(final String key,long luaCbPtr,final String jsonParam) {
        String ret = "";
        if(key.equals("test")){
            ret = "aaa_222";
            onNativeCallback(luaCbPtr,ret);
        }

    }
}
