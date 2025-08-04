/**
 * ParaEngine WebView for HarmonyOS
 * 对应Android项目中的ParaEngineWebView.java
 */

import webview from '@ohos.web.webview';

@Component
export struct ParaEngineWebView {
  private controller: webview.WebviewController = new webview.WebviewController();
  @State url: string = '';

  build() {
    Column() {
      Web({ src: this.url, controller: this.controller })
        .javaScriptAccess(true)
        .fileAccess(true)
        .imageAccess(true)
        .domStorageAccess(true)
        .geolocationAccess(true)
        .mediaPlayGestureAccess(true)
        .multiWindowAccess(false)
        .horizontalScrollBarAccess(false)
        .verticalScrollBarAccess(false)
        .password(true)
        .tableData(true)
        .wideViewModeAccess(false)
        // .webDebuggingAccess(false)  // 属性不存在，已注释
        .pinchSmooth(true)
        .allowWindowOpenMethod(true)
        .width('100%')
        .height('100%')
        .onPageBegin((event) => {
          console.info('WebView page begin: ' + event?.url);
        })
        .onPageEnd((event) => {
          console.info('WebView page end: ' + event?.url);
        })
        .onErrorReceive((event) => {
          console.error('WebView error: ' + event?.error?.getErrorInfo());
        })
    }
  }

  /**
   * 加载URL
   * @param url 网页地址
   */
  loadUrl(url: string): void {
    this.url = url;
  }

  /**
   * 执行JavaScript
   * @param script JavaScript代码
   */
  evaluateJavaScript(script: string): void {
    this.controller.runJavaScript(script);
  }

  /**
   * 返回上一页
   */
  goBack(): void {
    if (this.controller.accessBackward()) {
      this.controller.backward();
    }
  }

  /**
   * 前进到下一页
   */
  goForward(): void {
    if (this.controller.accessForward()) {
      this.controller.forward();
    }
  }

  /**
   * 刷新页面
   */
  reload(): void {
    this.controller.refresh();
  }
}
