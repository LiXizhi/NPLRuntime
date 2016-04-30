# -*- cmake -*-

if (WIN32)
  find_path(DIRECTX_INCLUDE_DIR dxdiag.h
            "$ENV{DXSDK_DIR}/Include"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2010)/Include"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (April 2007)/Include"
            "C:/DX90SDK/Include"
            "E:/DX90SDK/Include"
            "$ENV{PROGRAMFILES}/DX90SDK/Include"
            )
  if (DIRECTX_INCLUDE_DIR)
    include_directories(${DIRECTX_INCLUDE_DIR})
      message(STATUS "Found DirectX include: ${DIRECTX_INCLUDE_DIR}")
    set(DIRECTX_FOUND true)
  else (DIRECTX_INCLUDE_DIR)
    message(WARNING "Could not find DirectX SDK Include")
	set(DIRECTX_FOUND false)
  endif (DIRECTX_INCLUDE_DIR)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	find_path(DIRECTX_LIBRARY_DIR dxguid.lib
            "$ENV{DXSDK_DIR}/Lib/x64"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2010)/Lib/x64"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (April 2007)/Lib/x64"
			"C:/DX90SDK/Lib/x64"
            "E:/DX90SDK/Lib/x64"
            )
else()
	find_path(DIRECTX_LIBRARY_DIR dxguid.lib
            "$ENV{DXSDK_DIR}/Lib/x86"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2010)/Lib/x86"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (April 2007)/Lib/x86"
			"C:/DX90SDK/Lib/x86"
            "E:/DX90SDK/Lib/x86"
            )
endif()
  
  if (DIRECTX_LIBRARY_DIR)
    message(STATUS "Found DirectX library: ${DIRECTX_LIBRARY_DIR}")
  else (DIRECTX_LIBRARY_DIR)
    message(WARNING "Could not find DirectX SDK Libraries")
  endif (DIRECTX_LIBRARY_DIR)

endif (WIN32)
