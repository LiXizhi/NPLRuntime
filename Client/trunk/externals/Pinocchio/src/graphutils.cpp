#include "graphutils.h"
#include "debugging.h"

#define CHECK(pred) { if(!(pred)) { Debugging::out() << "Graph integrity error: " #pred << " in line " << __LINE__ << endl; return false; } }

bool PtGraph::integrityCheck() const
{
    CHECK(verts.size() == edges.size());
    
    int i, j, k;
    for(i = 0; i < (int)edges.size(); ++i) {
        for(j = 0; j < (int)edges[i].size(); ++j) {
            int cur = edges[i][j];
            CHECK(cur >= 0);
            CHECK(cur < (int)edges.size());
            CHECK(cur != i); //no self edges
            
            vector<int>::const_iterator it = find(edges[cur].begin(), edges[cur].end(), i);
            CHECK(it != edges[cur].end());
            
            for(k = 0; k < j; ++k) //duplicates
                CHECK(cur != edges[i][k]);
        }
    }
    
    return true;
}

ShortestPather::ShortestPather(const PtGraph &g, int root)
{
    int sz = g.verts.size();
    priority_queue<Inf> todo;
    vector<bool> done(sz, false);
    m_Prev.resize(sz, -1);
    m_Dist.resize(sz, -1);
            
    todo.push(Inf(0., root, -1));
    while(!todo.empty())
    {
        Inf cur = todo.top();
        todo.pop();
        if(done[cur.node])
            continue;
        done[cur.node] = true;
        m_Prev[cur.node] = cur.prev;
        m_Dist[cur.node] = cur.dist;
                
        const vector<int> &e = g.edges[cur.node];
        for(int i = 0; i < (int)e.size(); ++i) {
            if(!done[e[i]]) {
                double dist = cur.dist + (g.verts[cur.node] - g.verts[e[i]]).length();
                todo.push(Inf(dist, e[i], cur.node));
            }
        }
    }
}
