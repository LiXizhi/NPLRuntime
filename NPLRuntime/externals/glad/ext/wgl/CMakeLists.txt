include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
add_library(glad_ext_wgl STATIC src/glad_wgl.c)
target_include_directories(glad_ext_wgl PUBLIC include)
target_link_libraries(glad_ext_wgl PUBLIC glad_core)


if(MSVC)
    ucm_set_runtime(DYNAMIC)
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
endif()