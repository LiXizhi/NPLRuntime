package com.tatfook.paracraft.utils;

import java.util.Locale;

import android.content.Context;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

/*
 * DeviceInfoUtils.java
 *
 *  Created on: 2017-11-16
 *      Author: wangjie
 *
 *  Welcome attention to weixin public number get more info
 *
 *  WeiXin Public Number : ProgramAndroid
 *  微信公众号 ：程序员Android
 *
 */
public class DeviceUtil {

    /**
     * 获取设备宽度（px）
     *
     */
    public static int getDeviceWidth(Context context) {
        return context.getResources().getDisplayMetrics().widthPixels;
    }

    /**
     * 获取设备高度（px）
     */
    public static int getDeviceHeight(Context context) {
        return context.getResources().getDisplayMetrics().heightPixels;
    }

    /**
     * 获取设备的唯一标识， 需要 “android.permission.READ_Phone_STATE”权限
     */
    public static String getIMEI(Context context) {
//        TelephonyManager tm = (TelephonyManager) context
//                .getSystemService(Context.TELEPHONY_SERVICE);
//        String deviceId = tm.getDeviceId();
//        if (deviceId == null) {
//            return "UnKnown";
//        } else {
//            return deviceId;
//        }
        return "";
    }

    /**
     * 获取厂商名
     * **/
    public static String getDeviceManufacturer() {
        return android.os.Build.MANUFACTURER;
    }

    /**
     * 获取产品名
     * **/
    public static String getDeviceProduct() {
        return android.os.Build.PRODUCT;
    }

    /**
     * 获取手机品牌
     */
    public static String getDeviceBrand() {
        return android.os.Build.BRAND;
    }

    /**
     * 获取手机型号
     */
    public static String getDeviceModel() {
        return android.os.Build.MODEL;
    }

    /**
     * 获取手机主板名
     */
    public static String getDeviceBoard() {
        return android.os.Build.BOARD;
    }

    /**
     * 设备名
     * **/
    public static String getDeviceDevice() {
        return android.os.Build.DEVICE;
    }

    /**
     *
     *
     * fingerprit 信息
     * **/
    public static String getDeviceFubgerprint() {
        return android.os.Build.FINGERPRINT;
    }

    /**
     * 硬件名
     *
     * **/
    public static String getDeviceHardware() {
        return android.os.Build.HARDWARE;
    }

    /**
     * 主机
     *
     * **/
    public static String getDeviceHost() {
        return android.os.Build.HOST;
    }

    /**
     *
     * 显示ID
     * **/
    public static String getDeviceDisplay() {
        return android.os.Build.DISPLAY;
    }

    /**
     * ID
     *
     * **/
    public static String getDeviceId() {
        return android.os.Build.ID;
    }

    /**
     * 获取手机用户名
     *
     * **/
    public static String getDeviceUser() {
        return android.os.Build.USER;
    }

    /**
     * 获取手机 硬件序列号
     * **/
    public static String getDeviceSerial() {
        return android.os.Build.SERIAL;
    }

    /**
     * 获取手机Android 系统SDK
     *
     * @return
     */
    public static int getDeviceSDK() {
        return android.os.Build.VERSION.SDK_INT;
    }

    /**
     * 获取手机Android 版本
     *
     * @return
     */
    public static String getDeviceAndroidVersion() {
        return android.os.Build.VERSION.RELEASE;
    }

    /**
     * 获取当前手机系统语言。
     */
    public static String getDeviceDefaultLanguage() {
        return Locale.getDefault().getLanguage();
    }

    /**
     * 获取当前手机地区。
     */
    public static String getDeviceDefaultRegion() {
        return Locale.getDefault().getCountry();
    }

    /**
     * 获取当前系统上的语言列表(Locale列表)
     */
    public static String getDeviceSupportLanguage() {
        Log.e("wangjie", "Local:" + Locale.GERMAN);
        Log.e("wangjie", "Local:" + Locale.ENGLISH);
        Log.e("wangjie", "Local:" + Locale.US);
        Log.e("wangjie", "Local:" + Locale.CHINESE);
        Log.e("wangjie", "Local:" + Locale.TAIWAN);
        Log.e("wangjie", "Local:" + Locale.FRANCE);
        Log.e("wangjie", "Local:" + Locale.FRENCH);
        Log.e("wangjie", "Local:" + Locale.GERMANY);
        Log.e("wangjie", "Local:" + Locale.ITALIAN);
        Log.e("wangjie", "Local:" + Locale.JAPAN);
        Log.e("wangjie", "Local:" + Locale.JAPANESE);
        return Locale.getAvailableLocales().toString();
    }

    public static String getAndroidID(Context context) {
        String id = Settings.Secure.getString(
                context.getContentResolver(),
                Settings.Secure.ANDROID_ID
        );
        if ("9774d56d682e549c".equals(id)) return "";
        return id == null ? "" : id;
    }

    public static String getDeviceInfoJsonStr(Context context) {
        JSONObject obj = new JSONObject();
        try {
            String size = getDeviceWidth(context)+"x"+getDeviceHeight(context);
            obj.put("screenSize",size);
            obj.put("android_version",android.os.Build.VERSION.RELEASE);
            obj.put("android_sdk_version",android.os.Build.VERSION.SDK_INT);
            obj.put("manufacturer",android.os.Build.MANUFACTURER);
            obj.put("phone_model",android.os.Build.MODEL);
            obj.put("language",getDeviceDefaultLanguage());
            obj.put("region",getDeviceDefaultRegion());
            obj.put("sdcard",SDCardUtil.getStorageInfo(context, 0));
            obj.put("RAM",SDCardUtil.getRAMInfo(context));
            obj.put("androidid",getAndroidID(context));
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return obj.toString();
    }

    public static String getDeviceAllInfo(Context context) {

        return "\n1. IMEI: " + getIMEI(context)

                + "\n2. 设备宽度: " + getDeviceWidth(context)

                + "\n3. 设备高度: " + getDeviceHeight(context)

                + "\n4. 是否有内置SD卡: " + SDCardUtil.isSDCardMount()

                + "\n5. RAM 信息: " + SDCardUtil.getRAMInfo(context)

                + "\n6. 内部存储信息 " + SDCardUtil.getStorageInfo(context, 0)

                + "\n7. SD卡 信息: " + SDCardUtil.getStorageInfo(context, 1)

                + "\n10. 系统默认语言: " + getDeviceDefaultLanguage()

                + "\n11. 硬件序列号(设备名): " + android.os.Build.SERIAL

                + "\n12. 手机型号: " + android.os.Build.MODEL

                + "\n13. 生产厂商: " + android.os.Build.MANUFACTURER

                + "\n14. 手机Fingerprint标识: " + android.os.Build.FINGERPRINT

                + "\n15. Android 版本: " + android.os.Build.VERSION.RELEASE

                + "\n16. Android SDK版本: " + android.os.Build.VERSION.SDK_INT

                + "\n17. 安全patch 时间: " + android.os.Build.VERSION.SECURITY_PATCH

                + "\n19. 版本类型: " + android.os.Build.TYPE

                + "\n20. 用户名: " + android.os.Build.USER

                + "\n21. 产品名: " + android.os.Build.PRODUCT

                + "\n22. ID: " + android.os.Build.ID

                + "\n23. 显示ID: " + android.os.Build.DISPLAY

                + "\n24. 硬件名: " + android.os.Build.HARDWARE

                + "\n25. 产品名: " + android.os.Build.DEVICE

                + "\n26. Bootloader: " + android.os.Build.BOOTLOADER

                + "\n27. 主板名: " + android.os.Build.BOARD

                + "\n28. CodeName: " + android.os.Build.VERSION.CODENAME
                + "\n29. 语言支持: " + getDeviceSupportLanguage();

    }
}