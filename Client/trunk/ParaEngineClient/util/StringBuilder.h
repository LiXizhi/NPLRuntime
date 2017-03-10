#pragma once

#include "NPLMemPool.h"

namespace ParaEngine
{
	// forward declare
	template <typename UserAllocator = ParaEngine::CNPLPool_Char_allocator >
	class StringBuilderT;

	/** the string builder class using CNPLPool_Char_alloc */
	typedef StringBuilderT< ParaEngine::CNPLPool_Char_allocator > StringBuilder;

	/**
	A NON-thread-safe, mutable sequence of characters(Binary is also possible). 
	A string buffer is like a std::string, but does not have SSO(16 bytes small string optimization); use ParaEngine::NPLString if one wants SSO support. 
	A string buffer is usually associated with a memory pool allocator such as CNPLPool_Char_alloc<char>. 
	At any point in time it contains some particular sequence of characters, but the length and content of the sequence can be changed through certain method calls. 

	String buffers are NOT safe for use by multiple threads. One should use a mutex if the object is shared by multiple thread. 

	The principal operations on a StringBuffer are the append, which are overloaded so as to accept data of many types. 
	Each effectively converts a given datum to a string and then appends the characters of that string to the string buffer. 
	
	Every string buffer has a capacity. As long as the length of the character sequence contained in the string buffer does not exceed the capacity, 
	it is not necessary to allocate a new internal buffer array. If the internal buffer overflows, it is automatically made larger. 

	@NOTE: c_str() is NOT null terminated, unless one manually append('\0'); so always use a size() to retrieve it. 
	*/
	template <typename UserAllocator>
	class StringBuilderT
	{
	public:
		/** The standard string object which correspond to the builder. */
		typedef std::string string_type;
		typedef UserAllocator user_allocator;
		typedef char Char;

		/** Creates a new builder with the content of the given string.
		@param sz A string. Its content will be copied within the builder. */
		StringBuilderT(const string_type& sz);

		/** Creates a new builder with the content of the given character array.
		@param sz A character array. Its content will be copied within the builder.
		It must terminate with an element containing the 'zero' value. */
		StringBuilderT(const Char* sz);

		/** Creates a new builder with the given character repeated multiple times over the array.
		@param ch A character to repeat.
		@param count The number of times to repeat the given character. */
		StringBuilderT(Char ch, size_t count);

		/** Creates a new builder with an empty buffer. 
		@see reserve
		@param reserved The number of character slots to reserve within the empty buffer. */
		StringBuilderT(size_t reserved);

		/** Creates a new builder with an empty buffer. */
		StringBuilderT();

		/** Deletes the builder. Its buffer will be cleared. 
		Any pointers to its data will be dangling. */
		~StringBuilderT();

		/** Reserves a given number of character slots.
		Internally it uses buffer 32, 64, 128, 256, 512, 1024, 2048. Larger than 2048, length is used etc.
		@param length The number of character slots to reserve. */
		void reserve(size_t length);

		/** resize the buffer */
		void resize(size_t length);

		/** Retrieves the length of the content within the builder.
		@return The length of the string. */
		inline size_t length() const { return m_size; }
		inline size_t size() const { return m_size; }

		/** Clears the content of the builder.
		This does not re-allocate a new buffer. */
		void clear();

		/** Retrieves whether the builder is empty.
		A builder is considered empty when it has no content, regardless of
		the size or allocation status of its buffer.
		@return Whether the builder is empty. */
		inline bool empty() const { return m_size == 0; }

		/** Appends a character to the content of the builder.
		@param c A character. May not be the 'zero' value. */
		void append(Char c);

		/** Appends a string to the content of the builder.
		@param sz A string. */
		void append(const string_type& sz);

		/** Appends a character array to the content of the builder.
		@param sz A character array. It must terminate with an
		element containing the 'zero' value. */
		void append(const Char* sz);

		/** Appends a character array to the content of the builder.
		@param sz A character array. It should not contain any 'zero'
		characters before 'len'
		@param len The number of characters to read from sz. */
		void append(const Char* sz, size_t len);

		/** Appends the content of a builder to the content of this builder.
		@param b A string builder. */
		void append(const StringBuilderT<UserAllocator>& b);

		/** Appends the integer value, after converting it to a string,
		to the content of the builder.
		@param i An integer value. */
		void append(int32 i);
		void append(uint32 i); /**< See above. */
		void append(uint64 i); /**< See above. */

		void appendBinary(uint16 i);
		void appendBinary(uint32 i); /**< See above. */
		
		/** Appends the integer value, after converting it to a
		fm::string, in hexadecimal, to the content of the builder.
		The size of the integer will determine the number of characters used.
		@param i An unsigned integer value. */
		void appendHex(uint8 i);
		template <class T> inline void appendHex(const T& i) { for (size_t j = 0; j < sizeof(T); ++j) appendHex(*(((uint8*)&i) + j)); } /**< See above. */

#if defined(WIN32)
		inline void append(int i) { append((int32) i); } /**< See above. */
#ifdef _W64
		inline void append(_W64 unsigned int i) { append((uint32) i); } /**< See above. */
#else
		inline void append(unsigned int i) { append((uint32) i); } /**< See above. */
#endif
#else
		// inline void append(unsigned long i) { append((uint32) i); } /**< See above. */
		inline void append(long i) { append((int32) i); } /**< See above. */
#endif // platform-switch.

		/** Appends the floating-point value, after converting it to a string,
		to the content of the builder. If the floating-point value is the special token
		that represents infinity, the string "INF" is appended. If it represents
		the negative infinity, the string "-INF" is appended. If it represents the
		impossibility, the string "NaN" is appended.
		@param f A floating-point value. */
		void append(float f);
		void append(double f); /**< See above. */

		/** this is useful for writing to a previous cached location. */
		template<typename TYPE> 
		void WriteAt(int nIndex, const TYPE& val){
			// *((TYPE*)(m_buffer + nIndex)) = val;
			memcpy(m_buffer + nIndex, &val, sizeof(TYPE)); // fix byte alignment issue
		}

		void WriteAt(int nIndex, const Char* data, size_t nSize);


		/** Appends a value to the content of the builder.
		This is a shortcut for the append function.
		@see append
		@param val A value. This may be numerical, a character, a character array or a string. */
		template<typename TYPE> inline StringBuilderT& operator+=(const TYPE& val) { append(val); return *this; }

		/** Appends a character array to the content of the builder.
		A newline character will be appended after the character array.
		@param sz A character array. It must terminate with an
		element containing the 'zero' value. */
		void appendLine(const Char* sz);

		/** Removes a section of the content of the builder.
		Every character that occurs after the given index will be removed,
		resulting in a shrunk string.
		@param start An index within the content of the builder. */
		void remove(int32 start);

		/** Removes a section of the content of the builder.
		The substring defined by the 'start' and 'end' indices will be
		removed. The 'start' character is removed and is replaced by
		the 'end' character.
		@param start The index of the first character of the substring to remove.
		@param end The index of the first character after the removed substring. */
		void remove(int32 start, int32 end);

		/** Removes the last character of the content of the builder. */
		inline void pop_back() { if (m_size > 0) --m_size; }

		/** Sets the content of the builder to a given value.
		This clears the builder of all its content and appends the given value.
		@param val A value. This may be numerical, a character, a character array or a string. */
		template<typename TYPE> inline void set(const TYPE& val) { clear(); append(val); }
		template<typename TYPE> inline StringBuilderT& operator=(const TYPE& val) { clear(); append(val); return *this; } /**< See above. */

		/** Converts the content of the builder to a standard string.
		@return A string with the content of the builder. */
		string_type ToString() const { return string_type(c_str(), length()); };

		/** Converts the content of the builder to a character array. Please note the char array is not null terminated. 
		@return A character array with the content of the builder.
		This pointer is valid for the lifetime of the buffer of the builder, so
		do not keep it around. This character array should not be modified. */
		const Char* c_str() const;
		inline operator const Char*() const { return c_str(); } /**< See above. */

		/** get raw string */
		Char* str() { return m_buffer; };

		/** overload the operator []. You are at your own risk if nIndex is out of risk. There is no check for performance reasons.*/
		Char& operator[] (const int nIndex) { return *(m_buffer+nIndex); }; 

		/** Retrieves the index of the first character within the content of the builder
		that is equivalent to the given character.
		@param c The character to match.
		@return The index of the first equivalent character. -1 is returned if no
		character matches the given character. */
		int32 index(Char c) const;

		/** Retrieves the index of the last character within the content of the builder
		that is equivalent to the given character.
		@param c The character to match.
		@return The index of the last equivalent character. -1 is returned if no
		character matches the given character. */
		int32 rindex(Char c) const;

		/** Retrieves the last character within the content of the builder.
		@return The last character of the builder. */
		Char back() const { assert(m_size > 0); return m_buffer[m_size-1]; }
	private:
		void enlarge(size_t minimum);

	private:
		Char* m_buffer;
		size_t m_reserved;
		size_t m_size;
	};

	template <typename UserAllocator>
	void ParaEngine::StringBuilderT<UserAllocator>::appendBinary(uint32 i)
	{
		append((const char*)(&i), 4);
	}

	template <typename UserAllocator>
	void ParaEngine::StringBuilderT<UserAllocator>::appendBinary(uint16 i)
	{
		append((const char*)(&i), 2);
	}

}

// required by DLL interface
// EXPIMP_TEMPLATE template class PE_CORE_DECL ParaEngine::StringBuilderT< ParaEngine::CNPLPool_Char_allocator >;
