/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CUnoccupiedVehicleSync.cpp
*  PURPOSE:     Unoccupied vehicle sync manager
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Kevin Whiteside <kevuwk@gmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "net/SyncStructures.h"

using std::list;

extern CClientGame* g_pClientGame;

#define UNOCCUPIED_VEHICLE_SYNC_RATE 1000

CUnoccupiedVehicleSync::CUnoccupiedVehicleSync ( CClientVehicleManager* pVehicleManager )
{
    m_pVehicleManager = pVehicleManager;
    m_ulLastSyncTime = 0;
}


CUnoccupiedVehicleSync::~CUnoccupiedVehicleSync ( void )
{

}


bool CUnoccupiedVehicleSync::ProcessPacket ( unsigned char ucPacketID, NetBitStreamInterface& BitStream )
{
    switch ( ucPacketID )
    {
        case PACKET_ID_UNOCCUPIED_VEHICLE_STARTSYNC:
        {
            Packet_UnoccupiedVehicleStartSync ( BitStream );
            return true;
        }

        case PACKET_ID_UNOCCUPIED_VEHICLE_STOPSYNC:
        {
            Packet_UnoccupiedVehicleStopSync ( BitStream );
            return true;
        }

        case PACKET_ID_UNOCCUPIED_VEHICLE_SYNC:
        {
            Packet_UnoccupiedVehicleSync ( BitStream );
            return true;
        }
    }

    return false;
}


void CUnoccupiedVehicleSync::DoPulse ( void )
{
    // Check all our vehicles for damage
    UpdateDamageModels ();

    // Has it been long enough since our last state's sync?
    unsigned long ulCurrentTime = CClientTime::GetTime ();
    if ( ulCurrentTime >= m_ulLastSyncTime + UNOCCUPIED_VEHICLE_SYNC_RATE )
    {
        UpdateStates ();
        m_ulLastSyncTime = ulCurrentTime;
    }
}


void CUnoccupiedVehicleSync::AddVehicle ( CDeathmatchVehicle* pVehicle )
{
    m_List.push_front ( pVehicle );
    pVehicle->SetIsSyncing ( true );
}


void CUnoccupiedVehicleSync::RemoveVehicle ( CDeathmatchVehicle* pVehicle )
{
    if ( !m_List.empty() ) m_List.remove ( pVehicle );
    pVehicle->SetIsSyncing ( false );
}


void CUnoccupiedVehicleSync::ClearVehicles ( void )
{
    // Mark all vehicles as 'not syncing'
    list < CDeathmatchVehicle* > ::iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        (*iter)->SetIsSyncing ( false );
    }

    // Clear the list
    m_List.clear ();
}


bool CUnoccupiedVehicleSync::Exists ( CDeathmatchVehicle * pVehicle )
{
    list < CDeathmatchVehicle* > ::iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        if ( *iter == pVehicle )
            return true;
    }
    return false;
}


void CUnoccupiedVehicleSync::Packet_UnoccupiedVehicleStartSync ( NetBitStreamInterface& BitStream )
{
    // Read out the vehicle id
    ElementID ID;
    if ( BitStream.Read ( ID ) )
    {
        // Grab the vehicle
        CDeathmatchVehicle* pVehicle = static_cast < CDeathmatchVehicle* > ( m_pVehicleManager->Get ( ID ) );
        if ( pVehicle )
        {
            // Read out and set its current state, so we start off sync'd with the server
            
            // Read out the position
            CVector vecPosition;
            BitStream.Read ( vecPosition.fX );
            BitStream.Read ( vecPosition.fY );
            BitStream.Read ( vecPosition.fZ );

            // And rotation
            CVector vecRotationDegrees;
            BitStream.Read ( vecRotationDegrees.fX );
            BitStream.Read ( vecRotationDegrees.fY );
            BitStream.Read ( vecRotationDegrees.fZ );

            // And the move and turn speed
            CVector vecMoveSpeed;
            BitStream.Read ( vecMoveSpeed.fX );
            BitStream.Read ( vecMoveSpeed.fY );
            BitStream.Read ( vecMoveSpeed.fZ );

            CVector vecTurnSpeed;
            BitStream.Read ( vecTurnSpeed.fX );
            BitStream.Read ( vecTurnSpeed.fY );
            BitStream.Read ( vecTurnSpeed.fZ );

            // And health
            float fHealth;
            BitStream.Read ( fHealth );

            // Set data for interpolation
            pVehicle->SetTargetPosition ( vecPosition, TICK_RATE, false );
            pVehicle->SetTargetRotation ( vecRotationDegrees, TICK_RATE, false );
            pVehicle->SetMoveSpeed ( vecMoveSpeed );
            pVehicle->SetTurnSpeed ( vecTurnSpeed );

            // Set the new health
            pVehicle->SetHealth ( fHealth );

            // Start syncing it
            AddVehicle ( pVehicle );

#ifdef MTA_DEBUG
            pVehicle->m_pLastSyncer = g_pClientGame->GetLocalPlayer ();
            pVehicle->m_ulLastSyncTime = GetTickCount ();
            pVehicle->m_szLastSyncType = "unoccupied-start";
#endif
        }
    }
}


void CUnoccupiedVehicleSync::Packet_UnoccupiedVehicleStopSync ( NetBitStreamInterface& BitStream )
{
    // Read out the vehicle id
    ElementID ID;
    if ( BitStream.Read ( ID ) )
    {
        // Grab the vehicle
        CDeathmatchVehicle* pVehicle = static_cast < CDeathmatchVehicle* > ( m_pVehicleManager->Get ( ID ) );
        if ( pVehicle )
        {
            // Stop syncing it
            RemoveVehicle ( pVehicle );
        }
    }
}


void CUnoccupiedVehicleSync::Packet_UnoccupiedVehicleSync ( NetBitStreamInterface& BitStream )
{
    // While we're not out of vehicles
    while ( BitStream.GetNumberOfUnreadBits () >= 8 )
    {
        SUnoccupiedVehicleSync vehicle;
        if ( BitStream.Read ( &vehicle ) )
        {
            CClientVehicle* pVehicle = m_pVehicleManager->Get ( vehicle.data.vehicleID );
            if ( pVehicle && pVehicle->CanUpdateSync ( vehicle.data.ucTimeContext ) )
            {
                if ( vehicle.data.bSyncPosition )       pVehicle->SetTargetPosition ( vehicle.data.vecPosition, TICK_RATE, false );
                if ( vehicle.data.bSyncRotation )       pVehicle->SetTargetRotation ( vehicle.data.vecRotation, TICK_RATE, false );
                if ( vehicle.data.bSyncVelocity )       pVehicle->SetMoveSpeed ( vehicle.data.vecVelocity );
                if ( vehicle.data.bSyncTurnVelocity )   pVehicle->SetTurnSpeed ( vehicle.data.vecTurnVelocity );
                if ( vehicle.data.bSyncHealth )         pVehicle->SetHealth ( vehicle.data.fHealth );
                                                        pVehicle->SetEngineOn ( vehicle.data.bEngineOn );
                if ( pVehicle->GetVehicleType() == CLIENTVEHICLE_TRAIN )
                                                        pVehicle->SetDerailed ( vehicle.data.bDerailed );
#ifdef MTA_DEBUG
				pVehicle->m_pLastSyncer = NULL;
				pVehicle->m_ulLastSyncTime = GetTickCount ();
				pVehicle->m_szLastSyncType = "unoccupied";
#endif
            }
        }
        else
            break;
    }
}


void CUnoccupiedVehicleSync::UpdateDamageModels ( void )
{
    // Got any items?
    if ( m_List.size () > 0 )
    {
        list < CDeathmatchVehicle* > ::iterator iter = m_List.begin ();
        for ( ; iter != m_List.end (); iter++ )
        {
            // Sync its damage model changes if neccessary
            (*iter)->SyncDamageModel ();
        }           
    }
}


void CUnoccupiedVehicleSync::UpdateStates ( void )
{
    // Got any items?
    if ( m_List.size () > 0 )
    {
        // Create a packet
        NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
        if ( pBitStream )
        {
            // Write each vehicle to it
            bool bAnyVehicleAdded = false;
            list < CDeathmatchVehicle* > ::iterator iter = m_List.begin ();
            for ( ; iter != m_List.end (); iter++ )
            {
                if ( WriteVehicleInformation ( pBitStream, *iter ) )
                    bAnyVehicleAdded = true;
            }

            // Send and destroy the packet
            if ( bAnyVehicleAdded )
                g_pNet->SendPacket ( PACKET_ID_UNOCCUPIED_VEHICLE_SYNC, pBitStream, PACKET_PRIORITY_LOW, PACKET_RELIABILITY_UNRELIABLE_SEQUENCED );
            g_pNet->DeallocateNetBitStream ( pBitStream );
        }
    }
}


bool CUnoccupiedVehicleSync::WriteVehicleInformation ( NetBitStreamInterface* pBitStream, CDeathmatchVehicle* pVehicle )
{
    bool bSyncVehicle = false;
    SUnoccupiedVehicleSync vehicle;

    // Copy the vehicle data into the sync structures
    vehicle.data.vehicleID = pVehicle->GetID ();
    vehicle.data.ucTimeContext = pVehicle->GetSyncTimeContext ();

    pVehicle->GetPosition ( vehicle.data.vecPosition );
    pVehicle->GetRotationDegrees ( vehicle.data.vecRotation );
    if ( pVehicle->GetGameVehicle () )
    {
        pVehicle->GetMoveSpeed ( vehicle.data.vecVelocity );
        pVehicle->GetTurnSpeed ( vehicle.data.vecTurnVelocity );
    }

    vehicle.data.fHealth = pVehicle->GetHealth ();

    CClientVehicle* pTrailer = pVehicle->GetRealTowedVehicle ();
    if ( pTrailer )
        vehicle.data.trailer = pTrailer->GetID ();
    else
        vehicle.data.trailer = static_cast < ElementID > ( INVALID_ELEMENT_ID );


    // Adjust the sync flags
    if ( pVehicle->GetModelInfo ()->IsBoat () )
    {
        // Boats vary their z height a bit when on water due to the wave effect
        // It's not needed to sync these tiny changes.
        const CVector& vecLastPosition = pVehicle->m_LastSyncedData->vecPosition;
        if ( fabs ( vecLastPosition.fX - vehicle.data.vecPosition.fX ) > FLOAT_EPSILON ||
             fabs ( vecLastPosition.fY - vehicle.data.vecPosition.fY ) > FLOAT_EPSILON ||
             fabs ( vecLastPosition.fZ - vehicle.data.vecPosition.fZ ) > 1.2f )
        {
            bSyncVehicle = true;
            vehicle.data.bSyncPosition = true;
            pVehicle->m_LastSyncedData->vecPosition = vehicle.data.vecPosition;
        }

        const CVector& vecLastVelocity = pVehicle->m_LastSyncedData->vecMoveSpeed;
        if ( vecLastVelocity != vehicle.data.vecVelocity &&
             (
               fabs ( vehicle.data.vecVelocity.fX ) > FLOAT_EPSILON ||
               fabs ( vehicle.data.vecVelocity.fY ) > FLOAT_EPSILON ||
               fabs ( vehicle.data.vecVelocity.fZ ) > FLOAT_EPSILON
             )
           )
        {
            bSyncVehicle = true;
            vehicle.data.bSyncVelocity = true;
            pVehicle->m_LastSyncedData->vecMoveSpeed = vehicle.data.vecVelocity;
        }
    }
    else
    {
        if ( pVehicle->m_LastSyncedData->vecPosition != vehicle.data.vecPosition )
        {
            bSyncVehicle = true;
            vehicle.data.bSyncPosition = true;
            pVehicle->m_LastSyncedData->vecPosition = vehicle.data.vecPosition;
        }

        if ( pVehicle->m_LastSyncedData->vecMoveSpeed != vehicle.data.vecVelocity )
        {
            bSyncVehicle = true;
            vehicle.data.bSyncVelocity = true;
            pVehicle->m_LastSyncedData->vecMoveSpeed = vehicle.data.vecVelocity;
        }
    }

    if ( pVehicle->m_LastSyncedData->vecRotation != vehicle.data.vecRotation )
    {
        bSyncVehicle = true;
        vehicle.data.bSyncRotation = true;
        pVehicle->m_LastSyncedData->vecRotation = vehicle.data.vecRotation;
    }

    if ( pVehicle->m_LastSyncedData->vecTurnSpeed != vehicle.data.vecTurnVelocity )
    {
        bSyncVehicle = true;
        vehicle.data.bSyncTurnVelocity = true;
        pVehicle->m_LastSyncedData->vecTurnSpeed = vehicle.data.vecTurnVelocity;
    }

    if ( fabs ( pVehicle->m_LastSyncedData->fHealth - vehicle.data.fHealth ) > FLOAT_EPSILON )
    {
        bSyncVehicle = true;
        vehicle.data.bSyncHealth = true;
        pVehicle->m_LastSyncedData->fHealth = vehicle.data.fHealth;
    }

    if ( pVehicle->m_LastSyncedData->Trailer != vehicle.data.trailer )
    {
        bSyncVehicle = true;
        vehicle.data.bSyncTrailer = true;
        pVehicle->m_LastSyncedData->Trailer = vehicle.data.trailer;
    }

    if ( pVehicle->IsEngineOn () )
    {
        bSyncVehicle = true;
        vehicle.data.bEngineOn = true;
    }

    if ( pVehicle->IsDerailed () )
    {
        bSyncVehicle = true;
        vehicle.data.bDerailed = true;
    }

    // If nothing has changed we dont sync the vehicle
    if ( !bSyncVehicle )
        return false;

    // Write the data
    pBitStream->Write ( &vehicle );

    return true;
}
