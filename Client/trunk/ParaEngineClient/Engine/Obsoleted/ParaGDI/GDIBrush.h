#pragma once
namespace ParaEngine
{
	struct GUITextureElement;
	/**
	* CGDIBrush is an abstract base class that defines a CGDIBrush object. A CGDIBrush object is used to 
	* paint the interior of graphics shapes, such as rectangles, ellipses, pies, polyline, and paths
	*/
	class CGDIBrush {
	public:
		virtual int GetBrushType()const {return m_eType;}
		virtual CGDIBrush *Clone()const;
		void SetTexture(TextureEntity* pTex) {m_pTexture=pTex;}
		TextureEntity* GetTexture()const {return m_pTexture;}
		void GetRect(RECT *rect)const{*rect=m_rect;}
		~CGDIBrush();
		CGDIBrush &operator =(const CGDIBrush& brush);
		bool operator ==(const CGDIBrush& brush);
		friend class CGDIPen;
	protected:
		//we do not allow direct creation of a brush
		CGDIBrush(){};
		TextureEntity *m_pTexture;
		RECT	m_rect;
		//brush type
		int m_eType;

	};
	/**
	* The CGDISolidBrush class defines a solid color CGDIBrush object. A CGDIBrush object is used to fill in 
	* shapes similar to the way a paint brush can paint the inside of a shape. This class inherits from the CGDIBrush abstract base class.
	*/
	class CGDISolidBrush:public CGDIBrush
	{
	public:
		CGDISolidBrush(const Color &color);
		Color GetColor()const{return m_color;}
		void SetColor(const Color &color){m_color=color;}
	protected:
		Color m_color;
	};
	/**
	* The CGDITextureBrush class defines a CGDIBrush object that contains an texture object that is used for the fill. 
	* The fill image can be transformed by using the local Matrix object contained in the Brush object
	*/
	class CGDITextureBrush:public CGDIBrush
	{
	public:
		CGDITextureBrush(TextureEntity *pTexture, const RECT *rect); 
	};
}
