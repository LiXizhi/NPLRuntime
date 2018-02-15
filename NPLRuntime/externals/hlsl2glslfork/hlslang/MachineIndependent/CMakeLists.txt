add_library(MachineIndependent
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/ConstantFolding.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/HLSL2GLSL.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/InfoSink.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Intermediate.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/intermOut.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/IntermTraverse.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/ParseHelper.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/PoolAlloc.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/RemoveTree.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/SymbolTable.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang_tab.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Initialize.cpp"
    "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/localintermediate.h"
)

target_link_libraries(MachineIndependent PUBLIC preprocessor OSDependent GLSLCodeGen)
target_include_directories(MachineIndependent PUBLIC
"${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/"
"${PROJECT_SOURCE_DIR}/hlslang/"
"${PROJECT_SOURCE_DIR}/hlslang/include/"
"${PROJECT_SOURCE_DIR}/include/"
)

if(WIN32)
    target_compile_definitions(MachineIndependent PUBLIC
        _CRT_SECURE_NO_WARNINGS
    )
endif(WIN32)

option(HLSL2GLSLFORK_GenrateScannerAndParserCode " Generate scanner and parser source code " OFF)

if(WIN32)
    target_include_directories(MachineIndependent PUBLIC 
    "${PROJECT_SOURCE_DIR}/hlslang/OSDependent/Windows/"
    )
    if(HLSL2GLSLFORK_GenrateScannerAndParserCode)
        add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang.cpp
        COMMAND ${PROJECT_SOURCE_DIR}/tools/flex.exe ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang.l
        MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang.l
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/
        COMMENT "Executing flex on hlslang.l"
        )

        add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang_tab.cpp
        COMMAND bison.exe -d -t -v ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang.y
        COMMAND move /y hlslang.tab.h ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang_tab.h
        COMMAND move /y hlslang.tab.c ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang_tab.cpp
        MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang.y
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tools/bin/
        COMMENT "Executing Bison on hlslang.y"
        )
    endif()

elseif(APPLE)
    target_include_directories(MachineIndependent PUBLIC 
    "${PROJECT_SOURCE_DIR}/hlslang/OSDependent/Mac/"
    )

    if(HLSL2GLSLFORK_GenrateScannerAndParserCode)
        add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang.cpp
        COMMAND flex hlslang.l
        MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang.l
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/
        COMMENT "Executing flex on hlslang.l"
        )

        add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang_tab.cpp
        COMMAND bison -d -t -v hlslang.y
        COMMAND mv -f hlslang.tab.c Gen_hlslang_tab.cpp
        COMMAND mv -f hlslang.tab.h hlslang_tab.h
        MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang.y
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/
        COMMENT "Executing Bison on hlslang.y"
        )
    endif()

elseif(UNIX)
    target_include_directories(MachineIndependent PUBLIC 
    "${PROJECT_SOURCE_DIR}/hlslang/OSDependent/Linux/"
    )

    if(HLSL2GLSLFORK_GenrateScannerAndParserCode)   
        add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang.cpp
        COMMAND flex hlslang.l
        MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang.l
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/
        COMMENT "Executing flex on hlslang.l"
        )

        add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang_tab.cpp
        COMMAND bison -d -t -v hlslang.y
        COMMAND mv -f hlslang.tab.c Gen_hlslang_tab.cpp
        COMMAND mv -f hlslang.tab.h hlslang_tab.h
        MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang.y
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/
        COMMENT "Executing Bison on hlslang.y"
        )
    endif()

endif(WIN32)


if(HLSL2GLSLFORK_GenrateScannerAndParserCode)
    set_source_files_properties(${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang.cpp PROPERTIES GENERATED 1)
    set_source_files_properties(${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/Gen_hlslang_tab.cpp PROPERTIES GENERATED 1)
endif()

add_subdirectory(preprocessor)