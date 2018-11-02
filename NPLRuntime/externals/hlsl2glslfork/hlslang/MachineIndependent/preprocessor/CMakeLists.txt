file(GLOB source_files "${PROJECT_SOURCE_DIR}/hlslang/MachineIndependent/preprocessor/*.cpp")
add_library(preprocessor
    "${source_files}"
)
