cmake_minimum_required(VERSION 3.7)

project(Client)


# Ugly warnings
IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
ENDIF()


add_subdirectory(externals)
add_subdirectory(ParaEngineClient)
#add_subdirectory(PhysicsBT)
#add_subdirectory(ParaEngineClientApp)

