//-----------------------------------------------------------------------------
// File: D3DRes.h
//
// Desc: Resource definitions required by the CD3DApplication class.
//       Any application using the CD3DApplication class must include resources
//       with the following identifiers.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef D3DRES_H
#define D3DRES_H


#define IDI_MAIN_ICON          103 // Application icon
#define IDR_MAIN_ACCEL         113 // Keyboard accelerator
#define IDR_MENU               141 // Application menu
#define IDR_POPUP              142 // Popup menu
#define IDD_SELECTDEVICE       144 // "Change Device" dialog box

#define IDC_ADAPTER_COMBO         1002 // Adapter combobox for "SelectDevice" dlg
#define IDC_DEVICE_COMBO          1000 // Device combobox for "SelectDevice" dlg
#define IDC_ADAPTERFORMAT_COMBO   1003
#define IDC_RESOLUTION_COMBO      1004
#define IDC_MULTISAMPLE_COMBO     1005 // MultiSample combobox for "SelectDevice" dlg
#define IDC_REFRESHRATE_COMBO     1006
#define IDC_BACKBUFFERFORMAT_COMBO 1007
#define IDC_DEPTHSTENCILBUFFERFORMAT_COMBO 1008
#define IDC_VERTEXPROCESSING_COMBO 1009
#define IDC_PRESENTINTERVAL_COMBO 1010
#define IDC_MULTISAMPLE_QUALITY_COMBO   1011
#define IDC_DEVICECLIP_CHECK      1012
#define IDC_WINDOW                1016 // Radio button for windowed-mode
#define IDC_FULLSCREEN            1018 // Radio button for fullscreen-mode

#define IDM_CHANGEDEVICE     40002 // Command to invoke "Change Device" dlg
#define IDM_TOGGLEFULLSCREEN 40003 // Command to toggle fullscreen mode
#define IDM_TOGGLESTART      40004 // Command to toggle frame animation
#define IDM_SINGLESTEP       40005 // Command to single step frame animation
#define IDM_EXIT             40006 // Command to exit the application
#define IDM_HELP             40007 // Command to launch readme.txt


#endif // D3DRES_H
