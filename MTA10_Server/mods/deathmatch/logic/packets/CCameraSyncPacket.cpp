/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CCameraSyncPacket.cpp
*  PURPOSE:     Camera synchronization packet class
*  DEVELOPERS:  Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

bool CCameraSyncPacket::Read ( NetBitStreamInterface& BitStream )
{
    unsigned char ucTemp;
    if ( !BitStream.Read ( ucTemp ) )
        return false;

    m_bFixed = ( ucTemp == 1 );

    if ( m_bFixed )
    {
        return BitStream.Read ( m_vecPosition.fX ) &&
               BitStream.Read ( m_vecPosition.fY ) &&
               BitStream.Read ( m_vecPosition.fZ ) &&

               BitStream.Read ( m_vecLookAt.fX ) &&
               BitStream.Read ( m_vecLookAt.fY ) &&
               BitStream.Read ( m_vecLookAt.fZ );
    }
    else
    {
        return BitStream.Read ( m_TargetID );
    }
}
