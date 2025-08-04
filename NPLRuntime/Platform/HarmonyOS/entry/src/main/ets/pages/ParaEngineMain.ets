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
  private initNativeEngine(): void {
    // TODO: 调用native方法初始化ParaEngine
    console.info('Initializing ParaEngine native engine...');
    
    // 获取设备信息
    const deviceInfo = PlatformBridge.getDeviceUtil();
    console.info('Device info:', deviceInfo);
    
    // 模拟初始化完成
    setTimeout(() => {
      this.isLoading = false;
    }, 2000);
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
    console.info('ParaEngineMain onPageShow');
  }

  /**
   * 页面生命周期 - 隐藏
   */
  onPageHide(): void {
    console.info('ParaEngineMain onPageHide');
  }

  /**
   * 页面生命周期 - 销毁
   */
  onPageDestroy(): void {
    console.info('ParaEngineMain onPageDestroy');
  }
}
