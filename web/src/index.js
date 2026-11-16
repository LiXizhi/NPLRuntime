
let g_window_width = document.documentElement.clientWidth;
let g_window_height = document.documentElement.clientHeight;
const canvas = document.getElementById('canvas');
canvas.style.margin = "0px";
canvas.style.padding = "0px";
canvas.style.position = "fixed";
canvas.style.left = "0px";
canvas.style.top = "0px";
canvas.style.width = g_window_width + 'px';
canvas.style.height = g_window_height + 'px';
canvas.width = Math.ceil(g_window_width * window.devicePixelRatio);
canvas.height = Math.ceil(g_window_height * window.devicePixelRatio);

(function installNativeInnerSize() {
    function wrap(prop, nativeStore) {
        var desc = Object.getOwnPropertyDescriptor(Window.prototype, prop)
            || Object.getOwnPropertyDescriptor(window, prop);
        if (!desc || !desc.get) return;
        var nativeGet = desc.get;
        window[nativeStore] = function () {
            return nativeGet.call(window);
        };
        try {
            Object.defineProperty(window, prop, {
                configurable: true,
                enumerable: true,
                get: function () {
                    if (prop === "innerWidth" && window.__gameViewCssW) {
                        return window.__gameViewCssW;
                    }
                    return nativeGet.call(window);
                }
            });
        } catch (e) {
            console.warn("innerWidth patch skipped", e);
        }
    }
    wrap("innerWidth", "getNativeInnerWidth");
    wrap("innerHeight", "getNativeInnerHeight");
})();

(function patchDocumentClientWidth() {
    var proto = Element.prototype;
    var desc = Object.getOwnPropertyDescriptor(proto, "clientWidth")
        || Object.getOwnPropertyDescriptor(HTMLElement.prototype, "clientWidth");
    if (!desc || !desc.get) return;
    var nativeGet = desc.get;
    window.getNativeDocumentClientWidth = function () {
        return nativeGet.call(document.documentElement);
    };
    try {
        Object.defineProperty(proto, "clientWidth", {
            configurable: true,
            enumerable: desc.enumerable,
            get: function () {
                if (window.__gameViewCssW && (this === document.documentElement || this === document.body)) {
                    return window.__gameViewCssW;
                }
                return nativeGet.call(this);
            }
        });
    } catch (e) {
        console.warn("clientWidth patch skipped", e);
    }
})();

window.nativeViewportWidth = function () {
    if (typeof window.getNativeInnerWidth === "function") return window.getNativeInnerWidth();
    if (typeof window.getNativeDocumentClientWidth === "function") return window.getNativeDocumentClientWidth();
    return document.documentElement.clientWidth;
};

window.nativeViewportHeight = function () {
    if (typeof window.getNativeInnerHeight === "function") return window.getNativeInnerHeight();
    return document.documentElement.clientHeight;
};

window.getNplWikiDockWidth = function () {
    var inst = window.NplBrowserPopupInstance;
    if (inst && typeof inst.IsDockedVisible === "function" && inst.IsDockedVisible()) {
        return inst.GetDockWidth() || 0;
    }
    return 0;
};

(function patchCanvasBackingStore() {
    var proto = HTMLCanvasElement.prototype;
    var widthDesc = Object.getOwnPropertyDescriptor(proto, "width");
    var heightDesc = Object.getOwnPropertyDescriptor(proto, "height");
    if (!widthDesc || !widthDesc.set || !widthDesc.get) return;

    function expectedBufferSize() {
        if (!window.__gameViewCssW || !window.__gameViewCssH) return null;
        return {
            w: Math.ceil(window.__gameViewCssW * window.devicePixelRatio),
            h: Math.ceil(window.__gameViewCssH * window.devicePixelRatio),
        };
    }

    function isGameCanvas(el) {
        return el && el.id === "canvas";
    }

    Object.defineProperty(proto, "width", {
        configurable: true,
        enumerable: widthDesc.enumerable,
        get: function () {
            return widthDesc.get.call(this);
        },
        set: function (v) {
            v = Number(v) || 0;
            if (isGameCanvas(this)) {
                var exp = expectedBufferSize();
                if (exp && Math.abs(v - exp.w) > 1) v = exp.w;
                if (widthDesc.get.call(this) === v) return;
            }
            widthDesc.set.call(this, v);
        }
    });
    Object.defineProperty(proto, "height", {
        configurable: true,
        enumerable: heightDesc.enumerable,
        get: function () {
            return heightDesc.get.call(this);
        },
        set: function (v) {
            v = Number(v) || 0;
            if (isGameCanvas(this)) {
                var exp = expectedBufferSize();
                if (exp && Math.abs(v - exp.h) > 1) v = exp.h;
                if (heightDesc.get.call(this) === v) return;
            }
            heightDesc.set.call(this, v);
        }
    });
})();

var g_canvas_layout_timer = null;
var g_canvas_buf_w = 0;
var g_canvas_buf_h = 0;
var g_last_notify_w = 0;
var g_last_notify_h = 0;

function notifyEngineWindowSize(cssW, cssH) {
    var bufW = Math.ceil(cssW * window.devicePixelRatio);
    var bufH = Math.ceil(cssH * window.devicePixelRatio);
    if (g_last_notify_w === cssW && g_last_notify_h === cssH) {
        canvas.style.width = cssW + "px";
        canvas.style.height = cssH + "px";
        return;
    }
    g_last_notify_w = cssW;
    g_last_notify_h = cssH;
    if (typeof Module.setCanvasSize === "function") {
        try { Module.setCanvasSize(bufW, bufH); } catch (e) {}
    }
    canvas.style.width = cssW + "px";
    canvas.style.height = cssH + "px";
    window.dispatchEvent(new Event("resize"));
    requestAnimationFrame(function () {
        window.dispatchEvent(new Event("resize"));
    });
}

function commitGameCanvasBuffer(cssW, cssH) {
    var bufW = Math.ceil(cssW * window.devicePixelRatio);
    var bufH = Math.ceil(cssH * window.devicePixelRatio);
    var sizeChanged = !(canvas.width === bufW && canvas.height === bufH && g_canvas_buf_w === bufW);
    g_canvas_buf_w = bufW;
    g_canvas_buf_h = bufH;
    if (sizeChanged) {
        canvas.width = bufW;
        canvas.height = bufH;
    }
    canvas.style.width = cssW + "px";
    canvas.style.height = cssH + "px";
    notifyEngineWindowSize(cssW, cssH);
}

window.applyGameCanvasLayout = function (opts) {
    if (!canvas) return;
    var dock = window.getNplWikiDockWidth();
    var fullW = window.nativeViewportWidth();
    var fullH = window.nativeViewportHeight();
    var w = Math.max(80, fullW - dock);
    window.__gameViewCssW = dock > 0 ? w : 0;
    window.__gameViewCssH = dock > 0 ? fullH : 0;
    g_window_width = fullW;
    g_window_height = fullH;
    canvas.style.left = "0px";
    if (!opts || !opts.keepTop) canvas.style.top = "0px";
    canvas.style.width = w + "px";
    canvas.style.height = fullH + "px";
    if (g_canvas_layout_timer) {
        clearTimeout(g_canvas_layout_timer);
        g_canvas_layout_timer = null;
    }
    if (opts && opts.immediate) {
        commitGameCanvasBuffer(w, fullH);
    } else {
        g_canvas_layout_timer = setTimeout(function () {
            g_canvas_layout_timer = null;
            commitGameCanvasBuffer(w, fullH);
        }, 50);
    }
};

var Module = typeof Module != 'undefined' ? Module : {};
Module.canvas = canvas;

(function installLocalDevLogBridge() {
    const host = window.location.hostname;
    if (host !== "127.0.0.1" && host !== "localhost") return;
    const queue = [];
    let timer = null;
    function flush() {
        timer = null;
        if (!queue.length) return;
        const batch = queue.splice(0, 60);
        fetch(window.location.origin + "/fs/log", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ lines: batch }),
            keepalive: true,
        }).catch(function () {});
        if (queue.length) timer = setTimeout(flush, 250);
    }
    function enqueue(prefix, text) {
        if (!text) return;
        const s = String(text);
        queue.push(prefix ? (prefix + " " + s) : s);
        if (queue.length > 300) queue.splice(0, queue.length - 300);
        if (!timer) timer = setTimeout(flush, 250);
    }
    const prevPrint = Module.print;
    Module.print = function (text) {
        if (prevPrint) prevPrint(text);
        else console.log(text);
        enqueue("[out]", text);
    };
    const prevPrintErr = Module.printErr;
    Module.printErr = function (text) {
        if (prevPrintErr) prevPrintErr(text);
        else console.warn(text);
        enqueue("[err]", text);
    };
    const origError = console.error.bind(console);
    console.error = function () {
        origError.apply(console, arguments);
        try {
            enqueue("[console.error]", Array.from(arguments).map(a => (a && a.stack) ? a.stack : String(a)).join(" "));
        } catch (e) {}
    };
    window.addEventListener("error", function (ev) {
        enqueue("[window.error]", (ev && ev.message ? ev.message : "error") + (ev && ev.filename ? (" @" + ev.filename + ":" + ev.lineno) : ""));
    });
    window.addEventListener("unhandledrejection", function (ev) {
        const r = ev && ev.reason;
        enqueue("[unhandledrejection]", r && r.stack ? r.stack : String(r));
    });
})();

const GetQueryString = (key, decode) => {
    var value = "";                  // 查询值
    //取得查询字符串并去掉开头问号       
    var qs = location.search.length > 0 ? location.search.substring(1) : "";
    // 取得每一项     
    var items = qs.length ? qs.split('&') : [];
    //逐个将每一项添加到args对象中       
    for (var i = 0; i < items.length; i++) {
        var item = items[i].split('=');
        var name = decodeURIComponent(item[0]);
        if (name == key) {
            value = decode ? decodeURIComponent(item[1]) : item[1];
            break;
        }
    }
    return value;
};

let pid = GetQueryString("pid");
if (pid == "") {
    const pathname = window.location.pathname;
    const match = pathname.match(/\/pid\/(\d+)$/);
    if (match) {
        pid = match[1];
        const search = (window.location.search || "") + (window.location.search ? "&" : "?") + "pid=" + pid;
        window.history.replaceState({}, "", window.location.pathname + search);
    }
}

const cmdline = (typeof cmdlineFromCustomLoader !== 'undefined' ? cmdlineFromCustomLoader : null) || GetQueryString('cmdline', true);
const regex = /lang="([^"]+)"/;
const match = cmdline.match(regex);
const language = GetQueryString('language', true) || (match ? match[1] : ''); 

const is_english_language = language == "enUS";
const is_chinese_language = !is_english_language;
if (is_english_language) {
    document.getElementById("fullScreenBtn").innerText = "Exit Full Screen";
    document.getElementById("loadingTipText").innerText = "First intall may take 1-2 mins";
}

const loadingRegex = /loadingText="([^"]+)"/;
const loadingValue = cmdline.match(loadingRegex);
console.log("loadingValue===============:", loadingValue);
if (loadingValue && loadingValue.length > 0) {
    const loadingText = document.getElementById("loading-text");
    if (loadingText) {
        const innerText = loadingValue[1]
        console.log("loadingText===============:", loadingText,innerText);
        if (innerText && innerText != "" ) 
        {
            loadingText.innerText = innerText;
        } 
        else 
        {
            loadingText.innerText = "Made with Paracraft and NPL";
        }
    }
}

let IsStartAudioContext = false;
Module.canvas.addEventListener('click', () => {
    if (IsStartAudioContext) return;
    IsStartAudioContext = true;
    if (typeof Tone !== 'undefined' && typeof Tone.start === 'function') {
        Tone.start(); // 启动音频上下文（必须由用户事件触发）
    }
});

let isWPS = /WpsOfficeApp/i.test(navigator.userAgent);
let isTouchDevice = 'ontouchstart' in window || navigator.maxTouchPoints > 0 || navigator.msMaxTouchPoints > 0;
let isMobile = /iPhone|iPad|iPod|Android|OpenHarmony|HarmonyOS|Windows Phone|Mobile/i.test(navigator.userAgent);
var isiOS = /iPad|iPhone|iPod/i.test(navigator.userAgent) && !window.MSStream;

// tricky: when mouse down, let the input capture key event, otherwise the web paracraft will not get key event 
var canvasElem = document.getElementById('canvas');
var inputElem = document.getElementById('input')
if (canvasElem && inputElem && !isMobile) {
    canvasElem.addEventListener('mousedown', function (event) {
        inputElem.focus();
    }, false);
}

var load = isLoading(document.getElementById('canvas'), {
    'type': 'full-overlay',
    'text': is_chinese_language ? '页面正在加载, 请稍后...' : 'The page is loading, please wait...',
});

Module.preInit = function () {
    load.loading();
};

Module.HideLoading = function () {
    load.remove();
    const element = document.getElementById("load-tiptext");
    if (element) {
        element.parentNode.removeChild(element);
        broadcastToAllIframes({
            type: 'loadingProgress1',
            progress: 100,
        }, '*');
    }
};

Module.setStatus = function (text) {
    const els = document.getElementsByClassName("is-loading-text-wrapper");
    if (els.length == 0) return;
    const el = els[0];
    var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
    if (m) {
        const value = parseInt(m[2]) * 100;
        const max = parseInt(m[4]) * 100;
        const percentage = Math.ceil(value * 100 / max);
        el.innerHTML = is_chinese_language ? `页面正在加载, 请稍后(${percentage}%)...` : `Loading, please stand by (${percentage}%)...`;
        window.loading_percentage = percentage;
        broadcastToAllIframes({
            type: 'loadingProgress1',
            progress: Math.min(percentage, 99),
        }, '*');
    }
}

Module.onRuntimeInitializedCallbacks = [];
Module.noInitialRun = true;
if (window.LocalDiskFS && typeof window.LocalDiskFS.Install === 'function') {
    window.LocalDiskFS.Install(Module);
}

Module.onRuntimeInitialized = function () {
    console.log("================onRuntimeInitialized==================");
    Module.isRuntimeInitialized = true;
    let callbacks = Module.onRuntimeInitializedCallbacks;
    for (let i = 0; i < callbacks.length; i++) {
        (callbacks[i])();
    }
    try {
        Module._main();
    } catch (e) {
        if (e !== "unwind" && !(e && e.name === "ExitStatus")) {
            throw e;
        }
    }
}

Module.ParaCraftFileSystemInited = function() {
    if (window.LocalDiskFS && typeof window.LocalDiskFS.MergeDeferredMountDirs === 'function') {
        window.LocalDiskFS.MergeDeferredMountDirs();
    }
    const CopyDB = (src_candidates, dst_db_path) => {
        const candidates = Array.isArray(src_candidates) ? src_candidates : [src_candidates];
        let db_exist = false;
        try {
            const stat = FS.stat(dst_db_path);
            if (stat && FS.isFile(stat.mode) && stat.size > 0) db_exist = true;
        }
        catch (e) {
        }
        if (!db_exist) {
            for (let i = 0; i < candidates.length; i++) {
                try {
                    const src_path = candidates[i];
                    const src_stat = FS.stat(src_path);
                    if (src_stat && FS.isFile(src_stat.mode)) {
                        const data = FS.readFile(src_path, {encoding : "binary"});
                        FS.writeFile(dst_db_path, data);
                        break;
                    }
                } catch (e) {
                }
            }
        }
    };
    if (window.__HAQI1__ || window.__HAQI2__)
    {
        CopyDB("/config/database/characters.db", "/Database/characters.db");
        CopyDB("/config/database/extendedcost.db.mem", "/Database/extendedcost.db.mem");
        CopyDB("/config/database/globalstore.db.mem", "/Database/globalstore.db.mem");
        if (window.__HAQI2__)
        {
            CopyDB("/config/database/userdata.teen.db", "/Database/userdata.teen.db");
        }
    }
    else
    {
        CopyDB(["/config/characters.db", "/database/characters.db"], "/Database/characters.db");
    }

    FS.syncfs(false, function(){});
}

const input_info = {};
window.addEventListener("resize", function () {
    if (Module.canvas == undefined) return;
    // Module.canvas.width = document.documentElement.clientWidth * window.devicePixelRatio;
    // Module.canvas.height = document.documentElement.clientHeight * window.devicePixelRatio;
    console.log("========================window resize============================", input_info.inputing, input_info.window_height, document.documentElement.clientHeight)
    if (input_info.inputing && !((getOperatingSystem() == "win32" || getOperatingSystem() == "mac"))) {
        const ime_height = input_info.window_height - document.documentElement.clientHeight;
        const view_height = input_info.window_height - input_info.view_bottom;
        console.log("==============set canvas top====================", view_height, ime_height);
        if (ime_height > view_height) {
            canvas.style.top = (view_height - ime_height - document.documentElement.clientHeight / 2) + 'px';
        }
        else {
            canvas.style.top = '0px';
        }
    }
    else {
        if (g_window_width != window.nativeViewportWidth() || g_window_height != window.nativeViewportHeight()) {
            window.applyGameCanvasLayout();
            console.log("==============set canvas size====================", g_window_width, g_window_height);
        }
    }

    const cur_timestamp = new Date().getTime();
    const use_timestamp = cur_timestamp - input_info.timestamp;
    if (input_info.inputing && document.documentElement.clientHeight == input_info.window_height) {
        input_info.inputing = false;
        canvas.style.top = '0px';
    }
});

// input 解决中文输入与移动端软键盘退格问题
const PARA_VK_BACKSPACE = 14; // ParaEngine::EVirtualKey::KEY_BACK
const PARA_VK_RETURN = 28;    // ParaEngine::EVirtualKey::KEY_RETURN
const PARA_VK_DELETE = 211;   // ParaEngine::EVirtualKey::KEY_DELETE

const MOBILE_SENTINEL_CHAR = "\u200B";
const MOBILE_SENTINEL_INITIAL_LEN = 64;
const MOBILE_SENTINEL_MIN_LEN = 16;
const MOBILE_INPUT_SENTINEL = MOBILE_SENTINEL_CHAR.repeat(MOBILE_SENTINEL_INITIAL_LEN);

var input = document.getElementById('input');
if (input) {
    input.setAttribute("autocomplete", "off");
    input.setAttribute("autocorrect", "off");
    input.setAttribute("autocapitalize", "off");
    input.setAttribute("spellcheck", "false");
}

let mobileSentinelLen = 0;
let isImeComposing = false;
let justFinishedComposition = false;
let suppressInputBackspace = false;
let suppressInputBackspaceTimer = null;
let suppressInputEnter = false;
let suppressInputEnterTimer = null;

function sendParaKeyDown(vk) {
    if (Module && typeof Module._OnKeyDown === "function") {
        Module._OnKeyDown(Number(vk));
    }
}

function sendParaKeyUp(vk) {
    if (Module && typeof Module._OnKeyUp === "function") {
        Module._OnKeyUp(Number(vk));
    }
}

function tapParaKey(vk) {
    sendParaKeyDown(vk);
    sendParaKeyUp(vk);
}

function sendParaTextInput(text) {
    if (!text || !Module || typeof Module._OnTextInput !== "function") return;
    Module._OnTextInput(stringToNewUTF8(text));
}

function markBackspaceHandledByKeydown() {
    suppressInputBackspace = true;
    if (suppressInputBackspaceTimer) clearTimeout(suppressInputBackspaceTimer);
    suppressInputBackspaceTimer = setTimeout(function () {
        suppressInputBackspace = false;
        suppressInputBackspaceTimer = null;
    }, 0);
}

function markEnterHandledByKeydown() {
    suppressInputEnter = true;
    if (suppressInputEnterTimer) clearTimeout(suppressInputEnterTimer);
    suppressInputEnterTimer = setTimeout(function () {
        suppressInputEnter = false;
        suppressInputEnterTimer = null;
    }, 0);
}

function isTouchOrMobileMode() {
    return isTouchDevice || isMobile || ('ontouchstart' in window) || (navigator.maxTouchPoints > 0);
}

function resetMobileInputSentinel() {
    if (!input) return;
    input.value = MOBILE_INPUT_SENTINEL;
    mobileSentinelLen = MOBILE_SENTINEL_INITIAL_LEN;
    try {
        input.setSelectionRange(MOBILE_SENTINEL_INITIAL_LEN, MOBILE_SENTINEL_INITIAL_LEN);
    } catch (e) {}
}

Module.IsInputing = function () {
    return input_info.inputing ? 1 : 0;
}

Module.SetTextInputRect = function (x, y, w, h) {
    input.style.left = x;
    input.style.top = y;
    input.style.width = w;
    input.style.height = h;
}
Module.StartTextInput = function (view_bottom, params) {
    let default_value = "";
    try {
        params = UTF8ToString(params);
        default_value = JSON.parse(params).curEditText;
    } catch (e) { }
    if (input_info.inputing && (!input || document.activeElement === input)) return;
    if (isiOS) {
        // 引擎bug会调多次，导致ios中文输入框弹出多次 间隔时间太短直接忽略
        if (((new Date().getTime()) - input_info.timestamp) < 1000) return;
        input_info.inputing = true;
        input_info.value = prompt("输入:", default_value);
        if (input_info.value === null) input_info.value = default_value;
        Module._OnClearTextInput(stringToNewUTF8(input_info.value));
        input_info.inputing = false;
        input_info.value = "";
        input_info.timestamp = new Date().getTime();
        return;
    }
    if (!isImeComposing) {
        if (isTouchOrMobileMode()) {
            resetMobileInputSentinel();
        } else if (input) {
            input.value = "";
            mobileSentinelLen = 0;
        }
    }
    const wasInputing = input_info.inputing;
    input_info.inputing = true;
    input_info.width = Module.canvas.width;
    input_info.height = Module.canvas.height;
    input_info.window_width = document.documentElement.clientWidth;
    input_info.window_height = document.documentElement.clientHeight;
    input_info.timestamp = new Date().getTime();
    input_info.view_bottom = (view_bottom || 0) / window.devicePixelRatio;

    if (input) {
        if (!wasInputing && isTouchOrMobileMode() && document.activeElement === input) {
            input.blur();
        }
        input.focus();
    }
}

Module.StopTextInput = function () {
    if (isiOS) return;

    isImeComposing = false;
    if (input) input.blur();
    input_info.inputing = false;
    canvas.style.top = '0px';
}
input_info.isTouchDevice = isTouchDevice;

if (input) {
    input.addEventListener("blur", function () {
        if (isTouchOrMobileMode()) {
            isImeComposing = false;
            input_info.inputing = false;
            canvas.style.top = '0px';
        }
    });

    input.addEventListener("compositionstart", function () {
        isImeComposing = true;
        if (!isTouchOrMobileMode()) {
            input.value = "";
        }
    });

    input.addEventListener("compositionend", function (event) {
        isImeComposing = false;
        const committed = (event && typeof event.data === "string")
            ? event.data
            : (input.value || "").replace(/\u200B/g, "");
        if (committed) {
            sendParaTextInput(committed);
        }
        justFinishedComposition = true;
        if (isTouchOrMobileMode()) {
            resetMobileInputSentinel();
        } else {
            input.value = "";
            mobileSentinelLen = 0;
        }
        setTimeout(function () {
            justFinishedComposition = false;
        }, 0);
    });

    if (isTouchOrMobileMode()) {
        resetMobileInputSentinel();
    } else {
        input.focus();
    }

    input.addEventListener("beforeinput", function (event) {
        if (!isTouchOrMobileMode() || isImeComposing || event.isComposing) return;
        const inputType = event.inputType || "";
        if (inputType === "deleteContentBackward" || inputType === "deleteWordBackward" || inputType === "deleteSoftLineBackward") {
            if ((input.value || "").length === 0 && !suppressInputBackspace) {
                event.preventDefault();
                tapParaKey(PARA_VK_BACKSPACE);
                resetMobileInputSentinel();
            }
        } else if (inputType === "insertLineBreak" || inputType === "insertParagraph") {
            event.preventDefault();
            if (!suppressInputEnter) {
                markEnterHandledByKeydown();
                tapParaKey(PARA_VK_RETURN);
            }
        }
    });

    input.addEventListener("input", function (event) {
        if (!isTouchOrMobileMode() || isImeComposing || (event && event.isComposing)) return;
        if (justFinishedComposition) {
            resetMobileInputSentinel();
            return;
        }

        const rawValue = input.value || "";
        const typed = rawValue.replace(/\u200B/g, "");
        const inputType = (event && event.inputType) || "";

        if (typed !== "") {
            if (getOperatingSystem() === "pico" && typed.length > 1) {
                resetMobileInputSentinel();
                return;
            }
            const cleanText = typed.replace(/[\r\n]+/g, "");
            if (cleanText !== "") {
                sendParaTextInput(cleanText);
            }
            if (/[\r\n]/.test(typed) && !suppressInputEnter) {
                tapParaKey(PARA_VK_RETURN);
            }
            resetMobileInputSentinel();
            return;
        }

        const remainingSentinelLen = rawValue.length;
        if (suppressInputBackspace || inputType.indexOf("Forward") !== -1) {
            suppressInputBackspace = false;
            mobileSentinelLen = remainingSentinelLen;
            if (mobileSentinelLen < MOBILE_SENTINEL_MIN_LEN) {
                resetMobileInputSentinel();
            }
            return;
        }

        if (remainingSentinelLen < mobileSentinelLen || inputType === "deleteContentBackward" || inputType === "deleteWordBackward" || inputType === "deleteSoftLineBackward") {
            let deleteCount = mobileSentinelLen - remainingSentinelLen;
            if (deleteCount <= 0) {
                deleteCount = 1;
            } else if (remainingSentinelLen === 0 && mobileSentinelLen >= MOBILE_SENTINEL_MIN_LEN) {
                deleteCount = 1;
            } else if (deleteCount > 16) {
                deleteCount = 16;
            }
            mobileSentinelLen = remainingSentinelLen;
            for (let i = 0; i < deleteCount; i++) {
                tapParaKey(PARA_VK_BACKSPACE);
            }
            if (mobileSentinelLen < MOBILE_SENTINEL_MIN_LEN) {
                resetMobileInputSentinel();
            }
        } else if (remainingSentinelLen !== mobileSentinelLen) {
            mobileSentinelLen = remainingSentinelLen;
            if (mobileSentinelLen < MOBILE_SENTINEL_MIN_LEN) {
                resetMobileInputSentinel();
            }
        }
    });
}

window.ParaCraftModule = Module;

// Js 执行 C++ 代码时转换
if (typeof window.JsStringToCString != "function") {
    window.JsStringToCString = function (jsString) {
        var lengthBytes = lengthBytesUTF8(jsString) + 1;
        var stringOnWasmHeap = _malloc(lengthBytes);
        stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
        return stringOnWasmHeap;
    };
}

// C++ 执行 Js 代码时转换
if (typeof window.CStringToJsString != "function") {
    window.CStringToJsString = function (cString) {
        return UTF8ToString(cString);
    }
}

window.RecvMsgFromEmscripten = function (msg_json_data) {
    var msg = {};
    try {
        msg = JSON.parse(msg_json_data);
    }
    catch (e) {
        console.log(e);
        console.log(msg_json_data);
    }

    // console.log("RecvMsgFromEmscripten:", msg);
    if (msg.target == "webview") {
        WebViewInstance.OnMsg(msg)
    }

    if (msg.target == "nplbrowser" && window.NplBrowserPopupInstance) {
        window.NplBrowserPopupInstance.OnMsg(msg);
    }

    if (msg.target == "emscripten") {
        EmscriptenInstance.RecvMsg(msg);
    }

    if (msg.target == "external" && msg.msgname === 'customLoader') {
        setTimeout(() => {
            const messageEvent = new MessageEvent('message', {
                data: {
                    type: msg.msgname,
                    data: msg.msgdata,
                },
                origin: window.location.origin,
                source: window,
            });
            window.dispatchEvent(messageEvent);
        }, 0);
    }

    // 发送消息给父页面
    if (msg.target == "external" && window.parent && window.parent && window.parent !== window && window.parent.postMessage) {
        window.parent.postMessage(msg, "*");
    }
};

// 检查消息来源是否是父窗口
window.addEventListener("SendMsgToEmscripten", function (event) {
    if (window.parent !== window && event.source === window.parent) {
        event.data.filename = "external";
        window.SendMsgToEmscripten(event.data);
    }
});

window.SendMsgToEmscripten = function (msg) {
    if (!Module.isRuntimeInitialized) return;
    var c_filename = window.JsStringToCString(msg.filename);
    var c_msg_json_data = window.JsStringToCString(JSON.stringify(msg));
    // 执行C++函数
    Module._SendMsgToEmscripten(c_filename, c_msg_json_data);
};

window.SetParaCraftPaused = function (paused) {
    Module._SetAppPaused(paused);
}

if (typeof (SharedArrayBuffer) == "undefined" && window.location.hostname == "webparacraft.keepwork.com" && window.location.pathname == "/ParaCraftMultiThread.html") {
    window.location.href = "https://webparacraft.keepwork.com/ParaCraftSingleThread.html" + window.location.search;
}

if (typeof (SharedArrayBuffer) == "undefined" && window.location.hostname == "emscripten.keepwork.com" && window.location.pathname == "/ParaCraftMultiThread.html") {
    window.location.href = "https://emscripten.keepwork.com/ParaCraftSingleThread.html" + window.location.search;
}

function isFullScreen() {  
    return !!(  
      document.fullscreenElement ||  
      document.webkitFullscreenElement ||  
      document.mozFullScreenElement ||  
      document.msFullscreenElement  
    );  
}
// 进入全屏模式
function enterFullScreen() {
    var element = document.documentElement;
    if (element.requestFullscreen) {
        element.requestFullscreen();
    } else if (element.mozRequestFullScreen) { // Firefox
        element.mozRequestFullScreen();
    } else if (element.webkitRequestFullscreen) { // Chrome, Safari and Opera
        element.webkitRequestFullscreen();
    } else if (element.msRequestFullscreen) { // IE/Edge
        element.msRequestFullscreen();
    }
}

// 退出全屏模式
function exitFullScreen() {
    if (document.exitFullscreen) {
        if (document.fullscreenElement && document.hasFocus()) {
            document.exitFullscreen();
        }
    } else if (document.mozCancelFullScreen) { // Firefox
        document.mozCancelFullScreen();
    } else if (document.webkitExitFullscreen) { // Chrome, Safari and Opera
        document.webkitExitFullscreen();
    } else if (document.msExitFullscreen) { // IE/Edge
        document.msExitFullscreen();
    }
}

function getOperatingSystem() {
    let userAgent = navigator.userAgent || navigator.vendor || window.opera;

    if (/PICO/i.test(userAgent)) {
        return "pico";
    }
    if (/android/i.test(userAgent)) {
        return "android";
    }
    if (/OpenHarmony|HarmonyOS/i.test(userAgent)) {
        return "harmonyos";
    }
    if (/iPad|iPhone|iPod/.test(userAgent) && !window.MSStream) {
        return "ios";
    }
    if (/Macintosh|MacIntel|MacPPC|Mac68K/.test(userAgent)) {
        return "mac";
    }
    if (/Win32|Win64|Windows|WinCE/.test(userAgent)) {
        return "win32";
    }
    if (/Linux/.test(userAgent)) {
        return "linux";
    }
    return "unknown";
}
window.getOperatingSystem = getOperatingSystem;

let ParacraftKeyNameKeyCodeMap = undefined;
function InitParacraftKeyNameKeyCodeMap() {
    if (ParacraftKeyNameKeyCodeMap != undefined || !window.paracraft_keycode_name_map) return;
    const paracraft_keycode_name_map = window.paracraft_keycode_name_map || {};
    ParacraftKeyNameKeyCodeMap = {};
    for (const key in paracraft_keycode_name_map) {
        const name = paracraft_keycode_name_map[key];
        ParacraftKeyNameKeyCodeMap[name] = key;
        ParacraftKeyNameKeyCodeMap[name.toLowerCase()] = key;
    }
    ParacraftKeyNameKeyCodeMap[""] = ParacraftKeyNameKeyCodeMap["Space"];
    ParacraftKeyNameKeyCodeMap["Control"] = ParacraftKeyNameKeyCodeMap["Left Ctrl"];
    ParacraftKeyNameKeyCodeMap["Shift"] = ParacraftKeyNameKeyCodeMap["Left Shift"];
    ParacraftKeyNameKeyCodeMap["Alt"] = ParacraftKeyNameKeyCodeMap["Left Alt"];
    ParacraftKeyNameKeyCodeMap["Meta"] = ParacraftKeyNameKeyCodeMap["Menu"];
    ParacraftKeyNameKeyCodeMap["ArrowLeft"] = ParacraftKeyNameKeyCodeMap["Left"];
    ParacraftKeyNameKeyCodeMap["ArrowUp"] = ParacraftKeyNameKeyCodeMap["Up"];
    ParacraftKeyNameKeyCodeMap["ArrowRight"] = ParacraftKeyNameKeyCodeMap["Right"];
    ParacraftKeyNameKeyCodeMap["ArrowDown"] = ParacraftKeyNameKeyCodeMap["Down"];
    ParacraftKeyNameKeyCodeMap["PageUp"] = ParacraftKeyNameKeyCodeMap["Prior"];
    ParacraftKeyNameKeyCodeMap["Enter"] = ParacraftKeyNameKeyCodeMap["Return"];
}

window.isNumLockEnabled = 1;
document.addEventListener('keydown', function (event) {
    InitParacraftKeyNameKeyCodeMap();

    if (event.getModifierState && event.getModifierState('NumLock')) {
        window.isNumLockEnabled = 1;
        // console.log('NumLock is on');
    } else {
        window.isNumLockEnabled = 0;
        // console.log('NumLock is off');
    }
    if (event.code == "F11") {
        if (isFullScreen()) {
            exitFullScreen();
        }
        else {
            enterFullScreen();
        }
    }

    if (isWPS) {
        if (event.key === "Backspace" || event.key === "Delete") {
            markBackspaceHandledByKeydown();
        } else if (event.key === "Enter") {
            markEnterHandledByKeydown();
        }
        if (ParacraftKeyNameKeyCodeMap && ParacraftKeyNameKeyCodeMap[event.key]) {
            sendParaKeyDown(ParacraftKeyNameKeyCodeMap[event.key]);
        }
        return;
    }

    // Emscripten SDL2 only maps physical KeyboardEvent.code scancodes and drops virtual keyboard
    // events where event.code is "" or "Unidentified" (common on Android/HarmonyOS/mobile browsers).
    if (!isImeComposing && !event.isComposing) {
        if (event.key === "Backspace" || event.keyCode === 8) {
            markBackspaceHandledByKeydown();
            if (event.code !== "Backspace") {
                event.preventDefault();
                tapParaKey(PARA_VK_BACKSPACE);
                return;
            }
        } else if (event.key === "Delete" || event.keyCode === 46) {
            markBackspaceHandledByKeydown();
            if (event.code !== "Delete" && event.code !== "NumpadDecimal") {
                event.preventDefault();
                tapParaKey(PARA_VK_DELETE);
                return;
            }
        } else if (event.key === "Enter" || event.keyCode === 13) {
            markEnterHandledByKeydown();
            if (event.code !== "Enter" && event.code !== "NumpadEnter") {
                event.preventDefault();
                if (!event.repeat) {
                    tapParaKey(PARA_VK_RETURN);
                }
                return;
            }
        }
    }

    if (!input_info.inputing) {
        if (event.key == "Process" && event.code && /Key[A-Z]/.test(event.code)) {
            const keycode = event.code.substring(3).toLowerCase();
            if (ParacraftKeyNameKeyCodeMap && ParacraftKeyNameKeyCodeMap[keycode]) {
                sendParaKeyDown(ParacraftKeyNameKeyCodeMap[keycode]);
            } else {
                sendParaKeyDown(keycode.charCodeAt(0));
            }
        }
    }
});

document.addEventListener('keyup', function (event) {
    if (isWPS) {
        if (ParacraftKeyNameKeyCodeMap && ParacraftKeyNameKeyCodeMap[event.key]) {
            sendParaKeyUp(ParacraftKeyNameKeyCodeMap[event.key]);
        }
        return;
    } 
    
    if (!input_info.inputing) {
        if (event.key == "Process" && event.code && /Key[A-Z]/.test(event.code)) {
            const keycode = event.code.substring(3).toLowerCase();
            if (ParacraftKeyNameKeyCodeMap && ParacraftKeyNameKeyCodeMap[keycode]) {
                sendParaKeyUp(ParacraftKeyNameKeyCodeMap[keycode]);
            } else {
                sendParaKeyUp(keycode.charCodeAt(0));
            }
        }
    }
});

class SerialPort {
    constructor() {
        this.m_ports = {};
        this.m_index = 0;
    }

    GetPortName(port) {
        for (let key in this.m_ports) {
            const open_port = this.m_ports[key];
            if (open_port == port) {
                return key;
            }
        }
        this.m_index = this.m_index + 1;
        let name = port.name || ("COM_" + this.m_index);
        this.m_ports[name] = port;
        return name;
    }

    async RequestSerialPortAccess() {
        try {
            await navigator.serial.requestPort();
            console.log("已获取串口访问权限");
        } catch (error) {
            console.error("请求串口访问权限失败:", error);
        }
    }

    async GetSerialPorts(force_request) {
        if (Object.keys(this.m_ports).length == 0 || force_request) {
            await this.RequestSerialPortAccess();
        } else {
            return this.m_ports;
        }

        try {
            const ports = await navigator.serial.getPorts();
            ports.forEach((port) => {
                let name = this.GetPortName(port);
                this.m_ports[name] = port;
            });
            return this.m_ports;
        } catch (error) {
            console.error('Error getting serial ports:', error);
            return {};
        }
    }

    async GetGetSerialPortNames() {
        const ports = await this.GetSerialPorts();
        const names = Object.keys(ports).map(name => "[[" + name + "]]");
        console.log('Serial ports:', names);
        return names.join(",");
    }

    async Recv(port, name) {
        try {
            const reader = port.readable.getReader();
            const decoder = new TextDecoder();
            while (true) {
                const { value, done } = await reader.read();
                if (done) {
                    break;
                }
                const decodedData = decoder.decode(value);
                // console.log('Received data:', value);
                // console.log('Decoded data:', decodedData);

                const c_data = window.JsStringToCString(JSON.stringify({
                    filename: name,
                    data: decodedData
                }));

                if (Module._RecvSerialPortData) {
                    Module._RecvSerialPortData(c_data);
                }
            }

            reader.releaseLock();
        } catch (error) {
            console.error('Error receiving serial data:', error);
        }
    }
    async Open(name, rate) {
        const ports = await this.GetSerialPorts(true);
        const port = ports[name];
        if (!port) {
            console.log('Serial Ports Not Exist!!!', name);
            return;
        }
        try {
            this.Close(name);

            await port.open({ baudRate: rate });

            this.Recv(port, name);

            this.m_ports[name] = port;
        } catch (error) {
            console.error('Error opening serial port:', error);
        }
    }

    async Send(name, data) {
        const port = this.m_ports[name];
        if (port == undefined) {
            console.log('Serial Ports Not Open!!!', name);
            return;
        }
        if (port.writable) {
            const writer = port.writable.getWriter();
            writer.write(new TextEncoder().encode(data));
            writer.releaseLock(); // 释放锁
        } else {
            console.log('Serial Ports Not Writable!!!', name);
        }
    }

    async Close(name) {
        if (this.m_ports[name]) {
            try {
                await this.m_ports[name].close();
                this.m_ports[name] = undefined;
            } catch (error) {

            }
        }
    }
};

window.SerialPortInstance = new SerialPort();

window.jsfetch = async function (url, options) {
    const controller = new AbortController();

    options = options || {};
    options.signal = controller.signal;
    const method = (options.method || 'GET').toUpperCase();
    if (method === 'GET' || method === 'HEAD') {
        delete options.body;
    }

    try {
        const response = await fetch(url, options);
        let headers = '';
        response.headers.forEach((value, name) => {
            headers += `${name}: ${value}\n`;
        });

        const result = {
            status: response.status,
            headers: headers,
            body: "",
            stream: false,
            abort: function () { controller.abort(); },
        };
        const content_type = (response.headers.get('content-type') || '').toLowerCase();
        if (content_type.includes('application/json') || content_type.includes('text/')) {
            result.body = await response.text();
        } else if (content_type.includes('image/')) {
            result.body = await response.blob();
        } else if (response.body && typeof response.body.getReader === 'function') {
            const reader = response.body.getReader();
            if (reader) {
                result.stream = true;
                result.body = async function () {
                    return await reader.read();
                };
            }
        } else {
            result.body = await response.text();
        }
        return result;
    } catch (err) {
        console.warn('[jsfetch] failed:', url, err);
        return {
            status: 0,
            headers: '',
            body: '',
            stream: false,
            abort: function () {},
        };
    }
};

// 向页面中所有iframe发送消息
function broadcastToAllIframes(message, targetOrigin = '*') {
    const iframes = document.querySelectorAll('iframe');
    iframes.forEach(iframe => {
        try {
            iframe.contentWindow.postMessage(message, targetOrigin);
        } catch (error) {
            console.warn('无法向iframe发送消息:', error);
        }
    });
}

window.EnterFullScreen = enterFullScreen;
window.ExitFullScreen = exitFullScreen;

window.addEventListener("load", function () {
    document.addEventListener('fullscreenchange', (event) => {
        console.log("======================fullscreenchange========================load================================1212")
        if (document.fullscreenElement) {
            console.log('全屏模式已激活');
            document.getElementById('fullScreenBtn').style.display = 'block';
        } else {
            document.getElementById('fullScreenBtn').style.display = 'none';
            console.log('全屏模式已退出');
        }
        broadcastToAllIframes({
            type: 'fullscreenStatusResponse',
            isFullscreen: !!document.fullscreenElement
        });
    });
    document.getElementById('fullScreenBtn').addEventListener('click', function () {
        document.exitFullscreen();
    })
});

function handleCustomLoader(message_data) {
    switch (message_data.name) {
        case 'gameLoading':
            broadcastToAllIframes({
                type: 'loadingProgress2',
                progress: message_data.progress,
            }, '*');
            break;
        case 'remove':
            const iframe = document.getElementById('custom-loader');
            if (iframe) {
                iframe.remove();
            }
            break;
    }
}

window.addEventListener('message', (event) => {
    const data = event.data;
    if (data instanceof Object) {
        const message_type = data.type;
        const message_data = data.data;
        switch (message_type) {
            case 'EnterFullScreen':
                window.EnterFullScreen();
                break;
            case 'ExitFullScreen':
                window.ExitFullScreen();
                break;
            case 'requestFullscreenStatus':
                broadcastToAllIframes({
                    type: 'fullscreenStatusResponse',
                    isFullscreen: !!document.fullscreenElement
                });
                break;
            case 'customLoader':
                if (message_data instanceof Object) {
                    handleCustomLoader(message_data);
                }
                break;
        }
    }
});
