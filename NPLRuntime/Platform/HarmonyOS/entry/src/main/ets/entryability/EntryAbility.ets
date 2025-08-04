import UIAbility from '@ohos.app.ability.UIAbility';
import hilog from '@ohos.hilog';
import window from '@ohos.window';
import Want from '@ohos.app.ability.Want';
import AbilityConstant from '@ohos.app.ability.AbilityConstant';
import abilityAccessCtrl, { Permissions } from '@ohos.abilityAccessCtrl';
import { BusinessError } from '@ohos.base';
import bundleManager from '@ohos.bundle.bundleManager';
import common from '@ohos.app.ability.common';

/**
 * ParaEngine Entry Ability for HarmonyOS
 * 对应Android项目中的ParaEngineActivity
 */
export default class EntryAbility extends UIAbility {
  private static sContext: EntryAbility | null = null;
  private windowStage: window.WindowStage | null = null;
  private isPermissionGranted: boolean = false;
  private savedWant: Want | null = null;

  static getContext(): EntryAbility | null {
    return EntryAbility.sContext;
  }

  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    EntryAbility.sContext = this;
    this.savedWant = want;
    hilog.info(0x0000, 'ParaEngine', '%{public}s', 'Ability onCreate');
    
    // 处理启动Intent数据
    this.handleLauncherIntent(want);
  }

  onDestroy(): void {
    hilog.info(0x0000, 'ParaEngine', '%{public}s', 'Ability onDestroy');
    EntryAbility.sContext = null;
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    this.windowStage = windowStage;
    hilog.info(0x0000, 'ParaEngine', '%{public}s', 'Ability onWindowStageCreate');

    // 请求权限
    this.requestPermissions().then(() => {
      this.initWindow(windowStage);
    }).catch((err: BusinessError) => {
      hilog.error(0x0000, 'ParaEngine', 'Request permissions failed: %{public}s', JSON.stringify(err));
      this.initWindow(windowStage);
    });
  }

  /**
   * 初始化窗口设置
   */
  private initWindow(windowStage: window.WindowStage): void {
    windowStage.getMainWindow().then((windowClass) => {
      // 设置全屏模式
      windowClass.setWindowLayoutFullScreen(true).then(() => {
        hilog.info(0x0000, 'ParaEngine', 'Set window layout full screen success');
      }).catch((err: BusinessError) => {
        hilog.error(0x0000, 'ParaEngine', 'Set window layout full screen failed: %{public}s', JSON.stringify(err));
      });

      // 设置横屏方向（对应Android的SCREEN_ORIENTATION_LANDSCAPE）
      windowClass.setPreferredOrientation(window.Orientation.LANDSCAPE).then(() => {
        hilog.info(0x0000, 'ParaEngine', 'Set landscape orientation success');
      }).catch((err: BusinessError) => {
        hilog.error(0x0000, 'ParaEngine', 'Set landscape orientation failed: %{public}s', JSON.stringify(err));
      });

      // 保持屏幕常亮（对应Android的FLAG_KEEP_SCREEN_ON）
      windowClass.setWindowKeepScreenOn(true).then(() => {
        hilog.info(0x0000, 'ParaEngine', 'Set keep screen on success');
      }).catch((err: BusinessError) => {
        hilog.error(0x0000, 'ParaEngine', 'Set keep screen on failed: %{public}s', JSON.stringify(err));
      });
    });

    // 加载主页面
    windowStage.loadContent('pages/ParaEngineMain', (err, data) => {
      if (err.code) {
        hilog.error(0x0000, 'ParaEngine', 'Failed to load content. Cause: %{public}s', JSON.stringify(err) ?? '');
        return;
      }
      hilog.info(0x0000, 'ParaEngine', 'Succeeded in loading content. Data: %{public}s', JSON.stringify(data) ?? '');
    });
  }

  /**
   * 请求必要权限
   */
  private async requestPermissions(): Promise<void> {
    const permissions: Array<Permissions> = [
      'ohos.permission.INTERNET',
      'ohos.permission.WRITE_MEDIA', // 替代WRITE_USER_STORAGE
      'ohos.permission.READ_MEDIA', // 替代READ_USER_STORAGE
      'ohos.permission.CAMERA',
      'ohos.permission.MICROPHONE',
      'ohos.permission.LOCATION', // 移除ACCESS_LOCATION
      'ohos.permission.USE_BLUETOOTH',
      'ohos.permission.DISCOVER_BLUETOOTH'
    ];

    try {
      const atManager = abilityAccessCtrl.createAtManager();
      const bundleInfo = await bundleManager.getBundleInfoForSelf(bundleManager.BundleFlag.GET_BUNDLE_INFO_WITH_APPLICATION);
      const tokenId = bundleInfo.appInfo.accessTokenId;

      await atManager.requestPermissionsFromUser(this.context, permissions);
      this.isPermissionGranted = true;
      hilog.info(0x0000, 'ParaEngine', 'Permissions granted successfully');
    } catch (err) {
      hilog.error(0x0000, 'ParaEngine', 'Request permissions failed: %{public}s', JSON.stringify(err));
      this.isPermissionGranted = false;
    }
  }

  /**
   * 处理启动Intent数据（对应Android的handleIntent）
   */
  private handleLauncherIntent(want: Want): void {
    if (want && want.uri) {
      const uri = want.uri;
      const scheme = uri.split(':')[0];
      
      hilog.info(0x0000, 'ParaEngine', 'Handle launcher intent: %{public}s', uri);
      
      // 处理不同的URL scheme
      if (scheme === 'paracraft' || scheme === 'papa' || scheme === 'palakaedu') {
        // 调用原生处理方法
        this.onCmdProtocol(uri);
      }
    }
  }

  /**
   * 处理协议数据（对应Android的ParaEngineHelper.onCmdProtocol）
   */
  private onCmdProtocol(protocolData: string): void {
    hilog.info(0x0000, 'ParaEngine', 'Protocol data received: %{public}s', protocolData);
    // TODO: 调用native方法处理协议数据
  }

  /**
   * 获取启动Intent数据（对应Android的getLauncherIntentData）
   */
  static getLauncherIntentData(): string {
    const context = EntryAbility.getContext();
    if (!context || !context.savedWant) {
      return '';
    }
    
    return context.savedWant.uri || '';
  }

  /**
   * 设置屏幕方向（对应Android的setScreenOrientation）
   */
  static setScreenOrientation(type: number): void {
    const context = EntryAbility.getContext();
    if (!context || !context.windowStage) {
      return;
    }

    context.windowStage.getMainWindow().then((windowClass) => {
      let orientation: window.Orientation;
      if (type === 1) {
        orientation = window.Orientation.PORTRAIT; // 竖屏
      } else {
        orientation = window.Orientation.LANDSCAPE; // 横屏
      }
      
      windowClass.setPreferredOrientation(orientation).then(() => {
        hilog.info(0x0000, 'ParaEngine', 'Set orientation success: %{public}d', type);
      }).catch((err: BusinessError) => {
        hilog.error(0x0000, 'ParaEngine', 'Set orientation failed: %{public}s', JSON.stringify(err));
      });
    });
  }

  /**
   * 检查权限（对应Android的HasPermission）
   */
  static hasPermission(permission: string): boolean {
    const context = EntryAbility.getContext();
    if (!context) {
      return false;
    }
    
    try {
      const atManager = abilityAccessCtrl.createAtManager();
      const grantStatus = atManager.checkAccessTokenSync(
        context.context.applicationInfo.accessTokenId,
        permission as Permissions
      );
      return grantStatus === abilityAccessCtrl.GrantStatus.PERMISSION_GRANTED;
    } catch (err) {
      hilog.error(0x0000, 'ParaEngine', 'Check permission failed: %{public}s', JSON.stringify(err));
      return false;
    }
  }

  /**
   * 退出应用（对应Android的onExit）
   */
  static onExit(): void {
    const context = EntryAbility.getContext();
    if (context) {
      context.context.terminateSelf();
    }
  }

  /**
   * 用户同意隐私政策后的处理（对应Android的onAgreeUserPrivacy）
   */
  onAgreeUserPrivacy(): void {
    hilog.info(0x0000, 'ParaEngine', 'User agreed privacy policy');
    // TODO: 调用native方法通知隐私政策同意
  }

  onWindowStageDestroy(): void {
    // Main window is destroyed, release UI related resources
    hilog.info(0x0000, 'ParaEngine', '%{public}s', 'Ability onWindowStageDestroy');
    this.windowStage = null;
  }

  onForeground(): void {
    // Ability has brought to foreground
    hilog.info(0x0000, 'ParaEngine', '%{public}s', 'Ability onForeground');
  }

  onBackground(): void {
    // Ability has back to background
    hilog.info(0x0000, 'ParaEngine', '%{public}s', 'Ability onBackground');
  }

  /**
   * 处理新的Intent（对应Android的onNewIntent）
   */
  onNewWant(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    hilog.info(0x0000, 'ParaEngine', '%{public}s', 'Ability onNewWant');
    this.handleLauncherIntent(want);
  }
}
