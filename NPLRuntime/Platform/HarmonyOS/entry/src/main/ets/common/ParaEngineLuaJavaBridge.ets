/**
 * ParaEngine Lua Java Bridge for HarmonyOS
 * 对应Android项目中的ParaEngineLuaJavaBridge.java
 */

import hilog from '@ohos.hilog';
import { ParaEngineHelper } from './ParaEngineHelper';
import { PlatformBridge } from './PlatformBridge';

export class ParaEngineLuaJavaBridge {
  private static readonly TAG = 'ParaEngineLuaJavaBridge';

  /**
   * 调用Lua函数（对应Android的callLuaFunction）
   */
  static callLuaFunction(funcName: string, params: string): void {
    hilog.info(0x0000, ParaEngineLuaJavaBridge.TAG, 'Call Lua function: %{public}s', funcName);
    // TODO: 通过NAPI调用native层的Lua函数
  }

  /**
   * 从Lua端调用的方法（对应Android中@Keep标记的方法）
   */

  /**
   * 获取平台Bridge
   */
  static getPlatformBridge(): string {
    return JSON.stringify({
      deviceUtil: PlatformBridge.getDeviceUtil(),
      sdCardUtil: PlatformBridge.getSDCardUtil(),
      usbSerialUtil: PlatformBridge.getUSBSerialTransferUtil(),
      bluetoothUtil: PlatformBridge.getBluetoothUtil()
    });
  }

  /**
   * 保存图片到相册
   */
  static saveImageToGallery(imageData: string): void {
    ParaEngineHelper.saveImageToGallery(imageData).then((success: boolean) => {
      hilog.info(0x0000, ParaEngineLuaJavaBridge.TAG, 'Save image result: %{public}s', success.toString());
    });
  }

  /**
   * 打开编辑框
   */
  static openEditBox(placeholder: string, defaultText: string, maxLength: number, multiline: boolean): void {
    ParaEngineHelper.openEditBox({
      placeholder,
      defaultText,
      maxLength,
      multiline,
      callbackId: 'editbox_' + Date.now()
    });
  }

  /**
   * 显示WebView
   */
  static showWebView(url: string): void {
    ParaEngineHelper.showWebView(url);
  }

  /**
   * 隐藏WebView
   */
  static hideWebView(): void {
    ParaEngineHelper.hideWebView();
  }

  /**
   * 设置屏幕方向
   */
  static setScreenOrientation(orientation: number): void {
    ParaEngineHelper.setScreenOrientation(orientation);
  }

  /**
   * 退出应用
   */
  static exitApplication(): void {
    ParaEngineHelper.exitApp();
  }

  /**
   * 检查权限
   */
  static hasPermission(permission: string): boolean {
    return ParaEngineHelper.hasPermission(permission);
  }

  /**
   * 获取启动数据
   */
  static getLauncherData(): string {
    return ParaEngineHelper.getLauncherData();
  }

  /**
   * 处理用户隐私政策同意
   */
  static onAgreeUserPrivacy(): void {
    ParaEngineHelper.onAgreeUserPrivacy();
  }

  /**
   * 获取USB模式状态
   */
  static getUsbMode(): boolean {
    return ParaEngineHelper.checkUsbMode();
  }

  /**
   * 日志输出
   */
  static log(level: string, tag: string, message: string): void {
    const logLevel = level.toLowerCase();
    switch (logLevel) {
      case 'debug':
        hilog.debug(0x0000, tag, message);
        break;
      case 'info':
        hilog.info(0x0000, tag, message);
        break;
      case 'warn':
        hilog.warn(0x0000, tag, message);
        break;
      case 'error':
        hilog.error(0x0000, tag, message);
        break;
      default:
        hilog.info(0x0000, tag, message);
    }
  }
}
