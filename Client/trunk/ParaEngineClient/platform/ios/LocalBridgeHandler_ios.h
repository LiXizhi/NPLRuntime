#ifndef LocalBridgeHandler_H
#define LocalBridgeHandler_H
class LocalBridgeHandler
{
public:
    static void makeResponse(const char* key, const char* data, const char* tag);
    static void ios_localCall(const char* api_key, const char* data);
    static void vibrate_Request(const char* api_key, const char* data);
    static void getMemoryInfo_Request(const char* api_key, const char* data);
    static void openURL_Request(const char* api_key, const char* data);
};
#endif