
#include "JniHelper.h"
#include "StringHelper.h"

#include <string.h>
#include <pthread.h>


static pthread_key_t g_key;



void _detachCurrentThread(void* a) {
	ParaEngine::JniHelper::getJavaVM()->DetachCurrentThread();
}

namespace ParaEngine {

    JavaVM* JniHelper::_psJavaVM = nullptr;
    jmethodID JniHelper::loadclassMethod_methodID = nullptr;
    jobject JniHelper::classloader = nullptr;
    std::function<void()> JniHelper::classloaderCallback = nullptr;
    
    jobject JniHelper::_activity = nullptr;
    std::unordered_map<JNIEnv*, std::vector<jobject>> JniHelper::localRefs;

    JavaVM* JniHelper::getJavaVM() {
        pthread_t thisthread = pthread_self();
        LOGD("JniHelper::getJavaVM(), pthread_self() = %ld", thisthread);
        return _psJavaVM;
    }

    void JniHelper::setJavaVM(JavaVM *javaVM) {
        pthread_t thisthread = pthread_self();
        LOGD("JniHelper::setJavaVM(%p), pthread_self() = %ld", javaVM, thisthread);
        _psJavaVM = javaVM;

        pthread_key_create(&g_key, _detachCurrentThread);
    }

	jclass JniHelper::getClassID(const char *className) {
		if (nullptr == className) {
			return nullptr;
		}
		JNIEnv* env = getEnv();

		jstring _jstrClassName = env->NewStringUTF(className);

		jclass _clazz = (jclass)env->CallObjectMethod(classloader,
			loadclassMethod_methodID,
			_jstrClassName);

		if (nullptr == _clazz) {
			LOGE("Classloader failed to find class of %s", className);
			env->ExceptionClear();
		}

		env->DeleteLocalRef(_jstrClassName);

		return _clazz;
	}


	JNIEnv* JniHelper::cacheEnv(JavaVM* jvm) {
		JNIEnv* _env = nullptr;
		// get jni environment
		jint ret = jvm->GetEnv((void**)&_env, JNI_VERSION_1_4);

		switch (ret) {
		case JNI_OK:
			// Success!
			pthread_setspecific(g_key, _env);
			return _env;

		case JNI_EDETACHED:
			// Thread not attached
			if (jvm->AttachCurrentThread(&_env, nullptr) < 0)
			{
				LOGE("Failed to get the environment using AttachCurrentThread()");

				return nullptr;
			}
			else {
				// Success : Attached and obtained JNIEnv!
				pthread_setspecific(g_key, _env);
				return _env;
			}

		case JNI_EVERSION:
			// Cannot recover from this error
			LOGE("JNI interface version 1.4 not supported");
		default:
			LOGE("Failed to get the environment using GetEnv()");
			return nullptr;
		}
	}

	JNIEnv* JniHelper::getEnv() {
		JNIEnv *_env = (JNIEnv *)pthread_getspecific(g_key);
		if (_env == nullptr)
			_env = JniHelper::cacheEnv(_psJavaVM);
		return _env;
	}
    
    jobject JniHelper::getActivity() {
        return _activity;
    }

    bool JniHelper::setClassLoaderFrom(jobject activityinstance) {
        JniMethodInfo _getclassloaderMethod;
        if (!JniHelper::getMethodInfo_DefaultClassLoader(_getclassloaderMethod,
                                                         "android/content/Context",
                                                         "getClassLoader",
                                                         "()Ljava/lang/ClassLoader;")) {
            return false;
        }

        jobject _c = JniHelper::getEnv()->CallObjectMethod(activityinstance,
                                                                    _getclassloaderMethod.methodID);

        if (nullptr == _c) {
            return false;
        }

        JniMethodInfo _m;
        if (!JniHelper::getMethodInfo_DefaultClassLoader(_m,
                                                         "java/lang/ClassLoader",
                                                         "loadClass",
                                                         "(Ljava/lang/String;)Ljava/lang/Class;")) {
            return false;
        }

        JniHelper::classloader = JniHelper::getEnv()->NewGlobalRef(_c);
        JniHelper::loadclassMethod_methodID = _m.methodID;
		JniHelper::_activity = activityinstance;
        if (JniHelper::classloaderCallback != nullptr){
            JniHelper::classloaderCallback();
        }

        return true;
    }

    bool JniHelper::getStaticMethodInfo(JniMethodInfo &methodinfo,
                                        const char *className, 
                                        const char *methodName,
                                        const char *paramCode) {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            LOGE("Failed to get JNIEnv");
            return false;
        }
            
        jclass classID = getClassID(className);
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetStaticMethodID(classID, methodName, paramCode);
        if (! methodID) {
            LOGE("Failed to find static method id of %s", methodName);
            env->ExceptionClear();
            return false;
        }
            
        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;
        return true;
    }

    bool JniHelper::getMethodInfo_DefaultClassLoader(JniMethodInfo &methodinfo,
                                                     const char *className,
                                                     const char *methodName,
                                                     const char *paramCode) {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            return false;
        }

        jclass classID = env->FindClass(className);
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
        if (! methodID) {
            LOGE("Failed to find method id of %s", methodName);
            env->ExceptionClear();
            return false;
        }

        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;

        return true;
    }

	bool JniHelper::getMethodInfo(JniMethodInfo &methodinfo,
		jobject object,
		const char *methodName,
		const char *paramCode)
	{
		if ((nullptr == object) ||
			(nullptr == methodName) ||
			(nullptr == paramCode)) {
			return false;
		}

		JNIEnv *env = JniHelper::getEnv();
		if (!env) {
			return false;
		}

		jclass classID = env->GetObjectClass(object);
		if (!classID) {
			LOGE("Failed to find class");
			env->ExceptionClear();
			return false;
		}

		jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
		if (!methodID) {
			LOGE("Failed to find method id of %s", methodName);
			env->ExceptionClear();
			return false;
		}

		methodinfo.classID = classID;
		methodinfo.env = env;
		methodinfo.methodID = methodID;

		return true;
	}

    bool JniHelper::getMethodInfo(JniMethodInfo &methodinfo,
                                  const char *className,
                                  const char *methodName,
                                  const char *paramCode) {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            return false;
        }

        jclass classID = getClassID(className);
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
        if (! methodID) {
            LOGE("Failed to find method id of %s", methodName);
            env->ExceptionClear();
            return false;
        }

        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;

        return true;
    }

    std::u16string JniHelper::getStringUTF16CharsJNI(JNIEnv* env, jstring srcjStr, bool* ret)
    {
        if (srcjStr != nullptr)
        {
            const unsigned short * unicodeChar = (const unsigned short *)env->GetStringChars(srcjStr, nullptr);
            size_t unicodeCharLength = env->GetStringLength(srcjStr);
            std::u16string unicodeStr((const char16_t *)unicodeChar, unicodeCharLength);

            if (ret)
            {
                *ret = true;
            }

            env->ReleaseStringChars(srcjStr, unicodeChar);

            return unicodeStr;
        }
        else
        {
            if (ret)
            {
                *ret = false;
            }

            return std::u16string();
        }
    }

	std::string JniHelper::getStringUTFCharsJNI(JNIEnv* env, jstring srcjStr, bool* ret)
	{
		std::string utf8Str;
		if (srcjStr != nullptr)
		{
			const unsigned short * unicodeChar = (const unsigned short *)env->GetStringChars(srcjStr, nullptr);
			size_t unicodeCharLength = env->GetStringLength(srcjStr);
			const std::u16string unicodeStr((const char16_t *)unicodeChar, unicodeCharLength);
			bool flag = StringHelper::UTF16ToUTF8(unicodeStr, utf8Str);
			if (ret)
			{
				*ret = flag;
			}
			if (!flag)
			{
				utf8Str = "";
			}
			env->ReleaseStringChars(srcjStr, unicodeChar);
		}
		else
		{
			if (ret)
			{
				*ret = false;
			}
			utf8Str = "";
		}
		return utf8Str;
	}

	jstring JniHelper::newStringUTFJNI(JNIEnv* env, const std::string& utf8Str, bool* ret)
	{
		std::u16string utf16Str;
		bool flag = StringHelper::UTF8ToUTF16(utf8Str, utf16Str);

		if (ret)
		{
			*ret = flag;
		}

		if (!flag)
		{
			utf16Str.clear();
		}
		jstring stringText = env->NewString((const jchar*)utf16Str.data(), utf16Str.length());
		return stringText;
	}

    jstring JniHelper::newStringUTF16JNI(JNIEnv* env, const std::u16string& utf16Str)
    {
        jstring stringText = env->NewString((const jchar*)utf16Str.data(), utf16Str.length());
        return stringText;
    }

    std::string JniHelper::jstring2string(jstring jstr) {
        if (jstr == nullptr) {
            return "";
        }
        
        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            return "";
        }

        std::string strValue = getStringUTFCharsJNI(env, jstr);

        return strValue;
    }

    jstring JniHelper::convert(JniMethodInfo& t, const char* x) {
        jstring ret = newStringUTFJNI(t.env, x ? x : "");
        localRefs[t.env].push_back(ret);
        return ret;
    }

    jstring JniHelper::convert(JniMethodInfo& t, const std::string& x) {
        return convert(t, x.c_str());
    }

    void JniHelper::deleteLocalRefs(JNIEnv* env) {
        if (!env) {
            return;
        }

        for (const auto& ref : localRefs[env]) {
            env->DeleteLocalRef(ref);
        }
        localRefs[env].clear();
    }

    void JniHelper::reportError(const std::string& className, const std::string& methodName, const std::string& signature) {
        LOGE("Failed to find static java method. Class name: %s, method name: %s, signature: %s ",  className.c_str(), methodName.c_str(), signature.c_str());
    }

} //end namespace
