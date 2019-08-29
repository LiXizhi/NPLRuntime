package com.tatfook.paracraft;

import android.support.annotation.Keep;
import android.util.Log;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

@Keep
public class ParaEngineReflectionHelper {
    @Nullable
    @SuppressWarnings("unchecked")
    public static <T> T getConstantValue(final Class aClass, final String constantName) {
        try {
            Object obj = aClass.getDeclaredField(constantName).get(null);
            return (T)obj;
        } catch (NoSuchFieldException e) {
            Log.e("error", "can not find " + constantName + " in " + aClass.getName());
        }
        catch (IllegalAccessException e) {
            Log.e("error", constantName + " is not accessible");
        }
        catch (IllegalArgumentException e) {
            Log.e("error", "arguments error when get " + constantName);
        }
        catch (Exception e) {
            Log.e("error", "can not get constant" + constantName);
        }

        return null;
    }

    @Nullable
    @SuppressWarnings("unchecked")
    public static <T> T invokeInstanceMethod(@NotNull final Object instance, final String methodName,
                                             final Class[] parameterTypes, final Object[] parameters) {

        final Class<?> aClass = instance.getClass();
        try {
            Method method = aClass.getMethod(methodName, parameterTypes);
            if (method == null)
                return null;

            Object obj = method.invoke(instance, parameters);
            if (obj == null)
                return null;
            return (T)obj;
        } catch (NoSuchMethodException e) {
            Log.e("error", "can not find " + methodName + " in " + aClass.getName());
        }
        catch (IllegalAccessException e) {
            Log.e("error", methodName + " is not accessible");
        }
        catch (IllegalArgumentException e) {
            Log.e("error", "arguments are error when invoking " + methodName);
        }
        catch (InvocationTargetException e) {
            Log.e("error", "an exception was thrown by the invoked method when invoking " + methodName);
        }
        catch (Exception e) {
            Log.e("error", "can not get invoke instance method" + methodName);
        }

        return null;
    }
}
