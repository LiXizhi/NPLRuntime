#ifndef __EMSCRIPTEN_WEBSOCKET_H__
#define __EMSCRIPTEN_WEBSOCKET_H__

#include <emscripten/websocket.h>
#include <memory>
#include <string>
#include <deque>
#include <unordered_map>
#include <iostream>
#include <functional>

namespace NPL
{
    class CNPLConnection;
}

class EmscriptenWebSocket
{
public:
    EmscriptenWebSocket()
    {
        m_socket = 0;
        m_connecting = false;
        m_connection = nullptr;
    }

    bool Connect(const std::string &url)
    {
        if (IsConnecting() && m_url == url)
            return false;

        if (!emscripten_websocket_is_supported())
        {
            std::cout << "WebSockets are not supported, cannot continue!" << std::endl;
            return false;
        }

        EmscriptenWebSocketCreateAttributes attr;
        emscripten_websocket_init_create_attributes(&attr);

        // const char *url = "ws://localhost:9110/";
        attr.url = url.c_str();
        // attr.protocols = "binary,base64"; // We don't really use a special protocol on the server backend in this test, but check that it can be passed.

        EMSCRIPTEN_WEBSOCKET_T socket = emscripten_websocket_new(&attr);
        if (socket <= 0)
        {
            std::cout << "EmscriptenWebSocket creation failed, error code:" << (EMSCRIPTEN_RESULT)socket << std::endl;
            return false;
        }

        emscripten_websocket_set_onopen_callback(socket, (void *)this, WebSocketOpen);
        emscripten_websocket_set_onclose_callback(socket, (void *)this, WebSocketClose);
        emscripten_websocket_set_onerror_callback(socket, (void *)this, WebSocketError);
        emscripten_websocket_set_onmessage_callback(socket, (void *)this, WebSocketMessage);

        m_url = url;
        SetConnecting(true);

        return true;
    }

    bool Send(const std::string &data)
    {
        if (m_socket == 0)
            return false;
        emscripten_websocket_send_binary(m_socket, (void *)(data.data()), data.size());
        return true;
    }

    bool SendText(const std::string &data)
    {
        if (m_socket == 0)
            return false;
        return EMSCRIPTEN_RESULT_SUCCESS == emscripten_websocket_send_utf8_text(m_socket, data.c_str());
    }

    void Close()
    {
        if (m_socket == 0)
            return;
        OnClose();
        emscripten_websocket_close(m_socket, 0, 0);
        emscripten_websocket_delete(m_socket);
        m_socket = 0;
    }

    virtual void OnConnect()
    {
        SetConnecting(false);
        SetConnected(true);
        if (m_on_connect != nullptr)
            m_on_connect();
    }

    virtual void OnReceive(const std::string &data)
    {
        // std::cout << "WebSocketMessage:" << (m_on_receive == nullptr) << std::endl;
        if (m_on_receive != nullptr)
            m_on_receive(data);
    }

    virtual void OnClose()
    {
        SetConnecting(false);
        SetConnected(false);
        if (m_on_close != nullptr)
            m_on_close();
    }

    inline void SetSocket(EMSCRIPTEN_WEBSOCKET_T socket) { m_socket = socket; }
    inline void SetConnecting(bool connecting) { m_connecting = connecting; }
    inline bool IsConnecting() { return m_connecting; }
    inline void SetConnected(bool connected) { m_connected = connected; }
    inline bool IsConnected() { return m_connected; }
    inline void SetOnConnect(std::function<void()> on_connect) { m_on_connect = on_connect; }
    inline void SetOnClose(std::function<void()> on_close) { m_on_close = on_close; }
    inline void SetOnReceive(std::function<void(const std::string &)> on_receive) { m_on_receive = on_receive; }
    inline void SetConnection(std::shared_ptr<NPL::CNPLConnection> connection) { m_connection = connection; }
    inline NPL::CNPLConnection* GetConnection() { return m_connection.get(); }

private:
    EMSCRIPTEN_WEBSOCKET_T m_socket;
    std::function<void()> m_on_connect;
    std::function<void(const std::string &)> m_on_receive;
    std::function<void()> m_on_close;
    std::string m_url;
    bool m_connecting;
    bool m_connected;
    std::shared_ptr<NPL::CNPLConnection> m_connection;

public:
    static EM_BOOL WebSocketOpen(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData)
    {
        std::cout << "WebSocketOpen Success" << std::endl;
        EmscriptenWebSocket *socket = (EmscriptenWebSocket *)userData;
        socket->SetSocket(e->socket);
        socket->OnConnect();
        return 0;
    }

    static EM_BOOL WebSocketClose(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData)
    {
        std::cout << "WebSocketClose Success" << std::endl;
        EmscriptenWebSocket *socket = (EmscriptenWebSocket *)userData;
        socket->OnClose();
        return 0;
    }

    static EM_BOOL WebSocketError(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData)
    {
        std::cout << "WebSocketError => Close EmscriptenWebSocket" << std::endl;
        EmscriptenWebSocket *socket = (EmscriptenWebSocket *)userData;
        socket->Close();
        return 0;
    }

    static EM_BOOL WebSocketMessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData)
    {
        EmscriptenWebSocket *socket = (EmscriptenWebSocket *)userData;
        std::string data;
        if (e->isText)
            data = (char *)(e->data);
        else
        {
            data.resize(e->numBytes);
            memcpy(data.data(), e->data, e->numBytes);
        }
        // std::cout << "WebSocketMessage:" << data << std::endl;
        socket->OnReceive(data);
        return 0;
    }

    static std::shared_ptr<EmscriptenWebSocket> CreateGetWebSocket(const std::string &key)
    {
        static std::unordered_map<std::string, std::shared_ptr<EmscriptenWebSocket>> s_web_socket_map;
        auto it = s_web_socket_map.find(key);
        if (it != s_web_socket_map.end())
            return it->second;
        auto websocket = std::make_shared<EmscriptenWebSocket>();
        s_web_socket_map[key] = websocket;
        return websocket;
    }
};

#endif