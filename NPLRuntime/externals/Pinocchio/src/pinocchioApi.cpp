
#include "pinocchioApi.h"
#include "debugging.h"
#include <fstream>

using namespace Pinocchio;

ostream *Debugging::outStream = new ofstream();

PinocchioOutput Pinocchio::AutoRig(const Skeleton &given, const Mesh &m)
{
    int i;
    PinocchioOutput out;

    Mesh newMesh = PrepareMesh(m);

    if(newMesh.m_Vertices.size() == 0)
        return out;

    TreeType *distanceField = ConstructDistanceField(newMesh);

    //discretization
    vector<PSphere> medialSurface = SampleMedialSurface(distanceField);

    vector<PSphere> spheres = PackSpheres(medialSurface);

    PtGraph graph = ConnectSamples(distanceField, spheres);

    //discrete embedding
    vector<vector<int> > possibilities = ComputePossibilities(graph, spheres, given);

    //constraints can be set by respecifying possibilities for skeleton joints:
    //to constrain joint i to sphere j, use: possiblities[i] = vector<int>(1, j);

    vector<int> embeddingIndices = DiscreteEmbed(graph, spheres, given, possibilities);

    if(embeddingIndices.size() == 0) { //failure
        delete distanceField;
        return out;
    }

    vector<PVector3> discreteEmbedding = SplitPaths(embeddingIndices, graph, given);

    //continuous refinement
    vector<PVector3> medialCenters(medialSurface.size());
    for(i = 0; i < (int)medialSurface.size(); ++i)
        medialCenters[i] = medialSurface[i].center;

    out.embedding = RefineEmbedding(distanceField, medialCenters, discreteEmbedding, given);


    //attachment
    VisTester<TreeType> *tester = new VisTester<TreeType>(distanceField);
    out.attachment = new Attachment(newMesh, given, out.embedding, tester);

    //cleanup
   // delete tester;
    delete distanceField;

    return out;
}

