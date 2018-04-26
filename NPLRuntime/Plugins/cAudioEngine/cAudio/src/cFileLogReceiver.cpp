// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cFileLogReceiver.h"

#if CAUDIO_COMPILE_WITH_FILE_LOG_RECEIVER == 1
#include <iostream>
#include <fstream>

namespace cAudio
{
	cFileLogReceiver::cFileLogReceiver()
	{
		firsttime = false;
	}

	cFileLogReceiver::~cFileLogReceiver()
	{

	}

    bool cFileLogReceiver::OnLogMessage(const char* sender, const char* message, LogLevel level, float time)
	{
		std::ofstream outf;
		
		if(firsttime == false)
		{
			if( !outf.is_open() )
			{
			// Reset log file
			outf.setf( std::ios::fixed );
			outf.precision( 3 );
			outf.open( "cAudioEngineLog.html", std::ios::out );
			
			if( !outf ){
				return false;
			}

			outf<<"<html>\n";
			outf<<"<head>\n";
			outf<<"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n";
			outf<<"<title>cAudio Log</title>\n";
			outf<<"<style type=\"text/css\">\n";
		
			outf<<"body, html {\n";
			outf<<"background: #000000;\n";
			outf<<"width: 1000px;\n";
			outf<<"font-family: Arial;\n";
			outf<<"font-size: 16px;\n";
			outf<<"color: #C0C0C0;\n";
			outf<<"}\n";

			outf<<"h1 {\n";
			outf<<"color : #FFFFFF;\n";
			outf<<"border-bottom : 1px dotted #888888;\n";
			outf<<"}\n";

			outf<<"pre {\n";
			outf<<"font-family : arial;\n";
			outf<<"margin : 0;\n";
			outf<<"}\n";

			outf<<".box {\n";
			outf<<"border : 1px dotted #818286;\n";
			outf<<"padding : 5px;\n";
			outf<<"margin: 5px;\n";
			outf<<"width: 950px;\n";
			outf<<"background-color : #292929;\n";
			outf<<"}\n";

			outf<<".err {\n";
			outf<<"color: #EE1100;\n";
			outf<<"font-weight: bold\n";
			outf<<"}\n";

			outf<<".warn {\n";
			outf<<"color: #FFCC00;\n";
			outf<<"font-weight: bold\n";
			outf<<"}\n";

			outf<<".crit {\n";
			outf<<"color: #BB0077;\n";
			outf<<"font-weight: bold\n";
			outf<<"}\n";

			outf<<".info {\n";
			outf<<"color: #C0C0C0;\n";
			outf<<"}\n";

			outf<<".debug {\n";
			outf<<"color: #CCA0A0;\n";
			outf<<"}\n";

			outf<<"</style>\n";
			outf<<"</head>\n\n";

			outf<<"<body>\n";
			outf<<"<h1>cAudio Log</h1>\n";
			outf<<"<h3>" << "2.3.0" << "</h3>\n";
			outf<<"<div class=\"box\">\n";
			outf<<"<table>\n";

			outf.flush();
			
			}
			firsttime = true;
		}
		else
		{
			outf.open( "cAudioEngineLog.html", std::ios::out | std::ios::app );
			
			if( !outf ){
				return false;
			}

			outf<<"<tr>\n";
			outf<<"<td width=\"100\">";
			outf<<time;
			outf <<"</td>\n";
			outf<<"<td class=\"";
		
			switch( level )
			{
			case ELL_DEBUG:
				outf<<"debug";
				break;

			case ELL_INFO:
				outf<<"info";
				break;

			case ELL_WARNING:
				outf<<"warn";
				break;

			case ELL_ERROR:
				outf<<"err";
				break;

			case ELL_CRITICAL:
				outf<<"crit";
				break;

			case ELL_COUNT:
				outf<<"debug";
				break;

			default:
				outf<<"debug";
			}
		
			outf<<"\"><pre>\n";
			outf<<message;
			outf<<"\n</pre></td>\n";
			outf<<"</tr>\n";

			outf.flush();

		}
		outf.close();
		return true;
	}
};

#endif




