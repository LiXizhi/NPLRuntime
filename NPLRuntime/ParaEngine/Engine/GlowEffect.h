#pragma once

namespace ParaEngine
{
	class CEffectFile;
	class IRenderDevice;
	class IRHIRenderTarget;

	/**
	* implementation of the full screen glow effect. This is just a simple post-rendering universal effect to 
	* be applied after the main scene is rendered. The effect looks similar to the glow effect as in the game Tron 2.0.
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

		// fGlowThreshold : pixel intensity less than fGlowThreshold has no glow effect.
		// fGlowAmount is related to glow area size and weight usually between [0,10], 0 means no glow --clayman 2011.7.19
		HRESULT Render(float fGlowThreshold = 0.7f, float fGlowAmount = 2.0f, bool bUseAlpha = false);

		/** this is just for debugging purposes. */
		void SaveGlowTexturesToFile(const std::string& filename);

	private:
		bool EnsureRenderTargets(IRenderDevice* pRenderDevice, uint32_t fullWidth, uint32_t fullHeight);
		static bool SaveRenderTargetToFile(IRenderDevice* pRenderDevice, IRHIRenderTarget* pRT, const std::string& filename);

		IRHIRenderTarget* m_rhiSceneCopyRT;
		IRHIRenderTarget* m_rhiGlowSourceRT;
		IRHIRenderTarget* m_rhiBlurHorizRT;

		uint32_t m_fullWidth;
		uint32_t m_fullHeight;
		float m_glowtextureWidth;
		float m_glowtextureHeight;

		// calculate blurring texture coordinate offset --clayman 2011.7.19
		void SetBlurEffectParameters(float dx, float dy, float blurAmount, CEffectFile* effect);
		float ComputeGaussian(float n, float blurAmount);
	};
}
