/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClient3DMarker.h
*  PURPOSE:     3D marker entity class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*
*****************************************************************************/

#ifndef __CCLIENT3DMARKER_H
#define __CCLIENT3DMARKER_H

#include "CClientMarker.h"
class CClientMarker;

class CClient3DMarker : public CClientMarkerCommon
{
public:
    enum
    {
        TYPE_CYLINDER,
        TYPE_ARROW,
        TYPE_INVALID,
    };

                                    CClient3DMarker                     ( CClientMarker * pThis );
                                    ~CClient3DMarker                    ( void );

    inline unsigned int             GetMarkerType                       ( void ) const                      { return CClientMarkerCommon::CLASS_3DMARKER; };

    unsigned long                   Get3DMarkerType                     ( void );
    void                            Set3DMarkerType                     ( unsigned long ulType );

    bool                            IsHit                               ( const CVector& vecPosition ) const;

    inline void                     GetPosition                         ( CVector& vecPosition ) const      { vecPosition = m_vecPosition; };
    void                            SetPosition                         ( const CVector& vecPosition );

	inline void						GetUp								( CVector& vecUp ) const			{ vecUp = m_vecUp; };
	void							SetUp								( const CVector& vecUp );

    inline bool                     IsVisible                           ( void ) const                      { return m_bVisible; };
    inline void                     SetVisible                          ( bool bVisible )                   { m_bVisible = bVisible; };

    inline unsigned long            GetColor                            ( void )                            { return m_rgbaColor; };
    void                            GetColor                            ( unsigned char& Red, unsigned char& Green, unsigned char& Blue, unsigned char& Alpha ) const;
    inline unsigned char            GetColorRed                         ( void ) const                      { return static_cast < unsigned char > ( m_rgbaColor ); };
    inline unsigned char            GetColorGreen                       ( void ) const                      { return static_cast < unsigned char > ( m_rgbaColor >> 8 ); };
    inline unsigned char            GetColorBlue                        ( void ) const                      { return static_cast < unsigned char > ( m_rgbaColor >> 16 ); };
    inline unsigned char            GetColorAlpha                       ( void ) const                      { return static_cast < unsigned char > ( m_rgbaColor >> 24 ); };
    void                            SetColor                            ( unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha );
    inline void                     SetColor                            ( unsigned long ulColor )           { m_rgbaColor = ulColor; };

    inline float                    GetSize                             ( void ) const                      { return m_fSize; };
    inline void                     SetSize                             ( float fSize )                     { m_fSize = fSize; };

	inline float					GetPulseFraction					( void )							{ return static_cast < float > ( m_pMarker->GetPulseFraction() ); };
	void							SetPulseFraction					( float fFraction )					{ m_pMarker->SetPulseFraction ( fFraction ); };

protected:
    void                            StreamIn                            ( void );
    void                            StreamOut                           ( void );

    void                            DoPulse                             ( void );

private:
    CClientMarker *                 m_pThis;
    CVector                         m_vecPosition;
	CVector							m_vecUp;

    bool                            m_bVisible;
    DWORD                           m_dwType;
    float                           m_fSize;
    RGBA                            m_rgbaColor;
    C3DMarker*                      m_pMarker;
    unsigned int                    m_ulIdentifier;
    bool                            m_bMarkerStreamedIn;
};

#endif
