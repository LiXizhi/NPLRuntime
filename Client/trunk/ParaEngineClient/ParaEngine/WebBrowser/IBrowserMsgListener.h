//////////////////////////////////////////////////////////////////////////
//there is a head file conflict problem with boost and winSock,so we
//put interface in a separete file to avoid cross include.

#pragma once

namespace ParaEngine
{
	class IBrowserMsgListener
	{
	public:
		virtual void OnWebPageLoaded() = 0;
	};
}