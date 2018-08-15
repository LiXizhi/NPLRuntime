cmake_minimum_required(VERSION 3.10)
set (CMAKE_CXX_STANDARD 11)
project(hlsl2glslfork)

add_executable(FxConverter src/main.cpp)
target_link_libraries(FxConverter PRIVATE DxEffectsParser hlsl2glslfork)