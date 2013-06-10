/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/Core.cpp
*  PURPOSE:     Core library entry point
*  DEVELOPERS:  Derek Abdine <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#define DECLARE_PROFILER_SECTION_Core
#include "profiler/SharedUtil.Profiler.h"

CCore* g_pCore = NULL;
CGraphics* g_pGraphics = NULL;
bool IsRealDeal ( void );

int WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, PVOID pvNothing)
{
    CFilePathTranslator     FileTranslator;
    std::string             WorkingDirectory;

    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        WriteDebugEvent( SString( "DLL_PROCESS_ATTACH %08x", pvNothing ) );
        if ( IsRealDeal () )
        {
            FileTranslator.GetGTARootDirectory ( WorkingDirectory );
            SetCurrentDirectory ( WorkingDirectory.c_str ( ) );

            // For dll searches, this call replaces the current directory entry and turns off 'SafeDllSearchMode'
            // Meaning it will search the supplied path before the system and windows directory.
            // http://msdn.microsoft.com/en-us/library/ms682586%28VS.85%29.aspx
            SetDllDirectory( CalcMTASAPath ( "MTA" ) );

            g_pCore = new CCore;

            FileTranslator.GetGTARootDirectory ( WorkingDirectory );
            SetCurrentDirectory ( WorkingDirectory.c_str ( ) );
        }
    } 
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        WriteDebugEvent( SString( "DLL_PROCESS_DETACH %08x", pvNothing ) );
        if ( IsRealDeal () )
        {
            // For now, TerminateProcess if any destruction is attempted (or we'll crash)
            TerminateProcess ( GetCurrentProcess (), 0 );

            if ( g_pCore )
            {
                delete g_pCore;
                g_pCore = NULL;
            }
        }
    }

    return TRUE;
}


//
// Returns true if dll has been loaded with GTA.
//
bool IsRealDeal ( void )
{
    // Get current module full path
    char szBuffer[64000];
    GetModuleFileName ( NULL, szBuffer, sizeof(szBuffer) - 1 );
    WriteDebugEvent( SString( "ModuleFileName: %s", szBuffer ) );
    if ( SStringX( szBuffer ).EndsWithI( "gta_sa.exe" ) )
        return true;
    return false;
}
