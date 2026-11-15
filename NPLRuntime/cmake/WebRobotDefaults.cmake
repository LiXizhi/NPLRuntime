# Single-thread Web clients must be able to open robot worlds without a separate
# launcher. Default ON for single-thread Web while allowing explicit -DNPLRUNTIME_WEB_ROBOT=OFF.
if(EMSCRIPTEN AND EMSCRIPTEN_SINGLE_THREAD)
    option(NPLRUNTIME_WEB_ROBOT "Build synchronous MuJoCo and ONNX robot APIs for Web" ON)
else()
    option(NPLRUNTIME_WEB_ROBOT "Build synchronous MuJoCo and ONNX robot APIs for Web" OFF)
endif()
