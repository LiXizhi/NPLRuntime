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
endif()
