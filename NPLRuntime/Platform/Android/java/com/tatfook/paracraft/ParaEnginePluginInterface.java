package com.tatfook.paracraft;

import java.util.Map;
import android.content.Intent;
import android.os.Bundle;
import android.content.Context;
import com.tatfook.paracraft.ParaEnginePluginWrapper.PluginWrapperListener;

public interface ParaEnginePluginInterface {

	/**
	 * init all plugins
	 * 
	 * @param bDebug
	 *
	 * 
	 */
	public void onInit(Map<String, Object> cpInfo, boolean bDebug);

	/**
	 * 
	 * @param cxt
	 * @param listener
	 *
	 *
	 * @return is needed to wait for finished
	 */
	public boolean onCreate(Context cxt, Bundle savedInstanceState, PluginWrapperListener listener);

	public void onDestroy();
	public void onStart();
	public void onStop();
	public void onPause();
	public void onResume();
	public void onAppBackground();
	public void onAppForeground();
	public void onActivityResult(int requestCode, int resultCode, Intent data);
	public void setDebugMode(boolean debug);
	public void onSaveInstanceState(Bundle outState);
}