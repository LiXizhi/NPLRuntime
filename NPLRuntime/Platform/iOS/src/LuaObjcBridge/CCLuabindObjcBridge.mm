//-----------------------------------------------------------------------------
// Class: CCLuabindObjcBridge.mm
// Authors: kkvskkkk, big
// Emails: onedou@126.com
// CreateDate: 2018.9.21
// ModifyDate: 2023.3.23
//-----------------------------------------------------------------------------

#include "CCLuaObjcBridge.h"

#include <map>

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

//@interface OcObject : NSObject
//{}
//@end

namespace ParaEngine {
    LuaObjcBridge::ReturnObject _getReturnObjectForOcType(id oval)
    {
        if ([oval isKindOfClass:[NSNumber class]])
        {
            NSNumber *number = (NSNumber *)oval;
            const char *numberType = [number objCType];
            if (strcmp(numberType, @encode(BOOL)) == 0)
            {
                LuaObjcBridge::ReturnValue rv;
                rv.boolValue = [number boolValue];
                return LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeBoolean, rv);
            }
            else if (strcmp(numberType, @encode(int)) == 0)
            {
                LuaObjcBridge::ReturnValue rv;
                rv.intValue = [number intValue];
                return LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeInteger, rv);
            }
            else
            {
                LuaObjcBridge::ReturnValue rv;
                rv.floatValue = [number floatValue];
                return LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeFloat, rv);
            }
        }
        else if ([oval isKindOfClass:[NSString class]])
        {
            CFStringRef str_ref = (__bridge_retained CFStringRef)oval;
            LuaObjcBridge::ReturnObject ret = LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeString, (void *)str_ref);
            return ret;
        }
        else if ([oval isKindOfClass:[NSDictionary class]])
        {
            NSDictionary* dict = oval;
            
            if(dict.count > 0)
            {
                CFDictionaryRef dict_ref = (__bridge_retained CFDictionaryRef)oval;
                LuaObjcBridge::ReturnObject ret = LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeNSDictionary, (void*)dict_ref);
                return ret;
            }
        }
        //else if ([oval isKindOfClass:[OcObject class]])
        {
        //    return LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeObject, (__bridge_retained void *)oval);
        }
    }

    LuaObjcBridge::ReturnObject _getReturnObject(const char *returnType, id oval)
    {
        if (strcmp(returnType, "@") == 0)
        {
            return _getReturnObjectForOcType(oval);
        }
        else if (strcmp(returnType, "c") == 0) // BOOL
        {
            char ret_oc = [oval charValue];
            LuaObjcBridge::ReturnValue rv;
            rv.boolValue = bool(ret_oc);
            return LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeBoolean, rv);
        }
        else if (strcmp(returnType, "i") == 0) // int
        {
            int ret_oc = [oval intValue];
            LuaObjcBridge::ReturnValue rv;
            rv.intValue = ret_oc;
            return LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeInteger, rv);
        }
        else if (strcmp(returnType, "f") == 0) // float
        {
            float ret_oc = [oval floatValue];
            LuaObjcBridge::ReturnValue rv;
            rv.floatValue = ret_oc;
            return LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeFloat, rv);
        }
        else
        {
            NSLog(@"not support return type = %s", returnType);
            LuaObjcBridge::ReturnValue rv;
            return LuaObjcBridge::ReturnObject(LuaObjcBridge::TypeInvalid, rv);
        }
    }

    LuaObjcBridge::ReturnObject::Handle::Handle():m_obj(nullptr)
    {
    }

    LuaObjcBridge::ReturnObject::Handle::~Handle()
    {
        if(m_obj != NULL)
        {
            //int ptraddr = *(int *)m_obj;

            CFRelease(m_obj);
            m_obj = NULL;
        }
    }

    void LuaObjcBridge::ReturnObject::Handle::setData(const ValueType& valueType, const ReturnValue& returnValue)
    {
        _valueType = valueType;
        _returnValue = returnValue;
    }

    void LuaObjcBridge::ReturnObject::Handle::setData(const ValueType& valueType, void* oc_id)
    {
        _valueType = valueType;
        m_obj = oc_id;
        CFRetain(m_obj);
        //int ptraddr = *(int *)_handle->m_obj;
    }

    LuaObjcBridge::ReturnObject::ReturnObject(const ValueType& valueType, const ReturnValue& returnValue)
    {
        _handle.reset(new Handle());
        _handle->setData(valueType, returnValue);
    }

    LuaObjcBridge::ReturnObject::ReturnObject(const ValueType& valueType, void* oc_id)
    {
        _handle.reset(new Handle());
        _handle->setData(valueType, oc_id);
    }

    LuaObjcBridge::ReturnObject::ReturnObject(LuaObjcBridge::ReturnObject const&  other)
    {
        _handle = other._handle;
    }

    LuaObjcBridge::ReturnObject& LuaObjcBridge::ReturnObject::operator=(LuaObjcBridge::ReturnObject const& other)
    {
        _handle = other._handle;
    }

    const LuaObjcBridge::ValueType& LuaObjcBridge::ReturnObject::getReturnType() const
    {
        return _handle->_valueType;
    }

    const LuaObjcBridge::ReturnValue& LuaObjcBridge::ReturnObject::getReturnVaule() const
    {
        return _handle->_returnValue;
    }

    const char* LuaObjcBridge::ReturnObject::getStrVaule() const
    {
        //NSMutableString* nsstr = (__bridge id)_handle->m_obj;
        return "";//[nsstr UTF8String];
    }

    void* LuaObjcBridge::ReturnObject::getOcObj()
    {
        return _handle->m_obj;
    }

    void LuaObjcBridge::ReturnObject::getDictionaryMap(std::map<std::string, LuaObjcBridge::ReturnObject> &mapTarget) const
    {
        id oval = (__bridge id) _handle->m_obj;
        if ([oval isKindOfClass:[NSDictionary class]])
        {
            for (id key in oval)
            {
                const char *key_ = [[NSString stringWithFormat:@"%@", key] cStringUsingEncoding:NSUTF8StringEncoding];
                std::string key_str(key_);
                id value = [oval objectForKey:key];
                LuaObjcBridge::ReturnObject obj = _getReturnObjectForOcType(value);
                mapTarget.insert(std::make_pair(key_str, obj));
            }
        }
    }

    LuaObjcBridge::OcFunction::OcFunction()
    {
        id obj = [NSMutableDictionary dictionary];
        argvDatas = (__bridge_retained void *)(obj);
    }

    LuaObjcBridge::OcFunction::~OcFunction()
    {
        id obj = (__bridge_transfer id)argvDatas;
        obj = nil;
    }

    void LuaObjcBridge::OcFunction::pushOcValue(const std::string& key, const std::string& value)
    {
        NSMutableDictionary* dict = (__bridge NSMutableDictionary*)argvDatas;
        NSString *key_ = [NSString stringWithCString:key.c_str() encoding:NSUTF8StringEncoding];
        [dict setObject:[NSString stringWithCString:value.c_str() encoding:NSUTF8StringEncoding] forKey:key_];
    }

    void LuaObjcBridge::OcFunction::pushOcValue(const std::string& key, const float& value)
    {
        NSMutableDictionary* dict = (__bridge NSMutableDictionary*)argvDatas;
        NSString *key_ = [NSString stringWithCString:key.c_str() encoding:NSUTF8StringEncoding];
        [dict setObject:[NSNumber numberWithFloat:value] forKey:key_];
    }

    void LuaObjcBridge::OcFunction::pushOcValue(const std::string& key, bool value)
    {
        NSMutableDictionary* dict = (__bridge NSMutableDictionary*)argvDatas;
        NSString *key_ = [NSString stringWithCString:key.c_str() encoding:NSUTF8StringEncoding];
        [dict setObject:[NSNumber numberWithBool:value] forKey:key_];
    }

    LuaObjcBridge::ReturnObject LuaObjcBridge::OcFunction::callFunction(const std::string &className_, const std::string &methodName_)
    {
        const char* className = className_.c_str();
        const char* methodName = methodName_.c_str();

        LuaObjcBridge::ReturnValue rv;
        LuaObjcBridge::ReturnObject errRet(LuaObjcBridge::TypeInvalid, rv);
        
        Class targetClass = NSClassFromString([NSString stringWithCString:className encoding:NSUTF8StringEncoding]);
        if (!targetClass)
        {
            NSString * class_str= [NSString stringWithCString:className_.c_str() encoding:[NSString defaultCStringEncoding]];
            NSLog(@"OcFunction::callFunction -- error:%@ could not found!", class_str);
            return errRet;
        }
        
        SEL methodSel;
        NSMutableDictionary* dict = (__bridge NSMutableDictionary *)argvDatas;
        if (dict.count > 0) {
            NSString *methodName_ = [NSString stringWithCString:methodName encoding:NSUTF8StringEncoding];
            methodName_ = [NSString stringWithFormat:@"%@:", methodName_];
            methodSel = NSSelectorFromString(methodName_);
        } else {
            methodSel = NSSelectorFromString([NSString stringWithCString:methodName encoding:NSUTF8StringEncoding]);
        }
        
        if (methodSel == (SEL)0) {
            NSString * method_str= [NSString stringWithCString:methodName_.c_str() encoding:[NSString defaultCStringEncoding]];
            NSLog(@"OcFunction::callFunction -- error:%@ could not found!", method_str);
            return errRet;
        }
        
        NSMethodSignature *methodSig = [targetClass methodSignatureForSelector:(SEL)methodSel];
        if (methodSig == nil) {
            NSLog(@"OcFunction::callFunction -- error: methodSig is nil!");
            return errRet;
        }
        
        @try {
            NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:methodSig];
            [invocation setTarget:targetClass];
            [invocation setSelector:methodSel];
            NSUInteger returnLength = [methodSig methodReturnLength];
            const char *returnType = [methodSig methodReturnType];
            
            NSMutableDictionary* dict = (__bridge NSMutableDictionary *)argvDatas;
            
            if(dict.count > 0)
                [invocation setArgument:&dict atIndex:2];
            
            [invocation invoke];

            
            if (returnLength > 0)
            {
                id oval;
                [invocation getReturnValue:&oval];
                return _getReturnObject(returnType, oval);
            }
            return errRet;
        }
        @catch (NSException *exception)
        {
            NSLog(@"EXCEPTION THROW: %@", exception);
            return errRet;
        }
    }
}
