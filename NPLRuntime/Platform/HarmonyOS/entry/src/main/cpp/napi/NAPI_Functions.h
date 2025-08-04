/**
 * NAPI Functions Header
 * 提供给ArkTS调用的Native接口声明
 */

#ifndef NAPI_FUNCTIONS_H
#define NAPI_FUNCTIONS_H

#include <napi/native_api.h>

namespace ParaEngine {

// 基础引擎接口
napi_value NAPI_SetSurface(napi_env env, napi_callback_info info);
napi_value NAPI_SetAssetManager(napi_env env, napi_callback_info info);
napi_value NAPI_InitParaEngine(napi_env env, napi_callback_info info);
napi_value NAPI_CleanupParaEngine(napi_env env, napi_callback_info info);

// 生命周期回调
napi_value NAPI_OnSurfaceCreated(napi_env env, napi_callback_info info);
napi_value NAPI_OnSurfaceChanged(napi_env env, napi_callback_info info);
napi_value NAPI_OnDrawFrame(napi_env env, napi_callback_info info);

// 输入事件处理
napi_value NAPI_OnTouch(napi_env env, napi_callback_info info);
napi_value NAPI_OnKey(napi_env env, napi_callback_info info);

// 应用生命周期
napi_value NAPI_OnPause(napi_env env, napi_callback_info info);
napi_value NAPI_OnResume(napi_env env, napi_callback_info info);
napi_value NAPI_OnDestroy(napi_env env, napi_callback_info info);

// 消息传递和脚本执行
napi_value NAPI_SendMessage(napi_env env, napi_callback_info info);
napi_value NAPI_CallLuaFunction(napi_env env, napi_callback_info info);
napi_value NAPI_RegisterArkTSCallback(napi_env env, napi_callback_info info);

// 文件操作
napi_value NAPI_ReadAssetFile(napi_env env, napi_callback_info info);
napi_value NAPI_WriteFile(napi_env env, napi_callback_info info);
napi_value NAPI_FileExists(napi_env env, napi_callback_info info);

// 设备信息
napi_value NAPI_GetDeviceInfo(napi_env env, napi_callback_info info);
napi_value NAPI_GetStoragePath(napi_env env, napi_callback_info info);
napi_value NAPI_GetResourcePath(napi_env env, napi_callback_info info);

// 平台功能
napi_value NAPI_ShowWebView(napi_env env, napi_callback_info info);
napi_value NAPI_HideWebView(napi_env env, napi_callback_info info);
napi_value NAPI_OpenEditBox(napi_env env, napi_callback_info info);
napi_value NAPI_SetScreenOrientation(napi_env env, napi_callback_info info);
napi_value NAPI_ExitApplication(napi_env env, napi_callback_info info);

// 权限和隐私
napi_value NAPI_HasPermission(napi_env env, napi_callback_info info);
napi_value NAPI_SaveImageToGallery(napi_env env, napi_callback_info info);

// 屏幕录制
napi_value NAPI_StartScreenRecording(napi_env env, napi_callback_info info);
napi_value NAPI_StopScreenRecording(napi_env env, napi_callback_info info);
napi_value NAPI_PauseScreenRecording(napi_env env, napi_callback_info info);
napi_value NAPI_ResumeScreenRecording(napi_env env, napi_callback_info info);
napi_value NAPI_GetScreenRecordingStatus(napi_env env, napi_callback_info info);

// 蓝牙功能
napi_value NAPI_IsBluetoothAvailable(napi_env env, napi_callback_info info);
napi_value NAPI_EnableBluetooth(napi_env env, napi_callback_info info);
napi_value NAPI_DisableBluetooth(napi_env env, napi_callback_info info);
napi_value NAPI_StartBluetoothScan(napi_env env, napi_callback_info info);
napi_value NAPI_StopBluetoothScan(napi_env env, napi_callback_info info);
napi_value NAPI_GetPairedDevices(napi_env env, napi_callback_info info);

// 网络功能
napi_value NAPI_IsNetworkAvailable(napi_env env, napi_callback_info info);
napi_value NAPI_GetNetworkType(napi_env env, napi_callback_info info);

// 日志和调试
napi_value NAPI_LogMessage(napi_env env, napi_callback_info info);

// 其他功能
napi_value NAPI_GetLauncherData(napi_env env, napi_callback_info info);
napi_value NAPI_OnAgreeUserPrivacy(napi_env env, napi_callback_info info);
napi_value NAPI_GetUsbMode(napi_env env, napi_callback_info info);

} // namespace ParaEngine

#endif // NAPI_FUNCTIONS_H