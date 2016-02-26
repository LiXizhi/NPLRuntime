#pragma once

#include <vector>
#include <string>

namespace ParaEngine
{
	/**
	* Generic movie key frame class:
	* T is the data type stored in the key frame
	*/
	template <class T>
	class MovieKeyFrame
	{
	private:
		std::vector<float> times;
		std::vector<T> data;

		/** current position */
		int m_nPos;

		/** return a valid index from nPos. so that it is in the range [0,size)
		* value is boundary safe. but there should be at least one element. 
		*/
		int GetValidIndex(int nPos){
			if(nPos<0)
				nPos = 0;
			else if(nPos >= GetSize())
				nPos = GetSize()-1;
			return nPos;
		}
	public:
		void Clear(){
			times.clear();
			data.clear();
			m_nPos=0;
		};
		MovieKeyFrame():m_nPos(0){};
		/** append a new key to the end of the key frame list */
		void AppendNewKey(float fTime, const T& key)
		{
			times.push_back(fTime);
			data.push_back(key);
		}

		/** NOT TESTED: Insert a new key to the key frame list according to the time.
		* @param fTime: time at which to insert the key, 
		* @param key: the key to be inserted.
		*/
		void InsertNewKey(float fTime, const T& key)
		{
			int nPos = GetKeyIndexByTime(fTime);
			if(nPos<0 || nPos >= GetSize() )
			{
				AppendNewKey(fTime, key);
			}
			else
			{
				times.insert(times.begin()+nPos, fTime);
				data.insert(data.begin()+nPos, key);
			}
		}

		/** get the ending time */
		float GetEndingTime()
		{
			int nEndIndex = times.size()-1;
			if(nEndIndex>=0)
				return times[nEndIndex];
			else
				return 0;
		}
		/** update a key frame
		* value is boundary safe. but there should be at least one element. 
		* @param nIndex: the index of the key frame to update
		* @param fTime: the time
		* @param key: the key */
		void UpdateKeyFrame(int nIndex, float fTime, const T& key)
		{
			nIndex = GetValidIndex(nIndex);
			times[nIndex] = fTime;
			data[nIndex] = key;
		}
		/** pop a specified number of key frames from the end of the key frames 
		* @param nNum: the number of key frames to remove. 
		*/
		void PopKeyFrame(int nNum)
		{
			int nLen = GetSize();
			{
				std::vector<float>::iterator iterFrom, iterTo = times.end();
				if(nLen >nNum)
					iterFrom = iterTo - nNum;
				else
					iterFrom = times.begin();
				times.erase(iterFrom, iterTo);
			}
			{
				typename std::vector<T>::iterator iterFrom, iterTo = data.end();
				if(nLen >nNum)
					iterFrom = iterTo - nNum;
				else
					iterFrom = data.begin();
				data.erase(iterFrom, iterTo);
			}
		}
		/** indexer */
		T& operator [](int nIndex){
			return data[GetValidIndex(nIndex)];
		};
		/** get the time. 
		* value is boundary safe. but there should be at least one element. 
		*/
		float time(int nIndex){
			return times[GetValidIndex(nIndex)];
		};

		/** get the current position. 
		* value is not boundary safe
		*/
		int GetPos(){return m_nPos;};

		/** set the current position. 
		* value is not boundary safe
		*/
		void SetPos(int nPos){
			m_nPos = nPos;
		};

		/** Get the number of key frames. */
		int GetSize(){return (int)data.size();};

		/** get the data at current position + nShiftPos.
		* value is boundary safe. but there should be at least one element. 
		* @param nShiftPos: the shift position. default value is 0. which is the current one */
		T& GetRelative(int nShiftPos = 0){
			return data[GetValidIndex(m_nPos+nShiftPos)];
		}
		
		/** NOT TESTED: get the index nPos of the key frame, where time[nPos-1]<=fTime<time[nPos] 
		* a binary search is used internally. if there is no key frame found. -1 is returned
		* please note that times.size() may be returned in case the time is larger than all key times in the list
		*/
		int GetKeyIndexByTime(float fTime)
		{
			int nLen = GetSize();
			int nPos = GetPos();
			bool bFound = false;
			for(int i=0;i<nLen && !bFound;i++) {
				float fFrom = time(nPos-1);
				float fTo = time(nPos);
				if(fFrom<=fTo)
				{
					if(fFrom<= fTime && fTime<fTo)
						bFound = true;
					else if(fTo<=fTime){
						if(nPos == (nLen))
							bFound = true;
						else
							nPos++;
					}
					else if(fFrom>fTime){
						if(nPos == 0)
							bFound = true;
						else
							nPos--;
					}
				}
				else
				{
					// no rewinding is supported.
					break;
				}
			}
			if(bFound)
				return nPos;
			else
				return -1;
		}
		/** update the current time position by a given time in seconds.the current time position may not be the same 
		* as the given value in case a rewinding occurs. in such case, the input contains the actual time set.
		* value is not boundary safe.This function is safe when the key frames are empty.
		* @param fTimeElapsed: [in|out] time in seconds. the elapsed time might be reset by this function
		*		in case that a rewind occurs. 
		* @return: the current position is returned, it is guaranteed that time[nPos-1]<=fTimeElapsed<time[nPos]
		*  if the frames are empty, 0 is returned
		*/
		int UpdateTime(float& fTimeElapsed){
			int nLen = GetSize();
			int nPos = GetPos();
			bool bFound = false;
			bool bRewind = false;
			for(int i=0;i<nLen && !bFound;i++) {
				float fFrom = time(nPos-1);
				float fTo = time(nPos);
				if(fFrom<=fTo)
				{
					if(fFrom<= fTimeElapsed && fTimeElapsed<fTo)
						bFound = true;
					else if(fTo<=fTimeElapsed){
						if(nPos == (nLen))
							bFound = true;
						else
							nPos++;
					}
					else if(fFrom>fTimeElapsed){
						if(nPos == 0)
							bFound = true;
						else
							nPos--;
					}
				}
				else if(!bRewind)
				{
					// a rewind occurs, so to reset the time.
					fTimeElapsed = fTo;
					nPos = 0;
					bRewind = true;
				}
				else
					bFound = true;
			}
			SetPos(nPos);
			return GetPos();
		}
		/** trim the key frames to the specified time. All key frames after the input time will be removed. 
		* This function is safe when the key frames are empty.
		* @return: the number of keys trimmed is returned
		*/
		int TrimToTime(float fTimeElapsed)
		{
			int nPos = UpdateTime(fTimeElapsed);
			int nEndPos = GetSize();
			if(nPos < nEndPos)
			{
				PopKeyFrame(nEndPos - nPos);
				return nEndPos - nPos;
			}
			return 0;
		}
	};
	struct PosKey
	{
		DVector3 vPos;
		float fFacing;
		PosKey(float x,float y,float z,float facing):vPos(x,y,z), fFacing(facing){};
		PosKey(const DVector3& v,float facing):vPos(v), fFacing(facing){};
	};
	/** dialog key for movie*/
	struct DialogKey
	{
		/** format:
		* "sentence": speak the sentence in the background
		* "\say sentence": tell the current player to speak the sentence.
		* e.g.: "this is a demo." "\say hello!"
		*/
		string sDialog;
		DialogKey(const string& s):sDialog(s){};
		DialogKey(){};
	};
	/** refer to autocamera */
	struct CameraKey
	{
		float fRotY;
		float fLiftUpAngle;
		float fDist;
		CameraKey(float rotY, float LiftUpAngle, float Dist):fRotY(rotY), fLiftUpAngle(LiftUpAngle), fDist(Dist){};
	};
	/** the missile and magic effect movie key frame. */
	struct EffectKey
	{
		/** effect ID in the effect database*/
		int nEffectID;
		/** target name, it can a global biped name or <AttachmentID>. 
		For more information, please see the ParaCharacter::CastEffect();*/
		string sTarget;
	public:
		EffectKey(int effectid, const string& target);
		EffectKey(int effectid, const char* target);
	};
	/**
	* key for actions and animations. The key is internally stored as action string.
	* if the action string is a number, it is treated as the ID of the action in AnimTable, such as "11","0","123"
	* if the action string is not a number(i.e. failed convert to number using atoi()), then it is treated as the name
	* of the action, and the AnimTable is used to retrieve the animation ID for the name. If there  is no valid animation 
	* ID, 0 is returned.
	* some action string is reserved. they are "_toggletowalk","_toggletorun","_mount".
	*/
	struct ActionKey
	{
		const static int MAX_ACTIONNAME_LEN = 16;
		string sActionName;
		enum KeyID
		{
			TOGGLE_TO_WALK=0,
			TOGGLE_TO_RUN,
			JUMP,
			MOUNT, // mount on the nearest object
			NONE
		};
	public:
		ActionKey(const char* name);
		ActionKey(const std::string& name);
		ActionKey(KeyID keyID);
		ActionKey(int nAnimID);

		/** get the string of the action key.*/
		const char* ToString()const;

		/** get the animation ID,please refer to the animation table. 
		* if the action string is a number, it is treated as the ID of the action in AnimTable, such as "11","0","123"
		* if the action string is not a number(i.e. failed convert to number using atoi()), then it is treated as the name
		* of the action, and the AnimTable is used to retrieve the animation ID for the name. If there  is no valid animation 
		* ID, 0 is returned.
		*/
		int ToAnimID() const;

		/** whether this is a jump action key. whether the action string is "js" (jump start). */
		bool IsJump() const;
		/** whether this is a key that set the default moving style as walking. whether the action string is "_toggletowalk". */
		bool IsToggleToWalk() const;
		/** whether this is a key that set the default moving style as running. whether the action string is "_toggletorun".*/
		bool IsToggleToRun() const;
		/** whether this is a key that mount the character object on nearest object. whether the action string is "_mount".*/
		bool IsMount() const;
	};

}