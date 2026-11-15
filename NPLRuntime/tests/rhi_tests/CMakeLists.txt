# Automated CTest Verification Suite for ParaRHI & Unified Subsystems

if(WIN32)
    find_program(NPL_POWERSHELL_EXE NAMES powershell pwsh REQUIRED)

    add_test(
        NAME rhi_static_macro_audit
        COMMAND "${NPL_POWERSHELL_EXE}" -NoProfile -ExecutionPolicy Bypass
                -File "${CMAKE_CURRENT_SOURCE_DIR}/test_macro_audit.ps1"
                -ParaEngineDir "${PROJECT_SOURCE_DIR}/ParaEngine"
    )
    set_tests_properties(rhi_static_macro_audit PROPERTIES TIMEOUT 60)

    if(TARGET parafxc)
        add_test(
            NAME rhi_shader_verify_all_37
            COMMAND "$<TARGET_FILE:parafxc>" --verify-dir "${PROJECT_SOURCE_DIR}/ParaEngine/shaders/fx"
        )
        set_tests_properties(rhi_shader_verify_all_37 PROPERTIES TIMEOUT 120)
    endif()

    if(NOT NPLRUNTIME_SERVER AND TARGET ParaEngine)
        add_executable(test_world_perf_optimizations
            "${CMAKE_CURRENT_SOURCE_DIR}/test_world_perf_optimizations.cpp"
        )
        target_link_libraries(test_world_perf_optimizations PRIVATE
            ParaEngine
            gdiplus.lib
            comctl32.lib
            Dbghelp.lib
            wsock32.lib
            VFW32.lib
            SHLWAPI.lib
            Psapi.lib
            Ws2_32
            winmm
            Dnsapi
            Secur32
            Crypt32
            Wldap32
        )
        add_test(
            NAME world_perf_optimizations_unit
            COMMAND "$<TARGET_FILE:test_world_perf_optimizations>"
        )
        set_tests_properties(world_perf_optimizations_unit PROPERTIES TIMEOUT 60)
    endif()

    if(NOT NPLRUNTIME_SERVER AND TARGET WindowsApplication)
        add_test(
            NAME rhi_multi_backend_and_subsystems
            COMMAND "${NPL_POWERSHELL_EXE}" -NoProfile -ExecutionPolicy Bypass
                    -File "${CMAKE_CURRENT_SOURCE_DIR}/test_rhi_and_subsystems.ps1"
                    -BinDir "${CMAKE_BINARY_DIR}/bin"
        )
        set_tests_properties(rhi_multi_backend_and_subsystems PROPERTIES TIMEOUT 180)
    endif()
else()
    if(TARGET parafxc)
        add_test(
            NAME rhi_shader_verify_all_37
            COMMAND "$<TARGET_FILE:parafxc>" --verify-dir "${PROJECT_SOURCE_DIR}/ParaEngine/shaders/fx"
        )
        set_tests_properties(rhi_shader_verify_all_37 PROPERTIES TIMEOUT 120)
    endif()

    if(NOT NPLRUNTIME_SERVER AND NOT EMSCRIPTEN AND TARGET ParaEngine)
        add_executable(test_world_perf_optimizations
            "${CMAKE_CURRENT_SOURCE_DIR}/test_world_perf_optimizations.cpp"
        )
        target_link_libraries(test_world_perf_optimizations PRIVATE ParaEngine)
        add_test(
            NAME world_perf_optimizations_unit
            COMMAND "$<TARGET_FILE:test_world_perf_optimizations>"
        )
        set_tests_properties(world_perf_optimizations_unit PROPERTIES TIMEOUT 60)
    endif()
endif()
