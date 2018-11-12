
#include "lsqSolver.h"

#include <queue>
#include <set>
#include <iostream>
#include "hashutils.h"
#include "debugging.h"

#ifdef TAUCS //TAUCS

#include <complex>
#include <cmath>
extern "C" {
#include "taucs.h"
}

vector<int> SPDMatrix::computePerm() const
{
    return vector<int>(); //do nothing
}

class TaucsLLTMatrix : public LLTMatrix
{
public:
    TaucsLLTMatrix() : m(NULL), factorization(NULL), sz(0) {}
    
    ~TaucsLLTMatrix()
    {
        taucs_ccs_free(m);
        taucs_linsolve(NULL, &factorization, 0, NULL, NULL, NULL, NULL);
    }
    
    bool solve(vector<double> &b) const
    {
        TaucsLLTMatrix *cthis = const_cast<TaucsLLTMatrix *>(this);
        vector<double> x(b.size());
        char* solve [] = {"taucs.factor=false", NULL};
        int rc = taucs_linsolve(m, &(cthis->factorization), 1, &(*x.begin()), &(*b.begin()), solve, NULL);
        if(rc != TAUCS_SUCCESS) {
            Debugging::out() << "Taucs error solving: rc = " << rc << endl;
            assert(false);
        }
        b = x;
        return true;
    }
    
    int size() const { return sz; }
    
private:
    taucs_ccs_matrix *m;
    void *factorization;
    int sz;
    friend class SPDMatrix;
};

LLTMatrix *SPDMatrix::factor() const
{
    //taucs_logfile("stdout");
    int i, j;
    TaucsLLTMatrix *out = new TaucsLLTMatrix();
    int sz = out->sz = m.size();
    int nz = 0;
    Debugging::out() << "Size = " << sz << endl;
    
    for(i = 0; i < sz; ++i)
        nz += m[i].size();
    
    out->m = taucs_ccs_create(sz, sz, nz, TAUCS_DOUBLE | TAUCS_SYMMETRIC | TAUCS_LOWER);
    
    vector<vector<pair<int, double> > > mt(sz); //transposed values
    for(i = 0; i < sz; ++i) {
        for(j = 0; j < (int)m[i].size(); ++j) {
            mt[m[i][j].first].push_back(make_pair(i, m[i][j].second));
        }
    }
    
    out->m->colptr[0] = 0;
    for(i = 0; i < sz; ++i) {
        int cur = out->m->colptr[i];
        out->m->colptr[i + 1] = cur + mt[i].size();
        for(j = 0; j < (int)mt[i].size(); ++j) {
            if(mt[i][j].first < i)
                Debugging::out() << "Err!" << endl;
            out->m->rowind[cur + j] = mt[i][j].first;
            out->m->values.d[cur + j] = mt[i][j].second;
        }
    }
    
    //char* mf[]   = {"taucs.factor.LLT=true", "taucs.factor.mf=true", NULL};
    //char* ll[]   = {"taucs.factor.LLT=true", "taucs.factor.ll=true", NULL};
    char* def[]   = {"taucs.factor.LLT=true", NULL};

    int rc;
    rc = taucs_linsolve(out->m, &(out->factorization), 0, NULL, NULL, def, NULL);
    
    if(rc != TAUCS_SUCCESS) {
        Debugging::out() << "Taucs error factoring: rc = " << rc << endl;
        assert(false);
    }
    else Debugging::out() << "Factored!" << endl;
    
    return out;
}

#else
class MyLLTMatrix : public LLTMatrix
{
public:
    bool solve(vector<double> &b) const; //solves it in place
    int size() const { return m.size(); }

private:
    void initMt();
    vector<vector<pair<int, double> > > m; //off-diagonal values stored by rows
    vector<vector<pair<int, double> > > mt; //off-diagonal values transposed stored by rows
    vector<double> diag; //values on diagonal
    vector<int> perm; //permutation

    friend class SPDMatrix;
};


vector<int> SPDMatrix::computePerm() const
{
    int i, j;

    vector<int> out;
    int sz = m.size();

#if 0 //No permutation
    out.resize(sz);
    for(i = 0; i < sz; ++i)
        out[i] = i;
    return out;
    random_shuffle(out.begin(), out.end());
    return out;
#endif

    //initialize
    set<pair<int, int> > neighborSize;
    vector<unordered_set<int> > neighbors(sz);
    for(i = 0; i < sz; ++i) {
        for(j = 0; j < (int)m[i].size() - 1; ++j) {
            neighbors[i].insert(m[i][j].first);
            neighbors[m[i][j].first].insert(i);
        }
    }
    for(i = 0; i < sz; ++i)
        neighborSize.insert(make_pair(neighbors[i].size(), i));

    //iterate
    while(!neighborSize.empty()) {
        //remove the neighbor of minimum degree
        int cur = (neighborSize.begin())->second;
        neighborSize.erase(neighborSize.begin());

        out.push_back(cur);

        //collect the neighbors of eliminated vertex
        vector<int> nb(neighbors[cur].begin(), neighbors[cur].end()); 
        //erase them from the neighborSize set because their neighborhood sizes are about
        //to change
        for(i = 0; i < (int)nb.size(); ++i)
            neighborSize.erase(make_pair(neighbors[nb[i]].size(), nb[i]));
        //erase the eliminated vertex from their neighbor lists
        for(i = 0; i < (int)nb.size(); ++i)
            neighbors[nb[i]].erase(neighbors[nb[i]].find(cur));
        //make the neighbors all adjacent
        for(i = 0; i < (int)nb.size(); ++i) for(j = 0; j < i; ++j) {
            if(neighbors[nb[i]].count(nb[j]) == 0) {
                neighbors[nb[i]].insert(nb[j]);
                neighbors[nb[j]].insert(nb[i]);
            }
        }
        //and put them back into the neighborSize set
        for(i = 0; i < (int)nb.size(); ++i)
            neighborSize.insert(make_pair(neighbors[nb[i]].size(), nb[i]));
    }

    vector<int> oout = out;
    for(i = 0; i < sz; ++i) //invert the permutation
        out[oout[i]] = i;
        
    return out;
}

LLTMatrix *SPDMatrix::factor() const
{
    int i, j, k;
    MyLLTMatrix *outP = new MyLLTMatrix();
    MyLLTMatrix &out = *outP;
    int sz = m.size();
    out.m.resize(sz);
    out.diag.resize(sz);

    Debugging::out() << "Factoring size = " << sz << endl;
    
    out.perm = computePerm();

    Debugging::out() << "Perm computed" << endl;
    
    //permute matrix according to the permuation
    vector<vector<pair<int, double> > > pm(sz);
    for(i = 0; i < sz; ++i) {
        for(j = 0; j < (int)m[i].size(); ++j) {
            int ni = out.perm[i], nidx = out.perm[m[i][j].first];
            if(ni >= nidx)
                pm[ni].push_back(make_pair(nidx, m[i][j].second));
            else
                pm[nidx].push_back(make_pair(ni, m[i][j].second));
        }
    }
    for(i = 0; i < sz; ++i)
        std::sort(pm[i].begin(), pm[i].end());

    //prepare for decomposition
    vector<vector<pair<int, double> > > cols(sz);
    vector<double> dinv(sz); //inverses of out.diag

    vector<bool> added(sz, false);

    //Sparse cholesky decomposition
    for(i = 0; i < sz; ++i) { //current row
        vector<int> columnsAdded;
        columnsAdded.reserve(2 * pm[i].size());

        //compute columnsAdded (nonzero indices of factor in current row)
        int inCA = 0;
        for(j = 0; j < (int)pm[i].size() - 1; ++j) {
            added[pm[i][j].first] = true;
            columnsAdded.push_back(pm[i][j].first);
        }
        while(inCA < (int)columnsAdded.size()) {
            int idx = columnsAdded[inCA];
            ++inCA;
            for(k = 0; k < (int)cols[idx].size(); ++k) {
                int curCol = cols[idx][k].first;
                if(!added[curCol]) {
                    added[curCol] = true;
                    columnsAdded.push_back(curCol);
                }
            }
        }
        std::sort(columnsAdded.begin(), columnsAdded.end());

        for(j = 0; j < (int)columnsAdded.size(); ++j) {//add the columns and clear added
            added[columnsAdded[j]] = false;
            cols[columnsAdded[j]].push_back(make_pair(i, 0.));
        }

        for(j = 0; j < (int)pm[i].size() - 1; ++j) { //initialize it with m's entries
            int curCol = pm[i][j].first;
            cols[curCol].back().second = pm[i][j].second * dinv[curCol];
        }
        for(j = 0; j < (int)columnsAdded.size(); ++j) { //current column
            int idx = columnsAdded[j];
            int csz = cols[idx].size() - 1;
            for(k = 0; k < csz; ++k) { //index in column above current row -- inner loop
                int tidx = cols[idx][k].first; //index into current row
                double prod = cols[idx][k].second * cols[idx].back().second * dinv[tidx];
                cols[tidx].back().second -= prod;
            }
        }
        //now diagonal
        out.diag[i] = pm[i].back().second;
        for(j = 0; j < (int)columnsAdded.size(); ++j) {
            double val = cols[columnsAdded[j]].back().second;
            out.diag[i] -= SQR(val);
            out.m[i].push_back(make_pair(columnsAdded[j], val)); //also add rows to output
        }
        if(out.diag[i] <= 0.) { //not positive definite
            assert(false && "Not positive definite matrix (or ill-conditioned)");
            delete outP;
            return new MyLLTMatrix();
        }
        out.diag[i] = sqrt(out.diag[i]);
        dinv[i] = 1. / out.diag[i];
    }

    out.initMt();

    /* Error check 
    double totErr = 0;
    for(i = 0; i < m.size(); ++i) {
        for(j = 0; j < m[i].size(); ++j) {
            int q = m[i][j].first;
            double total = -m[i][j].second;
            out.m[i].push_back(make_pair(i, out.diag[i]));
            if(i != q) out.m[q].push_back(make_pair(q, out.diag[q]));
            for(k = 0; k < out.m[i].size(); ++k) {
                for(int z = 0; z < out.m[q].size(); ++z) {
                    if(out.m[i][k].first != out.m[q][z].first)
                        continue;
                    total += out.m[i][k].second * out.m[q][z].second;
                }
            }
            out.m[i].pop_back();
            if(i != q) out.m[q].pop_back();

            totErr += fabs(total);
        }
    }
    */

    return outP;
}

void MyLLTMatrix::initMt()  //compute the transposed entries (by rows)
{
    int i, j;

    mt.clear();
    mt.resize(m.size());

    for(i = 0; i < (int)m.size(); ++i) {
        for(j = 0; j < (int)m[i].size(); ++j) {
            mt[m[i][j].first].push_back(make_pair(i, m[i][j].second));
        }
    }
}

bool MyLLTMatrix::solve(vector<double> &b) const
{
    int i, j;

    if(b.size() != m.size())
        return false;

    vector<double> bp(b.size());
    //permute
    for(i = 0; i < (int)b.size(); ++i)
        bp[perm[i]] = b[i];

    //solve L (L^T x) = b for (L^T x)
    for(i = 0; i < (int)bp.size(); ++i) {
        for(j = 0; j < (int)m[i].size(); ++j)
            bp[i] -= bp[m[i][j].first] * m[i][j].second;
        bp[i] /= diag[i];
    }

    //solve L^T x = b for x
    for(i = bp.size() - 1; i >= 0; --i) {
        for(j = 0; j < (int)mt[i].size(); ++j)
            bp[i] -= bp[mt[i][j].first] * mt[i][j].second;
        bp[i] /= diag[i];
    }

    //unpermute
    for(i = 0; i < (int)b.size(); ++i)
        b[i] = bp[perm[i]];

    return true;
}
#endif
