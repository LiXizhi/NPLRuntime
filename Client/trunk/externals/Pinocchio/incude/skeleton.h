/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef SKELETON_H
#define SKELETON_H

#include <map>

#include "graphutils.h"

class PINOCCHIO_API Skeleton {
public:
    Skeleton() {}
    
    const PtGraph &fGraph() const { return m_fGraphV; }
    const vector<int> &fPrev() const { return m_fPrevV; }
    const vector<int> &fSym() const { return m_fSymV; }
    
    const PtGraph &cGraph() const { return m_cGraphV; }
    const vector<int> &cPrev() const { return m_cPrevV; }
    const vector<int> &cSym() const { return m_cSymV; }
    const vector<bool> &cFeet() const { return m_cFeetV; }
    const vector<bool> &cFat() const { return m_cFatV; }
    
    const vector<int> &cfMap() const { return m_cfMapV; }
    const vector<int> &fcMap() const { return m_fcMapV; }
    const vector<double> &fcFraction() const { return m_fcFractionV; }
    const vector<double> &cLength() const { return m_cLengthV; }

    int getJointForName(const std::string &name) const { if(jointNames.count(name)) return jointNames.find(name)->second; return -1; }
    
    void scale(double factor);
    
    void initCompressed();
    
    //help for creation
    map<string, int> jointNames;
	map<int, string> indexNameMap;
    void makeJoint(const string &name, const PVector3 &pos, const string &previous = string());
    void makeSymmetric(const string &name1, const string &name2);
    void setFoot(const string &name);
    void setFat(const string &name);
    
private:
     //full
    PtGraph m_fGraphV;
    vector<int> m_fPrevV; //previous vertices
    vector<int> m_fSymV; //symmetry
    
    //compressed (no degree 2 vertices)
    PtGraph m_cGraphV; 
    vector<int> m_cPrevV; //previous vertices
    vector<int> m_cSymV; //symmetry
    vector<bool> m_cFeetV; //whether the vertex should be near the ground
    vector<bool> m_cFatV; //whether the vertex should be in a large region
    
    vector<int> m_cfMapV; //compressed to full map
    vector<int> m_fcMapV; //full to compressed map, -1 when vertex is not in compressed
    vector<double> m_fcFractionV; //maps full vertex number to ratio of its prev edge length to total length of
                                //containing edge in the compressed graph
    vector<double> m_cLengthV; //lengths of the compressed bones
};

class PINOCCHIO_API HumanSkeleton : public Skeleton
{
public:
    HumanSkeleton();
};

class PINOCCHIO_API QuadSkeleton : public Skeleton
{
public:
    QuadSkeleton();
};

class PINOCCHIO_API HorseSkeleton : public Skeleton
{
public:
    HorseSkeleton();
};

class PINOCCHIO_API CentaurSkeleton : public Skeleton
{
public:
    CentaurSkeleton();
};

class PINOCCHIO_API FileSkeleton : public Skeleton
{
public:
    FileSkeleton(const std::string &filename);
}; 

#endif
