#pragma once
namespace ParaEngine
{
#ifdef USE_DIRECTX_RENDERER
	typedef IDirect3DVertexBuffer9* VertexBufferDevicePtr_type;
	typedef IDirect3DIndexBuffer9* IndexBufferDevicePtr_type;
#elif defined(USE_OPENGL_RENDERER)
	typedef GLuint VertexBufferDevicePtr_type;
	typedef GLuint IndexBufferDevicePtr_type;
#else 
	typedef int32_t VertexBufferDevicePtr_type;
	typedef int32_t IndexBufferDevicePtr_type;
#endif

	/** cross platform vertex/index buffer. 
	* this is light weighted, use this like an int, and copy by value. 
	*/
	class ParaVertexBuffer
	{
	public:
		ParaVertexBuffer();
		~ParaVertexBuffer();
		
		enum EnumBufferType
		{
			BufferType_VertexBuffer = 0,
			BufferType_IndexBuffer,
			BufferType_MemoryBuffer,
		};
	public:
		VertexBufferDevicePtr_type GetDevicePointer();

		IndexBufferDevicePtr_type GetDevicePointerAsIndexBuffer();
		char* GetMemoryPointer();

		/** recreate the buffer based on the new size 
		* @param dwFormat: only used in DirectX, ignored in openGL
		* @param dwUsage: only used in DirectX, ignored in openGL
		*/
		bool CreateBuffer(uint32 nBufferSize, DWORD dwFormat = 0, DWORD dwUsage = 0, D3DPOOL dwPool = D3DPOOL_MANAGED);
		bool CreateIndexBuffer(uint32 nBufferSize, DWORD dwFormat = 0, DWORD dwUsage = 0);
		bool CreateMemoryBuffer(uint32 nBufferSize, DWORD dwFormat = 0, DWORD dwUsage = 0);

		/** release the buffer. Please note one must manually call this function. 
		The function will not be called when class is destroyed. 
		*/
		void ReleaseBuffer();

		/** lock and return a writable buffer into which one can fill the buffer. */
		bool Lock(void** ppData, uint32 offsetToLock = 0, uint32 sizeToLock = 0, DWORD dwFlag = 0);
		/** unlock the buffer */
		void Unlock();

		/** get the current buffer size. */
		uint32 GetBufferSize();

		/** boolean: test validity */
		bool IsValid() const;
		operator bool() const { return IsValid(); };
		bool operator !() const { return !IsValid(); };
		void RendererRecreated();
		void UploadMemoryBuffer(const char* pBuffer, int32 nBufSize = -1);
	protected:
		union {
			IndexBufferDevicePtr_type m_indexBuffer;
			VertexBufferDevicePtr_type m_vertexBuffer;
		};
		
		uint32 m_nBufferSize;
		// only valid between Lock and Unlock or type is BufferType_MemoryBuffer
		char* m_buffer;
		EnumBufferType m_bufferType;
	};

	/** in OpenGL, there is no different between vertex and index buffer. 
	in directX, vertex and index buffer has similar interface, but different interface type. so here we just wrap around ParaVertexBuffer. 
	*/
	class ParaIndexBuffer : public ParaVertexBuffer
	{
	public:
		bool CreateBuffer(uint32 nBufferSize, DWORD dwFormat = 0, DWORD dwUsage = 0);
		IndexBufferDevicePtr_type GetDevicePointer();
	};

	/** pure memory buffer, useful for filling in another thread and then upload to a real vertex buffer in the render thread. 
	*/
	class ParaMemoryBuffer : public ParaVertexBuffer
	{
	public:
		bool CreateBuffer(uint32 nBufferSize, DWORD dwFormat = 0, DWORD dwUsage = 0);
		char* GetDevicePointer();
	};
	
}