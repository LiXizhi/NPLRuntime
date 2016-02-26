#pragma once

namespace ParaEngine
{
	class Quaternion;

	enum Interpolations {
		INTERPOLATION_NONE,
		INTERPOLATION_LINEAR,
		INTERPOLATION_HERMITE,
		// only use linear interpolation cross frames. a frame is assumed to be 1/30 seconds in the Engine. If two key frames are this close to each other, no interpolation is used.
		// this is the default setting for UV translation animations.
		INTERPOLATION_LINEAR_CROSSFRAME,
	};


	/** base class for AnimatedVariable. */
	class IAnimated
	{
	public:
		virtual ~IAnimated(){};

		/** get total number of animated keys. For non-animated attribute, this will return 1. */
		virtual int GetNumKeys();
		/** get set the total number of animated keys. */
		virtual void SetNumKeys(int nKeyCount);

		/** return the index of the first key whose time is larger than or equal to time.
		* return -1 if no suitable index is found.
		*/
		virtual int GetNextKeyIndex(int nTime);

		/** add a given key with default value at specified time
		* if there is already a key at the index, we will return the old index.
		* return the key index. 
		* @param isKeyExist: if not NULL, it will return whether the given key already exist at the index. 
		*/
		virtual int AddKey(int time, bool* isKeyExist = NULL);

		/** only applied to Animated attribute
		* @param nIndex: key index.
		* @param nTime: in milliseconds
		*/
		virtual void SetTime(int nIndex, int nTime);
		virtual int GetTime(int nIndex);
		
		virtual void SetValue(int nIndex, const Quaternion& val);
		virtual bool GetValue(int nIndex, Quaternion& val);
		virtual bool GetValueByTime(int nTime, Quaternion& val);

		virtual void SetValue(int nIndex, const Vector2& val);
		virtual bool GetValue(int nIndex, Vector2& val);
		virtual bool GetValueByTime(int nTime, Vector2& val);

		virtual void SetValue(int nIndex, const Vector3& val);
		virtual bool GetValue(int nIndex, Vector3& val);
		virtual bool GetValueByTime(int nTime, Vector3& val);

		virtual void SetValue(int nIndex, const double& val);
		virtual bool GetValue(int nIndex, double& val);
		virtual bool GetValueByTime(int nTime, double& val);

		virtual void SetValue(int nIndex, const float& val);
		virtual bool GetValue(int nIndex, float& val);
		virtual bool GetValueByTime(int nTime, float& val);

		virtual void SetValue(int nIndex, const int& val);
		virtual bool GetValue(int nIndex, int& val);
		virtual bool GetValueByTime(int nTime, int& val);

		virtual void SetValue(int nIndex, const std::string& val);
		virtual bool GetValue(int nIndex, std::string& val);
		virtual bool GetValueByTime(int nTime, std::string& val);
	};


	// interpolation functions
	template<class T>
	inline T interpolate(const float r, const T &v1, const T &v2)
	{
		return  (v1*(1.0f - r) + v2*r);
	}

	template<class T>
	inline T interpolateHermite(const float r, const T &v1, const T &v2, const T &in, const T &out)
	{
		// dummy
		//return interpolate<T>(r,v1,v2);

		// basis functions
		float h1 = 2.0f*r*r*r - 3.0f*r*r + 1.0f;
		float h2 = -2.0f*r*r*r + 3.0f*r*r;
		float h3 = r*r*r - 2.0f*r*r + r;
		float h4 = r*r*r - r*r;

		// interpolation
		return  static_cast<T>(v1*h1 + v2*h2 + in*h3 + out*h4);
	}

	// "linear" interpolation for quaternions should be slerp by default
	template<>
	inline Quaternion interpolate<Quaternion>(const float r, const Quaternion &v1, const Quaternion &v2)
	{
		return Quaternion::Slerp(r, v1, v2, true);
	}

	// Convert opacity values stored as shorts to floating point
	class ShortToFloat {
	public:
		inline static const float conv(const DWORD t)
		{
			return t / 32767.0f;
		}
	};

	template <class T>
	class Identity {
	public:
		inline static const T& conv(const T& t)
		{
			return t;
		}
	};

	/** key, value pairs */
	template <class T, class Conv = Identity<T> >
	class AnimatedVariable : public IAnimated
	{
	public:
		AnimatedVariable() : m_interpolationType(INTERPOLATION_LINEAR){
		};
		virtual ~AnimatedVariable(){};
	public:
		/** this function will return the interpolated animation vector frame number 
		* internally use binary search algorithm 
		*/
		virtual bool GetValueByTime(int time, T& val)
		{
			int nNumKeys = (int) m_data.size();
			if (nNumKeys > 1)
			{
				size_t pos = 0;

				int nStart = 0;
				int nEnd = nNumKeys-2;
				while (true)
				{
					if (nStart >= nEnd)
					{ // if no item left.
						pos = nStart;
						break;
					}
					int nMid = (nStart + nEnd) / 2;
					int startP = (m_times[nMid]);
					int endP = (m_times[nMid + 1]);

					if (startP <= time && time < endP)
					{ // if (middle item is target)
						pos = nMid;
						break;
					}
					else if (time < startP)
					{ // if (target < middle item)
						nEnd = nMid;
					}
					else if (time >= endP)
					{ // if (target >= middle item)
						nStart = nMid + 1;
					}
				}// while(nStart<=nEnd)
				
				int t1 = m_times[pos];
				int t2 = m_times[pos + 1];
				/*if (time > t2 || time < t1)
				{
					return false;
				}*/
				if (time >= t2)
				{
					val = m_data[pos + 1];
				}
				else if (time <= t1)
				{
					val = m_data[pos];
				}
				else
				{
					float r = (time - t1) / (float)(t2 - t1);

					if (m_interpolationType == INTERPOLATION_LINEAR)
						val = interpolate<T>(r, m_data[pos], m_data[pos + 1]);
					else if (m_interpolationType == INTERPOLATION_LINEAR_CROSSFRAME)
					{
						if ((t2 - t1) <= 34) // if the two key frames are less than 33 milliseconds away, do not interpolate, instead use the first one.
							val = m_data[pos];
						else
							val = interpolate<T>(r, m_data[pos], m_data[pos + 1]);
					}
					else {
						// INTERPOLATION_HERMITE is only used in cameras afraid?
						// return interpolateHermite<T>(r,data[pos],data[pos+1],in[pos],out[pos]);
						// TODO: use linear interpolation anyway
						val = interpolate<T>(r, m_data[pos], m_data[pos + 1]);
					}
				}
				return true;
			}
			else if (nNumKeys > 0)
			{
				// default value
				val = m_data[0];
				return true;
			}
			else
				return false;
		}

		virtual bool GetValue(int nIndex, T& val)
		{
			if (nIndex < GetNumKeys())
			{
				val = m_data[nIndex];
				return true;
			}
			else if (m_data.size() > 0)
			{
				val = m_data[m_data.size()-1];
				return true;
			}
			return false;
		}

		virtual void SetValue(int nIndex, const T& val)
		{
			if (nIndex < GetNumKeys())
			{
				m_data[nIndex] = val;
			}
			else if (nIndex>=0)
			{
				SetNumKeys(nIndex + 1);
				SetValue(nIndex, val);
			}
		}

		virtual int GetNumKeys()
		{
			return (int)m_data.size();
		}

		virtual void SetNumKeys(int nKeyCount)
		{
			if (GetNumKeys() != nKeyCount)
			{
				m_data.resize(nKeyCount);
				m_times.resize(nKeyCount);
			}
		}

		virtual void SetTime(int nIndex, int nTime)
		{
			if (nIndex < (int)m_times.size())
				m_times[nIndex] = nTime;
		}

		virtual int GetTime(int nIndex)
		{
			return (nIndex < (int)m_times.size()) ? m_times[nIndex] : 0;
		}

		virtual int GetNextKeyIndex(int time)
		{
			int dataCount = (int)m_data.size();
			if (dataCount == 1)
			{
				return 0;
			}
			else if (dataCount > 0)
			{
				int pos = 0;

				int nStart = 0;
				int nEnd = dataCount - 1;
				while (true)
				{
					if (nStart >= nEnd)
					{ // if no item left.
						pos = nStart;
						break;
					}
					int nMid = (nStart + nEnd) / 2;
					int startP = (m_times[nMid]);
					int endP = (m_times[nMid + 1]);

					if (startP <= time && time < endP)
					{ // if (middle item is target)
						pos = nMid;
						break;
					}
					else if (time < startP)
					{ // if (target < middle item)
						nEnd = nMid;
					}
					else if (time >= endP)
					{ // if (target >= middle item)
						nStart = nMid + 1;
					}
				}// while(nStart<=nEnd)

				for (int i = pos - 1; i >= 0; --i)
				{
					if (m_times[i] >= time)
						pos = i;
					else
						break;
				}
				return pos;
			}
			return -1;
		}

		virtual int AddKey(int time, bool* isKeyExist = NULL)
		{
			int index = GetNextKeyIndex(time);
			if (index < 0)
				index = 0;
			if (index < GetNumKeys())
			{
				int next_time = GetTime(index);
				if (next_time == time)
				{
					if (isKeyExist)
						*isKeyExist = true;
					return index;
				}
				else
				{
					if (next_time < time)
						index = index + 1;

					// insert before next_time;
					m_times.insert(m_times.begin()+index, time);
					m_data.insert(m_data.begin()+index, T());
					if (isKeyExist)
						*isKeyExist = false;
					return index;
				}
			}
			else
			{
				SetNumKeys(index + 1);
				if (isKeyExist)
					*isKeyExist = false;
				return index;
			}
		}
	private:
		/** interpolation of type Interpolations */
		int m_interpolationType;
		std::vector<int> m_times;
		std::vector<T> m_data;
	};
}