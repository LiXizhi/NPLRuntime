
rem rmdir /Q /S build\win32
rem mkdir build\win32

emcmake cmake -S NPLRuntime -B build\win32 -DEMSCRIPTEN=ON -DCURL_ENABLE_SSL=OFF

rem cd build\win32
rem emmake make
rem cd ..\..

rem 失效功能
rem openssl 
rem dir_monitor 
rem log.cpp:438


rem boost build\win32
rem .\bootstrap.bat vc14
