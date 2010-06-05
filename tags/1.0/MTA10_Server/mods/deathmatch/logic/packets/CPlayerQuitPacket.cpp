/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CPlayerQuitPacket.cpp
*  PURPOSE:     Player quit packet class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CPlayerQuitPacket::CPlayerQuitPacket ( void )
{
    m_PlayerID = INVALID_ELEMENT_ID;
    m_ucQuitReason = 0;
}


bool CPlayerQuitPacket::Write ( NetBitStreamInterface& BitStream ) const
{
    if ( m_PlayerID == INVALID_ELEMENT_ID )
        return false;
    BitStream.WriteCompressed ( m_PlayerID );

    SQuitReasonSync quitReason;
    quitReason.data.uiQuitReason = m_ucQuitReason;
    BitStream.Write ( &quitReason );

    return true;
}

