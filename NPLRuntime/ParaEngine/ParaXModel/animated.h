#pragma once
#include <utility>
#include <vector>

#include "modelheaders.h"
#include "AnimInstanceBase.h"
#include "IAnimated.h"

namespace ParaEngine
{
	typedef std::pair<uint32, uint32> AnimRange;

	// global time for global sequences
	extern int64_t globalTime;

	/* Generic animated value class:
	* T is the data type to animate
	* D is the data type stored in the file (by default this is the same as T)
	* Conv is a conversion object that defines T conv(D) to convert from D to T
	* (by default this is an identity function)
	*/
	template <class T, class D = T, class Conv = Identity<T> >
	class Animated : public IAnimated
	{
	public:
		Animated() :used(false), type(INTERPOLATION_LINEAR), seq(-1), globals(0) {};
		/** whether it is a constant value(not animated).*/
		bool used;
		/** interpolation of type Interpolations */
		int type;
		/**
		* - if this is 0 or above, it is the index into the global sequence.
		* - if this is -1, it is not a global animation, animID is used instead;
		* - if this is -2, it is a local global animation, where range[0] is used and a global timer is used to loop on this range;
		*/
		int seq;
		/** a reference to the global sequence object which is an array of time ranges for global sequences.*/
		int *globals;

		std::vector<AnimRange> ranges;
		std::vector<int> times;
		std::vector<T> data;
		// for nonlinear interpolations:
		std::vector<T> in, out;

		inline static float Absolute1(float v) { return fabs(v); };
		inline static float Absolute1(double v) { return (float)abs(v); };
		inline static float Absolute1(const Vector3& v) { return fabs(v.x) + fabs(v.y) + fabs(v.z); };
		inline static float Absolute1(const Quaternion& v) { return fabs(v.x) + fabs(v.y) + fabs(v.z) + fabs(v.w); };
		inline static float Absolute1(const Vector2& v) { return fabs(v.x) + fabs(v.y); };

		/** check if all key are equal*/
		bool CheckIsAnimated()
		{
			int nSize = (int)data.size();
			if (used && nSize > 0)
			{
				auto firstValue = data[0];
				for (int i = 1; i < nSize; ++i)
				{
					if (data[i] != firstValue)
						return true;
				}
			}
			return false;
		}

		/** if all animated values equals to the key, this animation will be set unused*/
		void SetConstantKey(T key)
		{
			int nSize = (int)data.size();
			for (int i = 0; i < nSize; ++i)
			{
				if (data[i] != key)
					return;
			}
			used = false;
		}
		/** if all animated values are very close to a given key, this animation will be set unused*/
		void SetConstantKey(T key, float fEpsilon)
		{
			int nSize = (int)data.size();
			for (int i = 0; i < nSize; ++i)
			{
				if (Absolute1(data[i] - key) > fEpsilon)
					return;
			}
			used = false;
		}

		/** this function will return the interpolated animation vector at the specified anim id and frame number*/
		T getValue(int anim, int time)
		{
			if (type != INTERPOLATION_NONE || data.size() > 1) {
				AnimRange range;

				// obtain a time value and a data range
				if (seq >= 0) {
					/// global animation has nothing to do the current animation. Such animation may be 
					/// the blinking of eyes and some effects which always loops regardless of the current animation.
					if (globals[seq] == 0)
						time = 0;
					else
						time = globalTime % globals[seq];
					range.first = 0;
					range.second = (uint32)(data.size()) - 1;
				}
				else if (seq == -2)
				{
					/// it is also a global animation, however, range[0] is used instead of globals
					range = ranges[0];
					if (range.second > range.first)
						time = times[range.first] + globalTime % (times[range.second] - times[range.first]);
					else
						time = 0;
				}
				else if (seq == -1) {
					/// get the range according to the current animation.
					if (anim >= 0 && anim < (int)ranges.size())
					{
						range = ranges[anim];
					}
					else
					{
						// default value
						return data[0];
					}
					// there is a chance that time = times[times.size()-1]
					//time %= times[times.size()-1]; // I think this might not be necessary?
				}
				if (time >= times[range.second])
				{
					return data[range.second];
				}
				else if (range.first != range.second && time > times[range.first])
				{
					size_t pos = range.first; // this can be 0.
					{
						/** by LiXizhi: use binary search for the time frame: 2005/09:
						* modify the brutal force search by binary search
						* and use interpolation.
						1.  location = -1;
						2.  while ((more than one item in list))
						2A.  look at the middle item
						2B.  if (middle item is target)
						have found target
						return location as the result
						else
						2C.  if (target < middle item)
						list = first half of list
						2D.  else (target > middle item)
						list = last half of list
						end while
						*/
						int nStart = (int)range.first;
						int nEnd = (int)range.second - 1;
						while (true)
						{
							if (nStart >= nEnd)
							{ // if no item left.
								pos = nStart;
								break;
							}
							int nMid = (nStart + nEnd) / 2;
							int startP = (times[nMid]);
							int endP = (times[nMid + 1]);

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
					}
					int t1 = times[pos];
					int t2 = times[pos + 1];
					float r = (time - t1) / (float)(t2 - t1);

					if (type == INTERPOLATION_LINEAR)
						return interpolate<T>(r, data[pos], data[pos + 1]);
					else if (type == INTERPOLATION_LINEAR_CROSSFRAME)
					{
						if ((t2 - t1) <= 34) // if the two key frames are less than 33 milliseconds away, do not interpolate, instead use the first one.
							return data[pos];
						else
							return interpolate<T>(r, data[pos], data[pos + 1]);
					}
					else {
						// INTERPOLATION_HERMITE is only used in cameras afraid?
						// return interpolateHermite<T>(r,data[pos],data[pos+1],in[pos],out[pos]);
						// TODO: use linear interpolation anyway
						return interpolate<T>(r, data[pos], data[pos + 1]);
					}
				}
				else {
					return data[range.first];
				}
			}
			else {
				// default value
				return data[0];
			}
		}

		// default value
		T getDefaultValue()
		{
			return data[0];
		}

		/**
		* get value with motion blending with a specified blending frame.
		* @param nCurrentAnim: current animation sequence ID
		* @param currentFrame: an absolute ParaX frame number denoting the current animation frame. It is always within
		* the range of the current animation sequence's start and end frame number.
		* @param nBlendingAnim: the animation sequence with which the current animation should be blended.
		* @param blendingFrame: an absolute ParaX frame number denoting the blending animation frame. It is always within
		* the range of the blending animation sequence's start and end frame number.
		* @param blendingFactor: by how much the blending frame should be blended with the current frame.
		* 1.0 will use solely the blending frame, whereas 0.0 will use only the current frame.
		* [0,1), blendingFrame*(blendingFactor)+(1-blendingFactor)*currentFrame
		*/
		T getValue(int nCurrentAnim, int currentFrame, int nBlendingAnim, int blendingFrame, float blendingFactor)
		{
			if (blendingFactor == 0.0f)
			{
				return getValue(nCurrentAnim, currentFrame);
			}
			else
			{
				if (blendingFactor == 1.0f)
				{
					return getValue(nBlendingAnim, blendingFrame);
				}
				else
				{
					T v1 = getValue(nCurrentAnim, currentFrame);
					T v2 = getValue(nBlendingAnim, blendingFrame);

					return interpolate<T>(blendingFactor, v1, v2);
				}
			}
		}

		/** it accept anim index of both local and external animation*/
		T getValue(const AnimIndex& Index)
		{
			return (Index.Provider == 0) ? getValue(Index.nIndex, Index.nCurrentFrame) : getDefaultValue();
		}

		/** it accept anim index of both local and external animation*/
		T getValue(const AnimIndex& CurrentAnim, const AnimIndex& BlendingAnim, float blendingFactor)
		{
			if (blendingFactor == 0.0f)
			{
				return getValue(CurrentAnim);
			}
			else
			{
				if (blendingFactor == 1.0f)
				{
					return getValue(BlendingAnim);
				}
				else
				{
					T v1 = getValue(CurrentAnim);
					T v2 = getValue(BlendingAnim);

					return interpolate<T>(blendingFactor, v1, v2);
				}
			}
		}

		static T BlendValues(const T& currentValue, const T& blendingValue, float blendingFactor)
		{
			if (blendingFactor == 0.0f)
			{
				return currentValue;
			}
			else
			{
				if (blendingFactor == 1.0f)
				{
					return blendingValue;
				}
				else
				{
					return interpolate<T>(blendingFactor, currentValue, blendingValue);
				}
			}
		}

		void SetRangeByAnimIndex(int nAnimIndex, const AnimRange& range)
		{
			if ((int)ranges.size() <= nAnimIndex)
				ranges.resize(nAnimIndex + 1, range);
			else
				ranges[nAnimIndex] = range;
		}
		void AppendKey(int time_, const T& data_)
		{
			times.push_back(time_);
			data.push_back(data_);
		}

		void UpdateLastKey(int time_, const T& data_)
		{
			int KeyCount = GetKeyNum();
			if (KeyCount > 0)
			{
				times[KeyCount - 1] = time_;
				data[KeyCount - 1] = data_;
			}
			else
				AppendKey(time_, data_);
		}
		int GetKeyNum()
		{
			return (int)data.size();
		}

		/**
		* if multiple adjacent keys can be linearly interpolated and get an error smaller than fEpsilon,
		* those keys will be collapsed to save space
		* Note: this function only works when there is only one animation range. otherwise ranges member may be invalid after compression.
		*/
		void CompressKeyLinear(float fEpsilon)
		{
			Animated <T>c_data; // compressed data

			// add initial key
			int nKeyCount, nFirstKeyIndex, nLastKeyIndex, i, curtime;
			T curkey;
			nLastKeyIndex = nFirstKeyIndex = c_data.GetKeyNum();
			curkey = data[0];

			c_data.AppendKey(times[0], curkey);

			T lastKey = curkey;
			T lastlastKey = lastKey;

			// output animation keys
			nKeyCount = GetKeyNum();

			for (i = 1; i < nKeyCount; ++i)
			{
				curtime = times[i];
				curkey = data[i];

				T predicatedKey = lastKey * 2 - lastlastKey;
				T delta = (curkey - predicatedKey);


				if (Absolute1(delta) >= fEpsilon)
				{
					// add new key
					if (nLastKeyIndex == nFirstKeyIndex)
					{
						// if this is the second key, modify the first key's time to animSequence.timeStart, and insert a constant key if necessary.
						int nKeyIndex = c_data.GetKeyNum() - 1;
						int nTime = c_data.times[nKeyIndex];
						c_data.times[nKeyIndex] = 0;
						if (i > 2)
						{
							c_data.AppendKey(nTime, lastKey);
							++nLastKeyIndex;
						}
					}
					c_data.AppendKey(curtime, curkey);
					predicatedKey = curkey;
					++nLastKeyIndex;
				}
				else
				{
					// override the last key
					c_data.UpdateLastKey(curtime, predicatedKey);
				}

				lastlastKey = lastKey;
				lastKey = predicatedKey;
			}
			if (nFirstKeyIndex == nLastKeyIndex)
				c_data.UpdateLastKey(0, lastKey);
			c_data.SetRangeByAnimIndex(0, AnimRange(nFirstKeyIndex, nLastKeyIndex));

			//////////////////////////////////////////////////////////////////////////
			// copy back
			times = c_data.times;
			data = c_data.data;
			SetRangeByAnimIndex(0, AnimRange(nFirstKeyIndex, nLastKeyIndex));
		}

		virtual int GetNumKeys()
		{
			return (int)data.size();
		}

		virtual void SetNumKeys(int nKeyCount)
		{
			if (GetNumKeys() != nKeyCount)
			{
				data.resize(nKeyCount);
				times.resize(nKeyCount);
			}
		}

		virtual void SetTime(int nIndex, int nTime)
		{
			if (nIndex < (int)times.size())
				times[nIndex] = nTime;
		}

		virtual int GetTime(int nIndex)
		{
			return (nIndex < (int)times.size()) ? times[nIndex] : 0;
		}

		/** if some keys are never used in ranges, we will remove them */
		void RemoveUnusedAnimKeys()
		{
			int nRangeCount = ranges.size();

			std::vector<int> times_;
			times_.reserve(times.size());
			std::vector<T>  data_;
			data_.reserve(data.size());
			std::vector<AnimRange> ranges_;
			ranges_.resize(nRangeCount);

			int nIndex = -1;
			for (int i=0; i<nRangeCount; ++i)
			{
				const auto& fromRange = (ranges[i]);
				auto& toRange = (ranges_[i]);
				
				toRange.first = nIndex > 0 ? nIndex : 0;
				for (auto k = fromRange.first; k <= fromRange.second; k++)
				{
					auto time = times[k];
					if (nIndex < 0 || times_[nIndex] != time)
					{
						times_.push_back(time);
						data_.push_back(data[k]);
						nIndex ++;
					}
				}
				toRange.second = nIndex > 0 ? nIndex : 0;
			}

			if (times_.size() < times.size())
			{
				times = times_;
				data = data_;
				ranges = ranges_;
			}
		}
	};

	typedef Animated<float, DWORD, ShortToFloat> AnimatedShort;

}
