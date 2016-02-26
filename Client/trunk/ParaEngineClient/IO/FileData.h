#pragma once

namespace ParaEngine
{
	/* a file data in memory*/
	class FileData
	{
	public:
		static const FileData Null;
		FileData();
		FileData(const FileData& other);
		FileData(FileData&& other);
		~FileData();

		FileData& operator= (const FileData& other);
		FileData& operator= (FileData&& other);

		char* GetBytes() const;
		size_t GetSize() const;

		/** Copies the buffer pointer and its size.
		*  @note This method will copy the whole buffer.
		*        Developer should free the pointer after invoking this method.
		*  @see Data::fastSet
		*/
		void copy(char* bytes, const size_t size);

		/** take ownership of the buffer. it can be null. */
		void SetOwnBuffer(char* bytes, const size_t size);
		void ReleaseOwnership();

		/** Clears data, free buffer and reset data size */
		void Clear();

		/** Check whether the data is null. */
		bool isNull() const;

	private:
		void move(FileData& other);

	private:
		char* m_bytes;
		size_t m_size;
	};
}