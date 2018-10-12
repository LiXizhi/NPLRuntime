#pragma once

namespace ParaEngine
{
	class LibGL
	{
	public:
		LibGL() = default;
		~LibGL() = default;

		static void BindTexture2D(int slot, int texture);
		static void UseProgram(int program);
		static void ClearCache();	
	};
}