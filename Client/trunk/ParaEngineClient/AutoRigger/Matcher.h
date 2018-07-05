// ----------------------------------------------------------------------------
// -                       Fast Global Registration                           -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) Intel Corporation 2016
// Qianyi Zhou <Qianyi.Zhou@gmail.com>
// Jaesik Park <syncle@gmail.com>
// Vladlen Koltun <vkoltun@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include <vector>
#include <flann/flann.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace ParaEngine
{
	class Matcher
	{
	public:

		Matcher();

		~Matcher();

		/** 0 - source model
		  * 1 - target model
		  */
		void SetModelFeatureType(unsigned int type);

		void AddFeature(float x, float y, float z, float nx, float ny, float nz);

		void CloseFeature();

		int Match();

	private:
		typedef std::vector<Eigen::Vector3f> Points;
		typedef std::vector<Eigen::VectorXf> Feature;

		void NormalizePoints();
		void AdvancedMatching();
		Eigen::Matrix4f GetTrans();
		double OptimizePairwise(bool decrease_mu_, int numIter_);
		void SearchFLANNTree(flann::Index<flann::L2<float>>* index,
			Eigen::VectorXf& input,
			std::vector<int>& indices,
			std::vector<float>& dists,
			int nn);
	private:
		// containers
		std::vector<Points> pointcloud_;
		std::vector<Feature> features_;
		Eigen::Matrix4f TransOutput_;
		std::vector<std::pair<int, int>> corres_;

		// for normalization
		Points Means;
		float GlobalScale;
		float StartScale;

		unsigned int m_CurrentModel;

	};

}
