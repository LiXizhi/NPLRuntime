#include "RenderWindowOSX.h"
#include "ParaAppOSX.h"
using namespace ParaEngine;
int main(int argc, const char * argv[]) {
    
    RenderWindowOSX renderWinwow(800,600);
    CParaEngineAppOSX app;
    app.InitApp(&renderWinwow, "");
    while(!renderWinwow.ShouldClose())
    {
        renderWinwow.PollEvents();
        app.DoWork();
    }
    return 0;
}
