package com.tatfook.paracraft;

import android.util.Log;
import androidx.annotation.Keep;

/**
 * ParaEngine 配置类
 * 
 * 这个类提供了一个简单的接口来配置 ParaEngineActivity 的各种设置，
 * 特别是加载模式的配置。外部开发者可以在应用启动前使用这个类来设置首选项。
 * 
 * 使用示例：
 * <pre>
 * // 在 Application.onCreate() 或 MainActivity.onCreate() 之前调用
 * ParaEngineConfig.setLoadingMode(ParaEngineConfig.LOADING_MODE_JAVA);
 * 
 * // 或者使用链式调用
 * ParaEngineConfig.builder()
 *     .setLoadingMode(ParaEngineConfig.LOADING_MODE_JAVA)
 *     .setWebViewLoadingUrl("https://custom-loading-page.com")
 *     .apply();
 * </pre>
 */
@Keep
public class ParaEngineConfig {
    
    private static final String TAG = "ParaEngineConfig";
    
    // 加载模式常量
    public static final int LOADING_MODE_JAVA = 0;
    public static final int LOADING_MODE_WEBVIEW = 1;
    
    // 默认配置值
    private static int sLoadingMode = LOADING_MODE_WEBVIEW;
    private static String sWebViewLoadingUrl = "https://keepwork.com/api/raw/maisi/maisi/webgames/data/custom_loader1?isLoading=true";
    private static boolean sDebugMode = false;
    
    /**
     * 设置加载模式
     * @param mode LOADING_MODE_JAVA (0) 或 LOADING_MODE_WEBVIEW (1)
     */
    @Keep
    public static void setLoadingMode(int mode) {
        if (mode != LOADING_MODE_JAVA && mode != LOADING_MODE_WEBVIEW) {
            Log.w(TAG, "Invalid loading mode: " + mode + ", using default WebView mode");
            mode = LOADING_MODE_WEBVIEW;
        }
        
        sLoadingMode = mode;
        ParaEngineActivity.setDefaultLoadingMode(mode);
        Log.d(TAG, "Loading mode configured: " + (mode == LOADING_MODE_JAVA ? "Java UI" : "WebView"));
    }
    
    /**
     * 获取当前配置的加载模式
     * @return LOADING_MODE_JAVA (0) 或 LOADING_MODE_WEBVIEW (1)
     */
    @Keep
    public static int getLoadingMode() {
        return sLoadingMode;
    }
    
    /**
     * 设置WebView加载模式使用的URL
     * @param url WebView要加载的URL
     */
    @Keep
    public static void setWebViewLoadingUrl(String url) {
        if (url == null || url.trim().isEmpty()) {
            Log.w(TAG, "Invalid WebView loading URL, using default");
            return;
        }
        
        sWebViewLoadingUrl = url;
        Log.d(TAG, "WebView loading URL configured: " + url);
    }
    
    /**
     * 获取当前配置的WebView加载URL
     * @return WebView加载URL
     */
    @Keep
    public static String getWebViewLoadingUrl() {
        return sWebViewLoadingUrl;
    }
    
    /**
     * 设置调试模式
     * @param debug 是否启用调试模式
     */
    @Keep
    public static void setDebugMode(boolean debug) {
        sDebugMode = debug;
        Log.d(TAG, "Debug mode " + (debug ? "enabled" : "disabled"));
    }
    
    /**
     * 获取调试模式状态
     * @return 是否启用调试模式
     */
    @Keep
    public static boolean isDebugMode() {
        return sDebugMode;
    }
    
    /**
     * 重置所有配置为默认值
     */
    @Keep
    public static void resetToDefaults() {
        sLoadingMode = LOADING_MODE_WEBVIEW;
        sWebViewLoadingUrl = "https://keepwork.com/api/raw/maisi/maisi/webgames/data/custom_loader1?isLoading=true";
        sDebugMode = false;
        
        ParaEngineActivity.setDefaultLoadingMode(sLoadingMode);
        Log.d(TAG, "Configuration reset to defaults");
    }
    
    /**
     * 打印当前配置
     */
    @Keep
    public static void printCurrentConfig() {
        Log.i(TAG, "=== ParaEngine Configuration ===");
        Log.i(TAG, "Loading Mode: " + (sLoadingMode == LOADING_MODE_JAVA ? "Java UI" : "WebView"));
        Log.i(TAG, "WebView URL: " + sWebViewLoadingUrl);
        Log.i(TAG, "Debug Mode: " + sDebugMode);
        Log.i(TAG, "==============================");
    }
    
    /**
     * 配置构建器
     * 提供链式调用的配置方式
     */
    @Keep
    public static class Builder {
        private int loadingMode = sLoadingMode;
        private String webViewUrl = sWebViewLoadingUrl;
        private boolean debugMode = sDebugMode;
        
        /**
         * 设置加载模式
         */
        public Builder setLoadingMode(int mode) {
            this.loadingMode = mode;
            return this;
        }
        
        /**
         * 设置WebView加载URL
         */
        public Builder setWebViewLoadingUrl(String url) {
            this.webViewUrl = url;
            return this;
        }
        
        /**
         * 设置调试模式
         */
        public Builder setDebugMode(boolean debug) {
            this.debugMode = debug;
            return this;
        }
        
        /**
         * 应用配置
         */
        public void apply() {
            ParaEngineConfig.setLoadingMode(loadingMode);
            ParaEngineConfig.setWebViewLoadingUrl(webViewUrl);
            ParaEngineConfig.setDebugMode(debugMode);
            
            Log.d(TAG, "Configuration applied via Builder");
            if (debugMode) {
                printCurrentConfig();
            }
        }
    }
    
    /**
     * 创建配置构建器
     * @return Builder实例
     */
    @Keep
    public static Builder builder() {
        return new Builder();
    }
    
    // 快捷方法
    
    /**
     * 快速设置为Java UI模式
     */
    @Keep
    public static void useJavaUI() {
        setLoadingMode(LOADING_MODE_JAVA);
    }
    
    /**
     * 快速设置为WebView模式
     */
    @Keep
    public static void useWebView() {
        setLoadingMode(LOADING_MODE_WEBVIEW);
    }
    
    /**
     * 快速设置为WebView模式并指定URL
     */
    @Keep
    public static void useWebView(String url) {
        setWebViewLoadingUrl(url);
        setLoadingMode(LOADING_MODE_WEBVIEW);
    }
}