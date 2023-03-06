


rem rmdir /Q /S build\emscripten
rem mkdir build\emscripten

emcmake cmake -S NPLRuntime -B build\emscripten -DEMSCRIPTEN=ON -DCURL_ENABLE_SSL=OFF -DBOOST_ROOT="D:/workspace/emscripten/boost_1_81_0"

rem cd build\emscripten
rem emmake make
rem cd ..\..

rem 失效功能
rem openssl 
rem dir_monitor 
rem log.cpp:438


rem boost build\emscripten
rem .\bootstrap.bat vc14
rem .\b2 toolset=emscripten runtime-link=static --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --locale
