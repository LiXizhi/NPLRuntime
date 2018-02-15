#pragma  once
namespace ParaEngine
{
	class IRenderWindow
	{
	public:
		IRenderWindow() = default;
		virtual ~IRenderWindow() = default;
	public:
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
	};
}