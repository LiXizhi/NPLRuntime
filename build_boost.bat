
if NOT "%GITHUB_WORKFLOW%" == "" (
    if EXIST "boost_1_78_0.7z" (
        "C:\Program Files\7-Zip\7z.exe" x boost_1_78_0.7z 
        move boost_1_78_0 boost
        cd boost
        bootstrap.bat
        REM b2 address-model=32 link=static runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        REM b2 address-model=64 link=static runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        b2 address-model=32 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        b2 address-model=64 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
    )
) else (
    if EXIST "boost_1_78_0.7z" (
        .\bin\7z.exe x boost_1_78_0.7z 
        move boost_1_78_0 boost
        cd boost
        bootstrap.bat
        REM b2 address-model=32 link=static runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        REM b2 address-model=64 link=static runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        b2 address-model=32 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage
        b2 address-model=64 runtime-link=static threading=multi variant=release --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-serialization --with-iostreams --with-regex stage

)
