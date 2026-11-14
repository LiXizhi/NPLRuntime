#include "ParaEngine.h"
#include "Matcher.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace ParaEngine
{
	namespace
	{
		/**
		 * Lightweight, self-contained 3D KD-tree for exact squared-L2 nearest-neighbor search.
		 * Eliminates external PCL/FLANN/Boost.MPI dependencies across Desktop, Mobile, and WASM.
		 */
		class KDTree3D
		{
		public:
			struct Node
			{
				int pointIdx = -1;
				int left = -1;
				int right = -1;
				int axis = 0;
			};

			explicit KDTree3D(const std::vector<Matcher::PointNormal3f>& pts)
				: m_pts(pts)
			{
				if (!m_pts.empty())
				{
					std::vector<int> indices(m_pts.size());
					std::iota(indices.begin(), indices.end(), 0);
					m_nodes.reserve(m_pts.size());
					m_root = BuildRecursive(indices.data(), static_cast<int>(indices.size()), 0);
				}
			}

			float NearestSquaredDist(float qx, float qy, float qz) const
			{
				if (m_root < 0)
					return 0.0f;
				float bestDistSq = (std::numeric_limits<float>::max)();
				const float q[3] = { qx, qy, qz };
				SearchRecursive(m_root, q, bestDistSq);
				return bestDistSq;
			}

		private:
			static inline float GetCoord(const Matcher::PointNormal3f& p, int axis)
			{
				return (axis == 0) ? p.x : ((axis == 1) ? p.y : p.z);
			}

			int BuildRecursive(int* indices, int count, int depth)
			{
				if (count <= 0)
					return -1;

				const int axis = depth % 3;
				const int mid = count / 2;
				std::nth_element(
					indices,
					indices + mid,
					indices + count,
					[this, axis](int a, int b) {
						return GetCoord(m_pts[a], axis) < GetCoord(m_pts[b], axis);
					});

				const int nodeIdx = static_cast<int>(m_nodes.size());
				m_nodes.push_back(Node{ indices[mid], -1, -1, axis });

				const int leftChild = BuildRecursive(indices, mid, depth + 1);
				const int rightChild = BuildRecursive(indices + mid + 1, count - mid - 1, depth + 1);
				m_nodes[nodeIdx].left = leftChild;
				m_nodes[nodeIdx].right = rightChild;
				return nodeIdx;
			}

			void SearchRecursive(int nodeIdx, const float q[3], float& bestDistSq) const
			{
				if (nodeIdx < 0)
					return;

				const Node& node = m_nodes[nodeIdx];
				const Matcher::PointNormal3f& p = m_pts[node.pointIdx];
				const float dx = q[0] - p.x;
				const float dy = q[1] - p.y;
				const float dz = q[2] - p.z;
				const float distSq = dx * dx + dy * dy + dz * dz;
				if (distSq < bestDistSq)
					bestDistSq = distSq;

				const float diff = q[node.axis] - GetCoord(p, node.axis);
				const int nearChild = (diff < 0.0f) ? node.left : node.right;
				const int farChild = (diff < 0.0f) ? node.right : node.left;

				SearchRecursive(nearChild, q, bestDistSq);
				if (diff * diff < bestDistSq)
				{
					SearchRecursive(farChild, q, bestDistSq);
				}
			}

		private:
			const std::vector<Matcher::PointNormal3f>& m_pts;
			std::vector<Node> m_nodes;
			int m_root = -1;
		};
	}

	Matcher::Matcher()
		: m_CurrentModel(0)
		, m_ijDist(0.0f)
		, m_jiDist(0.0f)
	{
	}

	Matcher::~Matcher() = default;

	void Matcher::SetModelFeatureType(unsigned int type)
	{
		if (type < 2)
		{
			m_CurrentModel = type;
			m_PointCloud[m_CurrentModel].clear();
		}
	}

	void Matcher::AddFeature(float x, float y, float z, float nx, float ny, float nz)
	{
		if (m_CurrentModel < 2)
		{
			m_PointCloud[m_CurrentModel].push_back(PointNormal3f{ x, y, z, nx, ny, nz });
		}
	}

	void Matcher::CloseFeature()
	{
		// Point positions and normals are stored directly in m_PointCloud during AddFeature.
	}

	int Matcher::Match()
	{
		ComputeTargetToSourceDistances();
		return EvaluateSimilarity();
	}

	int Matcher::EvaluateSimilarity() const
	{
		return static_cast<int>(m_ijDist);
	}

	void Matcher::ComputeTargetToSourceDistances()
	{
		m_ijDist = 0.0f;
		if (m_PointCloud[0].empty() || m_PointCloud[1].empty())
		{
			m_ijDist = static_cast<float>((std::numeric_limits<int>::max)());
			return;
		}

		KDTree3D sourceTree(m_PointCloud[1]);
		double sumSq = 0.0;
		for (const auto& pt : m_PointCloud[0])
		{
			sumSq += static_cast<double>(sourceTree.NearestSquaredDist(pt.x, pt.y, pt.z));
		}

		const double maxInt = static_cast<double>((std::numeric_limits<int>::max)() - 1);
		m_ijDist = static_cast<float>((std::min)(sumSq, maxInt));
	}
}
