/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef PINOCCHIOAPI_H
#define PINOCCHIOAPI_H

#include "mesh.h"
#include "quaddisttree.h"
#include "attachment.h"

namespace Pinocchio
{

	struct PinocchioOutput
	{
		PinocchioOutput() : attachment(NULL) {}

		vector<PVector3> embedding;
		Attachment *attachment; //user responsible for deletion
	};

	//calls the other functions and does the whole rigging process
	//see the implementation of this function to find out how to use the individual functions
	PinocchioOutput PINOCCHIO_API AutoRig(const Skeleton &given, const Mesh &m);

	//============================================individual steps=====================================

	//fits mesh inside unit cube, makes sure there's exactly one connected component
	Mesh PINOCCHIO_API PrepareMesh(const Mesh &m);


	typedef DRootNode<DistData<3>, 3, ArrayIndexer> TreeType; //our distance field octree type
	static const double defaultTreeTol = 0.01;// 0.003

	//constructs a distance field on an octree--user responsible for deleting output
	TreeType PINOCCHIO_API *ConstructDistanceField(const Mesh &m, double tol = defaultTreeTol);

	struct PSphere {
		PSphere() : radius(0.) {}
		PSphere(const PVector3 &inC, double inR) : center(inC), radius(inR) {}

		PVector3 center;
		double radius;
	};

	//samples the distance field to find spheres on the medial surface
	//output is sorted by radius in decreasing order
	vector<PSphere> PINOCCHIO_API SampleMedialSurface(TreeType *distanceField, double tol = defaultTreeTol);

	//takes sorted medial surface samples and sparsifies the vector
	vector<PSphere> PINOCCHIO_API PackSpheres(const vector<PSphere> &samples, int maxSpheres = 1000);

	//constructs graph on packed sphere centers
	PtGraph PINOCCHIO_API ConnectSamples(TreeType *distanceField, const vector<PSphere> &spheres);

	//finds which joints can be embedded into which sphere centers
	vector<vector<int> > PINOCCHIO_API ComputePossibilities(const PtGraph &graph, const vector<PSphere> &spheres,
		const Skeleton &skeleton);

	//finds discrete embedding
	vector<int> PINOCCHIO_API DiscreteEmbed(const PtGraph &graph, const vector<PSphere> &spheres,
		const Skeleton &skeleton, const vector<vector<int> > &possibilities);

	//reinserts joints for unreduced skeleton
	vector<PVector3> PINOCCHIO_API SplitPaths(const vector<int> &discreteEmbedding, const PtGraph &graph,
		const Skeleton &skeleton);

	//refines embedding
	vector<PVector3> PINOCCHIO_API RefineEmbedding(TreeType *distanceField, const vector<PVector3> &medialSurface,
		const vector<PVector3> &initialEmbedding, const Skeleton &skeleton);

	//to compute the attachment, create a new Attachment object

}// end namespace Pinocchio

#endif //PINOCCHIOAPI_H
