/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CUnoccupiedVehicleSyncPacket.cpp
*  PURPOSE:     Unoccupied vehicle synchronization packet class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CUnoccupiedVehicleSyncPacket::~CUnoccupiedVehicleSyncPacket ( void )
{
    vector < SyncData* > ::const_iterator iter = m_Syncs.begin ();
    for ( ; iter != m_Syncs.end (); iter++ )
    {
        delete *iter;
    }
    m_Syncs.clear ();
}


bool CUnoccupiedVehicleSyncPacket::Read ( NetServerBitStreamInterface& BitStream )
{
    // While we're not out of bytes
    while ( BitStream.GetNumberOfUnreadBits () > 32 )
    {
        // Read out the sync data
        SyncData* pData = new SyncData;
        pData->bSend = false;

        BitStream.Read ( pData->Model );

        // Read the sync time context
        BitStream.Read ( pData->ucSyncTimeContext );

        BitStream.Read ( pData->usFlags );

        if ( pData->usFlags & 0x01 )
        {
            BitStream.Read ( pData->vecPosition.fX );
            BitStream.Read ( pData->vecPosition.fY );
            BitStream.Read ( pData->vecPosition.fZ );
        }

        if ( pData->usFlags & 0x02 )
        {
            BitStream.Read ( pData->vecRotationDegrees.fX );
            BitStream.Read ( pData->vecRotationDegrees.fY );
            BitStream.Read ( pData->vecRotationDegrees.fZ );
        }

        if ( pData->usFlags & 0x04 )
        {
            BitStream.Read ( pData->vecVelocity.fX );
            BitStream.Read ( pData->vecVelocity.fY );
            BitStream.Read ( pData->vecVelocity.fZ );
        }

        if ( pData->usFlags & 0x08 )
        {
            BitStream.Read ( pData->vecTurnSpeed.fX );
            BitStream.Read ( pData->vecTurnSpeed.fY );
            BitStream.Read ( pData->vecTurnSpeed.fZ );
        }

        if ( pData->usFlags & 0x10 ) BitStream.Read ( pData->fHealth );

        if ( pData->usFlags & 0x20 ) BitStream.Read ( pData->TrailerID );  

        // Add it to our list. We no longer check if it's valid here
        // because CUnoccupiedVehicleSync does and it won't write bad ID's
        // back to clients.
        m_Syncs.push_back ( pData );
    }

    return m_Syncs.size () > 0;
}


bool CUnoccupiedVehicleSyncPacket::Write ( NetServerBitStreamInterface& BitStream ) const
{
    // While we're not out of syncs to write
    bool bSent = false;
    vector < SyncData* > ::const_iterator iter = m_Syncs.begin ();
    for ( ; iter != m_Syncs.end (); iter++ )
    {
        // If we're not supposed to ignore the packet
        SyncData* pData = *iter;
        if ( pData->bSend )
        {
            // Write vehicle ID
            BitStream.Write ( pData->Model );

            // Write the sync time context
            BitStream.Write ( pData->ucSyncTimeContext );

            // Write packet flags
            BitStream.Write ( pData->usFlags );

            // Position and rotation
            if ( pData->usFlags & 0x01 )
            {
                BitStream.Write ( pData->vecPosition.fX );
                BitStream.Write ( pData->vecPosition.fY );
                BitStream.Write ( pData->vecPosition.fZ );
            }

            if ( pData->usFlags & 0x02 )
            {
                BitStream.Write ( pData->vecRotationDegrees.fX );
                BitStream.Write ( pData->vecRotationDegrees.fY );
                BitStream.Write ( pData->vecRotationDegrees.fZ );
            }

            // Velocity
            if ( pData->usFlags & 0x04 )
            {
                BitStream.Write ( pData->vecVelocity.fX );
                BitStream.Write ( pData->vecVelocity.fY );
                BitStream.Write ( pData->vecVelocity.fZ );
            }

            // Turnspeed
            if ( pData->usFlags & 0x08 )
            {
                BitStream.Write ( pData->vecTurnSpeed.fX );
                BitStream.Write ( pData->vecTurnSpeed.fY );
                BitStream.Write ( pData->vecTurnSpeed.fZ );
            }

            // Health
            if ( pData->usFlags & 0x10 ) BitStream.Write ( pData->fHealth );

            // We've sent atleast one sync
            bSent = true;
        }
    }

    return bSent;
}
