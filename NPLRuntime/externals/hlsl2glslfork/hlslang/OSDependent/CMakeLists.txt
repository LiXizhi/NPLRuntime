if(WIN32)
add_library(OSDependent 
    "${PROJECT_SOURCE_DIR}/hlslang/OSDependent/Windows/ossource.cpp"
)
elseif(APPLE)
add_library(OSDependent 
    "${PROJECT_SOURCE_DIR}/hlslang/OSDependent/Mac/ossource.cpp"
)
elseif(UNIX)
add_library(OSDependent 
    "${PROJECT_SOURCE_DIR}/hlslang/OSDependent/Linux/ossource.cpp"
)
endif(WIN32)

target_include_directories(OSDependent PUBLIC ${PROJECT_SOURCE_DIR}/hlslang)