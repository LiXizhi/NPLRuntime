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

#include "Matcher.h"

#include "pcl/features/feature.h"
#include "pcl/point_cloud.h"
#include "pcl/point_types.h"
#include "pcl/features/fpfh_omp.h"

#define DIV_FACTOR			1.4		// Division factor used for graduated non-convexity
#define USE_ABSOLUTE_SCALE	0		// Measure distance in absolute scale (1) or in scale relative to the diameter of the model (0)
#define MAX_CORR_DIST		0.025	// Maximum correspondence distance (also see comment of USE_ABSOLUTE_SCALE)
#define ITERATION_NUMBER	64		// Maximum number of iteration
#define TUPLE_SCALE			0.95	// Similarity measure used for tuples of feature points.
#define TUPLE_MAX_CNT		1000	// Maximum tuple numbers.

using namespace std;
using namespace Eigen;
using namespace ParaEngine;


// Assume a point cloud with normal is given as
pcl::PointCloud<pcl::PointNormal>::Ptr tarObj(new pcl::PointCloud<pcl::PointNormal>());
pcl::PointCloud<pcl::PointNormal>::Ptr srcObj(new pcl::PointCloud<pcl::PointNormal>());

Matcher::Matcher()
	:m_CurrentModel(-1)
	,GlobalScale(1)
	,StartScale(1)
{
	pointcloud_.push_back(Points());// target model point cloud
	pointcloud_.push_back(Points());// source model point cloud
	features_.push_back(Feature());// target model point cloud
	features_.push_back(Feature());// source model point cloud
}

Matcher::~Matcher()
{

}


int Matcher::Match()
{
	this->NormalizePoints();
	this->AdvancedMatching();
	this->OptimizePairwise(true, ITERATION_NUMBER);
	return corres_.size();
}

void Matcher::SetModelFeatureType(unsigned int type)
{
	m_CurrentModel = type;
	if (m_CurrentModel == 0)tarObj->clear();
	if (m_CurrentModel == 1)srcObj->clear();
	pointcloud_[m_CurrentModel].clear();
	features_[m_CurrentModel].clear();
}

void Matcher::AddFeature(float x, float y, float z, float nx, float ny, float nz)
{
	pcl::PointNormal pt;
	pt.x = x;
	pt.y = y;
	pt.z = z;
	pt.normal_x = nx;
	pt.normal_y = ny;
	pt.normal_z = nz;
	if (m_CurrentModel == 0)tarObj->push_back(pt);
	if (m_CurrentModel == 1)srcObj->push_back(pt);
}

void Matcher::CloseFeature()
{
	pcl::PointCloud<pcl::PointNormal>::Ptr* obj = nullptr;
	pcl::FPFHEstimationOMP<pcl::PointNormal, pcl::PointNormal, pcl::FPFHSignature33> fest;
	pcl::PointCloud<pcl::FPFHSignature33>::Ptr object_features(new pcl::PointCloud<pcl::FPFHSignature33>());
	fest.setRadiusSearch(2.0);
	if (m_CurrentModel == 0) {
		obj = &tarObj;
	}else if (m_CurrentModel == 1) {
		obj = &srcObj;
	}else {
		return;
	}
	fest.setInputCloud(*obj);
	fest.setInputNormals(*obj);
	fest.compute(*object_features);
	int nV = (*obj)->size(), nDim = 33;
	for (int i = 0; i < nV; i++) {
		const pcl::PointNormal &pt = (*obj)->points[i];
		Vector3f pts_v( pt.x, pt.y, pt.z );
		VectorXf feat_v(nDim);
		const pcl::FPFHSignature33 &feature = object_features->points[i];
		memcpy(&feat_v(0), feature.histogram, nDim);
		pointcloud_[m_CurrentModel].push_back(pts_v);
		features_[m_CurrentModel].push_back(feat_v);
	}

	(*obj)->clear();
}

void Matcher::SearchFLANNTree(flann::Index<flann::L2<float>>* index,
							VectorXf& input,
							std::vector<int>& indices,
							std::vector<float>& dists,
							int nn)
{
	int rows_t = 1;
	int dim = input.size();

	std::vector<float> query;
	query.resize(rows_t*dim);
	for (int i = 0; i < dim; i++)
		query[i] = input(i);
	flann::Matrix<float> query_mat(&query[0], rows_t, dim);

	indices.resize(rows_t*nn);
	dists.resize(rows_t*nn);
	flann::Matrix<int> indices_mat(&indices[0], rows_t, nn);
	flann::Matrix<float> dists_mat(&dists[0], rows_t, nn);

	index->knnSearch(query_mat, indices_mat, dists_mat, nn, flann::SearchParams(128));
}

void Matcher::AdvancedMatching()
{
	int fi = 0;
	int fj = 1;

	printf("Advanced matching : [%d - %d]\n", fi, fj);
	bool swapped = false;

	if (pointcloud_[fj].size() > pointcloud_[fi].size())
	{
		int temp = fi;
		fi = fj;
		fj = temp;
		swapped = true;
	}

	int nPti = pointcloud_[fi].size();
	int nPtj = pointcloud_[fj].size();

	///////////////////////////
	/// BUILD FLANNTREE
	///////////////////////////

	// build FLANNTree - fi
	int rows, dim;
	rows = features_[fi].size();
	dim = features_[fi][0].size();

	std::vector<float> dataset_fi(rows * dim);
	flann::Matrix<float> dataset_mat_fi(&dataset_fi[0], rows, dim);

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < dim; j++)
			dataset_fi[i * dim + j] = features_[fi][i][j];

	flann::Index<flann::L2<float>> feature_tree_i(dataset_mat_fi, flann::KDTreeSingleIndexParams(15));
	feature_tree_i.buildIndex();

	// build FLANNTree - fj
	rows = features_[fj].size();
	dim = features_[fj][0].size();

	std::vector<float> dataset_fj(rows * dim);
	flann::Matrix<float> dataset_mat_fj(&dataset_fj[0], rows, dim);

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < dim; j++)
			dataset_fj[i * dim + j] = features_[fj][i][j];

	flann::Index<flann::L2<float>> feature_tree_j(dataset_mat_fj, flann::KDTreeSingleIndexParams(15));
	feature_tree_j.buildIndex();

	bool crosscheck = true;
	bool tuple = true;

	std::vector<int> corres_K, corres_K2;
	std::vector<float> dis;
	std::vector<int> ind;

	std::vector<std::pair<int, int>> corres;
	std::vector<std::pair<int, int>> corres_cross;
	std::vector<std::pair<int, int>> corres_ij;
	std::vector<std::pair<int, int>> corres_ji;

	///////////////////////////
	/// INITIAL MATCHING
	///////////////////////////

	std::vector<int> i_to_j(nPti, -1);
	for (int j = 0; j < nPtj; j++)
	{
		SearchFLANNTree(&feature_tree_i, features_[fj][j], corres_K, dis, 1);
		int i = corres_K[0];
		if (i_to_j[i] == -1)
		{
			SearchFLANNTree(&feature_tree_j, features_[fi][i], corres_K, dis, 1);
			int ij = corres_K[0];
			i_to_j[i] = ij;
		}
		corres_ji.push_back(std::pair<int, int>(i, j));
	}

	for (int i = 0; i < nPti; i++)
	{
		if (i_to_j[i] != -1)
			corres_ij.push_back(std::pair<int, int>(i, i_to_j[i]));
	}

	int ncorres_ij = corres_ij.size();
	int ncorres_ji = corres_ji.size();

	// corres = corres_ij + corres_ji;
	for (int i = 0; i < ncorres_ij; ++i)
		corres.push_back(std::pair<int, int>(corres_ij[i].first, corres_ij[i].second));
	for (int j = 0; j < ncorres_ji; ++j)
		corres.push_back(std::pair<int, int>(corres_ji[j].first, corres_ji[j].second));

	printf("points are remained : %d\n", (int)corres.size());

	///////////////////////////
	/// CROSS CHECK
	/// input : corres_ij, corres_ji
	/// output : corres
	///////////////////////////
	if (crosscheck)
	{
		printf("\t[cross check] ");

		// build data structure for cross check
		corres.clear();
		corres_cross.clear();
		std::vector<std::vector<int>> Mi(nPti);
		std::vector<std::vector<int>> Mj(nPtj);

		int ci, cj;
		for (int i = 0; i < ncorres_ij; ++i)
		{
			ci = corres_ij[i].first;
			cj = corres_ij[i].second;
			Mi[ci].push_back(cj);
		}
		for (int j = 0; j < ncorres_ji; ++j)
		{
			ci = corres_ji[j].first;
			cj = corres_ji[j].second;
			Mj[cj].push_back(ci);
		}

		// cross check
		for (int i = 0; i < nPti; ++i)
		{
			for (int ii = 0; ii < Mi[i].size(); ++ii)
			{
				int j = Mi[i][ii];
				for (int jj = 0; jj < Mj[j].size(); ++jj)
				{
					if (Mj[j][jj] == i)
					{
						corres.push_back(std::pair<int, int>(i, j));
						corres_cross.push_back(std::pair<int, int>(i, j));
					}
				}
			}
		}
		printf("points are remained : %d\n", (int)corres.size());
	}

	///////////////////////////
	/// TUPLE CONSTRAINT
	/// input : corres
	/// output : corres
	///////////////////////////
	if (tuple)
	{
		srand(time(NULL));

		printf("\t[tuple constraint] ");
		int rand0, rand1, rand2;
		int idi0, idi1, idi2;
		int idj0, idj1, idj2;
		float scale = TUPLE_SCALE;
		int ncorr = corres.size();
		int number_of_trial = ncorr * 100;
		std::vector<std::pair<int, int>> corres_tuple;

		int cnt = 0;
		int i;
		for (i = 0; i < number_of_trial; i++)
		{
			rand0 = rand() % ncorr;
			rand1 = rand() % ncorr;
			rand2 = rand() % ncorr;

			idi0 = corres[rand0].first;
			idj0 = corres[rand0].second;
			idi1 = corres[rand1].first;
			idj1 = corres[rand1].second;
			idi2 = corres[rand2].first;
			idj2 = corres[rand2].second;

			// collect 3 points from i-th fragment
			Eigen::Vector3f pti0 = pointcloud_[fi][idi0];
			Eigen::Vector3f pti1 = pointcloud_[fi][idi1];
			Eigen::Vector3f pti2 = pointcloud_[fi][idi2];

			float li0 = (pti0 - pti1).norm();
			float li1 = (pti1 - pti2).norm();
			float li2 = (pti2 - pti0).norm();

			// collect 3 points from j-th fragment
			Eigen::Vector3f ptj0 = pointcloud_[fj][idj0];
			Eigen::Vector3f ptj1 = pointcloud_[fj][idj1];
			Eigen::Vector3f ptj2 = pointcloud_[fj][idj2];

			float lj0 = (ptj0 - ptj1).norm();
			float lj1 = (ptj1 - ptj2).norm();
			float lj2 = (ptj2 - ptj0).norm();

			if ((li0 * scale < lj0) && (lj0 < li0 / scale) &&
				(li1 * scale < lj1) && (lj1 < li1 / scale) &&
				(li2 * scale < lj2) && (lj2 < li2 / scale))
			{
				corres_tuple.push_back(std::pair<int, int>(idi0, idj0));
				corres_tuple.push_back(std::pair<int, int>(idi1, idj1));
				corres_tuple.push_back(std::pair<int, int>(idi2, idj2));
				cnt++;
			}

			if (cnt >= TUPLE_MAX_CNT)
				break;
		}

		printf("%d tuples (%d trial, %d actual) \n", cnt, number_of_trial, i);
		corres.clear();

		for (int i = 0; i < corres_tuple.size(); ++i)
			corres.push_back(std::pair<int, int>(corres_tuple[i].first, corres_tuple[i].second));
	}

	if (swapped)
	{
		std::vector<std::pair<int, int>> temp;
		for (int i = 0; i < corres.size(); i++)
			temp.push_back(std::pair<int, int>(corres[i].second, corres[i].first));
		corres.clear();
		corres = temp;
	}

	printf("\t[final] matches %d \n", (int)corres.size());
	corres_ = corres;
}

// Normalize scale of points.
// X' = (X-\mu)/scale
void Matcher::NormalizePoints()
{
	int num = 2;
	float scale = 0;

	Means.clear();

	for (int i = 0; i < num; ++i)
	{
		float max_scale = 0;

		// compute mean
		Vector3f mean;
		mean.setZero();

		int npti = pointcloud_[i].size();
		for (int ii = 0; ii < npti; ++ii)
		{
			Eigen::Vector3f p(pointcloud_[i][ii](0), pointcloud_[i][ii](1), pointcloud_[i][ii](2));
			mean = mean + p;
		}
		mean = mean / npti;
		Means.push_back(mean);

		printf("normalize points :: mean[%d] = [%f %f %f]\n", i, mean(0), mean(1), mean(2));

		for (int ii = 0; ii < npti; ++ii)
		{
			pointcloud_[i][ii](0) -= mean(0);
			pointcloud_[i][ii](1) -= mean(1);
			pointcloud_[i][ii](2) -= mean(2);
		}

		// compute scale
		for (int ii = 0; ii < npti; ++ii)
		{
			Eigen::Vector3f p(pointcloud_[i][ii](0), pointcloud_[i][ii](1), pointcloud_[i][ii](2));
			float temp = p.norm(); // because we extract mean in the previous stage.
			if (temp > max_scale)
				max_scale = temp;
		}

		if (max_scale > scale)
			scale = max_scale;
	}

	//// mean of the scale variation
	if (USE_ABSOLUTE_SCALE) {
		GlobalScale = 1.0f;
		StartScale = scale;
	} else {
		GlobalScale = scale; // second choice: we keep the maximum scale.
		StartScale = 1.0f;
	}
	printf("normalize points :: global scale : %f\n", GlobalScale);

	for (int i = 0; i < num; ++i)
	{
		int npti = pointcloud_[i].size();
		for (int ii = 0; ii < npti; ++ii)
		{
			pointcloud_[i][ii](0) /= GlobalScale;
			pointcloud_[i][ii](1) /= GlobalScale;
			pointcloud_[i][ii](2) /= GlobalScale;
		}
	}
}

double Matcher::OptimizePairwise(bool decrease_mu_, int numIter_)
{
	printf("Pairwise rigid pose optimization\n");

	double par;
	int numIter = numIter_;
	TransOutput_ = Eigen::Matrix4f::Identity();

	par = StartScale;

	int i = 0;
	int j = 1;

	// make another copy of pointcloud_[j].
	Points pcj_copy;
	int npcj = pointcloud_[j].size();
	pcj_copy.resize(npcj);
	for (int cnt = 0; cnt < npcj; cnt++)
		pcj_copy[cnt] = pointcloud_[j][cnt];

	if (corres_.size() < 10)
		return -1;

	std::vector<double> s(corres_.size(), 1.0);

	Eigen::Matrix4f trans;
	trans.setIdentity();

	for (int itr = 0; itr < numIter; itr++) {

		// graduated non-convexity.
		if (decrease_mu_)
		{
			if (itr % 4 == 0 && par > MAX_CORR_DIST) {
				par /= DIV_FACTOR;
			}
		}

		const int nvariable = 6;	// 3 for rotation and 3 for translation
		Eigen::MatrixXd JTJ(nvariable, nvariable);
		Eigen::MatrixXd JTr(nvariable, 1);
		Eigen::MatrixXd J(nvariable, 1);
		JTJ.setZero();
		JTr.setZero();

		double r;
		double r2 = 0.0;

		for (int c = 0; c < corres_.size(); c++) {
			int ii = corres_[c].first;
			int jj = corres_[c].second;
			Eigen::Vector3f p, q;
			p = pointcloud_[i][ii];
			q = pcj_copy[jj];
			Eigen::Vector3f rpq = p - q;

			int c2 = c;

			float temp = par / (rpq.dot(rpq) + par);
			s[c2] = temp * temp;

			J.setZero();
			J(1) = -q(2);
			J(2) = q(1);
			J(3) = -1;
			r = rpq(0);
			JTJ += J * J.transpose() * s[c2];
			JTr += J * r * s[c2];
			r2 += r * r * s[c2];

			J.setZero();
			J(2) = -q(0);
			J(0) = q(2);
			J(4) = -1;
			r = rpq(1);
			JTJ += J * J.transpose() * s[c2];
			JTr += J * r * s[c2];
			r2 += r * r * s[c2];

			J.setZero();
			J(0) = -q(1);
			J(1) = q(0);
			J(5) = -1;
			r = rpq(2);
			JTJ += J * J.transpose() * s[c2];
			JTr += J * r * s[c2];
			r2 += r * r * s[c2];

			r2 += (par * (1.0 - sqrt(s[c2])) * (1.0 - sqrt(s[c2])));
		}

		Eigen::MatrixXd result(nvariable, 1);
		result = -JTJ.llt().solve(JTr);

		Eigen::Affine3d aff_mat;
		aff_mat.linear() = (Eigen::Matrix3d) Eigen::AngleAxisd(result(2), Eigen::Vector3d::UnitZ())
			* Eigen::AngleAxisd(result(1), Eigen::Vector3d::UnitY())
			* Eigen::AngleAxisd(result(0), Eigen::Vector3d::UnitX());
		aff_mat.translation() = Eigen::Vector3d(result(3), result(4), result(5));

		Eigen::Matrix4f delta = aff_mat.matrix().cast<float>();

		trans = delta * trans;

		// transform point clouds
		Matrix3f R = delta.block<3, 3>(0, 0);
		Vector3f t = delta.block<3, 1>(0, 3);
		for (int cnt = 0; cnt < npcj; cnt++)
			pcj_copy[cnt] = R * pcj_copy[cnt] + t;

	}

	TransOutput_ = trans * TransOutput_;
	return par;
}

Eigen::Matrix4f Matcher::GetTrans()
{
    Eigen::Matrix3f R;
	Eigen::Vector3f t;
	R = TransOutput_.block<3, 3>(0, 0);
	t = TransOutput_.block<3, 1>(0, 3);

	Eigen::Matrix4f transtemp;
	transtemp.fill(0.0f);

	transtemp.block<3, 3>(0, 0) = R;
	transtemp.block<3, 1>(0, 3) = -R*Means[1] + t*GlobalScale + Means[0];
	transtemp(3, 3) = 1;
    
    return transtemp;
}
   