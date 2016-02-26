#pragma once

namespace ParaEngine
{
	class CPainter;

	/** for reading texture parameters from strings like
	"texture/whitedot.png:10 10 10 10", "texture/whitedot.png;0 0 64 64:10 10 10 10"
	*/
	class TextureParams
	{
	public:
		TextureParams(const char * str);

		/**
		* @param str: can be filename[; left top width height][:left top toright to bottom], such as
		* "texture/whitedot.png", "texture/whitedot.png;0 0 64 64", "texture/whitedot.png:10 10 10 10", "texture/whitedot.png;0 0 64 64:10 10 10 10"
		* [:left top toright to bottom] is used to specify the inner rect of the center tile in a nine texture element UI object
		*/
		void Init(const char * str);
		int m_left;
		int m_top;
		int m_right;
		int m_bottom;

		int m_toLeft;
		int m_toTop;
		int m_toRight;
		int m_toBottom;
		TextureEntity* m_pTexture;

		/** get the texture */
		TextureEntity* GetTexture() { return m_pTexture; }
		
		/** whether it is nine tile image. */
		inline bool IsNineTileTexture()
		{
			return m_toLeft != 0 || m_toTop != 0 || m_toRight != 0 || m_toBottom != 0;
		};

		/** if a sub region of the original image should be used. */
		inline bool HasSubRegion() { return m_right != 0; };

		/** return all image rects 
		* @param rcTextures: pass in pointer of RECT[9]. 
		* @return 0, 1 or 9 depending on the texture type. 0 means image rect is not determined. 
		*/
		int GetImageRects(RECT* rcTextures);

		/** draw texture to given target. this is a helper function.  
		* @param rcTextures: if NULL, it will use GetImageRects(). 
		* @param nCount: 1 or 9 depending on the texture type.  -1 means automatic and only valid if rcTextures is NULL.
		*/
		void drawTexture(CPainter* painter, int x, int y, int w, int h, const RECT* rcTextures = NULL, int nCount = -1);

	protected:
		void DrawNineTileTexture(CPainter* painter, int x, int y, int w, int h, const RECT* rcTextures);
		void DrawSingleTexture(CPainter* painter, int x, int y, int w, int h, const RECT &rc);
	};
}