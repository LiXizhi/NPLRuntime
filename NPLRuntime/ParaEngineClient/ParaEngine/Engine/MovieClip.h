#pragma once
#include <string>
namespace ParaEngine
{
	using namespace std;

	/** It presents a script at a certain time. */
	struct SubScript
	{
		union{
			int m_nFrameOffset;
			float m_fTimeOffset;
		};
		string m_text;
	};
	/** movie sub scripts */
	typedef list<SubScript> MovieSubScripts;
	
	/** movie clip types*/
	enum MOVIE_CLIP_TYPES{
		movieclip_base=0,
		movieclip_screen_capture,
	};
	/** it represents a clip of movies, which may be screen captured or just some transition effects. */
	class CMovieClipBase
	{
	public:
		CMovieClipBase(void);
		virtual ~CMovieClipBase(void);
		/** get the clip type*/
		virtual MOVIE_CLIP_TYPES GetType(){return movieclip_base;};
	public:
		string GetFileName(){return m_filename;};
		void SetFileName(string filename){m_filename = filename;};

		string GetClipTitle(){return m_title;};
		void SetClipTitle(string title){m_title= title;};

		/** get the number of frames in the clips */
		int GetFrameCount(){return m_nFrameCount;};
		/** clip width in pixel*/
		int GetClipWidth(){return m_nClipWidth;}
		/** clip height in pixel*/
		int GetClipHeight(){return m_nClipHeight;}
	private:
		/** captured file name*/
		string m_filename;
		/** the clip title */
		string m_title;
		/** number of frames in the clips */
		int m_nFrameCount;
		/** clip width in pixel*/
		int m_nClipWidth;
		/** clip height in pixel*/
		int m_nClipHeight;
		/** movie scripts */
		MovieSubScripts m_scripts;
	};

	/** it represents a screen captured screen movie. */
	class CMovieClipScreenCapture: CMovieClipBase
	{
	public:
		CMovieClipScreenCapture(void);
		virtual ~CMovieClipScreenCapture(void);
		virtual MOVIE_CLIP_TYPES GetType(){return movieclip_screen_capture;};
	
	};

}
