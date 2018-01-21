
if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")
#{
    add_subdirectory(opengl)
#}
elseif(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
#{
    add_subdirectory(d3d9)
#}
endif()


