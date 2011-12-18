/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CModuleLoader.h
*  PURPOSE:     Header file for module loader class
*  DEVELOPERS:  Derek Abdine <>
*               Alberto Alonso <rydencillo@gmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CMODULELOADER_H
#define __CMODULELOADER_H

#include <windows.h>
#include <string>

class CModuleLoader
{
public:
    
                    CModuleLoader       ( const std::string& ModuleName );
                    CModuleLoader       ( );
                    ~CModuleLoader      ( ); 

    bool            LoadModule          ( const std::string& ModuleName );
    void            UnloadModule        ( );

    inline bool     IsOk                ( ) const { return m_bStatus; }
    const SString&  GetLastErrorMessage ( void ) const;
    
    PVOID           GetFunctionPointer  ( const std::string& FunctionName );

private:
    HMODULE     m_hLoadedModule;
    bool        m_bStatus;
    SString     m_strLastError;
};

#endif