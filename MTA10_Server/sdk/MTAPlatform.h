/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/Platform.cpp
*  PURPOSE:     Platform-specific defines and methods
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __PLATFORM_H
#define __PLATFORM_H

extern "C" bool g_bSilent;
extern "C" bool g_bNoTopBar;

/** Operating system identifiers **/
#if defined(WIN32)
	#define MTA_OS_STRING		"Windows"
	#define MTA_LIB_EXTENSION	".dll"
	#if defined(_DEBUG)
		#define MTA_LIB_SUFFIX	"_d"
	#else
		#define MTA_LIB_SUFFIX
	#endif
#elif defined(__linux__)
	#define MTA_OS_STRING		"GNU/Linux"
	#define MTA_LIB_EXTENSION	".so"
	#define MTA_LIB_SUFFIX
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	#define MTA_OS_STRING		"BSD"
	#define MTA_LIB_EXTENSION	".so"
	#define MTA_LIB_SUFFIX
#elif defined(__APPLE__) && defined(__MACH__)
	#define MTA_OS_STRING		"Mac OS X"
	#define MTA_LIB_EXTENSION	".so"
	#define MTA_LIB_SUFFIX
#else
	#error "Unsupported operating system"
#endif

/** Multi-platform defines **/
#ifdef WIN32
	/* Win32 */

	// Define includes
	#include <conio.h>
	#include <direct.h>
	#include <windows.h>

	// Define functions
	#define Print printf

	// Define types
	#define socklen_t int

	// Define keys
	#define KEY_BACKSPACE	0x08
	#define KEY_EXTENDED	0xE0
	#define KEY_LEFT	0x4B
	#define KEY_RIGHT	0x4D
	#define KEY_UP		0x48
	#define KEY_DOWN	0x50
#else
	/* POSIX */

	// Define includes
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <string.h>
	#include <string>
	#include <fcntl.h>
	#include <curses.h>
	#include <dlfcn.h>
	#include <sys/time.h>
	#include <sys/times.h>
	
	// Non-standard hash include
	#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
		#include <hash_fun.h>
	#else
		#include <ext/hash_fun.h>
	#endif

	#define MAX_PATH 255
	#define Print printw

	#define _vsnprintf vsnprintf
	#define _snprintf snprintf

	#ifndef stricmp
	#define stricmp strcasecmp
	#endif
	#ifndef strcmpi
	#define strcmpi strcasecmp
	#endif

	#define _copysign copysign

	#ifndef Sleep
		#define Sleep(duration) usleep(duration * 1000)
	#endif

	// Win32 wrapper function
	unsigned long GetTickCount ( void );

	// Itoa replacement function
	char* itoa ( int value, char* result, int base );

	// Hash function
	namespace __gnu_cxx
	{
		template<> struct hash < std::string >
		{
			size_t operator()( const std::string& str ) const
			{
				return hash< const char* >()( str.c_str() );
			}	
		};
	}
#endif

// This function should be used instead of mkdir to preserve multiplatform
// compatibility
extern int mymkdir ( const char* dirname );

// Set up export type definition for Win32
#ifdef WIN32
	#define MTAEXPORT extern "C" __declspec(dllexport)
#else
	#define MTAEXPORT extern "C"
#endif

#endif
