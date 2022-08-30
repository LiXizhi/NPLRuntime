include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

ucm_add_dirs(src TO app_src RECURSIVE)

#set(VCPKG_TARGET_TRIPLET x64-windows-static)
#set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

add_executable(ParaCraftSDL2 ${app_src})

target_include_directories(ParaCraftSDL2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ParaCraftSDL2 PUBLIC RenderSystemOpenGL ParaEngine)

find_package(SDL2 CONFIG REQUIRED PATHS ${SDL_ROOT})
target_link_libraries(ParaCraftSDL2 PRIVATE SDL2::SDL2 SDL2::SDL2main)
#if (MSVC) 
	#find_package(SDL2 CONFIG REQUIRED PATHS $ENV{VCPKG_ROOT}/installed/x64-windows-static)
#else()
	#find_package(SDL2 CONFIG REQUIRED PATHS /root/program/SDL-2.0.22)
#endif()
#target_link_libraries(ParaCraftSDL2 PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2-static)


target_compile_definitions(ParaCraftSDL2 PUBLIC PLATFORM_LINUX)
target_compile_definitions(ParaCraftSDL2 PUBLIC LINUX)
target_compile_definitions(ParaCraftSDL2 PUBLIC PLATFORM_SDL2)

ADD_CUSTOM_COMMAND(
	TARGET ParaCraftSDL2
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaCraftSDL2> ${OUTPUT_BIN_DIR}
)
