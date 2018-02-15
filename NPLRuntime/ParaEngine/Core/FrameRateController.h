#pragma once

namespace ParaEngine
{
/**
the class for all frame rate controller.
The class is used to generate smooth frame rate
call FrameMove() or FrameMoveDelta() at every frame move
*/
class CFrameRateController
{
public:
	enum ControllerType{
		FRC_NONE = 0,	// output delta time without interpolation
		FRC_CONSTANT,	// output a constant delta time disregarding the input time
		FRC_CONSTANT_OR_ABOVE,	// output a constant delta time or a smaller one
		FRC_FIRSTORDER,	// interpolating using a linear function
		FRC_SECONDORDER,	// interpolating using a second order function
		FRC_BELOW,	// ensure that frame rate is smaller than a given value. 
		FRC_CONSTANT_OR_BELOW,	// output a constant delta time or a smaller one
	};

	ControllerType	m_nType;		/// Frame Rate Controller Type
	float	m_fConstDeltaTime;		/// the Ideal delta Time rate
	float	m_fMaxDeltaTime;		/// the largest delta Time we could tolerate
	float	m_fMinDeltaTime;		/// the smallest delta Time we allow, some module need frame rate to be under a certain value
	
	float	m_fLinearSlope;			/// Slope for linear interpolation.d(dT)/d(n)
	float	m_fMaxLinearDeltaTime;	/// the largest delta Time we could tolerate at linear interpolation mode
	double	m_fNextTime;			/// the next ideal time to be called. 
public:

	double	m_fTime;				/// current time
	double	m_fLastTime;			/// last time, when framemove() is called

	double	m_fElapsedTime;			/// current elapsed time
	double	m_fLastElapsedTime;		/// last elapsed time, since framemove()

public:
	CFrameRateController(ControllerType type);
	CFrameRateController(void);
	~CFrameRateController(void);
public:
	/** get the time in seconds*/
	double	GetTime(){return m_fTime;}
	/** get the elapsed time: this function is disabled for a reason. Never use GetElapsedTime(). use FrameMove() instead.*/
	//double	GetElapsedTime(){return m_fElapsedTime;}
public:
	/** set the type of the controller */
	void SetType(ControllerType nType);

	/** set const delta time. */
	void SetConstDeltaTime(double fConstDeltaTime);

	/** frame move by an absolute current time */
	double FrameMove(double time);

	/** frame move by a delta time */
	double FrameMoveDelta(double deltatime);

public:
	static void LoadFRCNormal(float fIdealInterval = -1.f);
	static void LoadFRCRealtime(float fIdealInterval = -1.f);
	static void LoadFRCCapture(int nFPS);
	static void LoadFRCServerMode();
};

}