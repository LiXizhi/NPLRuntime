#include "ParaEngine.h"
#include "RenderWindowOSX.h"
#include "ParaAppOSX.h"
using namespace ParaEngine;
int main(int argc, const char * argv[]) {
    
    RenderWindowOSX renderWinwow(800,600);
    CParaEngineAppOSX app;
    bool ret = app.InitApp(&renderWinwow, "");
    if(!ret)
    {
        OUTPUT_LOG("Initialize ParaEngineApp failed.");
        return 1;
    }
    while(!renderWinwow.ShouldClose())
    {
        renderWinwow.PollEvents();
        app.DoWork();
    }
    return 0;
}
