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

#include "vector.h"
#include "rect.h"

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
    Mesh() : m_Scale(1.) {}
    Mesh(const string &file);

    bool integrityCheck() const;
    bool isConnected() const; //returns true if the mesh consists of a single connected component
    void computeVertexNormals();
    void normalizeBoundingBox();
    void computeTopology();
    void writeObj(const string &filename) const;
	void fixDupFaces();
	void sortEdges(); //sort edges so that triplets forming faces are adjacent
    
private:
    void readObj(istream &strm);
    void readOff(istream &strm);
    void readPly(istream &strm);
    void readGts(istream &strm);
    void readStl(istream &strm);

public: //data
    vector<MeshVertex> m_Vertices;
    vector<MeshEdge> m_Edges; //halfEdges, really

    PVector3 m_ToAdd;
    double m_Scale;
};

#endif
