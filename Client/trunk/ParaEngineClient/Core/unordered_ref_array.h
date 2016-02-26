#pragma once
#include <vector>

/** similar to unordered array except that it will automatically call addref/Release when obj is added/removed. 
* it has fast random access, insertion, and deletion. 
* This class is often used to keep track of a collection of CBaseObject or other reference counted objects, such as child nodes. 
* e.g.
* typedef unordered_ref_array<CBaseObject*> CChildObjectList_Type;
* typedef unordered_ref_array<CRefCounted*> CChildObjectList_Type;
*/
template <typename T>
class unordered_ref_array
{
public:
	// ------------------------------------------
	// Iterators
	// ------------------------------------------
	typedef vector<T>  base_class_type;
	typedef typename base_class_type::iterator iterator;
	typedef typename base_class_type::const_iterator const_iterator;
					 
	typedef typename base_class_type::reverse_iterator reverse_iterator;
	typedef typename base_class_type::const_reverse_iterator const_reverse_iterator;

	iterator begin() { return m_data.begin(); }
	const_iterator begin() const { return m_data.begin(); }

	iterator end() { return m_data.end(); }
	const_iterator end() const { return m_data.end(); }

	const_iterator cbegin() const { return m_data.cbegin(); }
	const_iterator cend() const { return m_data.cend(); }

	reverse_iterator rbegin() { return m_data.rbegin(); }
	const_reverse_iterator rbegin() const { return m_data.rbegin(); }

	reverse_iterator rend() { return m_data.rend(); }
	const_reverse_iterator rend() const { return m_data.rend(); }

	const_reverse_iterator crbegin() const { return m_data.crbegin(); }
	const_reverse_iterator crend() const { return m_data.crend(); }

	/** Constructor */
	unordered_ref_array<T>()
		: m_data()
	{
	}

	/** Constructor with a capacity */
	explicit unordered_ref_array<T>(size_t capacity)
		: m_data()
	{
		reserve(capacity);
	}

	/** Destructor */
	~unordered_ref_array<T>()
	{
		clear();
	}

	/** Copy constructor */
	unordered_ref_array<T>(const unordered_ref_array<T>& other)
	{
		m_data = other.m_data;
		addRefForAllObjects();
	}

	/** Move constructor */
	unordered_ref_array<T>(unordered_ref_array<T>&& other)
	{
		m_data = std::move(other._data);
	}

	/** Copy assignment operator */
	unordered_ref_array<T>& operator=(const unordered_ref_array<T>& other)
	{
		if (this != &other) {
			clear();
			m_data = other.m_data;
			addRefForAllObjects();
		}
		return *this;
	}

	/** Move assignment operator */
	unordered_ref_array<T>& operator=(unordered_ref_array<T>&& other)
	{
		if (this != &other) {
			clear();
			m_data = std::move(other._data);
		}
		return *this;
	}

	/** @brief Request a change in capacity
	*  @param capacity Minimum capacity for the array.
	*         If n is greater than the current array capacity,
	*         the function causes the container to reallocate its storage increasing its capacity to n (or greater).
	*/
	void reserve(size_t n)
	{
		m_data.reserve(n);
	}

	/** @brief Returns the size of the storage space currently allocated for the array, expressed in terms of elements.
	*  @note This capacity is not necessarily equal to the array size.
	*        It can be equal or greater, with the extra space allowing to accommodate for growth without the need to reallocate on each insertion.
	*  @return The size of the currently allocated storage capacity in the array, measured in terms of the number elements it can hold.
	*/
	size_t capacity() const
	{
		return m_data.capacity();
	}

	/** @brief Returns the number of elements in the vector.
	*  @note This is the number of actual objects held in the vector, which is not necessarily equal to its storage capacity.
	*  @return The number of elements in the container.
	*/
	size_t size() const
	{
		return  m_data.size();
	}

	/** @brief Returns whether the vector is empty (i.e. whether its size is 0).
	*  @note This function does not modify the container in any way. To clear the content of a vector, see Vector<T>::clear.
	*/
	bool empty() const
	{
		return m_data.empty();
	}

	/** Returns the maximum number of elements that the vector can hold. */
	size_t max_size() const
	{
		return m_data.max_size();
	}

	/** Returns index of a certain object, return UINT_MAX if doesn't contain the object */
	size_t getIndex(T object) const
	{
		auto iter = std::find(m_data.begin(), m_data.end(), object);
		if (iter != m_data.end())
			return iter - m_data.begin();

		return -1;
	}

	/** @brief Find the object in the vector.
	*  @return Returns an iterator to the first element in the range [first,last) that compares equal to val.
	*          If no such element is found, the function returns last.
	*/
	const_iterator find(T object) const
	{
		return std::find(m_data.begin(), m_data.end(), object);
	}

	iterator find(T object)
	{
		return std::find(m_data.begin(), m_data.end(), object);
	}

	/** Returns the element at position 'index' in the vector. */
	T at(size_t index) const
	{
		PE_ASSERT(index >= 0 && index < size());
		return m_data[index];
	}

	/** Returns the first element in the vector. */
	T front() const
	{
		return m_data.front();
	}

	/** Returns the last element of the vector. */
	T back() const
	{
		return m_data.back();
	}

	/** Returns a Boolean value that indicates whether object is present in vector. */
	bool contains(T object) const
	{
		return(std::find(m_data.begin(), m_data.end(), object) != m_data.end());
	}

	/** Returns true if the two vectors are equal */
	bool equals(const unordered_ref_array<T> &other)
	{
		size_t s = this->size();
		if (s != other.size())
			return false;

		for (size_t i = 0; i < s; i++)
		{
			if (this->at(i) != other.at(i))
			{
				return false;
			}
		}
		return true;
	}

	// Adds objects

	/** @brief Adds a new element at the end of the vector, after its current last element.
	*  @note This effectively increases the container size by one,
	*        which causes an automatic reallocation of the allocated storage space
	*        if -and only if- the new vector size surpasses the current vector capacity.
	*/
	void push_back(T object)
	{
		PE_ASSERT(object != nullptr);
		m_data.push_back(object);
		object->addref();
	}

	/** Push all elements of an existing vector to the end of current vector. */
	void push_back(const unordered_ref_array<T>& other)
	{
		for (const auto &obj : other) {
			m_data.push_back(obj);
			obj->addref();
		}
	}

	/** @brief Insert a certain object at a certain index
	*  @note The vector is extended by inserting new elements before the element at the specified 'index',
	*        effectively increasing the container size by the number of elements inserted.
	*        This causes an automatic reallocation of the allocated storage space
	*        if -and only if- the new vector size surpasses the current vector capacity.
	*/
	void insert(size_t index, T object)
	{
		PE_ASSERT(index >= 0 && index <= size());
		PE_ASSERT(object != nullptr);
		m_data.insert((std::begin(m_data) + index), object);
		object->addref();
	}

	// Removes Objects

	/** Removes the last element in the vector,
	*  effectively reducing the container size by one, decrease the reference count of the deleted object.
	*/
	void pop_back()
	{
		PE_ASSERT(!m_data.empty());
		auto last = m_data.back();
		m_data.pop_back();
		last->Release();
	}

	/** @brief Remove a certain object in Vector.
	*  @param object The object to be removed.
	*  @param removeAll Whether to remove all elements with the same value.
	*                   If its value is 'false', it will just erase the first occurrence.
	*/
	void eraseObject(T object, bool removeAll = false)
	{
		PE_ASSERT(object != nullptr);

		if (removeAll)
		{
			for (auto iter = m_data.begin(); iter != m_data.end();)
			{
				if ((*iter) == object)
				{
					iter = m_data.erase(iter);
					object->Release();
				}
				else
				{
					++iter;
				}
			}
		}
		else
		{
			auto iter = std::find(m_data.begin(), m_data.end(), object);
			if (iter != m_data.end())
			{
				m_data.erase(iter);
				object->Release();
			}
		}
	}

	/** @brief Removes from the vector with an iterator.
	*  @param position Iterator pointing to a single element to be removed from the vector.
	*  @return An iterator pointing to the new location of the element that followed the last element erased by the function call.
	*          This is the container end if the operation erased the last element in the sequence.
	*/
	iterator erase(iterator position)
	{
		PE_ASSERT(position >= m_data.begin() && position < m_data.end());
		(*position)->Release();
		size_t nIndex = position - m_data.begin();
		// move the last element to current position. 
		*position = m_data.back();
		m_data.pop_back();
		
		return !m_data.empty() ? (m_data.begin()+nIndex) : m_data.end();
	}

	/** @brief Removes from the vector with a range of elements (  [first, last)  ).
	*  @param first The beginning of the range
	*  @param last The end of the range, the 'last' will not used, it's only for indicating the end of range.
	*  @return An iterator pointing to the new location of the element that followed the last element erased by the function call.
	*          This is the container end if the operation erased the last element in the sequence.
	*/
	iterator erase(iterator first, iterator last)
	{
		for (auto iter = first; iter != last; ++iter)
		{
			(*iter)->Release();
		}

		return m_data.erase(first, last);
	}

	/** @brief Removes from the vector with an index.
	*  @param index The index of the element to be removed from the vector.
	*  @return An iterator pointing to the new location of the element that followed the last element erased by the function call.
	*          This is the container end if the operation erased the last element in the sequence.
	*/
	iterator erase(size_t index)
	{
		PE_ASSERT(!m_data.empty() && index >= 0 && index < size());
		auto it = std::next(begin(), index);
		(*it)->Release();
		*it = m_data.back();
		m_data.pop_back();
		return !m_data.empty() ? (m_data.begin()+index) : m_data.end();
	}

	/** @brief Removes all elements from the vector (which are destroyed), leaving the container with a size of 0.
	*  @note All the elements in the vector will be Released (reference count will be decreased).
	*/
	void clear()
	{
		for (auto it = std::begin(m_data); it != std::end(m_data); ++it) {
			(*it)->Release();
		}
		m_data.clear();
	}

	// Rearranging Content

	/** Swap two elements */
	void swap(T object1, T object2)
	{
		size_t idx1 = getIndex(object1);
		size_t idx2 = getIndex(object2);

		PE_ASSERT(idx1 >= 0 && idx2 >= 0);

		std::swap(m_data[idx1], m_data[idx2]);
	}

	/** Swap two elements with certain indexes */
	void swap(size_t index1, size_t index2)
	{
		PE_ASSERT(index1 >= 0 && index1 < size() && index2 >= 0 && index2 < size());

		std::swap(m_data[index1], m_data[index2]);
	}

	/** Replace object at index with another object. */
	void replace(size_t index, T object)
	{
		PE_ASSERT(index >= 0 && index < size());
		PE_ASSERT(object != nullptr);

		m_data[index]->Release();
		m_data[index] = object;
		object->addref();
	}

	/** reverses the vector */
	void reverse()
	{
		std::reverse(std::begin(m_data), std::end(m_data));
	}

	/** Shrinks the vector so the memory footprint corresponds with the number of items */
	void shrink_to_fit()
	{
		m_data.shrink_to_fit();
	}

protected:

	/** addrefs all the objects in the vector */
	void addRefForAllObjects()
	{
		for (const auto &obj : m_data) {
			obj->addref();
		}
	}

	base_class_type m_data;
};

