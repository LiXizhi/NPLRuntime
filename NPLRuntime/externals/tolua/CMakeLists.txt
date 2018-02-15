set(TOLUA_SRC
  tolua_event.c
  tolua_is.c
  tolua_map.c
  tolua_push.c
  tolua_to.c
)

add_library(tolua STATIC
  ${TOLUA_SRC}
)
target_include_directories(tolua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tolua PUBLIC lua)