#pragma once

#include <vector>

namespace ParaEngine
{
	/**
	 * Cross-platform 3D point-cloud nearest-neighbor template matcher for AutoRigger.
	 * Uses an internal, zero-dependency 3D KD-tree (replacing legacy PCL/FLANN).
	 */
	class Matcher
	{
	public:
		struct PointNormal3f
		{
			float x;
			float y;
			float z;
			float nx;
			float ny;
			float nz;
		};

		Matcher();
		~Matcher();

		/**
		 * Select active point cloud slot and clear its previous points:
		 *   0 - target model
		 *   1 - source template model
		 */
		void SetModelFeatureType(unsigned int type);

		/** Append a 3D surface point and normal to the active point cloud slot. */
		void AddFeature(float x, float y, float z, float nx, float ny, float nz);

		/** Optional finalization hook kept for API compatibility with legacy Matcher. */
		void CloseFeature();

		/**
		 * Compute sum of squared nearest-neighbor L2 distances from target (0) to source (1).
		 * Smaller return value indicates higher geometric similarity.
		 */
		int Match();

	private:
		int EvaluateSimilarity() const;
		void ComputeTargetToSourceDistances();

	private:
		std::vector<PointNormal3f> m_PointCloud[2];
		unsigned int m_CurrentModel;
		float m_ijDist;
		float m_jiDist;
	};
}
