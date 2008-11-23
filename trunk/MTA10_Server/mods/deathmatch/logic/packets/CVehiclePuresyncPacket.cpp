/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CVehiclePuresyncPacket.cpp
*  PURPOSE:     Vehicle pure synchronization packet class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CGame* g_pGame;

bool CVehiclePuresyncPacket::Read ( NetServerBitStreamInterface& BitStream )
{
    // Got a player to read?
    if ( m_pSourceElement )
    {
        CPlayer * pSourcePlayer = static_cast < CPlayer * > ( m_pSourceElement );

        // Player is in a vehicle?
        CVehicle* pVehicle = pSourcePlayer->GetOccupiedVehicle ();
        if ( pVehicle )
        {
            // Read out the time context
            unsigned char ucTimeContext = 0;
            BitStream.Read ( ucTimeContext );

            // Only read this packet if it matches the current time context that
            // player is in.
            if ( !pSourcePlayer->CanUpdateSync ( ucTimeContext ) )
            {
                return false;
            }

            // Read out the keysync data
            CControllerState ControllerState;
            ReadFullKeysync ( ControllerState, BitStream );
            pSourcePlayer->GetPad ()->NewControllerState ( ControllerState );

            // Read out its position
            CVector vecPosition;
            BitStream.Read ( vecPosition.fX );
            BitStream.Read ( vecPosition.fY );
            BitStream.Read ( vecPosition.fZ );
            pSourcePlayer->SetPosition ( vecPosition );

            // Jax: don't allow any outdated packets through
            unsigned char ucSeat;
            BitStream.Read ( ucSeat );
            if ( ucSeat != pSourcePlayer->GetOccupiedVehicleSeat () )
            {
                // Mis-matching seats can happen when we warp into a different one,
                // which will screw up the whole packet
                return false;
            }

            // Read out the vehicle matrix only if he's the driver
            CVector vecTemp;
            unsigned int uiSeat = pSourcePlayer->GetOccupiedVehicleSeat ();
            if ( uiSeat == 0 )
            {
                // Read out the vehicle rotation in degrees
                CVector vecRotationDegrees;
                BitStream.Read ( vecRotationDegrees.fX );
                BitStream.Read ( vecRotationDegrees.fY );
                BitStream.Read ( vecRotationDegrees.fZ );

                // Set it
                pVehicle->SetPosition ( vecPosition );
                pVehicle->SetRotationDegrees ( vecRotationDegrees );

                // Move speed vector
                BitStream.Read ( vecTemp.fX );
                BitStream.Read ( vecTemp.fY );
                BitStream.Read ( vecTemp.fZ );

                pVehicle->SetVelocity ( vecTemp );
                pSourcePlayer->SetVelocity ( vecTemp );

                // Turn speed vector
                BitStream.Read ( vecTemp.fX );
                BitStream.Read ( vecTemp.fY );
                BitStream.Read ( vecTemp.fZ );

                pVehicle->SetTurnSpeed ( vecTemp );

                // Health
                float fHealth = 1000.0f;
	            BitStream.Read ( fHealth );
                float fPreviousHealth = pVehicle->GetHealth ();                

                // Less than last time?
                if ( fHealth < fPreviousHealth )
                {                 
                    // Grab the delta health
                    float fDeltaHealth = fPreviousHealth - fHealth;

					if ( fDeltaHealth > 0.0f )
					{
						// Call the onVehicleDamage event
						CLuaArguments Arguments;
						Arguments.PushNumber ( fDeltaHealth );
						pVehicle->CallEvent ( "onVehicleDamage", Arguments );
					}
                }
                pVehicle->SetHealth ( fHealth );

                // Trailer chain
                CVehicle* pTowedByVehicle = pVehicle;
                CVehicle* pTrailer = NULL;
                ElementID TrailerID;
                BitStream.Read ( TrailerID );

                while ( TrailerID != INVALID_ELEMENT_ID )
                {
                    CElement* pElement = CElementIDs::GetElement ( TrailerID );
                    if ( pElement )
                        pTrailer = static_cast < CVehicle* > ( pElement );
                    
                    // Read out the trailer position and rotation
                    CVector vecTrailerPosition, vecTrailerRotationDegrees;
                    BitStream.Read ( vecTrailerPosition.fX );
                    BitStream.Read ( vecTrailerPosition.fY );
                    BitStream.Read ( vecTrailerPosition.fZ );

                    BitStream.Read ( vecTrailerRotationDegrees.fX );
                    BitStream.Read ( vecTrailerRotationDegrees.fY );
                    BitStream.Read ( vecTrailerRotationDegrees.fZ );

                    // If we found the trailer
                    if ( pTrailer )
                    {
                        // Set its position and rotation
                        pTrailer->SetPosition ( vecTrailerPosition );
                        pTrailer->SetRotationDegrees ( vecTrailerRotationDegrees );
    
                        // Is this a new trailer, attached?
                        CVehicle* pCurrentTrailer = pTowedByVehicle->GetTowedVehicle ();
                        if ( pCurrentTrailer != pTrailer )
                        {
                            // If theres a trailer already attached
                            if ( pCurrentTrailer )
                            {
                                pTowedByVehicle->SetTowedVehicle ( NULL, true );

                                // Tell everyone to detach them
                                CVehicleTrailerPacket AttachPacket ( pTowedByVehicle, pCurrentTrailer, false );
                                g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( AttachPacket );

                                // Execute the attach trailer script function
                                CLuaArguments Arguments;
                                Arguments.PushElement ( pTowedByVehicle );
                                pCurrentTrailer->CallEvent ( "onTrailerDetach", Arguments );
                            }

                            // If something else is towing this trailer
                            CVehicle* pCurrentVehicle = pTrailer->GetTowedByVehicle ();
                            if ( pCurrentVehicle )
                            {
                                pCurrentVehicle->SetTowedVehicle ( NULL, true );

                                // Tell everyone to detach them
                                CVehicleTrailerPacket AttachPacket ( pCurrentVehicle, pTrailer, false );
                                g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( AttachPacket );

                                // Execute the attach trailer script function
                                CLuaArguments Arguments;
                                Arguments.PushElement ( pCurrentVehicle );
                                pTrailer->CallEvent ( "onTrailerDetach", Arguments );
                            }

                            pTowedByVehicle->SetTowedVehicle ( pTrailer, true );

                            // Tell everyone to attach the new one
                            CVehicleTrailerPacket AttachPacket ( pTowedByVehicle, pTrailer, true );
                            g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( AttachPacket );

                            // Execute the attach trailer script function
                            CLuaArguments Arguments;
                            Arguments.PushElement ( pTowedByVehicle );
                            bool bContinue = pTrailer->CallEvent ( "onTrailerAttach", Arguments );

                            if ( !bContinue )
                            {
                                // Detach them
                                CVehicleTrailerPacket DetachPacket ( pTowedByVehicle, pTrailer, false );
                                g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( DetachPacket );
                            }
                        }
                    }
                    else
                        break;

                    pTowedByVehicle = pTrailer;

                    if ( !BitStream.Read ( TrailerID ) )
                        break;
                }

                // If there was a trailer before
                CVehicle* pCurrentTrailer = pTowedByVehicle->GetTowedVehicle ();
                if ( pCurrentTrailer )
                {
                    pTowedByVehicle->SetTowedVehicle ( NULL, true );

                    // Tell everyone else to detach them
                    CVehicleTrailerPacket AttachPacket ( pTowedByVehicle, pCurrentTrailer, false );
                    g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( AttachPacket );

                    // Execute the detach trailer script function
                    CLuaArguments Arguments;
                    Arguments.PushElement ( pTowedByVehicle );
                    pCurrentTrailer->CallEvent ( "onTrailerDetach", Arguments );                    
                }
            }

            // Player health
            unsigned char ucTemp;
	        BitStream.Read ( ucTemp );

            float fHealth = static_cast < float > ( ucTemp ) / 1.25f;
            float fOldHealth = pSourcePlayer->GetHealth ();
			float fHealthLoss = fOldHealth - fHealth;

            // Less than last packet's frame?
            if ( fHealth < fOldHealth && fHealthLoss > 0 )
            {
                // Call the onPlayerDamage event
                CLuaArguments Arguments;
                Arguments.PushNumber ( fHealthLoss );
                pSourcePlayer->CallEvent ( "onPlayerDamage", Arguments );
            }
            pSourcePlayer->SetHealth ( fHealth );

			// Armor
			BitStream.Read ( ucTemp );

			float fArmor = static_cast < float > ( ucTemp ) / 1.25f;
			float fOldArmor = pSourcePlayer->GetArmor ();
			float fArmorLoss = fOldArmor - fArmor;

			// Less than last packet's frame?
			if ( fArmor < fOldArmor && fArmorLoss > 0 )
			{
				// Call the onPlayerDamage event
				CLuaArguments Arguments;
				CElement* pKillerElement = pSourcePlayer->GetPlayerAttacker ();
				if ( pKillerElement )
					Arguments.PushElement ( pKillerElement );
				else
					Arguments.PushNil ();
				Arguments.PushNumber ( pSourcePlayer->GetAttackWeapon () );
				Arguments.PushNumber ( pSourcePlayer->GetAttackBodyPart () );
				Arguments.PushNumber ( fArmorLoss );

				pSourcePlayer->CallEvent ( "onPlayerDamage", Arguments );					
			}
            pSourcePlayer->SetArmor ( fArmor );

            // Flags
            unsigned char ucFlags;
            BitStream.Read ( ucFlags );
            bool bWearingGoggles = ( ucFlags & 0x01 ) ? true:false;
            bool bDoingGangDriveby = ( ucFlags & 0x02 ) ? true:false;
            bool bSirenActive = ( ucFlags & 0x04 ) ? true:false;
            bool bSmokeTrail = ( ucFlags & 0x08 ) ? true:false;
            bool bLandingGearDown = ( ucFlags & 0x10 ) ? true:false;
            bool bOnGround = ( ucFlags & 0x20 ) ? true:false;
            bool bInWater = ( ucFlags & 0x40 ) ? true:false;

            pSourcePlayer->SetWearingGoggles ( bWearingGoggles );
            pSourcePlayer->SetDoingGangDriveby ( bDoingGangDriveby );            

            // Current weapon slot
            unsigned char ucCurrentWeaponSlot;
            BitStream.Read ( ucCurrentWeaponSlot );
            pSourcePlayer->SetWeaponSlot ( ucCurrentWeaponSlot );
            unsigned char ucCurrentWeapon = pSourcePlayer->GetWeaponType ();
            if ( ucCurrentWeapon != 0 )
            {
                // Read out the weapon state
                BitStream.Read ( ucTemp );
                pSourcePlayer->SetCurrentWeaponState ( ucTemp );

                // Read out the ammo state
                unsigned short usAmmoInClip;
                BitStream.Read ( usAmmoInClip );
                pSourcePlayer->SetWeaponAmmoInClip ( usAmmoInClip );

                // Read out the aim directions
                float fArmX, fArmY;
                BitStream.Read ( fArmX );
                BitStream.Read ( fArmY );
                pSourcePlayer->SetAimDirections ( fArmX, fArmY );

                // Source vector
                BitStream.Read ( vecTemp.fX );
                BitStream.Read ( vecTemp.fY );
                BitStream.Read ( vecTemp.fZ );
                pSourcePlayer->SetSniperSourceVector ( vecTemp );

                // Target vector
                BitStream.Read ( vecTemp.fX );
                BitStream.Read ( vecTemp.fY );
                BitStream.Read ( vecTemp.fZ );
                pSourcePlayer->SetTargettingVector ( vecTemp );

                // Read out the driveby direction
                unsigned char ucDriveByDirection;
                BitStream.Read ( ucDriveByDirection );
                pSourcePlayer->SetDriveByDirection ( ucDriveByDirection );
            }

            // Vehicle specific data if he's the driver
            if ( uiSeat == 0 )
            {
                ReadVehicleSpecific ( pVehicle, BitStream );

                // Set vehicle specific stuff if he's the driver
                pVehicle->SetSirenActive ( bSirenActive );
                pVehicle->SetSmokeTrailEnabled ( bSmokeTrail );
                pVehicle->SetLandingGearDown ( bLandingGearDown );
                pVehicle->SetOnGround ( bOnGround );
                pVehicle->SetInWater ( bInWater );
            }

            // Success
            return true;
        }
    }

    return false;
}


bool CVehiclePuresyncPacket::Write ( NetServerBitStreamInterface& BitStream ) const
{
    // Got a player to send?
    if ( m_pSourceElement )
    {
        CPlayer * pSourcePlayer = static_cast < CPlayer * > ( m_pSourceElement );

        // Player is in a vehicle and is the driver?
        CVehicle* pVehicle = pSourcePlayer->GetOccupiedVehicle ();
        if ( pVehicle )
        {
            // Player ID
            ElementID PlayerID = pSourcePlayer->GetID ();
            BitStream.Write ( PlayerID );

            // Write the time context of that player
            BitStream.Write ( pSourcePlayer->GetSyncTimeContext () );

            // Write his ping divided with 2 plus a small number so the client can find out when this packet was sent
            unsigned short usLatency = pSourcePlayer->GetPing ();
            BitStream.Write ( usLatency );

            // Write the keysync data
            CControllerState ControllerState = pSourcePlayer->GetPad ()->GetCurrentControllerState ();
            WriteFullKeysync ( ControllerState, BitStream );

            // Write the vehicle matrix only if he's the driver
            CVector vecTemp;
            unsigned int uiSeat = pSourcePlayer->GetOccupiedVehicleSeat ();
            if ( uiSeat == 0 )
            {
                // Vehicle position
                const CVector& vecPosition = pVehicle->GetPosition ();
                BitStream.Write ( vecPosition.fX );
                BitStream.Write ( vecPosition.fY );
                BitStream.Write ( vecPosition.fZ );

                // Vehicle rotation
                CVector vecRotationDegrees;
                pVehicle->GetRotationDegrees ( vecRotationDegrees );
                BitStream.Write ( vecRotationDegrees.fX );
                BitStream.Write ( vecRotationDegrees.fY );
                BitStream.Write ( vecRotationDegrees.fZ );

                // Move speed vector
                vecTemp = pVehicle->GetVelocity ();
                BitStream.Write ( vecTemp.fX );
                BitStream.Write ( vecTemp.fY );
                BitStream.Write ( vecTemp.fZ );

                // Turn speed vector
                vecTemp = pVehicle->GetTurnSpeed ();
                BitStream.Write ( vecTemp.fX );
                BitStream.Write ( vecTemp.fY );
                BitStream.Write ( vecTemp.fZ );

                // Health
                BitStream.Write ( pVehicle->GetHealth () );                
            }

            // Player health and armor
            unsigned char ucHealth = static_cast < unsigned char > ( pSourcePlayer->GetHealth () * 1.25f );
            unsigned char ucArmor = static_cast < unsigned char > ( pSourcePlayer->GetArmor () * 1.25f );
	        BitStream.Write ( ucHealth );
            BitStream.Write ( ucArmor );

            // Flags
            unsigned char ucFlags = 0;
            if ( pSourcePlayer->IsWearingGoggles () ) ucFlags |= 0x01;
            if ( pSourcePlayer->IsDoingGangDriveby () ) ucFlags |= 0x02;
            if ( pVehicle->IsSirenActive () ) ucFlags |= 0x04;
            if ( pVehicle->IsSmokeTrailEnabled () ) ucFlags |= 0x08;
            if ( pVehicle->IsLandingGearDown () ) ucFlags |= 0x10;
            if ( pVehicle->IsOnGround () ) ucFlags |= 0x20;
            if ( pVehicle->IsInWater () ) ucFlags |= 0x40;

            // Write the flags
            BitStream.Write ( ucFlags );

            // Current weapon id
            unsigned char ucWeaponType = pSourcePlayer->GetWeaponType ();
            BitStream.Write ( ucWeaponType );
            if ( ucWeaponType != 0 )
            {
                // Write the weapon state and ammo in clip
                BitStream.Write ( pSourcePlayer->GetCurrentWeaponState () );
                BitStream.Write ( pSourcePlayer->GetWeaponAmmoInClip () );

                // Write the aim directions
                BitStream.Write ( pSourcePlayer->GetAimDirectionX () );
                BitStream.Write ( pSourcePlayer->GetAimDirectionY () );

                 // Source vector
                vecTemp = pSourcePlayer->GetSniperSourceVector ();
                BitStream.Write ( vecTemp.fX );
                BitStream.Write ( vecTemp.fY );
                BitStream.Write ( vecTemp.fZ );

                // Targetting vector
                pSourcePlayer->GetTargettingVector ( vecTemp );
                BitStream.Write ( vecTemp.fX );
                BitStream.Write ( vecTemp.fY );
                BitStream.Write ( vecTemp.fZ );

                // Write the driveby aim directoin
                BitStream.Write ( pSourcePlayer->GetDriveByDirection () );
            }

            // Vehicle specific data only if he's the driver
            if ( uiSeat == 0 )
            {
                WriteVehicleSpecific ( pVehicle, BitStream );
            }

            // Success
            return true;
        }
    }

    return false;
}


void CVehiclePuresyncPacket::ReadVehicleSpecific ( CVehicle* pVehicle, NetServerBitStreamInterface& BitStream )
{
    // Turret data
    unsigned short usModel = pVehicle->GetModel ();
    if ( CVehicleManager::HasTurret ( usModel ) ) 
    {
        // Read out the turret position
        float fTurretX;
        float fTurretY;
        BitStream.Read ( fTurretX );
        BitStream.Read ( fTurretY );

        // Set the data
        pVehicle->SetTurretPosition ( fTurretX, fTurretY );
    }

    // Adjustable property value
    if ( CVehicleManager::HasAdjustableProperty ( usModel ) )
    {
        unsigned short usAdjustableProperty;
        if ( BitStream.Read ( usAdjustableProperty ) )
        {
            pVehicle->SetAdjustableProperty ( usAdjustableProperty );
        }
    }
}


void CVehiclePuresyncPacket::WriteVehicleSpecific ( CVehicle* pVehicle, NetServerBitStreamInterface& BitStream ) const
{
    // Turret states
    unsigned short usModel = pVehicle->GetModel ();
    if ( CVehicleManager::HasTurret ( usModel ) )
    {
        // Grab the turret position
        float fHorizontal;
        float fVertical;
        pVehicle->GetTurretPosition ( fHorizontal, fVertical );

        // Write it
        BitStream.Write ( fHorizontal );
        BitStream.Write ( fVertical );
    }

    // Adjustable property value
    if ( CVehicleManager::HasAdjustableProperty ( usModel ) )
    {
        BitStream.Write ( pVehicle->GetAdjustableProperty () );
    }
}
