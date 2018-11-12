
#include "mesh.h"
#include "hashutils.h"
#include "utils.h"
#include "debugging.h"
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

using namespace Pinocchio;

Mesh::Mesh() 
	: m_Scale(1.0)
	, m_Normalized(false)
	, m_ComputedNormal(false)
	, m_ToAdd(PVector3())
{
}

void Mesh::computeTopology()
{
    int i;
    for(i = 0; i < (int)m_Edges.size(); ++i)
        m_Edges[i].prev = (i - i % 3) + (i + 2) % 3;
    
    vector<map<int, int> > halfEdgeMap(m_Vertices.size());
    for(i = 0; i < (int)m_Edges.size(); ++i) {
        int v1 = m_Edges[i].vertex;
        int v2 = m_Edges[m_Edges[i].prev].vertex;

        m_Vertices[v1].edge = m_Edges[m_Edges[i].prev].prev; //assign the vertex' edge
        
        if(halfEdgeMap[v1].count(v2)) {
            Debugging::out() << "Error: duplicate edge detected: " << v1 << " to " << v2 << endl;
        }
        halfEdgeMap[v1][v2] = i;
        if(halfEdgeMap[v2].count(v1)) {
            int twin = halfEdgeMap[v2][v1];
            m_Edges[twin].twin = i;
            m_Edges[i].twin = twin;
        }
    }
}

void Mesh::computeVertexNormals()
{
	if (m_ComputedNormal) return;
    for(int i = 0; i < (int)m_Vertices.size(); ++i)
        m_Vertices[i].normal = PVector3();
    for(int i = 0; i < (int)m_Edges.size(); i += 3) {
        int i1 = m_Edges[i].vertex;
        int i2 = m_Edges[i + 1].vertex;
        int i3 = m_Edges[i + 2].vertex;
        PVector3 normal = ((m_Vertices[i2].pos - m_Vertices[i1].pos) % (m_Vertices[i3].pos - m_Vertices[i1].pos)).normalize();
        m_Vertices[i1].normal += normal;
        m_Vertices[i2].normal += normal;
        m_Vertices[i3].normal += normal;
    }
    for(int i = 0; i < (int)m_Vertices.size(); ++i)
        m_Vertices[i].normal = m_Vertices[i].normal.normalize();
	m_ComputedNormal = true;
}

void Mesh::normalizeBoundingBox()
{
	if (m_Normalized) return;
    vector<PVector3> positions;
    for(int i = 0; i < (int)m_Vertices.size(); ++i) {
        positions.push_back(m_Vertices[i].pos);
    }
    Rect3 boundingBox = Rect3(positions.begin(), positions.end());
    double cscale = .9 / boundingBox.getSize().accumulate(ident<double>(), maximum<double>());
    PVector3 ctoAdd = PVector3(0.5, 0.5, 0.5) - boundingBox.getCenter() * cscale;
    for(int i = 0; i < (int)m_Vertices.size(); ++i) {
        m_Vertices[i].pos = ctoAdd + m_Vertices[i].pos * cscale;
    }
	m_ToAdd = ctoAdd + cscale * m_ToAdd;
	m_Scale *= cscale;
	m_Normalized = true;
}

void Mesh::sortEdges()
{
    //TODO: implement for when reading files other than obj
}

struct MFace
{
    MFace(int v1, int v2, int v3)
    {
        v[0] = v1; v[1] = v2; v[2] = v3;
        std::sort(v, v + 3);
    }
    
    bool operator<(const MFace &f) const { return lexicographical_compare(v, v + 3, f.v, f.v + 3); }
    int v[3];
};

void Mesh::fixDupFaces()
{
    int i;
    map<MFace, int> faces;
    for(i = 0; i < (int)m_Edges.size(); i += 3) {
        MFace current(m_Edges[i].vertex, m_Edges[i + 1].vertex, m_Edges[i + 2].vertex);
        
        if(faces.count(current)) {
            int oth = faces[current];
            if(oth == -1) {
                faces[current] = i;
                continue;
            }
            faces[current] = -1;
            int newOth = m_Edges.size() - 6;
            int newCur = m_Edges.size() - 3;
            
            m_Edges[oth] = m_Edges[newOth];
            m_Edges[oth + 1] = m_Edges[newOth + 1];
            m_Edges[oth + 2] = m_Edges[newOth + 2];
            m_Edges[i] = m_Edges[newCur];
            m_Edges[i + 1] = m_Edges[newCur + 1];
            m_Edges[i + 2] = m_Edges[newCur + 2];
            
            MFace newOthF(m_Edges[newOth].vertex, m_Edges[newOth + 1].vertex, m_Edges[newOth + 2].vertex);
            faces[newOthF] = newOth;
            
            m_Edges.resize(m_Edges.size() - 6);
            i -= 3;
        }
        else {
            faces[current] = i;
        }
    }

    //scan for unreferenced vertices and get rid of them
    set<int> referencedVerts;
    for(i = 0; i < (int)m_Edges.size(); ++i) {
        if(m_Edges[i].vertex < 0 || m_Edges[i].vertex >= (int)m_Vertices.size())
            continue;
        referencedVerts.insert(m_Edges[i].vertex);
    }

    vector<int> newIdxs(m_Vertices.size(), -1);
    int curIdx = 0;
    for(i = 0; i < (int)m_Vertices.size(); ++i) {
        if(referencedVerts.count(i))
            newIdxs[i] = curIdx++;
    }

    for(i = 0; i < (int)m_Edges.size(); ++i) {
        if(m_Edges[i].vertex < 0 || m_Edges[i].vertex >= (int)m_Vertices.size())
            continue;
        m_Edges[i].vertex = newIdxs[m_Edges[i].vertex];
    }
    for(i = 0; i < (int)m_Vertices.size(); ++i) {
        if(newIdxs[i] > 0)
            m_Vertices[newIdxs[i]] = m_Vertices[i];
    }
    m_Vertices.resize(referencedVerts.size());
}


class StlVtx : public PVector3
{
public:
    StlVtx(double x, double y, double z) : PVector3(x, y, z) {}
    bool operator==(const StlVtx &o) const { return (*this)[0] == o[0] && (*this)[1] == o[1] && (*this)[2] == o[2]; }
    bool operator<(const StlVtx &o) const { return (*this)[0] < o[0] || ((*this)[0] == o[0] &&
        ((*this)[1] < o[1] || ((*this)[1] == o[1] && (*this)[2] < o[2]))); }
};

//MAKE_HASH(StlVtx, return (int)(p[0] * 100000. + p[1] * 200000. + p[2] * 400000.););

// specialize the std::hash template for our StlVtx - type.
namespace std {
	template <>
	struct hash<StlVtx>
	{
		std::size_t operator()(const StlVtx& p) const
		{
			return (int)(p[0] * 100000. + p[1] * 200000. + p[2] * 400000.);
		}
	};

}

bool Mesh::isConnected() const
{
    if(m_Vertices.size() == 0)
        return false;

    vector<bool> reached(m_Vertices.size(), false);
    vector<int> todo(1, 0);
    reached[0] = true;
    unsigned int reachedCount = 1;

    int inTodo = 0;
    while(inTodo < (int)todo.size()) {
        int startEdge = m_Vertices[todo[inTodo++]].edge;
        int curEdge = startEdge;
        do {
            curEdge = m_Edges[m_Edges[curEdge].prev].twin; //walk around
            int vtx = m_Edges[curEdge].vertex;
            if(!reached[vtx]) {
                reached[vtx] = true;
                ++reachedCount;
                todo.push_back(vtx);
            }
        } while(curEdge != startEdge);        
    }

    return reachedCount == m_Vertices.size();
}

#define CHECK(pred) { if(!(pred)) { Debugging::out() << "Mesh integrity error: " #pred << " in line " << __LINE__ << endl; return false; } }

bool Mesh::integrityCheck() const
{
    int i;
    int vs = m_Vertices.size();
    int es = m_Edges.size();
    
    if(vs == 0) { //if there are no vertices, shouldn't be any edges either
        CHECK(es == 0);
        return true;
    }
    
    CHECK(es > 0); //otherwise, there should be edges
    
    //check index range validity
    for(i = 0; i < vs; ++i) {
        CHECK(m_Vertices[i].edge >= 0);
        CHECK(m_Vertices[i].edge < es);
    }
    
    for(i = 0; i < es; ++i) {
        CHECK(m_Edges[i].vertex >= 0 && m_Edges[i].vertex < vs);
        CHECK(m_Edges[i].prev >= 0 && m_Edges[i].prev < es);
        CHECK(m_Edges[i].twin >= 0 && m_Edges[i].twin < es);
    }
    
    //check basic edge and vertex relationships
    for(i = 0; i < es; ++i) {
        CHECK(m_Edges[i].prev != i); //no loops
        CHECK(m_Edges[m_Edges[m_Edges[i].prev].prev].prev == i); //we have only triangles
        CHECK(m_Edges[i].twin != i); //no self twins
        CHECK(m_Edges[m_Edges[i].twin].twin == i); //twins are valid

        CHECK(m_Edges[m_Edges[i].twin].vertex == m_Edges[m_Edges[i].prev].vertex); //twin's vertex and prev's vertex should be the same
    }
    
    for(i = 0; i < vs; ++i) {
        CHECK(m_Edges[m_Edges[m_Vertices[i].edge].prev].vertex == i); //make sure the edge pointer is correct
    }
    
    //check that the edges around a vertex form a cycle -- by counting
    vector<int> edgeCount(vs, 0); //how many edges adjacent to each vertex
    for(i = 0; i < es; ++i)
        edgeCount[m_Edges[i].vertex] += 1;
    
    for(i = 0; i < vs; ++i) {
        int startEdge = m_Vertices[i].edge;
        int curEdge = startEdge;
        int count = 0;
        do {
            curEdge = m_Edges[m_Edges[curEdge].prev].twin; //walk around
            ++count;
        } while(curEdge != startEdge && count <= edgeCount[i]);
        CHECK(count == edgeCount[i] && "Non-manifold vertex found");
    }
    
    return true;
}
