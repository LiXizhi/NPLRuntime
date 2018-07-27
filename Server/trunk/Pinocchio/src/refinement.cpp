
#include "pinocchioApi.h"
#include "deriv.h"
#include "debugging.h"
#include <algorithm>

using namespace Pinocchio;

struct RP //information for refined embedding
{
    RP(TreeType *inD, const Skeleton &inSk, const vector<PVector3> &medialSurface)
        : distanceField(inD), given(inSk)
    {
        vector<Vec3Object> mpts;
        for(int i = 0; i < (int)medialSurface.size(); ++i)
            mpts.push_back(medialSurface[i]);

        medProjector = ObjectProjector<3, Vec3Object>(mpts);
    }

    TreeType *distanceField;
    const Skeleton &given;
    ObjectProjector<3, Vec3Object> medProjector;
};

template<class Real> Real computeFineError(const vector<PVector<Real, 3> > &match, RP *rp)
{
    Real out = Real();
    int i;
    for(i = 1; i < (int)match.size(); ++i) {
        int prev = rp->given.fPrev()[i];
        
        Real surfPenalty = Real();
        Real lenPenalty = Real();
        Real anglePenalty = Real();
        Real symPenalty = Real();
        
        //-----------------surf
        const int samples = 10;
        for(int k = 0; k < samples; ++k) {
            double frac = double(k) / double(samples);
            PVector<Real, 3> cur = match[i] * Real(1. - frac) + match[prev] * Real(frac);
            PVector3 m = rp->medProjector.project(cur);
            Real medDist = (cur - PVector<Real, 3>(m)).length();
            Real surfDist = -rp->distanceField->locate(cur)->evaluate(cur);
            Real penalty = SQR(min(medDist, Real(0.001) + max(Real(0.), Real(0.05) - surfDist)));
            if(penalty > Real(SQR(0.003)))
                surfPenalty += Real(1. / double(samples)) * penalty;
        }
        
        //---------------length
        Real optDistSq = (rp->given.fGraph().verts[i] - rp->given.fGraph().verts[prev]).lengthsq();
        Real distSq = SQR(max(Real(-10.), (match[i] - match[prev]) *
                                        (rp->given.fGraph().verts[i] - rp->given.fGraph().verts[prev]))) / optDistSq;
        lenPenalty = max(Real(.5), (Real(0.0001) + optDistSq) / (Real(0.0001) + distSq));
        
        //---------------sym
        if(rp->given.fSym()[i] != -1) {
            int s = rp->given.fSym()[i];
            int sp = rp->given.fPrev()[s];
            
            Real sDistSq = (match[s] - match[sp]).lengthsq();
            symPenalty = max(Real(1.05), max(distSq / (Real(0.001) + sDistSq), sDistSq / (Real(0.001) + distSq)));
        }
        
        //--------------angle
        if(distSq > Real(1e-16)) {
            PVector<Real, 3> curDir = (match[i] - match[prev]).normalize();
            PVector<Real, 3> skelDir = (rp->given.fGraph().verts[i] - rp->given.fGraph().verts[prev]).normalize();
            if(curDir * skelDir < Real(1. - 1e-8))
                anglePenalty = Real(0.5) * acos(curDir * skelDir);
            anglePenalty = CUBE(Real(0.3) + anglePenalty);
            if(curDir * skelDir < Real(0.))
                anglePenalty *= 10.;
        }
        
        out += Real(15000.) * surfPenalty + Real(0.25) * lenPenalty + Real(2.0) * anglePenalty + symPenalty;
    }
    
    return out;
}

vector<PVector3> optimizeEmbedding1D(vector<PVector3> fineEmbedding, vector<PVector3> dir, RP *rp)
{
    int i;
    double step = 0.001;
    
    for(i = 0; i < (int)fineEmbedding.size(); ++i) {
        step += dir[i].lengthsq();
    }
    
    step = 0.0005 / sqrt(step);
    
    double prevErr = -1e10;
    int count = 0;
    while(++count) {
        double curErr = computeFineError(fineEmbedding, rp);
        if(prevErr == -1e10 || curErr < prevErr) {
            step *= 2.;
            for(i = 0; i < (int)fineEmbedding.size(); ++i) {
                fineEmbedding[i] += dir[i] * step;
            }
            prevErr = curErr;
        }
        else {
            if(count > 2) {
                for(i = 0; i < (int)fineEmbedding.size(); ++i) {
                    fineEmbedding[i] -= dir[i] * step;
                }
            }
            break;
        }
    }
    
    return fineEmbedding;
}



//refines embedding
vector<PVector3> Pinocchio::RefineEmbedding(TreeType *distanceField, const vector<PVector3> &medialSurface,
                                const vector<PVector3> &initialEmbedding, const Skeleton &skeleton)
{
    RP rp(distanceField, skeleton, medialSurface);

    int sz = initialEmbedding.size();
    vector<PVector3> fineEmbedding = initialEmbedding;
    int i, k;
    for(k = 0; k < 10; ++k) {
        typedef Deriv<double, 6> DType;
        typedef Deriv<double, -1> DType1;
        
        Debugging::out() << "E = " << computeFineError(fineEmbedding, &rp) << endl;
        
        for(int j = 0; j < 2; ++j) {
            vector<PVector<DType1, 3> > dMatch(sz);
            for(i = 0; i < sz; ++i) {
                dMatch[i][0] = DType1(fineEmbedding[i][0], i * 3);
                dMatch[i][1] = DType1(fineEmbedding[i][1], i * 3 + 1);
                dMatch[i][2] = DType1(fineEmbedding[i][2], i * 3 + 2);
            }

            DType1 err = computeFineError(dMatch, &rp) + (DType1() * DType1(0., 3 * sz));
            vector<PVector3> dir(sz);
        
            for(i = 0; i < sz; ++i) {
                dir[i][0] = -err.getDeriv(i * 3);
                dir[i][1] = -err.getDeriv(i * 3 + 1);
                dir[i][2] = -err.getDeriv(i * 3 + 2);
            }
            fineEmbedding = optimizeEmbedding1D(fineEmbedding, dir, &rp);
        }
        
        int cur;
        for(cur = 1; cur < sz; ++cur) {
            int prev = skeleton.fPrev()[cur];
            
            vector<PVector<DType, 3> > dMatch(sz);
            for(i = 0; i < sz; ++i) {
                dMatch[i][0] = DType(fineEmbedding[i][0]);
                dMatch[i][1] = DType(fineEmbedding[i][1]);
                dMatch[i][2] = DType(fineEmbedding[i][2]);
                
                int varNum = -1;
                if(i == cur)
                    varNum = 0;
                else if(i == prev)
                    varNum = 3;
                
                if(varNum >= 0) {
                    dMatch[i][0] *= DType(1, varNum + 0);
                    dMatch[i][1] *= DType(1, varNum + 1);
                    dMatch[i][2] *= DType(1, varNum + 2);
                }
            }
            
            DType err = computeFineError(dMatch, &rp);
            
            vector<PVector3> dir(sz);
        
            for(i = 0; i < sz; ++i) {
                int varNum = -1;
                if(i == cur)
                    varNum = 0;
                else if(i == prev)
                    varNum = 3;
                
                if(varNum >= 0) {
                    dir[i][0] = -err.getDeriv(varNum);
                    dir[i][1] = -err.getDeriv(varNum + 1);
                    dir[i][2] = -err.getDeriv(varNum + 2);
                }
            }
            fineEmbedding = optimizeEmbedding1D(fineEmbedding, dir, &rp);
        }
    }
    
    return fineEmbedding;
}

