package com.tatfook.paracraft.luabridge;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.util.DisplayMetrics;
import android.util.Log;

import com.tatfook.paracraft.ParaEngineActivity;
import com.tatfook.paracraft.ParaEngineWebViewHelper;
import com.tatfook.paracraft.utils.DeviceUtil;

import org.json.JSONException;
import org.json.JSONObject;

public class PlatformBridge {
    public static native void onNativeCallback(long luaCbPtr,final String result);

    public static String call_native(final String key,final String jsonParam) {
        ParaEngineActivity act = ParaEngineActivity.getContext();
        String ret = "";
        if(key.equals("test")){
            ret = "aaa_111";
            ret = DeviceUtil.getDeviceInfoJsonStr(act);
        }
        if(key.equals("getDeviceInfo")){
            ret = DeviceUtil.getDeviceInfoJsonStr(act);
        }else if(key.equals("getAppInfo")){
            ret = DeviceUtil.getAppInfoJsonStr(act);
        }else if(key.equals("show_weview")){
//            int x=0,y=0;
//            int width = DeviceUtil.getDeviceWidth(act),height = DeviceUtil.getDeviceHeight(act);
//
//            float alpha = 1.0f;
//            String url = "";
//            boolean cleanCacheData = false;
//            boolean withTouchMask = false;//是否背景区域点击关闭
//            boolean ignoreCloseWhenClickBack = false;//忽略返回键
//            try {
//                JSONObject obj = new JSONObject(jsonParam);
//                x = obj.optInt("x",x);
//                y = obj.optInt("y",y);
//                width = obj.optInt("width",width);
//                height = obj.optInt("height",height);
//                url = obj.optString("url","");
//                alpha = (float)obj.optDouble("alpha",alpha);
//                withTouchMask = obj.optBoolean("withTouchMask",withTouchMask);
//                ignoreCloseWhenClickBack = obj.optBoolean("ignoreCloseWhenClickBack",ignoreCloseWhenClickBack);
//
//            }catch (JSONException e) {
//                e.printStackTrace();
//            }
//
//            if(url.isEmpty()){
//                return "";
//            }
//
//            int webViewId = ParaEngineWebViewHelper.createWebView(x,y,width,height);
//            ParaEngineWebViewHelper.setViewAlpha(webViewId,alpha);
//            ParaEngineWebViewHelper.loadUrl(webViewId,url,cleanCacheData);
//
//            ParaEngineWebViewHelper.setMaskVisible(withTouchMask);
//            ParaEngineWebViewHelper.SetIgnoreCloseWhenClickBack(webViewId,ignoreCloseWhenClickBack);
//
//            ret = webViewId+"";
        }else if(key.equals("close_weview")){

        }else if(key.equals("getChannelId")){
            ret = getChannelId(act);
        }else if(key.equals("onAgreeUserPrivacy")){
            act.onAgreeUserPrivacy();
        } else if (key.equals("getDevicePermission")) {
            String strPermission = "";
            try {
                JSONObject obj = new JSONObject(jsonParam);
                strPermission = obj.optString("permission");
            }catch (JSONException e){
                e.printStackTrace();
            }
            if (strPermission != null && strPermission != ""){
                ret = String.valueOf(ParaEngineActivity.HasPermission(strPermission));
            }
        }
        return ret;
    }

    public static void call_native_withCB(final String key,long luaCbPtr,final String jsonParam) {
        Activity act = ParaEngineActivity.getContext();
        String ret = "";
        if(key.equals("test")){
            ret = "aaa_222";
            onNativeCallback(luaCbPtr,ret);
        }

    }

    public static void showWebView(){

    }

    public static String getChannelId(Activity act) {
        Context ctx = act;

        String channelId = "";

        try {
            ApplicationInfo appInfo = ctx.getPackageManager().getApplicationInfo(ctx.getPackageName(), PackageManager.GET_META_DATA);
            Object temp = appInfo.metaData.get("CHANNEL_ID");
            if(temp!=null){
                Log.d("GetChannelIdFunction",">>>>>>>>>>>>>>> GetChannelIdFunction "+ temp.toString());
                channelId = temp.toString();
            }

            channelId = channelId.trim();
        } catch(Exception e) {
            Log.e("GetChannelIdFunction", e.getMessage(), e);
        }

        return channelId;
    }
}
