//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2007 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Email: LiXizhi@yeah.net
//-----------------------------------------------------------------------------
#pragma once

///////////////////////////////////////////////////////////////////////////
// most of the interface is from uBrowser project. see. uBrowser.com 

#include <string>
#include <map>

#define HTMLRENDERER_CLASS_ID		0x200A20

namespace ParaEngine
{

	class ParaEmbeddedBrowser;
	class ParaEmbeddedBrowserWindow;

	////////////////////////////////////////////////////////////////////////////////
	// data class that is passed with an event
	class ParaEmbeddedBrowserWindowEvent
	{
	public:
		ParaEmbeddedBrowserWindowEvent( int eventWindowIdIn, std::string uriIn ) :
		  mEventWindowId( eventWindowIdIn ),
			  mEventUri( uriIn )
		  {
		  };

		  // single int passed with the event - e.g. progress
		  ParaEmbeddedBrowserWindowEvent( int eventWindowIdIn, std::string uriIn, int intValIn ) :
		  mEventWindowId( eventWindowIdIn ),
			  mEventUri( uriIn ),
			  mIntVal( intValIn )
		  {
		  };

		  // string passed with the event
		  ParaEmbeddedBrowserWindowEvent( int eventWindowIdIn, std::string uriIn, std::string stringValIn ) :
		  mEventWindowId( eventWindowIdIn ),
			  mEventUri( uriIn ),
			  mStringVal( stringValIn )
		  {
		  };

		  // string and an int passed with the event
		  ParaEmbeddedBrowserWindowEvent( int eventWindowIdIn, std::string uriIn, std::string stringValIn, int intValIn ) :
		  mEventWindowId( eventWindowIdIn ),
			  mEventUri( uriIn ),
			  mStringVal( stringValIn ),
			  mIntVal( intValIn )
		  {
		  };

		  // 4 ints passed (semantically as a rectangle but could be anything - didn't want to make a RECT type structure)
		  ParaEmbeddedBrowserWindowEvent( int eventWindowIdIn, std::string uriIn, int xIn, int yIn, int widthIn, int heightIn ) :
		  mEventWindowId( eventWindowIdIn ),
			  mEventUri( uriIn ),
			  mXVal( xIn ),
			  mYVal( yIn ),
			  mWidthVal( widthIn ),
			  mHeightVal( heightIn )
		  {
		  };

		  virtual ~ParaEmbeddedBrowserWindowEvent()
		  {
		  };

		  int getEventWindowId() const
		  {
			  return mEventWindowId;
		  };

		  std::string getEventUri() const
		  {
			  return mEventUri;
		  };

		  int getIntValue() const
		  {
			  return mIntVal;
		  };

		  std::string getStringValue() const
		  {
			  return mStringVal;
		  };

		  void getRectValue( int& xOut, int& yOut, int& widthOut, int& heightOut ) const
		  {
			  xOut = mXVal;
			  yOut = mYVal;
			  widthOut = mWidthVal;
			  heightOut = mHeightVal;
		  };

	private:
		int mEventWindowId;
		std::string mEventUri;
		int mIntVal;
		std::string mStringVal;
		int mXVal;
		int mYVal;
		int mWidthVal;
		int mHeightVal;
	};

	////////////////////////////////////////////////////////////////////////////////
	// derive from this class and override these methods to observe these events
	class IEmbeddedBrowserWindowObserver
	{
	public:
		virtual ~IEmbeddedBrowserWindowObserver() { };
		typedef ParaEmbeddedBrowserWindowEvent EventType;

		virtual void onPageChanged( const EventType& eventIn ) { };
		virtual void onNavigateBegin( const EventType& eventIn ) { };
		virtual void onNavigateComplete( const EventType& eventIn ) { };
		virtual void onUpdateProgress( const EventType& eventIn ) { };
		virtual void onStatusTextChange( const EventType& eventIn ) { };
		virtual void onLocationChange( const EventType& eventIn ) { };
		virtual void onClickLinkHref( const EventType& eventIn ) { };
	};

	/**
	* NPL interface of a XML Web service client proxy 
	* this class is implemented by ParaEngine.Net plug-in. See ParaEnginePlugin/MozHTMLRenderer
	* class ID is HTMLRENDERER_CLASS_ID
	*/
	class IHTMLBrowserManager
	{
	public:
		/** delete this object when it is no longer needed.*/
		virtual void DeleteThis()=0;

		// housekeeping
		virtual bool init( std::string appBaseDirIn, std::string profileDirNameIn )=0;	
		virtual bool reset()=0;
		virtual bool clearCache()=0;
		virtual int getLastError()=0;														
		virtual const std::string getVersion()=0;											
		virtual void setBrowserAgentId( std::string idIn )=0;								

		// browser window - creation/deletion, mutation etc.
		virtual int createBrowserWindow( void* nativeWindowHandleIn, int browserWindowWidthIn, int browserWindowHeightIn )=0;
		virtual bool destroyBrowserWindow( int browserWindowIdIn )=0;
		virtual bool setSize( int browserWindowIdIn, int widthIn, int heightIn )=0;
		virtual bool scrollByLines( int browserWindowIdIn, int linesIn )=0;
		virtual bool setBackgroundColor( int browserWindowIdIn, const int redIn, const int greenIn, const int blueIn )=0;
		virtual bool setEnabled( int browserWindowIdIn, bool enabledIn )=0;

		// add/remove yourself as an observer on browser events - see IEmbeddedBrowserWindowObserver declaration
		virtual bool addObserver( int browserWindowIdIn, IEmbeddedBrowserWindowObserver* subjectIn )=0;
		virtual bool remObserver( int browserWindowIdIn, IEmbeddedBrowserWindowObserver* subjectIn )=0;

		// navigation - self explanatory
		virtual bool navigateTo( int browserWindowIdIn, const std::string uriIn )=0;
		virtual bool navigateStop( int browserWindowIdIn )=0;
		virtual bool canNavigateBack( int browserWindowIdIn )=0;
		virtual bool navigateBack( int browserWindowIdIn )=0;
		virtual bool canNavigateForward( int browserWindowIdIn )=0;
		virtual bool navigateForward( int browserWindowIdIn )=0;

		// access to rendered bitmap data
		virtual const unsigned char* grabBrowserWindow( int browserWindowIdIn )=0;		// renders page to memory and returns pixels
		virtual const unsigned char* getBrowserWindowPixels( int browserWindowIdIn )=0;	// just returns pixels - no render
		virtual const int getBrowserWidth( int browserWindowIdIn )=0;						// current browser width (can vary slightly after page is rendered)
		virtual const int getBrowserHeight( int browserWindowIdIn )=0;					// current height
		virtual const int getBrowserDepth( int browserWindowIdIn )=0;						// depth in bytes
		virtual const int getBrowserRowSpan( int browserWindowIdIn )=0;					// width in pixels * depth in bytes

		// mouse/keyboard interaction
		virtual bool mouseDown( int browserWindowIdIn, int xPosIn, int yPosIn )=0;		// send a mouse down event to a browser window at given XY in browser space
		virtual bool mouseUp( int browserWindowIdIn, int xPosIn, int yPosIn )=0;			// send a mouse up event to a browser window at given XY in browser space
		virtual bool mouseMove( int browserWindowIdIn, int xPosIn, int yPosIn )=0;		// send a mouse move event to a browser window at given XY in browser space
		virtual bool keyPress( int browserWindowIdIn, int keyCodeIn )=0;					// send a key press event to a browser window 
		virtual bool focusBrowser( int browserWindowIdIn, bool focusBrowserIn )=0;		// set/remove focus to given browser window

		// accessor/mutator for scheme that browser doesn't follow - e.g. secondlife.com://
		virtual void setNoFollowScheme( int browserWindowIdIn, std::string schemeIn )=0;
		virtual std::string getNoFollowScheme( int browserWindowIdIn )=0;
	};
}