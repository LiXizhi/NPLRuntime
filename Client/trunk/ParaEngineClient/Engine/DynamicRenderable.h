#pragma once

namespace ParaEngine
{
	/**
	* a base class for rendering objects using dynamic buffer from memory data. 
	* it does not create vertex or index buffer, instead it just feeds raw triangles into the dynamic buffer
	* Usage: derive your class or keep a member of this class. 
	*  - call PrepareBuffers()
	*  - fill or update in the the vertex and index buffer data in memory by directly manipulating m_Positons, m_indices, etc. inside lock/unlock pairs. 
	*  - on each render frame, call DrawPrimitive or DrawIndexedPrimitive.
	* Note: Dynamic renderable object uses system memory to store vertex and index buffer, making it extremely simple to update the vertex per frame.
	*  however, the performance is not good as hardware buffers. CDynamicRenderable is therefore only used for per vertex animated objects, and objects in editing mode, such as an editable voxel mesh. 
	*/
	class CDynamicRenderable
	{
	public:
		CDynamicRenderable(void);
		virtual ~CDynamicRenderable(void);

		/* Renders a sequence of nonindexed, geometric primitives of the specified type from 
		the current set of vertex data */
		HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex,	UINT PrimitiveCount);

		/** Based on indexing, renders the specified geometric primitive into an array of vertices.
		*/
		HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE Type,	INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,	UINT StartIndex,UINT PrimitiveCount);

		/** call this function when the vertex or index buffer size changes. 
		* @param nVertexCount: size of the vertex buffer
		* @param nIndexCount: size of the index buffer, if 0 index buffer is not used. 
		*/
		void PrepareBuffers(int nVertexCount, int nIndexCount);

		/** it locks everything for the caller to prevent simultaneous access. */
		bool lock();
		/** it unlocks everything for the caller to prevent simultaneous access. */
		bool unlock();

	public:
		// current vertex format: current only POS|Normal|UV is supported. 
		int m_nFVF;
		int m_nVertexBufSize;
		int m_nIndexBufSize;
		//
		// Vertex data
		//
		vector <Vector3> m_Positions;
		vector <Vector3> m_Normals;
		vector <DWORD> m_Colors;
		vector <Vector2> m_UVs;

		// index buffer
		vector <unsigned short> m_indices;
	};

}
