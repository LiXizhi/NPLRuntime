//-----------------------------------------------------------------------------
// Class:	Screen shot system in ParaEngine
// based on code written by James Dougherty (UltimaX)
// notes: Some memory leak is repaired by me (Li, Xizhi) when porting the screenshot class
// Fixes: LiXizhi: the video crash problem has been corrected.
// Adds: LiuWeili: mouse cursor capture supported
//-----------------------------------------------------------------------------


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
//::|сс| FILENAME	   :: ScreenShotSystem.cpp											  |сс|::
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

//-----------------------------------------------------------------------------------------------
//--
//-- THIS IS FREE SOFTWARE
//-- USE IT ANYWAY YOU WANT TO AS LONG AS THESE REQUESTS ARE MET
//-- 
//-- A) The above copyright and this notice must stay intact
//-- B) If you use it, please send me an e-mail. Not only
//--	would I love to hear from you, I'm just curious how
//--	you like it and where it ends up. Also, let me know
//--	what project you use it in.
//--	My e-mail is: arielproductions@zoominternet.net
//-- C) Give me a little credit
//-- D) Enjoy it
//-- E) That's not to much to ask for is it?
//--
//------------------------------------------------------------------------------------------------
#include "ParaEngine.h"
#include "DirectXEngine.h"
#include "ScreenShotSystem.h"
#include "FileManager.h"
#include "SceneState.h"
#include <Math.h>
#include <Time.h>
#include "StringHelper.h"
#pragma comment(lib, "VFW32.lib")

#include "memdebug.h"
using namespace ParaEngine;

namespace ScreenShot
{
//------------------------------------------------------------------------------------------------
//--
//--This needs optimized a lot.
//--There's a lot of duplicate code in some places and the pixel writting can use
//--a little sprucing up. I leave that up to you though.
//--It might not be that big of a deal since a screen shot is quick, but the
//--movie mode would be something to work on.
//--My e-mail is: arielproductions@zoominternet.net
//--Many thanks to everyone at the gamedev forums.
//--
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//--Thanks to Aeroum for the time stamp suggestion.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//--
//--Frustum(*)
//--Version: 1.0
//--
//-- ?001 Alex Vlachos and Evan Hart
//-- Game Programming Gems II PG. 406
//--
//------------------------------------------------------------------------------------------------
void Frustrum(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
	float Width  =  Right - Left;
	float Height =  Top - Bottom;
	float Depth  =  Far - Near;
	float M001   =  2.0f * Near / Width;
	float M002   =  2.0f * Near / Height;
	float M003	 =  (Right + Left) / Width;
	float M004	 =  (Top + Bottom) / Height;
	float M005	 = -(Far + Near) / Depth;
	float M006	 = -1.0f;
	float M007	 = -(2.0f * Far * Near) / Depth;

	//--If the active engine is Direct3D
	{
		Matrix4 Matrix;
		Matrix = Matrix4::IDENTITY;

		//--Set the matrix parameters
		Matrix._11 = M001; Matrix._21 = 0.0f; Matrix._31 = M003; Matrix._41 = 0.0f;
		Matrix._12 = 0.0f; Matrix._22 = M002; Matrix._32 = M004; Matrix._42 = 0.0f;
		Matrix._13 = 0.0f; Matrix._23 = 0.0f; Matrix._33 = M005; Matrix._43 = M007;
		Matrix._14 = 0.0f; Matrix._24 = 0.0f; Matrix._34 = M006; Matrix._44 = 0.0f;

		//--Apply the matrix to the projection matrix
		CGlobals::GetRenderDevice()->SetTransform(D3DTS_PROJECTION, Matrix.GetConstPointer());
	}
}

//------------------------------------------------------------------------------------------------
//--
//--PerspectiveEX(*)
//--Version: 1.0
//--
//-- ?001 Alex Vlachos and Evan Hart
//-- Game Programming Gems II PG. 406
//--
//------------------------------------------------------------------------------------------------
void PerspectiveEX(float FOV, float Aspect, float Near, float Far, int Subrect)
{
	float Angle =   (FOV * 0.017453292519943295769236907684883f) * 0.5f;
	float Top   =   Near / ((float)cos(Angle) / (float)sin(Angle));
	float Bottom = -Top;
	float Right  =  Top * Aspect;
	float Left   = -Right;

	if(Subrect == -1)
	{
		Frustrum(Left, Right, Bottom, Top, Near, Far);
	}
	else if(Subrect == 0)
	{
		Frustrum(Left, Left / 3.0f, Top / 3.0f, Top, Near, Far);
	}
	else if(Subrect == 1)
	{
		Frustrum(Left / 3.0f, Right / 3.0f, Top / 3.0f, Top, Near, Far);
	}
	else if(Subrect == 2)
	{
		Frustrum(Right / 3.0f, Right, Top / 3.0f, Top, Near, Far);
	}
	else if(Subrect == 3)
	{
		Frustrum(Left, Left / 3.0f, Bottom / 3.0f, Top / 3.0f, Near, Far);
	}
	else if(Subrect == 4)
	{
		Frustrum(Left / 3.0f, Right / 3.0f, Bottom / 3.0f, Top / 3.0f, Near, Far);
	}
	else if(Subrect == 5)
	{
		Frustrum(Right / 3.0f, Right, Bottom / 3.0f, Top / 3.0f, Near, Far);
	}
	else if(Subrect == 6)
	{
		Frustrum(Left, Left / 3.0f, Bottom, Bottom / 3.0f, Near, Far);
	}
	else if(Subrect == 7)
	{
		Frustrum(Left / 3.0f, Right / 3.0f, Bottom, Bottom / 3.0f, Near, Far);
	}
	else if(Subrect == 8)
	{
		Frustrum(Right / 3.0f, Right, Bottom, Bottom / 3.0f, Near, Far);
	}
}

//------------------------------------------------------------------------------------------------
//--
//--SCREENSHOTSYSTEM(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
SCREENSHOTSYSTEM::SCREENSHOTSYSTEM() : m_HRScreenShotData(NULL),
									   m_InProgress(false),m_pOffScreenSurface(NULL),
									   m_TimeStamp(NULL)
{
	//--Setup the watermark data
	m_Watermark.Width    = 0;
	m_Watermark.Height   = 0;
	m_Watermark.Strength = 0.3f;
	m_Watermark.Data     = NULL;
	m_Watermark.Valid    = false;
	m_Watermark.Position = WP_BOTTOMLEFT;

	//--Setup the default video parameters
	m_Video.VideoWidth			= 0;
	m_Video.VideoHeight			= 0;
	m_Video.NumberOfFrames		= 0;
	m_Video.VideoOK			    = true;
	m_Video.AllowCodecSelection = false;

	m_bFromTexture=false;
	m_pTexture=NULL;
	m_eCodec=CODEC_XVID;
	//--Create the "Screen Shots" directory
	//--If it returns zero, the directory most likely already exists.
	//--Just ignore it and continue.
	m_sCodecOptionFile = "Screen Shots\\codec_options.bin";
}

//------------------------------------------------------------------------------------------------
//--
//--~SCREENSHOTSYSTEM(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
SCREENSHOTSYSTEM::~SCREENSHOTSYSTEM()
{
	if(m_Video.VideoOK)
	{
		EndMovieCapture();
	}
	//--Set the needs cleaned flag to true
	static int NeedsCleaned = 1;

	//--If this is the first time calling the destructor (Hopefully the only time)
	if(NeedsCleaned--)
	{
		//--If the watermark is valid and the watermark data is valid
		if(m_Watermark.Valid == true && m_Watermark.Data)
		{
			//--Free the watermark data
			SAFE_DELETE(m_Watermark.Data);
			m_Watermark.Data = NULL;
		}
	}
	SAFE_DELETE(m_TimeStamp);
}

//------------------------------------------------------------------------------------------------
//--
//--GetSystem(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
SCREENSHOTSYSTEM *SCREENSHOTSYSTEM::GetClassInstance()
{
	//--Return the instance of this class (Easy Singleton)
	static SCREENSHOTSYSTEM Instance;
	return &Instance;
}

//------------------------------------------------------------------------------------------------
//--
//--InProgress(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
bool SCREENSHOTSYSTEM::InProgress()
{
	//--Return the 'InProgress' flag
	return m_InProgress;
}

//------------------------------------------------------------------------------------------------
//--
//--ScreenShotExists(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
bool SCREENSHOTSYSTEM::ScreenShotExists(const char* Filename)
{
	//--Make sure a valid filename is passed in
	if(Filename == NULL)
	{
		//--Not valid, return false
		return false;
	}

	//--Try to find the file
	HANDLE FileHandle;
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
	WIN32_FIND_DATAW FindData;
	LPCWSTR Filename16 = StringHelper::MultiByteToWideChar(Filename, DEFAULT_FILE_ENCODING);
	FileHandle = FindFirstFileW(Filename16, &FindData);
#else
	WIN32_FIND_DATA FindData;
	FileHandle = FindFirstFile(Filename, &FindData);
#endif

	//--If the file handle turned out to be INVALID_HANDLE_VALUE,
	//--then the screen shot does not exist
	bool Exists = (FileHandle != INVALID_HANDLE_VALUE ? true : false);

	//--Close the file
	FindClose(FileHandle);

	//--Return the result
	return Exists;
}

//------------------------------------------------------------------------------------------------
//--
//--GetTimeStamp(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::GetTimeStamp(void)
{
	char   Buffer[512];
	time_t Time;

	//--Get the current time
	time(&Time);
	tm* LocalTime = localtime(&Time);

	//--Check if it is afternoon
	if(LocalTime->tm_hour > 12)
	{
		//--If it is subtract 12 hours and then print the current time to the buffer
		sprintf(Buffer, "%d%002d%002d", (LocalTime->tm_hour - 12), LocalTime->tm_min, LocalTime->tm_sec);
	}
	else
	{
		//--Print the current time to the buffer
		sprintf(Buffer, "%d%002d%002d", LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec);
	}

	//--Make sure the previous time buffer is cleared
	SAFE_DELETE(m_TimeStamp);
	
	//--Add the null terminator to the buffer
	Buffer[strlen(Buffer)] = '\0';

	//--Allocate memory for the time stamp
	m_TimeStamp = new char[strlen(Buffer) + 1];

	//--Copy the buffer to the time stamp
	strcpy(m_TimeStamp, Buffer);
}

//------------------------------------------------------------------------------------------------
//--
//--StripDirectoryFromFilename(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::StripDirectoryFromFilename(char* Filename)
{
	char Buffer[256];

	//--Make sure a valid filename is passed in
	if(Filename == NULL)
	{
		return;
	}
	else
	{
		//--Copy the filename to the buffer
		sprintf(Buffer, "%s", Filename);
	}

	//--See if the filename contains a directory separator
	char* InString = strrchr(Buffer, '\\');

	//--It does
	if(InString)
	{
		//--Copy the file title to the filename
		strcpy(Filename, (InString + 1));
		return;
	}

	//--See if the filename contains this kind of directory seperator
	InString = strrchr(Buffer, '/');

	//--It does
	if(InString)
	{
		//--Copy the file title to the filename
		strcpy(Filename, (InString + 1));
	}
}

//------------------------------------------------------------------------------------------------
//--
//--StripExtensionFromFilename(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::StripExtensionFromFilename(char* Filename)
{
	int  Position = 0;
	char Extension[5];
	char Buffer[256];

	//--If the filename is valid and it length is >= 4
	if(Filename && strlen(Filename) >= 4)
	{
		//--Copy the last 4 characters the the extension
		strcpy(Extension, &Filename[strlen(Filename) - 4]);

		//--Add the null terminator
		Extension[strlen(Extension)] = '\0';
	}
	else
	{
		//--Not valid, return
		return;
	}

	//--If it has a valid extension on it
	if(Extension[0] == '.')
	{
		//--Loop through every character except the extension
		for(unsigned int ID = 0; ID < strlen(Filename) - 4; ID++)
		{
			//--Copy the current character to the buffer
			Buffer[Position] = Filename[ID];

			//--Accumulate the buffer position
			Position++;
		}

		//--Add the null terminator
		Buffer[Position] = '\0';

		//--Copy the modified character array to the filename
		strcpy(Filename, Buffer);
	}
}

//------------------------------------------------------------------------------------------------
//--
//--GetValidFilename(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::GetValidFilename(char* ValidFilename, char* Filename, bool InMovieMode)
{
	int NewIndex = 2;

	//--Make sure the filenames are valid
	if(ValidFilename == NULL || Filename == NULL)
	{
		return;
	}

	//--Get the current time stamp
	GetTimeStamp();

	if(InMovieMode == true)
	{
		//--Try this filename to see if it exists
		sprintf(ValidFilename, "Screen Shots\\%s_%0003d_%s.avi", Filename, NewIndex - 1, m_TimeStamp);
	}
	else
	{
		//--Try this filename to see if it exists
		sprintf(ValidFilename, "Screen Shots\\%s_%0003d_%s.jpg", Filename, NewIndex - 1, m_TimeStamp);
	}

	//--See if this screen shot already exists
	if(ScreenShotExists(ValidFilename) == true)
	{
		//--Keep looping until we get a valid filename
		while(ScreenShotExists(ValidFilename) == true)
		{
			if(InMovieMode == true)
			{
				//--Get the current time stamp (Seconds Count!)
				GetTimeStamp();

				//--Create the new filename as discussed in the article _[###]
				sprintf(ValidFilename, "Screen Shots\\%s_%0003d_%s.avi", Filename, NewIndex, m_TimeStamp);
			}
			else
			{
				//--Get the current time stamp (Seconds Count!)
				GetTimeStamp();

				//--Create the new filename as discussed in the article _[###]
				sprintf(ValidFilename, "Screen Shots\\%s_%0003d_%s.jpg", Filename, NewIndex, m_TimeStamp);
			}

			//--Accumulate the index
			NewIndex++;
		}
	}
}

//------------------------------------------------------------------------------------------------
//--
//--SaveScreenShot(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
inline void SaveScreenShot(char* Filename, int Width, int Height, int Channels, unsigned char* ScreenData, bool ConvertToBGR = true)
{
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	HRESULT hr = D3DXCreateTexture(CGlobals::GetRenderDevice(), Width, Height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pTexture);
	if(FAILED(hr))
	{
		OUTPUT_LOG("failed creating snap shot texture\n");
	}
	else
	{
		D3DLOCKED_RECT lockedRect;
		pTexture->LockRect(0, &lockedRect, NULL, 0);
		
		byte*pp = (byte*)lockedRect.pBits;
		int index = 0, x = 0, y = 0;
		
		for (y=Height-1; y >=0 ; y--)
		{ // correct upside down
			for (x=0; x < Width; x++)
			{
				int n = (y*3*Width)+3*x;	
				// Convert BGR to RGB
				pp[index++] = ScreenData[n+2]; 
				pp[index++] = ScreenData[n+1];
				pp[index++] = ScreenData[n];
				pp[index++] = 0xff;
			}
			index += lockedRect.Pitch - (Width*4);
		}
		pTexture->UnlockRect(0);

		
		string sExt = CParaFile::GetFileExtension(Filename);

		D3DXIMAGE_FILEFORMAT FileFormat = D3DXIFF_PNG;

		if(sExt == "dds")
		{
			FileFormat = D3DXIFF_DDS;
		}
		else if(sExt == "jpg")
		{
			FileFormat = D3DXIFF_JPG;
		}
		else if(sExt == "bmp")
		{
			FileFormat = D3DXIFF_BMP;
		}
		else if(sExt == "tga")
		{
			FileFormat = D3DXIFF_TGA;
		}
		else // if(sExt == "png")
		{
			FileFormat = D3DXIFF_PNG;
		}

		// this will create the directory is not ready.
		if(CParaFile::CreateDirectory(Filename))
		{
			// save texture
			
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
			LPCWSTR Filename16 = StringHelper::MultiByteToWideChar(Filename, DEFAULT_FILE_ENCODING);
			D3DXSaveTextureToFileW(Filename16, FileFormat, pTexture, NULL);
#else 
			D3DXSaveTextureToFile(Filename, FileFormat, pTexture, NULL);
#endif
		}
		
		
		SAFE_RELEASE(pTexture);
	}
	
#ifdef OLD_CODE_AS_BMP

	BITMAPINFOHEADER HeaderInformation;
	BITMAPFILEHEADER Header;
	unsigned char    Pixel;

	//--Open/Create the new bitmap 
	FILE *File = fopen(Filename, "wb");

	//--If there was an error trying to create the bitmap
	if(File == NULL)
	{
		//--Just return
		return;
	}

	//--Set the bit depth
	int BitDepth = (Channels == 3 ? 24 : 32);

	//--Fill in the bitmaps file header
	Header.bfSize = sizeof(BITMAPFILEHEADER);
	Header.bfType = 0x4D42;
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//--Fill in the bitmaps information header
	HeaderInformation.biSize = sizeof(BITMAPINFOHEADER);
	HeaderInformation.biPlanes = 1;
	HeaderInformation.biClrUsed = 0;
	HeaderInformation.biWidth = Width;
	HeaderInformation.biHeight = Height;
	HeaderInformation.biClrImportant = 0;
	HeaderInformation.biXPelsPerMeter = 0;
	HeaderInformation.biYPelsPerMeter = 0;
	HeaderInformation.biBitCount = BitDepth;
	HeaderInformation.biCompression = BI_RGB;
	HeaderInformation.biSizeImage = Width * Height * Channels;

	//--If convert to RGB
	if(ConvertToBGR == true)
	{
		//--Loop through all of the pixels in the screen data
		for(unsigned long PixelID = 0; PixelID < HeaderInformation.biSizeImage; PixelID += Channels)
		{
			//--Swap them to be BGR instead of RGB
			Pixel = ScreenData[PixelID];
			ScreenData[PixelID + 0] = ScreenData[PixelID + 2];
			ScreenData[PixelID + 2] = Pixel;
		}
	}

	//--Write the bitmap file header
	fwrite(&Header, 1, sizeof(BITMAPFILEHEADER), File);

	//--Write the bitmap information header
	fwrite(&HeaderInformation, 1, sizeof(BITMAPINFOHEADER), File);

	//--Write the screen data
	fwrite(ScreenData, 1, HeaderInformation.biSizeImage, File);

	//--Close the bitmap file
	fclose(File);
#endif
}

//------------------------------------------------------------------------------------------------
//--
//--LoadWatermark(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::LoadWatermark(WATERMARK* Watermark, const char* Filename, BITMAPINFOHEADER *HeaderInformation)
{
	BITMAPFILEHEADER Header;
	unsigned char    Pixel;

	//--See if the filename is valid
	if(Filename == NULL)
	{
		//--Return NULL
		return;
	}

	//--If the memory for the watermark could not be allocated
	if(Watermark != NULL)
	{
		if(Watermark->Valid == true && Watermark->Data)
		{
			//--Free the watermark data
			SAFE_DELETE(Watermark->Data);
		}
		Watermark->Valid = false;
	}

	//--Open the bitmap
	
#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR Filename16 = StringHelper::MultiByteToWideChar(Filename, DEFAULT_FILE_ENCODING);
	FILE* File = _wfopen(Filename16, L"rb");
#else
	FILE* File = fopen(Filename, "rb");
#endif

	//--If the bitmap couldn't be opened
	if(File == NULL)
	{
		//--Display an error message
		char Buffer[256];
		sprintf(Buffer, "Unable to load the texture: %s", Filename);
		MessageBox(NULL, Buffer, "[ERROR]", MB_OK);

		//--Set the watermark as invalid
		Watermark->Valid = false;

		//--Return NULL
		return;
	}

	//--Read the bitmap file header
	fread(&Header, sizeof(BITMAPFILEHEADER), 1, File);
	if(Header.bfType != 0x4D42)
	{
		//--Clost the file
		fclose(File);

		//--Set the watermark as invalid
		Watermark->Valid = false;

		//--Return NULL
		return;
	}

	//--Read the bitmap information header
	fread(HeaderInformation, sizeof(BITMAPINFOHEADER), 1, File);

	//--Seek to the pixel data
	fseek(File, Header.bfOffBits, SEEK_SET);

	//--Allocate memory for the watermark data
	Watermark->Data = new BYTE[HeaderInformation->biSizeImage * 3];

	//--If the memory for the watermark data could not be allocated
	if(!Watermark->Data)
	{
		//--Clost the file
		fclose(File);
		
		//--Set the watermark as invalid
		Watermark->Valid = false;

		//--Return NULL
		return;
	}

	//--Read the water data
	fread(Watermark->Data, 1, HeaderInformation->biSizeImage, File);

	//--If the watermark data could not be read from the file
	if(Watermark->Data == NULL)
	{
		//--Clost the file
		fclose(File);

		//--Set the watermark as invalid
		Watermark->Valid = false;

		//--Return NULL
		return;
	}

	//--Loop through every pixel in the watermark
	for(unsigned long PixelID = 0; PixelID < HeaderInformation->biSizeImage; PixelID += 3)
	{
		//--And convert the pixels to RGB instead of BGR
		Pixel = Watermark->Data[PixelID];
		Watermark->Data[PixelID + 0] = Watermark->Data[PixelID + 2];
		Watermark->Data[PixelID + 2] = Pixel;
	}

	//--Clost the file
	fclose(File);

	//--Set the watermark as invalid
	Watermark->Valid = true;

	//--Store the width and height
	Watermark->Width  = HeaderInformation->biWidth;
	Watermark->Height = HeaderInformation->biHeight;
}

//------------------------------------------------------------------------------------------------
//--
//--CaptureScreen(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void* SCREENSHOTSYSTEM::CaptureScreen(bool bCaptureMouse)
{
	int PixelID = 0;
	BITMAP Bitmap;

	unsigned char* MemoryData = NULL;

	//--Get the interface device context
	HDC SourceHDC = GetDC(CGlobals::GetAppHWND());

	//--Create a compatible device context from the interface context
    HDC CompatibleHDC = CreateCompatibleDC(SourceHDC);

	int nWidth = GetValidDeviceWidth();
	int nHeight = GetValidDeviceHeight();

	//--Create a compatible bitmap
    HBITMAP BitmapHandle = CreateCompatibleBitmap(SourceHDC, nWidth, nHeight); 
    
	//--Validate the data
	if(BitmapHandle == 0 || !SelectObject(CompatibleHDC, BitmapHandle))
    {
		return NULL;
    }

	//--Blit the bitmap to the memory device context
    if(!BitBlt(CompatibleHDC, 0, 0, nWidth, nHeight, SourceHDC, 0, 0, SRCCOPY)) 
    {
		return NULL;
    }
	//by lwl, copy the mouse cursor to the bitmap
	if (bCaptureMouse) {
		HCURSOR hc=::GetCursor();
		CURSORINFO cursorinfo;
		ICONINFO iconinfo;
		cursorinfo.cbSize=sizeof(CURSORINFO);
		::GetCursorInfo(&cursorinfo);
		::GetIconInfo(cursorinfo.hCursor,&iconinfo);
		::ScreenToClient(CGlobals::GetAppHWND(),&cursorinfo.ptScreenPos);
		::DrawIcon(CompatibleHDC,cursorinfo.ptScreenPos.x-iconinfo.xHotspot,cursorinfo.ptScreenPos.y-iconinfo.yHotspot,cursorinfo.hCursor);
	}

	//--Get the bitmap
	GetObject(BitmapHandle, sizeof(BITMAP), &Bitmap);

	//--Compute the bitmap size
	unsigned long BitmapSize = sizeof(BITMAPINFOHEADER) + (Bitmap.bmWidth * Bitmap.bmHeight * 3);

	//--Allocate a memory block for the bitmap
	BYTE* MemoryHandle = new BYTE[BitmapSize];
	memset(MemoryHandle, 0, BitmapSize);

	//--Validate the memory handle
	if(!MemoryHandle)
	{
		return NULL;
	}

	//--Setup the bitmap data
	LPBITMAPINFOHEADER WatermarkInformation = (LPBITMAPINFOHEADER)MemoryHandle;
	WatermarkInformation->biSizeImage		= 0;//BitmapSize - sizeof(BITMAPINFOHEADER);
	WatermarkInformation->biSize			= sizeof(BITMAPINFOHEADER);
	WatermarkInformation->biHeight			= Bitmap.bmHeight;
	WatermarkInformation->biWidth			= Bitmap.bmWidth;
	WatermarkInformation->biCompression		= BI_RGB;
	WatermarkInformation->biBitCount		= 24;
	WatermarkInformation->biPlanes			= 1;
	WatermarkInformation->biXPelsPerMeter	= 0;
	WatermarkInformation->biYPelsPerMeter	= 0;
	WatermarkInformation->biClrUsed	  		= 0;
	WatermarkInformation->biClrImportant	= 0;

	//--Get the bitmap data from memory
	if(GetDIBits(CompatibleHDC, BitmapHandle, 0, Bitmap.bmHeight, (unsigned char*)(WatermarkInformation + 1), (LPBITMAPINFO)WatermarkInformation, DIB_RGB_COLORS)==0)
	{
		OUTPUT_LOG("failed to capture screen\n");
	}

	//--Clear the handle to the device context
	DeleteDC(CompatibleHDC);

	//--Delete the bitmap handle to free the resource memory used
    DeleteObject(BitmapHandle);

	//--Return the memory handle
	return MemoryHandle;
}
//------------------------------------------------------------------------------------------------
//--
//--CaptureTexture(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::CaptureTexture(byte** ppBmpBuffer, int nMode)
{
	BYTE  *pBmpBuffer,*pTxtBuffer; // Bitmap buffer, texture buffer
	LONG  lVidPitch,lTxtPitch;                // Pitch of bitmap, texture
	
	BYTE  * pbD = NULL;
	DWORD * pdwS = NULL;
	DWORD * pdwD = NULL;
	int row, col;

	LPDIRECT3DSURFACE9 pSurface = NULL;
	if(FAILED(m_pTexture->GetSurfaceLevel(0, &pSurface)))
		return;
	if(m_pOffScreenSurface!=0)
	{
		if(FAILED(CGlobals::GetRenderDevice()->GetRenderTargetData(pSurface, m_pOffScreenSurface)))
			return;
		SAFE_RELEASE(pSurface);
		pSurface = m_pOffScreenSurface;
	}
	// Lock the Texture
	D3DLOCKED_RECT d3dlr;
	if (FAILED(pSurface->LockRect(&d3dlr, 0, D3DLOCK_READONLY)))
		return;
	// Get the texture buffer & pitch
	pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
	lTxtPitch = d3dlr.Pitch;
	
	//calculate the bitmap pitch
	lVidPitch  = m_TextureWidth * 3 ;

	//allocate the output buffer;
	size_t bmpsize=sizeof(BITMAPINFOHEADER)+m_TextureWidth*m_TextureHeight*3;
	static vector<byte> g_buffer;

	if(g_buffer.size()!=bmpsize)
		g_buffer.resize(bmpsize);

	pBmpBuffer=&(g_buffer[0]);
	*ppBmpBuffer=pBmpBuffer;
	//--Setup the bitmap data
	LPBITMAPINFOHEADER WatermarkInformation = (LPBITMAPINFOHEADER)pBmpBuffer;
	WatermarkInformation->biSizeImage = (int)(bmpsize - sizeof(BITMAPINFOHEADER));
	WatermarkInformation->biSize			= sizeof(BITMAPINFOHEADER);
	WatermarkInformation->biHeight			= m_TextureHeight;
	WatermarkInformation->biWidth			= m_TextureWidth;
	WatermarkInformation->biCompression		= BI_RGB;
	WatermarkInformation->biBitCount		= 24;
	WatermarkInformation->biPlanes			= 1;
	WatermarkInformation->biXPelsPerMeter	= 0;
	WatermarkInformation->biYPelsPerMeter	= 0;
	WatermarkInformation->biClrUsed	  		= 0;
	WatermarkInformation->biClrImportant	= 0;

	pBmpBuffer+=sizeof(BITMAPINFOHEADER);

	// Copy the bits
	if (m_TextureFormat == D3DFMT_X8R8G8B8 || (m_TextureFormat == D3DFMT_A8R8G8B8))
	{
		if(nMode == 4 || nMode == 5)
		{
			// left, right
			int nHalfWidth = m_TextureWidth / 2;
			for( row = m_TextureHeight-1; row>=0; row--)
			{
				pdwS = ( DWORD*)(pTxtBuffer+row*lTxtPitch);
				pbD = pBmpBuffer;

				if(nMode == 5)
				{
					pbD += nHalfWidth*3;
				}
				for( col = 0; col < nHalfWidth; col++ )
				{
					pbD[0] = (BYTE)(pdwS[0] & 0x000000ff);
					pbD[1]=(BYTE)((pdwS[0]&0x0000ff00)>>8);
					pbD[2] = (BYTE)((pdwS[0] & 0x00ff0000) >> 16);
					pdwS+=2;
					pbD+=3;
				}
				pBmpBuffer+=lVidPitch;
			}
		}
		else if(nMode == 6 || nMode == 7)
		{
			if(nMode == 7)
			{
				pBmpBuffer += lVidPitch * m_TextureHeight / 2;
			}
			for( row = m_TextureHeight-1; row>=0; row-=2)
			{
				pdwS = ( DWORD*)(pTxtBuffer+row*lTxtPitch);
				pbD = pBmpBuffer;

				for( col = 0; col < m_TextureWidth; col ++ )
				{
					pbD[0]=pdwS[0]&0x000000ff;
					pbD[1]=(BYTE)((pdwS[0]&0x0000ff00)>>8);
					pbD[2] = (BYTE)((pdwS[0] & 0x00ff0000) >> 16);

					pdwS++;
					pbD+=3;
				}

				pBmpBuffer+=lVidPitch;
			}
		}
		else
		{
			int nHalfHeight = m_TextureHeight / 2;
			for( row = m_TextureHeight-1; row>=0; row--)
			{
				pdwS = ( DWORD*)(pTxtBuffer+row*lTxtPitch);
				pbD = pBmpBuffer;

				bool bWriteLine = false;
				if(nMode == 0)
					bWriteLine = true;
				else// if(nMode == 2 || nMode == 3)
					bWriteLine = (row|0x1)==0;

				if(bWriteLine)
				{
					for( col = 0; col < m_TextureWidth; col ++ )
					{
						pbD[0]=pdwS[0]&0x000000ff;
						pbD[1]=(BYTE)((pdwS[0]&0x0000ff00)>>8);
						pbD[2] = (BYTE)((pdwS[0] & 0x00ff0000) >> 16);

						pdwS++;
						pbD+=3;
					}
				}

				pBmpBuffer+=lVidPitch;
			}
		}
	}
	// Unlock the Texture
	if (FAILED(pSurface->UnlockRect())){
		SAFE_DELETE(pBmpBuffer);
		return;
	}
	if(m_pOffScreenSurface==0)
	{
		SAFE_RELEASE(pSurface);
	}
}

//------------------------------------------------------------------------------------------------
//--
//--CaptureScreen(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::CaptureScreen(unsigned char* ScreenData,bool bCaptureMouse)
{
	int PixelID = 0;
	BITMAP Bitmap;

	//--Make sure the screen data is valid
	if(ScreenData == NULL)
	{
		return;
	}

	//--Allocate memory for the blend data
	unsigned char* MemoryData = NULL;

	//--Get the interface device context
	HDC SourceHDC = GetDC(CGlobals::GetAppHWND());

	//--Create a compatible device context from the interface context
    HDC CompatibleHDC = CreateCompatibleDC(SourceHDC);

	int nWidth = GetValidDeviceWidth();
	int nHeight = GetValidDeviceHeight();

	//--Create a compatible bitmap
    HBITMAP BitmapHandle = CreateCompatibleBitmap(SourceHDC, nWidth, nHeight); 
    
	//--Validate the data
	if(BitmapHandle == 0 || !SelectObject(CompatibleHDC, BitmapHandle))
    {
		return;
    }

	//--Blit the bitmap to the memory device context
    if(!BitBlt(CompatibleHDC, 0, 0, nWidth, nHeight, SourceHDC, 0, 0, SRCCOPY)) 
    {
		return;
    }

	//by lwl, copy the mouse cursor to the bitmap
	if (bCaptureMouse) {
		HCURSOR hc=::GetCursor();
		CURSORINFO cursorinfo;
		ICONINFO iconinfo;
		cursorinfo.cbSize=sizeof(CURSORINFO);
		::GetCursorInfo(&cursorinfo);
		::GetIconInfo(cursorinfo.hCursor,&iconinfo);
		::ScreenToClient(CGlobals::GetAppHWND(),&cursorinfo.ptScreenPos);
		::DrawIcon(CompatibleHDC,cursorinfo.ptScreenPos.x-iconinfo.xHotspot,cursorinfo.ptScreenPos.y-iconinfo.yHotspot,cursorinfo.hCursor);
	}

	//--Get the bitmap
	GetObject(BitmapHandle, sizeof(BITMAP), &Bitmap);

	//--Compute the bitmap size
	unsigned long BitmapSize = sizeof(BITMAPINFOHEADER) + (Bitmap.bmWidth  * Bitmap.bmHeight* 3);

	//--Allocate a memory block for the bitmap
	BYTE* MemoryHandle = new BYTE[BitmapSize];
	memset(MemoryHandle, 0, BitmapSize);

	//--Validate the memory handle
	if(!MemoryHandle)
	{
		return;
	}

	//--Setup the bitmap data
	LPBITMAPINFOHEADER WatermarkInformation = (LPBITMAPINFOHEADER)(MemoryHandle);
	WatermarkInformation->biSizeImage		= 0;//BitmapSize - sizeof(BITMAPINFOHEADER);
	WatermarkInformation->biSize			= sizeof(BITMAPINFOHEADER);
	WatermarkInformation->biHeight			= Bitmap.bmHeight;
	WatermarkInformation->biWidth			= Bitmap.bmWidth;
	WatermarkInformation->biCompression		= BI_RGB;
	WatermarkInformation->biBitCount		= 24;
	WatermarkInformation->biPlanes			= 1;
	WatermarkInformation->biXPelsPerMeter	= 0;
	WatermarkInformation->biYPelsPerMeter	= 0;
	WatermarkInformation->biClrUsed	  		= 0;
	WatermarkInformation->biClrImportant	= 0;

	//--Get the bitmap data from memory
	GetDIBits(CompatibleHDC, BitmapHandle, 0, Bitmap.bmHeight, (unsigned char*)(WatermarkInformation + 1), (LPBITMAPINFO)WatermarkInformation, DIB_RGB_COLORS);

	//--Clear the handle to the device context
	DeleteDC(CompatibleHDC);

	//--Get the data
	LPBITMAPINFOHEADER Data = (LPBITMAPINFOHEADER)MemoryHandle;
	MemoryData = (unsigned char*)Data + Data->biSize + Data->biClrUsed * sizeof(RGBQUAD);
	
	//--Copy the memory data to the screen data
	for(int Y = 0; Y < Bitmap.bmHeight; Y++)
	{
		for(int X = 0; X < Bitmap.bmWidth; X++)
		{
			PixelID = (3 * (Y * Bitmap.bmWidth + X));
			ScreenData[PixelID + 0] = MemoryData[PixelID + 2];
			ScreenData[PixelID + 1] = MemoryData[PixelID + 1];
			ScreenData[PixelID + 2] = MemoryData[PixelID + 0];
		}
	}

	//--Free the memory handle
	SAFE_DELETE(MemoryHandle);
}

//------------------------------------------------------------------------------------------------
//--
//--ApplyWatermark(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::ApplyWatermark(WATERMARK* Watermark, unsigned char* ScreenData, SCREENSHOTRESOLUTION& Resolution, int ResolutionID, bool UseTransparentColor, WATERMARKTRANSPARENTCOLOR& TransparentColor)
{
	int	PositionX = 0;
	int	PositionY = 0;
	int PixelID0  = 0;
	int PixelID1  = 0;

	//--Make sure the screen data and watermark is valid
	if(Watermark == NULL || ScreenData == NULL)
	{
		return;
	}

	//--If hi resolution and watermark is in the top left corner
	if(Resolution == 0x01 && Watermark->Position == 0x00)
	{
		//--Make sure the ID is 0 (See TakeScreenShot(*) for ID details)
		if(ResolutionID != 0)
		{
			//--If not, do a normal screen shot and return
			if (m_bFromTexture) {
				CaptureTexture(&ScreenData);
			}else{
				CaptureScreen(ScreenData);
			}
			return;
		}
	}

	//--If hi resolution and watermark is in the top right corner
	else if(Resolution == 0x01 && Watermark->Position == 0x01)
	{
		//--Make sure the ID is 2 (See TakeScreenShot(*) for ID details)
		if(ResolutionID != 2)
		{
			//--If not, do a normal screen shot and return
			if (m_bFromTexture) {
				CaptureTexture(&ScreenData);
			}else{
				CaptureScreen(ScreenData);
			}
			return;
		}
	}

	//--If hi resolution and watermark is in the bottom left corner
	else if(Resolution == 0x01 && Watermark->Position == 0x02)
	{
		//--Make sure the ID is 6 (See TakeScreenShot(*) for ID details)
		if(ResolutionID != 6)
		{
			//--If not, do a normal screen shot and return
			if (m_bFromTexture) {
				CaptureTexture(&ScreenData);
			}else{
				CaptureScreen(ScreenData);
			}
			return;
		}
	}

	//--If hi resolution and watermark is in the bottom right corner
	else if(Resolution == 0x01 && Watermark->Position == 0x04)
	{
		//--Make sure the ID is 8 (See TakeScreenShot(*) for ID details)
		if(ResolutionID != 8)
		{
			//--If not, do a normal screen shot and return
			if (m_bFromTexture) {
				CaptureTexture(&ScreenData);
			}else{
				CaptureScreen(ScreenData);
			}
			return;
		}
	}

	int nWidth = GetValidDeviceWidth();
	int nHeight = GetValidDeviceHeight();

	//--Make sure the dimensions of the watermark are not bigger than the screen shot
	if(Watermark->Width > (int)nWidth || Watermark->Height > (int)nHeight)
	{
		//--Display an error and return
		MessageBox(NULL, "The watermark is bigger than the screen shot...\nThe screen shot will not be created", "[ERROR]", MB_OK);
		return;
	}

	//--Allocate memory for the blend data
	unsigned char* MemoryData = new BYTE[nWidth * nHeight * 3 ];

	//--If memory could not be allocated for the blend data
	if(MemoryData == NULL)
	{
		//--Just return
		return;
	}

	if (m_bFromTexture) {
		CaptureTexture(&MemoryData);
	}else{
		CaptureScreen(MemoryData);
	}

	//--If the watermark is in the top left corner
	if(Watermark->Position == 0x00)
	{
		PositionX = 0;
		PositionY = (nHeight - Watermark->Height);
	}

	//--Else if the watermark is in the top right corner
	else if(Watermark->Position == 0x01)
	{
		PositionX = (nWidth - Watermark->Width);
		PositionY = (nHeight - Watermark->Height);
	}

	//--Else if the watermark is in the bottom left corner
	else if(Watermark->Position == 0x02)
	{
		PositionX = 0;
		PositionY = 0;
	}

	//--Else if the watermark is in the bottom right corner
	else if(Watermark->Position == 0x04)
	{
		PositionX = (nWidth - Watermark->Width);
		PositionY = 0;
	}

	//--Copy the memory pixels to the screen data
	int Y;
	for(Y = 0; Y < (int)nHeight; Y++)
	{
		for(int X = 0; X < (int)nWidth; X++)
		{
			PixelID0 = (3 * (Y * nWidth + X));
			ScreenData[PixelID0 + 0] = MemoryData[PixelID0 + 0];
			ScreenData[PixelID0 + 1] = MemoryData[PixelID0 + 1];
			ScreenData[PixelID0 + 2] = MemoryData[PixelID0 + 2];
		}
	}

	//--Loop through every pixel in the watermark
	for(Y = 0; Y < Watermark->Height; Y++)
	{
		for(int X = 0; X < Watermark->Width; X++)
		{
			//--Compute the pixel positions
			PixelID0 = (3 * (Y * Watermark->Width + X));
			PixelID1 = (3 * ((PositionY + Y) * nWidth + (PositionX + X)));

			//--If using a transparent color
			if(UseTransparentColor == true)
			{
				//--If the transparent color matches the watermark color
				if(TransparentColor.Red   == Watermark->Data[PixelID0 + 0] &&
				   TransparentColor.Green == Watermark->Data[PixelID0 + 1] &&
				   TransparentColor.Blue  == Watermark->Data[PixelID0 + 2])
				{
					//--Set that pixel to the memory data
					Watermark->Data[PixelID0 + 0] = MemoryData[PixelID1 + 0];
					Watermark->Data[PixelID0 + 1] = MemoryData[PixelID1 + 1];
					Watermark->Data[PixelID0 + 2] = MemoryData[PixelID1 + 2];
				}
			}
			
			//--If the watermark strength is 0, then just use the original pixel
			if(Watermark->Strength == 0.0f)
			{
				ScreenData[PixelID1 + 0] = MemoryData[PixelID1 + 0];
				ScreenData[PixelID1 + 1] = MemoryData[PixelID1 + 1];
				ScreenData[PixelID1 + 2] = MemoryData[PixelID1 + 2];
			}

			//--If the watermark strength is 1, then use the watermark pixel
			else if(Watermark->Strength == 1.0f)
			{
				ScreenData[PixelID1 + 0] = Watermark->Data[PixelID0 + 0];
				ScreenData[PixelID1 + 1] = Watermark->Data[PixelID0 + 1];
				ScreenData[PixelID1 + 2] = Watermark->Data[PixelID0 + 2];
			}

			//--If the watermark strength is in between 0 and 1, compute and apply the blend value
			//--Use a simple linea interpolation to blend the 2 pixel values together
			else
			{
				ScreenData[PixelID1 + 0] = MemoryData[PixelID1 + 0] + (unsigned char)(Watermark->Strength * (Watermark->Data[PixelID0 + 0] - MemoryData[PixelID1 + 0]));
				ScreenData[PixelID1 + 1] = MemoryData[PixelID1 + 1] + (unsigned char)(Watermark->Strength * (Watermark->Data[PixelID0 + 1] - MemoryData[PixelID1 + 1]));
				ScreenData[PixelID1 + 2] = MemoryData[PixelID1 + 2] + (unsigned char)(Watermark->Strength * (Watermark->Data[PixelID0 + 2] - MemoryData[PixelID1 + 2]));
			}
		}
	}

	//--If the blend data is still valid
	SAFE_DELETE(MemoryData);
	
	//--If the watermark data is valid
	SAFE_DELETE(Watermark->Data);
	
	//--Set the watermark as invalid
	Watermark->Valid = false;
}

//------------------------------------------------------------------------------------------------
//--
//--SnapScreenShot(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::SnapScreenShot(const char* Filename, SCREENSHOTRESOLUTION& Resolution, int ResolutionID, bool UseTransparentColor, WATERMARKTRANSPARENTCOLOR& TransparentColor)
{
	char ValidFilename[1024];
	char TempFilename[1024];
	int	PositionX		= 0;
	int	PositionY		= 0;
	int PixelID0		= 0;
	int PixelID1		= 0;

	//--Validate the filenames

	if(ValidFilename == NULL || TempFilename == NULL)
	{
		return;
	}

	if(Filename == NULL || Filename[0]=='\0')
	{
		//--Copy the filename
		strcpy(TempFilename, "paraengine");

		//--Strip the directory off of the filename if one exists
		StripDirectoryFromFilename(TempFilename);

		//--Strip the file extension off of the filename if one is on there
		StripExtensionFromFilename(TempFilename);

		//--Get a valid filename to save the screen shot to
		GetValidFilename(ValidFilename, TempFilename);
	}
	else
	{
		//--Copy the filename
		snprintf(ValidFilename, 1024, "%s", Filename);
	}
	

	int nWidth = GetValidDeviceWidth();
	int nHeight = GetValidDeviceHeight();
	//--Allocate enough memory to hold the screen shot
	BYTE* ScreenData = NULL;
	
	if(m_Watermark.Valid == true && m_Watermark.Data != NULL)
	{
		//--Set the watermark on the screen shot
		ApplyWatermark(&m_Watermark, ScreenData, Resolution, ResolutionID, UseTransparentColor, TransparentColor);
	}
	else
	{
		//--Just do a normal screen shot
		if (m_bFromTexture) {
			CaptureTexture( &ScreenData);
		}else{
			CaptureScreen(ScreenData);
		}
	}

	//--Low resolution
	if(Resolution == 0x00)
	{
		//--Save the screen data to the specified file
		SaveScreenShot(ValidFilename, nWidth, nHeight, 3, ScreenData);
	}

	//--High resolution
	else if(Resolution == 0x01)
	{
		//--Compute the position to store the current screen shot at
		//--Remember, the ID's are as follows:
		//--
		//--  -----------------------
		//-- |       |       |       |
		//-- |  (0)  |  (1)  |  (2)  |
		//-- |       |       |       |
		//--  -------+-------+-------
		//-- |       |       |       |
		//-- |  (3)  |  (4)  |  (5)  |
		//-- |       |       |       |
		//--  -------+-------+-------
		//-- |       |       |       |
		//-- |  (6)  |  (7)  |  (8)  |
		//-- |       |       |       |
		//--  -----------------------
		//--
		switch(ResolutionID)
		{
			case 0:
				PositionX = 0;
				PositionY = ((nHeight * 3) - nHeight);
				break;

			case 1:
				PositionX = ((nWidth * 3) - nWidth) / 2;
				PositionY = ((nHeight * 3) - nHeight);
				break;

			case 2:
				PositionX = ((nWidth * 3) - nWidth);
				PositionY = ((nHeight * 3) - nHeight);
				break;

			case 3:
				PositionX = 0;
				PositionY = ((nHeight * 3) - nHeight) / 2;
				break;

			case 4:
				PositionX = ((nWidth * 3) - nWidth) / 2;
				PositionY = ((nHeight * 3) - nHeight) / 2;
				break;

			case 5:
				PositionX = ((nWidth * 3) - nWidth);
				PositionY = ((nHeight * 3) - nHeight) / 2;
				break;

			case 6:
				PositionX = 0;
				PositionY = 0;
				break;

			case 7:
				PositionX = ((nWidth * 3) - nWidth) / 2;
				PositionY = 0;
				break;

			case 8:
				PositionX = ((nWidth * 3) - nWidth);
				PositionY = 0;
				break;
		}

		//--Loop through all the pixels in the current screen shot
		for(int Y = 0; Y < (int)nHeight; Y++)
		{
			for(int X = 0; X < (int)nWidth; X++)
			{
				//--Compute the pixel positions
				PixelID0 = (3 * ((PositionY + Y) * (nWidth * 3) + (PositionX + X)));
				PixelID1 = (3 * (Y * nWidth + X));

				//--Set the pixel data
				m_HRScreenShotData[PixelID0 + 0] = ScreenData[PixelID1 + 0];
				m_HRScreenShotData[PixelID0 + 1] = ScreenData[PixelID1 + 1];
				m_HRScreenShotData[PixelID0 + 2] = ScreenData[PixelID1 + 2];
			}
		}

		//--If this is the last shot
		if(ResolutionID >= 8)
		{
			//--Save the screen data to the specified file
			SaveScreenShot(ValidFilename, nWidth * 3, nHeight * 3, 3, m_HRScreenShotData, true);
		}
	}

	
}

//------------------------------------------------------------------------------------------------
//--
//--TakeScreenShot(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::TakeScreenShot(const char* Filename)
{
	//--Set the flag indicating the system is taking a screen shot
	m_InProgress = true;

	//--Default to low resolution
	SCREENSHOTRESOLUTION Resolution = SSR_LORES;

	//--Snap the screen shot
	SnapScreenShot(Filename, Resolution, 0, false, WATERMARKTRANSPARENTCOLOR(0));
	
	//--Set the flag indicating the system is done taking the screen shot
	m_InProgress = false;
}

//------------------------------------------------------------------------------------------------
//--
//--TakeScreenShot(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
//void SCREENSHOTSYSTEM::TakeScreenShot(char* Filename, void (*RenderFunction)(void), SCREENSHOTRESOLUTION Resolution)
//{
//	//--Make sure a valid filename is passed in
//	if(Filename == NULL)
//	{
//		//--Nope, simply return
//		return;
//	}
//
//	//--Set the flag indicating the system is taking a screen shot
//	m_InProgress = true;
//
//	//--Low resolution screen shot
//	if(Resolution == 0x00)
//	{
//		//--Call the rendering function
//		RenderFunction();
//
//		//--Snap a low resolution screen shot
//		SnapScreenShot(Filename, Resolution, 0, false, WATERMARKTRANSPARENTCOLOR(0));
//	}
//
//	//--High resolution screen shot
//	else if(Resolution == 0x01)
//	{
//		int nWidth = GetValidDeviceWidth();
//		int nHeight = GetValidDeviceHeight();
//
//		//--Allocate enough memory to hold the high resolution screen shot data
//		m_HRScreenShotData = new BYTE[(nWidth * 3) * (nHeight * 3) * 3];
//
//		//--Validate the high resolution screen shot data
//		if(m_HRScreenShotData == NULL)
//		{
//			return;
//		}
//
//		//--It takes 9 passes to produce the high resolution screen shot
//		//--
//		//--Heres the ID's for the PerspectiveEX
//		//--
//		//--(-1) is a Normal perspective projection
//		//--The other ID's will shift the projection as illustrated:
//		//--  -----------------------
//		//-- |       |       |       |
//		//-- |  (0)  |  (1)  |  (2)  |
//		//-- |       |       |       |
//		//--  -------+-------+-------
//		//-- |       |       |       |
//		//-- |  (3)  |  (4)  |  (5)  |
//		//-- |       |       |       |
//		//--  -------+-------+-------
//		//-- |       |       |       |
//		//-- |  (6)  |  (7)  |  (8)  |
//		//-- |       |       |       |
//		//--  -----------------------
//		//--
//		for(int ID = 0; ID < 9; ID++)
//		{
//			//--Set the current perspective projection as illustrated above
//			PerspectiveEX(45.0f, (float)nWidth / (float)nHeight, 0.1f, 5000.0f, ID);
//
//			//--Call the rendering function
//			RenderFunction();
//
//			//--Snap the current screen shot
//			SnapScreenShot(Filename, Resolution, ID, false, WATERMARKTRANSPARENTCOLOR(0));
//		}
//
//		//--If the high resolution screen shot data is valid
//		SAFE_DELETE(m_HRScreenShotData);
//		
//		//--Set the projection and frustum back to normal
//		PerspectiveEX(45.0f, (float)nWidth / (float)nHeight, 0.1f, 5000.0f, -1);
//	}
//
//	//--Set the flag indicating the system is done taking the screen shot
//	m_InProgress = false;
//}
//
////------------------------------------------------------------------------------------------------
////--
////--TakeScreenShot(*)
////--Version: 1.0
////--
////------------------------------------------------------------------------------------------------
//void SCREENSHOTSYSTEM::TakeScreenShotW(char* Filename, char* Watermark, WATERMARKPOSITION WatermarkPosition, float Strength, bool UseTransparentColor, WATERMARKTRANSPARENTCOLOR TransparentColor)
//{
//	BITMAPINFOHEADER BHEADER;
//
//	//--Validate the filename
//	if(Filename == NULL || Watermark == NULL)
//	{
//		//--Take the screen shot like normal and it will handle adding the watermark
//		TakeScreenShot(Filename);
//		return;
//	}
//
//	//--Load the watermark
//	LoadWatermark(&m_Watermark, Watermark, &BHEADER);
//
//	//--Set the watermark's strength
//	m_Watermark.Strength = Strength;
//
//	//--Set the watermark position
//	m_Watermark.Position = WatermarkPosition;
//
//	//--Set the flag indicating the system is taking a screen shot
//	m_InProgress = true;
//
//	//--Default to low resolution
//	SCREENSHOTRESOLUTION Resolution = SSR_LORES;
//
//	//--Snap the screen shot
//	SnapScreenShot(Filename, Resolution, 0, UseTransparentColor, TransparentColor);
//	
//	//--Set the flag indicating the system is done taking the screen shot
//	m_InProgress = false;
//}
//
////------------------------------------------------------------------------------------------------
////--
////--TakeScreenShot(*)
////--Version: 1.0
////--
////------------------------------------------------------------------------------------------------
//void SCREENSHOTSYSTEM::TakeScreenShotW(char* Filename, void (*RenderFunction)(void), SCREENSHOTRESOLUTION Resolution, char* Watermark, WATERMARKPOSITION WatermarkPosition, float Strength, bool UseTransparentColor, WATERMARKTRANSPARENTCOLOR TransparentColor)
//{
//	BITMAPINFOHEADER BHEADER;
//
//	//--Validate the filename
//	if(Filename == NULL || Watermark == NULL)
//	{
//		//--Take the screen shot like normal and it will handle adding the watermark
//		TakeScreenShot(Filename, RenderFunction, Resolution);
//		return;
//	}
//
//	//--Load the watermark
//	LoadWatermark(&m_Watermark, Watermark, &BHEADER);
//
//	//--Set the watermark's strength
//	m_Watermark.Strength = Strength;
//
//	//--Set the watermark position
//	m_Watermark.Position = WatermarkPosition;
//
//	//--Set the flag indicating the system is taking a screen shot
//	m_InProgress = true;
//
//	//--Low resolution screen shot
//	if(Resolution == 0x00)
//	{
//		//--Call the rendering function
//		RenderFunction();
//
//		//--Snap a low resolution screen shot
//		SnapScreenShot(Filename, Resolution, 0, UseTransparentColor, TransparentColor);
//	}
//
//	//--High resolution screen shot
//	else if(Resolution == 0x01)
//	{
//		int nWidth = GetValidDeviceWidth();
//		int nHeight = GetValidDeviceHeight();
//
//		//--Allocate enough memory to hold the high resolution screen shot data
//		m_HRScreenShotData = new BYTE[(nWidth * 3) * (nHeight * 3) * 3];
//
//		//--Validate the high resolution screen shot data
//		if(m_HRScreenShotData == NULL)
//		{
//			return;
//		}
//
//		//--It takes 9 passes to produce the high resolution screen shot
//		//--
//		//--Heres the ID's for the PerspectiveEX
//		//--
//		//--(-1) is a Normal perspective projection
//		//--The other ID's will shift the projection as illustrated:
//		//--  -----------------------
//		//-- |       |       |       |
//		//-- |  (0)  |  (1)  |  (2)  |
//		//-- |       |       |       |
//		//--  -------+-------+-------
//		//-- |       |       |       |
//		//-- |  (3)  |  (4)  |  (5)  |
//		//-- |       |       |       |
//		//--  -------+-------+-------
//		//-- |       |       |       |
//		//-- |  (6)  |  (7)  |  (8)  |
//		//-- |       |       |       |
//		//--  -----------------------
//		//--
//		for(int ID = 0; ID < 9; ID++)
//		{
//			//--Set the current perspective projection as illustrated above
//			PerspectiveEX(45.0f, (float)nWidth / (float)nHeight, 0.1f, 5000.0f, ID);
//
//			//--Call the rendering function
//			RenderFunction();
//
//			//--Snap the current screen shot
//			SnapScreenShot(Filename, Resolution, ID, UseTransparentColor, TransparentColor);
//		}
//
//		//--If the high resolution screen shot data is valid
//		SAFE_DELETE(m_HRScreenShotData);
//		
//		//--Set the projection and frustum back to normal
//		PerspectiveEX(45.0f, (float)nWidth / (float)nHeight, 0.1f, 5000.0f, -1);
//	}
//
//	//--Set the flag indicating the system is done taking the screen shot
//	m_InProgress = false;
//}
//
//
/**
* LiXizhi: It appears that some codex such as Xvid, does not support padding to LONG for each scan lines.
* (the AVIStreamSetFormat() will fail for padded screen image size) 
* I will manually enforce that the width of the image is a multiple of 4.(a scan line will not be padded)
*/
int	SCREENSHOTSYSTEM::GetValidDeviceWidth()
{
	int width = (int)CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Width;
	return (width - width%4);
}

int	SCREENSHOTSYSTEM::GetValidDeviceHeight()
{
	int height = (int)CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Height;
	return (height - height%4);
}

int SCREENSHOTSYSTEM::GetCurrentFrameNumber()
{
	return m_Video.NumberOfFrames;
}
//------------------------------------------------------------------------------------------------
//--
//--BeginMovieCapture(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
bool SCREENSHOTSYSTEM::BeginMovieCapture(const char* Filename, unsigned int FramesPerSecond, bool AllowCodecSelection, bool EnableInProcessFlag)
{
	char* ValidFilename = new char[1024];
	char* TempFilename  = new char[1024];

	//--Validate the filenames
	if(Filename == NULL || ValidFilename == NULL || TempFilename == NULL)
	{
		return false;
	}

	//--Copy the filename
	strcpy(TempFilename, Filename);

	//--Strip the directory off of the filename if one exists
	StripDirectoryFromFilename(TempFilename);

	//--Strip the file extension off of the filename if one is on there
	StripExtensionFromFilename(TempFilename);

	//--Get a valid filename to save the movie to
	GetValidFilename(ValidFilename, TempFilename, true);

	//--Store the video parameters
	m_Video.VideoOK				= true;
	m_InProgress				= EnableInProcessFlag;
	m_Video.AllowCodecSelection = AllowCodecSelection;
	if(m_bFromTexture)
	{
		m_Video.VideoWidth			= m_TextureWidth;
		m_Video.VideoHeight			= m_TextureHeight;
	}
	else
	{
		m_Video.VideoWidth			= GetValidDeviceWidth();
		m_Video.VideoHeight			= GetValidDeviceHeight();
	}
	
	//--Initialize the AVI file
	AVIFileInit();

	//--Open/Create the AVI file
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR ValidFilename16 = StringHelper::MultiByteToWideChar(ValidFilename, DEFAULT_FILE_ENCODING);
	if (AVIFileOpenW(&m_Video.VideoFile, ValidFilename16, OF_WRITE | OF_CREATE, NULL) != AVIERR_OK)
#else
	if (AVIFileOpen(&m_Video.VideoFile, ValidFilename, OF_WRITE | OF_CREATE, NULL) != AVIERR_OK)
#endif
	
	{
		//--The avi couldn't be opened/created
		//--Set the OK flag to false so the system knows not to try and write to the AVI file
		m_Video.VideoOK = false;
	}
	else
	{
		m_sVideoFileName =  ValidFilename;
	}

	//--If the filename is valid
	SAFE_DELETE(ValidFilename);
	//--If the temporary filename is valid
	SAFE_DELETE(TempFilename);
	
	// allow the user to choose recorder, by rendering a fake frame at 0;
	assert(m_Video.NumberOfFrames == 0);
	return UpdateMovieCapture(FramesPerSecond);
}

//------------------------------------------------------------------------------------------------
//--
//--BeginMovieCapture(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
//void SCREENSHOTSYSTEM::BeginMovieCaptureW(char* Filename, bool AllowCodecSelection, bool EnableInProcessFlag, char* Watermark, WATERMARKPOSITION WatermarkPosition, float Strength, bool UseTransparentColor, WATERMARKTRANSPARENTCOLOR TransparentColor)
//{
//	char* ValidFilename = new char[1024];
//	char* TempFilename  = new char[1024];
//	BITMAPINFOHEADER BHEADER;
//
//	//--Validate the filenames
//	if(Filename == NULL || ValidFilename == NULL || TempFilename == NULL || Watermark == NULL)
//	{
//		//--Proceed to the other BeginMovieCapture(*) function to initialize the video settings
//		BeginMovieCapture(Filename, AllowCodecSelection, EnableInProcessFlag);
//		return;
//	}
//
//	//--Copy the filename
//	strcpy(TempFilename, Filename);
//
//	//--Strip the directory off of the filename if one exists
//	StripDirectoryFromFilename(TempFilename);
//
//	//--Strip the file extension off of the filename if one is on there
//	StripExtensionFromFilename(TempFilename);
//
//	//--Get a valid filename to save the movie to
//	GetValidFilename(ValidFilename, TempFilename, true);
//
//	//--Load the watermark
//	LoadWatermark(&m_Watermark, Watermark, &BHEADER);
//
//	//--Set the watermark's strength
//	m_Watermark.Strength = Strength;
//
//	//--Set the watermark position
//	m_Watermark.Position = WatermarkPosition;
//
//	//--Store the video parameters
//	m_Video.VideoOK				= true;
//	m_Video.TransparentColor	= TransparentColor;
//	m_InProgress				= EnableInProcessFlag;
//	m_Video.AllowCodecSelection = AllowCodecSelection;
//	m_Video.UseTransparentColor = UseTransparentColor;
//	m_Video.VideoWidth			= GetValidDeviceWidth();
//	m_Video.VideoHeight			= GetValidDeviceHeight();
//
//	//--Initialize the AVI file
//	AVIFileInit();
//
//	//--Open/Create the AVI file
//	if(AVIFileOpen(&m_Video.VideoFile, ValidFilename, OF_WRITE | OF_CREATE, NULL) != AVIERR_OK)
//	{
//		//--The avi couldn't be opened/created
//		//--Set the OK flag to false so the system knows not to try and write to the AVI file
//		m_Video.VideoOK = false;
//	}
//
//	//--If the filename is valid
//	SAFE_DELETE(ValidFilename);
//
//	//--If the temporary filename is valid
//	SAFE_DELETE(TempFilename);
//}
//
void SCREENSHOTSYSTEM::SelectCodecOptions()
{
	//SaveCodecOptions(m_sCodecOptionFile.c_str());
}


bool SCREENSHOTSYSTEM::SaveCodecOptions(const char* szFilename, const AVICOMPRESSOPTIONS& CompressionOptions)
{
	CParaFile file;
	if(!file.CreateNewFile(szFilename))
	{
		OUTPUT_LOG("Can't open %s for writing", szFilename);
		return false;
	}
	file.write(&CompressionOptions, sizeof(CompressionOptions));
	file.write(CompressionOptions.lpParms, CompressionOptions.cbParms);
	file.close();
	return false;
}

void SCREENSHOTSYSTEM::LoadCodecOptions(const char* szFilename,void* data, DWORD *size)
{
#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR szFilename16 = StringHelper::MultiByteToWideChar(szFilename, DEFAULT_FILE_ENCODING);
	FILE* file = _wfopen(szFilename16, L"rb");
#else
	FILE* file = fopen(szFilename, "rb");
#endif
	
	if (file==NULL) {
		OUTPUT_LOG("Can't open %s for reading", szFilename);
		return;
	}
	*size = (int)fread(data, 1, *((size_t*)size), file);
	fclose(file);
}
//------------------------------------------------------------------------------------------------
//--
//--UpdateMovieCapture(*)
//--Version: 0.5
//--
//------------------------------------------------------------------------------------------------
bool SCREENSHOTSYSTEM::UpdateMovieCapture(unsigned int FramesPerSecond, int nMode)
{
	unsigned char* Watermark = NULL;
	int	PositionX = 0;
	int	PositionY = 0;
	int PixelID0  = 0;
	int PixelID1  = 0;

	//--If the file was not Opened/Created
	if(m_Video.VideoOK == false)
	{
		//--Return so the system doesn't write to the file
		return false;
	}

	//--Get the screen data
	LPBITMAPINFOHEADER ScreenData = NULL;
	if (m_bFromTexture) {
		CaptureTexture( (byte**)&ScreenData, nMode);
		
		// we will escape even lines until the odd lines are ready.
		if(nMode == 2 || nMode == 4 || nMode == 6)
			return true;
	}else{
		ScreenData = (LPBITMAPINFOHEADER)CaptureScreen();
	}
	if(ScreenData == 0)
		return false;

	if(m_Watermark.Valid == true && m_Watermark.Data != NULL)
	{
		int nWidth = GetValidDeviceWidth();
		int nHeight = GetValidDeviceHeight();

		//--Get the memory data
		Watermark = (unsigned char*)ScreenData + ScreenData->biSize + ScreenData->biClrUsed * sizeof(RGBQUAD);

		//--If the watermark is in the top left corner
		if(m_Watermark.Position == 0x00)
		{
			PositionX = 0;
			PositionY = (nHeight - m_Watermark.Height);
		}

		//--Else if the watermark is in the top right corner
		else if(m_Watermark.Position == 0x01)
		{
			PositionX = (nWidth - m_Watermark.Width);
			PositionY = (nHeight - m_Watermark.Height);
		}

		//--Else if the watermark is in the bottom left corner
		else if(m_Watermark.Position == 0x02)
		{
			PositionX = 0;
			PositionY = 0;
		}

		//--Else if the watermark is in the bottom right corner
		else if(m_Watermark.Position == 0x04)
		{
			PositionX = (nWidth - m_Watermark.Width);
			PositionY = 0;
		}

		//--Loop through every pixel in the watermark
		for(int Y = 0; Y < m_Watermark.Height; Y++)
		{
			for(int X = 0; X < m_Watermark.Width; X++)
			{
				//--Compute the pixel positions
				PixelID0 = (3 * (Y * m_Watermark.Width + X));
				PixelID1 = (4 * ((PositionY + Y) * nWidth + (PositionX + X)));

				//--If using a transparent color
				if(m_Video.UseTransparentColor == true)
				{
					//--If the transparent color matches the watermark color
					if(m_Video.TransparentColor.Red   == m_Watermark.Data[PixelID0 + 0] &&
					   m_Video.TransparentColor.Green == m_Watermark.Data[PixelID0 + 1] &&
					   m_Video.TransparentColor.Blue  == m_Watermark.Data[PixelID0 + 2])
					{
						//--Set that pixel to the memory data
						m_Watermark.Data[PixelID0 + 0] = Watermark[PixelID1 + 0];
						m_Watermark.Data[PixelID0 + 1] = Watermark[PixelID1 + 1];
						m_Watermark.Data[PixelID0 + 2] = Watermark[PixelID1 + 2];
					}
				}

				//--If the watermark strength is 0, then just use the original pixel
				if(m_Watermark.Strength == 0.0f)
				{
					Watermark[PixelID1 + 0] = Watermark[PixelID1 + 0];
					Watermark[PixelID1 + 1] = Watermark[PixelID1 + 1];
					Watermark[PixelID1 + 2] = Watermark[PixelID1 + 2];
					Watermark[PixelID1 + 3] = 255;
				}

				//--If the watermark strength is 1, then use the watermark pixel
				else if(m_Watermark.Strength == 1.0f)
				{
					Watermark[PixelID1 + 0] = m_Watermark.Data[PixelID0 + 2];
					Watermark[PixelID1 + 1] = m_Watermark.Data[PixelID0 + 1];
					Watermark[PixelID1 + 2] = m_Watermark.Data[PixelID0 + 0];
					Watermark[PixelID1 + 3] = 255;
				}

				//--If the watermark strength is in between 0 and 1, compute and apply the blend value
				//--Use a simple linear interpolation to blend the 2 pixel values together
				else
				{
					Watermark[PixelID1 + 0] = Watermark[PixelID1 + 0] + (unsigned char)(m_Watermark.Strength * (m_Watermark.Data[PixelID0 + 2] - Watermark[PixelID1 + 0]));
					Watermark[PixelID1 + 1] = Watermark[PixelID1 + 1] + (unsigned char)(m_Watermark.Strength * (m_Watermark.Data[PixelID0 + 1] - Watermark[PixelID1 + 1]));
					Watermark[PixelID1 + 2] = Watermark[PixelID1 + 2] + (unsigned char)(m_Watermark.Strength * (m_Watermark.Data[PixelID0 + 0] - Watermark[PixelID1 + 2]));
					Watermark[PixelID1 + 3] = 255;
				}
			}
		}
	}

	//--If this is the first frame
	if(m_Video.NumberOfFrames == 0)
	{
		//--Setup the video stream information
		memset(&m_Video.StreamInformation, 0, sizeof(AVISTREAMINFO));
		m_Video.StreamInformation.rcFrame.top		    = 0;
		m_Video.StreamInformation.rcFrame.left		    = 0;
		m_Video.StreamInformation.fccHandler			= 0;
		m_Video.StreamInformation.dwScale               = 1;
		m_Video.StreamInformation.dwRate                = FramesPerSecond;
		m_Video.StreamInformation.rcFrame.right		    = m_Video.VideoWidth;
		m_Video.StreamInformation.rcFrame.bottom	    = m_Video.VideoHeight;
		m_Video.StreamInformation.fccType               = mmioFOURCC('v', 'i', 'd', 's');
		m_Video.StreamInformation.dwSuggestedBufferSize = m_Video.VideoWidth * m_Video.VideoHeight;

		//--Create the main file stream
		if(AVIFileCreateStream(m_Video.VideoFile, &m_Video.Stream, &m_Video.StreamInformation) != AVIERR_OK)
		{
			//--If the watermark was set
			if(m_Watermark.Valid == true)
			{
				
			}

			//--Set the flag letting the system know there was an error
			//--and not to write to the AVI file anymore
			m_Video.VideoOK = false;
			return false;
		}

		//--Setup the codec compression data
		AVICOMPRESSOPTIONS CompressionOptions;
		AVICOMPRESSOPTIONS FAR* CompressionOptionsF[1] = {&CompressionOptions};
		memset(&CompressionOptions, 0, sizeof(CompressionOptions));

		// load codec
		CParaFile  file(m_sCodecOptionFile.c_str());

		

		//--If allowing to select the codec
		if(m_Video.AllowCodecSelection || file.isEof())
		{
			if (!file.isEof())
			{
				memcpy(&CompressionOptions, file.getBuffer(), sizeof(CompressionOptions));
				file.seek(sizeof(CompressionOptions));
				// CompressionOptions.lpParms = (void*)file.getPointer();
				CompressionOptions.lpParms = NULL;
				CompressionOptions.cbParms = 0;
			}
			else
			{
				// TODO: how to default to select the "MicroSoft Video 1" Compression. 
				CompressionOptions.fccType = 0;
				CompressionOptions.fccHandler = mmioFOURCC('M', 'S', 'V', 'C');
				CompressionOptions.dwFlags = 8;
				CompressionOptions.dwQuality = 7500;
			}

			//--Show the dialog box to select the codec
			for (int i = 0; i < 2;++i)
			{
				try
				{
					if (!AVISaveOptions(CGlobals::GetAppHWND(), 0, 1, &m_Video.Stream, (LPAVICOMPRESSOPTIONS FAR*)&CompressionOptionsF))
					{
						//--If the watermark was set
						if (m_Watermark.Valid == true)
						{

						}

						//--Set the flag letting the system know there was an error
						//--and not to write to the AVI file anymore
						m_Video.VideoOK = false;
						return false;
					}
					else
					{
						// save the last option.
						SaveCodecOptions(m_sCodecOptionFile.c_str(), CompressionOptions);
					}
					break;
				}
				catch (...)
				{
					// try again with empty input. 
					if (i == 0)
						memset(&CompressionOptions, 0, sizeof(CompressionOptions));
					else
					{
						m_Video.VideoOK = false;
						return false;
					}
				}
			}
		}
		else
		{
			memcpy(&CompressionOptions, file.getBuffer(), sizeof(CompressionOptions));
			file.seek(sizeof(CompressionOptions));
			CompressionOptions.lpParms = (void*)file.getPointer();

//			///////////////////////////////////////////////////////////////////////////
//			// Uncomment the following line to save settings to file.some settings are already saved in codec_X files. Please note that debug and release built are different
//			//SaveCodecOptions("codec_wmv_release.dat");
//			switch(m_eCodec) {
//			case CODEC_XVID:
//				{
//					CompressionOptions.fccType=0;
//					CompressionOptions.fccHandler=1684633208;
//					CompressionOptions.dwKeyFrameEvery=0;
//					CompressionOptions.dwQuality=0;
//					CompressionOptions.dwBytesPerSecond=0;
//					CompressionOptions.dwFlags=8;
//					CompressionOptions.lpFormat=0;
//					CompressionOptions.cbFormat=0;
//					CompressionOptions.dwInterleaveEvery=0;
//					//CompressionOptions.cbParms=3264;
//#ifdef _DEBUG
//					CParaFile file(":IDR_CODEC_XVIDD");//"codec_xvidd.dat"
//#else
//					CParaFile file(":IDR_CODEC_XVID");//"codec_xvid.dat"
//#endif
//					CompressionOptions.cbParms=(int)file.getSize();
//					CompressionOptions.lpParms=file.getBuffer();
//					// resource object is persistent, no need to delete
//					//file.GiveupBufferOwnership(); 
//					break;
//				}
//			case CODEC_WMV:
//				{
//					CompressionOptions.fccType=0;
//					CompressionOptions.fccHandler=863399287;
//					CompressionOptions.dwKeyFrameEvery=0;
//					CompressionOptions.dwQuality=0;
//					CompressionOptions.dwBytesPerSecond=0;
//					CompressionOptions.dwFlags=8;
//					CompressionOptions.lpFormat=0;
//					CompressionOptions.cbFormat=0;
//					CompressionOptions.dwInterleaveEvery=0;
//					//CompressionOptions.cbParms=2976;
//#ifdef _DEBUG
//					CParaFile file(":IDR_CODEC_WMVD");//"codec_wmvd.dat"
//#else
//					CParaFile file(":IDR_CODEC_WMV"); //"codec_wmv.dat"
//#endif
//					CompressionOptions.cbParms=(int)file.getSize();
//					CompressionOptions.lpParms=file.getBuffer();
//					// resource object is persistent, no need to delete
//					//file.GiveupBufferOwnership();
//				}
//				break;
//			}
		}

		//--Make the compression stream
		if(AVIMakeCompressedStream(&m_Video.CompressionStream, m_Video.Stream, &CompressionOptions, NULL) != AVIERR_OK)
		{
			//--If the watermark was set
			if(m_Watermark.Valid == true)
			{
				
			}

			//--Set the flag letting the system know there was an error
			//--and not to write to the AVI file anymore
			m_Video.VideoOK = false;
			return false;
		}

		//--Setup the stream format
		if(AVIStreamSetFormat(m_Video.CompressionStream, 0, ScreenData, ScreenData->biSize + ScreenData->biClrUsed * sizeof(RGBQUAD)) != AVIERR_OK)
		{
			//--If the watermark was set
			if(m_Watermark.Valid == true)
			{
				
			}

			//--Set the flag letting the system know there was an error
			//--and not to write to the AVI file anymore
			m_Video.VideoOK = false;
			return false;
		}
		/*if(!m_Video.AllowCodecSelection){
			SAFE_DELETE(CompressionOptions.lpParms);
		}*/
	}
	//--Write the current screen shot to the AVI file, we will skip the first frame
	else if(AVIStreamWrite(m_Video.CompressionStream, m_Video.NumberOfFrames-1, 1, (unsigned char*)ScreenData + ScreenData->biSize + ScreenData->biClrUsed * sizeof(RGBQUAD), ScreenData->biSizeImage, AVIIF_KEYFRAME, NULL, NULL) != AVIERR_OK)
	{
		//--If the watermark was set
		if(m_Watermark.Valid == true)
		{
			
		}

		//--Set the flag letting the system know there was an error
		//--and not to write to the AVI file anymore
        m_Video.VideoOK = false;
		return false;
	}

	//--If the watermark was set
	if(m_Watermark.Valid == true)
	{
		
	}

	m_Video.NumberOfFrames++;
	return true;
}
int SCREENSHOTSYSTEM::GetCodec()
{
	return m_eCodec;
}
void SCREENSHOTSYSTEM::SetUsingCodec(int etype)
{
	if (InProgress()) {
		return;
	}
	m_eCodec=etype;
}

void SCREENSHOTSYSTEM::SetCaptureTexture(LPDIRECT3DTEXTURE9 pTexture)
{
	if (InProgress()) {
		return;
	}
	HRESULT hr;
	//SAFE_RELEASE(m_pTexture);
	if (pTexture==NULL) {
		m_bFromTexture=false;
		SAFE_RELEASE(m_pOffScreenSurface);
		m_pTexture = NULL;
	}else{
		SAFE_RELEASE(m_pOffScreenSurface);
		m_bFromTexture=true;
		m_pTexture=pTexture;
		//get texture information
		D3DSURFACE_DESC ddsd;
		ZeroMemory(&ddsd, sizeof(ddsd));

		if ( FAILED( hr = (pTexture)->GetLevelDesc( 0, &ddsd ) ) ) {
			OUTPUT_LOG(TEXT("Could not get level Description of D3DX texture! hr = 0x%x"), hr);
			return ;
		}

		
		m_TextureWidth= ddsd.Width;
		m_TextureHeight=ddsd.Height;
		IDirect3DSurface9* pSurf;

		if (SUCCEEDED(hr = (pTexture)->GetSurfaceLevel(0, &pSurf)))
			pSurf->GetDesc(&ddsd);

		// Save format info
		m_TextureFormat = ddsd.Format;

		if (m_TextureFormat != D3DFMT_X8R8G8B8 && (m_TextureFormat != D3DFMT_A8R8G8B8)) {
			OUTPUT_LOG(TEXT("Texture is format we can't handle! Format = 0x%x"), m_TextureFormat);
			return ;
		}
		pSurf->Release();

		if((ddsd.Usage & D3DUSAGE_RENDERTARGET)!=0 )
		{
			if( FAILED(CGlobals::GetRenderDevice()->CreateOffscreenPlainSurface(m_TextureWidth, m_TextureHeight, m_TextureFormat, 
				D3DPOOL_SYSTEMMEM,&m_pOffScreenSurface, NULL)) )
			{
				m_bFromTexture=false;
				m_pTexture = NULL;
				m_pOffScreenSurface = NULL;
			}
		}
	}
}
//------------------------------------------------------------------------------------------------
//--
//--EndMovieCapture(*)
//--Version: 1.0
//--
//------------------------------------------------------------------------------------------------
void SCREENSHOTSYSTEM::EndMovieCapture()
{
	//--If the main stream is still open
	if(m_Video.Stream)
	{
		//--Close the main stream
		AVIStreamClose(m_Video.Stream);

		//--Then clear it
		m_Video.Stream = NULL;
	}

	//--If the compression stream is still open
	if(m_Video.CompressionStream)
	{
		//--Close the compression stream
		AVIStreamClose(m_Video.CompressionStream);

		//--Then clear it
		m_Video.CompressionStream = NULL;
	}

	//--If the AVI file is still open
	if(m_Video.VideoFile)
    {
		//--Close the file
		AVIFileClose(m_Video.VideoFile);

		//--Then clear it
		m_Video.VideoFile = NULL;
    }

	//--Call the AVI exit function
	if(m_Video.VideoOK)
		AVIFileExit();

	//--If the process flag was set
	if(m_InProgress == true)
	{
		//--Reset it
		m_InProgress = false;
	}

	//--Set the 'video ok' flag to false
	m_Video.VideoOK = false;

	//--Clear the number of frames
	m_Video.NumberOfFrames = 0;

}

bool SCREENSHOTSYSTEM::HasLastCodec()
{
	return CParaFile::DoesFileExist(m_sCodecOptionFile.c_str(),false);
}

}//namespace ScreenShot