/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientSound.h
*  PURPOSE:     Sound entity class header
*  DEVELOPERS:  Stanislav Bobrov <lil_Toady@hotmail.com>
*               Marcus Bauer <mabako@gmail.com>
*
*****************************************************************************/

class CClientSoundManager;

#ifndef __CCLIENTSOUNDMANAGER_H
#define __CCLIENTSOUNDMANAGER_H

#include <list>
#include <irrKlang.h>
#include "CClientSound.h"

using namespace irrklang;

class CClientSoundManager : public ISoundStopEventReceiver
{
public:

                            CClientSoundManager         ( CClientManager* pClientManager );
                            ~CClientSoundManager        ( void );

    ISoundEngine*           GetEngine                   ( void )                    { return m_pSoundEngine; };

    void                    DoPulse                     ( void );

    inline unsigned short   GetDimension                ( void )                    { return m_usDimension; }
    void                    SetDimension                ( unsigned short usDimension );

    CClientSound*           PlaySound2D                 ( const char* szFile, bool bLoop );
    CClientSound*           PlaySound3D                 ( const char* szFile, CVector vecPosition, bool bLoop );

    void                    AddToList                   ( CClientSound* pSound )    { m_Sounds.push_back ( pSound ); }
    void                    RemoveFromList              ( CClientSound* pSound )    { m_Sounds.remove ( pSound ); }
    bool                    Exists                      ( CClientSound* pSound );
    CClientSound*           Get                         ( ISound* pSound );

    virtual void            OnSoundStopped              ( ISound* sound, E_STOP_EVENT_CAUSE reason, void* pObj );

    void                    UpdateVolume                ( void );

private:

    bool                        m_bUse3DBuffers;

    CClientManager*             m_pClientManager;
    ISoundEngine*               m_pSoundEngine;

    unsigned short              m_usDimension;

    std::list < CClientSound* > m_Sounds;
};

#endif
