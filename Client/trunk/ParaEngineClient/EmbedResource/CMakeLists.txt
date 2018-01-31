include(${PROJECT_SOURCE_DIR}/cmake/ConfigBoost.cmake)
## embed with relative to project path
function(embed_resources out_var)
    set(result)
    foreach(in_f ${ARGN})
        file(RELATIVE_PATH src_f ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${in_f})
        set(out_f "${PROJECT_BINARY_DIR}/${in_f}.cxx")
        add_custom_command(OUTPUT ${out_f}
                COMMAND embed-resource ${out_f} ${src_f}
                DEPENDS ${in_f}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Building binary file for embedding ${out_f}"
                VERBATIM)
        list(APPEND result "${out_f}")
    endforeach()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

## embed with absolute path
function(embed_resources_abs out_var)
    set(result)
    foreach(in_f ${ARGN})
        set(src_f ${in_f})
		get_filename_component(filename ${src_f} NAME)
        set(out_f "${PROJECT_BINARY_DIR}/resource_embedded/${filename}.cxx")

        add_custom_command(OUTPUT ${out_f}
                COMMAND embed-resource ${out_f} ${src_f}
                DEPENDS ${in_f}
                COMMENT "Building binary file for embedding ${out_f}"
                VERBATIM)
        list(APPEND result "${out_f}")
    endforeach()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

find_package(Boost COMPONENTS filesystem REQUIRED)

include_directories("${Boost_INCLUDE_DIRS}")

add_executable(embed-resource embedresource.cpp embedresource.h README.md)
target_link_libraries(embed-resource ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})


set(EMBED_RES_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
