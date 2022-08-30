package com.tatfook.paracraft.luabridge;

import android.app.Activity;

import com.tatfook.paracraft.ParaEngineActivity;
import com.tatfook.paracraft.utils.DeviceUtil;

public class PlatformBridge {
    public static native void onNativeCallback(long luaCbPtr,final String result);

    public static String call_native(final String key,final String jsonParam) {
        Activity act = ParaEngineActivity.getContext();
        String ret = "";
        if(key.equals("test")){
            ret = "aaa_111";
            ret = DeviceUtil.getDeviceInfoJsonStr(act);
        }
        if(key.equals("getDeviceInfo")){
            ret = DeviceUtil.getDeviceInfoJsonStr(act);
        }else if(key.equals("getAppInfo")){
            ret = DeviceUtil.getAppInfoJsonStr(act);
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
