//-----------------------------------------------------------------------------
// ParaEngineApplication.java
// Authors: LanZhihong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

package com.tatfook.paracraft;

import android.app.Application;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.util.Log;
import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;
import java.util.Iterator;
import java.util.HashMap;
import com.tatfook.paracraft.ParaEnginePluginWrapper.PluginInfo;

public class ParaEngineApplication extends Application {
	public static final String META_DATA_PLUGIN_INFO = "PlguinWrapper";

	@Override
	public void onCreate()	{
		super.onCreate();
		this.initPluginWrapper();
	}

	private void initPluginWrapper() {
		ApplicationInfo applicationInfo; 
		String info = "";

		try {
			applicationInfo = this.getPackageManager().getApplicationInfo(getPackageName(), PackageManager.GET_META_DATA);
            if (applicationInfo.metaData != null) {
                info = applicationInfo.metaData.getString(META_DATA_PLUGIN_INFO);
			}
		} catch (PackageManager.NameNotFoundException e) {
            throw new RuntimeException("Error getting application info", e);
        }

		Log.i("ParaEngine", info);

		if (info.equals(""))
			return;

		try {
			JSONArray pluginInfos = new JSONArray(info);

			for (int i = 0; i < pluginInfos.length(); i++)
			{
				JSONObject pluginInfo = pluginInfos.getJSONObject(i);
				String packname = pluginInfo.optString("packname");
				boolean bDebug = pluginInfo.optBoolean("debug");

				PluginInfo pi = new PluginInfo();
				pi.debug = bDebug;
				pi.name = packname;

				if (pluginInfo.length() > 2) {
					pi.initParams = new HashMap<String, Object>();

					Iterator<String> iterator = pluginInfo.keys(); 
					while (iterator.hasNext()) {
						String key = (String)iterator.next();
						if (key.equals("packname") || key.equals("debug"))
							continue;

						Object value = pluginInfo.get(key);
						pi.initParams.put(key, value);
					}
				}

				ParaEnginePluginWrapper.addPluginInfo(pi);
			}
		} catch(JSONException e) {
			throw new RuntimeException("Error parser plugin info", e);
		}
	}
}
