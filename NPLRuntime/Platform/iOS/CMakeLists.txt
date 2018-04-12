cmake_minimum_required(VERSION 3.11)


project(GLDemo)

set(SRC 
    src/AppDelegate.h
    src/AppDelegate.m
    src/GLView.h
    src/GLView.m
    src/main.m
    src/ViewController.h
    src/ViewController.m
)

find_library(UIKit UIKit)
find_library(OpenGLES OpenGLES)
find_library(QuartzCore QuartzCore)

add_executable(GLDemo MACOSX_BUNDLE ${SRC})
target_link_libraries(GLDemo PRIVATE ${UIKit} ${OpenGLES} ${QuartzCore})




set_target_properties(GLDemo PROPERTIES 
MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/info.plist
)



