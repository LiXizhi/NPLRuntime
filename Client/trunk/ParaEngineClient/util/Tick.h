#pragma once
namespace ParaEngine
{
	/** Check to see if it is a tick.  usually used by class with a real time framemove function. */
	class Tick
	{
	public:
		Tick();
		Tick(float fInterval);
		void SetFPS(float fFPS);

		/* in seconds */
		void SetInterval(float fInterval);
		/* in seconds */
		float GetInverval() const;

		/* check to see if we should tick. For example, some function may be called with deltaTime in 30fps,
		however, we only want to process at 20FPS, such as physics, we can use this function is easily limit function calling rate.
		@param deltaTime: delta time in seconds, since last call
		*/
		bool IsTick(float deltaTime);
	protected:
		float m_fElapsedTime;
		float m_invervalSeconds;
	};
}