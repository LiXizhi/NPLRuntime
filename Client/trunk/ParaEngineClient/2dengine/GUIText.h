#pragma once
#include "GUIBase.h"

namespace ParaEngine
{
struct SpriteFontEntity;
struct AssetEntity;
/**
* This class represents a string inside a rectangular box in the screen coordinates
* Multiple Type_GUIText object with the same font are batched together at rendering time
* So it is important that you are not rendering them in the z-ordering.
* This class is suitable of rendering large blocks of text, or multiple short text
* with the same font. It's good practice to keep the text instance as few as possible,
* however, the Type_GUIText has been extremely optimized so that the only overhead of having
* many instances of this class is in the data structure keeping. There will be NO
* or very few texture swapping during batched rendering.
* 
* You can dynamically create and discard many instances of this object during frame
* rendering. Only simple data structure is recreated, not any D3D device objects.  
*/
class CGUIText : public CGUIBase
{
public:
	static void		StaticInit();
	CGUIText(void);
	virtual ~CGUIText(void);

	ATTRIBUTE_DEFINE_CLASS(CGUIText);

public:
	virtual std::string ToScript(int option=0);
	/**
	* Set the text of this object
	* @param szText 
	*/
	virtual void SetTextA(const char* szText);
	/**
	* Set the text of this object
	* The control internally store everything in Unicode.
	* @param wszText 
	*/
	virtual void SetText(const char16_t* wszText);
	/**
	* get the text of the control
	* @param szText [out] buffer to receive the text
	* @param nLength size of the input buffer
	* @return return the number of bytes written to the buffer. If nLength is 0, this function returns the total number of characters of the text
	*/
	virtual int GetTextA(std::string& out);
	/**
	* get the text of the control 
	* The control internally store everything in Unicode. So it is faster this version than the ascii version.
	* @return 
	*/
	virtual const char16_t* GetText();

	/** try to automatically adjust the size of this object to contain all its content. */
	virtual bool DoAutoSize();

	/** get the text line size in pixels, supposing the current font and text will be rendered in a single line. */
	void GetTextLineSize(int* width, int* height);

	void SetColor(DWORD color);
	void SetTransparency(DWORD transparency);

	/**
	Set the text align and other text displaying formats
	@param dwFormat: the new format. It can be any combination of the following values. 
	DT_BOTTOM (0x00000008)
	Justifies the text to the bottom of the rectangle. This value must be combined with DT_SINGLELINE. 
	DT_CALCRECT (0x00000400)
	Determines the width and height of the rectangle. If there are multiple lines of text, ID3DXFont::DrawText uses the width of the rectangle pointed to by the pRect parameter and extends the base of the rectangle to bound the last line of text. If there is only one line of text, ID3DXFont::DrawText modifies the right side of the rectangle so that it bounds the last character in the line. In either case, ID3DXFont::DrawText returns the height of the formatted text but does not draw the text. 
	DT_CENTER (0x00000001)
	Centers text horizontally in the rectangle. 
	DT_EXPANDTABS (0x00000040)
	Expands tab characters. The default number of characters per tab is eight.
	DT_LEFT (0x00000000)
	Aligns text to the left. 
	DT_NOCLIP (0x00000100)
	Draws without clipping. ID3DXFont::DrawText is somewhat faster when DT_NOCLIP is used. 
	DT_RIGHT (0x00000002)
	Aligns text to the right. 
	DT_RTLREADING
	Displays text in right-to-left reading order for bi-directional text when a Hebrew or Arabic font is selected. The default reading order for all text is left-to-right. 
	DT_SINGLELINE (0x00000020)
	Displays text on a single line only. Carriage returns and line feeds do not break the line. 
	DT_TOP (0x00000000)
	Top-justifies text. 
	DT_VCENTER (0x00000004)
	Centers text vertically (single line only). 
	DT_WORDBREAK (0x00000010)
	Breaks words. Lines are automatically broken between words if a word would extend past the edge of the rectangle specified by the pRect parameter. A carriage return/line feed sequence also breaks the line. 
	*/
	void SetTextFormat(DWORD dwFormat);
	//bool MsgProc(CGUIEvent *event=NULL);
	virtual void InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
	virtual void UpdateRects();
	virtual HRESULT Render(GUIState* pGUIState ,float fElapsedTime);	
	/** this function is similar to render(), except that it assume that all states have been set properly*/
	virtual HRESULT RenderInBatch(GUIState* pGUIState);

	virtual void Clone(IObject* pobj)const;
	virtual IObject* Clone()const;
	virtual const IType* GetType()const{return m_type;}

	/** set the text scale the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border. 
	This is great for rendering text in 3d scene with a boarder using just normal system font. */
	virtual void SetTextScale(float fScale);
	/** get the text scale the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border. 
	This is great for rendering text in 3d scene with a boarder using just normal system font. */
	virtual float GetTextScale();
	
	bool IsAutoSize() const;
	void SetAutoSize(bool val);
protected:
	static const IType* m_type;
	/// Text to output to screen
	std::u16string		m_szText;
	bool	m_bAutoSize;
	
	bool	m_bNeedCalRect;
	/** the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border. 
	This is great for rendering text in 3d scene with a boarder using just normal system font. */
	float	m_text_scale;
};
}