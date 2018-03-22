package com.tatfook.paracraft;

import android.view.WindowManager;
import android.os.Bundle;
import android.app.NativeActivity;


public class AppActivity extends NativeActivity{  
	@Override   
	protected void onCreate(Bundle icicle) {   
		super.onCreate(icicle);  
  
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);   
	} 
}
