#pragma once

#include <unordered_set>

namespace ParaEngine
{
/**
For shadow volume: indexed edge building
*/
class EdgeHash
{
public:
    EdgeHash();
    EdgeHash(int v0, int v1);

    unsigned int hashCode() const;
    bool operator==(const EdgeHash& rhs) const;

    DWORD m_v0; // indices
	DWORD m_v1; // indices
};

/**
the hash compare class for stdext hash_set used in shadow volume edge building
*/
class hash_compare_edge
{
public:
	/// parameters for hash table
	enum{	
	bucket_size = 4,	// 0 < bucket_size
	min_buckets = 128};	// min_buckets = 2 ^^ N, 0 < N
	
	size_t operator( )( const EdgeHash&  Key ) const
	{
		return Key.hashCode();
	}
	bool operator( )( const EdgeHash&  _Key1,const EdgeHash&  _Key2) const
	{
		return !(_Key1== _Key2);
	}
};

/**
a set of static function for edge building for shadow volume
*/
class CEdgeBuilder
{
public:
	CEdgeBuilder(void);
	~CEdgeBuilder(void);
public:
	static void AddEdge( std::unordered_set <EdgeHash, hash_compare_edge> & edgeTable, DWORD& dwNumEdges, WORD v0, WORD v1 );
};
}