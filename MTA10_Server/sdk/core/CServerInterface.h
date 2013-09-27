/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/core/CServerInterface.h
*  PURPOSE:     Core server interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CSERVERINTERFACE_H
#define __CSERVERINTERFACE_H

#include "net/CNetServer.h"
#include "CModManager.h"
#include <xml/CXML.h>

class CServerInterface
{
public:
    virtual CNetServer*     GetNetwork          ( void ) = 0;
    virtual CModManager*    GetModManager       ( void ) = 0;
    virtual CXML*           GetXML              ( void ) = 0;

    virtual const char*     GetServerModPath    ( void ) = 0;
    virtual SString         GetAbsolutePath     ( const char* szRelative ) = 0;

    virtual void            Printf              ( const char* szFormat, ... ) = 0;
    virtual bool            IsRequestingExit    ( void ) = 0;
};

#endif
