/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CObject.h
*  PURPOSE:     Object entity class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/
#ifndef __COBJECT_H
#define __COBJECT_H

//Kayl: There is now too many includes here, try to make it work with StdInc.h if possible
#include "CElement.h"
#include "CEvents.h"
#include "Utils.h"

#include "CEasingCurve.h"
#include "TInterpolation.h"
#include "CPositionRotationAnimation.h"

class CObject : public CElement
{    

public:
    explicit                    CObject                 ( CElement* pParent, CXMLNode* pNode, class CObjectManager* pObjectManager );
    explicit                    CObject                 ( const CObject& Copy );
                                ~CObject                ( void );

    bool                        IsEntity                ( void )                    { return true; }

    void                        Unlink                  ( void );
    bool                        ReadSpecialData         ( void );

    const CVector&              GetPosition             ( void );
    void                        SetPosition             ( const CVector& vecPosition );

    void                        GetRotation             ( CVector & vecRotation );
    void                        SetRotation             ( const CVector& vecRotation );

    bool                        IsMoving                ( void );
    void                        Move                    ( const CPositionRotationAnimation& a_rMoveAnimation );
    void                        StopMoving              ( void );
    const CPositionRotationAnimation*   GetMoveAnimation    ( );

    void                        AttachTo                ( CElement* pElement );

    inline unsigned char        GetAlpha                ( void )                        { return m_ucAlpha; }
    inline void                 SetAlpha                ( unsigned char ucAlpha )       { m_ucAlpha = ucAlpha; }

    inline unsigned short       GetModel                ( void )                        { return m_usModel; }
    inline void                 SetModel                ( unsigned short usModel )      { m_usModel = usModel; }

    inline float                GetScale                ( void )                        { return m_fScale; }
    inline void                 SetScale                ( float fScale )                { m_fScale = fScale; }

    inline bool                 GetCollisionEnabled     ( void )                        { return m_bCollisionsEnabled; }
    inline void                 SetCollisionEnabled     ( bool bCollisionEnabled )      { m_bCollisionsEnabled = bCollisionEnabled; }

    inline bool                 IsStatic                ( void )                        { return m_bIsStatic; }
    inline void                 SetStatic               ( bool bStatic )                { m_bIsStatic = bStatic; }

private:
    CObjectManager*             m_pObjectManager;
    char                        m_szName [MAX_ELEMENT_NAME_LENGTH + 1];
    CVector                     m_vecRotation;
    unsigned char               m_ucAlpha;
    unsigned short              m_usModel;
    float                       m_fScale;
    bool                        m_bIsStatic;

protected:
    bool                        m_bCollisionsEnabled;

public:
    CPositionRotationAnimation* m_pMoveAnimation;
};

#endif
