#ifndef __JS_H__
#define __JS_H__
#include <iostream>
#include <string>
#include <functional>
#ifdef __EMSCRIPTEN__
#include "emscripten.h"


// 定义函数导出宏
// __EMSCRIPTEN__宏用于探测是否是Emscripten环境
// __cplusplus用于探测是否C++环境
// EMSCRIPTEN_KEEPALIVE是Emscripten特有的宏，用于告知编译器后续函数在优化时必须保留，并且该函数将被导出至JavaScript
#ifndef EM_PORT_API
#	if defined(__EMSCRIPTEN__)
#		include <emscripten.h>
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype EMSCRIPTEN_KEEPALIVE
#		else
#			define EM_PORT_API(rettype) rettype EMSCRIPTEN_KEEPALIVE
#		endif
#	else
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype
#		else
#			define EM_PORT_API(rettype) rettype
#		endif
#	endif
#endif

namespace JS
{

void StaticInit();
const std::string JsStringToString(const char* js_str);
const std::string GetQueryStringArg(const std::string key, bool decode = true);


const static int JS_LANGUAGE_EN = 0;   // 英文
const static int JS_LANGUAGE_ZH = 1;   // 中文
int GetBrowserLanguage();


// 通信
void SendMsgToJS(const std::string msg_data_json);
void SetRecvMsgFromJSCallback(std::function<void(const std::string, const std::string)>);
void RecvMsgFromJS(const std::string filename, const std::string msg_data_json);

// 剪切板
std::string GetClipboardText();
void SetClipboardText(std::string text);
// 文本输入
void SetTextInputCallback(std::function<void(const std::string)>);
};

#endif

#endif