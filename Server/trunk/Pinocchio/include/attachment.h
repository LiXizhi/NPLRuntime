/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef ATTACHMENT_H
#define ATTACHMENT_H

#include "mesh.h"
#include "skeleton.h"
#include "transform.h"

class VisibilityTester
{
public:
    virtual ~VisibilityTester() {}
    virtual bool canSee(const PVector3 &v1, const PVector3 &v2) const = 0;
};

template<class T> class VisTester : public VisibilityTester
{
public:
    VisTester(const T *t) : m_Tree(t) {}

    virtual bool canSee(const PVector3 &v1, const PVector3 &v2) const //faster when v2 is farther inside than v1
    {
        const double maxVal = 0.01;// original 0.002
        double atV2 = m_Tree->locate(v2)->evaluate(v2);
		double step = 100.0;
        double left = (v2 - v1).length();
        double leftInc = left / step;
        PVector3 diff = (v2 - v1) / step;
        PVector3 cur = v1 + diff;
        while(left >= 0.) {
            double curDist = m_Tree->locate(cur)->evaluate(cur);
            if(curDist > maxVal)
                return false;
            //if curDist and atV2 are so negative that distance won't reach above maxVal, return true
            if(curDist + atV2 + left <= maxVal)
                return true;
            cur += diff;
            left -= leftInc;
        }
        return true;
    }

private:
    const T* m_Tree;
};
template<class T> VisibilityTester *makeVisibilityTester(const T *tree) { return new VisTester<T>(tree); } //be sure to delete afterwards

class AttachmentPrivate;

class PINOCCHIO_API Attachment
{
public:
    Attachment() : m_Attachment(NULL) {}
    Attachment(const Attachment &);
    Attachment(const Mesh &mesh, const Skeleton &skeleton, const vector<PVector3> &match, const VisibilityTester *tester, double initialHeatWeight=1.);
    virtual ~Attachment();

    Mesh deform(const Mesh &mesh, const vector<Transform<> > &transforms) const;
    PVector<double, -1> getWeights(int i) const;
private:
    AttachmentPrivate *m_Attachment;
};

#endif
