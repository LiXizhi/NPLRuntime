package com.tatfook.paracraft;

import android.view.WindowManager;
import android.os.Bundle;
import android.app.NativeActivity;
import android.view.KeyEvent;  
import android.content.pm.PackageManager;
import android.content.pm.ActivityInfo;


public class AppActivity extends NativeActivity{  
	@Override   
	protected void onCreate(Bundle icicle) {   

		String libname = "main";
		ActivityInfo ai;

		 try {
            ai = getPackageManager().getActivityInfo(
                    getIntent().getComponent(), PackageManager.GET_META_DATA);
            if (ai.metaData != null) {
                String ln = ai.metaData.getString(META_DATA_LIB_NAME);
                if (ln != null) libname = ln;
            }
        } catch (PackageManager.NameNotFoundException e) {
            throw new RuntimeException("Error getting activity info", e);
        }

		System.loadLibrary(libname);

		super.onCreate(icicle);  
		
		// TODO: add styles to remove both title and action bar
		//getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);   
		//getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

	} 

	@Override    
    public boolean onKeyDown(int keyCode, KeyEvent event) {    
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {    
             return false; 
		}
		else if (keyCode == KeyEvent.KEYCODE_MENU) {
			return false;
		}
        else {    
            return super.onKeyDown(keyCode, event);    
        }    
	}
}
