/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/rpc/CHandlingRPCs.cpp
*  PURPOSE:     Handling remote procedure calls
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               Florian Busse <flobu@gmx.net>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "CHandlingRPCs.h"

void CHandlingRPCs::LoadFunctions ( void )
{
    AddHandler ( SET_VEHICLE_HANDLING_PROPERTY, SetVehicleHandlingProperty, "SetVehicleHandlingProperty" );
    AddHandler ( RESET_VEHICLE_HANDLING_PROPERTY, RestoreVehicleHandlingProperty, "RestoreVehicleHandlingProperty" );
    AddHandler ( RESET_VEHICLE_HANDLING, RestoreVehicleHandling, "RestoreVehicleHandling" );
}


// Enum with property id's for handling
enum eHandlingProperty
{
    HANDLING_MASS = 1,
    HANDLING_TURNMASS,
    HANDLING_DRAGCOEFF,
    HANDLING_CENTEROFMASS,
    HANDLING_PERCENTSUBMERGED,
    HANDLING_TRACTIONMULTIPLIER,
    HANDLING_DRIVETYPE,
    HANDLING_ENGINETYPE,
    HANDLING_NUMOFGEARS,
    HANDLING_ENGINEACCELLERATION,
    HANDLING_ENGINEINERTIA,
    HANDLING_MAXVELOCITY,
    HANDLING_BRAKEDECELLERATION,
    HANDLING_BRAKEBIAS,
    HANDLING_ABS,
    HANDLING_STEERINGLOCK,
    HANDLING_TRACTIONLOSS,
    HANDLING_TRACTIONBIAS,
    HANDLING_SUSPENSION_FORCELEVEL,
    HANDLING_SUSPENSION_DAMPING,
    HANDLING_SUSPENSION_HIGHSPEEDDAMPING,
    HANDLING_SUSPENSION_UPPER_LIMIT,
    HANDLING_SUSPENSION_LOWER_LIMIT,
    HANDLING_SUSPENSION_FRONTREARBIAS,
    HANDLING_SUSPENSION_ANTIDIVEMULTIPLIER,
    HANDLING_COLLISIONDAMAGEMULTIPLIER,
    HANDLING_SEATOFFSETDISTANCE,
    HANDLING_MONETARY,
    HANDLING_HANDLINGFLAGS,
    HANDLING_MODELFLAGS,
    HANDLING_HEADLIGHT,
    HANDLING_TAILLIGHT,
    HANDLING_ANIMGROUP,
};


void CHandlingRPCs::SetVehicleHandlingProperty ( NetBitStreamInterface& bitStream )
{
    // Read out the handling id and property id
    ElementID ID;
    unsigned char ucProperty;
    if ( bitStream.Read ( ID ) &&
         bitStream.Read ( ucProperty ) )
    {
        // Grab it and check its type
        CClientEntity* pEntity = CElementIDs::GetElement ( ID );
        if ( pEntity && pEntity->GetType () == CCLIENTVEHICLE )
        {
            // Grab the vehicle handling entry
            CClientVehicle& Vehicle = static_cast < CClientVehicle& > ( *pEntity );
            CHandlingEntry* pHandlingEntry = Vehicle.GetHandlingData();

            // Temporary storage for reading out data
            union
            {
                unsigned char ucChar;
                unsigned int uiInt;
                float fFloat;
            };

            // Depending on what property
            switch ( ucProperty )
            {
                case HANDLING_MASS:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetMass ( fFloat );
                    break;
                    
                case HANDLING_TURNMASS:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetTurnMass ( fFloat );
                    break;

                case HANDLING_DRAGCOEFF:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetDragCoeff ( fFloat );
                    break;

                case HANDLING_CENTEROFMASS:
                {
                    CVector vecVector;
                    bitStream.Read ( vecVector.fX );
                    bitStream.Read ( vecVector.fY );
                    bitStream.Read ( vecVector.fZ );
                    pHandlingEntry->SetCenterOfMass ( vecVector );
                    break;
                }

                case HANDLING_PERCENTSUBMERGED:
                    bitStream.Read ( uiInt );
                    pHandlingEntry->SetPercentSubmerged ( uiInt );
                    break;

                case HANDLING_TRACTIONMULTIPLIER:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetTractionMultiplier ( fFloat );
                    break;

                case HANDLING_DRIVETYPE:
                {
                    bitStream.Read ( ucChar );
                    if ( ucChar != CHandlingEntry::FOURWHEEL &&
                         ucChar != CHandlingEntry::RWD &&
                         ucChar != CHandlingEntry::FWD )
                    {
                        ucChar = CHandlingEntry::RWD;
                    }

                    pHandlingEntry->SetCarDriveType ( static_cast < CHandlingEntry::eDriveType > ( ucChar ) );
                    break;
                }

                case HANDLING_ENGINETYPE:
                {
                    bitStream.Read ( ucChar );
                    if ( ucChar != CHandlingEntry::DIESEL &&
                         ucChar != CHandlingEntry::ELECTRIC &&
                         ucChar != CHandlingEntry::PETROL )
                    {
                        ucChar = CHandlingEntry::PETROL;
                    }

                    pHandlingEntry->SetCarEngineType ( static_cast < CHandlingEntry::eEngineType > ( ucChar ) );
                    break;
                }

                case HANDLING_NUMOFGEARS:
                    bitStream.Read ( ucChar );
                    pHandlingEntry->SetNumberOfGears ( ucChar );
                    break;

                case HANDLING_ENGINEACCELLERATION:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetEngineAccelleration ( fFloat );
                    break;

                case HANDLING_ENGINEINERTIA:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetEngineInertia ( fFloat );
                    break;

                case HANDLING_MAXVELOCITY:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetMaxVelocity ( fFloat );
                    break;

                case HANDLING_BRAKEDECELLERATION:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetBrakeDecelleration ( fFloat );
                    break;

                case HANDLING_BRAKEBIAS:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetBrakeBias ( fFloat );
                    break;

                case HANDLING_ABS:
                    bitStream.Read ( ucChar );
                    pHandlingEntry->SetABS ( ucChar != 0 );
                    break;

                case HANDLING_STEERINGLOCK:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetSteeringLock ( fFloat );
                    break;

                case HANDLING_TRACTIONLOSS:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetTractionLoss ( fFloat );
                    break;

                case HANDLING_TRACTIONBIAS:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetTractionBias ( fFloat );
                    break;

                case HANDLING_SUSPENSION_FORCELEVEL:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetSuspensionForceLevel ( fFloat );
                    break;

                case HANDLING_SUSPENSION_DAMPING:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetSuspensionDamping ( fFloat );
                    break;

                case HANDLING_SUSPENSION_HIGHSPEEDDAMPING:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetSuspensionHighSpeedDamping ( fFloat );
                    break;

                case HANDLING_SUSPENSION_UPPER_LIMIT:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetSuspensionUpperLimit ( fFloat );
                    break;

                case HANDLING_SUSPENSION_LOWER_LIMIT:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetSuspensionLowerLimit ( fFloat );
                    break;

                case HANDLING_SUSPENSION_FRONTREARBIAS:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetSuspensionFrontRearBias ( fFloat );
                    break;

                case HANDLING_SUSPENSION_ANTIDIVEMULTIPLIER:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetSuspensionAntidiveMultiplier ( fFloat );
                    break;

                case HANDLING_COLLISIONDAMAGEMULTIPLIER:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetCollisionDamageMultiplier ( fFloat );
                    break;

                case HANDLING_SEATOFFSETDISTANCE:
                    bitStream.Read ( fFloat );
                    pHandlingEntry->SetSeatOffsetDistance ( fFloat );
                    break;

                case HANDLING_MONETARY:
                    bitStream.Read ( uiInt );
                    pHandlingEntry->SetMonetary ( uiInt );
                    break;

                case HANDLING_HANDLINGFLAGS:
                    bitStream.Read ( uiInt );
                    pHandlingEntry->SetHandlingFlags ( uiInt );
                    break;

                case HANDLING_MODELFLAGS:
                    bitStream.Read ( uiInt );
                    pHandlingEntry->SetModelFlags ( uiInt );
                    break;

                case HANDLING_HEADLIGHT:
                    bitStream.Read ( ucChar );
                    if ( ucChar > CHandlingEntry::TALL )
                        ucChar = CHandlingEntry::TALL;

                    pHandlingEntry->SetHeadLight ( static_cast < CHandlingEntry::eLightType > ( ucChar ) );
                    break;

                case HANDLING_TAILLIGHT:
                    bitStream.Read ( ucChar );
                    if ( ucChar > CHandlingEntry::TALL )
                        ucChar = CHandlingEntry::TALL;

                    pHandlingEntry->SetTailLight ( static_cast < CHandlingEntry::eLightType > ( ucChar ) );
                    break;

                case HANDLING_ANIMGROUP:
                    bitStream.Read ( ucChar );
                    pHandlingEntry->SetAnimGroup ( ucChar );
                    break;
            }

            Vehicle.ApplyHandling();
        }
    }
}


void CHandlingRPCs::RestoreVehicleHandlingProperty ( NetBitStreamInterface& bitStream )
{
    // Read out the handling id and property id
    ElementID ID;
    unsigned char ucProperty;
    if ( bitStream.Read ( ID ) &&
         bitStream.Read ( ucProperty ) )
    {
        // Grab it and check its type
        CClientEntity* pEntity = CElementIDs::GetElement ( ID );
        if ( pEntity && pEntity->GetType () == CCLIENTVEHICLE )
        {
            // Grab the vehicle handling entry and the original handling
            CClientVehicle& Vehicle = static_cast < CClientVehicle& > ( *pEntity );
            CHandlingEntry* pHandlingEntry = Vehicle.GetHandlingData ();
            const CHandlingEntry* pOriginalEntry = Vehicle.GetOriginalHandlingData ();

            // Depending on what property
            switch ( ucProperty )
            {
                case HANDLING_MASS:
                    pHandlingEntry->SetMass ( pOriginalEntry->GetMass () );
                    break;
                    
                case HANDLING_TURNMASS:
                    pHandlingEntry->SetTurnMass ( pOriginalEntry->GetTurnMass () );
                    break;

                case HANDLING_DRAGCOEFF:
                    pHandlingEntry->SetDragCoeff ( pOriginalEntry->GetDragCoeff () );
                    break;

                case HANDLING_CENTEROFMASS:
                    pHandlingEntry->SetCenterOfMass ( pOriginalEntry->GetCenterOfMass () );
                    break;

                case HANDLING_PERCENTSUBMERGED:
                    pHandlingEntry->SetPercentSubmerged ( pOriginalEntry->GetPercentSubmerged () );
                    break;

                case HANDLING_TRACTIONMULTIPLIER:
                    pHandlingEntry->SetTractionMultiplier ( pOriginalEntry->GetTractionMultiplier () );
                    break;

                case HANDLING_DRIVETYPE:
                    pHandlingEntry->SetCarDriveType ( pOriginalEntry->GetCarDriveType () );
                    break;

                case HANDLING_ENGINETYPE:
                    pHandlingEntry->SetCarEngineType ( pOriginalEntry->GetCarEngineType () );
                    break;

                case HANDLING_NUMOFGEARS:
                    pHandlingEntry->SetNumberOfGears ( pOriginalEntry->GetNumberOfGears () );
                    break;

                case HANDLING_ENGINEACCELLERATION:
                    pHandlingEntry->SetEngineAccelleration ( pOriginalEntry->GetEngineAccelleration () );
                    break;

                case HANDLING_ENGINEINERTIA:
                    pHandlingEntry->SetEngineInertia ( pOriginalEntry->GetEngineInertia () );
                    break;

                case HANDLING_MAXVELOCITY:
                    pHandlingEntry->SetMaxVelocity ( pOriginalEntry->GetMaxVelocity () );
                    break;

                case HANDLING_BRAKEDECELLERATION:
                    pHandlingEntry->SetBrakeDecelleration ( pOriginalEntry->GetBrakeDecelleration () );
                    break;

                case HANDLING_BRAKEBIAS:
                    pHandlingEntry->SetBrakeBias ( pOriginalEntry->GetBrakeBias () );
                    break;

                case HANDLING_ABS:
                    pHandlingEntry->SetABS ( pOriginalEntry->GetABS () );
                    break;

                case HANDLING_STEERINGLOCK:
                    pHandlingEntry->SetSteeringLock ( pOriginalEntry->GetSteeringLock () );
                    break;

                case HANDLING_TRACTIONLOSS:
                    pHandlingEntry->SetTractionLoss ( pOriginalEntry->GetTractionLoss () );
                    break;

                case HANDLING_TRACTIONBIAS:
                    pHandlingEntry->SetTractionBias ( pOriginalEntry->GetTractionBias () );
                    break;

                case HANDLING_SUSPENSION_FORCELEVEL:
                    pHandlingEntry->SetSuspensionForceLevel ( pOriginalEntry->GetSuspensionForceLevel () );
                    break;

                case HANDLING_SUSPENSION_DAMPING:
                    pHandlingEntry->SetSuspensionDamping ( pOriginalEntry->GetSuspensionDamping () );
                    break;

                case HANDLING_SUSPENSION_HIGHSPEEDDAMPING:
                    pHandlingEntry->SetSuspensionHighSpeedDamping ( pOriginalEntry->GetSuspensionHighSpeedDamping () );
                    break;

                case HANDLING_SUSPENSION_UPPER_LIMIT:
                    pHandlingEntry->SetSuspensionUpperLimit ( pOriginalEntry->GetSuspensionUpperLimit () );
                    break;

                case HANDLING_SUSPENSION_LOWER_LIMIT:
                    pHandlingEntry->SetSuspensionLowerLimit ( pOriginalEntry->GetSuspensionLowerLimit () );
                    break;

                case HANDLING_SUSPENSION_FRONTREARBIAS:
                    pHandlingEntry->SetSuspensionFrontRearBias ( pOriginalEntry->GetSuspensionFrontRearBias () );
                    break;

                case HANDLING_SUSPENSION_ANTIDIVEMULTIPLIER:
                    pHandlingEntry->SetSuspensionAntidiveMultiplier ( pOriginalEntry->GetSuspensionAntidiveMultiplier () );
                    break;

                case HANDLING_COLLISIONDAMAGEMULTIPLIER:
                    pHandlingEntry->SetCollisionDamageMultiplier ( pOriginalEntry->GetCollisionDamageMultiplier () );
                    break;

                case HANDLING_SEATOFFSETDISTANCE:
                    pHandlingEntry->SetSeatOffsetDistance ( pOriginalEntry->GetSeatOffsetDistance () );
                    break;

                case HANDLING_HANDLINGFLAGS:
                    pHandlingEntry->SetHandlingFlags ( pOriginalEntry->GetHandlingFlags () );
                    break;

                case HANDLING_MODELFLAGS:
                    pHandlingEntry->SetModelFlags ( pOriginalEntry->GetModelFlags () );
                    break;

                case HANDLING_HEADLIGHT:
                    pHandlingEntry->SetHeadLight ( pOriginalEntry->GetHeadLight () );
                    break;

                case HANDLING_TAILLIGHT:
                    pHandlingEntry->SetTailLight ( pOriginalEntry->GetTailLight () );
                    break;

                case HANDLING_ANIMGROUP:
                    pHandlingEntry->SetAnimGroup ( pOriginalEntry->GetAnimGroup () );
                    break;
            }

            Vehicle.ApplyHandling();
        }
    }
}


void CHandlingRPCs::RestoreVehicleHandling ( NetBitStreamInterface& bitStream )
{
    // Read out the handling id
    ElementID ID;
    if ( bitStream.Read ( ID ) )
    {
        // Grab it and check its type
        CClientEntity* pEntity = CElementIDs::GetElement ( ID );
        if ( pEntity && pEntity->GetType () == CCLIENTVEHICLE )
        {
            // Grab the vehicle handling entry and restore all data
            CClientVehicle& Vehicle = static_cast < CClientVehicle& > ( *pEntity );
            Vehicle.GetHandlingData()->Restore();

            Vehicle.ApplyHandling();
        }
    }
}