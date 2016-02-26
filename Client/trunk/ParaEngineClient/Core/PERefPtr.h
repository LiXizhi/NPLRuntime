#pragma once
/** for backward compatibilities. ref_ptr is now preferred way. */
#define asset_ptr  ref_ptr

namespace ParaEngine
{
	/**
	* The ref_ptr class template stores a pointer to a dynamically allocated (AssetEntity|CRefCounted|BaseAsset) derived object.
	* Every ref_ptr meets the CopyConstructible and Assignable requirements of the C++ Standard Library,
	* and so can be used in standard library containers. this is similar to boost::shared_ptr, except that it mainly works with AssetEntity.
	* i.e. use in a class like this: ref_ptr<TextureEntity> p;
	*/
	template<class T> struct ref_ptr
	{
	protected:
		// contained pointer
		T * px;
	public:
		typedef T element_type;
		typedef T value_type;
		typedef T * pointer;

		ref_ptr() : px(0)
		{
		}

		~ref_ptr()
		{
			if (px != 0)
				px->Release();
		}

		template<class Y>
		explicit ref_ptr(Y * p) : px(p) // Y must be complete
		{
			if (px != 0)
				px->addref();
		}

		ref_ptr(const ref_ptr & r) : px(r.px) // never throws
		{
			if (px != 0)
				px->addref();
		}

		ref_ptr & operator=(const ref_ptr & r) // never throws
		{
			if (px != r.px)
			{
				if (px != 0)
					px->Release();
				px = r.px;
				if (px != 0)
					px->addref();
			}
			return *this;
		}

		template<class Y>
		ref_ptr & operator=(Y* r) // never throws
		{
			if (px != r)
			{
				if (px != 0)
					px->Release();
				px = r;
				if (px != 0)
					px->addref();
			}
			return *this;
		}


		T& operator* () const // never throws
		{
			PE_ASSERT(px != 0);
			return *px;
		}

		T * operator-> () const // never throws
		{
			PE_ASSERT(px != 0);
			return px;
		}

		T * get() const // never throws
		{
			return px;
		}

		operator bool() const
		{
			return px != 0;
		}

		bool operator! () const // never throws
		{
			return px == 0;
		}

		int use_count() const // never throws
		{
			return (px != 0) ? px->GetRefCount() : 0;
		}

		bool unique() const // nothrow
		{
			return use_count() == 1;
		}

		void reset(T* r = 0)
		{
			if (px != r)
			{
				if (px != 0)
					px->Release();
				px = r;
				if (px != 0)
					px->addref();
			}
		}
	};

	template<class T, class U> inline bool operator==(ref_ptr<T> const & a, ref_ptr<U> const & b)
	{
		return a.get() == b.get();
	}

	template<class T, class U> inline bool operator!=(ref_ptr<T> const & a, ref_ptr<U> const & b)
	{
		return a.get() != b.get();
	}

	template<class T, class U> inline bool operator==(ref_ptr<T> const & a, U const * b)
	{
		return a.get() == b;
	}

	template<class T, class U> inline bool operator!=(ref_ptr<T> const & a, U const * b)
	{
		return a.get() != b;
	}
}