package com.tatfook.paracraft;

import java.util.Locale;

public class ParaEngineHelper {
	public static String getCurrentLanguage() {
		return Locale.getDefault().getLanguage();
	}
}