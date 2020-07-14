if(NPLRUNTIME_PHYSICS)
	add_subdirectory(PhysicsBT ${PROJECT_BINARY_DIR}/PhysicsBT)
endif()

if (NPLRUNTIME_SERVER)
else()
	if(NPLRUNTIME_AUDIO)
		add_subdirectory(cAudioEngine ${PROJECT_BINARY_DIR}/cAudioEngine)
	endif()
endif()

add_subdirectory(sqlite3)

if (NPLRUNTIME_NPLMono2)
	add_subdirectory(NPLMono/NPLMono2)
endif()

if (NPLRUNTIME_OCE)
	add_subdirectory(NplOce)
endif()