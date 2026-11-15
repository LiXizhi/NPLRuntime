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

const GetSystem = () => {
    if (window.paracraft_platform) return window.paracraft_platform;

    const platform = GetQueryString("platform", true);
    if (platform != "") return platform;

    // window webview2
    if (window.chrome && window.chrome.webview) return "windows";

    const { userAgent } = navigator;

    if (/PICO/.test(userAgent) || userAgent.includes("Pico")) {
        return "pico";
    }
    if (/OpenHarmony/.test(userAgent) || userAgent.includes("OpenHarmony")) {
        return "harmony";
    }
    if (userAgent.match('Macintosh')) {
        return 'macos';
    }
    if (userAgent.match('Windows')) {
        return 'windows';
    }
    if (userAgent.match('Android')) {
        return 'android';
    }
    if (userAgent.match('iPhone')) {
        return 'iPhone';
    }
    if (userAgent.match('iPad')) {
        return 'iPad';
    }
    return 'unknown';
};

const allMsg = {};

window.NPL = {
    activate: (filename, msg) => {
        if (GetSystem() === 'iPhone' || GetSystem() === 'macos') {
            const params = { filename, msg };
            params.msg = JSON.stringify(msg);
            if (window.webkit &&
                window.webkit.messageHandlers &&
                window.webkit.messageHandlers.activate &&
                window.webkit.messageHandlers.activate.postMessage) {
                window.webkit.messageHandlers.activate.postMessage(params);
            }

        } else if (GetSystem() === 'windows') { //window.chrome.webview.postMessage( //window.chrome.webview.addEventListener
            if (window.chrome &&
                window.chrome.webview &&
                window.chrome.webview.postMessage) {
                const params = { filename, msg };
                // params.msg = JSON.stringify(msg);
                window.chrome.webview.postMessage(JSON.stringify(params))
                return
            }
            if (!window.is_edge_browser)
            {
                // window webview 不支持此方式通信, 执行此代码会导致页面异常
                const params = { filename, msg };
                window.location.href = 'paracraft://sendMsg?' + JSON.stringify(params);
            }
        } else if (GetSystem() === 'emscripten' && window.NPL.postMessage) {
            window.NPL.postMessage({ filename, msg });
        } else if (GetSystem() === 'android') {
            msg = JSON.stringify(msg);
            android.nplActivate(filename, msg);
        }
    },
    this: (callback, params) => {
        if (params && params.filename) {
            allMsg[params.filename] = callback;
        }
    },
    receive: (filename, msg, version) => {
        if (window.chrome && window.chrome.webview && window.chrome_webview_message_version != version) return;
        if (allMsg[filename]) {
            allMsg[filename](msg);
        }
    },
};

// emscripten
if ((!window.chrome || !window.chrome.webview) && !window.android && !window.webkit) {
    window.addEventListener("message", function (event) {
        const msg = event.data;
        if (!msg.is_paracraft_message) return;
        const cmd = msg.cmd;
        if (cmd == "load") {
            if (msg.platform) window.paracraft_platform = msg.platform;
            // 回一个确定消息
            event.source.postMessage(msg, "*");

            window.NPL.postMessage = function (msg) {
                msg.is_paracraft_message = true;
                msg.cmd = "PostMessage";
                event.source.postMessage(msg, "*");
            }
        }
        else if (cmd == "PostMessage") {
            window.NPL.receive(msg.filename, msg.msg);
        }
    });
}

class NPLJS {
    constructor() {
        this.m_msg_callbacks = {};
        this.m_ready = false;
        this.m_msgid = 0;
        this.m_msgid_callbacks = {};
        this.m_loaded = false;
        this.m_retry_callback = {};
        this.m_onload_callback = [];
    }

    IsLoaded() {
        return this.m_loaded;
    }

    Load() {
        if (this.m_loaded) return;
        const self = this;
        self.SendMsg("load");
        self.m_onload_timerid = setInterval(function () {
            self.SendMsg("load");
        }, 200);
    }

    HandleLoadMsg() {
        if (this.m_loaded) return;
        console.log("==========================NPLJS Loaded=========================", window.chrome && window.chrome.webview);
        clearInterval(this.m_onload_timerid);
        this.m_loaded = true;
        this.SendMsg("load");

        this.m_onload_callback.forEach(function (callback) {
            callback();
        });
    }

    OnLoad(callback) {
        if (this.IsLoaded()) return callback();
        this.m_onload_callback.push(callback);
    }

    GetNextMsgId() {
        this.m_msgid = this.m_msgid + 1;
        return `js_${this.m_msgid}`;
    }

    Send(msg) {
        window.NPL.activate("NPLJS", msg);
    }

    SendMsg(msgname, msgdata, msgid, callback) {
        if (!msgid) msgid = this.GetNextMsgId();
        if (typeof (callback) == "function") this.m_msgid_callbacks[msgid] = callback;

        this.Send({
            msgid: msgid,
            msgname: msgname,
            msgdata: msgdata,
        });
    }

    RecvMsg(msg) {
        const msgid = msg.msgid;
        const msgname = msg.msgname;
        const msgdata = msg.msgdata;
        const msgid_callback = this.m_msgid_callbacks[msgid];

        if (msgid_callback) {
            msgid_callback(msgdata);
            delete (this.m_msgid_callbacks[msgid]);
        }

        if (msg.request_reply && (msgid_callback || this.m_msg_callbacks[msgname])) {
            msg.response_reply = true;
            this.Send(msg);
        }

        if (msgname == "load") {
            this.HandleLoadMsg();
        }
        else {
            this.EmitMsg(msgname, msgdata, msgid);
        }
    }

    OnMsg(msgname, callback) {
        if (this.m_msg_callbacks[msgname] == undefined) this.m_msg_callbacks[msgname] = {};
        this.m_msg_callbacks[msgname][callback] = callback;
    }

    OffMsg(msgname, callback) {
        if (this.m_msg_callbacks[msgname] == undefined) this.m_msg_callbacks[msgname] = {};
        delete this.m_msg_callbacks[msgname][callback];
    }

    EmitMsg(msgname, msgdata, msgid) {
        let callbacks = this.m_msg_callbacks[msgname];
        for (let key in callbacks) {
            let callback = callbacks[key];
            if (callback) {
                callback(msgdata, msgid);
            }
        }
    }
};

window.NPLJSInstance = new NPLJS();
window.local_debug = window.location.hostname == "127.0.0.1" || window.location.hostname == "localhost";
window.paracraft_platform = GetSystem();
window.is_edge_browser = navigator.userAgent.includes('Edg');

// DEBUG
// if (window.local_debug && !window.is_edge_browser) window.NPLJSInstance = undefined;

let is_received = false;
NPL.this(function (msg) {
    is_received = true;
    try {
        msg = JSON.parse(msg);
        if (typeof (msg.msgdata) == "string") {
            msg.msgdata = atob(msg.msgdata);
            msg.msgdata = JSON.parse(msg.msgdata);
        }
    } catch (e) {
        console.log(e);
        console.log(msg);
        return;
    }

    window.NPLJSInstance.RecvMsg(msg);
}, { filename: "NPLJS" });

window.addEventListener("load", function () {
    console.log("============================platform==========================", window.paracraft_platform, window.chrome && window.chrome.webview);
    if (window.paracraft_platform == "windows" && window.is_edge_browser && window.chrome && window.chrome.webview == undefined && window.chrome.app !== undefined && window.chrome.csi !== undefined) {
        window.sessionStorage.setItem("refresh_count", parseInt(window.sessionStorage.getItem("refresh_count") || "0") + 1);
        window.location.reload();
    }

    if (window.paracraft_platform == "windows" && window.is_edge_browser) {
        setTimeout(function () {
            if (!is_received) {
                console.log("NPLJS not received");
                window.location.reload();
            }
        }, 3000);
    }

    if (window.NPLJSInstance) {
        if (window.local_debug && window.paracraft_platform == "windows" && !window.is_edge_browser) return; // debug
        window.NPLJSInstance.Load();
    }
});