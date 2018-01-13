//-----------------------------------------------------------------------------
// File: D3DSettings.cpp
//
// Desc: Settings class and change-settings dialog class for the Direct3D 
//       samples framework library.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
CD3DSettingsDialog* s_pSettingsDialog = NULL;


//-----------------------------------------------------------------------------
// Name: D3DDevTypeToString
// Desc: Returns the string for the given D3DDEVTYPE.
//-----------------------------------------------------------------------------
TCHAR* D3DDevTypeToString(D3DDEVTYPE devType)
{
    switch (devType)
    {
    case D3DDEVTYPE_HAL:        return TEXT("D3DDEVTYPE_HAL");
    case D3DDEVTYPE_SW:         return TEXT("D3DDEVTYPE_SW");
    case D3DDEVTYPE_REF:        return TEXT("D3DDEVTYPE_REF");
    default:                    return TEXT("Unknown devType");
    }
}


//-----------------------------------------------------------------------------
// Name: MultisampleTypeToString
// Desc: Returns the string for the given D3DMULTISAMPLE_TYPE.
//-----------------------------------------------------------------------------
TCHAR* MultisampleTypeToString(D3DMULTISAMPLE_TYPE MultiSampleType)
{
    switch (MultiSampleType)
    {
    case D3DMULTISAMPLE_NONE:   return TEXT("D3DMULTISAMPLE_NONE");
    case D3DMULTISAMPLE_NONMASKABLE: return TEXT("D3DMULTISAMPLE_NONMASKABLE");
    case D3DMULTISAMPLE_2_SAMPLES: return TEXT("D3DMULTISAMPLE_2_SAMPLES");
    case D3DMULTISAMPLE_3_SAMPLES: return TEXT("D3DMULTISAMPLE_3_SAMPLES");
    case D3DMULTISAMPLE_4_SAMPLES: return TEXT("D3DMULTISAMPLE_4_SAMPLES");
    case D3DMULTISAMPLE_5_SAMPLES: return TEXT("D3DMULTISAMPLE_5_SAMPLES");
    case D3DMULTISAMPLE_6_SAMPLES: return TEXT("D3DMULTISAMPLE_6_SAMPLES");
    case D3DMULTISAMPLE_7_SAMPLES: return TEXT("D3DMULTISAMPLE_7_SAMPLES");
    case D3DMULTISAMPLE_8_SAMPLES: return TEXT("D3DMULTISAMPLE_8_SAMPLES");
    case D3DMULTISAMPLE_9_SAMPLES: return TEXT("D3DMULTISAMPLE_9_SAMPLES");
    case D3DMULTISAMPLE_10_SAMPLES: return TEXT("D3DMULTISAMPLE_10_SAMPLES");
    case D3DMULTISAMPLE_11_SAMPLES: return TEXT("D3DMULTISAMPLE_11_SAMPLES");
    case D3DMULTISAMPLE_12_SAMPLES: return TEXT("D3DMULTISAMPLE_12_SAMPLES");
    case D3DMULTISAMPLE_13_SAMPLES: return TEXT("D3DMULTISAMPLE_13_SAMPLES");
    case D3DMULTISAMPLE_14_SAMPLES: return TEXT("D3DMULTISAMPLE_14_SAMPLES");
    case D3DMULTISAMPLE_15_SAMPLES: return TEXT("D3DMULTISAMPLE_15_SAMPLES");
    case D3DMULTISAMPLE_16_SAMPLES: return TEXT("D3DMULTISAMPLE_16_SAMPLES");
    default:                    return TEXT("Unknown Multisample Type");
    }
}


//-----------------------------------------------------------------------------
// Name: VertexProcessingTypeToString
// Desc: Returns the string for the given VertexProcessingType.
//-----------------------------------------------------------------------------
TCHAR* VertexProcessingTypeToString(VertexProcessingType vpt)
{
    switch (vpt)
    {
    case SOFTWARE_VP:      return TEXT("SOFTWARE_VP");
    case MIXED_VP:         return TEXT("MIXED_VP");
    case HARDWARE_VP:      return TEXT("HARDWARE_VP");
    case PURE_HARDWARE_VP: return TEXT("PURE_HARDWARE_VP");
    default:               return TEXT("Unknown VertexProcessingType");
    }
}


//-----------------------------------------------------------------------------
// Name: PresentIntervalToString
// Desc: Returns the string for the given present interval.
//-----------------------------------------------------------------------------
TCHAR* PresentIntervalToString( UINT pi )
{
    switch( pi )
    {
    case D3DPRESENT_INTERVAL_IMMEDIATE: return TEXT("D3DPRESENT_INTERVAL_IMMEDIATE");
    case D3DPRESENT_INTERVAL_DEFAULT:   return TEXT("D3DPRESENT_INTERVAL_DEFAULT");
    case D3DPRESENT_INTERVAL_ONE:       return TEXT("D3DPRESENT_INTERVAL_ONE");
    case D3DPRESENT_INTERVAL_TWO:       return TEXT("D3DPRESENT_INTERVAL_TWO");
    case D3DPRESENT_INTERVAL_THREE:     return TEXT("D3DPRESENT_INTERVAL_THREE");
    case D3DPRESENT_INTERVAL_FOUR:      return TEXT("D3DPRESENT_INTERVAL_FOUR");
    default:                            return TEXT("Unknown PresentInterval");
    }
}




//-----------------------------------------------------------------------------
// Name: DialogProcHelper
// Desc: 
//-----------------------------------------------------------------------------
INT_PTR CALLBACK DialogProcHelper( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return s_pSettingsDialog->DialogProc( hDlg, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: CD3DSettingsDialog constructor
// Desc: 
//-----------------------------------------------------------------------------
CD3DSettingsDialog::CD3DSettingsDialog( CD3DEnumeration* pEnumeration, 
                                        CD3DSettings* pSettings)
{
    s_pSettingsDialog = this;
    m_pEnumeration = pEnumeration;
    m_d3dSettings = *pSettings;
}




//-----------------------------------------------------------------------------
// Name: ComboBoxAdd
// Desc: Adds an entry to the combo box.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::ComboBoxAdd( int id, void* pData, LPCTSTR pstrDesc )
{
    HWND hwndCtrl = GetDlgItem( m_hDlg, id );
    DWORD dwItem = ComboBox_AddString( hwndCtrl, pstrDesc );
    ComboBox_SetItemData( hwndCtrl, dwItem, pData );
}




//-----------------------------------------------------------------------------
// Name: ComboBoxSelect
// Desc: Selects an entry in the combo box.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::ComboBoxSelect( int id, void* pData )
{
    HWND hwndCtrl = GetDlgItem( m_hDlg, id );
    UINT count = ComboBoxCount( id );
    for( UINT iItem = 0; iItem < count; iItem++ )
    {
        if( (void*)ComboBox_GetItemData( hwndCtrl, iItem ) == pData )
        {
            ComboBox_SetCurSel( hwndCtrl, iItem );
            PostMessage( m_hDlg, WM_COMMAND, 
                MAKEWPARAM( id, CBN_SELCHANGE ), (LPARAM)hwndCtrl );
            return;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: ComboBoxSelectIndex
// Desc: Selects an entry in the combo box.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::ComboBoxSelectIndex( int id, int index )
{
    HWND hwndCtrl = GetDlgItem( m_hDlg, id );
    ComboBox_SetCurSel( hwndCtrl, index );
    PostMessage( m_hDlg, WM_COMMAND, MAKEWPARAM( id, CBN_SELCHANGE ), 
        (LPARAM)hwndCtrl );
}




//-----------------------------------------------------------------------------
// Name: ComboBoxSelected
// Desc: Returns the data for the selected entry in the combo box.
//-----------------------------------------------------------------------------
void* CD3DSettingsDialog::ComboBoxSelected( int id )
{
    HWND hwndCtrl = GetDlgItem( m_hDlg, id );
    int index = ComboBox_GetCurSel( hwndCtrl );
    if( index < 0 )
        return NULL;
    return (void*)ComboBox_GetItemData( hwndCtrl, index );
}




//-----------------------------------------------------------------------------
// Name: ComboBoxSomethingSelected
// Desc: Returns whether any entry in the combo box is selected.  This is 
//       more useful than ComboBoxSelected() when you need to distinguish 
//       between having no item selected vs. having an item selected whose 
//       itemData is NULL.
//-----------------------------------------------------------------------------
bool CD3DSettingsDialog::ComboBoxSomethingSelected( int id )
{
    HWND hwndCtrl = GetDlgItem( m_hDlg, id );
    int index = ComboBox_GetCurSel( hwndCtrl );
    return ( index >= 0 );
}




//-----------------------------------------------------------------------------
// Name: ComboBoxCount
// Desc: Returns the number of entries in the combo box.
//-----------------------------------------------------------------------------
UINT CD3DSettingsDialog::ComboBoxCount( int id )
{
    HWND hwndCtrl = GetDlgItem( m_hDlg, id );
    return ComboBox_GetCount( hwndCtrl );
}




//-----------------------------------------------------------------------------
// Name: ComboBoxClear
// Desc: Clears the entries in the combo box.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::ComboBoxClear( int id )
{
    HWND hwndCtrl = GetDlgItem( m_hDlg, id );
    ComboBox_ResetContent( hwndCtrl );
}




//-----------------------------------------------------------------------------
// Name: ComboBoxContainsText
// Desc: Returns whether the combo box contains the given text.
//-----------------------------------------------------------------------------
bool CD3DSettingsDialog::ComboBoxContainsText( int id, LPCTSTR pstrText )
{
    TCHAR strItem[200];
    HWND hwndCtrl = GetDlgItem( m_hDlg, id );
    UINT count = ComboBoxCount( id );
    for( UINT iItem = 0; iItem < count; iItem++ )
    {
        if( ComboBox_GetLBTextLen( hwndCtrl, iItem ) >= 200 )
            continue; // shouldn't happen, but don't overwrite buffer if it does
        ComboBox_GetLBText( hwndCtrl, iItem, strItem );
        if( lstrcmp( strItem, pstrText ) == 0 )
            return true;
    }
    return false;
}




//-----------------------------------------------------------------------------
// Name: ShowDialog
// Desc: Show the D3D settings dialog.
//-----------------------------------------------------------------------------
INT_PTR CD3DSettingsDialog::ShowDialog( HWND hwndParent, HINSTANCE hInst )
{
    return DialogBox( hInst, MAKEINTRESOURCE( IDD_SELECTDEVICE ), 
        hwndParent, DialogProcHelper );
}




//-----------------------------------------------------------------------------
// Name: DialogProc
// Desc: Handle window messages in the dialog.
//-----------------------------------------------------------------------------
INT_PTR CD3DSettingsDialog::DialogProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( lParam );

    switch( msg )
    {
    case WM_INITDIALOG:
        {
            m_hDlg = hDlg;

            // Fill adapter combo box.  Updating the selected adapter will trigger
            // updates of the rest of the dialog.
            for( UINT iai = 0; iai < m_pEnumeration->m_pAdapterInfoList->Count(); iai++ )
            {
                D3DAdapterInfo* pAdapterInfo;
                pAdapterInfo = (D3DAdapterInfo*)m_pEnumeration->m_pAdapterInfoList->GetPtr(iai);
                TCHAR strDescription[512];
                DXUtil_ConvertAnsiStringToGenericCch( strDescription, pAdapterInfo->AdapterIdentifier.Description, 512 );
                ComboBoxAdd( IDC_ADAPTER_COMBO, pAdapterInfo, strDescription );
                if( pAdapterInfo->AdapterOrdinal == m_d3dSettings.AdapterOrdinal() )
                    ComboBoxSelect( IDC_ADAPTER_COMBO, pAdapterInfo );
            }
            if( !ComboBoxSomethingSelected( IDC_ADAPTER_COMBO ) &&
                ComboBoxCount( IDC_ADAPTER_COMBO ) > 0 )
            {
                ComboBoxSelectIndex( IDC_ADAPTER_COMBO, 0 );
            }
        }
        return TRUE;

    case WM_COMMAND:
        switch( LOWORD(wParam) )
        {
        case IDOK:
            EndDialog( hDlg, IDOK );
            break;
        case IDCANCEL:
            EndDialog( hDlg, IDCANCEL );
            break;
        case IDC_ADAPTER_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                AdapterChanged();
            break;
        case IDC_DEVICE_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                DeviceChanged();
            break;
        case IDC_ADAPTERFORMAT_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                AdapterFormatChanged();
            break;
        case IDC_RESOLUTION_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                ResolutionChanged();
            break;
        case IDC_REFRESHRATE_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                RefreshRateChanged();
            break;
        case IDC_BACKBUFFERFORMAT_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                BackBufferFormatChanged();
            break;
        case IDC_DEPTHSTENCILBUFFERFORMAT_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                DepthStencilBufferFormatChanged();
            break;
        case IDC_MULTISAMPLE_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                MultisampleTypeChanged();
            break;
        case IDC_MULTISAMPLE_QUALITY_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                MultisampleQualityChanged();
            break;
        case IDC_VERTEXPROCESSING_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                VertexProcessingChanged();
            break;
        case IDC_PRESENTINTERVAL_COMBO:
            if( CBN_SELCHANGE == HIWORD(wParam) )
                PresentIntervalChanged();
            break;
        case IDC_DEVICECLIP_CHECK:
            if( BN_CLICKED == HIWORD(wParam) )
                DeviceClipChanged();
            break;
        case IDC_WINDOW:
        case IDC_FULLSCREEN:
            WindowedFullscreenChanged();
            break;
        }
        return TRUE;

    default:
        return FALSE;
    }
}



//-----------------------------------------------------------------------------
// Name: AdapterChanged
// Desc: Respond to a change of selected adapter.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::AdapterChanged( void )
{
    D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)ComboBoxSelected( IDC_ADAPTER_COMBO );
    if( pAdapterInfo == NULL )
        return;
    
    if( m_d3dSettings.IsWindowed )
    {
        m_d3dSettings.pWindowed_AdapterInfo = pAdapterInfo;

        // Set the windowed display mode format to what the
        // adapter is currently in.
        IDirect3D9* pD3D = m_pEnumeration->GetD3D();
        if( pD3D )
        {
            D3DDISPLAYMODE Mode;
            if( SUCCEEDED( pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &Mode ) ) )
                m_d3dSettings.Windowed_DisplayMode.Format = Mode.Format;
            SAFE_RELEASE( pD3D );
        }
    }
    else
        m_d3dSettings.pFullscreen_AdapterInfo = pAdapterInfo;

    // Update device combo box
    ComboBoxClear( IDC_DEVICE_COMBO );
    for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
    {
        D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
        ComboBoxAdd( IDC_DEVICE_COMBO, pDeviceInfo, 
                     D3DDevTypeToString( pDeviceInfo->DevType ) );
        if( pDeviceInfo->DevType == m_d3dSettings.DevType() )
            ComboBoxSelect( IDC_DEVICE_COMBO, pDeviceInfo );
    }
    if( !ComboBoxSomethingSelected( IDC_DEVICE_COMBO ) &&
        ComboBoxCount( IDC_DEVICE_COMBO ) > 0 )
    {
        ComboBoxSelectIndex( IDC_DEVICE_COMBO, 0 );
    }
}




//-----------------------------------------------------------------------------
// Name: DeviceChanged
// Desc: Respond to a change of selected device by resetting the 
//       fullscreen/windowed radio buttons.  Updating these buttons will 
//       trigger updates of the rest of the dialog.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::DeviceChanged( void )
{
    D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)ComboBoxSelected( IDC_DEVICE_COMBO );
    if( pDeviceInfo == NULL )
        return;

    if( m_d3dSettings.IsWindowed )
        m_d3dSettings.pWindowed_DeviceInfo = pDeviceInfo;
    else
        m_d3dSettings.pFullscreen_DeviceInfo = pDeviceInfo;

    // Update fullscreen/windowed radio buttons
    bool HasWindowedDeviceCombo = false;
    bool HasFullscreenDeviceCombo = false;
    for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
    {
        D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);
        if( pDeviceCombo->IsWindowed )
            HasWindowedDeviceCombo = true;
        else
            HasFullscreenDeviceCombo = true;
    }
    EnableWindow( GetDlgItem( m_hDlg, IDC_WINDOW ), HasWindowedDeviceCombo );
    EnableWindow( GetDlgItem( m_hDlg, IDC_FULLSCREEN ), HasFullscreenDeviceCombo );
    if (m_d3dSettings.IsWindowed && HasWindowedDeviceCombo)
    {
        CheckRadioButton( m_hDlg, IDC_WINDOW, IDC_FULLSCREEN, IDC_WINDOW );
    }
    else
    {
        CheckRadioButton( m_hDlg, IDC_WINDOW, IDC_FULLSCREEN, IDC_FULLSCREEN );
    }
    WindowedFullscreenChanged();
}




//-----------------------------------------------------------------------------
// Name: WindowedFullscreenChanged
// Desc: Respond to a change of windowed/fullscreen state by rebuilding the
//       adapter format list, resolution list, and refresh rate list.
//       Updating the selected adapter format will trigger updates of the 
//       rest of the dialog.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::WindowedFullscreenChanged( void )
{
   
}




//-----------------------------------------------------------------------------
// Name: AdapterFormatChanged
// Desc: Respond to a change of selected adapter format by rebuilding the
//       resolution list and back buffer format list.  Updating the selected 
//       resolution and back buffer format will trigger updates of the rest 
//       of the dialog.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::AdapterFormatChanged( void )
{
    
}




//-----------------------------------------------------------------------------
// Name: ResolutionChanged
// Desc: Respond to a change of selected resolution by rebuilding the
//       refresh rate list.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::ResolutionChanged( void )
{

}




//-----------------------------------------------------------------------------
// Name: RefreshRateChanged
// Desc: Respond to a change of selected refresh rate.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::RefreshRateChanged( void )
{

}




//-----------------------------------------------------------------------------
// Name: BackBufferFormatChanged
// Desc: Respond to a change of selected back buffer format by rebuilding
//       the depth/stencil format list, multisample type list, and vertex
//       processing type list.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::BackBufferFormatChanged( void )
{
   
}




//-----------------------------------------------------------------------------
// Name: DepthStencilBufferFormatChanged
// Desc: Respond to a change of selected depth/stencil buffer format.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::DepthStencilBufferFormatChanged( void )
{

}





//-----------------------------------------------------------------------------
// Name: MultisampleTypeChanged
// Desc: Respond to a change of selected multisample type.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::MultisampleTypeChanged( void )
{

}




//-----------------------------------------------------------------------------
// Name: MultisampleQualityChanged
// Desc: Respond to a change of selected multisample quality.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::MultisampleQualityChanged( void )
{

}




//-----------------------------------------------------------------------------
// Name: VertexProcessingChanged
// Desc: Respond to a change of selected vertex processing type.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::VertexProcessingChanged( void )
{

}




//-----------------------------------------------------------------------------
// Name: PresentIntervalChanged
// Desc: Respond to a change of selected present interval.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::PresentIntervalChanged( void )
{

}




//-----------------------------------------------------------------------------
// Name: DeviceClipChanged
// Desc: Respond to a change of selected present interval.
//-----------------------------------------------------------------------------
void CD3DSettingsDialog::DeviceClipChanged( void )
{

}
