#include "ParaEngine.h"
#include "RenderWindowOSX.h"
#include "ParaAppOSX.h"
using namespace ParaEngine;
int main(int argc, const char * argv[]) {
    

    RenderWindowOSX renderWindow(1280,720);
    CParaEngineAppOSX app;
    bool ret = app.InitApp(&renderWindow, "");
    if(!ret)
    {
        OUTPUT_LOG("Initialize ParaEngineApp failed.");
        return 1;
    }
    
    return app.Run(0);
}
