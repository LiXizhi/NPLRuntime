#include "RenderWindowOSX.h"
using namespace ParaEngine;
int main(int argc, const char * argv[]) {
    
    RenderWindowOSX renderWinwow(800,600);
    while(!renderWinwow.ShouldClose())
    {
        renderWinwow.PollEvents();
    }
    return 0;
}
