#pragma once
#include "GDIBrush.h"
namespace ParaEngine
{
	struct TextureEntity;
	/**
	* a pen object in the GDI class
	*/
	class CGDIPen
	{
	public:
		CGDIPen();
		CGDIPen(const CGDIBrush &brush,float width=1.0);
		CGDIPen(Color color, float width=1.0);
		CGDIPen(TextureEntity *pTexture, float width=1.0);
		/**
		* Get the CGDIBrush object that is currently set for this CGDIPen object
		*/
		CGDIBrush* GetBrush() const;
		CGDIPen* Clone()const ;
		/**
		* Get the color of this pen
		*/
		Color GetColor()const;
		void SetColor(const Color &color);

		/** if it is fully transparent. */
		bool IsTransparent() const;

		/**
		* Get the width of this pen, in pixels by default. 
		*/
		float GetWidth()const{return m_fWidth;};
		void SetWidth(float width){m_fWidth=width;};

		/**
		* Get the line start cap type
		*/
		int GetStartCap( )const{return m_eLineStartCap;}
		void SetStartCap(int linecap){m_eLineStartCap=linecap;}
		/**
		* Get the line end cap type
		*/
		int GetEndCap( )const{return m_eLineEndCap;}
		void SetEndCap(int linecap){m_eLineEndCap=linecap;}

		/**
		* Get the pen's scale type
		*/
		int GetScaleType()const {return m_eScale;}
		void SetScaleType(int scaletype){m_eScale=scaletype;}

		~CGDIPen();
		static CGDIPen &GetDefaultPen();
		CGDIPen &operator =(const CGDIPen &pen);
		bool operator ==(const CGDIPen &pen);
	protected:
		Color m_color;
		float m_fWidth;
		CGDIBrush m_brush;
		int m_eLineStartCap;
		int m_eLineEndCap;
		int m_eScale;
	};
}