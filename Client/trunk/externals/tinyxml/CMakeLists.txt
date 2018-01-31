project(tinyxml)
add_library(tinyxml tinyxml.cpp tinyxml.h tinyxmlerror.cpp tinyxmlparser.cpp)
target_include_directories(tinyxml PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_definitions(tinyxml PUBLIC TIXML_USE_STL)