file(GLOB source_files "${PROJECT_SOURCE_DIR}/hlslang/GLSLCodeGen/*.cpp" "${PROJECT_SOURCE_DIR}/hlslang/GLSLCodeGen/*.h")
add_library(GLSLCodeGen
    ${source_files}
)
target_include_directories(GLSLCodeGen PUBLIC 
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/"
    "${PROJECT_SOURCE_DIR}/hlslang/include"
    "${PROJECT_SOURCE_DIR}/hlslang/GLSLCodeGen/"
    "${PROJECT_SOURCE_DIR}/include/"

)
target_link_libraries(GLSLCodeGen PUBLIC MachineIndependent)


if(WIN32)
    target_compile_definitions(GLSLCodeGen PUBLIC
        _CRT_SECURE_NO_WARNINGS
    )
endif(WIN32)

