

class WebViewItem {
    constructor(key, domId, reuseIframe) {
        this.m_key = key;
        this.m_id = domId;
        this.m_iframe = reuseIframe || document.createElement("iframe");
        this.m_iframe.id = this.m_id;
        this.m_iframe.setAttribute("data-webview-id", key);
        this.m_iframe.style.display = "none";
        this.m_iframe.style.position = "fixed";
        this.m_iframe.style.border = "none";
        this.m_iframe.style.zIndex = "1000";
        this.m_iframe.style.backgroundColor = "#ffffff";
        this.m_iframe.allow = "camera; microphone; autoplay; clipboard-read; clipboard-write; allow-downloads";
        if (!reuseIframe) {
            document.body.appendChild(this.m_iframe);
        }
        try {
            if (this.m_iframe.contentWindow) {
                this.m_iframe.contentWindow.paracraft_platform = "emscripten";
            }
        } catch (e) {
        }
        this.m_url = "";
        this.m_fullscreen = false;
        this.m_iframe_loaded = false;
        this.m_iframe_loaded_callbacks = [];
        this.m_iframe_loaded_timer = undefined;
        this.m_x = 0;
        this.m_y = 0;
        this.m_width = 0;
        this.m_height = 0;
    }

    SetTransparent(transparent) {
        this.m_transparent = !!transparent;
        this.m_iframe.style.backgroundColor = this.m_transparent ? "transparent" : "#ffffff";
    }

    Show() {
        this.m_iframe.style.display = "block";
    }

    Hide() {
        this.m_iframe.style.display = "none";
    }

    ClearLoadTimer() {
        if (this.m_iframe_loaded_timer) {
            clearInterval(this.m_iframe_loaded_timer);
            this.m_iframe_loaded_timer = undefined;
        }
    }

    Destroy() {
        this.ClearLoadTimer();
        if (this.m_iframe && this.m_iframe.parentNode) {
            this.m_iframe.parentNode.removeChild(this.m_iframe);
        }
    }

    SetPosition(x, y, width, height) {
        const dpr = window.devicePixelRatio || 1;
        if (!this.m_fullscreen) {
            if (width <= 0 || height <= 0) {
                this.m_iframe.style.left = "auto";
                this.m_iframe.style.top = "auto";
                this.m_iframe.style.right = "0px";
                this.m_iframe.style.bottom = "0px";
                this.m_iframe.style.width = "4px";
                this.m_iframe.style.height = "4px";
                this.m_iframe.style.opacity = "0.01";
                this.m_iframe.style.pointerEvents = "none";
                this.m_iframe.style.zIndex = "999";
                this.m_iframe.width = 4;
                this.m_iframe.height = 4;
            } else {
                const cssLeft = x / dpr;
                const cssTop = y / dpr;
                const cssW = Math.max(1, width / dpr);
                const cssH = Math.max(1, height / dpr);
                this.m_iframe.style.right = "auto";
                this.m_iframe.style.bottom = "auto";
                this.m_iframe.style.left = cssLeft + "px";
                this.m_iframe.style.top = cssTop + "px";
                this.m_iframe.style.width = cssW + "px";
                this.m_iframe.style.height = cssH + "px";
                this.m_iframe.style.opacity = "1";
                this.m_iframe.style.pointerEvents = "auto";
                this.m_iframe.style.zIndex = "1000";
                this.m_iframe.width = Math.round(cssW);
                this.m_iframe.height = Math.round(cssH);
            }
        }
        this.m_x = x;
        this.m_y = y;
        this.m_width = width;
        this.m_height = height;
    }

    _resolveRouteUrl(rawUrl) {
        if (!rawUrl) return "";
        let url = String(rawUrl).trim();
        if (!url) return "";
        url = url.replace(/^https?:\/\/(?:www\.)?keepwork\.com(?=\/|\?|#|$)/i, "https://cross.keepwork.com");
        try {
            const parsed = new URL(url);
            if (parsed.protocol === "npl:" && window.NplWebserverBridge) {
                const nplPath = "/" + parsed.hostname + parsed.pathname;
                return window.NplWebserverBridge.Route(nplPath + parsed.search + parsed.hash);
            }
            const localOriginBase = (window.WEBPARACRAFT_ORIGIN || (window.location.origin + "/")).replace(/\/?$/, "/");
            const isWebParacraftHost = /^(?:webparacraft|emscripten)\.keepwork\.com$/i.test(parsed.hostname)
                || ((parsed.hostname === "127.0.0.1" || parsed.hostname === "localhost") && /^\/npl\/webparacraft\//i.test(parsed.pathname));
            if (isWebParacraftHost) {
                const relPath = parsed.pathname.replace(/^\/npl\/webparacraft\//i, "").replace(/^\/+/, "");
                if (/^(?:webpage\/|camera\/|about_blank\.html$)/i.test(relPath)) {
                    return localOriginBase + relPath + parsed.search + parsed.hash;
                }
            }
            const isLoopback = parsed.protocol === "http:" && (parsed.hostname === "127.0.0.1" || parsed.hostname === "localhost");
            if (isLoopback && parsed.port !== window.location.port && window.NplWebserverBridge) {
                const match = parsed.pathname.match(/^\/webserver\/([^/]+)(\/.*)?$/);
                const path = match ? (match[2] || "/") : parsed.pathname;
                return window.NplWebserverBridge.Route(path + parsed.search + parsed.hash);
            }
        } catch (e) {
        }
        return url;
    }

    SetUrl(rawUrl) {
        const url = this._resolveRouteUrl(rawUrl);
        const is_about_blank = !url || url === "about:blank" || url.endsWith("/about_blank.html");
        if (is_about_blank) {
            this.m_url = "about:blank";
            this.ClearLoadTimer();
            this.m_iframe_loaded = false;
            this.m_iframe_loaded_callbacks = [];
            if (this.m_iframe.getAttribute("src") && this.m_iframe.getAttribute("src") !== "about:blank") {
                this.m_iframe.src = "about:blank";
            }
            this.Hide();
            return;
        }

        if (this.m_url === url) return;
        this.m_url = url;

        const hashIdx = url.indexOf("#");
        const beforeHash = hashIdx >= 0 ? url.substring(0, hashIdx) : url;
        const hashPart = hashIdx >= 0 ? url.substring(hashIdx) : "";
        const arg_index = beforeHash.indexOf("?");
        const finalSrc = beforeHash + (arg_index >= 0 ? "&" : "?") + "platform=emscripten&hash=" + (new Date()).getTime().toString() + hashPart;

        this.m_iframe.src = finalSrc;
        this.m_iframe_loaded = false;
        this.m_iframe_loaded_callbacks = [];
        this.ClearLoadTimer();

        const self = this;
        let attempts = 0;
        this.m_iframe_loaded_timer = setInterval(function () {
            attempts++;
            if (attempts > 300) {
                self.ClearLoadTimer();
                return;
            }
            try {
                if (self.m_iframe && self.m_iframe.contentWindow) {
                    self.m_iframe.contentWindow.postMessage({
                        cmd: "load",
                        is_paracraft_message: true,
                        platform: "emscripten"
                    }, "*");
                }
            } catch (e) {
            }
        }, 100);
    }

    OnIFrameMsg(msg) {
        if (!msg || typeof msg !== "object" || !msg.is_paracraft_message) return;

        const cmd = msg.cmd;
        if (cmd == "load") {
            if (this.m_iframe_loaded) return;
            this.ClearLoadTimer();
            this.m_iframe_loaded = true;
            for (let i = 0; i < this.m_iframe_loaded_callbacks.length; i++) {
                const callback = this.m_iframe_loaded_callbacks[i];
                callback();
            }
            this.m_iframe_loaded_callbacks = [];
            console.log("===========webview load url:", this.m_iframe.src);
        }
        else if (cmd == "enter_fullscreen") {
            this.m_fullscreen = true;
            this.m_iframe.style.left = "0px";
            this.m_iframe.style.top = "0px";
            this.m_iframe.style.width = window.innerWidth + "px";
            this.m_iframe.style.height = window.innerHeight + "px";
            this.m_iframe.style.opacity = "1";
            this.m_iframe.style.pointerEvents = "auto";
            this.m_iframe.style.zIndex = "1000";
            this.m_iframe.width = window.innerWidth;
            this.m_iframe.height = window.innerHeight;
        }
        else if (cmd == "exit_fullscreen") {
            this.m_fullscreen = false;
            this.SetPosition(this.m_x, this.m_y, this.m_width, this.m_height);
        }
        else if (cmd == "PostMessage") {
            if (typeof window.SendMsgToEmscripten === "function") {
                window.SendMsgToEmscripten(msg);
            }
        }
    }

    EmitIFrameMsg(msg) {
        const self = this;
        if (!self.m_iframe_loaded) {
            return self.m_iframe_loaded_callbacks.push(function () {
                self.EmitIFrameMsg(msg);
            });
        }
        if (self.m_iframe && self.m_iframe.contentWindow) {
            self.m_iframe.contentWindow.postMessage({
                is_paracraft_message: true,
                cmd: "PostMessage",
                platform: "emscripten",
                filename: msg.to_filename,
                msg: msg.params,
            }, "*");
        }
    }
}

class WebView {
    constructor() {
        this.m_id = "webview";
        this.m_instances = Object.create(null);
        this.m_primaryKey = null;
        this.m_primaryItem = new WebViewItem("default", "webview", null);
        this.m_iframe = this.m_primaryItem.m_iframe;

        const self = this;
        window.addEventListener("message", function (event) {
            self.OnIFrameMsg(event.data, event.source);
        }, false);
        const notifyUserGesture = function () {
            for (const k in self.m_instances) {
                if (Object.prototype.hasOwnProperty.call(self.m_instances, k)) {
                    const item = self.m_instances[k];
                    if (item && item.m_iframe && item.m_iframe.contentWindow) {
                        try {
                            item.m_iframe.contentWindow.postMessage({ type: "paracraft_user_gesture" }, "*");
                        } catch (e) {}
                    }
                }
            }
        };
        window.addEventListener("pointerdown", notifyUserGesture, { passive: true });
        window.addEventListener("keydown", notifyUserGesture, { passive: true });
        window.addEventListener("resize", function () {
            for (const k in self.m_instances) {
                if (Object.prototype.hasOwnProperty.call(self.m_instances, k)) {
                    const item = self.m_instances[k];
                    if (item.m_fullscreen && item.m_iframe) {
                        item.m_iframe.style.width = window.innerWidth + "px";
                        item.m_iframe.style.height = window.innerHeight + "px";
                        item.m_iframe.width = window.innerWidth;
                        item.m_iframe.height = window.innerHeight;
                    }
                }
            }
        });
    }

    _getOrCreateItem(id, createIfMissing) {
        const key = String(id || "default");
        if (this.m_instances[key]) {
            return this.m_instances[key];
        }
        if (!createIfMissing) {
            if (this.m_primaryKey === null && key === "default") {
                return this.m_primaryItem;
            }
            return null;
        }
        if (this.m_primaryKey === null) {
            this.m_primaryKey = key;
            this.m_primaryItem.m_key = key;
            this.m_primaryItem.m_iframe.setAttribute("data-webview-id", key);
            this.m_instances[key] = this.m_primaryItem;
            this.m_iframe = this.m_primaryItem.m_iframe;
            return this.m_primaryItem;
        }
        const safeDomId = "webview_" + key.replace(/[^a-zA-Z0-9_-]/g, "_");
        const item = new WebViewItem(key, safeDomId, null);
        this.m_instances[key] = item;
        this.m_iframe = item.m_iframe;
        return item;
    }

    Show() {
        this.m_primaryItem.Show();
    }

    Hide() {
        this.m_primaryItem.Hide();
    }

    Destroy() {
        for (const k in this.m_instances) {
            if (Object.prototype.hasOwnProperty.call(this.m_instances, k)) {
                this.m_instances[k].Destroy();
            }
        }
        this.m_instances = Object.create(null);
        this.m_primaryKey = null;
    }

    SetPosition(x, y, width, height) {
        this.m_primaryItem.SetPosition(x, y, width, height);
    }

    SetUrl(url) {
        this.m_primaryItem.SetUrl(url);
    }

    OnIFrameMsg(msg, sourceWindow) {
        if (!msg || typeof msg !== "object" || !msg.is_paracraft_message) return;
        for (const k in this.m_instances) {
            if (Object.prototype.hasOwnProperty.call(this.m_instances, k)) {
                const item = this.m_instances[k];
                if (sourceWindow && item.m_iframe && item.m_iframe.contentWindow === sourceWindow) {
                    item.OnIFrameMsg(msg);
                    return;
                }
            }
        }
        this.m_primaryItem.OnIFrameMsg(msg);
    }

    EmitIFrameMsg(msg) {
        if (msg && msg.id && this.m_instances[msg.id]) {
            this.m_instances[msg.id].EmitIFrameMsg(msg);
            return;
        }
        let sent = false;
        for (const k in this.m_instances) {
            if (Object.prototype.hasOwnProperty.call(this.m_instances, k)) {
                this.m_instances[k].EmitIFrameMsg(msg);
                sent = true;
            }
        }
        if (!sent) {
            this.m_primaryItem.EmitIFrameMsg(msg);
        }
    }

    GetMsgName() {
        return "script/apps/Aries/Creator/Game/NplBrowser/NplBrowserPlugin.lua";
    }

    OnMsg(msg) {
        if (!msg || typeof msg !== "object") return;
        var cmd = msg.cmd;
        // 消息通知逻辑
        if (cmd == "PostMessage") {
            return this.EmitIFrameMsg(msg);
        }

        var scaleX = msg.scale_x || 1;
        var scaleY = msg.scale_y || 1;
        var x = Math.floor((msg.x || 0) * scaleX);
        var y = Math.floor((msg.y || 0) * scaleY);
        var width = Math.floor((msg.width || 0) * scaleX);
        var height = Math.floor((msg.height || 0) * scaleY);
        var url = msg.url || "";

        if (cmd == "Start") {
            const item = this._getOrCreateItem(msg.id, true);
            item.SetTransparent(msg.transparent);
            item.SetPosition(x, y, width, height);
            item.SetUrl(url);
            if (url && url !== "about:blank" && !url.endsWith("/about_blank.html")) {
                item.Show();
            }
        }
        else if (cmd == "Show") {
            const item = this._getOrCreateItem(msg.id, !!msg.visible);
            if (!item) return;
            if (msg.visible) {
                if (width > 0 && height > 0) {
                    item.SetPosition(x, y, width, height);
                }
                if (url && !item.m_url) {
                    item.SetUrl(url);
                }
                if (item.m_url && item.m_url !== "about:blank") {
                    item.Show();
                }
            }
            else {
                if (url && (url === "about:blank" || url.endsWith("/about_blank.html"))) {
                    item.SetUrl(url);
                }
                item.Hide();
            }
        }
        else if (cmd == "ChangePosSize") {
            const item = this._getOrCreateItem(msg.id, false);
            if (item) {
                item.SetPosition(x, y, width, height);
            }
        }
        else if (cmd == "Open") {
            const item = this._getOrCreateItem(msg.id, true);
            if (typeof msg.transparent === "boolean") {
                item.SetTransparent(msg.transparent);
            }
            item.SetUrl(url);
            if (typeof msg.width === "number" && typeof msg.height === "number") {
                item.SetPosition(x, y, width, height);
            }
            if (msg.visible === false) {
                item.Hide();
            } else if (url && url !== "about:blank" && !url.endsWith("/about_blank.html")) {
                item.Show();
            }
        }
        else if (cmd == "Quit" || cmd == "Close") {
            const key = String(msg.id || "default");
            const item = this.m_instances[key];
            if (item) {
                if (item === this.m_primaryItem) {
                    item.SetUrl("about:blank");
                    item.Hide();
                    delete this.m_instances[key];
                    this.m_primaryKey = null;
                } else {
                    item.Destroy();
                    delete this.m_instances[key];
                }
            }
        }
    }
};

window.WebViewInstance = new WebView();

function hostInnerWidth() {
    if (typeof window.getNativeInnerWidth === "function") return window.getNativeInnerWidth();
    return document.documentElement.clientWidth;
}

function hostInnerHeight() {
    if (typeof window.getNativeInnerHeight === "function") return window.getNativeInnerHeight();
    return document.documentElement.clientHeight;
}

var NPLB_ICON_REFRESH = '<svg viewBox="0 0 16 16" aria-hidden="true"><path d="M3.2 8a4.8 4.8 0 0 1 8.2-3.3M13 4.2v3h-3" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/><path d="M12.8 8a4.8 4.8 0 0 1-8.2 3.3M3 11.8v-3h3" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/></svg>';
var NPLB_ICON_GO = '<svg viewBox="0 0 16 16" aria-hidden="true"><path d="M3 8h9M9 4l4 4-4 4" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/></svg>';
var NPLB_ICON_COPY = '<svg viewBox="0 0 16 16" aria-hidden="true"><rect x="5" y="5" width="8" height="9" rx="1.2" fill="none" stroke="currentColor" stroke-width="1.5"/><rect x="3" y="2" width="8" height="9" rx="1.2" fill="none" stroke="currentColor" stroke-width="1.5"/></svg>';
var NPLB_ICON_FLOAT = '<svg viewBox="0 0 16 16" aria-hidden="true"><rect x="2" y="5" width="9" height="9" rx="1.2" fill="none" stroke="currentColor" stroke-width="1.5"/><path d="M8 3h5v5M13 3L8 8" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round"/></svg>';
var NPLB_ICON_DOCK = '<svg viewBox="0 0 16 16" aria-hidden="true"><rect x="1.5" y="2.5" width="13" height="11" rx="1.2" fill="none" stroke="currentColor" stroke-width="1.5"/><rect x="9" y="4" width="4" height="8" fill="currentColor"/></svg>';

class NplBrowserPopup {
    constructor() {
        this.STORAGE_KEY = "webparacraft.nplcodewiki";
        this.m_url = "";
        this.m_docked = false;
        this.m_rect = null;
        this.m_dockWidth = 0;
        this.m_dragging = false;
        this.m_resizing = false;
        this.m_bridgeRetryTimer = null;
        this._loadState();
        this._injectStyle();
        this._build();
        this.Hide();
        var self = this;
        window.addEventListener("resize", function (ev) {
            if (!ev.isTrusted) return;
            self._applyLayout();
        });
    }

    _defaultRect() {
        var w = Math.min(720, Math.max(360, hostInnerWidth() - 48));
        var h = Math.min(560, Math.max(280, hostInnerHeight() - 72));
        return {
            left: Math.max(24, hostInnerWidth() - w - 24),
            top: 48,
            width: w,
            height: h,
        };
    }

    _loadState() {
        this.m_rect = this._defaultRect();
        this.m_dockWidth = Math.min(560, Math.floor(hostInnerWidth() * 0.42));
        this.m_docked = true;
        try {
            var raw = localStorage.getItem(this.STORAGE_KEY);
            if (!raw) return;
            var data = JSON.parse(raw);
            if (data && typeof data === "object") {
                if (typeof data.docked === "boolean") {
                    this.m_docked = data.docked;
                }
                if (data.rect && typeof data.rect.width === "number") {
                    this.m_rect = data.rect;
                }
                if (typeof data.dockWidth === "number" && data.dockWidth > 240) {
                    this.m_dockWidth = data.dockWidth;
                }
            }
        } catch (e) {
        }
    }

    _saveState() {
        try {
            localStorage.setItem(this.STORAGE_KEY, JSON.stringify({
                docked: this.m_docked,
                rect: this.m_rect,
                dockWidth: this.m_dockWidth,
            }));
        } catch (e) {
        }
    }

    _injectStyle() {
        if (document.getElementById("nplb-style")) return;
        var style = document.createElement("style");
        style.id = "nplb-style";
        style.textContent = [
            "#nplb-panel{position:fixed;z-index:10050;display:none;flex-direction:column;background:#1e1e1e;color:#eee;border:1px solid #444;box-shadow:0 8px 28px rgba(0,0,0,.45);min-width:280px;min-height:200px;overflow:hidden;}",
            "#nplb-panel.nplb-visible{display:flex;}",
            "#nplb-panel.nplb-docked{box-shadow:none;border-top:none;border-right:none;border-bottom:none;}",
            "#nplb-title{height:36px;flex:0 0 36px;display:flex;align-items:center;gap:6px;padding:0 8px 0 8px;background:#2d2d2d;cursor:move;user-select:none;}",
            "#nplb-panel.nplb-docked #nplb-title{cursor:default;}",
            "#nplb-title .nplb-url{flex:1;min-width:0;height:24px;margin:0;border:1px solid #555;border-radius:3px;background:#1a1a1a;color:#ddd;font:12px/22px sans-serif;padding:0 8px;outline:none;cursor:text;user-select:text;}",
            "#nplb-title .nplb-url:focus{border-color:#888;background:#111;}",
            "#nplb-title .nplb-status{font:12px/1 sans-serif;color:#8cdc8c;white-space:nowrap;}",
            "#nplb-title button{flex:0 0 auto;height:24px;width:24px;padding:0;border:1px solid #555;border-radius:3px;background:#3a3a3a;color:#eee;cursor:pointer;display:inline-flex;align-items:center;justify-content:center;}",
            "#nplb-title button:hover{background:#4a4a4a;}",
            "#nplb-title button.nplb-close{font-size:16px;line-height:22px;}",
            "#nplb-title button svg{width:14px;height:14px;fill:currentColor;display:block;}",
            "#nplb-frame{flex:1 1 auto;width:100%;border:0;background:#fff;}",
            "#nplb-panel.nplb-busy #nplb-frame{pointer-events:none;}",
            ".nplb-grip{position:absolute;z-index:2;}",
            ".nplb-grip-w{left:0;top:36px;bottom:0;width:6px;cursor:ew-resize;}",
            ".nplb-grip-e{right:0;top:36px;bottom:0;width:6px;cursor:ew-resize;}",
            ".nplb-grip-s{left:0;right:0;bottom:0;height:6px;cursor:ns-resize;}",
            ".nplb-grip-se{right:0;bottom:0;width:12px;height:12px;cursor:nwse-resize;}",
            "#nplb-panel.nplb-docked .nplb-grip-e,#nplb-panel.nplb-docked .nplb-grip-s,#nplb-panel.nplb-docked .nplb-grip-se{display:none;}",
        ].join("");
        document.head.appendChild(style);
    }

    _build() {
        var self = this;
        var panel = document.createElement("div");
        panel.id = "nplb-panel";
        panel.innerHTML = [
            '<div id="nplb-title">',
            '<input class="nplb-url" type="text" spellcheck="false" autocomplete="off" aria-label="NPL Code Wiki URL">',
            '<span class="nplb-status"></span>',
            '<button type="button" class="nplb-go" title="打开"></button>',
            '<button type="button" class="nplb-refresh" title="刷新"></button>',
            '<button type="button" class="nplb-dock" title="浮动"></button>',
            '<button type="button" class="nplb-copy" title="复制"></button>',
            '<button type="button" class="nplb-close" title="关闭">×</button>',
            "</div>",
            '<iframe id="nplb-frame" allow="clipboard-read; clipboard-write; allow-downloads"></iframe>',
            '<div class="nplb-grip nplb-grip-w" data-edge="w"></div>',
            '<div class="nplb-grip nplb-grip-e" data-edge="e"></div>',
            '<div class="nplb-grip nplb-grip-s" data-edge="s"></div>',
            '<div class="nplb-grip nplb-grip-se" data-edge="se"></div>',
        ].join("");
        document.body.appendChild(panel);
        this.m_panel = panel;
        this.m_iframe = panel.querySelector("#nplb-frame");
        this.m_urlEl = panel.querySelector(".nplb-url");
        this.m_statusEl = panel.querySelector(".nplb-status");
        this.m_goBtn = panel.querySelector(".nplb-go");
        this.m_dockBtn = panel.querySelector(".nplb-dock");
        this.m_copyBtn = panel.querySelector(".nplb-copy");
        this.m_refreshBtn = panel.querySelector(".nplb-refresh");
        this.m_copyBtn.innerHTML = NPLB_ICON_COPY;
        this.m_goBtn.innerHTML = NPLB_ICON_GO;
        this.m_refreshBtn.innerHTML = NPLB_ICON_REFRESH;
        this._refreshDockBtn();

        window.addEventListener("keydown", function (e) {
            if (e.target !== self.m_urlEl) return;
            e.stopImmediatePropagation();
            if (e.key && e.key.length === 1 && !e.ctrlKey && !e.metaKey && !e.altKey && !e.isComposing) {
                e.preventDefault();
                var start = self.m_urlEl.selectionStart;
                var end = self.m_urlEl.selectionEnd;
                self.m_urlEl.setRangeText(e.key, start, end, "end");
                self.m_urlEl.dispatchEvent(new Event("input", { bubbles: true }));
            } else if (e.key === "Enter") {
                e.preventDefault();
                self._go(self.m_urlEl.value, false);
            } else if (e.key === "Escape") {
                e.preventDefault();
                self.m_urlEl.value = self.m_url || "";
                self.m_urlEl.blur();
            }
        }, true);
        window.addEventListener("keypress", function (e) {
            if (e.target === self.m_urlEl) e.stopImmediatePropagation();
        }, true);
        window.addEventListener("keyup", function (e) {
            if (e.target === self.m_urlEl) e.stopImmediatePropagation();
        }, true);

        panel.querySelector(".nplb-close").addEventListener("click", function (e) {
            e.stopPropagation();
            self.Hide();
        });
        panel.querySelector(".nplb-copy").addEventListener("click", function (e) {
            e.stopPropagation();
            self._copyUrl(self.m_urlEl.value.trim() || self.m_url, true);
        });
        this.m_goBtn.addEventListener("click", function (e) {
            e.stopPropagation();
            self._go(self.m_urlEl.value, false);
        });
        this.m_refreshBtn.addEventListener("click", function (e) {
            e.stopPropagation();
            self._go(self.m_urlEl.value, true);
        });
        this.m_urlEl.addEventListener("mousedown", function (e) {
            e.stopPropagation();
        });
        this.m_urlEl.addEventListener("keypress", function (e) {
            e.stopPropagation();
        });
        this.m_urlEl.addEventListener("compositionstart", function (e) {
            e.stopPropagation();
        });
        this.m_urlEl.addEventListener("compositionend", function (e) {
            e.stopPropagation();
        });
        this.m_urlEl.addEventListener("keydown", function (e) {
            e.stopPropagation();
            if (e.key === "Enter") {
                e.preventDefault();
                self._go(self.m_urlEl.value, false);
            } else if (e.key === "Escape") {
                e.preventDefault();
                self.m_urlEl.value = self.m_url || "";
                self.m_urlEl.blur();
            }
        });
        this.m_dockBtn.addEventListener("click", function (e) {
            e.stopPropagation();
            if (self.m_docked) self._undock();
            else self._dock();
        });

        var title = panel.querySelector("#nplb-title");
        title.addEventListener("mousedown", function (e) {
            if (e.button !== 0) return;
            if (e.target && e.target.closest && (e.target.closest("button") || e.target.closest("input"))) return;
            if (self.m_docked) return;
            e.preventDefault();
            self._beginDrag(e);
        });

        var grips = panel.querySelectorAll(".nplb-grip");
        for (var i = 0; i < grips.length; i++) {
            (function (grip) {
                grip.addEventListener("mousedown", function (e) {
                    if (e.button !== 0) return;
                    e.preventDefault();
                    e.stopPropagation();
                    self._beginResize(e, grip.getAttribute("data-edge"));
                });
            })(grips[i]);
        }
    }

    _refreshDockBtn() {
        if (!this.m_dockBtn) return;
        if (this.m_docked) {
            this.m_dockBtn.title = "浮动";
            this.m_dockBtn.innerHTML = NPLB_ICON_FLOAT;
        } else {
            this.m_dockBtn.title = "右侧";
            this.m_dockBtn.innerHTML = NPLB_ICON_DOCK;
        }
    }

    _setBusy(busy) {
        if (busy) this.m_panel.classList.add("nplb-busy");
        else this.m_panel.classList.remove("nplb-busy");
    }

    _beginDrag(e) {
        var self = this;
        if (this.m_docked) return;
        this.m_dragging = true;
        this._setBusy(true);
        var startX = e.clientX;
        var startY = e.clientY;
        var orig = {
            left: this.m_rect.left,
            top: this.m_rect.top,
        };
        function onMove(ev) {
            self.m_rect.left = orig.left + (ev.clientX - startX);
            self.m_rect.top = orig.top + (ev.clientY - startY);
            self._clampRect();
            self._applyLayout();
        }
        function onUp(ev) {
            document.removeEventListener("mousemove", onMove);
            document.removeEventListener("mouseup", onUp);
            self.m_dragging = false;
            self._setBusy(false);
            if (ev.clientX > hostInnerWidth() - 48) {
                self._dock();
            } else {
                self._saveState();
            }
        }
        document.addEventListener("mousemove", onMove);
        document.addEventListener("mouseup", onUp);
    }

    _beginResize(e, edge) {
        var self = this;
        this.m_resizing = true;
        this._setBusy(true);
        var startX = e.clientX;
        var startY = e.clientY;
        var orig = {
            left: this.m_rect.left,
            top: this.m_rect.top,
            width: this.m_docked ? this.m_dockWidth : this.m_rect.width,
            height: this.m_rect.height,
        };
        function onMove(ev) {
            var dx = ev.clientX - startX;
            var dy = ev.clientY - startY;
            if (self.m_docked) {
                if (edge === "w") {
                    self.m_dockWidth = Math.max(280, Math.min(hostInnerWidth() - 80, orig.width - dx));
                }
            } else {
                if (edge.indexOf("w") >= 0) {
                    var width = Math.max(280, orig.width - dx);
                    self.m_rect.left = orig.left + orig.width - width;
                    self.m_rect.width = width;
                }
                if (edge.indexOf("e") >= 0) {
                    self.m_rect.width = Math.max(280, orig.width + dx);
                }
                if (edge.indexOf("s") >= 0) {
                    self.m_rect.height = Math.max(200, orig.height + dy);
                }
                self._clampRect();
            }
            self._applyLayout();
        }
        function onUp() {
            document.removeEventListener("mousemove", onMove);
            document.removeEventListener("mouseup", onUp);
            self.m_resizing = false;
            self._setBusy(false);
            self._saveState();
            self._syncGameCanvas(true);
        }
        document.addEventListener("mousemove", onMove);
        document.addEventListener("mouseup", onUp);
    }

    _clampRect() {
        var r = this.m_rect;
        r.width = Math.max(280, Math.min(r.width, hostInnerWidth()));
        r.height = Math.max(200, Math.min(r.height, hostInnerHeight()));
        r.left = Math.max(0, Math.min(r.left, hostInnerWidth() - 80));
        r.top = Math.max(0, Math.min(r.top, hostInnerHeight() - 36));
    }

    _dock() {
        this.m_docked = true;
        this.m_panel.classList.add("nplb-docked");
        this._refreshDockBtn();
        this._applyLayout();
        this._syncGameCanvas(true);
        this._saveState();
    }

    _undock() {
        this._undockAt(null, null);
        this._saveState();
    }

    _undockAt(clientX, clientY) {
        this.m_docked = false;
        this.m_panel.classList.remove("nplb-docked");
        this._refreshDockBtn();
        if (typeof clientX === "number") {
            this.m_rect.left = Math.max(0, clientX - Math.floor(this.m_rect.width / 2));
            this.m_rect.top = Math.max(0, clientY - 18);
        }
        this._clampRect();
        this._applyLayout();
        this._syncGameCanvas(true);
    }

    _applyLayout() {
        var panel = this.m_panel;
        if (this.m_docked) {
            var width = Math.max(280, Math.min(this.m_dockWidth, hostInnerWidth() - 80));
            this.m_dockWidth = width;
            panel.style.left = (hostInnerWidth() - width) + "px";
            panel.style.top = "0px";
            panel.style.width = width + "px";
            panel.style.height = hostInnerHeight() + "px";
            panel.style.right = "0px";
            panel.style.bottom = "0px";
        } else {
            this._clampRect();
            var r = this.m_rect;
            panel.style.left = r.left + "px";
            panel.style.top = r.top + "px";
            panel.style.width = r.width + "px";
            panel.style.height = r.height + "px";
            panel.style.right = "auto";
            panel.style.bottom = "auto";
        }
        this._syncGameCanvas();
    }

    IsDockedVisible() {
        return this.m_docked && this.m_panel && this.m_panel.classList.contains("nplb-visible");
    }

    GetDockWidth() {
        return this.m_dockWidth || 0;
    }

    _syncGameCanvas(immediate) {
        if (typeof window.applyGameCanvasLayout === "function") {
            window.applyGameCanvasLayout({ immediate: !!immediate });
        }
    }

    _flashCopied() {
        var el = this.m_statusEl;
        el.textContent = "已复制";
        if (this.m_statusTimer) clearTimeout(this.m_statusTimer);
        this.m_statusTimer = setTimeout(function () {
            el.textContent = "";
        }, 1600);
    }

    _copyFallback(text) {
        var ta = document.createElement("textarea");
        ta.value = text;
        ta.setAttribute("readonly", "");
        ta.style.position = "fixed";
        ta.style.left = "-9999px";
        document.body.appendChild(ta);
        ta.select();
        var ok = false;
        try {
            ok = document.execCommand("copy");
        } catch (e) {
        }
        document.body.removeChild(ta);
        if (ok) this._flashCopied();
    }

    _copyUrl(text, fromClick) {
        var self = this;
        if (!text) return;
        if (navigator.clipboard && navigator.clipboard.writeText) {
            navigator.clipboard.writeText(text).then(function () {
                self._flashCopied();
            }).catch(function () {
                self._copyFallback(text);
            });
            return;
        }
        this._copyFallback(text);
        if (fromClick) {
            // click is a user gesture; fallback is enough
        }
    }

    Show() {
        this.m_panel.classList.add("nplb-visible");
        if (this.m_docked) {
            this.m_panel.classList.add("nplb-docked");
        } else {
            this.m_panel.classList.remove("nplb-docked");
        }
        this._refreshDockBtn();
        this._applyLayout();
        this._syncGameCanvas(true);
    }

    Hide() {
        this.m_panel.classList.remove("nplb-visible");
        this._syncGameCanvas(true);
    }

    _normalizeUrl(raw) {
        var text = String(raw || "").trim();
        if (!text) return "";
        if (text.indexOf("://") < 0) {
            if (text.charAt(0) === "/") {
                try {
                    var base = this.m_url || (this.m_iframe && this.m_iframe.src) || "http://127.0.0.1:8089/";
                    text = new URL(text, base).href;
                } catch (e) {
                    return "";
                }
            } else {
                text = "http://" + text;
            }
        }
        try {
            var parsed = new URL(text);
            if (parsed.protocol !== "http:" && parsed.protocol !== "https:" && parsed.protocol !== "npl:") return "";
            return parsed.href;
        } catch (e) {
            return "";
        }
    }

    _nplUrl(path, search, hash) {
        var route = String(path || "/").replace(/^\/+/, "");
        return "npl://" + route + (search || "") + (hash || "");
    }

    _publicUrl(url) {
        try {
            var parsed = new URL(url);
            if (parsed.protocol === "npl:") return parsed.href;
            var bridgeMatch = parsed.pathname.match(/\/__npl_webserver__(\/.*)?$/);
            if (bridgeMatch) {
                var bridgePath = bridgeMatch[1] || "/";
                var legacyId = bridgePath.match(/^\/[0-9a-f]{8}-[0-9a-f-]{27,}(\/.*)?$/i);
                return this._nplUrl(legacyId ? (legacyId[1] || "/") : bridgePath, parsed.search, parsed.hash);
            }
            if (parsed.protocol !== "http:" || (parsed.hostname !== "127.0.0.1" && parsed.hostname !== "localhost")) {
                return url;
            }
            var match = parsed.pathname.match(/^\/webserver\/[^/]+(\/.*)?$/);
            var path = match ? (match[1] || "/") : parsed.pathname;
            return this._nplUrl(path, parsed.search, parsed.hash);
        } catch (e) {
            return url;
        }
    }

    _iframeUrl(url) {
        try {
            var parsed = new URL(url);
            if (parsed.protocol === "npl:") {
                if (!window.NplWebserverBridge) return "";
                var nplPath = "/" + parsed.hostname + parsed.pathname;
                return window.NplWebserverBridge.Route(nplPath + parsed.search + parsed.hash);
            }
            var isLoopback = parsed.protocol === "http:" && (parsed.hostname === "127.0.0.1" || parsed.hostname === "localhost");
            var isPublicAlias = parsed.origin === window.location.origin;
            if (!isLoopback && !isPublicAlias) {
                return url;
            }
            if (!window.NplWebserverBridge) return "";
            var match = parsed.pathname.match(/^\/webserver\/([^/]+)(\/.*)?$/);
            var path = match ? (match[2] || "/") : parsed.pathname;
            return window.NplWebserverBridge.Route(path + parsed.search + parsed.hash);
        } catch (e) {
            return url;
        }
    }

    _go(raw, forceReload) {
        var url = this._normalizeUrl(raw || this.m_url);
        if (!url) {
            this.m_urlEl.value = this.m_url || "";
            return;
        }
        var publicUrl = this._publicUrl(url);
        var iframeUrl = this._iframeUrl(publicUrl);
        if (!iframeUrl) {
            this.m_url = publicUrl;
            this.m_urlEl.value = this.m_url;
            this.m_urlEl.title = this.m_url;
            this.Show();
            if (this.m_bridgeRetryTimer === null && "serviceWorker" in navigator) {
                var self = this;
                this.m_bridgeRetryTimer = setTimeout(function () {
                    self.m_bridgeRetryTimer = null;
                    self._go(url, forceReload);
                }, 50);
            }
            return;
        }
        if (this.m_bridgeRetryTimer !== null) {
            clearTimeout(this.m_bridgeRetryTimer);
            this.m_bridgeRetryTimer = null;
        }
        this.m_url = publicUrl;
        this.m_urlEl.value = publicUrl;
        this.m_urlEl.title = publicUrl;
        var current = this.m_iframe.getAttribute("src") || "";
        var same = current.split("#")[0] === iframeUrl.split("#")[0];
        if (forceReload || !same) {
            if (forceReload && same) this.m_iframe.removeAttribute("src");
            this.m_iframe.src = iframeUrl;
        }
        this.Show();
    }

    Open(url) {
        if (!url) return;
        this._go(url, false);
        this._copyUrl(this.m_url, false);
    }

    OnMsg(msg) {
        var cmd = msg && msg.cmd;
        if (cmd == "Open" || cmd == "Start") {
            this.Open(msg.url);
        } else if (cmd == "Show") {
            if (msg.visible === false) this.Hide();
            else this.Show();
        } else if (cmd == "Close") {
            this.Hide();
        }
    }
}

window.NplBrowserPopupInstance = new NplBrowserPopup();

// setTimeout(function(){
//     console.log("=================================================111")
//     WebViewInstance.OnMsg({
//         cmd : "Start",
//         url : "http://127.0.0.1:8088/npl/webparacraft/test/iframe.html",
//         x : 100,
//         y: 100,
//         width: 1280,
//         height: 720,
//     })
// }, 500);