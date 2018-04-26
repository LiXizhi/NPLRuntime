// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

namespace cAudio
{
	class IThread
	{
	public:
		virtual bool start() = 0;
		virtual void join() = 0;
		virtual void shutdown() = 0;
		virtual bool isRunning() = 0;
	};

	class IThreadWorker
	{
	public:
		virtual void run() = 0;
	};
}