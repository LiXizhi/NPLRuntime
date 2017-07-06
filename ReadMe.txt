Author(s): Cellfy
Date Created:      Mar 14, 2016
Data Last Updated: July 5, 2017

------------
Preparation:
    - Install VS2017 Community.
    - Download and Install CMake GUI 3.5 or later.
    - Install DXSDK_Jun10.exe (the latest one, from then on, dxsdk is included in Windows SDK)
    - Clone http://<your_name>@10.0.1.201/FrontEnd/NPLRuntime_truck_star.git into your local computer ( your_local_git_path/NPLRuntime_truck_star ).
    - Checkout branch: origin/truck_star_develop ( which means creating a local branch "truck_star_develop that tracks remote branch "origin/truck_star_develop" )
    - Download boost_1_64_0.rar from ftp://10.0.1.201/NPL_SDK/DEVKIT/boost_1_64_0.rar
    - Extract boost_1_64_0.rar into your_local_git_path/NPLRuntime_truck_star/Server/trunk/ and rename it to "boost_1_61_0", so it will be like this: your_local_git_path/NPLRuntime_truck_star/Server/trunk/boost_1_61_0/...

------------
Build:
    - Run Cmake, set
            "Where is the source code" to:    "your_local_git_path/NPLRuntime_truck_star"
            "where to build the binaries" to: "your_vs_project_path_for_nplruntime" (which basically means whatever-folder-you-like)
    - Click "Configure", Select "VisualStudio 2017" Toolset.
    - When configure finished, there will be an option "PARAENGINE_CLIENT_DLL" with a default "off" value. Tick it "on".
    - Click "Configure" again, there should be no errors (only warnings), ignore them.
    - Click "Generate".
    - VS project and solutions should be successfully generated in "your_vs_project_path_for_nplruntime".
    - Open "your_vs_project_path_for_nplruntime/ParaEngine.sln" with VS2017
    - Choose Debug or Release, build solution.
    - Check your_vs_project_path_for_nplruntime/Client/trunk/ParaEngineClient/Debug (or Release), PaperStar[_d].dll should be the thing you want.
    - Done.
    
------------
How to run and test:
- Copy the newly generated "PaperStar.dll" and "PaperStar.exe" into TruckStarClient/ClientMain/redist/
- Run YouCraft_32bits.bat

------------
Notes:
- treat "truck_star_develop" branch as the real master, and DO NOT mess around with the so-named "master" branch. Only commit to "truck_star_develop" or other branches created by yourself.
- Inform me (Cellfy) if you have features done and want it to be integrated into official NPLRuntime.
- Remember: DO NOT Edit source files within Visual Studio unless you're absolutely sure the line endings is set to Unix Convention (LF), or it may cause problem building Linux Version.
            Since there's no convenient way to do this in VS itself, maybe you will need 3rd party plugin such as Strip'em.
