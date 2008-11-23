/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CCameraSyncPacket.h
*  PURPOSE:     Camera synchronization packet class
*  DEVELOPERS:  Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __PACKETS_CCAMERASYNCPACKET_H
#define __PACKETS_CCAMERASYNCPACKET_H

#include "../CCommon.h"
#include "CPacket.h"
#include "../CElement.h"

class CCameraSyncPacket : public CPacket
{
public:

    inline ePacketID                        GetPacketID                 ( void ) const                  { return PACKET_ID_CAMERA_SYNC; };
    unsigned long                           GetFlags                    ( void ) const                  { return 0; };

    bool                                    Read                        ( NetServerBitStreamInterface& BitStream );

    bool                                    m_bFixed;
    CVector                                 m_vecPosition, m_vecLookAt;
    ElementID                               m_TargetID;
};

#endif
