/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef INDEXER_H
#define INDEXER_H

#include "hashutils.h"
#include "vector.h"

template<class Node, int Dim>
class DumbIndexer
{ 
    public:
        typedef typename Node::Vec Vec;
    
        DumbIndexer() : m_Root(NULL) {}
    
        void setRoot(Node *n) 
        {
            m_Root = n;
        } 
        
        void preprocessIndex() {}
    
        Node *locate(const Vec &v) const
        {
            Node *out = m_Root;
            while(out->getChild(0)) {
                Vec center = out->getChild(0)->getRect().getHi();
                out = out->getChild(BitComparator<Dim>::less(center, v));
            }
            return out;
        }
    
    private:
        Node *m_Root;
};

extern PINOCCHIO_API unsigned int interLeaveLookupTable[32768];
extern PINOCCHIO_API unsigned int interLeave3LookupTable[1024];

inline unsigned int _lookup(const PVector2 &vec)
{
    return interLeaveLookupTable[int(vec[0] * 32767.999)] + (interLeaveLookupTable[int(vec[1] * 32767.999)] << 1);
}

inline unsigned int _lookup(const PVector3 &vec)
{
    return interLeave3LookupTable[int(vec[0] * 1023.999)] +
          (interLeave3LookupTable[int(vec[1] * 1023.999)] << 1) +
          (interLeave3LookupTable[int(vec[2] * 1023.999)] << 2);
}

template<class Node, int Dim>
class Indexer
{ 
public:
    typedef typename Node::Vec Vec;

    Indexer() : m_Root(NULL) {}

    void setRoot(Node *n) 
    {
        m_Root = n;
    } 
    
    void preprocessIndex() {}

    Node *locate(const Vec &v) const
    {
        Node *out = m_Root;
        unsigned int idx = _lookup(v);
        static const int mask = (1 << Dim) - 1;
        while(out->getChild(idx & mask)) {
            out = out->getChild(idx & mask);
            idx = idx >> Dim;
        }
        return out;
    }
private:
    Node *m_Root;
};

template<class Node, int Dim>
class ArrayIndexer
{ 
public:
    typedef typename Node::Vec Vec;

    ArrayIndexer() : m_Root(NULL) {}

    void setRoot(Node *n) 
    {
        m_Root = n;
    } 

    static const int bits = 16 - (16 % Dim);

    void preprocessIndex()
    {
        for(int i = 0; i < (1 << bits); ++i) {
            m_Table[i] = m_Root;
            int cur = i;
            int cnt = 0;
            static const int mask = (1 << Dim) - 1;
            while(m_Table[i]->getChild(0) && cnt < (bits / Dim)) {
                ++cnt;
                m_Table[i] = m_Table[i]->getChild(cur & mask);
                cur = cur >> Dim;
            }
        }
    }

    Node *locate(const Vec &v) const
    {
        unsigned int idx = _lookup(v);
        Node *out = m_Table[idx & ((1 << bits) - 1)];
        if(!out->getChild(0))
            return out;
        idx = idx >> bits;
        static const int mask = (1 << Dim) - 1;
        do {
            out = out->getChild(idx & mask);
            idx = idx >> Dim;
        } while(out->getChild(idx & mask));
        return out;
    }
private:
    Node *m_Root;
    Node *m_Table[(1 << bits)];
};

template<class Node, int Level> class HashIndex
{
public:
    HashIndex() { for(int i = 0; i < num; ++i) m_NodeMap[i] = make_pair(-1, (Node *)NULL); }

    void add(Node *node, unsigned int idx)
    {
        int idxx = (idx >> (Level - bits));
        //int idxx = idx % num;
        if(m_NodeMap[idxx].first == -1)
            m_NodeMap[idxx] = make_pair(idx, node);
    }

    Node *lookup(unsigned int idx) const
    {
        const pair<int, Node *> &p = m_NodeMap[(idx >> (Level - bits))];
        //const pair<int, Node *> &p = nodeMap[idx % num];
        return p.first == idx ? p.second : NULL;
    }

    static const int bits = 16;
    static const int num = (1 << bits);
    //static const int num = 75437;

private:
    pair<int, Node *> m_NodeMap[num];
};

template<class Node, int Dim>
class HashIndexer
{ 
    public:
        typedef typename Node::Vec Vec;

        HashIndexer() : m_Root(NULL) {}

        void setRoot(Node *n) 
        {
            m_Root = n;
        } 

        static const int bits = 16;
        static const int hlev = 22;

        void preprocessIndex()
        {
            for(int i = 0; i < (1 << bits); ++i) {
                m_Table[i] = m_Root;
                int cur = i;
                int cnt = 0;
                while(m_Table[i]->getChild(0) && cnt < (bits / 2)) {
                    ++cnt;
                    m_Table[i] = m_Table[i]->getChild(cur & 3);
                    cur = cur >> 2;
                }
            }
            add(m_Root, 0, 0);
        }

        Node *locate(const Vec &v) const
        {
            unsigned int idx = _lookup(v);
            Node *out = m_Table[idx & ((1 << bits) - 1)];
            if(!out->getChild(0))
                return out;
            Node *n = m_hNodes.lookup(idx & ((1 << hlev) - 1));
            if(!n)
                idx = idx >> bits;
            else {
                out = n;
                if(!out->getChild(0))
                    return out;
                idx = idx >> hlev;
            }
            do {
                out = out->getChild(idx & 3);
                idx = idx >> 2;
            } while(out->getChild(idx & 3));
            return out;
        }
    private:
        unsigned int getIndex(Node *n) const
        {
            if(n == m_Root)
                return 0;
            int level = getLevel(n);
            return (n->getChildIndex() << (2 * level - 2)) + getIndex(n->getParent());
        }
        int getLevel(Node *n) const
        {
            if(n == m_Root)
                return 0;
            return 1 + getLevel(n->getParent());
        }
        
        void add(Node *cur, int level, unsigned int idx)
        {
            if(level == hlev) {
                m_hNodes.add(cur, idx);
                return;
            }
            if(cur->getChild(0) == NULL)
                return;
            for(int i = 0; i < 4; ++i)
                add(cur->getChild(i), level + 2, idx + (i << level));
        }

        Node *m_Root;
        Node *m_Table[(1 << bits)];
        HashIndex<Node, hlev> m_hNodes;
};
#endif
