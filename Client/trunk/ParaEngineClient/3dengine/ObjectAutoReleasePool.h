#pragma once

namespace ParaEngine
{
	/** when base object is created it has zero reference, and is usually added to auto release pool to be removed in the next frame.
	* when detach a base object from a node, the node is usually added to auto release pool as well, 
	* so that we may attach it again to another node before end of frame. 
	* @note: Objects in the pools are released, not reused. Some pool may allow reuse or caching of pool objects, like EntityPool or VectorPool,etc. 
	*/
	class CObjectAutoReleasePool
	{
	public:
		static CObjectAutoReleasePool* GetInstance();
		static void DestoryInstance();

		CObjectAutoReleasePool();
		virtual ~CObjectAutoReleasePool();

		/**
		* Add a given object to this pool. it will addref each time each function is called.
		*
		* The same object may be added several times to the same pool; When the
		* pool is destructed, the object's Release() method will be called
		* for each time it was added.
		*/
		void AddObject(CRefCounted *object);

		/**
		* Clear the auto release pool.
		* Release() will be called for each time the managed object is added to the pool.
		*/
		void clear();

		/**
		* Checks whether the pool contains the specified object.
		*/
		bool contains(CRefCounted* object) const;

		/**
		* Dump the objects that are put into autorelease pool. It is used for debugging.
		*
		* The result will look like:
		* Object pointer address     object id     reference count
		*
		*/
		void dump();

	private:
		/** all objects in the pool */
		std::vector<CRefCounted*> m_object_pool;

	protected:
		static CObjectAutoReleasePool* s_singleInstance;

	};
}

