/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CPedWastedPacket.cpp
*  PURPOSE:     Ped wasted state packet class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               Cazomino05 <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CPedWastedPacket::CPedWastedPacket ( void )
{
    m_PedID = INVALID_ELEMENT_ID;
    m_Killer = INVALID_ELEMENT_ID;
    m_ucKillerWeapon = 0xFF;
    m_usAmmo = 0;
    m_bStealth = false;
}


CPedWastedPacket::CPedWastedPacket ( CPed * pPed, CElement * pKiller, unsigned char ucKillerWeapon, unsigned char ucBodyPart, bool bStealth, AssocGroupId animGroup, AnimationId animID )
{
    m_PedID = pPed->GetID ();
    m_Killer = ( pKiller ) ? pKiller->GetID () : INVALID_ELEMENT_ID;
    m_ucKillerWeapon = ucKillerWeapon;
    m_ucBodyPart = ucBodyPart;
    m_vecPosition = pPed->GetPosition ();
    m_usAmmo = 0;
    m_bStealth = bStealth;
    m_AnimGroup = animGroup;
    m_AnimID = animID;
    // Generate a new sync-time context for the dead ped
    m_ucTimeContext = pPed->GenerateSyncTimeContext ();
}


bool CPedWastedPacket::Read ( NetBitStreamInterface& BitStream )
{
    BitStream.Read ( m_AnimGroup );
    BitStream.Read ( m_AnimID );
    BitStream.Read ( m_Killer );
    BitStream.Read ( m_ucKillerWeapon );
    BitStream.Read ( m_ucBodyPart );
    BitStream.Read ( m_vecPosition.fX );
    BitStream.Read ( m_vecPosition.fY );
    BitStream.Read ( m_vecPosition.fZ );
    BitStream.Read ( m_usAmmo );
    BitStream.Read ( m_PedID );
    return true;
}


bool CPedWastedPacket::Write ( NetBitStreamInterface& BitStream ) const
{    
    BitStream.Write ( m_PedID );
    BitStream.Write ( m_Killer );
    BitStream.Write ( m_ucKillerWeapon );
    BitStream.Write ( m_ucBodyPart );
    BitStream.Write ( ( unsigned char ) ( ( m_bStealth ) ? 1 : 0 ) );    
    BitStream.Write ( m_ucTimeContext );
    BitStream.Write ( m_AnimGroup );
    BitStream.Write ( m_AnimID );

    return true;
}
