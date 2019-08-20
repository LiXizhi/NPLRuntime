#pragma once

#include "ParaEngine.h"
#include <jni.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <android/log.h>

#define  LOG_TAG    "ParaEngine"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

namespace ParaEngine {

	typedef struct JniMethodInfo_
	{
		JNIEnv *    env;
		jclass      classID;
		jmethodID   methodID;
	} JniMethodInfo;

	class JniHelper
	{
	public:
		static void setJavaVM(JavaVM *javaVM);
		static JavaVM* getJavaVM();
		static JNIEnv* getEnv();
		static jobject getActivity();

		static jclass getClassID(const char *className);

		static std::string getStringUTFCharsJNI(JNIEnv* env, jstring srcjStr, bool* ret = nullptr);
		static std::u16string getStringUTF16CharsJNI(JNIEnv* env, jstring srcjStr, bool* ret = nullptr);
		static jstring newStringUTFJNI(JNIEnv* env, const std::string& utf8Str, bool* ret = nullptr);
		static jstring newStringUTF16JNI(JNIEnv* env, const std::u16string& utf16Str);

		static bool setClassLoaderFrom(jobject activityInstance);
		static bool getStaticMethodInfo(JniMethodInfo &methodinfo,
			const char *className,
			const char *methodName,
			const char *paramCode);
		static bool getMethodInfo(JniMethodInfo &methodinfo,
			const char *className,
			const char *methodName,
			const char *paramCode);

		static bool getMethodInfo(JniMethodInfo &methodinfo,
			jobject object,
			const char *methodName,
			const char *paramCode);

		static std::string jstring2string(jstring str);

		static jmethodID loadclassMethod_methodID;
		static jobject classloader;
		static std::function<void()> classloaderCallback;

		template <typename... Ts>
		static void callStaticVoidMethod(const std::string& className,
			const std::string& methodName,
			Ts... xs) {
			JniMethodInfo t;
			std::string signature = "(" + std::string(getJNISignature(xs...)) + ")V";
			if (JniHelper::getStaticMethodInfo(t, className.c_str(), methodName.c_str(), signature.c_str())) {
				t.env->CallStaticVoidMethod(t.classID, t.methodID, convert(t, xs)...);
				t.env->DeleteLocalRef(t.classID);
				deleteLocalRefs(t.env);
			}
			else {
				reportError(className, methodName, signature);
			}
		}

		template <typename... Ts>
		static bool callStaticBooleanMethod(const std::string& className,
			const std::string& methodName,
			Ts... xs) {
			jboolean jret = JNI_FALSE;
			JniMethodInfo t;
			std::string signature = "(" + std::string(getJNISignature(xs...)) + ")Z";
			if (JniHelper::getStaticMethodInfo(t, className.c_str(), methodName.c_str(), signature.c_str())) {
				jret = t.env->CallStaticBooleanMethod(t.classID, t.methodID, convert(t, xs)...);
				t.env->DeleteLocalRef(t.classID);
				deleteLocalRefs(t.env);
			}
			else {
				reportError(className, methodName, signature);
			}
			return (jret == JNI_TRUE);
		}

		template <typename... Ts>
		static int callStaticIntMethod(const std::string& className,
			const std::string& methodName,
			Ts... xs) {
			jint ret = 0;
			JniMethodInfo t;
			std::string signature = "(" + std::string(getJNISignature(xs...)) + ")I";
			if (JniHelper::getStaticMethodInfo(t, className.c_str(), methodName.c_str(), signature.c_str())) {
				ret = t.env->CallStaticIntMethod(t.classID, t.methodID, convert(t, xs)...);
				t.env->DeleteLocalRef(t.classID);
				deleteLocalRefs(t.env);
			}
			else {
				reportError(className, methodName, signature);
			}
			return ret;
		}

		template <typename... Ts>
		static float callStaticFloatMethod(const std::string& className,
			const std::string& methodName,
			Ts... xs) {
			jfloat ret = 0.0;
			JniMethodInfo t;
			std::string signature = "(" + std::string(getJNISignature(xs...)) + ")F";
			if (JniHelper::getStaticMethodInfo(t, className.c_str(), methodName.c_str(), signature.c_str())) {
				ret = t.env->CallStaticFloatMethod(t.classID, t.methodID, convert(t, xs)...);
				t.env->DeleteLocalRef(t.classID);
				deleteLocalRefs(t.env);
			}
			else {
				reportError(className, methodName, signature);
			}
			return ret;
		}

		template <typename... Ts>
		static float* callStaticFloatArrayMethod(const std::string& className,
			const std::string& methodName,
			Ts... xs) {
			static float ret[32];
			JniMethodInfo t;
			std::string signature = "(" + std::string(getJNISignature(xs...)) + ")[F";
			if (JniHelper::getStaticMethodInfo(t, className.c_str(), methodName.c_str(), signature.c_str())) {
				jfloatArray array = (jfloatArray)t.env->CallStaticObjectMethod(t.classID, t.methodID, convert(t, xs)...);
				jsize len = t.env->GetArrayLength(array);
				if (len <= 32) {
					jfloat* elems = t.env->GetFloatArrayElements(array, 0);
					if (elems) {
						memcpy(ret, elems, sizeof(float) * len);
						t.env->ReleaseFloatArrayElements(array, elems, 0);
					};
				}
				t.env->DeleteLocalRef(t.classID);
				deleteLocalRefs(t.env);
				return &ret[0];
			}
			else {
				reportError(className, methodName, signature);
			}
			return nullptr;
		}

		/*
		template <typename... Ts>
		static Vec3 callStaticVec3Method(const std::string& className,
			const std::string& methodName,
			Ts... xs) {
			Vec3 ret;
			cocos2d::JniMethodInfo t;
			std::string signature = "(" + std::string(getJNISignature(xs...)) + ")[F";
			if (cocos2d::JniHelper::getStaticMethodInfo(t, className.c_str(), methodName.c_str(), signature.c_str())) {
				jfloatArray array = (jfloatArray)t.env->CallStaticObjectMethod(t.classID, t.methodID, convert(t, xs)...);
				jsize len = t.env->GetArrayLength(array);
				if (len == 3) {
					jfloat* elems = t.env->GetFloatArrayElements(array, 0);
					ret.x = elems[0];
					ret.y = elems[1];
					ret.z = elems[2];
					t.env->ReleaseFloatArrayElements(array, elems, 0);
				}
				t.env->DeleteLocalRef(t.classID);
				deleteLocalRefs(t.env);
			}
			else {
				reportError(className, methodName, signature);
			}
			return ret;
		}
		*/

		template <typename... Ts>
		static double callStaticDoubleMethod(const std::string& className,
			const std::string& methodName,
			Ts... xs) {
			jdouble ret = 0.0;
			JniMethodInfo t;
			std::string signature = "(" + std::string(getJNISignature(xs...)) + ")D";
			if (JniHelper::getStaticMethodInfo(t, className.c_str(), methodName.c_str(), signature.c_str())) {
				ret = t.env->CallStaticDoubleMethod(t.classID, t.methodID, convert(t, xs)...);
				t.env->DeleteLocalRef(t.classID);
				deleteLocalRefs(t.env);
			}
			else {
				reportError(className, methodName, signature);
			}
			return ret;
		}

		template <typename... Ts>
		static std::string callStaticStringMethod(const std::string& className,
			const std::string& methodName,
			Ts... xs) {
			std::string ret;

			JniMethodInfo t;
			std::string signature = "(" + std::string(getJNISignature(xs...)) + ")Ljava/lang/String;";
			if (JniHelper::getStaticMethodInfo(t, className.c_str(), methodName.c_str(), signature.c_str())) {
				jstring jret = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID, convert(t, xs)...);
				ret = JniHelper::jstring2string(jret);
				t.env->DeleteLocalRef(t.classID);
				t.env->DeleteLocalRef(jret);
				deleteLocalRefs(t.env);
			}
			else {
				reportError(className, methodName, signature);
			}
			return ret;
		}

	private:
		static JNIEnv* cacheEnv(JavaVM* jvm);
		static bool getMethodInfo_DefaultClassLoader(JniMethodInfo &methodinfo,
			const char *className,
			const char *methodName,
			const char *paramCode);

		static JavaVM* _psJavaVM;

		static jobject _activity;

		static jstring convert(JniMethodInfo& t, const char* x);

		static jstring convert(JniMethodInfo& t, const std::string& x);

		template <typename T>
		static T convert(JniMethodInfo&, T x) {
			return x;
		}

		static std::unordered_map<JNIEnv*, std::vector<jobject>> localRefs;

		static void deleteLocalRefs(JNIEnv* env);

		static std::string getJNISignature() {
			return "";
		}

		static std::string getJNISignature(bool) {
			return "Z";
		}

		static std::string getJNISignature(char) {
			return "C";
		}

		static std::string getJNISignature(short) {
			return "S";
		}

		static std::string getJNISignature(int) {
			return "I";
		}

		static std::string getJNISignature(long) {
			return "J";
		}

		static std::string getJNISignature(float) {
			return "F";
		}

		static  std::string getJNISignature(double) {
			return "D";
		}

		static std::string getJNISignature(const char*) {
			return "Ljava/lang/String;";
		}

		static std::string getJNISignature(const std::string&) {
			return "Ljava/lang/String;";
		}

		template <typename T>
		static std::string getJNISignature(T x) {
			// This template should never be instantiated
			static_assert(sizeof(x) == 0, "Unsupported argument type");
			return "";
		}

		template <typename T, typename... Ts>
		static std::string getJNISignature(T x, Ts... xs) {
			return getJNISignature(x) + getJNISignature(xs...);
		}

		static void reportError(const std::string& className, const std::string& methodName, const std::string& signature);
	};

} // end namespace
