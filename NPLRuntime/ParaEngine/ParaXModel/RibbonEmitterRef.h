#pragma once

namespace ParaEngine
{
	class RibbonEmitter;

	/** Holding a reference to a RibbonEmitter's parent CParaXModel so global ribbon trails can finish decaying safely. */
	struct RibbonEmitterRef
	{
		RibbonEmitterRef() : m_ptr(nullptr) {}
		explicit RibbonEmitterRef(RibbonEmitter* ptr);
		RibbonEmitterRef(const RibbonEmitterRef& r);
		~RibbonEmitterRef();

		RibbonEmitterRef& operator=(const RibbonEmitterRef& r);

		inline RibbonEmitter* get() const { return m_ptr; }
		operator bool() const { return m_ptr != nullptr; }
		bool operator!() const { return m_ptr == nullptr; }
		RibbonEmitter& operator*() const { PE_ASSERT(m_ptr != nullptr); return *m_ptr; }
		RibbonEmitter* operator->() const { PE_ASSERT(m_ptr != nullptr); return m_ptr; }

		inline bool operator==(const RibbonEmitterRef& b) const { return this->get() == b.get(); }
		inline bool operator!=(const RibbonEmitterRef& b) const { return this->get() != b.get(); }
		inline bool operator==(RibbonEmitter const* b) const { return this->get() == b; }
		inline bool operator!=(RibbonEmitter const* b) const { return this->get() != b; }

	protected:
		RibbonEmitter* m_ptr;
	};
}
