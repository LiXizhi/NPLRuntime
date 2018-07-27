#include "pinocchioApi.h"
#include "deriv.h"
#include "debugging.h"

#include <algorithm>

using namespace Pinocchio;

//fits mesh inside unit cube, makes sure there's exactly one connected component
Mesh  Pinocchio::PrepareMesh(const Mesh &m)
{
    Mesh out = m;

    if(!m.isConnected()) {
        Debugging::out() << "Bad mesh: should be a single connected component" << endl;
        return Mesh();
    }

    out.computeVertexNormals();
    out.normalizeBoundingBox();

    return out;
}


//constructs a distance field on an octree--user responsible for deleting output
TreeType* Pinocchio::ConstructDistanceField(const Mesh &m, double tol)
{
    vector<Tri3Object> triobjvec;
    for(int i = 0; i < (int)m.m_Edges.size(); i += 3) {
        PVector3 v1 = m.m_Vertices[m.m_Edges[i].vertex].pos;
        PVector3 v2 = m.m_Vertices[m.m_Edges[i + 1].vertex].pos;
        PVector3 v3 = m.m_Vertices[m.m_Edges[i + 2].vertex].pos;
        
        triobjvec.push_back(Tri3Object(v1, v2, v3));
    }
    
    ObjectProjector<3, Tri3Object> proj(triobjvec);

    TreeType *out = OctTreeMaker<TreeType>().make(proj, m, tol);

    Debugging::out() << "Done fullSplit " << out->countNodes() << " " << out->maxLevel() << endl;

    return out;
}

double getMinDot(TreeType *distanceField, const PVector3 &c, double step)
{
    typedef Deriv<double, 3> D;
    typedef PVector<D, 3> VD;
    
    int i, j;
    vector<PVector3> vecs;
    vecs.push_back(PVector3(step, step, step));
    vecs.push_back(PVector3(step, step, -step));
    vecs.push_back(PVector3(step, -step, step));
    vecs.push_back(PVector3(step, -step, -step));
    vecs.push_back(PVector3(-step, step, step));
    vecs.push_back(PVector3(-step, step, -step));
    vecs.push_back(PVector3(-step, -step, step));
    vecs.push_back(PVector3(-step, -step, -step));
    
    for(i = 0; i < (int)vecs.size(); ++i) {
        vecs[i] += c;
        VD vd = VD(D(vecs[i][0], 0), D(vecs[i][1], 1), D(vecs[i][2], 2));
        
        D result = distanceField->locate(vecs[i])->evaluate(vd);
        vecs[i] = PVector3(result.getDeriv(0), result.getDeriv(1), result.getDeriv(2)).normalize();
    }
    
    double minDot = 1.;
    
    for(i = 1; i < (int)vecs.size(); ++i) for(j = 0; j < i; ++j) {
        minDot = min(minDot, vecs[i] * vecs[j]);
    }
    
    return minDot;
}

bool sphereComp(const PSphere &s1, const PSphere &s2) { return s1.radius > s2.radius; }

//samples the distance field to find spheres on the medial surface
//output is sorted by radius in decreasing order
vector<PSphere> Pinocchio::SampleMedialSurface(TreeType *distanceField, double tol)
{
    int i;
    vector<PSphere> out;

    vector<OctTreeNode *> todo;
    todo.push_back(distanceField);
    int inTodo = 0;
    while(inTodo < (int)todo.size()) {
        OctTreeNode *cur = todo[inTodo];
        ++inTodo;
        if(cur->getChild(0)) {
            for(i = 0; i < 8; ++i) {
                todo.push_back(cur->getChild(i));
            }
            continue;
        }
        
        //we are at octree leaf
        Rect3 r = cur->getRect();
        double rad = r.getSize().length() / 2.;
        PVector3 c = r.getCenter();
        double dot = getMinDot(distanceField, c, rad);
        if(dot > 0.)
            continue;
    
        //we are likely near medial surface
        double step = tol;
        double x, y;
        vector<PVector3> pts;
        double sz = r.getSize()[0];
        for(x = 0; x <= sz; x += step) for(y = 0; y <= sz; y += step) {
            pts.push_back(r.getLo() + PVector3(x, y, 0));
            if(y != 0.)
                pts.push_back(r.getLo() + PVector3(x, 0, y));
            if(x != 0. && y != 0.)
                pts.push_back(r.getLo() + PVector3(0, x, y));
        }
        
        //pts now contains a grid on 3 of the octree cell faces (that's enough)
        for(i = 0; i < (int)pts.size(); ++i) {
            PVector3 &p = pts[i];
            double dist = -distanceField->locate(p)->evaluate(p);
            if(dist <= 2. * step)
                continue; //we want to be well inside
            double dot = getMinDot(distanceField, p, step * 0.001);
            if(dot > 0.0)
                continue;
            out.push_back(PSphere(p, dist));
        }
    }// end while
    
    Debugging::out() << "Medial axis points = " << out.size() << endl;
    
    std::sort(out.begin(), out.end(), sphereComp);

    return out;
}

//takes sorted medial surface samples and sparsifies the vector
vector<PSphere> Pinocchio::PackSpheres(const vector<PSphere> &samples, int maxSpheres)
{
    int i, j;
    vector<PSphere> out;

    for(i = 0; i < (int)samples.size(); ++i) {
        for(j = 0; j < (int)out.size(); ++j) {
            if((out[j].center - samples[i].center).lengthsq() < SQR(out[j].radius))
                    break;
        }
        if(j < (int)out.size())
            continue;

        out.push_back(samples[i]);
        if((int)out.size() > maxSpheres)
            break;
    }

    return out;
}

double getMaxDist(TreeType *distanceField, const PVector3 &v1, const PVector3 &v2, double maxAllowed)
{
    double maxDist = -1e37;
    PVector3 diff = (v2 - v1) / 100.;
    for(int k = 0; k < 101; ++k) {
        PVector3 pt = v1 + diff * double(k);
        maxDist = max(maxDist, distanceField->locate(pt)->evaluate(pt));
        if(maxDist > maxAllowed)
            break;
    }
    return maxDist;
}

//constructs graph on packed sphere centers
PtGraph Pinocchio::ConnectSamples(TreeType *distanceField, const vector<PSphere> &spheres)
{
    int i, j;
    PtGraph out;

    for(i = 0; i < (int)spheres.size(); ++i)
        out.verts.push_back(spheres[i].center);
    out.edges.resize(spheres.size());
        
    for(i = 1; i < (int)spheres.size(); ++i) for(j = 0; j < i; ++j) {
        int k;
        PVector3 ctr = (spheres[i].center + spheres[j].center) * 0.5;
        double radsq = (spheres[i].center - spheres[j].center).lengthsq() * 0.25;
        if(radsq < SQR(spheres[i].radius + spheres[j].radius) * 0.25) { //if spheres intersect, there should be an edge
            out.edges[i].push_back(j);
            out.edges[j].push_back(i);
            continue;
        }
        for(k = 0; k < (int)spheres.size(); ++k) {
            if(k == i || k == j)
                continue;
            if((spheres[k].center - ctr).lengthsq() < radsq)
                break; //gabriel graph condition violation
        }
        if(k < (int)spheres.size())
            continue;
        //every point on edge should be at least this far in:
        double maxAllowed = -.5 * min(spheres[i].radius, spheres[j].radius);
        if(getMaxDist(distanceField, spheres[i].center, spheres[j].center, maxAllowed) < maxAllowed) {
            out.edges[i].push_back(j);
            out.edges[j].push_back(i);
        }
    }

    return out;
}
