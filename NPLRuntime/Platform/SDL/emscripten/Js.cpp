#include "Js.h"

#ifdef __EMSCRIPTEN__

namespace JS
{

    EM_JS(void, StaticInit_JS, (), {
        window.JsStringToCString = function(jsString)
        {
            var lengthBytes = lengthBytesUTF8(jsString) + 1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
            return stringOnWasmHeap;
        };

        // window.RecvMsgFromEmscripten = function(msg_name, msg_data_json)
        // {
        //     console.log("RecvMsgFromEmscripten:" + json_msg);
        // };
        // window.SendMsgToEmscripten = function(msg_name, msg_data_json)
        // {
        //     Module._SendMsgToEmscripten(window.JsStringToCString(msg_name), window.JsStringToCString(msg_data_json));
        // };
    })

    void StaticInit()
    {
        static bool s_inited = false;
        if (s_inited)
            return;
        s_inited = true;
        StaticInit_JS();
    }

    const std::string JsStringToString(const char *js_str)
    {
        const std::string c_str = js_str;
        free((void *)js_str);
        return c_str;
    }

    EM_JS(const char *, GetQueryStringArg_JS, (const char *c_key, bool decode), {
        var key = UTF8ToString(c_key); // 查询键
        var value = "";                // 查询值
        // 取得查询字符串并去掉开头问号
        var qs = location.search.length > 0 ? location.search.substring(1) : "";
        // 取得每一项
        var items = qs.length ? qs.split('&') : [];
        // 逐个将每一项添加到args对象中
        for (var i = 0; i < items.length; i++)
        {
            var item = items[i].split('=');
            var name = decodeURIComponent(item[0]);
            if (name == key)
            {
                value = decode ? decodeURIComponent(item[1]) : item[1];
                break;
            }
        }

        return JsStringToCString(value);
    })

    const std::string GetQueryStringArg(const std::string key, bool decode)
    {
        return JsStringToString(GetQueryStringArg_JS(key.c_str(), decode));
    }

    int GetBrowserLanguage()
    {
        int js_language = EM_ASM_INT({
            var browser_language = (navigator.language || navigator.browserLanguage).toLowerCase();
            if (browser_language.indexOf('zh') >= 0)
            {
                console.log("浏览器语言是---中文");
                return 1;
            }
            else if (browser_language.indexOf('en') >= 0)
            {
                console.log("浏览器语言是---英文");
                return 0;
            }
            else
            {
                console.log("浏览器语言是---其他语言");
                return -1;
            }
        });
        return js_language;
    }

    int IsTouchDevice()
    {
        // clang-format off
        int is_touch_device = EM_ASM_INT({
            if (/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent))
            {
                return 1;
            }
            else
            {
                return 0;
            }
            // if ('ontouchstart' in window || navigator.maxTouchPoints > 0 || navigator.msMaxTouchPoints > 0)
            // {
            //     return 1;
            // }
            // else
            // {
            //     return 0;
            // }
        });
        // clang-format oon
        return is_touch_device;
    }

    // 定义C++方法执行JS代码
    EM_JS(void, SendMsgToJS_JS, (const char *c_msg_data_json), {
        var msg_data_json = UTF8ToString(c_msg_data_json);
        if (typeof RecvMsgFromEmscripten == "function")
        {
            RecvMsgFromEmscripten(msg_data_json);
        }
    })

    // 定义JS方法执行C++代码
    EM_PORT_API(void)
    SendMsgToEmscripten(const char *js_filename, const char *js_msg_data_json)
    {
        // std::cout << "RecvMsgFromJS:" << js_filename << " msg data:"<<  js_msg_data_json << std::endl;
        RecvMsgFromJS(JsStringToString(js_filename), JsStringToString(js_msg_data_json));
    }

    void SendMsgToJS(const std::string msg_data_json)
    {
        SendMsgToJS_JS(msg_data_json.c_str());
    }

    static std::function<void(const std::string, const std::string)> s_recv_msg_from_js_callback = nullptr;
    void SetRecvMsgFromJSCallback(std::function<void(const std::string, const std::string)> callback)
    {
        s_recv_msg_from_js_callback = callback;
    }

    void RecvMsgFromJS(const std::string filename, const std::string msg_data_json)
    {
        if (s_recv_msg_from_js_callback != nullptr)
        {
            s_recv_msg_from_js_callback(filename, msg_data_json);
        }
    }

    EM_ASYNC_JS(const char *, GetClipboardText_JS, (), {
        try
        {
            var text = await navigator.clipboard.readText();
            return stringToNewUTF8(text);
        }
        catch (e)
        {
            console.log(e);
            return stringToNewUTF8("");
        }
    })

    std::string GetClipboardText()
    {
        return JsStringToString(GetClipboardText_JS());
    }

    EM_ASYNC_JS(void, SetClipboardText_JS, (const char *text), {
        try
        {
            await navigator.clipboard.writeText(UTF8ToString(text));
        }
        catch (e)
        {
            console.log(e);
            return stringToNewUTF8("");
        }
    })

    void SetClipboardText(std::string text)
    {
        SetClipboardText_JS(text.c_str());
    }

    static std::function<void(const std::string)> s_on_text_input_callback = nullptr;
    void SetTextInputCallback(std::function<void(const std::string)> on_text_input_callback)
    {
        s_on_text_input_callback = on_text_input_callback;
    }
    EM_PORT_API(void)
    OnTextInput(const char *js_text)
    {
        if (s_on_text_input_callback == nullptr)
            return;
        s_on_text_input_callback(JsStringToString(js_text));
    }

    static std::function<void(int keycode)> s_on_key_down_callback = nullptr;
    void SetKeyDownCallback(std::function<void(int keycode)> on_key_down_callback)
    {
        s_on_key_down_callback = on_key_down_callback;
    }

    EM_PORT_API(void)
    OnKeyDown(int keycode)
    {
        if (s_on_key_down_callback == nullptr)
            return;
        s_on_key_down_callback(keycode);
    }

    static std::function<void(int keycode)> s_on_key_up_callback = nullptr;
    void SetKeyUpCallback(std::function<void(int keycode)> on_key_up_callback)
    {
        s_on_key_up_callback = on_key_up_callback;
    }

    EM_PORT_API(void)
    OnKeyUp(int keycode)
    {
        if (s_on_key_up_callback == nullptr)
            return;
        s_on_key_up_callback(keycode);
    }

    static std::function<void(const std::string)> s_on_clear_text_input_callback = nullptr;
    void SetClearTextInputCallback(std::function<void(const std::string)> on_clear_text_input_callback)
    {
        s_on_clear_text_input_callback = on_clear_text_input_callback;
    }
    EM_PORT_API(void)
    OnClearTextInput(const char *js_text)
    {
        if (s_on_clear_text_input_callback == nullptr) return;
        s_on_clear_text_input_callback(JsStringToString(js_text));
    }
};

#endif
