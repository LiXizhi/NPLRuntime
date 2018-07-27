
#include "mesh.h"
#include "hashutils.h"
#include "utils.h"
#include "debugging.h"
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

Mesh::Mesh(const string &file)
    : m_Scale(1.)
{
    int i;
#define OUT { m_Vertices.clear(); m_Edges.clear(); return; }
    ifstream obj(file.c_str());
    
    if(!obj.is_open()) {
        Debugging::out() << "Error opening file " << file << endl;
        return;
    }
    
    Debugging::out() << "Reading " << file << endl;
    
    if(file.length() < 4) {
        Debugging::out() << "I don't know what kind of file it is" << endl;
        return;
    }
    
    if(string(file.end() - 4, file.end()) == string(".obj"))
        readObj(obj);
    else if(string(file.end() - 4, file.end()) == string(".ply"))
        readPly(obj);        
    else if(string(file.end() - 4, file.end()) == string(".off"))
        readOff(obj);        
    else if(string(file.end() - 4, file.end()) == string(".gts"))
        readGts(obj);
    else if(string(file.end() - 4, file.end()) == string(".stl"))
        readStl(obj);        
    else {
        Debugging::out() << "I don't know what kind of file it is" << endl;
        return;
    }
    
    //reconstruct the rest of the information
    int verts = m_Vertices.size();
    
    if(verts == 0)
        return;
    
    for(i = 0; i < (int)m_Edges.size(); ++i) { //make sure all vertex indices are valid
        if(m_Edges[i].vertex < 0 || m_Edges[i].vertex >= verts) {
            Debugging::out() << "Error: invalid vertex index " << m_Edges[i].vertex << endl;
            OUT;
        }
    }
    
    fixDupFaces();
    
    computeTopology();
    
    if(integrityCheck())
        Debugging::out() << "Successfully read " << file << ": " << m_Vertices.size() << " vertices, " << m_Edges.size() << " edges" << endl;
    else
        Debugging::out() << "Somehow read " << file << ": " << m_Vertices.size() << " vertices, " << m_Edges.size() << " edges" << endl;
    
    computeVertexNormals();
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
            OUT;
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
    int i;
    for(i = 0; i < (int)m_Vertices.size(); ++i)
        m_Vertices[i].normal = PVector3();
    for(i = 0; i < (int)m_Edges.size(); i += 3) {
        int i1 = m_Edges[i].vertex;
        int i2 = m_Edges[i + 1].vertex;
        int i3 = m_Edges[i + 2].vertex;
        PVector3 normal = ((m_Vertices[i2].pos - m_Vertices[i1].pos) % (m_Vertices[i3].pos - m_Vertices[i1].pos)).normalize();
        m_Vertices[i1].normal += normal;
        m_Vertices[i2].normal += normal;
        m_Vertices[i3].normal += normal;
    }
    for(i = 0; i < (int)m_Vertices.size(); ++i)
        m_Vertices[i].normal = m_Vertices[i].normal.normalize();
}

void Mesh::normalizeBoundingBox()
{
    int i;
    vector<PVector3> positions;
    for(i = 0; i < (int)m_Vertices.size(); ++i) {
        positions.push_back(m_Vertices[i].pos);
    }
    Rect3 boundingBox = Rect3(positions.begin(), positions.end());
    double cscale = .9 / boundingBox.getSize().accumulate(ident<double>(), maximum<double>());
    PVector3 ctoAdd = PVector3(0.5, 0.5, 0.5) - boundingBox.getCenter() * cscale;
    for(i = 0; i < (int)m_Vertices.size(); ++i) {
        m_Vertices[i].pos = ctoAdd + m_Vertices[i].pos * cscale;
    }
    m_ToAdd = ctoAdd + cscale * m_ToAdd;
    m_Scale *= cscale;
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

void Mesh::readObj(istream &strm)
{
    int i;
    int lineNum = 0;
    while(!strm.eof()) {
        ++lineNum;
        
        vector<string> words = readWords(strm);
        
        if(words.size() == 0)
            continue;
        if(words[0][0] == '#') //comment
            continue;
        
        if(words[0].size() != 1) //unknown line
            continue;
        
        //deal with the line based on the first word
        if(words[0][0] == 'v') {
            if(words.size() != 4) {
                Debugging::out() << "Error on line " << lineNum << endl;
                OUT;
            }
            
            double x, y, z;
            sscanf(words[1].c_str(), "%lf", &x);
            sscanf(words[2].c_str(), "%lf", &y);
            sscanf(words[3].c_str(), "%lf", &z);
            
            m_Vertices.resize(m_Vertices.size() + 1);
            m_Vertices.back().pos = PVector3(x, y, z);
        }
        
        if(words[0][0] == 'f') {
            if(words.size() < 4 || words.size() > 15) {
                Debugging::out() << "Error on line " << lineNum << endl;
                OUT;
            }
            
            int a[16];
            for(i = 0; i < (int)words.size() - 1; ++i)
                sscanf(words[i + 1].c_str(), "%d", a + i);

            //swap(a[1], a[2]); //TODO:remove

            for(int j = 2; j < (int)words.size() - 1; ++j) {
                int first = m_Edges.size();
                m_Edges.resize(m_Edges.size() + 3);
                m_Edges[first].vertex = a[0] - 1;
                m_Edges[first + 1].vertex = a[j - 1] - 1;
                m_Edges[first + 2].vertex = a[j] - 1;
            }
        }
        
        //otherwise continue -- unrecognized line
    }
}

void Mesh::readPly(istream &strm)
{
    int i;
    int lineNum = 0;
    
    bool outOfHeader = false;
    int vertsLeft = -1;
    
    while(!strm.eof()) {
        ++lineNum;
        
        vector<string> words = readWords(strm);
        
        if(words.size() == 0)
            continue;
        if(words[0][0] == '#') //comment
            continue;
        
        if(!outOfHeader) { //look for end_header or verts
            if(words[0] == string("end_header")) {
                if(vertsLeft < 0) {
                    Debugging::out() << "Error: no vertex count in header" << endl;
                    OUT;
                }
                outOfHeader = true;
                continue;
            }
            if(words.size() < 3) //not "element vertex n"
                continue;
            if(words[0] == string("element") && words[1] == string("vertex")) {
                sscanf(words[2].c_str(), "%d", &vertsLeft);
            }
            continue;
        }
        
        //if there are verts left, current line is a vertex
        if(vertsLeft > 0) {
            --vertsLeft;
            if(words.size() < 3) {
                Debugging::out() << "Error on line " << lineNum << endl;
                OUT;
            }
            
            double x, y, z;
            sscanf(words[0].c_str(), "%lf", &x);
            sscanf(words[1].c_str(), "%lf", &y);
            sscanf(words[2].c_str(), "%lf", &z);
            
            m_Vertices.resize(m_Vertices.size() + 1);
            m_Vertices.back().pos = PVector3(-z, x, -y);
            continue;
        }
        
        //otherwise it's a face
        if(words.size() != 4) {
            Debugging::out() << "Error on line " << lineNum << endl;
            OUT;
        }
            
        int a[3];
        for(i = 0; i < 3; ++i)
            sscanf(words[i + 1].c_str(), "%d", a + i);
        
        int first = m_Edges.size();
        m_Edges.resize(m_Edges.size() + 3);
        for(i = 0; i < 3; ++i) {
            m_Edges[first + i].vertex = a[i]; //indices in file are 0-based
        }
        
        //otherwise continue -- unrecognized line
    }
}

void Mesh::readOff(istream &strm)
{
    int i;
    int lineNum = 0;
    
    bool outOfHeader = false;
    int vertsLeft = -1;
    
    while(!strm.eof()) {
        ++lineNum;
        
        vector<string> words = readWords(strm);
        
        if(words.size() == 0)
            continue;
        if(words[0][0] == '#') //comment
            continue;
        
        if(!outOfHeader) { //look for number of verts
            if(words.size() < 3) //not "vertices faces 0"
                continue;
            sscanf(words[0].c_str(), "%d", &vertsLeft);
            outOfHeader = true;
            continue;
        }
        
        //if there are verts left, current line is a vertex
        if(vertsLeft > 0) {
            --vertsLeft;
            if(words.size() < 3) {
                Debugging::out() << "Error on line " << lineNum << endl;
                OUT;
            }
            
            double x, y, z;
            sscanf(words[0].c_str(), "%lf", &x);
            sscanf(words[1].c_str(), "%lf", &y);
            sscanf(words[2].c_str(), "%lf", &z);
            
            m_Vertices.resize(m_Vertices.size() + 1);
            m_Vertices.back().pos = PVector3(x, y, z);
            
            continue;
        }
        
        //otherwise it's a face
        if(words.size() != 4) {
            Debugging::out() << "Error on line " << lineNum << endl;
            OUT;
        }
            
        int a[3];
        for(i = 0; i < 3; ++i)
            sscanf(words[i + 1].c_str(), "%d", a + i);
        
        int first = m_Edges.size();
        m_Edges.resize(m_Edges.size() + 3);
        for(i = 0; i < 3; ++i) {
            m_Edges[first + i].vertex = a[i]; //indices in file are 0-based
        }
        
        //otherwise continue -- unrecognized line
    }
}

void Mesh::readGts(istream &strm)
{
    int i;
    int lineNum = 0;
    
    bool outOfHeader = false;
    int vertsLeft = -1;
    int edgesLeft = -1;
    
    vector<pair<int, int> > fedges;
    
    while(!strm.eof()) {
        ++lineNum;
        
        vector<string> words = readWords(strm);
        
        if(words.size() == 0)
            continue;
        if(words[0][0] == '#') //comment
            continue;
        
        if(!outOfHeader) { //look for number of verts
            if(words.size() < 3) //not "vertices faces 0"
                continue;
            sscanf(words[0].c_str(), "%d", &vertsLeft);
            sscanf(words[1].c_str(), "%d", &edgesLeft);
            outOfHeader = true;
            continue;
        }
        
        //if there are verts left, current line is a vertex
        if(vertsLeft > 0) {
            --vertsLeft;
            if(words.size() < 3) {
                Debugging::out() << "Error on line " << lineNum << endl;
                OUT;
            }
            
            double x, y, z;
            sscanf(words[0].c_str(), "%lf", &x);
            sscanf(words[1].c_str(), "%lf", &y);
            sscanf(words[2].c_str(), "%lf", &z);
            
            m_Vertices.resize(m_Vertices.size() + 1);
            m_Vertices.back().pos = PVector3(-x, z, y);
            
            continue;
        }
        
        if(edgesLeft > 0) {
            --edgesLeft;
            if(words.size() != 2) {
                Debugging::out() << "Error (edge) on line " << lineNum << endl;
                OUT;
            }
            int e1, e2;
            sscanf(words[0].c_str(), "%d", &e1);
            sscanf(words[1].c_str(), "%d", &e2);
            fedges.push_back(make_pair(e1 - 1, e2 - 1));
            continue;
        }
        
        //otherwise it's a face
        if(words.size() != 3) {
            Debugging::out() << "Error on line " << lineNum << endl;
            OUT;
        }
            
        int a[3];
        for(i = 0; i < 3; ++i) {
            sscanf(words[i].c_str(), "%d", a + i);
            --(a[i]);  //indices in file are 1-based
        }
        
        int first = m_Edges.size();
        m_Edges.resize(m_Edges.size() + 3);
        for(i = 0; i < 3; ++i) {
            int ni = (i + 1) % 3;
            
            if(fedges[a[i]].first == fedges[a[ni]].first)
                m_Edges[first + i].vertex = fedges[a[i]].first;
            else if(fedges[a[i]].first == fedges[a[ni]].second)
                m_Edges[first + i].vertex = fedges[a[i]].first;
            else if(fedges[a[i]].second == fedges[a[ni]].first)
                m_Edges[first + i].vertex = fedges[a[i]].second;
            else if(fedges[a[i]].second == fedges[a[ni]].second)
                m_Edges[first + i].vertex = fedges[a[i]].second;
        }
        
        //otherwise continue -- unrecognized line
    }
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

void Mesh::readStl(istream &strm)
{
    int i;
    int lineNum = 0;
    
	unordered_map<StlVtx, int> vertexIdx;
    
    vector<int> lastIdxs;
    
    PVector3 normal;
    
    while(!strm.eof()) {
        ++lineNum;
        
        vector<string> words = readWords(strm);
        
        if(words.size() == 0)
            continue;
        if(words[0][0] == '#') //comment
            continue;
        
        if(words[0] == string("vertex")) {
            double x, y, z;
            sscanf(words[1].c_str(), "%lf", &x);
            sscanf(words[2].c_str(), "%lf", &y);
            sscanf(words[3].c_str(), "%lf", &z);
            
            StlVtx cur(y, z, x);
            int idx;
            
            if(vertexIdx.find(cur) == vertexIdx.end()) {
                idx = m_Vertices.size();
                vertexIdx[cur] = idx;
                m_Vertices.resize(m_Vertices.size() + 1);
                m_Vertices.back().pos = cur;
            }
            else
                idx = vertexIdx[cur];
            
            lastIdxs.push_back(idx);
            if(lastIdxs.size() > 3)
                lastIdxs.erase(lastIdxs.begin());
            continue;
        }
        
        if(words[0] == string("endfacet")) {
            if(lastIdxs[0] == lastIdxs[1] || lastIdxs[1] == lastIdxs[2] || lastIdxs[0] == lastIdxs[2]) {
                Debugging::out() << "Duplicate vertex in triangle" << endl;
                continue;
            }
            int first = m_Edges.size();
            m_Edges.resize(m_Edges.size() + 3);
            for(i = 0; i < 3; ++i) {
                m_Edges[first + i].vertex = lastIdxs[i]; //indices in file are 0-based
            }
            continue;
        }
        
        //otherwise continue -- unrecognized line
    }
}

void Mesh::writeObj(const string &filename) const
{
    int i;
    ofstream os(filename.c_str());
    
    for(i = 0; i < (int)m_Vertices.size(); ++i)
        os << "v " << m_Vertices[i].pos[0] << " " << m_Vertices[i].pos[1] << " " << m_Vertices[i].pos[2] << endl;

    for(i = 0; i < (int)m_Edges.size(); i += 3)
        os << "f " << m_Edges[i].vertex + 1 << " " << m_Edges[i + 1].vertex + 1 << " " << m_Edges[i + 2].vertex + 1 << endl;
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
