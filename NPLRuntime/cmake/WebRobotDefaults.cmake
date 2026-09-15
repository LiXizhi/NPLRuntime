# Single-thread Web clients must be able to open robot worlds without a separate
# launcher. FORCE also upgrades caches created when this option defaulted to OFF.
if(EMSCRIPTEN AND EMSCRIPTEN_SINGLE_THREAD)
    set(NPLRUNTIME_WEB_ROBOT ON CACHE BOOL "Build synchronous MuJoCo and ONNX robot APIs for Web" FORCE)
else()
    option(NPLRUNTIME_WEB_ROBOT "Build synchronous MuJoCo and ONNX robot APIs for Web" OFF)
endif()
