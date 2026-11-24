
var Module = typeof Module != 'undefined' ? Module : {};

class Emscripten {
    constructor() {
        this.m_msg_callbacks = {};
        this.m_ready = false;
        this.m_msgid = 0;
        this.m_msgid_callbacks = {};
        this.m_loaded = false;

        let self = this;

        // this.m_load_timer = setInterval(function () {
        //     try {
        //         self.SendMsg("load");
        //     } catch (e) {
        //     }
        // }, 100);
    }

    IsLoaded() {
        return this.m_loaded;
    }

    OnLoad() {
        if (this.m_loaded) return;
        this.SendMsg("load");
        console.log("==========================ParaCraft Emscripten Loaded=========================");
        this.m_loaded = true;
        clearInterval(this.m_load_timer);

        //         const self = this;
        //         setTimeout(function () {
        //             self.ExecuteLuaCode(`
        // local Emscripten = NPL.load("(gl)script/apps/Aries/Creator/Game/Emscripten/Emscripten.lua");
        // Emscripten:SendMsg("Py2Lua", [[
        // def max(a, b):
        //     if a > b:
        //         return a
        //     else:
        //         return b    
        // ]], nil, function(result)
        //     echo(result, true);
        // end);
        //                     `);
        //         }, 1000);
    }

    GetNextMsgId() {
        this.m_msgid = this.m_msgid + 1;
        return this.m_msgid;
    }

    SendMsg(msgname, msgdata, msgid, callback) {
        if (!msgid) msgid = this.GetNextMsgId();
        if (typeof (callback) == "function") this.m_msgid_callbacks[msgid] = callback;

        SendMsgToEmscripten({
            filename: "emscripten",
            // filename: "script/apps/Aries/Creator/Game/Emscripten/Emscripten.lua",
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
        if (msgid_callback) msgid_callback(msgdata);

        if (msgname == "echo") {
            console.log(msg)
        }
        else if (msgname == "echo_and_reply") {
            console.log(msg);
            this.SendMsg("echo", msgdata);
        }
        else if (msgname == "load") {
            this.OnLoad();
        }
        else if (msgname == "Py2Lua") {
            this.ExecutePy2Lua(msgdata, msgid);
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

    ExecuteLuaCode(code, option, callback) {
        this.SendMsg("execute", { code: code, option: option }, undefined, callback);
        // const base64 = this.base64 ? this.base64 : btoa;
        // this.SendMsg("execute", { code: base64(code), option: option }, undefined, callback);
    }

    ExecuteCodeBlockCode(options, callback) {
        const language = options.language;
        if (language == "python_wasm") {
            this.ExecutePythonCode(options.code, callback);
        } else {
            this.SendMsg("ExecuteCodeBlockCode", options, undefined, callback);
        }
    }

    async LoadPy2Lua(callback) {
        const self = this;
        if (window.Py2LuaInstance) {
            await window.Py2LuaInstance.Load();
            callback(window.Py2LuaInstance);
        } else {
            const timer = setInterval(function () {
                if (!window.Py2LuaInstance) return;
                clearInterval(timer);
                self.LoadPy2Lua(callback);
            }, 100);
        }
    }

    async ExecutePythonCode(pycode, callback) {
        await this.LoadPy2Lua(async function (Py2LuaInstance) {
            const output_text = await Py2LuaInstance.ExecutePythonCode(pycode);
            const output_image = Py2LuaInstance.GetImageBase64Content();
            callback({ output: output_text, output_image: output_image });
        });
    }

    async ExecutePy2Lua(msgdata, msgid) {
        const self = this;
        await this.LoadPy2Lua(function () {
            self.SendMsg("Py2LuaReply", window.Py2Lua(msgdata), msgid);
        });
    }
};

let EmscriptenInstance = new Emscripten();
// 此对象用于为ParaEngine提供接口, 外部网页端请勿使用, 外部网页端检测和使用 window.EmscriptenInstance 对象
window.EmscriptenInstanceApi = EmscriptenInstance;

EmscriptenInstance.OnMsg("Function_PlayText", function(msgdata, msgid) {
    // 检查浏览器是否支持 SpeechSynthesis API
    if ('speechSynthesis' in window) {
        // 创建一个新的 SpeechSynthesisUtterance 对象
        const msg = new SpeechSynthesisUtterance();
    
        // 设置要转换为语音的文本
        msg.text = msgdata.text || 'Hello, this is a text-to-speech example.';
    
        // 设置语速（范围：0.1 - 10）[-10, 10]
        if (msgdata.rate) {
            msg.rate = msgdata.rate < 0 ? (msgdata.rate / 10 + 1) : msgdata.rate;   // 默认值为1，正常语速
            msg.rate = msg.rate < 0.1 ? 0.1 : msg.rate;
        }
    
        // 设置音量（范围：0 - 1）[0 100]
        if (msgdata.volume) {
            msg.volume = msgdata.volume / 100; // 默认值为1，最大音量
        }
        
        // 设置音调（范围：0 - 2）[-10, 10]
        if (msgdata.pitch) {
            msg.pitch = (msgdata.pitch + 10) / 10;
        }
    
        // 监听播放完成事件
        msg.onend = function(event) {
            console.log('Speech playback finished.');
            EmscriptenInstance.SendMsg("Function_PlayTextReply", {}, msgid);
        };
        
        // 使用默认语音合成器进行播放
        speechSynthesis.speak(msg);
    } else {
        console.log('Speech synthesis not supported.');
    }
});

const InitEmscriptenInstance = function () {
    EmscriptenInstance.base64 = function (str) {
        try {
            return btoa(str);
        } catch (e) {
            return Module.ParaMisc_base64 ? Module.ParaMisc_base64(str) : 'print("invalid code!!!")';
        }
    }

    EmscriptenInstance.unbase64 = function (str) {
        try {
            return atob(str);
        } catch (e) {
            return Module.ParaMisc_unbase64 ? Module.ParaMisc_unbase64(str) : "";
        }
    }
    console.log("EmscriptenInstance.base64 中国: ", EmscriptenInstance.base64("中国"));
}

if (Module.isRuntimeInitialized) {
    window.EmscriptenInstance = EmscriptenInstance;
    // window.top.EmscriptenInstance = EmscriptenInstance;
    InitEmscriptenInstance();
}
else {
    Module.onRuntimeInitializedCallbacks.push(function () {
        try {
            window.EmscriptenInstance = EmscriptenInstance;
            // window.top.EmscriptenInstance = EmscriptenInstance;
            InitEmscriptenInstance();
        } catch (e) {
            console.log(e);
        }
    })
}

class EmscriptenWebSocket {
    static s_id = 0;
    static s_sockets = {};

    async LoadSocketIO() {
        if (window.io) return;

        const hostname = window.location.hostname;
        const is_local_debug = hostname == "localhost" || hostname == "127.0.0.1";
        const src = (window.WEBPARACRAFT_ORIGIN + "libs/socket.io-2.2.0.js") || "https://webparacraft.keepwork.com/libs/socket.io-2.2.0.js";
        return new Promise(function (resolve) {
            const script = document.createElement('script');
            script.src = src;
            script.onload = resolve;
            document.body.appendChild(script);
        });
    }

    static Init() {
        EmscriptenInstance.OnMsg("WebSocket", function (msgdata, msgid) {
            const id = msgdata.id;
            const action = msgdata.action;
            const data = msgdata.data;
            if (action == "open") {
                const socket = new EmscriptenWebSocket();
                socket.m_id = id || socket.m_id;
                socket.Open(data);
            } else {
                const socket = EmscriptenWebSocket.s_sockets[id];
                if (socket) {
                    if (action == "send") {
                        socket.Send(data);
                    } else if (action == "close") {
                        socket.Close(data);
                    }
                }
            }
        });
    }
    constructor() {
        this.m_socket = null;
        this.m_url = null;
        this.m_id = (EmscriptenWebSocket.s_id + 1).toString();
    }

    async Open(msg) {
        await this.LoadSocketIO();
        if (!window.io) return;
        const socket = this;
        const query = {};
        msg.token && (query.token = msg.token);
        msg.platform && (query.platform = msg.platform);
        msg.machineCode && (query.machineCode = msg.machineCode);
        this.m_socket = io(msg.url, {
            path: msg.path || "/socket.io/",
            query: query,
            transports: ['websocket'],
        });
        this.m_socket.on('connect', () => { socket.OnConnect(); });
        this.m_socket.on('disconnect', () => { socket.OnClose(); });
        this.m_socket.on('error', () => { socket.OnClose(); });

        this.m_socket.on('ping', (...args) => { socket.OnMsg('ping', ...args); });
        this.m_socket.on('msg', (...args) => { socket.OnMsg('msg', ...args); });
        this.m_socket.on('broadcast', (...args) => { socket.OnMsg('broadcast', ...args); });
        this.m_socket.on('paracraftGlobal', (...args) => { socket.OnMsg('paracraftGlobal', ...args); });
        this.m_socket.on('app/msg', (...args) => { socket.OnMsg('app/msg', ...args); });
        this.m_socket.on('join/ack', (...args) => { socket.OnMsg('join/ack', ...args); });
        // this.m_socket.onAny(function (...args) {
        //     console.log("WebSocket 收到消息:", event, args);
        //     socket.OnMsg(event, ...args);
        // });
    }

    OnMsg(...args) {
        const params = []; // 创建空数组

        for (var i = 0; i < arguments.length; i++) {
            params.push(arguments[i]); // 将参数添加到数组中
        }

        // console.log("WebSocket 收到消息:", params);

        const msg = {
            action: "message",
            id: this.m_id,
            data: params
        };

        EmscriptenInstance.SendMsg("WebSocket", msg);
    }

    OnConnect() {
        // console.log("已成功连接到 WebSocket 服务器");
        EmscriptenWebSocket.s_sockets[this.m_id] = this;

        EmscriptenInstance.SendMsg("WebSocket", {
            action: "open",
            id: this.m_id,
        });
    }

    OnClose() {
        // console.log("已断开与 WebSocket 服务器的连接");
        EmscriptenWebSocket.s_sockets[this.m_id] = undefined;

        EmscriptenInstance.SendMsg("WebSocket", {
            action: "close",
            id: this.m_id,
        });
    }

    Send(msg) {
        if (this.m_socket) {
            this.m_socket.emit(...msg);
        }
    }

    Close() {
        if (this.m_socket) {
            this.m_socket.close();
        }
    }
}

EmscriptenWebSocket.Init();
window.socket_io_test = function () {
    const socket = new EmscriptenWebSocket();
    socket.Open({
        url: "wss://im.keepwork.com",
        token: "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VySWQiOjMsInVzZXJuYW1lIjoieGlhb3lhbyIsImNlbGxwaG9uZSI6IjE4NzAyNzU5Nzk2IiwicGxhdGZvcm0iOiJFRFUiLCJyb2xlSWQiOjYsIm9yZ0lkIjoxLCJtYWNoaW5lQ29kZSI6ImFjM2JkYjUwLWU3YTgtNDQ2MS1iODA1LTMyMzA2MmUxZjNlNiIsImlhdCI6MTcxNzQ2NzkwMn0.dseeIylFMkUIgPD0oKikVV_f31OVydLzPcadjDsn0JM",
        platform: "web",
        platform: "EDU",
        machineCode: "ac3bdb50-e7a8-4461-b805-323062e1f3e6",
    });
}

class EmscriptenMQTT {
    static s_id = 0;
    static s_mqtts = {};

    static OnEmscriptenInstanceMsg(msgdata, msgid) {
        const id = msgdata.id;
        const action = msgdata.action;
        const data = msgdata.data;

        if (!EmscriptenMQTT.s_mqtts[id]) EmscriptenMQTT.s_mqtts[id] = new EmscriptenMQTT();
        const self = EmscriptenMQTT.s_mqtts[id];
        self.m_id = id;

        if (action == "connect")
            self.Connect(data, msgid);
        else if (action == "subscribe") {
            self.Subscribe(data);
        } else if (action == "unsubscribe") {
            self.Unsubscribe(data);
        } else if (action == "publish") {
            self.Publish(data);
        } else if (action == "close") {
            self.Close();
        }
    }

    static Init() {
        EmscriptenInstance.OnMsg("MQTT", EmscriptenMQTT.OnEmscriptenInstanceMsg);
    }

    constructor() {
        this.m_mqtt = null;
        this.m_url = null;
        this.m_id = (EmscriptenMQTT.s_id + 1).toString();
        this.m_connected = false;
        this.m_connect_success = false;
        this.m_subscribes = {};
    }

    async LoadMQTT() {
        if (window.mqtt) return;

        const hostname = window.location.hostname;
        const is_local_debug = hostname == "localhost" || hostname == "127.0.0.1";
        const src = window.location.origin + (is_local_debug ? "/npl/webparacraft" : "") + "/libs/mqtt.min.js";
        return new Promise(function (resolve) {
            const script = document.createElement('script');
            script.src = src;
            script.onload = resolve;
            document.body.appendChild(script);
        });
    }

    async Connect(msg) {
        await this.LoadMQTT("libs/mqtt.min.js");

        const self = this;
        if (self.m_connected) {
            self.Close();
            if (self.m_connect_success) {
                self.OnConnect();
            }
            return;
        }

        const options = {
            // keepalive: msg.keepalive || 60,
            keepalive: 60,
            clientId: msg.clientId,
            username: msg.username,
            password: msg.password,
            protocolId: 'MQTT',
            protocolVersion: 4,
            clean: true,
            reconnectPeriod: 3000,
            connectTimeout: 30 * 1000,
        }

        console.log('Connecting mqtt client', self.m_id);
        self.m_mqtt = window.mqtt.connect(msg.url || 'ws://mqtt.keepwork.com:8083/mqtt', options);
        self.m_mqtt.on("connect", () => self.OnConnect());
        self.m_mqtt.on('message', (topic, message) => self.OnMessage(topic, message));
        self.m_mqtt.on('close', () => self.OnClose());
        self.m_mqtt.on('error', (err) => { console.log('Connection error: ', err); });
        self.m_connected = true;
    }

    Subscribe(msg) {
        this.m_subscribes[msg.topic] = true;
        this.m_mqtt.subscribe(msg.topic, { qos: msg.qos || 0 });
    }

    Unsubscribe(msg) {
        this.m_subscribes[msg.topic] = false;
        this.m_mqtt.unsubscribe(msg.topic);
    }

    Publish(msg) {
        let topic = msg.topic;
        let data = msg.payload || msg.data;
        if (typeof data !== 'string') {
            data = JSON.stringify(data);
        }
        this.m_mqtt.publish(topic, data, { qos: msg.qos || 0 });
    }

    Close() {
        for (let topic in this.m_subscribes) {
            if (this.m_subscribes[topic]) {
                this.m_mqtt.unsubscribe(topic);
                this.m_subscribes[topic] = false;
            }
        }
        // this.m_mqtt.end(true);
    }

    OnConnect() {
        console.log('Client connected');
        this.m_connect_success = true;
        EmscriptenInstance.SendMsg("MQTT", {
            action: "connect",
            id: this.m_id,
        });
    }

    OnMessage(topic, message) {
        console.log('Received Message: ' + message.toString() + '\nOn topic: ' + topic);
        let data = message.toString();
        try {
            data = JSON.parse(message.toString());
        } catch (e) {
            data = message.toString();
        }
        EmscriptenInstance.SendMsg("MQTT", {
            action: "message",
            id: this.m_id,
            topic: topic,
            data: data,
        });
    }

    OnClose() {
        console.log("mqtt close");
        this.m_connect_success = false;
        if (this.m_connected) {
            EmscriptenInstance.SendMsg("MQTT", {
                action: "close",
                id: this.m_id,
            });
        }
    }
}

EmscriptenMQTT.Init();

class NplWebserverBridge {
    static UseRelativeServiceWorker() {
        const hostname = window.location.hostname;
        const isLocalWebParacraft = hostname === "localhost" || hostname === "127.0.0.1";
        const isHostedWebParacraft = hostname === "webparacraft.keepwork.com" || hostname === "emscripten.keepwork.com";
        return !isLocalWebParacraft && !isHostedWebParacraft;
    }

    static Init() {
        if (!("serviceWorker" in navigator)) return;
        const workerVersion = window.WEBPARACRAFT_VERSION ? "?v=" + encodeURIComponent(window.WEBPARACRAFT_VERSION) : "";
        const useRelativeServiceWorker = NplWebserverBridge.UseRelativeServiceWorker();
        const defaultWorkerUrl = useRelativeServiceWorker
            ? new URL("ServiceWorker" + workerVersion, window.location.href).href
            : new URL("/ServiceWorker.js" + workerVersion, window.location.origin).href;
        const defaultWorkerScope = useRelativeServiceWorker ? new URL("./", window.location.href).pathname : "/";
        const workerUrl = window.WEBPARACRAFT_SERVICE_WORKER_URL || defaultWorkerUrl;
        const workerScope = window.WEBPARACRAFT_SERVICE_WORKER_SCOPE || defaultWorkerScope;
        const expectedScope = new URL(workerScope, window.location.origin).href;
        navigator.serviceWorker.register(workerUrl).then(async function (rootRegistration) {
            const registrations = await navigator.serviceWorker.getRegistrations();
            let removedLegacyScope = false;
            for (const registration of registrations) {
                const worker = registration.active || registration.waiting || registration.installing;
                if (registration.scope !== expectedScope && worker && new URL(worker.scriptURL).pathname.endsWith("/ServiceWorker.js")) {
                    removedLegacyScope = (await registration.unregister()) || removedLegacyScope;
                }
            }
            const controller = navigator.serviceWorker.controller;
            if (removedLegacyScope && controller && controller.scriptURL !== workerUrl) {
                if (!sessionStorage.getItem("webparacraft.rootServiceWorkerReload")) {
                    sessionStorage.setItem("webparacraft.rootServiceWorkerReload", "1");
                    await rootRegistration.update();
                    window.location.replace(window.location.href);
                }
            } else if (controller && controller.scriptURL === workerUrl) {
                sessionStorage.removeItem("webparacraft.rootServiceWorkerReload");
            }
        }).catch(function () {});
        const bridge = new NplWebserverBridge();
        window.NplWebserverBridge = bridge;
        navigator.serviceWorker.addEventListener("message", function (event) {
            bridge.OnServiceWorkerMessage(event);
        });
        navigator.serviceWorker.addEventListener("controllerchange", function () {
            bridge.Connect();
        });
        navigator.serviceWorker.ready.then(function () {
            bridge.Connect();
        });
    }

    Connect() {
        const controller = navigator.serviceWorker.controller;
        if (!controller) return;
        controller.postMessage({ type: "npl-webserver-register" });
    }

    Route(path) {
        const routePath = path && path.startsWith("/") ? path : "/" + (path || "");
        if (NplWebserverBridge.UseRelativeServiceWorker()) {
            return new URL("__npl_webserver__" + routePath, new URL("./", window.location.href)).href;
        }
        return new URL(routePath, window.location.origin).href;
    }

    OnServiceWorkerMessage(event) {
        const data = event.data || {};
        if (data.type !== "npl-webserver-request" || !event.ports || !event.ports[0]) return;
        const port = event.ports[0];
        setTimeout(function () {
            EmscriptenInstance.SendMsg("paracraft-cli", {
                v: 1,
                action: "http_request",
                replyTarget: "emscripten",
                params: data.request || {},
            }, undefined, function (reply) {
                const response = reply && reply.result ? reply.result : reply;
                if (!response) port.postMessage({ error: "empty NPL WebServer response" });
                else if (reply && reply.ok === false && !reply.result) port.postMessage({ error: reply.error || "NPL WebServer request failed" });
                else port.postMessage({ response: response });
            });
        }, 0);
    }
}

NplWebserverBridge.Init();

class KeepworkHub {
    static instance = null;

    static Init() {
        KeepworkHub.instance = new KeepworkHub();
        EmscriptenInstance.OnMsg("KeepworkHub", function (msgdata) {
            KeepworkHub.instance.OnLua(msgdata || {});
        });
    }

    constructor() {
        this.running = false;
        this.host = "http://127.0.0.1:8089";
        this.identity = null;
        this.clientId = "";
        this.pollAbort = null;
        this.hbTimer = null;
        this.pollGen = 0;
        this.sentRegistered = false;
        this.lastRoot = "";
    }

    OnLua(msg) {
        const action = msg.action;
        if (action === "start") this.Start(msg);
        else if (action === "stop") this.Stop();
        else if (action === "results") this.PostResults(msg.results);
        else if (action === "identity") {
            this.identity = msg.identity || this.identity;
            this.Register().catch(function () { /* heartbeat */ });
        }
    }

    // SendMsgToEmscripten is a sync C++ call. Never invoke it from a fetch
    // microtask: that re-enters NPL on the render thread and hitchs WebGL.
    yieldToRenderer() {
        return new Promise(function (resolve) {
            requestAnimationFrame(function () { setTimeout(resolve, 0); });
        });
    }

    SendNow(msg) {
        if (window.EmscriptenInstance) EmscriptenInstance.SendMsg("KeepworkHub", msg);
    }

    // Jobs only: never from a fetch microtask (that re-enters NPL mid-frame).
    // Handshake (registered/down) must be SendNow so F11 does not miss the wiki root.
    SendAfterPaint(msg) {
        const self = this;
        return this.yieldToRenderer().then(function () {
            self.SendNow(msg);
        });
    }

    async fetchJson(path, opts, timeoutMs) {
        const ctrl = new AbortController();
        const timer = setTimeout(function () { ctrl.abort(); }, timeoutMs || 20000);
        if (opts && opts.signal) {
            opts.signal.addEventListener("abort", function () { ctrl.abort(); });
        }
        try {
            const res = await fetch(this.host + path, Object.assign({
                headers: { Accept: "application/json", "Content-Type": "application/json" },
                signal: ctrl.signal,
            }, opts || {}));
            const data = await res.json().catch(function () { return {}; });
            return { ok: res.ok, status: res.status, data: data };
        } finally {
            clearTimeout(timer);
        }
    }

    async Start(msg) {
        this.Stop();
        this.running = true;
        const host = String(msg.host || this.host || "").replace(/\/$/, "");
        if (host) this.host = host;
        this.identity = msg.identity || {};
        this.clientId = String(this.identity.clientId || "");
        this.sentRegistered = false;
        this.lastRoot = "";
        try {
            const health = await this.fetchJson("/health", { method: "GET", headers: { Accept: "application/json" } }, 1200);
            const d = health.data || {};
            if (!health.ok || d.name !== "keepwork-mcp" || !d.webserverBase) {
                this.SendNow({ action: "down" });
                this.running = false;
                return;
            }
            const origin = String(d.webserverBase).match(/^(https?:\/\/[^/]+)/);
            if (origin) this.host = origin[1];
            const reg = await this.Register();
            if (!reg) {
                this.SendNow({ action: "down" });
                this.running = false;
                return;
            }
            this.Heartbeat();
            this.PollLoop();
        } catch (e) {
            this.SendNow({ action: "down" });
            this.running = false;
        }
    }

    async Register() {
        if (!this.identity) return false;
        const reg = await this.fetchJson("/paracraft/register", {
            method: "POST",
            body: JSON.stringify(this.identity),
        }, 8000);
        if (!reg.ok || (reg.data && reg.data.ok === false)) return false;
        this.clientId = String(this.identity.clientId || (reg.data && reg.data.clientId) || this.clientId);
        const root = reg.data && reg.data.webserverRoot;
        const inst = reg.data && reg.data.webserverInstance;
        if (!this.sentRegistered || root !== this.lastRoot) {
            this.sentRegistered = true;
            this.lastRoot = root || "";
            // Immediate: F11 waits on Lua register waiters / site_url.
            this.SendNow({
                action: "registered",
                ok: true,
                webserverRoot: root,
                webserverInstance: inst,
            });
        }
        return true;
    }

    Stop() {
        this.running = false;
        this.pollGen += 1;
        if (this.pollAbort) {
            try { this.pollAbort.abort(); } catch (e) { /* ignore */ }
            this.pollAbort = null;
        }
        if (this.hbTimer) {
            clearTimeout(this.hbTimer);
            this.hbTimer = null;
        }
        const id = this.clientId;
        if (id) {
            fetch(this.host + "/paracraft/unregister", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ clientId: id }),
            }).catch(function () { /* ignore */ });
        }
    }

    Heartbeat() {
        const self = this;
        if (this.hbTimer) clearTimeout(this.hbTimer);
        this.hbTimer = setTimeout(function () {
            if (!self.running) return;
            self.Register().catch(function () { /* ignore */ }).then(function () {
                if (self.running) self.Heartbeat();
            });
        }, 10000);
    }

    async PollLoop() {
        const gen = this.pollGen;
        while (this.running && gen === this.pollGen) {
            this.pollAbort = new AbortController();
            try {
                const id = this.clientId;
                if (!id) {
                    await this.sleep(1000);
                    continue;
                }
                const started = Date.now();
                const res = await fetch(this.host + "/paracraft/" + encodeURIComponent(id) + "/jobs/poll", {
                    method: "POST",
                    headers: { Accept: "application/json", "Content-Type": "application/json" },
                    body: JSON.stringify({ waitMs: 10000 }),
                    signal: this.pollAbort.signal,
                });
                if (!this.running || gen !== this.pollGen) return;
                if (res.status === 404) {
                    await this.Register();
                    await this.sleep(500);
                    continue;
                }
                if (!res.ok) {
                    await this.sleep(2000);
                    continue;
                }
                const data = await res.json().catch(function () { return {}; });
                const jobs = data && Array.isArray(data.jobs) ? data.jobs : [];
                if (jobs.length) {
                    // Whole batch after paint. One-job-per-frame starved the wiki
                    // iframe (console HTML/CSS/JS never finished in time).
                    await this.SendAfterPaint({ action: "jobs", jobs: jobs });
                } else if (Date.now() - started < 200) {
                    // Hub returned immediately (kicked poller / empty wait).
                    // Do not spin fetch+json on the main thread.
                    await this.sleep(250);
                }
                await this.yieldToRenderer();
            } catch (e) {
                if (!this.running || gen !== this.pollGen) return;
                await this.sleep(2000);
            }
        }
    }

    PostResults(results) {
        const id = this.clientId;
        if (!id || !results || !results.length) return;
        fetch(this.host + "/paracraft/" + encodeURIComponent(id) + "/jobs/results", {
            method: "POST",
            headers: { Accept: "application/json", "Content-Type": "application/json" },
            body: JSON.stringify({ results: results }),
        }).catch(function () { /* ignore */ });
    }

    sleep(ms) {
        return new Promise(function (resolve) { setTimeout(resolve, ms); });
    }
}

KeepworkHub.Init();

class EmscriptenBlueTooth {
    static s_instances = {};

    static OnEmscriptenInstanceMsg(msgdata, msgid) {
        const id = msgdata.id || 0;
        const action = msgdata.action;

        const self = EmscriptenBlueTooth.s_instances[id] || new EmscriptenBlueTooth(id);

        if (action == "connect")
            self.Connect(msgdata);
        else if (action == "send") {
            self.Send(msgdata);
        } else if (action == "close") {
            self.Close();
        }
    }

    static Init() {
        EmscriptenInstance.OnMsg("BlueTooth", EmscriptenBlueTooth.OnEmscriptenInstanceMsg);
        EmscriptenInstance.OnMsg("BluetoothDiscovery", EmscriptenBlueTooth.Discover);
        EmscriptenInstance.OnMsg("BluetoothGatt", msg => EmscriptenBlueTooth.HandleGatt(msg));
    }

    static selectedDevices = new Map();
    static chooserGeneration = 0;
    static gattSession = null;
    static pendingGatt = new WeakMap();

    static GattReply(session, status, fields = {}) {
        setTimeout(() => EmscriptenInstance.SendMsg("BluetoothGatt", { request: session.request, status, ...fields }), 0);
    }

    static ReleaseGatt(session) {
        if (this.gattSession !== session) return;
        this.gattSession = null;
        session.device.removeEventListener("gattserverdisconnected", session.onLost);
        session.device.gatt.disconnect();
    }

    static async WriteGatt(current, { request, service, characteristic, operation, hex, allowWithoutResponse }) {
        const target = current?.writable.get(service + "/" + characteristic);
        let status = "writeFailed";
        if (current && current.request === request && current.device.gatt.connected && target && !current.writing &&
            (target.properties.write || (allowWithoutResponse === true && target.properties.writeWithoutResponse)) &&
            typeof hex === "string" && hex.length <= 1024 && /^(?:[0-9a-f]{2})+$/i.test(hex)) {
            current.writing = true;
            try {
                const bytes = Uint8Array.from(hex.match(/../g), pair => parseInt(pair, 16));
                if (target.properties.write) await target.writeValueWithResponse(bytes);
                else await target.writeValueWithoutResponse(bytes);
                status = "written";
            } catch {
                // The caller keeps track of the last acknowledged packet.
            } finally {
                current.writing = false;
            }
        }
        this.GattReply({ request }, status, { operation });
    }

    static HandleGatt(message) {
        const { cmd, request, id } = message;
        const current = this.gattSession;
        if (cmd === "gattWrite") return this.WriteGatt(current, message);
        if (cmd === "gattDisconnect") {
            if (current && current.request === request) this.ReleaseGatt(current);
            this.GattReply({ request }, "disconnected");
            return;
        }
        if (cmd !== "gattConnect") return;
        const device = this.selectedDevices.get(id);
        if (!device?.gatt) return this.GattReply({ request }, "unavailable");
        if (current) this.ReleaseGatt(current);
        const session = { request, device, writable: new Map() };
        session.onLost = () => {
            if (this.gattSession !== session || device.gatt.connected) return;
            this.ReleaseGatt(session);
            this.GattReply(session, "disconnected");
        };
        this.gattSession = session;
        // A cancelled attempt can still resolve on the same device.
        const previous = this.pendingGatt.get(device) || Promise.resolve();
        this.pendingGatt.set(device, previous.then(() => this.OpenGatt(session)));
    }

    static async OpenGatt(session) {
        const { device } = session;
        if (this.gattSession !== session) return;
        try {
            const server = await device.gatt.connect();
            if (this.gattSession !== session) { server.disconnect(); return; }
            device.addEventListener("gattserverdisconnected", session.onLost);
            const services = [];
            let limited = false;
            try {
                for (const service of await server.getPrimaryServices()) {
                    if (this.gattSession !== session) return;
                    const row = { uuid: service.uuid, characteristics: [] };
                    services.push(row);
                    try {
                        for (const ch of await service.getCharacteristics()) {
                            const p = ch.properties;
                            if (p.write || p.writeWithoutResponse) session.writable.set(service.uuid + "/" + ch.uuid, ch);
                            const flags = [p.broadcast, p.read, p.writeWithoutResponse, p.write, p.notify, p.indicate, p.authenticatedSignedWrites, p.reliableWrite || p.writableAuxiliaries];
                            row.characteristics.push({ uuid: ch.uuid, properties: flags.reduce((bits, set, i) => set ? bits | (1 << i) : bits, 0) });
                        }
                    } catch { row.partial = true; }
                }
            } catch { limited = true; }
            if (this.gattSession !== session) return;
            if (!server.connected) return session.onLost();
            this.GattReply(session, "connected", { services, limited });
        } catch (error) {
            if (this.gattSession !== session) return;
            this.ReleaseGatt(session);
            this.GattReply(session, error.name === "SecurityError" || error.name === "NotAllowedError" ? "denied" : "unavailable");
        }
    }

    static async ReadAdvertisementRssi(device) {
        if (typeof device.watchAdvertisements !== "function" || typeof AbortController === "undefined") return;
        const controller = new AbortController();
        this.rssiWatch = controller;
        let listener, timer;
        try {
            return await new Promise(resolve => {
                listener = ({ rssi }) => {
                    if (Number.isInteger(rssi) && rssi >= -127 && rssi <= 20) resolve(rssi);
                };
                device.addEventListener("advertisementreceived", listener);
                controller.signal.addEventListener("abort", () => resolve(), { once: true });
                timer = setTimeout(resolve, 1500);
                device.watchAdvertisements({ signal: controller.signal }).catch(() => resolve());
            });
        } catch {
            // The chooser can succeed even when advertisement access is unavailable.
        } finally {
            clearTimeout(timer);
            device.removeEventListener("advertisementreceived", listener);
            controller.abort();
            if (this.rssiWatch === controller) this.rssiWatch = null;
        }
    }

    static async Discover({ request, serviceUuids, clear }) {
        const owner = EmscriptenBlueTooth;
        owner.rssiWatch?.abort();
        if (clear) {
            owner.chooserGeneration++;
            owner.selectedDevices.clear();
            return;
        }
        const generation = ++owner.chooserGeneration;
        // Deliver results after the Lua-to-JS call has unwound.
        const reply = (status, devices) => setTimeout(() => {
            EmscriptenInstance.SendMsg("BluetoothDiscovery", { request, status, devices });
        }, 0);
        if (!window.isSecureContext) return reply("insecure");
        if (!navigator.bluetooth?.requestDevice) return reply("unsupported");
        try {
            // Invoke the chooser while the panel click still provides user activation.
            const optionalServices = ["device_information", "battery_service"];
            for (let uuid of (serviceUuids || "").toLowerCase().split(/[\s,;，；]+/).filter(Boolean)) {
                if (/^(?:[0-9a-f]{4}|[0-9a-f]{8})$/.test(uuid))
                    uuid = uuid.padStart(8, "0") + "-0000-1000-8000-00805f9b34fb";
                if (!/^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/.test(uuid))
                    return reply("invalidService");
                optionalServices.push(uuid);
            }
            const choice = navigator.bluetooth.requestDevice({ acceptAllDevices: true, optionalServices });
            reply("choosing");
            const device = await choice;
            if (generation !== owner.chooserGeneration) return;
            owner.selectedDevices.set(device.id, device);
            const row = { id: device.id, name: device.name || "" };
            const rssi = await owner.ReadAdvertisementRssi(device);
            if (generation !== owner.chooserGeneration) return;
            if (rssi !== undefined) row.rssi = rssi;
            reply("ok", [row]);
        } catch (error) {
            const status = { NotFoundError: "cancelled", NotAllowedError: "denied", SecurityError: "denied" };
            reply(status[error.name] || "unavailable");
        }
    }

    constructor(id) {
        this.m_id = id || 0;
        this.m_service_uuid = null;
        this.m_name = null;
        this.m_device = null;
        this.m_server = null;
        this.m_characteristics = [];
        this.m_connected = false;

        EmscriptenBlueTooth.s_instances[this.m_id] = this;
    }

    async Connect(msgdata) {
        msgdata = msgdata || {};
        const name = msgdata.name || "paracraft_ble";
        const optionalServices = [0x181A, 0x182A];
        if (msgdata.service_uuid) optionalServices.push(msgdata.service_uuid);

        try {
            // 请求用户授权访问蓝牙设备
            this.m_device = await navigator.bluetooth.requestDevice({
                filters: [
                    { namePrefix: name }
                ],
                optionalServices: optionalServices,
            });

            // 监听设备的GATT连接状态变化
            this.m_device.addEventListener('gattserverdisconnected', () => {
                console.log('蓝牙设备已断开');
                this.OnClose();
            });

            // 连接选定的蓝牙设备
            this.m_server = await this.m_device.gatt.connect();
            this.m_characteristics = [];
            const services = await this.m_server.getPrimaryServices();
            for (let i = 0; i < services.length; i++) {
                const service = services[i];
                const characteristics = await service.getCharacteristics();
                for (let j = 0; j < characteristics.length; j++) {
                    const characteristic = characteristics[j];
                    characteristic.addEventListener('characteristicvaluechanged', e => this.Recv(e.target.value));
                    characteristic.startNotifications();
                    this.m_characteristics.push(characteristic);
                }
            }
            this.m_connected = true;
            console.log('bluetooth connected');
            this.OnConnect();
        } catch (error) {
            console.error('无法连接蓝牙设备:', error);
        }
    }

    async Send(msgdata) {
        const name = msgdata.name;
        const data = msgdata.data;
        const text = name + "|" + JSON.stringify(data);
        const encoder = new TextEncoder();
        const utf8Array = encoder.encode(text);
        for (let i = 0; i < this.m_characteristics.length; i++) {
            const characteristic = this.m_characteristics[i];
            await characteristic.writeValue(utf8Array);
        }
    }

    async Recv(utf8Array) {
        const decoder = new TextDecoder();
        const text = decoder.decode(utf8Array);
        const pos = text.indexOf("|");
        const name = text.substring(0, pos);
        let data = text.substring(pos + 1);
        try {
            data = JSON.parse(data);
        } catch (e) {
            data = data;
        }
        this.OnRecv(name, data);
    }

    async Close() {
        this.m_device.gatt.disconnect();
    }

    OnClose() {
        this.m_device = null;
        this.m_server = null;
        this.m_characteristics = [];
        this.m_connected = false;
        EmscriptenRawWebSocket.s_instances[this.m_id] = undefined;

        if (window.EmscriptenInstance) {
            window.EmscriptenInstance.SendMsg("BlueTooth", {
                id: this.m_id,
                action: "close",
            });
        }
        //if (window.NPLJSInstance) {
        //     window.NPLJSInstance.SendMsg("BlueTooth", {
        //         id: this.m_id,
        //         action: "close",
        //     });
        // }
    }

    OnRecv(name, data) {
        if (window.EmscriptenInstance) {
            EmscriptenInstance.SendMsg("BlueTooth", {
                id: this.m_id,
                action: "recv",
                name: name,
                data: data,
            });
        }
        // if (window.NPLJSInstance) {
        //     window.NPLJSInstance.SendMsg("BlueTooth", {
        //         id: this.m_id,
        //         action: "recv",
        //         name: name,
        //         data: data,
        //     })
        // }
    }

    OnConnect() {
        if (window.EmscriptenInstance) {
            window.EmscriptenInstance.SendMsg("BlueTooth", {
                id: this.m_id,
                action: "connect",
            });
        }
        // if (window.NPLJSInstance) {
        //     window.NPLJSInstance.SendMsg("BlueTooth", {
        //         id: this.m_id,
        //         action: "connect",
        //     });
        // }
    }
};

EmscriptenBlueTooth.Init();

class EmscriptenRawWebSocket {
    static s_msgname = "RawWebSocket";
    static s_instances = {};

    static OnEmscriptenInstanceMsg(msgdata, msgid) {
        const id = msgdata.id || 0;
        const action = msgdata.action;

        const self = EmscriptenRawWebSocket.s_instances[id] || new EmscriptenRawWebSocket(id);

        if (action == "connect")
            self.Connect(msgdata);
        else if (action == "send") {
            self.Send(msgdata);
        } else if (action == "close") {
            self.Close();
        }
    }

    static Init() {
        EmscriptenInstance.OnMsg(EmscriptenRawWebSocket.s_msgname, EmscriptenRawWebSocket.OnEmscriptenInstanceMsg);
        // if (window.NPLJSInstance) NPLJSInstance.OnMsg(EmscriptenRawWebSocket.s_msgname, EmscriptenRawWebSocket.OnEmscriptenInstanceMsg);
    }

    constructor(id) {
        this.m_id = id || 0;
        this.m_websocket = undefined;
        EmscriptenRawWebSocket.s_instances[this.m_id] = this;
    }

    GetMsgName() {
        return EmscriptenRawWebSocket.s_msgname;
    }

    async Connect(msgdata) {
        this.m_websocket = new WebSocket(msgdata.url);
        this.m_connected = false;

        const self = this;
        // 监听连接打开事件
        this.m_websocket.onopen = function () {
            // console.log("WebSocket连接已打开");
            self.OnConnect();
            self.m_connected = true;
        };

        // 监听接收消息事件
        this.m_websocket.onmessage = function (event) {
            // console.log(event.data);
            self.OnMessage(event.data);
        };

        // 监听连接关闭事件
        this.m_websocket.onclose = function () {
            // console.log("WebSocket连接已关闭");
            self.OnClose();
            self.m_connected = false;
        };

        // 监听发生错误事件
        this.m_websocket.onerror = function (error) {
            self.m_connected = false;
            console.error("发生错误: " + error);
        };
    }

    async Send(msgdata) {
        // console.log("send", msgdata.data);
        if (this.m_websocket && this.m_connected) this.m_websocket.send(JSON.stringify(msgdata.data));
    };

    async Close() {
        if (this.m_websocket) this.m_websocket.close();
    };

    OnConnect() {
        if (window.EmscriptenInstance) {
            window.EmscriptenInstance.SendMsg(this.GetMsgName(), {
                id: this.m_id,
                action: "connect",
            });
        }

        // if (window.NPLJSInstance) {
        //     window.NPLJSInstance.SendMsg(this.GetMsgName(), {
        //         id: this.m_id,
        //         action: "connect",
        //     });
        // }
    }

    OnMessage(msgdata) {
        // console.log("recv", msgdata);
        if (window.EmscriptenInstance) {
            window.EmscriptenInstance.SendMsg(this.GetMsgName(), {
                id: this.m_id,
                action: "message",
                data: msgdata,
            });
        }

        // if (window.NPLJSInstance) {
        //     window.NPLJSInstance.SendMsg(this.GetMsgName(), {
        //         id: this.m_id,
        //         action: "message",
        //         data: msgdata,
        //     });
        // }
    }

    OnClose() {
        this.m_websocket = null;
        EmscriptenRawWebSocket.s_instances[this.m_id] = undefined;

        if (window.EmscriptenInstance) {
            window.EmscriptenInstance.SendMsg(this.GetMsgName(), {
                id: this.m_id,
                action: "close",
            });
        }

        // if (window.NPLJSInstance) {
        //     window.NPLJSInstance.SendMsg(this.GetMsgName(), {
        //         id: this.m_id,
        //         action: "close",
        //     });
        // }
    }
};

EmscriptenRawWebSocket.Init();
