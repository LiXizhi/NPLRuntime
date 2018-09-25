/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef INTERSECTOR_H
#define INTERSECTOR_H

#include "mesh.h"
#include "vecutils.h"

class PINOCCHIO_API Intersector {
public:
    Intersector() : m_Mesh(NULL) {}
    Intersector(const Mesh &m, const PVector3 &inDir) : m_Mesh(&m), m_Dir(inDir.normalize()) { init(); }
    
    vector<PVector3> intersect(const PVector3 &pt, vector<int> *outIndices = NULL) const;    
	vector<PVector3> intersect_orig(const PVector3 &pt, vector<int> *outIndices = NULL) const;
    const PVector3& getDir() const { return m_Dir; }
private:
    void init();
    void getIndex(const PVector2 &pt, int &x, int &y) const;
    
    const Mesh* m_Mesh;
    PVector3 m_Dir;
    PVector3 m_V1, m_V2; //basis
    Rect2 m_Bounds; //within the basis
    
    vector<PVector2> m_Points;
    vector<PVector3> m_ScaledNormals; //they are scaled for intersection
    vector<vector<int> > m_Triangles;
};

#endif //INTERSECTOR_H
