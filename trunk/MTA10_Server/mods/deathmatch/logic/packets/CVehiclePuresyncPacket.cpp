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
#include "net/SyncStructures.h"

extern CGame* g_pGame;

bool CVehiclePuresyncPacket::Read ( NetBitStreamInterface& BitStream )
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
            if ( !BitStream.Read ( ucTimeContext ) )
                return false;

            // Only read this packet if it matches the current time context that
            // player is in.
            if ( !pSourcePlayer->CanUpdateSync ( ucTimeContext ) )
            {
                return false;
            }

            // Read out the keysync data
            CControllerState ControllerState;
            if ( !ReadFullKeysync ( ControllerState, BitStream ) )
                return false;
            pSourcePlayer->GetPad ()->NewControllerState ( ControllerState );

            // Read out its position
            SPositionSync position ( false );
            if ( !BitStream.Read ( &position ) )
                return false;
            pSourcePlayer->SetPosition ( position.data.vecPosition );

            // Jax: don't allow any outdated packets through
            unsigned char ucSeat;
            if ( !BitStream.Read ( ucSeat ) )
                return false;
            if ( ucSeat != pSourcePlayer->GetOccupiedVehicleSeat () )
            {
                // Mis-matching seats can happen when we warp into a different one,
                // which will screw up the whole packet
                return false;
            }

            // Read out the vehicle matrix only if he's the driver
            unsigned int uiSeat = pSourcePlayer->GetOccupiedVehicleSeat ();
            if ( uiSeat == 0 )
            {
                // Read out the vehicle rotation in degrees
                SRotationDegreesSync rotation;
                if( !BitStream.Read ( &rotation ) )
                    return false;

                // Set it
                pVehicle->SetPosition ( position.data.vecPosition );
                pVehicle->SetRotationDegrees ( rotation.data.vecRotation );

                // Move speed vector
                SVelocitySync velocity;
                if ( !BitStream.Read ( &velocity ) )
                    return false;

                pVehicle->SetVelocity ( velocity.data.vecVelocity );
                pSourcePlayer->SetVelocity ( velocity.data.vecVelocity );

                // Turn speed vector
                SVelocitySync turnSpeed;
                if ( !BitStream.Read ( &turnSpeed ) )
                    return false;

                pVehicle->SetTurnSpeed ( turnSpeed.data.vecVelocity );

                // Health
                SVehicleHealthSync health;
                if ( !BitStream.Read ( &health ) )
                    return false;
                float fPreviousHealth = pVehicle->GetHealth ();                
                float fHealth = health.data.fValue;

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
                if ( !BitStream.ReadCompressed ( TrailerID ) )
                    return false;

                while ( TrailerID != INVALID_ELEMENT_ID )
                {
                    CElement* pElement = CElementIDs::GetElement ( TrailerID );
                    if ( pElement )
                        pTrailer = static_cast < CVehicle* > ( pElement );
                    
                    // Read out the trailer position and rotation
                    SPositionSync trailerPosition ( false );
                    if ( !BitStream.Read ( &trailerPosition ) )
                        return false;

                    SRotationDegreesSync trailerRotation;
                    if ( !BitStream.Read ( &trailerRotation ) )
                        return false;

                    // If we found the trailer
                    if ( pTrailer )
                    {
                        // Set its position and rotation
                        pTrailer->SetPosition ( trailerPosition.data.vecPosition );
                        pTrailer->SetRotationDegrees ( trailerRotation.data.vecRotation );
    
                        // Is this a new trailer, attached?
                        CVehicle* pCurrentTrailer = pTowedByVehicle->GetTowedVehicle ();
                        if ( pCurrentTrailer != pTrailer )
                        {
                            // If theres a trailer already attached
                            if ( pCurrentTrailer )
                            {
                                pTowedByVehicle->SetTowedVehicle ( NULL );
                                pCurrentTrailer->SetTowedByVehicle ( NULL );

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
                                pCurrentVehicle->SetTowedVehicle ( NULL );
                                pTrailer->SetTowedByVehicle ( NULL );

                                // Tell everyone to detach them
                                CVehicleTrailerPacket AttachPacket ( pCurrentVehicle, pTrailer, false );
                                g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( AttachPacket );

                                // Execute the attach trailer script function
                                CLuaArguments Arguments;
                                Arguments.PushElement ( pCurrentVehicle );
                                pTrailer->CallEvent ( "onTrailerDetach", Arguments );
                            }

                            pTowedByVehicle->SetTowedVehicle ( pTrailer );
                            pTrailer->SetTowedByVehicle ( pTowedByVehicle );

                            // Execute the attach trailer script function
                            CLuaArguments Arguments;
                            Arguments.PushElement ( pTowedByVehicle );
                            bool bContinue = pTrailer->CallEvent ( "onTrailerAttach", Arguments );

                            // Attach or detach trailers depending on the event outcome
                            CVehicleTrailerPacket TrailerPacket ( pTowedByVehicle, pTrailer, bContinue );
                            g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( TrailerPacket );
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
                    pTowedByVehicle->SetTowedVehicle ( NULL );
                    pCurrentTrailer->SetTowedByVehicle ( NULL );

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
            SPlayerHealthSync health;
            if ( !BitStream.Read ( &health ) )
                return false;
            float fHealth = health.data.fValue;

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
            SPlayerArmorSync armor;
            if ( !BitStream.Read ( &armor ) )
                return false;
            float fArmor = armor.data.fValue;

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
            if ( !BitStream.Read ( ucFlags ) )
                return false;
            bool bWearingGoggles    = ( ucFlags & 0x01 ) ? true:false;
            bool bDoingGangDriveby  = ( ucFlags & 0x02 ) ? true:false;
            bool bSirenActive       = ( ucFlags & 0x04 ) ? true:false;
            bool bSmokeTrail        = ( ucFlags & 0x08 ) ? true:false;
            bool bLandingGearDown   = ( ucFlags & 0x10 ) ? true:false;
            bool bOnGround          = ( ucFlags & 0x20 ) ? true:false;
            bool bInWater           = ( ucFlags & 0x40 ) ? true:false;
            bool bIsDerailed        = ( ucFlags & 0x80 ) ? true:false;

            pSourcePlayer->SetWearingGoggles ( bWearingGoggles );
            pSourcePlayer->SetDoingGangDriveby ( bDoingGangDriveby );            

            // Weapon stuff no compressed yet
            // Current weapon slot
            unsigned char ucCurrentWeaponSlot;
            if ( !BitStream.Read ( ucCurrentWeaponSlot ) )
                return false;
            pSourcePlayer->SetWeaponSlot ( ucCurrentWeaponSlot );
            unsigned char ucCurrentWeapon = pSourcePlayer->GetWeaponType ();
            if ( ucCurrentWeapon != 0 )
            {
                // Read out the ammo state
                unsigned short usAmmoInClip;
                if ( !BitStream.Read ( usAmmoInClip ) )
                    return false;
                pSourcePlayer->SetWeaponAmmoInClip ( usAmmoInClip );

                // Read out aim data
                SWeaponAimSync aim ( pSourcePlayer->GetWeaponRange () );
                if ( !BitStream.Read ( &aim ) )
                    return false;

                pSourcePlayer->SetAimDirection ( aim.data.fArm );
                pSourcePlayer->SetSniperSourceVector ( aim.data.vecOrigin );
                pSourcePlayer->SetTargettingVector ( aim.data.vecTarget );

                // Read out the driveby direction
                unsigned char ucDriveByDirection;
                if ( !BitStream.Read ( ucDriveByDirection ) )
                    return false;
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
                pVehicle->SetDerailed ( bIsDerailed );
            }

            // Success
            return true;
        }
    }

    return false;
}


bool CVehiclePuresyncPacket::Write ( NetBitStreamInterface& BitStream ) const
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
            BitStream.WriteCompressed ( usLatency );

            // Write the keysync data
            CControllerState ControllerState = pSourcePlayer->GetPad ()->GetCurrentControllerState ();
            WriteFullKeysync ( ControllerState, BitStream );

            // Write the vehicle matrix only if he's the driver
            CVector vecTemp;
            unsigned int uiSeat = pSourcePlayer->GetOccupiedVehicleSeat ();
            if ( uiSeat == 0 )
            {
                // Vehicle position
                SPositionSync position ( false );
                position.data.vecPosition = pVehicle->GetPosition ();
                BitStream.Write ( &position );

                // Vehicle rotation
                SRotationDegreesSync rotation;
                pVehicle->GetRotationDegrees ( rotation.data.vecRotation );
                BitStream.Write ( &rotation );

                // Move speed vector
                SVelocitySync velocity;
                velocity.data.vecVelocity = pVehicle->GetVelocity ();
                BitStream.Write ( &velocity );

                // Turn speed vector
                SVelocitySync turnSpeed;
                turnSpeed.data.vecVelocity = pVehicle->GetTurnSpeed ();
                BitStream.Write ( &turnSpeed );

                // Health
                SVehicleHealthSync health;
                health.data.fValue = pVehicle->GetHealth ();
                BitStream.Write ( &health );
            }

            // Player health and armor
            SPlayerHealthSync health;
            health.data.fValue = pSourcePlayer->GetHealth ();
            BitStream.Write ( &health );

            SPlayerArmorSync armor;
            armor.data.fValue = pSourcePlayer->GetArmor ();
            BitStream.Write ( &armor );

            // Flags
            unsigned char ucFlags = 0;
            if ( pSourcePlayer->IsWearingGoggles () )   ucFlags |= 0x01;
            if ( pSourcePlayer->IsDoingGangDriveby () ) ucFlags |= 0x02;
            if ( pVehicle->IsSirenActive () )           ucFlags |= 0x04;
            if ( pVehicle->IsSmokeTrailEnabled () )     ucFlags |= 0x08;
            if ( pVehicle->IsLandingGearDown () )       ucFlags |= 0x10;
            if ( pVehicle->IsOnGround () )              ucFlags |= 0x20;
            if ( pVehicle->IsInWater () )               ucFlags |= 0x40;
            if ( pVehicle->IsDerailed () )              ucFlags |= 0x80;

            // Write the flags
            BitStream.Write ( ucFlags );

            // Weapon stuff not compressed yet
            // Current weapon id
            unsigned char ucWeaponType = pSourcePlayer->GetWeaponType ();
            BitStream.Write ( ucWeaponType );
            if ( ucWeaponType != 0 )
            {
                // Write the ammo in clip
                BitStream.Write ( pSourcePlayer->GetWeaponAmmoInClip () );

                // Write the weapon aim data
                SWeaponAimSync aim ( 0.0f );
                aim.data.vecOrigin = pSourcePlayer->GetSniperSourceVector ();
                pSourcePlayer->GetTargettingVector ( aim.data.vecTarget );
                aim.data.fArm = pSourcePlayer->GetAimDirection ();
                BitStream.Write ( &aim );

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


void CVehiclePuresyncPacket::ReadVehicleSpecific ( CVehicle* pVehicle, NetBitStreamInterface& BitStream )
{
    // Turret data
    unsigned short usModel = pVehicle->GetModel ();
    if ( CVehicleManager::HasTurret ( usModel ) ) 
    {
        // Read out the turret position
        SVehicleSpecific vehicle;
        if ( !BitStream.Read ( &vehicle ) )
            return;

        // Set the data
        pVehicle->SetTurretPosition ( vehicle.data.fTurretX, vehicle.data.fTurretY );
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


void CVehiclePuresyncPacket::WriteVehicleSpecific ( CVehicle* pVehicle, NetBitStreamInterface& BitStream ) const
{
    // Turret states
    unsigned short usModel = pVehicle->GetModel ();
    if ( CVehicleManager::HasTurret ( usModel ) )
    {
        SVehicleSpecific vehicle;
        pVehicle->GetTurretPosition ( vehicle.data.fTurretX, vehicle.data.fTurretY );

        BitStream.Write ( &vehicle );
    }

    // Adjustable property value
    if ( CVehicleManager::HasAdjustableProperty ( usModel ) )
    {
        BitStream.Write ( pVehicle->GetAdjustableProperty () );
    }
}
