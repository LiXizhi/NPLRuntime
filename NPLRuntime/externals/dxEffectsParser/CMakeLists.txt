cmake_minimum_required(VERSION 3.1)

project(DxEffectsParser)

option(DXEFFECTSPARSER_GenerateScannerAndParserCode "Generate scanner and parser code " OFF)


set (CMAKE_CXX_STANDARD 11)

if(WIN32)

  if(DXEFFECTSPARSER_GenerateScannerAndParserCode)
    #$(YACC) -o parser.cc --defines=parser.h parser.yy
    add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/parser.cc ${PROJECT_SOURCE_DIR}/src/parser.h
    COMMAND ${PROJECT_SOURCE_DIR}/tools/bison.exe -o parser.cc --defines=parser.h ${PROJECT_SOURCE_DIR}/src/parser.yy
    COMMAND move /y parser.cc ${PROJECT_SOURCE_DIR}/src/parser.cc
    COMMAND move /y parser.h ${PROJECT_SOURCE_DIR}/src/parser.h
    DEPENDS ${PROJECT_SOURCE_DIR}/src/parser.yy
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tools/
    COMMENT "Executing Bison on /parser.yy"
    )
    #$(LEX) -o scanner.cc scanner.ll
    add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/scanner.cc 
    ${PROJECT_SOURCE_DIR}/src/location.hh 
    ${PROJECT_SOURCE_DIR}/src/position.hh
    ${PROJECT_SOURCE_DIR}/src/stack.hh
    COMMAND ${PROJECT_SOURCE_DIR}/tools/flex.exe -o scanner.cc ${PROJECT_SOURCE_DIR}/src/scanner.l
    COMMAND move /y scanner.cc ${PROJECT_SOURCE_DIR}/src/scanner.cc
    COMMAND move /y location.hh ${PROJECT_SOURCE_DIR}/src/location.hh
    COMMAND move /y position.hh ${PROJECT_SOURCE_DIR}/src/position.hh
    COMMAND move /y stack.hh ${PROJECT_SOURCE_DIR}/src/stack.hh
    DEPENDS ${PROJECT_SOURCE_DIR}/src/scanner.l
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tools/
    COMMENT "Executing flex on scanner.l"
    )
  endif()
endif(WIN32)

if(APPLE)

  if(DXEFFECTSPARSER_GenerateScannerAndParserCode)
    #$(YACC) -o parser.cc --defines=parser.h parser.yy
    add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/parser.cc ${PROJECT_SOURCE_DIR}/src/parser.h
      COMMAND bison -o parser.cc --defines=parser.h parser.yy
      DEPENDS ${PROJECT_SOURCE_DIR}/src/parser.yy
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/
      COMMENT "Executing Bison on /parser.yy"
    )
    #$(LEX) -o scanner.cc scanner.ll
    add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/scanner.cc
      ${PROJECT_SOURCE_DIR}/src/location.hh 
      ${PROJECT_SOURCE_DIR}/src/position.hh
      ${PROJECT_SOURCE_DIR}/src/stack.hh
      COMMAND flex -o scanner.cc scanner.l
      DEPENDS ${PROJECT_SOURCE_DIR}/src/scanner.l
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/
      COMMENT "Executing flex on scanner.l"
    )
  endif()

endif(APPLE)

if(DXEFFECTSPARSER_GenerateScannerAndParserCode)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/location.hh PROPERTIES GENERATED 1)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/position.hh PROPERTIES GENERATED 1)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/stack.hh PROPERTIES GENERATED 1)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/scanner.cc PROPERTIES GENERATED 1)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/parser.cc PROPERTIES GENERATED 1)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/parser.h PROPERTIES GENERATED 1)
endif()


add_library(fxParser
${PROJECT_SOURCE_DIR}/src/dxeffects.h
${PROJECT_SOURCE_DIR}/src/parser.cc
${PROJECT_SOURCE_DIR}/src/parser.h
${PROJECT_SOURCE_DIR}/src/scanner.cc
${PROJECT_SOURCE_DIR}/src/scanner.h
${PROJECT_SOURCE_DIR}/src/location.hh
${PROJECT_SOURCE_DIR}/src/position.hh
${PROJECT_SOURCE_DIR}/src/stack.hh
${PROJECT_SOURCE_DIR}/src/driver.h
${PROJECT_SOURCE_DIR}/src/driver.cc
)
target_include_directories(fxParser PUBLIC ${PROJECT_SOURCE_DIR}/src/)

#add_executable(fxParserTest ${PROJECT_SOURCE_DIR}/src/exprtest.cc)

#target_link_libraries(fxParserTest fxParser)

