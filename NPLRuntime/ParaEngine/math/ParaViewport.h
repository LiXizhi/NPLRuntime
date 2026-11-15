#pragma once
#include <cstdint>
namespace ParaEngine
{

	class ParaViewport
	{
	public:

		uint32_t       X;
		uint32_t       Y;            /* Viewport Top left */
		uint32_t       Width;
		uint32_t       Height;       /* Viewport Dimensions */
		float       MinZ;         /* Min/max of clip Volume */
		float       MaxZ;
	public:
		ParaViewport() :X(0), Y(0), Width(0), Height(0), MinZ(0.f), MaxZ(1.f){};
		ParaViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height, float minZ = 0.f, float maxZ = 1.f)
			: X(x), Y(y), Width(width), Height(height), MinZ(minZ), MaxZ(maxZ) {}

		//ParaViewport(const D3DVIEWPORT9& v){
		//	*this = reinterpret_cast<const ParaViewport&>(v);
		//};

		//inline operator D3DVIEWPORT9&() {
		//	return reinterpret_cast<D3DVIEWPORT9&>(*this);
		//}
		//inline operator const D3DVIEWPORT9&() const{
		//	return reinterpret_cast<const D3DVIEWPORT9&>(*this);
		//}

		bool operator == (const ParaViewport& r) const {
			return X == r.X && Y == r.Y && Width == r.Width && Height == r.Height;
		}

		bool operator != (const ParaViewport& r) const {
			return X != r.X || Y != r.Y || Width != r.Width || Height != r.Height;
		}
	};

}
