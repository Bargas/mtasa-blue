/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientCamera.h
*  PURPOSE:     Camera entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*
*****************************************************************************/

#ifndef __CCLIENTCAMERA_H
#define __CCLIENTCAMERA_H

#include "CClientCommon.h"
#include "CClientEntity.h"

class CClientEntity;
class CClientManager;
class CClientPlayer;
class CClientPlayerManager;

enum eClientCameraAttachMode
{
    CLIENTCAMERA_3RDPERSON,
};

class CClientCamera : public CClientEntity
{
    friend CClientManager;

public:
    inline void                 Unlink                      ( void )                        { };
    void                        DoPulse                     ( void );

    inline eClientEntityType    GetType                     ( void ) const                  { return CCLIENTCAMERA; };

    bool                        GetMatrix                   ( CMatrix& Matrix ) const;
    void                        GetPosition                 ( CVector& vecPosition ) const;
    void                        GetRotation                 ( CVector& vecRotation ) const;
    void                        SetPosition                 ( const CVector& vecPosition );
    void                        SetRotation                 ( const CVector& vecRotation );
    void                        GetTarget                   ( CVector& vecTarget ) const;
    void                        SetTarget                   ( const CVector& vecPosition );

    void                        FadeIn                      ( float fTime );
    void                        FadeOut                     ( float fTime, unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue );

    inline CClientPlayer*       GetFocusedPlayer            ( void )                        { return m_pFocusedPlayer; };
    void                        SetFocus                    ( CClientEntity* pEntity, eCamMode eMode, bool bSmoothTransition = false );
    void                        SetFocus                    ( CClientPlayer* pPlayer, eCamMode eMode, bool bSmoothTransition = false );
    void                        SetFocus                    ( CVector * vecTarget, bool bSmoothTransition );
    void                        SetFocusToLocalPlayer       ( void );

    bool                        SetCameraMode               ( eCamMode eMode );               

    inline bool                 IsInFixedMode               ( void )                        { return m_bFixed; }

    void                        ToggleCameraFixedMode       ( bool bEnabled );

    CClientEntity *             GetTargetEntity             ( void );

    void                        UnreferencePlayer           ( CClientPlayer* pPlayer );

private:
                                CClientCamera               ( CClientManager* pManager );
                                ~CClientCamera              ( void );

    static void                 ProcessFixedCamera          ( CCam* pCam );

    void                        SetFocusToLocalPlayerImpl   ( void );

    void                        UnreferenceEntity           ( CClientEntity* pEntity );    
    void                        InvalidateEntity            ( CClientEntity* pEntity );
    void                        RestoreEntity               ( CClientEntity* pEntity );

    CClientPlayerManager*       m_pPlayerManager;

    CClientPlayer*              m_pFocusedPlayer;
    CClientEntity*              m_pFocusedEntity;
    CEntity*                    m_pFocusedGameEntity;
    bool                        m_bInvalidated;

    bool                        m_bFixed;
    CVector                     m_vecFixedPosition;
    CVector                     m_vecFixedTarget;

    CCamera*                    m_pCamera;
};

#endif
