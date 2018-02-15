#pragma once
#include <string>
#include <vector>

namespace ParaEngine
{

	/**
	* a class for tracking the history of values. 
	* It is used for network values such as player positions, etc.
	* It keeps a history of most recent (time, value) pairs.
	*/
	template <class T>
	class CValueTracker
	{
	public:
		CValueTracker(void)
		{
			m_nCurrentPos = 0;
			SetSize(2); // default size is always 2
		}
		~CValueTracker(void)
		{

		}

	public:
		/** make the tracker empty*/
		void Reset()
		{
			int nSize = GetSize();
			for (int i=0;i<nSize;++i)
			{
				m_times[i] = INVALID_TIME;
			}
			m_nCurrentPos = 0;
		}
		/** Get how many items to be kept in the tracker */
		int GetSize()
		{
			return m_nSize;
		}
		/** set how many items to be kept in the tracker */
		void SetSize(int nSize)
		{
			m_nSize = nSize;

			if(m_nCurrentPos<0 || m_nCurrentPos>=m_nSize)
				m_nCurrentPos = 0;
			m_times.resize(m_nSize, INVALID_TIME);
			m_values.resize(m_nSize);
		}


		/**
		* push a new item to the tracker.
		* @param time 
		* @param value 
		*/
		void Push(int time, const T& value)
		{
			++m_nCurrentPos;
			if(m_nCurrentPos>=m_nSize)
				m_nCurrentPos = 0;
			
			Update(time, value);
		}

		/**
		* update the current (latest) value 
		* @param time 
		* @param value 
		*/
		void Update(int time, const T& value)
		{
			m_times[m_nCurrentPos] = time;
			m_values[m_nCurrentPos] = value;
		}

		/**
		* get the time in history. 
		* @param nPos 0 means current one, -1 means last one. 1 means first one
		*/
		int GetTime(int nPos=0)
		{
			nPos = m_nCurrentPos+nPos;
			if(nPos<0)
			{
				nPos += m_nSize;
				if(nPos>=0)
					return m_times[nPos];
			}
			else if(nPos>=m_nSize)
			{
				nPos -= m_nSize;
				if(nPos>=0)
					return m_times[nPos];
			}
			else
			{
				return m_times[nPos];
			}
			return INVALID_TIME;
		}

		/**
		* get the value in history. 
		* @param nPos 0 means current one, -1 means last one. 1 means first one
		*/
		const T& GetValue(int nPos=0)
		{
			nPos = m_nCurrentPos+nPos;
			if(nPos<0)
			{
				nPos += m_nSize;
				if(nPos>=0)
					return m_values[nPos];
			}
			else if(nPos>=m_nSize)
			{
				nPos -= m_nSize;
				if(nPos>=0)
					return m_values[nPos];
			}
			else
			{
				return m_values[nPos];
			}
			// this should never be called
			static T g_empty;
			return g_empty;
		}
		/**
		* get the value in history. 
		* @param value: [out] if there is a value at the time, it will be returned in this struct. 
		* @param nPos 0 means current one, -1 means last one. 1 means first one
		* @return: true if a value is returned, or false if there is no value at the position.
		*/
		bool GetValue(T& value, int nPos)
		{
			if(GetTime(nPos) == INVALID_TIME)
				return false;
			nPos = m_nCurrentPos+nPos;
			if(nPos<0)
			{
				nPos += m_nSize;
				if(nPos>=0)
				{
					value = m_values[nPos];
					return true;
				}
			}
			else if(nPos>=m_nSize)
			{
				nPos -= m_nSize;
				if(nPos>=0)
				{
					value = m_values[nPos];
					return true;
				}
			}
			else
			{
				value = m_values[nPos];
				return true;
			}
			// no value is found
			return false;
		}

		
		/**
		* return true if the last value. i.e. GetValue(0) is the same as the passed value
		* if the last value is empty, it will always return false.
		* @param right with which to compare
		* @return 
		*/
		BOOL CompareWith(const T& right)
		{
			if(m_times[m_nCurrentPos] == INVALID_TIME)
				return false;
			return (m_values[m_nCurrentPos] == right);
		}

		/**
		* return whether all values in the tracker are the same. 
		* @note: if any slot of the tracker is empty, this function will return false.
		*/
		bool IsConstant()
		{
			bool bFirstValue = true;
			bool bConstant = false;
			T lastValue;
			for (int i=0;i<m_nSize;++i)
			{
				if( m_times[i] != INVALID_TIME)
				{
					if(bFirstValue)
					{
						lastValue = m_values[i];
						bConstant = true;
						bFirstValue = false;
					}
					else
					{
						if(lastValue != m_values[i])
							return false;
					}
				}
				else
					return false;
			}
			return bConstant;
		}

	private:
		vector <int> m_times;
		vector <T> m_values;
		int m_nCurrentPos;
		int m_nSize;
		static const int INVALID_TIME = -1;
	};

	/**
	* for tracking binary data, such as custom model data.
	*/
	class CNetByteArray: public std::vector<byte>
	{
	public:
		/** comparison*/
		bool operator == (const CNetByteArray& right)
		{
			return EqualsTo(&(right[0]), (int)right.size());
		}

		/** set data */
		void SetData(const byte* pData, int nSize)
		{
			this->resize(nSize);
			if(nSize>0)
			{
				memcpy(&((*this)[0]), pData, nSize);
			}
		}

		/** assignment */
		CNetByteArray& operator = (const CNetByteArray& right)
		{
			SetData(&(right[0]), (int)right.size());
			return *this;
		}

		/** compare*/
		bool EqualsTo(const byte* pData, int nSize)
		{
			if((int)(this->size()) ==  nSize)
			{
				int i=0;
				for (;i<nSize && ((*this)[i]==pData[i]);++i)
				{
				}
				return (i==nSize);
			}
			else
				return false;
		}
	};

	typedef CValueTracker<Vector3> CVector3Tracker;
	typedef CValueTracker<int> CIntTracker;
	typedef CValueTracker<float> CFloatTracker;
	typedef CValueTracker<std::string> CStringTracker;
	typedef CValueTracker<bool> CBooleanTracker;
	typedef CValueTracker<CNetByteArray> CNetByteArrayTracker;
}

