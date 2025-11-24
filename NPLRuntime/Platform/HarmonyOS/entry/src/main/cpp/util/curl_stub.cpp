//-----------------------------------------------------------------------------
// curl_stub.cpp - Stub implementations for curl functions
// This is a temporary solution until curl is properly built for HarmonyOS
//-----------------------------------------------------------------------------

#include <curl/curl.h>
#include <hilog/log.h>

#define LOG_TAG "curl_stub"

// Stub implementations - these will return errors/defaults
extern "C" {

CURLcode curl_global_init(long flags) {
    OH_LOG_WARN(LOG_APP, "curl_global_init called - stub implementation");
    return CURLE_OK;
}

void curl_global_cleanup(void) {
    OH_LOG_WARN(LOG_APP, "curl_global_cleanup called - stub implementation");
}

CURL *curl_easy_init(void) {
    OH_LOG_WARN(LOG_APP, "curl_easy_init called - stub implementation");
    return nullptr;
}

void curl_easy_cleanup(CURL *curl) {
    OH_LOG_WARN(LOG_APP, "curl_easy_cleanup called - stub implementation");
}

CURLcode curl_easy_perform(CURL *curl) {
    OH_LOG_WARN(LOG_APP, "curl_easy_perform called - stub implementation");
    return CURLE_FAILED_INIT;
}

CURLcode curl_easy_setopt(CURL *curl, CURLoption option, ...) {
    OH_LOG_WARN(LOG_APP, "curl_easy_setopt called - stub implementation");
    return CURLE_OK;
}

CURLcode curl_easy_getinfo(CURL *curl, CURLINFO info, ...) {
    OH_LOG_WARN(LOG_APP, "curl_easy_getinfo called - stub implementation");
    return CURLE_OK;
}

void curl_easy_reset(CURL *curl) {
    OH_LOG_WARN(LOG_APP, "curl_easy_reset called - stub implementation");
}

struct curl_slist *curl_slist_append(struct curl_slist *list, const char *string) {
    OH_LOG_WARN(LOG_APP, "curl_slist_append called - stub implementation");
    return nullptr;
}

void curl_slist_free_all(struct curl_slist *list) {
    OH_LOG_WARN(LOG_APP, "curl_slist_free_all called - stub implementation");
}

CURLFORMcode curl_formadd(struct curl_httppost **httppost, struct curl_httppost **last_post, ...) {
    OH_LOG_WARN(LOG_APP, "curl_formadd called - stub implementation");
    return CURL_FORMADD_OK;
}

void curl_formfree(struct curl_httppost *form) {
    OH_LOG_WARN(LOG_APP, "curl_formfree called - stub implementation");
}

CURLM *curl_multi_init(void) {
    OH_LOG_WARN(LOG_APP, "curl_multi_init called - stub implementation");
    return nullptr;
}

CURLMcode curl_multi_cleanup(CURLM *multi_handle) {
    OH_LOG_WARN(LOG_APP, "curl_multi_cleanup called - stub implementation");
    return CURLM_OK;
}

CURLMcode curl_multi_add_handle(CURLM *multi_handle, CURL *curl_handle) {
    OH_LOG_WARN(LOG_APP, "curl_multi_add_handle called - stub implementation");
    return CURLM_OK;
}

CURLMcode curl_multi_remove_handle(CURLM *multi_handle, CURL *curl_handle) {
    OH_LOG_WARN(LOG_APP, "curl_multi_remove_handle called - stub implementation");
    return CURLM_OK;
}

CURLMcode curl_multi_perform(CURLM *multi_handle, int *running_handles) {
    OH_LOG_WARN(LOG_APP, "curl_multi_perform called - stub implementation");
    if (running_handles) *running_handles = 0;
    return CURLM_OK;
}

CURLMsg *curl_multi_info_read(CURLM *multi_handle, int *msgs_in_queue) {
    OH_LOG_WARN(LOG_APP, "curl_multi_info_read called - stub implementation");
    if (msgs_in_queue) *msgs_in_queue = 0;
    return nullptr;
}

} // extern "C"
