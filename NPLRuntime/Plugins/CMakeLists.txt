if(NPLRUNTIME_PHYSICS)
	add_subdirectory(PhysicsBT ${PROJECT_BINARY_DIR}/PhysicsBT)
endif()

set(SQLITE_STATICLIB TRUE)
add_subdirectory(sqlite3)