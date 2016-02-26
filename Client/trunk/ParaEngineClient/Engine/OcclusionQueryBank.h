#pragma once

#include <vector>
namespace ParaEngine
{
	// Little struct used to store queries and their outstanding state
	struct LatentQueryElement
	{
		LPDIRECT3DQUERY9 query;
		enum 
		{
			UNUSED = 0,
			ISSUEDBEGIN = 1,
			ISSUEDEND = 2,
		} state;

		/** user data associated with a specific query object. */
		void* pUserData;
		LatentQueryElement():query(NULL), state(UNUSED), pUserData(NULL){};
	};

	typedef std::vector<LatentQueryElement> QueryVector;

	/**
	This is a bank of occlusion queries.  It is meant to allow multiple occlusion
	queries to be used in conjunction with a rendering strategy that culls based
	on the results.  All calls into this class return immediately, so, it is possible
	for the call to GetLatestResults to return S_FALSE, indicating that there are
	no new results.

	(Background: An occlusion query returns the number of pixels that pass 
	z-testing; if greater than zero, you know an object is visible.  For more 
	information, look up D3DQUERYTYPE or D3DQUERYTYPE_OCCLUSION in MSDN.)

	Usage:

	Use just like a regular D3DQuery, replacing the following calls:

	LPDIRECT3DQUERY9        -> LatentOcclusionQueryBank
	GetData()                -> GetLatestResults()
	Issue(D3DISSUE_BEGIN)    -> BeginNextQuery()
	Issue(D3DISSUE_BEGIN)    -> EndNextQuery()
	Release()                -> Free()

	Remember to call Free()!

	*/
	class LatentOcclusionQueryBank
	{
	public:
		LatentOcclusionQueryBank( IDirect3DDevice9 *  pD3DDev);
		~LatentOcclusionQueryBank();
		HRESULT GetLatestResults(DWORD *count);
		HRESULT BeginNextQuery();
		HRESULT EndNextQuery();
		uint32 GetNumActiveQueries();

		
		/** start a new query*/
		HRESULT BeginNewQuery(void* pUserData = NULL);
		/** end a new query*/
		HRESULT EndNewQuery();
		/** wait until the first query result is available. and return the pixel count in count
		* @param pUserData: [out] user data when the query is issued.
		*/
		HRESULT WaitForFirstResult(DWORD *count, void** pUserData=NULL);
		/** check the first query result is available. and return immediately.
		* @param pUserData: [out] user data when the query is issued.
		*/
		HRESULT CheckForFirstResult(DWORD *count, void** pUserData=NULL);
		/** return true if there is unused query in the bank*/
		bool HasUnusedQuery();


		void Cleanup();
		bool IsValid();
		
	protected:
		bool m_bIsValid;
		QueryVector m_Queries;
		int m_head;

		/// the index of the first unused query in the bank
		int m_nFirstUnusedIndex;
		/// the index of the oldest query in the bank
		int m_nFirstQueryIndex;
	};
}

