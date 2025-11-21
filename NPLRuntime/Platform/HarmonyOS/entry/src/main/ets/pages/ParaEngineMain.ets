/**
 * ParaEngine Main Page for HarmonyOS
 * 对应Android项目中ParaEngineActivity的主界面
 */

// import { XComponent } from '@ohos.arkui.advanced.XComponent';
// import { XComponent } from '@ohos.arkui.XComponent';
// 暂时注释掉XComponent导入，使用基础组件替代
// import { XComponent } from '@ohos.arkui.XComponent';
import { ParaEngineWebView } from '../common/ParaEngineWebView';
import { PlatformBridge } from '../common/PlatformBridge';
import hilog from '@ohos.hilog';
import { BusinessError } from '@ohos.base';
import resourceManager from '@ohos.resourceManager';
import i18n from '@ohos.i18n';

// 导入native模块
import libentry from 'libparacraft.so';

// 定义 WebView 参数接口
interface WebViewParams {
  url: string;
}

@Entry
@Component
struct ParaEngineMain {
  @State isLoading: boolean = true;
  @State showWebView: boolean = false;
  @State webViewUrl: string = '';
  // private xComponentController: XComponentController = new XComponentController();

  build() {
    Stack() {
      // 3D渲染组件 (对应Android的GLSurfaceView)
      // 暂时使用基础组件替代XComponent
      Column() {
        Text('ParaEngine 3D渲染区域')
          .fontSize(16)
          .fontColor(Color.White)
          .textAlign(TextAlign.Center)
      }
      .width('100%')
      .height('100%')
      .backgroundColor(Color.Black)
      .justifyContent(FlexAlign.Center)
      .alignItems(HorizontalAlign.Center)
      .onAppear(() => {
        console.info('ParaEngine render area appeared');
        this.isLoading = false;
        // 初始化native引擎
        this.initNativeEngine();
      })

      // WebView组件 (对应Android的ParaEngineWebView)
      if (this.showWebView) {
        ParaEngineWebView({ url: this.webViewUrl })
          .width('100%')
          .height('100%')
          .zIndex(10)
      }

      // 启动加载界面
      if (this.isLoading) {
        Column() {
          Image($r('app.media.app_icon'))
            .width(120)
            .height(120)
            .margin({ bottom: 20 })

          Text('帕拉卡3D虚拟社区学习软件')
            .fontSize(24)
            .fontColor(Color.White)
            .fontWeight(FontWeight.Bold)
            .margin({ bottom: 10 })

          Text('正在启动...')
            .fontSize(16)
            .fontColor(Color.Gray)

          LoadingProgress()
            .width(50)
            .height(50)
            .margin({ top: 20 })
            .color(Color.Blue)
        }
        .width('100%')
        .height('100%')
        .backgroundColor(Color.Black)
        .justifyContent(FlexAlign.Center)
        .alignItems(HorizontalAlign.Center)
        .zIndex(100)
      }
    }
    .width('100%')
    .height('100%')
  }

  /**
   * 初始化原生引擎
   */
  private async initNativeEngine(): Promise<void> {
    try {
      hilog.info(0x0000, 'ParaEngineMain', 'Initializing ParaEngine native engine...');
      
      // 获取设备信息
      const deviceInfo = PlatformBridge.getDeviceUtil();
      hilog.info(0x0000, 'ParaEngineMain', 'Device info: %{public}s', deviceInfo);
      
      // 1. 设置资源管理器
      await this.setupAssetManager();
      
      // 2. 初始化ParaEngine核心
      this.initParaEngineCore();
      
      // 3. 设置渲染表面
      this.setupRenderSurface();
      
      // 4. 完成初始化
      this.completeInitialization();
      
    } catch (error) {
      hilog.error(0x0000, 'ParaEngineMain', 'Failed to initialize ParaEngine: %{public}s', JSON.stringify(error));
      this.handleInitializationError(error as BusinessError);
    }
  }

  /**
   * 设置资源管理器
   */
  private async setupAssetManager(): Promise<void> {
    try {
      // 获取资源管理器
      const context = getContext(this);
      const resMgr = context.resourceManager;
      
      // 调用native方法设置资源管理器
      libentry.setAssetManager(resMgr);
      hilog.info(0x0000, 'ParaEngineMain', 'Asset manager set successfully');
    } catch (error) {
      hilog.error(0x0000, 'ParaEngineMain', 'Failed to setup asset manager: %{public}s', JSON.stringify(error));
      throw new Error(`Failed to setup asset manager: ${error}`);
    }
  }

  /**
   * 初始化ParaEngine核心
   */
  private initParaEngineCore(): void {
    try {
      // 调用native方法初始化ParaEngine
      libentry.initParaEngine();
      hilog.info(0x0000, 'ParaEngineMain', 'ParaEngine core initialized successfully');
    } catch (error) {
      hilog.error(0x0000, 'ParaEngineMain', 'Failed to initialize ParaEngine core: %{public}s', JSON.stringify(error));
      throw new Error(`Failed to initialize ParaEngine core: ${error}`);
    }
  }

  /**
   * 设置渲染表面
   */
  private setupRenderSurface(): void {
    try {
      // 调用native方法创建渲染表面
      libentry.onSurfaceCreated();
      
      // 设置表面大小（使用默认屏幕尺寸）
      // TODO: 获取实际屏幕尺寸
      const screenWidth = 1080;
      const screenHeight = 1920;
      libentry.onSurfaceChanged(screenWidth, screenHeight);
      
      hilog.info(0x0000, 'ParaEngineMain', 'Render surface setup completed: %{public}dx%{public}d', screenWidth, screenHeight);
    } catch (error) {
      hilog.error(0x0000, 'ParaEngineMain', 'Failed to setup render surface: %{public}s', JSON.stringify(error));
      throw new Error(`Failed to setup render surface: ${error}`);
    }
  }

  /**
   * 完成初始化
   */
  private completeInitialization(): void {
    try {
      // 注册ArkTS回调
      libentry.registerArkTSCallback(this);
      
      // 设置初始化完成状态
      setTimeout(() => {
        this.isLoading = false;
        hilog.info(0x0000, 'ParaEngineMain', 'ParaEngine initialization completed successfully');
      }, 1000);
    } catch (error) {
      hilog.error(0x0000, 'ParaEngineMain', 'Failed to complete initialization: %{public}s', JSON.stringify(error));
      throw new Error(`Failed to complete initialization: ${error}`);
    }
  }

  getCurrentLanguage(): string {
    try {
      const lang = i18n.getSystemLanguage().toLowerCase();
      if (lang.startsWith('zh')) return 'zh';
      if (lang.startsWith('en')) return 'en';
      if (lang.startsWith('fr')) return 'fr';
      if (lang.startsWith('it')) return 'it';
      if (lang.startsWith('de')) return 'de';
      if (lang.startsWith('es')) return 'es';
      if (lang.startsWith('ru')) return 'ru';
      if (lang.startsWith('nl')) return 'nl';
      if (lang.startsWith('ko')) return 'ko';
      if (lang.startsWith('ja')) return 'ja';
      if (lang.startsWith('hu')) return 'hu';
      if (lang.startsWith('pt')) return 'pt';
      if (lang.startsWith('ar')) return 'ar';
      if (lang.startsWith('nb') || lang.startsWith('no')) return 'nb';
      if (lang.startsWith('pl')) return 'pl';
      if (lang.startsWith('tr')) return 'tr';
      if (lang.startsWith('uk')) return 'uk';
      if (lang.startsWith('ro')) return 'ro';
      if (lang.startsWith('bg')) return 'bg';
      return 'en';
    } catch (error) {
      return 'en';
    }
  }

  /**
   * 处理初始化错误
   */
  private handleInitializationError(error: BusinessError): void {
    hilog.error(0x0000, 'ParaEngineMain', 'ParaEngine initialization failed with error: %{public}s', error.message);
    
    // 显示错误信息给用户
    setTimeout(() => {
      this.isLoading = false;
      // TODO: 显示错误对话框或错误页面
    }, 500);
  }

  /**
   * 显示WebView
   */
  showWebViewComponent(url: string): void {
    this.webViewUrl = url;
    this.showWebView = true;
  }

  /**
   * 隐藏WebView
   */
  hideWebViewComponent(): void {
    this.showWebView = false;
    this.webViewUrl = '';
  }

  /**
   * 页面生命周期 - 显示
   */
  onPageShow(): void {
    hilog.info(0x0000, 'ParaEngineMain', 'onPageShow');
    try {
      // 恢复native引擎
      libentry.onResume();
    } catch (error) {
      hilog.error(0x0000, 'ParaEngineMain', 'Failed to resume ParaEngine: %{public}s', JSON.stringify(error));
    }
  }

  /**
   * 页面生命周期 - 隐藏
   */
  onPageHide(): void {
    hilog.info(0x0000, 'ParaEngineMain', 'onPageHide');
    try {
      // 暂停native引擎
      libentry.onPause();
    } catch (error) {
      hilog.error(0x0000, 'ParaEngineMain', 'Failed to pause ParaEngine: %{public}s', JSON.stringify(error));
    }
  }

  /**
   * 页面生命周期 - 销毁
   */
  onPageDestroy(): void {
    hilog.info(0x0000, 'ParaEngineMain', 'onPageDestroy');
    try {
      // 销毁native引擎
      libentry.onDestroy();
      libentry.cleanupParaEngine();
    } catch (error) {
      hilog.error(0x0000, 'ParaEngineMain', 'Failed to cleanup ParaEngine: %{public}s', JSON.stringify(error));
    }
  }

  /**
   * 处理来自native层的回调
   */
  onNativeCallback(method: string, params: string): void {
    hilog.info(0x0000, 'ParaEngineMain', 'Native callback: %{public}s, params: %{public}s', method, params);
    
    switch (method) {
      case 'showWebView':
        try {
          const data: WebViewParams = JSON.parse(params) as WebViewParams;
          this.showWebViewComponent(data.url);
        } catch (error) {
          hilog.error(0x0000, 'ParaEngineMain', 'Failed to parse showWebView params: %{public}s', JSON.stringify(error));
        }
        break;
        
      case 'hideWebView':
        this.hideWebViewComponent();
        break;
        
      default:
        hilog.warn(0x0000, 'ParaEngineMain', 'Unknown native callback method: %{public}s', method);
        break;
    }
  }
}
