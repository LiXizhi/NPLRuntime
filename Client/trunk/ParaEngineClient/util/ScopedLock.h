#pragma once

namespace ParaEngine
{
	/** simple scoped lock for a boolean pointer. */
	class ScopedBoolean_Lock
	{
	public:
		ScopedBoolean_Lock(bool* pBoolValue) :
			m_pBoolValue(pBoolValue)
		{
			if(m_pBoolValue)
				*m_pBoolValue = true;
		}
		~ScopedBoolean_Lock() {
			if (m_pBoolValue)
				*m_pBoolValue = false;
		}
	private:
		bool* m_pBoolValue;
	};
}


