#pragma once
namespace ParaEngine
{
    struct Color4f
    {
        float r;
        float g;
        float b;
        float a;
        Color4f()
        :r(0)
        ,g(0)
        ,b(0)
        ,a(0)
        {
        
        }
        Color4f(float R,float G,float B,float A)
        :r(R)
        ,g(G)
        ,b(B)
        ,a(A)
        {
        
        }
    };
}