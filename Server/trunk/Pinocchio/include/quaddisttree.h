/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef QUADDISTTREE_H
#define QUADDISTTREE_H

#include "hashutils.h"
#include "dtree.h"
#include "multilinear.h"
#include "intersector.h"
#include "pointprojector.h"
#include <numeric>
#include <map>

template<int Dim>
class DistFunction : public Multilinear<double, Dim>
{
    typedef Multilinear<double, Dim> super;
    typedef Rect<double, Dim> MyRect;
public:
    template<class Eval> void initFunc(const Eval &eval, const MyRect &rect)
    {
        for(int i = 0; i < (1 << Dim); ++i) {
            setValue(i, eval(rect.getCorner(i)));
        }
        return;
    }
    
private:
};

template<int Dim>
class DistData : public DistFunction<Dim>
{
public:
    typedef DistFunction<Dim> super;
    typedef DNode<DistData<Dim>, Dim> NodeType;

    DistData(NodeType *inNode) : m_Node(inNode) {}

    void init() { }

    template<class Eval, template<typename Node, int IDim> class Indexer>
    void fullSplit(const Eval &eval, double tol, DRootNode<DistData<Dim>, Dim, Indexer> *rootNode, int level = 0, bool cropOutside = false)
    {
        int i;
        const Rect<double, Dim> &rect = m_Node->getRect();
        m_Node->initFunc(eval, rect);
        
        bool nextCropOutside = cropOutside;
        if(cropOutside && level > 0) {
            double center = eval(rect.getCenter());
            double len = rect.getSize().length() * 0.5;
            if(center > len)
                return;
            if(center < -len)
                nextCropOutside = false;
        }
        
        if(level == (32 / Dim))// why ?
            return;
        bool doSplit = false;
        if(m_Node->getParent() == NULL)
            doSplit = true;
        if(!doSplit) {
            int idx[Dim + 1];
            for(i = 0; i < Dim + 1; ++i)
                idx[i] = 0;
            PVector<double, Dim> center = rect.getCenter();
            while(idx[Dim] == 0) {
                PVector<double, Dim> cur;
                bool anyMid = false;
                for(i = 0; i < Dim; ++i) {
                    switch(idx[i]) {
                        case 0: cur[i] = rect.getLo()[i]; break;
                        case 1: cur[i] = rect.getHi()[i]; break;
                        case 2: cur[i] = center[i]; anyMid = true; break;
                    }
                }
                if(anyMid && fabs(evaluate(cur) - eval(cur)) > tol) {
                    doSplit = true;
                    break;
                }
                for(i = 0; i < Dim + 1; ++i) {
                    if(idx[i] != 2) {
                        idx[i] += 1;
                        for(--i; i >= 0; --i)
                            idx[i] = 0;
                        break;
                    }
                }
            }
        }
        if(!doSplit)
            return;
        rootNode->split(m_Node);
        for(i = 0; i < NodeType::numChildren; ++i) {
            eval.setRect(Rect<double, Dim>(rect.getCorner(i)) | Rect<double, Dim>(rect.getCenter()));
            m_Node->getChild(i)->fullSplit(eval, tol, rootNode, level + 1, nextCropOutside);
        }
    }

    template<class Real> Real evaluate(const PVector<Real, Dim> &v)
    {
        if(m_Node->getChild(0) == NULL) {
            return super::evaluate((v - m_Node->getRect().getLo()).apply(divides<Real>(),
                                                                       m_Node->getRect().getSize()));
        }
        PVector<Real, Dim> center = m_Node->getRect().getCenter();
        int idx = 0;
        for(int i = 0; i < Dim; ++i)
            if(v[i] > center[i])
                idx += (1 << i);
        return m_Node->getChild(idx)->evaluate(v);
    }

    template<class Real> Real integrate(Rect<Real, Dim> r)
    {
        r &= Rect<Real, Dim>(m_Node->getRect());
        if(r.isEmpty())
            return Real();
        if(m_Node->getChild(0) == NULL) {
            PVector<Real, Dim> corner = m_Node->getRect().getLo(), size = m_Node->getRect().getSize();
            Rect<Real, Dim> adjRect((r.getLo() - corner).apply(divides<Real>(), size),
                                    (r.getHi() - corner).apply(divides<Real>(), size));
            return Real(m_Node->getRect().getContent()) * super::integrate(adjRect);
        }
        Real out = Real();
        for(int i = 0; i < NodeType::numChildren; ++i)
            out += m_Node->getChild(i)->integrate(r);
        return out;
    }

private:
    NodeType *m_Node;
};

typedef DistData<3>::NodeType OctTreeNode;
typedef DRootNode<DistData<3>, 3> OctTreeRoot;

template<class RootNode = OctTreeRoot> class OctTreeMaker 
{
public:
    static RootNode *make(const ObjectProjector<3, Tri3Object> &proj, const Mesh &m, double tol)
    {
        DistObjEval eval(proj, m);
        RootNode *out = new RootNode();

        out->fullSplit(eval, tol, out, 0, true);
        out->preprocessIndex();

        return out;
    }

    static RootNode *make(const ObjectProjector<3, Vec3Object> &proj, double tol, const RootNode *dTree = NULL)
    {
        PointObjDistEval eval(proj, dTree);
        RootNode *out = new RootNode();

        out->fullSplit(eval, tol, out);
        out->preprocessIndex();

        return out;
    }

private:
    class DistObjEval
    {
    public:
        DistObjEval(const ObjectProjector<3, Tri3Object> &inProj, const Mesh &m) : m_Proj(inProj), m_Intersector(m, PVector3(1, 1, 1))
        {
            m_Level = 0;
            m_Rects[0] = Rect3(PVector3(), PVector3(1.));
            m_Inside[0] = 0;
        }

        double operator()(const PVector3 &vec) const
        {
            unsigned int cur = ROUND(vec[0] * 1023.) + 1024 * (ROUND(vec[1] * 1023.) + 1024 * ROUND(vec[2] * 1023.));
            unsigned int sz = m_Cache.size();
            double &d = m_Cache[cur];
            if(sz == m_Cache.size())
                return d;
            return d = compute(vec);
        }

        void setRect(const Rect3 &r) const
        {
            while(!(m_Rects[m_Level].contains(r.getCenter()))) --m_Level;

            m_Inside[m_Level + 1] = m_Inside[m_Level];
            ++m_Level;
            if(!m_Inside[m_Level]) {
                double d = (*this)(r.getCenter());
                double diag2 = r.getSize().length() * 0.5;
            
                if(d >= diag2)
                    m_Inside[m_Level] = 1;
                else if(d <= -diag2)
                    m_Inside[m_Level] = -1;
            }
                       
            m_Rects[m_Level] = r;
        }

    private:
        double compute(const PVector3 &vec) const
        {
            int i, ins = m_Inside[m_Level];
            if(!ins) {
				// Count the number of triangles the ray intersection where the x - coordinate( the coordinate thrown out) is greater than the x - coordinate of the point.
				// An even number of intersections means it is outside the mesh.
				// An odd number of intersections means it is inside the mesh.
                ins = 1;
                vector<PVector3> isecs = m_Intersector.intersect(vec);
                for(i = 0; i < (int)isecs.size(); ++i) {
                    //if(isecs[i][0] > vec[0])
                    //    ins = -ins;
					if ( (isecs[i] - vec) * m_Intersector.getDir() > 0 ) {
						ins = -ins;
					}
                }
            }
            
            return (vec - m_Proj.project(vec)).length() * ins;
        }
        
        mutable map<unsigned int, double> m_Cache;
        const ObjectProjector<3, Tri3Object>& m_Proj;
        Intersector m_Intersector;
        mutable Rect3 m_Rects[11];
        mutable int m_Inside[11];
        mutable int m_Level; //essentially index of last rect
    };
    
    class PointObjDistEval
    {
    public:
        PointObjDistEval(const ObjectProjector<3, Vec3Object> &inProj, const RootNode *inDTree) : m_Proj(inProj), m_dTree(inDTree) {}

        double operator()(const PVector3 &vec) const
        {
            unsigned int cur = ROUND(vec[0] * 1023.) + 1024 * (ROUND(vec[1] * 1023.) + 1024 * ROUND(vec[2] * 1023.));
            unsigned int sz = m_Cache.size();
            double &d = m_Cache[cur];
            if(sz == m_Cache.size())
                return d;
            return d = (vec - m_Proj.project(vec)).length();
        }

        void setRect(const Rect3 &r) const { }

    private:
        mutable unordered_map<unsigned int, double> m_Cache;
        const ObjectProjector<3, Vec3Object>& m_Proj;
        const RootNode* m_dTree;
    };
};
#endif
