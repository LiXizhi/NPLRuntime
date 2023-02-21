cmake_minimum_required (VERSION 3.20.0)

project(luasocket VERSION 3.0.0 LANGUAGES C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if (LINUX)
	add_definitions(-DLUASOCKET_DEBUG)
elseif(MACOS OR IOS)
	add_definitions(-DLUASOCKET_DEBUG)
	add_definitions(-DUNIX_HAS_SUN_LEN)
elseif(WIN32)
	add_definitions(-DLUASOCKET_DEBUG)
	add_definitions(-DNDEBUG)
	if (MINGW32)
		add_definitions(-DWINVER=0x0501)
	endif()
else()
endif()

list(APPEND SOCKET_SRC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/luasocket.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/timeout.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/io.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/auxiliar.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/compat.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/options.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/inet.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/except.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/select.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/tcp.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/udp.c
)
if (MSVC OR WIN32)
	list(APPEND SOCKET_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/wsocket.c)
else()
	list(APPEND SOCKET_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/usocket.c)
endif()

list(APPEND MIME_SRC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/mime.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/compat.c
)

add_library(${PROJECT_NAME} STATIC ${SOCKET_SRC_FILES} ${MIME_SRC_FILES})

# message("--------------LUA_SOURCE_DIR---------------" ${LUA_SOURCE_DIR})
# 依赖库
target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_SOURCE_DIR}/src)

# list(APPEND UNIX_SRC_FILE_LIST
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/auxiliar.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/options.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/timeout.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/io.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/usocket.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/unixstream.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/unixdgram.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/compat.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/unix.c
# )

# list(APPEND SERIAL_SRC_FILE_LIST
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/buffer.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/compat.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/auxiliar.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/options.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/timeout.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/io.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/usocket.c
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/serial.c
# )

# install(FILES
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/ltn12.lua
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/socket.lua
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/mime.lua
# 	DESTINATION lua
# )

# install(FILES
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/http.lua 
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/url.lua 
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/tp.lua 
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/ftp.lua 
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/headers.lua 
# 	${CMAKE_CURRENT_SOURCE_DIR}/src/smtp.lua
# 	DESTINATION lua/scoket
# )

# # make install or cmake --install 
# # 拷贝可执行文件和库文件,头文件到安装目录的bin|lib|include
# install(TARGETS ${PROJECT_NAME}
# 	EXPORT ${PROJECT_NAME}Targets
# 	ARCHIVE DESTINATION lib 
# 	LIBRARY DESTINATION lib 
# 	RUNTIME DESTINATION bin
# )

# # 识别静态库后缀名
# if (MSVC OR WIN32) 
# 	set(STATIC_LIB_SUFFIX .lib)
# else()
# 	set(STATIC_LIB_SUFFIX .a)
# endif()

# # 设置头文件目录和库路径以便传递给LuaConfig.cmake
# set(LUASOCKET_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libscoket${STATIC_LIB_SUFFIX})

# # 生成版本兼容配置
# include(CMakePackageConfigHelpers)
# write_basic_package_version_file(
# 	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
# 	VERSION ${PACKAGE_VERSION}
# 	COMPATIBILITY SameMajorVersion
# )

# # 安装导出目标CMake
# install(EXPORT ${PROJECT_NAME}Targets
# 	FILE ${PROJECT_NAME}Targets.cmake
# 	DESTINATION lib/cmake/${PROJECT_NAME}
# )

# # 生成配置文件LuaConfig.cmake
# configure_package_config_file(
# 	Config.cmake.in ${PROJECT_NAME}Config.cmake
# 	INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
# 	PATH_VARS LUASOCKET_LIBRARIES
# )

# # 安装配置文件到相应目录
# install(FILES 
# 	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
# 	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
# 	DESTINATION lib/cmake/${PROJECT_NAME}
# )
