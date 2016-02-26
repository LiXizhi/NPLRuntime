#pragma once

namespace ParaEngine
{
	class CPaintEngine;
	class CPainter;

	/** base class for a surface to paint to. The CRenderTarget subclass this. 
	*/
	class CPaintDevice
	{
	protected:
		/** refcount */
		int32        painters;
	public:
		enum PaintDeviceMetric {
			PdmWidth = 1,
			PdmHeight,
			PdmDepth,
		};

		virtual ~CPaintDevice();

		virtual int devType() const;
		inline bool paintingActive() const{
			return (painters != 0);
		};
		
		/** Returns the paint engine.

		Note that this function should not be called explicitly by the
		user, since it's meant for reimplementation purposes only. The
		function is called internally, and the default
		implementation may not always return a valid pointer.
		*/
		virtual CPaintEngine * paintEngine() const = 0;

		int width() const { return metric(PdmWidth); }
		int height() const { return metric(PdmHeight); }
		int depth() const { return metric(PdmDepth); }
		
		virtual float GetUIScalingX() const { return 1.f; }
		virtual float GetUIScalingY() const { return 1.f; }
		virtual float GetViewportLeft() const { return 0.f; };
		virtual float GetViewportTop() const { return 0.f; };
	protected:
		CPaintDevice();
		virtual int metric(PaintDeviceMetric metric) const;
		virtual void initPainter(CPainter *painter) const;
		virtual CPainter *sharedPainter() const;

		friend class CPainter;
	};
}

