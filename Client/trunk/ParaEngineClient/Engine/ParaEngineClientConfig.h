#pragma once

// the configured options and settings for ParaEngineServer
#define ParaEngineServer_VERSION_MAJOR 1
#define ParaEngineServer_VERSION_MINOR 0

/** Turn on this line to debug vertex shaders */
//#define DEBUG_VS   

/** Turn on this line to debug pixel shaders */
//#define DEBUG_PS   

/** ParaEngine PKG file version */
#define		PKG_FILE_VERSION		1

/** 
Turn on the NO_FRAME_RATE_CONTROL flag, the engine will force the card to render as fast as possible,
However, the fluctration of rendering rate will compromise with the simulation engine; the latter is likely to
generate un-smooth animations. The situation is aggravated when V-sync is on.
Turn off this flag, the engine will force the card to run under IDEAL_FRAME_RATE. In most cases, 
we should undefine this flag, unless we want to see the actual FPS for investigation.
*/
//#define NO_FRAME_RATE_CONTROL

/**  the ideal frame rate */
#define IDEAL_FRAME_RATE		(1/30.f)

/**  (one meter in the game engine) = (OBJ_UNIT system units)
one meter length in the object's unit system.*/
#define OBJ_UNIT		1.0f

/**  Define _USE_NORMAL_  if you want to compile in support for normalized terrain. Norm will slow both the loading and rendering performance
and it will usually be rendered with light0 */
//#define _USE_NORMAL_

/**  turn on FAST_RENDER, when you are debugging the application, the graphic will be less good though.*/
//#define FAST_RENDER
	
/**  Chosen based on the expected number of triangles that will be visible on-screen at any one time 
(the terrain mesh will typically have far more triangles than are seen at one time, especially with dynamic tessellation)
*/
#define MAX_NUM_VISIBLE_TERRAIN_TRIANGLES 20000

/**  the maximum number of characters in file names */
#define MAX_FILENAME_LEN	300

/**  if the macro is on, all models will be rendered without front cap, using ZFAIL shadow volume
this could be inaccurate for a number of cases, for instance, shadow will be transparent to walls
and far plane clipping is another problem. However, turn on the macro will increase performance */
//#define SHADOW_ZFAIL_WITHOUTCAPS

/**  the minimum shadow casting distance, if the projection of the object's height to the 2D screen is
smaller than this value(measured in pixels), then shadows will NOT be casted for it */
#define MIN_SHADOW_CAST_DISTANCE		20

/**  For camera projection matrix: careful with these values, since they will affect depth testing precision */
#define NEAR_PLANE	(0.5f*OBJ_UNIT)
#define FAR_PLANE	(150.f*OBJ_UNIT)
#define FIELDOFVIEW	60.0f

/**  sun height relative to the camera position */
#define SUN_HEIGHT			(100.f*OBJ_UNIT)

/** the minimum terrain detail threshold in pixels. details smaller than this will not be rendered. 
* the application can change threshold at runtime, but may not be set larger than this value.
* Currently this is a squared value.i.e. If it is 7 pixels, then the value should be 49.
*/
#define MIN_DETAILTHRESHOLD		50

/**
* The maximum length of the sql string
*/
#define MAX_SQL_LENGTH 2047
/**
* decide if we use boost regular expression library
*/
//#define USE_BOOST_REGULAR_EXPRESSION

/** default encoding for char * in GUI system.  UTF8 is a good choice, although ansi code page is also fine. */
//#define DEFAULT_GUI_ENCODING		CP_ACP
#define DEFAULT_GUI_ENCODING		CP_UTF8
