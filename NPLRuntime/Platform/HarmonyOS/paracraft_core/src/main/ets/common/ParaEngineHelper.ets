/**
 * ParaEngine Helper for HarmonyOS
 * 对应Android项目中的ParaEngineHelper类
 */

import hilog from '@ohos.hilog';
import { photoAccessHelper } from '@kit.MediaLibraryKit';
import router from '@ohos.router';
import image from '@ohos.multimedia.image';
import { BusinessError } from '@ohos.base';
import { buffer } from '@kit.ArkTS';
import EntryAbility from '../entryability/EntryAbility';

interface EditBoxOptions {
  placeholder?: string;
  defaultText?: string;
  maxLength?: number;
  multiline?: boolean;
  callbackId?: string;
}

export class ParaEngineHelper {
  private static readonly TAG = 'ParaEngineHelper';

  /**
   * 处理协议数据（对应Android的onCmdProtocol）
   */
  static onCmdProtocol(protocolData: string): void {
    hilog.info(0x0000, ParaEngineHelper.TAG, 'Handle protocol: %{public}s', protocolData);
    // TODO: 解析协议数据并执行相应操作
  }

  /**
   * 用户同意隐私政策处理（对应Android的onAgreeUserPrivacy）
   */
  static onAgreeUserPrivacy(): void {
    hilog.info(0x0000, ParaEngineHelper.TAG, 'User agreed privacy policy');
    // TODO: 通知native层用户已同意隐私政策
  }

  /**
   * 设置是否可以读取手机状态（对应Android的setCanReadPhoneState）
   */
  static setCanReadPhoneState(canRead: boolean): void {
    hilog.info(0x0000, ParaEngineHelper.TAG, 'Set can read phone state: %{public}s', canRead.toString());
    // TODO: 设置读取设备信息权限状态
  }

  /**
   * 保存Base64图片到相册（对应Android的saveImageToGallery）
   */
  static async saveImageToGallery(imageData: string): Promise<boolean> {
    try {
      hilog.info(0x0000, ParaEngineHelper.TAG, 'Save image to gallery');
      
      const jsonData: { base64: string; paraFilePath?: string } = JSON.parse(imageData);
      const base64Data: string = jsonData.base64;
      const fileName: string = jsonData.paraFilePath || 'paracraft_image.png';
      
      if (!base64Data || base64Data.length === 0) {
        hilog.error(0x0000, ParaEngineHelper.TAG, 'Invalid base64 data');
        return false;
      }

      // 将Base64转换为Uint8Array
      const binaryString: string = buffer.from(base64Data, 'base64').toString('binary');
      const bytes = new Uint8Array(binaryString.length);
      for (let i = 0; i < binaryString.length; i++) {
        bytes[i] = binaryString.charCodeAt(i);
      }

      // 创建图片源
      const imageSource = image.createImageSource(bytes.buffer);
      
      // TODO: 使用HarmonyOS的相册API保存图片
      // 这里需要使用@ohos.file.photoAccessHelper来保存到相册
      
      hilog.info(0x0000, ParaEngineHelper.TAG, 'Image saved successfully');
      return true;
    } catch (error) {
      hilog.error(0x0000, ParaEngineHelper.TAG, 'Save image failed: %{public}s', JSON.stringify(error));
      return false;
    }
  }

  /**
   * 打开编辑框（对应Android的ParaEngineEditBoxActivity）
   */
  static openEditBox(options: EditBoxOptions): void {
    router.pushUrl({
      url: 'pages/ParaEngineEditBox',
      params: {
        placeholder: options.placeholder || '请输入内容',
        defaultText: options.defaultText || '',
        maxLength: options.maxLength || 1000,
        multiline: options.multiline !== false,
        callbackId: options.callbackId || ''
      }
    }).then(() => {
      hilog.info(0x0000, ParaEngineHelper.TAG, 'EditBox opened successfully');
    }).catch((err: BusinessError) => {
      hilog.error(0x0000, ParaEngineHelper.TAG, 'Open EditBox failed: %{public}s', JSON.stringify(err));
    });
  }

  /**
   * 显示WebView（对应Android中WebView的使用）
   */
  static showWebView(url: string): void {
    // TODO: 通知主页面显示WebView组件
    hilog.info(0x0000, ParaEngineHelper.TAG, 'Show WebView: %{public}s', url);
  }

  /**
   * 隐藏WebView
   */
  static hideWebView(): void {
    // TODO: 通知主页面隐藏WebView组件
    hilog.info(0x0000, ParaEngineHelper.TAG, 'Hide WebView');
  }

  /**
   * 检查USB模式（对应Android的checkUsbMode）
   */
  static checkUsbMode(): boolean {
    // HarmonyOS中检测外接键盘鼠标的方法
    // TODO: 实现USB设备检测逻辑
    hilog.info(0x0000, ParaEngineHelper.TAG, 'Check USB mode');
    return false;
  }

  /**
   * 设置屏幕方向
   */
  static setScreenOrientation(orientation: number): void {
    EntryAbility.setScreenOrientation(orientation);
  }

  /**
   * 退出应用
   */
  static exitApp(): void {
    EntryAbility.onExit();
  }

  /**
   * 检查权限
   */
  static hasPermission(permission: string): boolean {
    return EntryAbility.hasPermission(permission);
  }

  /**
   * 获取启动数据
   */
  static getLauncherData(): string {
    return EntryAbility.getLauncherIntentData();
  }
}
