include_directories(./)
include_directories(./../../)
add_library(android_jniiosystem STATIC AndroidJNIIOSystem.cpp)
TARGET_LINK_LIBRARIES(android_jniiosystem android log)
