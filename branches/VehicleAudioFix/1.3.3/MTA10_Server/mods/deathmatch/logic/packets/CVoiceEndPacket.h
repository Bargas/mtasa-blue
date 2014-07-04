/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CVoiceDataPacket.h
*  PURPOSE:     Voice end packet class
*  DEVELOPERS:  Philip Farquharson <philip@philipfarquharson.co.uk>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __PACKETS_CVOICEENDPACKET_H
#define __PACKETS_CVOICEENDPACKET_H

#include "CPacket.h"

class CVoiceEndPacket : public CPacket
{
public:
    CVoiceEndPacket            ( class CPlayer* pPlayer = NULL );
    ~CVoiceEndPacket            ( );

    ePacketID               GetPacketID                 ( void ) const              { return PACKET_ID_VOICE_END; }
    unsigned long           GetFlags                    ( void ) const              { return PACKET_LOW_PRIORITY | PACKET_SEQUENCED; };
    virtual ePacketOrdering GetPacketOrdering           ( void ) const              { return PACKET_ORDERING_VOICE; }

    bool                    Read                        ( NetBitStreamInterface& BitStream );
    bool                    Write                       ( NetBitStreamInterface& BitStream ) const;

    ElementID               GetPlayer                   ( void );
    void                    SetPlayer                   ( ElementID PlayerID );

private:

    ElementID               m_PlayerID;
};

#endif
