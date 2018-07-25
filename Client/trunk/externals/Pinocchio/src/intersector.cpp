
#include "intersector.h"

#include <queue>
#include <set>

//------------------Intersector-----------------

static const int NUM_CELLS = 10;

void Intersector::getIndex(const PVector2 &pt, int &x, int &y) const
{
    PVector2 c = (pt - m_Bounds.getLo()).apply(divides<double>(), m_Bounds.getSize());
    x = int(c[0] * double(NUM_CELLS));
    y = int(c[1] * double(NUM_CELLS));// both c[0]?
    x = max(0, min(NUM_CELLS - 1, x));
    y = max(0, min(NUM_CELLS - 1, y));
}

void Intersector::init()
{
    int i, j, k;
    const vector<MeshVertex> &vtc = m_Mesh->m_Vertices;
    const vector<MeshEdge> &edges = m_Mesh->m_Edges;
    
    m_Dir = m_Dir.normalize();
    getBasis(m_Dir, m_V1, m_V2);// compute coordinate frame
    
    m_Points.resize(vtc.size());
    m_ScaledNormals.resize(edges.size() / 3);
    for(i = 0; i < (int)vtc.size(); ++i) {
        m_Points[i] = PVector2(vtc[i].pos * m_V1, vtc[i].pos * m_V2);
    }
    
    m_Bounds = Rect2(m_Points.begin(), m_Points.end());
    
    m_Triangles.resize(NUM_CELLS * NUM_CELLS);
    for(i = 0; i < (int)edges.size(); i += 3) {
        Rect2 triRect;
        for(j = 0; j < 3; ++j)
            triRect |= Rect2(m_Points[edges[i + j].vertex]);
        
        int fromx, fromy, tox, toy;
        getIndex(triRect.getLo(), fromx, fromy);
        getIndex(triRect.getHi(), tox, toy);
        
		for (j = fromy; j <= toy; ++j) {
			for (k = fromx; k <= tox; ++k) {
				m_Triangles[j * NUM_CELLS + k].push_back(i);
			}
		}
        
        PVector3 cross = (vtc[edges[i + 1].vertex].pos - vtc[edges[i].vertex].pos) % (vtc[edges[i + 2].vertex].pos - vtc[edges[i].vertex].pos);
        j = i / 3;
        m_ScaledNormals[j] = cross.normalize();
		if (fabs(m_ScaledNormals[j] * m_Dir) <= 1e-8) {
			//zero if coplanar
			m_ScaledNormals[j] = PVector3(); 
		}  else {
			//prescaled for intersection
			m_ScaledNormals[j] = m_ScaledNormals[j] / (m_ScaledNormals[j] * m_Dir); 
		}    
    }
}

vector<PVector3> Intersector::intersect(const PVector3 &pt, vector<int> *outIndices) const
{
    int i;
    const vector<MeshVertex> &vtc = m_Mesh->m_Vertices;
    const vector<MeshEdge> &edg = m_Mesh->m_Edges;
    
    vector<PVector3> out;
    
    PVector2 pt2(pt * m_V1, pt * m_V2);
    if(!m_Bounds.contains(pt2))
        return out; //no intersections
    
    int x, y;
    getIndex(pt2, x, y);
    const vector<int> &tris = m_Triangles[y * NUM_CELLS + x];
    for(i = 0; i < (int)tris.size(); ++i) {
        int j;
        //check if triangle intersects line
        int sign[3];
        int idx[3];
        for(j = 0; j < 3; ++j) {
            idx[j] = edg[tris[i] + j].vertex;
        }
        for(j = 0; j < 3; ++j) {
            PVector2 d1 = m_Points[idx[(j + 1) % 3]] - m_Points[idx[j]];
            PVector2 d2 = pt2 - m_Points[idx[j]];
            sign[j] = SIGN(d1[0] * d2[1] - d1[1] * d2[0]);
        }
        if(sign[0] != sign[1] || sign[1] != sign[2])
            continue; //no intersection

        if(outIndices)
	       outIndices->push_back(tris[i]);
        
        //now compute the plane intersection
        const PVector3 &n = m_ScaledNormals[tris[i] / 3];
        if(n.lengthsq() == 0) {
			// triangle and line coplanar
			// just project the triangle center to the line and hope for the best
            PVector3 ctr = (vtc[idx[0]].pos + vtc[idx[1]].pos + vtc[idx[2]].pos) * (1. / 3.);
            out.push_back(projToLine(ctr, pt, m_Dir));
            continue;
        }

        out.push_back(pt + m_Dir * (n * (vtc[idx[0]].pos - pt))); //intersection
    }// end outer for
    
    return out;
}
