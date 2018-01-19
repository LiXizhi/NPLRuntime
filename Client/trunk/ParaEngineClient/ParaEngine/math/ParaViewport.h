#pragma once
namespace ParaEngine
{

	class ParaViewport
	{
	public:

		DWORD       X;
		DWORD       Y;            /* Viewport Top left */
		DWORD       Width;
		DWORD       Height;       /* Viewport Dimensions */
		float       MinZ;         /* Min/max of clip Volume */
		float       MaxZ;
	public:
		ParaViewport() :X(0), Y(0), Width(0), Height(0), MinZ(0.f), MaxZ(0.f){};

		ParaViewport(const D3DVIEWPORT9& v){
			*this = reinterpret_cast<const ParaViewport&>(v);
		};

		inline operator D3DVIEWPORT9&() {
			return reinterpret_cast<D3DVIEWPORT9&>(*this);
		}
		inline operator const D3DVIEWPORT9&() const{
			return reinterpret_cast<const D3DVIEWPORT9&>(*this);
		}

		bool operator == (const ParaViewport& r) const {
			return X == r.X && Y == r.Y && Width == r.Width && Height == r.Height;
		}

		bool operator != (const ParaViewport& r) const {
			return X != r.X || Y != r.Y || Width != r.Width || Height != r.Height;
		}
	};

}