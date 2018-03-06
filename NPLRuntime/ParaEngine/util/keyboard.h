#pragma once
/**
* author: LiXizhi
* date : 2009.4.9
* desc: Cross Platform Key Board implementation
sets up a Unix and emulates the DOS <conio.h> functions kbhit() and getch():

// example: 
int main(int argc, char *argv[])
{
	while (!_kbhit()) {
	}
	int c = _getch(); // consume the character
}
*/

//////////////////////////////////////////////////////////////////////////
//
// sets up a Unix and emulates the DOS <conio.h> functions kbhit() and getch():
//
//////////////////////////////////////////////////////////////////////////

#ifdef WIN32
// for kbhit() function
#include <conio.h> 
#else
extern int _kbhit(); 
#endif
