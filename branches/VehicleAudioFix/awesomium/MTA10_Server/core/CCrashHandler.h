/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CCrashHandler.h
*  PURPOSE:     Crash handler class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CCRASHHANDLER_H
#define __CCRASHHANDLER_H

#include "CCrashHandlerAPI.h"

#ifdef WIN32
    #include <windows.h>
#else
    #include <signal.h>
#endif

class CCrashHandler
{
public:
    static void                 Init                    ( const SString& strServerPath );

private:
    #ifdef WIN32

    static long WINAPI          HandleExceptionGlobal   ( _EXCEPTION_POINTERS* pException );
    static void                 DumpMiniDump            ( _EXCEPTION_POINTERS* pException, CExceptionInformation* pExceptionInformation );

    static void                 RunErrorTool            ( void );

    #else

    static void                 HandleExceptionGlobal   ( int iSig );

    #endif
};


#endif
