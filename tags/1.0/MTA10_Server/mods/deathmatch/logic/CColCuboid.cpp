/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CColCuboid.cpp
*  PURPOSE:     Cube-shaped collision entity class
*  DEVELOPERS:  Kevin Whiteside <>
*               Ed Lyons <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CColCuboid::CColCuboid ( CColManager* pManager, CElement* pParent, const CVector& vecPosition, const CVector& vecSize, CXMLNode* pNode ) : CColShape ( pManager, pParent, pNode )
{
    m_vecPosition = vecPosition;
    m_vecSize = vecSize;

   SetTypeName ( "colcuboid" );
}


bool CColCuboid::DoHitDetection  ( const CVector& vecLastPosition, const CVector& vecNowPosition, float fRadius )
{
    // FIXME: What about radius?

    // See if the now position is within our cube
    return ( vecNowPosition.fX >= m_vecPosition.fX &&
             vecNowPosition.fX <= m_vecPosition.fX + m_vecSize.fX &&
             vecNowPosition.fY >= m_vecPosition.fY &&
             vecNowPosition.fY <= m_vecPosition.fY + m_vecSize.fY &&
             vecNowPosition.fZ >= m_vecPosition.fZ &&
             vecNowPosition.fZ <= m_vecPosition.fZ + m_vecSize.fZ );
}


bool CColCuboid::ReadSpecialData ( void )
{
	int iTemp;
	if ( GetCustomDataInt ( "dimension", iTemp, true ) )
        m_usDimension = static_cast < unsigned short > ( iTemp );

    float fWidth = 1.0f, fDepth = 1.0f, fHeight = 1.0f;
    GetCustomDataFloat ( "width", fWidth, true );
    GetCustomDataFloat ( "depth", fDepth, true );
    GetCustomDataFloat ( "height", fHeight, true );

    m_vecSize.fX = fWidth;
    m_vecSize.fY = fDepth;
    m_vecSize.fZ = fHeight;
    return true;
}
