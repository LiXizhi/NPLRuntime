#pragma once

/* this template is based on Nikolaus Gebhardt's Array Class in the "Irrlicht Engine". */

/** define this macro will make the array class boundary safe.*/
// #define BOUNDARY_SAFE
namespace ParaEngine
{
	

	/// Sinks an element into the heap.
	template<class T>
		inline void heapsink(T*array, int element, int max)
	{
		while ((element<<1) < max)	// there is a left child
		{
			int j = (element<<1);

			if (j+1 < max && array[j] < array[j+1])
				j = j+1;							// take right child

			if (array[element] < array[j])
			{
				T t = array[j];						// swap elements
				array[j] = array[element];
				array[element] = t;
				element = j;
			}
			else
				return;
		}
	}


	/// Sorts an array with size 'size' using heapsort.
	template<class T>
		inline void heapsort(T* array, int size)
	{
		// for heapsink we pretend this is not c++, where
		// arrays start with index 0. So we decrease the array pointer,
		// the maximum always +2 and the element always +1

		T* virtualArray = array - 1;
		int virtualSize = size + 2;
		int i;

		// build heap

		for (i=((size-1)/2); i>=0; --i)	
			heapsink(virtualArray, i+1, virtualSize-1);

		// sort array

		for (i=size-1; i>=0; --i)	
		{
			T t = array[0];
			array[0] = array[i];
			array[i] = t;
			heapsink(virtualArray, 1, i + 1);
		}
	}

	///	Self reallocating template array (like stl vector) with additional features.
	/** Some features are: Heap sorting, binary search methods, easier debugging.
	*/
	template <class T>
	class array
	{

	public:

		array()
			: data(0), used(0), allocated(0),
			free_when_destroyed(true), is_sorted(true)
		{
		}

		/// Constructs a array and allocates an initial chunk of memory.
		/// \param start_count: Amount of elements to allocate.
		array(DWORD start_count)
			: data(0), used(0), allocated(0),
			free_when_destroyed(true),	is_sorted(true)
		{
			reallocate(start_count);
		}


		/// Copy constructor
		array(const array<T>& other)
			: data(0)
		{
			*this = other;
		}



		/// Destructor. Frees allocated memory, if set_free_when_destroyed
		/// was not set to false by the user before.
		~array()
		{
			if (free_when_destroyed)
				delete [] data;
		}



		/// Reallocates the array, make it bigger or smaller.
		/// \param new_size: New size of array.
		void reallocate(DWORD new_size)
		{
			T* old_data = data;

			data = new T[new_size];
			allocated = new_size;

			int end = used < new_size ? used : new_size;
			for (int i=0; i<end; ++i)
				data[i] = old_data[i];

			if (allocated < used)
				used = allocated;

			delete [] old_data;
		}



		/// Adds an element at back of array. If the array is to small to 
		/// add this new element, the array is made bigger.
		/// \param element: Element to add at the back of the array.
		void push_back(const T& element)
		{
			if (used + 1 > allocated)
				reallocate(used * 2 +1);

			data[used++] = element;
			is_sorted = false;
		}



		/// Clears the array and deletes all allocated memory.
		void clear()
		{
			delete [] data;
			data = 0;
			used = 0;
			allocated = 0;
			is_sorted = true;
		}



		/// Sets pointer to new array, using this as new workspace.
		/// \param newPointer: Pointer to new array of elements.
		/// \param size: Size of the new array.
		void set_pointer(T* newPointer, DWORD size)
		{
			delete [] data;
			data = newPointer;
			allocated = size;
			used = size;
			is_sorted = false;
		}



		/// Sets if the array should delete the memory it used.
		/// \param f: If true, the array frees the allocated memory in its
		/// destructor, otherwise not. The default is true.
		void set_free_when_destroyed(bool f)
		{
			free_when_destroyed = f;
		}



		/// Sets the size of the array.
		/// \param usedNow: Amount of elements now used.
		void set_used(DWORD usedNow)
		{
			if (allocated < usedNow)
				reallocate(usedNow);

			used = usedNow;
		}



		/// Assignment operator
		void operator=(const array<T>& other)
		{
			if (data)
				delete [] data;

			//if (allocated < other.allocated)
			if (other.allocated == 0)
				data = 0;
			else
				data = new T[other.allocated];

			used = other.used;
			free_when_destroyed = other.free_when_destroyed;
			is_sorted = other.is_sorted;
			allocated = other.allocated;

			for (DWORD i=0; i<other.used; ++i)
				data[i] = other.data[i];
		}


		/// Direct access operator
		T& operator [](DWORD index)
		{
#ifdef BOUNDARY_SAFE
			if (index>=used)
				return; // access violation
#endif

				return data[index];
		}



		/// Direct access operator
		const T& operator [](DWORD index) const
		{
#ifdef BOUNDARY_SAFE
			if (index>=used)
				return; // access violation
#endif

				return data[index];
		}

		/// Gets last frame
		const T& getLast() const
		{
#ifdef BOUNDARY_SAFE
			if (!used)
				return; // access violation
#endif

				return data[used-1];
		}

		/// Gets last frame
		T& getLast()
		{
#ifdef BOUNDARY_SAFE
			if (!used)
				return; // access violation
#endif

				return data[used-1];
		}


		/// Returns a pointer to the array.
		/// \return Pointer to the array.
		T* pointer()
		{
			return data;
		}



		/// Returns a const pointer to the array.
		/// \return Pointer to the array.
		const T* const_pointer() const
		{
			return data;
		}



		/// Returns size of used array.
		/// \return Size of elements in the array.
		DWORD size() const
		{
			return used;
		}



		/// Returns amount memory allocated.
		/// \return Returns amount of memory allocated. The amount of bytes
		/// allocated would  be allocated_size() * sizeof(ElementsUsed);
		DWORD allocated_size() const
		{
			return allocated;
		}



		/// Returns true if array is empty
		/// \return True if the array is empty, false if not.
		bool empty() const
		{
			return used == 0;
		}



		/// Sorts the array using heapsort. There is no additional memory waste and
		/// the algorithm performs (O) n log n in worst case.
		void sort()
		{
			if (is_sorted || used<2)
				return;

			heapsort(data, used);
			is_sorted = true;
		}



		/// Performs a binary search for an element, returns -1 if not found.
		/// The array will be sorted before the binary search if it is not
		/// already sorted.
		/// \param element: Element to search for.
		/// \return Returns position of the searched element if it was found,
		/// otherwise -1 is returned.
		int binary_search(const T& element)
		{
			return binary_search(element, 0, used-1);
		}



		/// Performs a binary search for an element, returns -1 if not found.
		/// The array will be sorted before the binary search if it is not
		/// already sorted.
		/// \param element: Element to search for.
		/// \param left: First left index
		/// \param right: Last right index.
		/// \return Returns position of the searched element if it was found,
		/// otherwise -1 is returned.
		int binary_search(const T& element, int left, int right)
		{
			if (!used)
				return -1;

			sort();

			int m;

			do
			{
				m = (left+right)>>1;

				if (element < data[m])
					right = m - 1;
				else
					left = m + 1;

			} while((element < data[m] || data[m] < element) && left<=right);

			// this last line equals to:
			// " while((element != array[m]) && left<=right);"
			// but we only want to use the '<' operator.
			// the same in next line, it is "(element == array[m])"

			if (!(element < data[m]) && !(data[m] < element))
				return m;

			return -1;
		}


		/// Finds an element in linear time, which is very slow. Use
		/// binary_search for faster finding. Only works if =operator is implemented.
		/// \param element: Element to search for.
		/// \return Returns position of the searched element if it was found,
		/// otherwise -1 is returned.
		int linear_search(T& element)
		{
			for (DWORD i=0; i<used; ++i)
				if (!(element < data[i]) && !(data[i] < element))
					return (int)i;

			return -1;
		}


		/// Finds an element in linear time, which is very slow. Use
		/// binary_search for faster finding. Only works if =operator is implemented.
		/// \param element: Element to search for.
		/// \return Returns position of the searched element if it was found,
		/// otherwise -1 is returned.
		int linear_reverse_search(T& element)
		{
			for (int i=used-1; i>=0; --i)
				if (data[i] == element)
					return (int)i;

			return -1;
		}



		/// Erases an element from the array. May be slow, because all elements 
		/// following after the erased element have to be copied.
		/// \param index: Index of element to be erased.
		void erase(DWORD index)
		{
#ifdef BOUNDARY_SAFE
			if (index>=used || index<0)
				return; // access violation
#endif

				for (DWORD i=index+1; i<used; ++i)
					data[i-1] = data[i];

			--used;
		}


		/// Erases some elements from the array. may be slow, because all elements 
		/// following after the erased element have to be copied.
		/// \param index: Index of the first element to be erased.
		/// \param count: Amount of elements to be erased.
		void erase(DWORD index, int count)
		{
#ifdef BOUNDARY_SAFE
			if (index>=used || index<0 || count<1 || index+count>used)
				return; // access violation
#endif

				for (DWORD i=index+count; i<used; ++i)
					data[i-count] = data[i];

			used-= count;
		}


		/// Sets if the array is sorted
		void set_sorted(bool _is_sorted)
		{
			is_sorted = _is_sorted;
		}


	private:

		T* data;
		DWORD allocated;
		DWORD used;
		bool free_when_destroyed;
		bool is_sorted;
	};

}
