# Automated CTest Verification Suite for ParaRHI & Unified Subsystems

if(NOT EMSCRIPTEN)
    add_executable(test_texture_compression
        test_texture_compression.cpp
        "${PROJECT_SOURCE_DIR}/RenderSystem/rhi_core/RHISoftwareS3TC.cpp")
    target_include_directories(test_texture_compression PRIVATE
        "${PROJECT_SOURCE_DIR}/ParaEngine"
        "${PROJECT_SOURCE_DIR}/RenderSystem/rhi_core")
    add_test(NAME texture_compression_unit COMMAND test_texture_compression)
endif()

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
        # Compiling the full shader set is slower on shared Windows CI workers.
        set_tests_properties(rhi_shader_verify_all_37 PROPERTIES TIMEOUT 600)
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

if(TARGET test_world_perf_optimizations)
    target_sources(test_world_perf_optimizations PRIVATE headless_platform_stubs.cpp)
    add_executable(test_texture_upload test_texture_upload.cpp headless_platform_stubs.cpp)
    get_target_property(texture_test_libraries test_world_perf_optimizations LINK_LIBRARIES)
    target_link_libraries(test_texture_upload PRIVATE ${texture_test_libraries})
    add_test(NAME texture_upload_unit COMMAND test_texture_upload)
    add_executable(ParticleAndParaxTest "${PROJECT_SOURCE_DIR}/tests/ParticleAndParaxTest.cpp" headless_platform_stubs.cpp)
    target_link_libraries(ParticleAndParaxTest PRIVATE ${texture_test_libraries})
    add_test(NAME particle_and_parax_unit COMMAND ParticleAndParaxTest)
    if(TARGET PhysicsBT)
        if(PHYSICS_STATICLIB)
            add_executable(PhysicsContraptionTest
                "${PROJECT_SOURCE_DIR}/tests/PhysicsContraptionTest.cpp"
                headless_platform_stubs.cpp)
            target_link_libraries(PhysicsContraptionTest PRIVATE
                ${texture_test_libraries}
                PhysicsBT BulletDynamics BulletCollision LinearMath BulletSoftBody)
        else()
            add_executable(PhysicsContraptionTest
                "${PROJECT_SOURCE_DIR}/tests/PhysicsContraptionTest.cpp"
                "${PROJECT_SOURCE_DIR}/Plugins/PhysicsBT/ParaPhysicsWorld.cpp"
                "${PROJECT_SOURCE_DIR}/Plugins/PhysicsBT/PhysicsDebugDraw.cpp"
                headless_platform_stubs.cpp)
            target_link_libraries(PhysicsContraptionTest PRIVATE
                ${texture_test_libraries}
                BulletDynamics BulletCollision LinearMath BulletSoftBody)
        endif()
        target_include_directories(PhysicsContraptionTest PRIVATE
            "${PROJECT_SOURCE_DIR}/ParaEngine"
            "${PROJECT_SOURCE_DIR}/ParaEngine/Core"
            "${PROJECT_SOURCE_DIR}/ParaEngine/util"
            "${PROJECT_SOURCE_DIR}/ParaEngine/Framework/Interface"
            "${PROJECT_SOURCE_DIR}/Plugins/PhysicsBT"
            "${BULLET_PHYSICS_SOURCE_DIR}/src")
        add_test(NAME physics_contraption_unit COMMAND PhysicsContraptionTest)
    endif()
    if(WIN32 AND TARGET RenderSystemD3D12 AND BLOCK_SKY_DXC)
        add_executable(test_block_sky_dxr test_block_sky_dxr.cpp headless_platform_stubs.cpp)
        target_include_directories(test_block_sky_dxr PRIVATE "${PROJECT_SOURCE_DIR}/RenderSystem")
        target_link_libraries(test_block_sky_dxr PRIVATE RenderSystemD3D12 ${texture_test_libraries})
        add_test(NAME block_sky_dxr_gpu COMMAND test_block_sky_dxr)
        set_tests_properties(block_sky_dxr_gpu PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
    endif()
elseif(NPLRUNTIME_SERVER AND NOT EMSCRIPTEN AND TARGET ParaEngine)
    add_executable(ParticleAndParaxTest "${PROJECT_SOURCE_DIR}/tests/ParticleAndParaxTest.cpp" headless_platform_stubs.cpp)
    target_link_libraries(ParticleAndParaxTest PRIVATE RenderSystemRHICore RenderSystemNull ParaEngine)
    if(MAC_SERVER)
        target_link_libraries(ParticleAndParaxTest PRIVATE "-framework CoreFoundation" "-framework CoreServices")
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(ParticleAndParaxTest PRIVATE dl)
    endif()
    add_test(NAME particle_and_parax_unit COMMAND ParticleAndParaxTest)
endif()
