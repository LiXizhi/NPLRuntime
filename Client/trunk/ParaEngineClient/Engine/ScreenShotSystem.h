//::|сссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссс|::
//::|сс ---------------------------------------------------------------------------------- сс|::
//::|сс|						~ A R I E L   P R O D U C T I O N S ~                     |сс|::
//::|сс ---------------------------------------------------------------------------------- сс|::
//::|сс| ~ PROGRAM DATA ~													      /-----/ |сс|::
//::|сс|							            								/-----/ | |сс|::
//::|сс| PROGRAMMER:: James Dougherty (UltimaX)									| |   | | |сс|::
//::|сс| COPYRIGHT :: ?003 Ariel Productions									| |   | | |сс|::
//::|сс| TYPE      :: Screen Shot System										| /---|-/ |сс|::
//::|сс|																		/-----/   |сс|::
//::|сс ---------------------------------------------------------------------------------- сс|::
//::|сс|																				  |сс|::
//::|сс| FILENAME	   :: ScreenShotSystem.h											  |сс|::
//::|сс| VERSION	   :: 1.1															  |сс|::
//::|сс| REVISION DATE :: 09.28.03														  |сс|::
//::|сс|																				  |сс|::
//::|сс| REVISION DATA																	  |сс|::
//::|сс| | 																				  |сс|::
//::|сс| |-03.13.04																		  |сс|::
//::|сс| |-Fixed bug in video mode														  |сс|::
//::|сс| |																				  |сс|::
//::|сс| |-03.14.04																		  |сс|::
//::|сс| |-Added time stamp to the file name in the screen shot manager					  |сс|::
//::|сс| |																				  |сс|::
//::|сс|																				  |сс|::
//::|сс| BUGS																			  |сс|::
//::|сс| |																				  |сс|::
//::|сс| |-N/A																			  |сс|::
//::|сс| |																				  |сс|::
//::|сс|																				  |сс|::
//::|сс ---------------------------------------------------------------------------------- сс|::
//::|сссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссссс|::


#ifndef SCREENSHOTSYSTEM_H
#define SCREENSHOTSYSTEM_H

#include <VFW.h>	// Video for Windows header
#include <string>

/** Video and image capture system.

  Screen Shot System Version 1.0 Features
  ---------------------------------------

	Low Resolution Screen Shots
	High Resolution Screen Shots
	Movie Mode
	Screen Shot File Management
	Watermarking System (Even In Movie Mode)

  Descriptions
  ------------

	--Low Resolution Screen Shots
		Low resolution screen shots will take a screen shot of the current rendered
		scene. It will be the size of the window. So if the window is 800x600 it will
		produce an 800x600 screen shot. This is good for web pages, e-mail, and scaling
		it down to make a splash screen for your game. These are not recommended for
		screen shots that are going to be featured in a magazine, book, etc.

	--High Resolution Screen Shots
		High resolution screen shots are the same as the low resolution screen shots
		except one major difference. Instead of producing an 800x600 screen shot, it
		will become 2400 x 1800. If the window dimensions are 1024x768, it will
		produce a screen shot thats 3072x2304. These are good for magazines, books, etc.

	--Movie Mode
		This will do as it says. It will make an avi video while you run your game or demo.
		A little warning: It will make your game run very slow. When you use a codec, it has
		to take a screen shot, apply the watermark, compress the screen shot, and then it will
		finally write the current frame to the avi. I tested this with my terrain engine.
		The terrain engine was running at 400+ FPS. When I was recording it though, it ran
		around 10 FPS. Good thing it doesn't happen in the final production huh :) If you
		record the video with no codecs (Uncompressed) it will run 4x as fast, but the video
		will be 10x as big. My 2 favorite codecs I found for this is DIVX and Intel Indeo
		Video 4.5, they produce decent quality while keeping the file size low. A video thats
		5 MB with DIVX would be around 80-100 MB uncompressed.

	--Screen Shot/Movie File Management
		This will manage the screen shot files by making sure no saved screen shots get
		written over. Say you make a screen shot named "SlayerSS.bmp" then you take another
		screen shot right after the first one. The first one would be overwritten if no file
		manager was being used. The file manager would check and see that "SlayerSS.bmp"
		already exists, and will name it "SlayerSS_001.bmp" so it will not get overwritten.
		If "SlayerSS_001.bmp" exists, it will make a new one called "SlayerSS_002.bmp" and
		so on.

	--Watermarking System (Even In Movie Mode)
		This will add a watermark to the screen shot before it is saved. I thought it
		would be pretty simple, but it turned out to be tricky. (And very fun :)) Anyway,
		the watermark can be placed in any corner of the screen shot. You can also specify
		the strength of the watermark. The strength is from 0-1. If it's 0, none of the watermark
		will show up. If it's 1, the watermark will just be placed in the specified location.
		If there is a value in between 0 and 1, it will be blended into the screen shot using
		a simple linear interpolation. Think of the strength as the alpha component.
*/
namespace ScreenShot
{
	using namespace std;
	
	///Access to the class through the singleton
	#define GSSHOTSYSTEM SCREENSHOTSYSTEM::GetClassInstance()

	enum SCREENSHOTRESOLUTION
	{
		SSR_LORES = 0x00, ///Low Resolution Screen Shot
		SSR_HIRES = 0x01  ///High Resolution Screen Shot
	};

	enum WATERMARKPOSITION
	{
		WP_TOPLEFT	   = 0x00, ///Watermark will be placed in the top left corner of the screen shot
		WP_TOPRIGHT	   = 0x01, ///Watermark will be placed in the top right corner of the screen shot
		WP_BOTTOMLEFT  = 0x02, ///Watermark will be placed in the bottom left corner of the screen shot
		WP_BOTTOMRIGHT = 0x04  ///Watermark will be placed in the bottom right corner of the screen shot
	};

	///Simple structure to specify the watermarks transparent color
	///
	struct WATERMARKTRANSPARENTCOLOR
	{
		WATERMARKTRANSPARENTCOLOR(){}
		WATERMARKTRANSPARENTCOLOR(unsigned char C){Red = Green = Blue = C;}
		WATERMARKTRANSPARENTCOLOR(unsigned char R, unsigned char G, unsigned char B){Red = R; Green = G; Blue = B;}

		unsigned char Red;
		unsigned char Green;
		unsigned char Blue;
	};

	///
	/// ~ The watermark structure is for internal use ~
	///	  ~ There is no need to use it at all ~
	///
	struct WATERMARK
	{
		unsigned char*    Data;        ///The watermark pixel data
		bool		      Valid;       ///If the watermark is valid
		int			      Width;       ///The watermark's width
		int			      Height;      ///The watermark's height
		float			  Strength;    ///The strength of the the watermark (How visible it is)
		WATERMARKPOSITION Position;	   ///The watermark's position [DEFAULT: WP_BOTTOMLEFT]
		bool			  Transparent; ///If the watermark should be transparent. If this is set
									///to true, it will read the fist pixel and make that the
									///the transparent color.
	};
	
	/**
	the screen shot system main class
	*/
	class SCREENSHOTSYSTEM
	{
	private:
		struct VIDEODATA
		{
			PAVISTREAM	  Stream;			   ///The videos stream
			bool		  VideoOK;			   ///Make sure the video process has no errors
			PAVIFILE	  VideoFile;		   ///The video file
			unsigned int  VideoWidth;	       ///The videos frame width
			unsigned int  VideoHeight;	       ///The videos frame height
			int			  NumberOfFrames;	   ///Current number of frames in the video
			AVISTREAMINFO StreamInformation;   ///The video streams information
			PAVISTREAM    CompressionStream;   ///The videos compression stream
			bool		  AllowCodecSelection; ///Allows manual selection of the codec
			bool		  UseTransparentColor; ///Using a transparent color in the watermark
			WATERMARKTRANSPARENTCOLOR TransparentColor; ///The watermarks transparent color
		};

	private:
		VIDEODATA	   m_Video;			   ///The video data
		char*          m_TimeStamp;		   ///Used to hold the time stamp
		WATERMARK	   m_Watermark;		   ///The watermark data
		bool		   m_InProgress;	   ///Flag to see if the system is currently taking a screen shot
		bool		   m_ApplyWatermark;   ///Flag to see if a watermark needs added
		unsigned char* m_HRScreenShotData; ///High resolution screen shot data
		bool		   m_bFromTexture;
		LPDIRECT3DTEXTURE9 m_pTexture;
		LPDIRECT3DSURFACE9 m_pOffScreenSurface;
		D3DFORMAT	m_TextureFormat;
		int			m_TextureWidth,m_TextureHeight;
		int			m_eCodec;
		string		m_sVideoFileName; /// current file name
		string		m_sCodecOptionFile;

		/** 
		*  - 0 for disable stereo capture(default); 
		*  - 1 for line interlaced stereo.
		*  - 2 for left right stereo; 
		*  - 3 for above below stereo;
		*  - 4 for frame interlaved mode, where the odd frame is the left eye and even frame is the right image; 
		*/
		int m_nStereoCaptureMode;

	private:
		SCREENSHOTSYSTEM();

		int	GetValidDeviceHeight();
		int	GetValidDeviceWidth();


		///This function captures the screen and returns the pointer to the data
		///Used for movie mode
		void* CaptureScreen(bool bCaptureMouse=true);

		///This function captures the screen and returns the pixel data
		///
		void CaptureScreen(unsigned char* ScreenData,bool bCaptureMouse=true);

		/**
		* @param pBmpBuffer: [out] bmp data in buffer.
		* @param nMode: 0 full image is written. 2 : only even lines are written. 3: only odd lines are written
		*/
		void CaptureTexture(byte** pBmpBuffer, int nMode=0);

		///This will see if a screen shot exists in the screen shot directory
		///with the same name as the query filename.
		bool ScreenShotExists(const char* Filename);

		///This will get the current time stamp for the screen shot manager
		void GetTimeStamp(void);
		
		/** display a dialog to select options, and save to the given file.*/
		bool SaveCodecOptions(const char* szFilename, const AVICOMPRESSOPTIONS& CompressionOptions);

		void LoadCodecOptions(const char* szFilename,void* data, DWORD *size);
		///This will strip any path from the filename.
		///This is needed to ensure every screen shot goes to the
		///screen shot directory.
		void StripDirectoryFromFilename(char* Filename);

		///This will strip the extension off of the filename if there is one.
		///Filenames should be passed in with no extension. There's a couple
		///of reasons for this. 1) Right now I only have it saving the screen
		///shots as a bitmap. 2) If the system needs to add the _[###] to the filename,
		///it would have to strip the extension.
		void StripExtensionFromFilename(char* Filename);

		///This is where the screen shots gets taken. This will pass the filename to the
		///screen shot manager functions (As described in the article) so we have a valid
		///filename. Once a valid filename is obtained, it will take the screen shot and
		///save it to the 'Screen Shots' directory.
		void SnapScreenShot(const char* Filename, SCREENSHOTRESOLUTION& Resolution, int ResolutionID, bool UseTransparentColor, WATERMARKTRANSPARENTCOLOR& TransparentColor = WATERMARKTRANSPARENTCOLOR(0));

		///This will load a watermark...
		void LoadWatermark(WATERMARK* Watermark, const char* Filename, BITMAPINFOHEADER *HeaderInformation);

		///This function will check the 'Screen Shots' directory and see if the same screen shot
		///exists. If the same screen shot exists, it will modify the filename and return a valid
		///filename. This way multiple screen shots can be taken without replacing the same screen
		///shot every time.
		void GetValidFilename(char* ValidFilename, char* Filename, bool InMovieMode = false);

		///This function will put the watermark on the screen shot
		void ApplyWatermark(WATERMARK* Watermark, unsigned char* ScreenData, SCREENSHOTRESOLUTION& Resolution, int ResolutionID, bool UseTransparentColor, WATERMARKTRANSPARENTCOLOR& TransparentColor = WATERMARKTRANSPARENTCOLOR(0));


	public:
		virtual ~SCREENSHOTSYSTEM();
		static SCREENSHOTSYSTEM *GetClassInstance();

	public:
		enum USING_CODEC_TYPE{
			CODEC_XVID,CODEC_WMV
		};
		string GetVideoFileName(){return m_sVideoFileName;};
		///Lets you know if the system is currently taking screen shots.
		///This is good if you don't want data displayed during the progress
		///For instance, you could could do something like:
		///
		///if(!GSSHOTSYSTEM->InProgress())
		///{
		///		DisplayFPS();
		///}
		bool InProgress();

		void SetCaptureTexture(LPDIRECT3DTEXTURE9 pTexture);

		///This will produce a low resolution screen shot.
		///Quick, easy, and no hassle. Just supply a filename
		///and will do the rest.
		void TakeScreenShot(const char* Filename);

		///This will produce a low resolution screen shot with a watermark.
		///The watermark must be a true color bitmap
//		void TakeScreenShotW(char* Filename, char* Watermark, WATERMARKPOSITION WatermarkPosition = WP_TOPRIGHT, float Strength = 0.2f, bool UseTransparentColor = false, WATERMARKTRANSPARENTCOLOR TransparentColor = WATERMARKTRANSPARENTCOLOR(0));

		///This function will produce a high resolution screen shot or low resolution screen shot.
		///The 'void (*RenderFunction)(void)' is the function you render with. For those that might
		///have guessed, on high resolution screen shots, it will render the scene 9 times. Finally
		///just let it know the resolution.
//		void TakeScreenShot(char* Filename, void (*RenderFunction)(void), SCREENSHOTRESOLUTION Resolution = SSR_LORES);

		///This function will produce a high resolution screen shot or low resolution screen shot with a watermark.
		///The 'void (*RenderFunction)(void)' is the function you render with. For those that might
		///have guessed, on high resolution screen shots, it will render the scene 9 times. Finally
		///just let it know the resolution. The watermark must be a true color bitmap
//		void TakeScreenShotW(char* Filename, void (*RenderFunction)(void), SCREENSHOTRESOLUTION Resolution = SSR_LORES, char* Watermark = NULL, WATERMARKPOSITION WatermarkPosition = WP_BOTTOMLEFT, float Strength = 0.2f, bool UseTransparentColor = false, WATERMARKTRANSPARENTCOLOR TransparentColor = WATERMARKTRANSPARENTCOLOR(0));

		/** This will setup the video settings and create the video file
		* anything UpdateMovieCapture() calls between BeginMovieCapture() and EndMovieCapture() will be saved to one file.
		*/
		bool BeginMovieCapture(const char* Filename, unsigned int FramesPerSecond = 20, bool AllowCodecSelection = false, bool EnableInProcessFlag = false);

		///This will setup the video settings and watermark settings then create the video file
	//	void BeginMovieCaptureW(char* Filename, bool AllowCodecSelection = true, bool EnableInProcessFlag = false, char* Watermark = NULL, WATERMARKPOSITION WatermarkPosition = WP_BOTTOMLEFT, float Strength = 0.2f, bool UseTransparentColor = false, WATERMARKTRANSPARENTCOLOR TransparentColor = WATERMARKTRANSPARENTCOLOR(0));

		/** This function will write the current screen shot to the avi file.
		* if m_Video.NumberOfFrames == 0 when calling this function, it will open dialog to allow the user to specify codec. 
		* hence the first frame (m_Video.NumberOfFrames == 0) will not be rendered.
		* @param FramesPerSecond: this is only used when m_Video.NumberOfFrames == 0
		* @param nMode: 0 normal capture, 2 even lines, 3 odd lines. 4 only left, 5 only right, 6 only above, 7 only below
		* @return:  true if succeed. and false if there is an error or the user canceled recording in the first frame.
		*/
		bool UpdateMovieCapture(unsigned int FramesPerSecond = 0, int nMode = 0);

		///This function will close all of the video streams and clear them
		void EndMovieCapture();
		void SetUsingCodec(int etype);
		int GetCodec();

		/** return true if there is a file called Screen Shots\\codec_options.bin, which contains the last codec options of the user.*/
		bool HasLastCodec();
		/** display a dialog which allows the user to set the codec to be used. */
		void SelectCodecOptions();

		/** get the current frame number to be recorded. This is usually used for stereo vision capture mode, when the even frame number is left eye, and odd is for right eye.
		* if capture has not begun, it is always 0.
		*/
		int GetCurrentFrameNumber();

		/** 
		* set the stereo capture mode. This is used to generate video files that can be viewed by 3d eye glasses and stereo video player. 
		*  - 0 for disable stereo capture(default); 
		*  - 1 for line interlaced stereo.
		*  - 2 for left right stereo; 
		*  - 3 for above below stereo;
		*  - 4 for frame interlaved mode, where the odd frame is the left eye and even frame is the right image; 
		*/
		void SetStereoCaptureMode(int nMode) {m_nStereoCaptureMode = nMode;};

		/** 
		* Get the stereo capture mode. This is used to generate video files that can be viewed by 3d eye glasses and stereo video player. 
		*  - 0 for disable stereo capture(default); 
		*  - 1 for line interlaced stereo.
		*  - 2 for left right stereo; 
		*  - 3 for above below stereo;
		*  - 4 for frame interlaved mode, where the odd frame is the left eye and even frame is the right image; 
		*/
		int GetStereoCaptureMode(){ return m_nStereoCaptureMode; };
	};
}

#endif ///SCREENSHOTSYSTEM_H