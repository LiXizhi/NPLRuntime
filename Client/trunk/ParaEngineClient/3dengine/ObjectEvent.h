#pragma once

namespace ParaEngine
{
	/// event that can be passed to scene object
	struct ObjectEvent
	{
	protected:
		std::string			m_sEvent;
		int				m_nEvent;

	public:
		const std::string&			GetEventString() const;
		void			SetEvent(const char* str);
		void			SetEvent(int nEvent);
		bool operator == (const ObjectEvent& r) const;
		static int		ParseEvent(const std::string& sEvent);

	public:
		ObjectEvent();
		ObjectEvent(const char* str, int nEvent = 0);
		~ObjectEvent();

	};
}