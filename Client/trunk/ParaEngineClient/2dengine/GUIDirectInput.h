#include "GUIBase.h"
#include "GUIResource.h"
#include "ParaEngine.h"
#include "GUIKeyboardVirtual.h"
#include "GUIMouseVirtual.h"

namespace ParaEngine
{
#if defined(USE_DIRECTX_RENDERER)
	/**
	* key board
	*/
	class CDirectKeyboard : public CGUIKeyboardVirtual
	{
	public:
		CDirectKeyboard(HWND hDlg);
		~CDirectKeyboard() { Free(); };

		HRESULT CreateDevice(HWND hDlg);
		void Free();
		virtual HRESULT ReadBufferedData();
		virtual HRESULT ReadImmediateData();

		virtual bool IsKeyPressed(const EVirtualKey& nKey);
		virtual void SetKeyPressed(const EVirtualKey& nKey, bool bPressed);
		virtual void Reset();

		LPDIRECTINPUT8       m_pDI; // The DirectInput object         
		LPDIRECTINPUTDEVICE8 m_pKeyboard; // The keyboard device 

		BYTE  m_keystateUserDefined[256];   // DirectInput keyboard state buffer 
	};

	/** mouse object */
	class CDirectMouse : public CGUIMouseVirtual
	{
	public:
		CDirectMouse(HWND hDlg);
		virtual ~CDirectMouse();
		HRESULT CreateDevice(HWND hDlg);
		void Free();
		virtual HRESULT ReadBufferedData();
		virtual HRESULT ReadImmediateData();
		virtual void Update();

		/** true to lock the mouse at its current location*/
		virtual void SetLock(bool bLock);

		/** true to show the cursor. only show if previous call is hide and vice versa. */
		virtual void ShowCursor(bool bShowCursor);

		/** Force the device to show its cursor. this is different from ShowCursor in that it always calls the device API to set the cursor. */
		virtual void ForceShowCursor(bool bShow);

		/** get current cursor position. */
		virtual void GetDeviceCursorPos(int& x, int& y);
		virtual void SetDeviceCursorPos(int x, int y);

		virtual bool IsButtonDown(EMouseButton nMouseButton);

		/**
		* Set the current cursor to use. One can call very often, since it will does nothing with identical cursor file and hot spot.
		* Typically, hardware supports only 32x32 cursors and, when windowed, the system might support only 32x32 cursors.
		* @param szCursor cursor file name: The contents of this texture will be copied and potentially format-converted into an internal buffer from which the cursor is displayed. The dimensions of this surface must be less than the dimensions of the display mode, and must be a power of two in each direction, although not necessarily the same power of two. The alpha channel must be either 0.0 or 1.0.
		* @param XHotSpot [in] X-coordinate offset (in pixels) that marks the center of the cursor. The offset is relative to the upper-left corner of the cursor. When the cursor is given a new position, the image is drawn at an offset from this new position determined by subtracting the hot spot coordinates from the position.
		* @param YHotSpot [in] Y-coordinate offset (in pixels) that marks the center of the cursor. The offset is relative to the upper-left corner of the cursor. When the cursor is given a new position, the image is drawn at an offset from this new position determined by subtracting the hot spot coordinates from the position.
		*/
		void SetCursorFromFile(const char* szCursor, int XHotSpot = 0, int YHotSpot = 0, bool force = false);
		/*
		* get the current cursor file.
		* @param pXHotSpot: if not NULL, it will return the hot spot X
		* @param pYHotSpot: if not NULL, it will return the hot spot Y
		* @return: the cursor file is returned.
		*/
		const char* GetCursorFile(int* pXHotSpot = NULL, int* pYHotSpot = NULL);

		/** obsolete*/
		void SetCursorFont(SpriteFontEntity* pFont, Color defaultFontColor, DWORD dwTextFormat = DT_CENTER | DT_VCENTER);
		/** obsolete*/
		void SetCursorText(const char16_t* wszText);

		/** obsolete*/
		void SetCursorTextA(const char* szText);
		/** obsolete*/
		void SetCursorTexture(TextureEntity* pTexture, RECT* prcTexture, Color defaultTextureColor = 0xffffffff);
		/** obsolete*/
		virtual HRESULT	RenderCursor(GUIState* pGUIState, float fElapsedTime);

		/** this may return "" if cursor is not set. */
		const std::string& GetCursorName() const;

		/** only change m_x, m_y, which is pretty fast. */
		virtual void SetMousePosition(int x, int y);

		virtual void Reset();

		void ResetCursorPosition();
	public:
		HWND					m_hwnd;// window handle of the default window
		std::string					m_szCursorName;//the name of the cursor file 

		int m_XHotSpot, m_YHotSpot; // mouse hot spot.
		LPDIRECTINPUT8			m_pDI; // The DirectInput object         
		LPDIRECTINPUTDEVICE8	m_pMouse; // The Mouse device 
		bool					m_bShowCursor;

	};
#else
	class CDirectKeyboard : public CGUIKeyboardVirtual
	{
	public:
		CDirectKeyboard(HWND hDlg = 0) {};
	};

	class CDirectMouse : public CGUIMouseVirtual
	{
	public:
		CDirectMouse(HWND hDlg = 0) {};
		virtual ~CDirectMouse() {};
	};
#endif
}
