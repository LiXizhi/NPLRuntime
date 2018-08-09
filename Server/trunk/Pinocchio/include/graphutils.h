/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef GRAPHUTILS_H
#define GRAPHUTILS_H

#include <queue>
#include "vector.h"

struct PtGraph
{
    vector<PVector3> verts;
    vector<vector<int> > edges;
    
    bool integrityCheck() const;
};
    
class ShortestPather
{
public:
    ShortestPather(const PtGraph &g, int root);
        
    vector<int> pathFrom(int vtx) const
    {
        vector<int> out(1, vtx);
        while(m_Prev[vtx] >= 0)
            out.push_back(vtx = m_Prev[vtx]);
        return out;
    }
    double distFrom(int vtx) const { return m_Dist[vtx]; }
            
private:
    struct Inf
    {
        Inf(double inDist, int inNode, int inPrev) : dist(inDist), node(inNode), prev(inPrev) {}
        bool operator<(const Inf &inf) const { return dist > inf.dist; }
        double dist;
        int node, prev;
    };
        
    vector<int> m_Prev;
    vector<double> m_Dist;
};

class AllShortestPather
{
public:
    AllShortestPather() {}
    
    AllShortestPather(const PtGraph &g)
    {
        for(int i = 0; i < (int)g.verts.size(); ++i)
            m_Paths.push_back(ShortestPather(g, i));
    }
    
    vector<int> path(int from, int to) const { return m_Paths[to].pathFrom(from); }
    double dist(int from, int to) const { return m_Paths[to].distFrom(from); }
    
private:
    vector<ShortestPather> m_Paths;
};


#endif
