
add_library(libpng STATIC
png.c
pngerror.c
pngget.c
pngmem.c
pngpread.c
pngread.c
pngrio.c
pngrtran.c
pngrutil.c
pngset.c
pngtrans.c
pngwio.c
pngwrite.c
pngwtran.c
pngwutil.c  
)
target_link_libraries(libpng PUBLIC zlib)
target_include_directories(libpng PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(libpng PRIVATE PNG_ARM_NEON_OPT=0)