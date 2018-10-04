#pragma once
namespace ParaEngine
{
    struct Rect
    {
        int32_t x;
		int32_t y;
		int32_t z;
		int32_t w;
        Rect()
        :x(0),y(0),z(0),w(0)
        {

        }

		Rect(int32_t _x,int32_t _y,int32_t _z,int32_t _w)
			:x(_x), y(_y), z(_z), w(_w)
		{

		}
    };
}