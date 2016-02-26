#pragma once

namespace ParaEngine
{
	/**
	* implementation of the full screen glow effect. This is just a simple post-rendering universal effect to 
	* be applied after the main scene is rendered.The effect looks similar to the glow effect as in the game Tron 2.0.
	* by default, it works on the current render target. One can call CGlowEffect::Render() at any time of the render pipeline.
	*/
	class CGlowEffect
	{
	public:
		CGlowEffect(void);
		~CGlowEffect(void);
	public:
		HRESULT InvalidateDeviceObjects(); // called just before device is Reset
		HRESULT RestoreDeviceObjects();    // called when device is restored

		/**
		* apply the full screen glow effect to the current render target.
		* @param fDestBlendFactor: the destination blend factor. default value is one, where an addictive operation will be used for the back buffer and glow
		*	if this is 0, only the glow texture is rendered. Anything in the middle will reduce the destination color.
		* @param bUseAlpha if true the glow source's color will be multiplied by its alpha for each pixels.
		*	this allows pre-rendered scene to control exactly which portion of the scene is glowing on a per pixel basis. 
		*	One may use the object to camera distance or glow texture alpha to generate the alpha in previous rendering stages. 
		*	however, by default this is false. which the half sized version of the back buffer is used unmodified as the glow source. 
		* @return S_OK if succeed
		*/

		//the first and second parameter on longer stands for blend factor and we don't use bUseAlpha any more;
		//fGlowThreshold : pixel intensity less than fGlowThreshold has no glow effect.
		//fGlowAmount is related to glow area size and weight usually between [0,10] , 0 means no glow   --clayman 2011.7.19
		HRESULT Render(float fGlowThreshold = 0.7f, float fGlowAmount= 2.0f, bool bUseAlpha = false);

		/** this is just for debugging purposes.*/
		void SaveGlowTexturesToFile(const string& filename);
	private:
		/** holding the current back buffer information. */
		LPDIRECT3DSURFACE9 m_pBackBuffer, m_pZBuffer; 

		LPDIRECT3DTEXTURE9 m_pRTHalfSizeTexture;
		LPDIRECT3DTEXTURE9 m_pRTGlowSourceTexture;
		LPDIRECT3DTEXTURE9 m_pRTBlurHorizTexture;
		LPDIRECT3DTEXTURE9 m_pRTBlurVertTexture; // usually same RT texture as m_pRTGlowSourceTexture

		LPDIRECT3DSURFACE9 m_pRTHalfSizeSurface;
		LPDIRECT3DSURFACE9 m_pRTGlowSourceSurface;
		LPDIRECT3DSURFACE9 m_pRTBlurHorizSurface;
		LPDIRECT3DSURFACE9 m_pRTBlurVertSurface;
		
		float m_glowtextureWidth;
		float m_glowtextureHeight;

		//calculate blurring texture coordinate offset   --clayman 2011.7.19
		void SetBlurEffectParameters(float dx,float dy,float blurAmount,CEffectFile* effect);
		float ComputeGaussian(float n,float blurAmount);
	};

}
