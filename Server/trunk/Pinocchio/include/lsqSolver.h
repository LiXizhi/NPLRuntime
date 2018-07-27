/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef LSQSOLVER_H_INCLUDED
#define LSQSOLVER_H_INCLUDED

#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>

#include "mathutils.h"

/**
* Represents a factored spd matrix -- primary intended use is inside LSQSystem
*/
class LLTMatrix
{
public:
    virtual ~LLTMatrix() {}
    virtual bool solve(vector<double> &b) const = 0;
    virtual int size() const = 0;
};

/**
* Represents a symmetric positive definite (spd) matrix -- 
* primary intended use is inside LSQSystem (because it's symmetric, only the lower triangle
* is stored)
*/
class SPDMatrix
{
public:
    SPDMatrix(const vector<vector<pair<int, double> > > &inM) : m(inM) {}
    LLTMatrix *factor() const;

private:
    vector<int> computePerm() const; //computes a fill-reduction permutation

    vector<vector<pair<int, double> > > m; //rows -- lower triangle
};

/**
* Sparse linear least squares solver -- with support for hard constraints
* Intended usage:
*    LSQSystem<V, C> s;
*    [
*      s.addConstraint(...); //boolean flag specifies whether soft or hard
*    ]
*    s.factor();
*    [
*      [
*        s.setRhs(...);
*      ]
*      s.solve();
*      [
*        ... = s.getResult(...);
*      ]
*    ]
* Where the stuff in brackets [] may be repeated multiple times
*/
template<class V, class C> class LSQSystem
{
public:
    LSQSystem() : m_FactoredMatrix(NULL) {}
    ~LSQSystem() { if(m_FactoredMatrix) delete m_FactoredMatrix; }

    void addConstraint(bool hard, const map<V, double> &lhs, const C &id)
    {
        m_Constraints[make_pair(id, -1)] = Constraint(hard, lhs);
    }

    void addConstraint(bool hard, double rhs, const map<V, double> &lhs)
    {
        m_Constraints[make_pair(C(), (int)m_Constraints.size())] = Constraint(hard, lhs, rhs);
    }

    void setRhs(const C &id, double rhs)
    {
        assert(m_Constraints.count(make_pair(id, -1)));
        m_Constraints[make_pair(id, -1)].rhs = rhs;
    }

    bool factor()
    {
        int i;
        typename map<pair<C, int>, Constraint>::iterator it;

        //This is a mess.  This function needs to assign indices to both variables and
        //constraints, row-reduce the hard-constraint matrix, and construct the soft constraint
        //matrix by substituting the reduced hard-constraints into the soft constraints.
        //All while keeping track of what's happening to the right hand side of the
        //system.
        //The result is somewhat difficult to follow.
        //One possible optimization (optimize factorization and especially solving in
        //the presence of large amounts of equality constraints and the like)
        //is to treat the constraints whose rhs will not change separately.
        //The way to make it prettier (should this become necessary) is to
        //make structures for the different types of indices (instead of using ints and pairs)
        //also put hard constraints in front rather than at the end.

        //init
        m_VarIds.clear();
        m_ConstraintMap.clear();
        m_SubstitutedHard.clear();
        m_RhsTransform.clear();
        m_SoftMatrix.clear();
        m_SoftNum = 0;

        //assign indices to soft constraints
        for(it = m_Constraints.begin(); it != m_Constraints.end(); ++it) {
            if(!it->second.hard) {
                m_ConstraintMap[it->first] = m_SoftNum++;
            }
        }

        //isolate hard constraints
        vector<map<V, double> > hardConstraints;
        vector<pair<C, int> > hardConstraintIds;
        int hardNum = 0;
        for(it = m_Constraints.begin(); it != m_Constraints.end(); ++it) {
            if(it->second.hard) {
                hardConstraints.push_back(it->second.lhs);
                hardConstraintIds.push_back(it->first);
                ++hardNum;
            }
        }

        map<V, map<V, double> > substitutions; //"substitutions[x] = (y, 3.), (z, 2.)" means "x = 3y+2z + c"
        map<V, map<pair<C, int>, double> > substitutionsRhs; //the "c" in the above equation as a lin.comb. of rhs's
        map<V, int> substitutionConstraintIdx; //keeps track of which constraint a substitution came from
        vector<map<pair<C, int>, double> > hardRhs(hardConstraints.size());
        for(i = 0; i < (int)hardConstraints.size(); ++i)
            hardRhs[i][hardConstraintIds[i]] = 1.;
        while(hardConstraints.size()) {
            typename map<V, double>::iterator it;
            typename map<pair<C, int>, double>::iterator rit;
            //find best variable and equation -- essentially pivoting
            V bestVar;
            int bestEq = -1;
            double bestVal = 0;
            for(i = 0; i < (int)hardConstraints.size(); ++i) {
                for(it = hardConstraints[i].begin(); it != hardConstraints[i].end(); ++it) {
                    //take the variable with the max absolute weight, but also heavily
                    //prefer variables with simple substitutions
                    double val = fabs(it->second) / (double(hardConstraints[i].size()) - 0.9);
                    if(val > bestVal) {
                        bestVal = val;
                        bestEq = i;
                        bestVar = it->first;

                        //an equality or hard assignment constraint is always good enough
                        if(val > .5 && hardConstraints[i].size() <= 2) {
                            i = hardConstraints.size(); //break from the outer loop as well
                            break;
                        }
                    }
                }
            }

            if(bestVal < 1e-10)
                return false; //near-singular matrix

            substitutionConstraintIdx[bestVar] = substitutions.size();
            substitutionsRhs[bestVar] = hardRhs[bestEq];
            m_ConstraintMap[hardConstraintIds[bestEq]] = m_SoftNum + substitutions.size();

            swap(hardConstraints[bestEq], hardConstraints.back());
            swap(hardConstraintIds[bestEq], hardConstraintIds.back());
            swap(hardRhs[bestEq], hardRhs.back());
            double factor = -1. / hardConstraints.back()[bestVar];
            //figure out the substitution
            map<V, double> &curSub = substitutions[bestVar];
            map<pair<C, int>, double> &curSubRhs = substitutionsRhs[bestVar];
            for(it = hardConstraints.back().begin(); it != hardConstraints.back().end(); ++it) {
                if(it->first != bestVar)
                    curSub[it->first] = it->second * factor;
            }
            for(rit = curSubRhs.begin(); rit != curSubRhs.end(); ++rit)
                rit->second *= -factor;

            //now substitute into the unprocessed hard constraints
            hardConstraints.pop_back();
            hardConstraintIds.pop_back();
            hardRhs.pop_back();
            for(i = 0; i < (int)hardConstraints.size(); ++i) {
                if(hardConstraints[i].count(bestVar) == 0)
                    continue;
                double varWeight = hardConstraints[i][bestVar];
                hardConstraints[i].erase(bestVar);
                for(it = curSub.begin(); it != curSub.end(); ++it)
                    hardConstraints[i][it->first] += it->second * varWeight;
                //and the rhs
                for(rit = curSubRhs.begin(); rit != curSubRhs.end(); ++rit)
                    hardRhs[i][rit->first] -= rit->second * varWeight;
            }

            //now substitute into the other substitutions
            typename map<V, map<V, double> >::iterator sit;
            for(sit = substitutions.begin(); sit != substitutions.end(); ++sit) {
                if(sit->second.count(bestVar) == 0)
                    continue;
                double varWeight = sit->second[bestVar];
                sit->second.erase(bestVar);
                for(it = curSub.begin(); it != curSub.end(); ++it)
                    sit->second[it->first] += it->second * varWeight;
                //and the rhs
                map<pair<C, int>, double> &srhs = substitutionsRhs[sit->first];
                for(rit = curSubRhs.begin(); rit != curSubRhs.end(); ++rit) {
                    srhs[rit->first] += rit->second * varWeight;
                }
            }
        }

        //now that we know which variables are determined by hard constraints, give indices to the rest
        map<V, int> varMap; //maps variables to indices
        //variables from soft constraints first
        for(it = m_Constraints.begin(); it != m_Constraints.end(); ++it) {
            if(it->second.hard)
                continue;
            typename map<V, double>::iterator it2;
            for(it2 = it->second.lhs.begin(); it2 != it->second.lhs.end(); ++it2) {
                if(varMap.count(it2->first) || substitutions.count(it2->first))
                    continue;
                varMap[it2->first] = m_VarIds.size();
                m_VarIds.push_back(it2->first);
            }
        }
        int softVars = m_VarIds.size();
        //then the hard constraint variables
        m_VarIds.resize(softVars + hardNum);
        typename map<V, map<V, double> >::iterator sit;
        for(sit = substitutions.begin(); sit != substitutions.end(); ++sit) {
            int idx = substitutionConstraintIdx[sit->first] + softVars;
            varMap[sit->first] = idx;
            m_VarIds[idx] = sit->first;
        }

        //now compute substitutedHard -- the substitutions with respect to the indices
        m_SubstitutedHard.resize(substitutions.size());
        for(sit = substitutions.begin(); sit != substitutions.end(); ++sit) {
            typename map<V, double>::iterator it;
            int idx = substitutionConstraintIdx[sit->first];
            for(it = sit->second.begin(); it != sit->second.end(); ++it) {
                if(varMap.count(it->first) == 0)
                    return false; //variable is left free by both hard and soft constraints--bad system
                m_SubstitutedHard[idx].push_back(make_pair(varMap[it->first], it->second));
            }
        }

        //compute the softMatrix (and the rhs transform)
        vector<map<int, double> > rhsTransformMap(hardNum); //the rhsTransform matrix as a map
        m_SoftMatrix.resize(m_SoftNum);
        for(it = m_Constraints.begin(); it != m_Constraints.end(); ++it) {
            if(it->second.hard)
                continue;
            map<V, double> modLhs = it->second.lhs;
            typename map<V, double>::iterator it2, it3;
            int idx = m_ConstraintMap[it->first];
            for(it2 = it->second.lhs.begin(); it2 != it->second.lhs.end(); ++it2) {
                if(substitutions.count(it2->first) == 0)
                    continue;
                double fac = it2->second;
                map<V, double> &curSub = substitutions[it2->first];
                for(it3 = curSub.begin(); it3 != curSub.end(); ++it3) {
                    modLhs[it3->first] += fac * it3->second;
                }
                map<pair<C, int>, double> &curRhsSub = substitutionsRhs[it2->first];
                typename map<pair<C, int>, double>::iterator it4;
                for(it4 = curRhsSub.begin(); it4 != curRhsSub.end(); ++it4) {
                    rhsTransformMap[m_ConstraintMap[it4->first] - m_SoftNum][idx] -= fac * it4->second;
                }
            }
            for(it2 = modLhs.begin(); it2 != modLhs.end(); ++it2) { //write modLhs into the right form
                if(substitutions.count(it2->first))
                    continue;
                m_SoftMatrix[idx].push_back(make_pair(varMap[it2->first], it2->second));
            }
            sort(m_SoftMatrix[idx].begin(), m_SoftMatrix[idx].end());
        }

        //add the rhs transforms for the hard constraints
        //and get the rhsTransform into the right form
        typename map<V, map<pair<C, int>, double> >::iterator rit;
        for(rit = substitutionsRhs.begin(); rit != substitutionsRhs.end(); ++rit) {
            typename map<pair<C, int>, double>::iterator it;
            int idx = substitutionConstraintIdx[rit->first] + m_SoftNum;
            for(it = rit->second.begin(); it != rit->second.end(); ++it) {
                rhsTransformMap[m_ConstraintMap[it->first] - m_SoftNum][idx] += it->second;
            }
        }
        for(i = 0; i < hardNum; ++i) {
            m_RhsTransform.push_back(vector<pair<int, double> >(rhsTransformMap[i].begin(),
                                                              rhsTransformMap[i].end()));
        }

        //multiply the softMatrix by its transpose to get an SPDMatrix
        vector<vector<pair<int, double> > > spdm; //the lower triangle of A^T * A
        vector<map<int, double> > spdMap(softVars); //the lower triangle of A^T * A as a map
        for(i = 0; i < (int)m_SoftMatrix.size(); ++i) {
            int j, k;
            for(j = 0; j < (int)m_SoftMatrix[i].size(); ++j) for(k = 0; k <= j; ++k) {
                spdMap[m_SoftMatrix[i][j].first][m_SoftMatrix[i][k].first] +=
                    m_SoftMatrix[i][j].second * m_SoftMatrix[i][k].second;
            }
        }
        for(i = 0; i < softVars; ++i)
            spdm.push_back(vector<pair<int, double> >(spdMap[i].begin(), spdMap[i].end()));

        //factor the SPDMatrix to get the LLTMatrix
        SPDMatrix spdMatrix(spdm);
        if(m_FactoredMatrix)
            delete m_FactoredMatrix;
        m_FactoredMatrix = spdMatrix.factor();
        if(m_FactoredMatrix->size() != softVars)
            return false;

        return true;
    }

    bool solve()
    {
        m_Result.clear();
        typename map<pair<C, int>, Constraint>::const_iterator it;
        vector<double> rhs0, rhs1;

        //grab the rhs's of the constraints
        for(it = m_Constraints.begin(); it != m_Constraints.end(); ++it) {
            int idx = m_ConstraintMap[it->first];
            if((int)rhs0.size() <= idx)
                rhs0.resize(idx + 1, 0.);
            rhs0[idx] = it->second.rhs;
        }

        rhs1 = rhs0;
        int i, j;
        for(i = m_SoftNum; i < (int)rhs1.size(); ++i)
            rhs1[i] = 0; //for hard constraints, transform is absolute, not "additive"
        //transform them (as per hard constraints substitution)
        for(i = 0; i < (int)m_RhsTransform.size(); ++i) {
            for(j = 0; j < (int)m_RhsTransform[i].size(); ++j) {
                rhs1[m_RhsTransform[i][j].first] += m_RhsTransform[i][j].second * rhs0[m_SoftNum + i];
            }
        }

        //multiply by A^T (as in (A^T A)^-1 x = A^T b )
        vector<double> rhs2(m_FactoredMatrix->size(), 0);
        for(i = 0; i < (int)m_SoftMatrix.size(); ++i) { //i is row
            for(j = 0; j < (int)m_SoftMatrix[i].size(); ++j) { //softMatrix[i][j].first is column
                int col = m_SoftMatrix[i][j].first;
                //but the matrix is transposed :)
                rhs2[col] += m_SoftMatrix[i][j].second * rhs1[i];
            }
        }

        if(!m_FactoredMatrix->solve(rhs2))
            return false;

        for(i = 0; i < (int)rhs2.size(); ++i) {
            m_Result[m_VarIds[i]] = rhs2[i];
        }
        
        //now solve for the hard constraints
        int hardNum = (int)m_VarIds.size() - (int)rhs2.size();
        for(i = 0; i < hardNum; ++i) {
            double cur = rhs1[m_SoftNum + i];
            for(j = 0; j < (int)m_SubstitutedHard[i].size(); ++j) {
                cur += m_SubstitutedHard[i][j].second * rhs2[m_SubstitutedHard[i][j].first];
            }
            m_Result[m_VarIds[i + rhs2.size()]] = cur;
        }

        return true;
    }

    double getResult(const V &var) const
    {
        assert(m_Result.find(var) != m_Result.end());
        return m_Result.find(var)->second;
    }

private:
    struct Constraint {
        Constraint() {}
        Constraint(bool inHard, const map<V, double> &inLhs, double inRhs = 0.)
            : hard(inHard), lhs(inLhs), rhs(inRhs) {}

        bool hard;
        map<V, double> lhs;
        double rhs;
    };

    map<pair<C, int>, Constraint> m_Constraints;

    //set during solve
    map<V, double> m_Result;

    //variables set during factor
    int m_SoftNum; //number of soft constraints
    vector<V> m_VarIds; //first the variables softly solved for, then the ones substituted
    map<pair<C, int>, int> m_ConstraintMap;
    vector<vector<pair<int, double> > > m_SubstitutedHard;
    vector<vector<pair<int, double> > > m_RhsTransform;
    vector<vector<pair<int, double> > > m_SoftMatrix;
    LLTMatrix *m_FactoredMatrix;
};

#endif //LSQSOLVER_H_INCLUDED
