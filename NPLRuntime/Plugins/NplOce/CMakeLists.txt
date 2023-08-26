# if (NPLRUNTIME_SERVER OR ((NOT WIN32) AND (NOT ANDROID) AND (NOT (APPLE AND (NOT IOS)))))
# 	message(FATAL_ERROR "Unspported system.")
# endif()

set(CROSS_PLATFORM true)

SET(FREETYPE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/externals/freetype-2.8.1/include)
SET(FREETYPE_INCLUDE_DIR ${FREETYPE_INCLUDE_DIRS})
SET(FREETYPE_FOUND true)
SET(FREETYPE_STATIC ON)

#SET(HAVE_FT_GET_VAR_BLEND_COORDINATES TRUE)
#SET(HAVE_FT_SET_VAR_BLEND_COORDINATES TRUE)
#SET(HAVE_FT_DONE_MM_VAR FALSE)
set(HB_HAVE_FREETYPE ON)
add_subdirectory(NplOce-Deps/libs/harfbuzz)

SET(OCE_VISUALISATION OFF)
SET(OCE_DATAEXCHANGE OFF)
SET(OCE_OCAF OFF)
SET(OCE_ENABLE_DEB_FLAG OFF)
SET(OCE_COPY_HEADERS_BUILD ON)
SET(OCE_TESTING OFF)
SET(OCE_BUILD_SHARED_LIB OFF)
SET(OCE_NO_LIBRARY_VERSION true)

SET(OCE_INCLUDE_DIRS ${PROJECT_BINARY_DIR}/oce/include ${PROJECT_BINARY_DIR}/oce/build_inc)
add_subdirectory(NplOce-Deps/libs/oce ${PROJECT_BINARY_DIR}/oce)
	
	
set(DEPS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/NplOce-Deps/libs)
include(${PROJECT_SOURCE_DIR}/cmake/ConfigBoost.cmake)
set(NPLRUNTIME_ROOT ${PROJECT_SOURCE_DIR}/../)
add_subdirectory(NplOce)

if (MSVC)
	target_link_options(nploce PUBLIC /export:NplOce_StaticLoad)
else()
	#target_link_options(nploce PUBLIC -defsym,NplOce_StaticLoad=NplOce_StaticLoad)
	#target_link_options(nploce PUBLIC -undefined=NplOce_StaticLoad)
	#target_compile_options(nploce PUBLIC -Wl,-undefined=NplOce_StaticLoad)
	
	target_sources(nploce PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/PlaceHolder.cpp)
endif()
 