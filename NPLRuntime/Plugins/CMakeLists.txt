if(NPLRUNTIME_PHYSICS)
	add_subdirectory(PhysicsBT ${PROJECT_BINARY_DIR}/PhysicsBT)
endif()

if(NPLRUNTIME_AUDIO)
	add_subdirectory(cAudioEngine ${PROJECT_BINARY_DIR}/cAudioEngine)
endif()


add_subdirectory(sqlite3)