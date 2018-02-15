# WebSocket Reader/Writer
### References
 - The WebSocket Protocol [Base Framing Protocol](https://tools.ietf.org/html/rfc6455#section-5.2)
 - [Java Parser](http://git.eclipse.org/c/jetty/org.eclipse.jetty.project.git/plain/jetty-websocket/websocket-common/src/main/java/org/eclipse/jetty/websocket/common/Parser.java)
 - [Java Writer](http://git.eclipse.org/c/jetty/org.eclipse.jetty.project.git/plain/jetty-websocket/websocket-common/src/main/java/org/eclipse/jetty/websocket/common/Generator.java)
 - [The WebSocket Handshake](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers)
 - [WebSocket Echo Test](http://www.websocket.org/echo.html)
 ### Debug using vs2013
 ```
 Command: $(ProjectDir)..\..\..\ParaWorld\ParaEngineClient_d.exe
 Command Arguments: mc="true"  loadpackage="npl_packages/NplWebSocketSample/"  world="worlds/DesignHouse/test"
 Working Directory: ..\..\..\ParaWorld
 ```
 ### Transmission Protocol
 - Client Handshake Request
 ```js
 var ws = new WebSocket("ws://localhost:8099/ajax/nplsocketsample?action=handshake&user_id=" + user_id);
 ```
 - Server Handshake Response
 ```lua
 if(is_ajax()) then
	add_action('wp_ajax_handshake', function()

        local user_id = request:get("user_id");
        if(not user_id or user_id == "")then
            user_id = "leio";
        end
        local key = request:header("Sec-WebSocket-Key");

        if(key and key ~= "")then
            response.headers = {};
            key = key .. "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
            key = Encoding.sha1(key,"base64");
            response:add_header("status", "101 Switching Protocols");
            response:set_header("Connection", "Upgrade");
            response:set_header("Upgrade", "websocket");
            response:set_header("Sec-WebSocket-Accept", key);
            response:send_headers();

            local tid = request.nid;
            -- quick authentication, just accept any connection as simpleclient
            local nid = user_id;
            NPL.accept(tid, nid);

            -- for tesing broadcast
            NplWebSocketSample.AddUser(nid);
            return;
        end
    end)
    return
end
 ```
 - Send json message to server
 ```js
 var server_handle_id = 20; // a shord id which defined on npl server is used to receive message
 var msg = "Hello World!"; // custom message
var data = {
    s_id: server_handle_id,
    msg: [msg]
}
var s = JSON.stringify(data);
ws.send(s);
 ```
 - Send json message to Client
 ```lua
local json_from = commonlib.Json.Encode(msg);
NPL.activate(string.format("%s:websocket",nid),json_from);
 ```

 - Details sample is [here](https://github.com/tatfook/NplWebSocketSample)
 ![image](https://cloud.githubusercontent.com/assets/5885941/25740166/e926952e-31b7-11e7-9512-7b2eb22b8b27.png)
