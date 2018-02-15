#pragma once
#include "BipedController.h"

namespace ParaEngine
{
	/** a movie controller 
	* it will control the biped to follow a path defined in a movie file or record the biped' actions to a file. It will also 
	* play animations at certain time. In short, it is a movie player and recorder. 
	* if it is still recording without saving the file when exits, we will save the movie to a default file.
	* at "script/movie/movie_recorded.txt"
	*/
	class CMovieCtrler : public CBipedController
	{
	public:
		CMovieCtrler(void);
		CMovieCtrler(CAIBase* pAI);
		virtual ~CMovieCtrler(void);
	private:
		/** Position key frames of the movie 
		* if one want to define a looped movie, just insert the first frame to the end of the movie key frames. 
		* so that it will loop from the beginning.
		*/
		MovieKeyFrame<PosKey> m_keyframesPos;
		/** all positions in the position key frames are relative to this location.*/
		Vector3 m_vPosOffset;
		/** action key frames of the movie 
		*/
		MovieKeyFrame<ActionKey> m_keyframesAction;
		/** effect key frames for the current biped. */
		MovieKeyFrame<EffectKey> m_keyframesEffect;
	
		/// last action key which is played
		int m_nLastAction;
		/// last effect key which is played
		int m_nLastEffect;
		/// last animation state as seen by the recorder
		DWORD m_nLastAnimState;
		/** whether the offset position is set. */
		bool m_bHasOffsetPosition:1;
		/** whether the offset position is set. */
		bool HasOffsetPosition();
		/** dialog key frames of the movie 
		*/
		MovieKeyFrame<DialogKey> m_keyframesDialog;
		/** last dialog index*/
		int m_nLastDialog;

	public:
		virtual void SetTime(float fTime);
		virtual void Resume();
		virtual void Suspend();

		/** Record a dialog key at the specified time 
		* please note that we will only record if it is not playing.
		* @param sDialog: the dialog text to be added
		* @param fTime: the time at which to insert the key frame. 
		* If this value is -1.0f, it will be appended to the current recording time.
		*/
		void RecordDialogKey(const string& sDialog, float fTime = -1.0f);

		/** Record a action key at the specified time 
		* please note that we will only record if it is not playing.
		* @param sActionName: the action name, it can be animation ID in string format, or it can be the 
		*		name of the animation as defined in the animation table in AnimTable.
		* @param fTime: the time at which to insert the key frame. 
		* If this value is -1.0f, it will be appended to the current recording time.
		* @return: return true if recorded.
		*/
		bool RecordActionKey(const string& sActionName, float fTime = -1.0f);
		bool RecordActionKey(const ActionKey* actionKey, float fTime = -1.0f);
		
		/** record the effect key, such as magic and missile effect 
		* @param fTime: the time at which to insert the key frame. 
		* If this value is -1.0f, it will be appended to the current recording time.
		*/
		void RecordEffectKey(int effectID, const string& sTarget, float fTime = -1.0f);

		/** Display a dialog */
		void DisplayDialog(const string& sDialog);

		/** Set the position offset. 
		* all positions in the position key frames are relative to this location.*/
		void SetPosOffset(const Vector3& vPos);
		/** get the offset position.*/
		const Vector3& GetPosOffset();
		/**
		* load a movie from file. The internal timer will be set to zero. 
		* call SetTime() to play from a specified time. Setting negative time will delay
		* the movie from playing. Setting a value which is out side the range of the key frames
		* will default to either the start or end of the key frame. 
		* the file format is given below:
		* time [pos(x, y, z, facing)] [actionname]\r\n
		* time\r\nDialog\r\r
		* e.g. "0 pos(100,0,90, 3.14) j\r\n", "2 pos(100,0,90, 3.14)\r\n", "5 e_d\r\n" "5\r\nHello\r\r"
		* Note: the time series should generally be in increasing order, except for the last frame
		* the last frame can be a rewinding frame, which tells the movie to roll back to a previous frame
		* for example:if t->{0,1,(2),3,(4),(2)}, then suppose the movie is played from 0, it will be a looping movie
		* with time series as {0,1,2,3,4,2,3,4,2,3,4, ...}. To play consistent movie (2)(4)(2) in the time series should be identical.
		* Note1: the position offset is set to the current location of the biped.
		* @param filename: a file name from which to load the file. It can also have a position offset value
		* in the format:"filename pos(x,y,z)"| "filename".
		* e.g. LoadFromFile("moive/movie1.txt pos(100,0,100)");
		*  LoadFromFile("moive/movie1.txt"); If the position offset is not specified. it is assumed to be the current 
		*  position of the associated biped.If specified, it is the current biped's position plus this offset value
		*/
		void LoadFromFile(const std::string& filename);

		enum MovieMode
		{
			MOVIE_PLAYING,
			MOVIE_RECORDING,
			MOVIE_SUSPENDED
		};
		MovieMode	m_currentMode;

		/** set the mode for the current movie: there are 4 modes: Playing, Recording, Suspended.
		* once playing or recording mode is set, the local movie time is set to 0 again.
		* if we set to recording mode, the offset position will be set to the biped's current position 
		* when this function is called. */
		void SetMode(MovieMode mode);

		/**
		* Save the key frames previously recorded to a specified file.
		* @param filename: file path.
		* @see: LoadFromFile() for information on the output file format.
		*/
		void SaveToFile(const std::string& filename);
		/**
		* It will add an additional key frame to the end, which point to the beginning of the animation.
		* @param fLoopToTime : to which time to loop to 
		*/
		void AddLoopingFrame(float fLoopToTime = 0.f);

		/** a virtual function which is called every frame to process the controller. 
		* @param fDeltaTime: the time elapsed since the last frame move is called. 
		* @param pInput: It holds any information that is perceived by a Active Biped object
		*/
		virtual void FrameMove(float fDeltaTime);
	};
}