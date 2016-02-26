#pragma once

#include "Terrain.h"

namespace ParaTerrain
{
	/// \brief A "brush" that can be used to paint detail texture "splats" on a Terrain object's surface.

	/// This one simple example of how a brush can be implemented. This brush paints in a circular pattern, with
	/// opaque texture at the center of the splat and decreasing opaqueness as you move along the radius to the outside
	/// of the circle. You may derive from this class to create brushed of other shapes and behaviors for detail painting.
	class Brush
	{
	      public:
		Brush(int width);
		virtual ~Brush();
		void SetWidth(int width);
		virtual void SetErase(bool bErase);
		virtual void SetIntensity(float intensity);
		virtual void SetMaxIntensity(float maxIntensity);
		virtual void Paint(Terrain * pTerrain, int detailTextureIndex, float x, float y);
	      private:
		void Paint(Terrain * pTerrain, int detailTextureIndex, int cellX, int cellY, int x, int y);
		void BuildBuffer();
		unsigned char *m_pBuffer;
		int m_Width;
		float m_Intensity;
		float m_MaxIntensity;
		bool m_bErase;
	};
}

