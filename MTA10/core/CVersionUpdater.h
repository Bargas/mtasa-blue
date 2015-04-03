/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CVersionUpdater.h
*  PURPOSE:     Header file for version update check and message dialog
*  DEVELOPERS:  ccccccccccccccccccccccccccccw
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CVERSIONUPDATER_H
#define __CVERSIONUPDATER_H


class CVersionUpdaterInterface
{
public:
    virtual                ~CVersionUpdaterInterface        ( void ) {}
    virtual void            DoPulse                         ( void ) = 0;
    virtual void            InitiateUpdate                  ( const SString& strType, const SString& strHost ) = 0;
    virtual bool            IsOptionalUpdateInfoRequired    ( const SString& strHost ) = 0;
    virtual void            InitiateDataFilesFix            ( void ) = 0;
};

CVersionUpdaterInterface* GetVersionUpdater ();


#endif
