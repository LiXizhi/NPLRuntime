#pragma once
#include "PainterState.h"

namespace ParaEngine
{
	class CPaintEngineGPUState : public CPainterState
	{
	public:
		CPaintEngineGPUState(CPaintEngineGPUState &other);
		CPaintEngineGPUState();
		virtual ~CPaintEngineGPUState();

		/** set the global transform to sprite object. */
		virtual void SetSpriteTransform(const Matrix4 * pMatrix = NULL);

		/** set sprite use world matrix. */
		virtual void SetSpriteUseWorldMatrix(bool bEnable);

		virtual bool IsSpriteUseWorldMatrix();

	public:
		bool m_bSpriteUseWorldMatrix : 1;
		bool isNew : 1;
		bool needsClipBufferClear : 1;
		bool clipTestEnabled : 1;
		bool canRestoreClip : 1;
		bool matrixChanged : 1;
		bool compositionModeChanged : 1;
		bool opacityChanged : 1;
		bool renderHintsChanged : 1;
		bool clipChanged : 1;
		bool currentClip : 8;

		int32 m_nStencilValue;
		QRect rectangleClip;
	};
}