//-----------------------------------------------------------------------------
// ParaEnginePluginWrapper.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import java.util.Map;
import java.util.LinkedList;
import java.util.List;
import java.util.LinkedHashMap;

public class ParaEnginePluginWrapper {
    protected static Context sContext = null;
    private static final String TAG = "ParaEngine";

    protected static Map<String, Object> sPlugins = new LinkedHashMap<String, Object>();
    protected static List<PluginInfo> pluginInfoList = new LinkedList<PluginInfo>();
    private static int count = 0;

    public static interface PluginWrapperListener {
        public void onInit();
    }

    public static class PluginInfo {
        public boolean debug;
        public String name;
        public Map<String, Object> initParams = null;
    }

    public static void addPluginInfo(PluginInfo info) {
        pluginInfoList.add(info);
    }

    public static Context getContext() {
        return sContext;
    }

    protected static Object initPlugin(String classFullName, Map<String, Object> initParams, boolean bDebug) {
        Log.i(TAG, "class name : ----" + classFullName + "----");

        if (sPlugins.containsKey(classFullName)) {
            return sPlugins.get(classFullName);
        }

        Class<?> c = null;

        try {
            String fullName = classFullName.replace('/', '.');
            c = Class.forName(fullName);
        } catch (ClassNotFoundException e) {
            Log.i(TAG, "Class " + classFullName + " not found.");
            e.printStackTrace();
            return null;
        }

        try {
            Object o = c.newInstance();
            if (o != null) {
                sPlugins.put(classFullName, o);

                ParaEnginePluginInterface p = (ParaEnginePluginInterface)o;
                p.onInit(initParams, bDebug);

                return o;
            } else {
                Log.i(TAG, "Plugin " + classFullName + " wasn't initialized.");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    public static Object getPlugin(String classFullName)
    {
        if (sPlugins.containsKey(classFullName)) {
            return sPlugins.get(classFullName);
        } else {
            return null;
        }
    }

    public static boolean loadPlugin(String classFullName, Map<String , Object> initParams, boolean bDebug, final PluginWrapperListener listener)
    {
        if (getContext() == null)
        {
            return false;
        }

        if (sPlugins.containsKey(classFullName)) 
        {
            return false;
        }
        
        Object p = initPlugin(classFullName, initParams, bDebug);
        
        if (p != null)
        {
            ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)p;

            return plugin.onCreate(getContext(), null, listener);
        }
        
        return false;
    }

    public static boolean initPluginInfo() {
        return false;
    }

    /**
     * init all plugins
     * 
     * @param context
     * @param listener
     *
     * @return is needed to wait for finished
     */
     public static boolean init(Context context, Bundle savedInstanceState, final PluginWrapperListener listener) {
        count = 0;
        sContext = context;

        if (pluginInfoList == null) {
            return false;
        }

        boolean bRet = false;

        try {
            for (int i = 0; i < pluginInfoList.size(); i++) {
                PluginInfo info = pluginInfoList.get(i);
                Object p = initPlugin(info.name, info.initParams, info.debug);
                if (p != null) {
                    ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)p;

                    if (plugin.onCreate(sContext, savedInstanceState,
                        new ParaEnginePluginWrapper.PluginWrapperListener() {
                            @Override
                            public void onInit() {
                                count--;
                                assert(count >= 0);
                                if (count == 0) {
                                    listener.onInit();
                                }
                            }
                        })) {
                        count++;

                        bRet = true;
                    } else {
                        Log.e(TAG, "==========initPlugin(info.name) is null=======");
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return bRet;
     }

    public static void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();
                if (plugin != null) {
                    plugin.onRequestPermissionsResult(requestCode, permissions, grantResults);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void onDestroy() {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();
                if (plugin != null) {
                    plugin.onDestroy();
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void onStart() {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();
                if (plugin != null) {
                    plugin.onStart();

                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void onStop() {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();
                if (plugin != null) {
                    plugin.onStop();
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void onPause() {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();
                if (plugin != null) {
                    plugin.onPause();
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void onResume() {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();
                if (plugin != null) {
                    plugin.onResume();
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void onActivityResult(int requestCode, int resultCode, Intent data) {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();

                if (plugin != null) {
                    plugin.onActivityResult(requestCode, resultCode, data);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void onAppBackground() {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();
                if (plugin != null) {
                    plugin.onAppBackground();
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void onAppForeground() {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();
                if (plugin != null) {
                    plugin.onAppForeground();
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void onSaveInstanceState(Bundle outState) {
        try {
            for (Map.Entry<String, Object> entry : sPlugins.entrySet()) {
                ParaEnginePluginInterface plugin = (ParaEnginePluginInterface)entry.getValue();
                if (plugin != null) {
                    plugin.onSaveInstanceState(outState);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
