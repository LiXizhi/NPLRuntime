/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef MESH_H
#define MESH_H

#include <vector>

#include "pvector.h"
#include "prect.h"

struct MeshVertex
{
    MeshVertex() : edge(-1) {}
    
    PVector3 pos;
    PVector3 normal;
    int edge; //an edge such that edge->prev->vertex is this
};

struct MeshEdge
{
    MeshEdge() : vertex(-1), prev(-1), twin(-1) {}
    
    int vertex; //the vertex the edge points to--the start vertex is prev->vertex
    int prev; //ccw, next is prev->prev
    int twin;
};

class PINOCCHIO_API Mesh {
public:
	Mesh();
    Mesh(const string &file);

    bool integrityCheck() const;
    bool isConnected() const; //returns true if the mesh consists of a single connected component
    void computeVertexNormals();
    void normalizeBoundingBox();
    void computeTopology();
	void fixDupFaces();
	void sortEdges(); //sort edges so that triplets forming faces are adjacent
    
public: //data
    vector<MeshVertex> m_Vertices;
    vector<MeshEdge> m_Edges; //halfEdges, really

    PVector3 m_ToAdd;
    double m_Scale;
	bool m_Normalized;
	bool m_ComputedNormal;
};

#endif
