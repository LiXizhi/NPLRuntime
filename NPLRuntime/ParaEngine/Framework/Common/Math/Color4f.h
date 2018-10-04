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

		// ARGB 
		Color4f(uint32_t color)
		{

#ifdef PLATFORM_LITTLE_ENDIAN
			uint8_t ia = (color >> 24);
			uint8_t ir = (color >> 16) & 0x000000ff;
			uint8_t ig = (color >> 8) & 0x000000ff;
			uint8_t ib = color & 0x000000ff;
#else
			uint8_t ib = (color >> 24);
			uint8_t ig = (color >> 16) & 0x000000ff;
			uint8_t ir = (color >> 8) & 0x000000ff;
			uint8_t ia = color & 0x000000ff;
#endif // PLATFORM_LITTLE_ENDIAN
			r = (float)ir / 255.0f;
			g = (float)ig / 255.0f;
			b = (float)ib / 255.0f;
			a = (float)ia / 255.0f;
		}

		uint32_t GetDWColor() const
		{
#ifdef PLATFORM_LITTLE_ENDIAN
			uint8_t ia = (uint8_t)(a * 255.0f);
			uint8_t ir = (uint8_t)(r * 255.0f);
			uint8_t ig = (uint8_t)(g * 255.0f);
			uint8_t ib = (uint8_t)(b * 255.0f);
#else
			uint8_t ia = (uint8_t)(b * 255.0f);
			uint8_t ir = (uint8_t)(g * 255.0f);
			uint8_t ig = (uint8_t)(r * 255.0f);
			uint8_t ib = (uint8_t)(a * 255.0f);
#endif
			uint32_t color = (ia << 24) | (ir << 16) | (ig << 8) | ib;
			return color;
		}
    };
}