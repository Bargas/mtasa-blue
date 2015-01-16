/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Vehicle.cpp
*  PURPOSE:     Lua function definitions class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Chris McArthur <>
*               Derek Abdine <>
*               Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*
*****************************************************************************/

#include "StdInc.h"

int CLuaFunctionDefs::GetVehicleType ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );

    unsigned long ucModel = 0;

    if ( argStream.NextIsUserData ( ) )
    {
        argStream.ReadUserData ( pVehicle );

        if ( !argStream.HasErrors () )
            ucModel = pVehicle->GetModel ();
    }
    else
    {
        argStream.ReadNumber ( ucModel );
    }

    if ( !argStream.HasErrors () )
    {
        lua_pushstring ( luaVM, CVehicleNames::GetVehicleTypeName ( ucModel ) ); // Range check will be done by GetVehicleTypeName
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );
    
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::IsVehicleTaxiLightOn ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        if ( pVehicle->GetModel() == 438 || pVehicle->GetModel() == 420 )
        {
            bool bLightState = pVehicle->IsTaxiLightOn ();
            if ( bLightState )
            {
                lua_pushboolean ( luaVM, bLightState );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetVehicleTaxiLightOn ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    bool bLightState = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadBool ( bLightState );

    if ( !argStream.HasErrors ( ) )
    {
        if ( pVehicle->GetModel() == 438 || pVehicle->GetModel() == 420 )
        {
            pVehicle->SetTaxiLightOn ( bLightState );
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleVariant ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    bool bLightState = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned char ucVariant = -1;
        unsigned char ucVariant2 = -1;
        if ( CStaticFunctionDefinitions::GetVehicleVariant ( pVehicle, ucVariant, ucVariant2 ) )
        {
            lua_pushnumber ( luaVM, ucVariant );
            lua_pushnumber ( luaVM, ucVariant2 );
            return 2;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleColor ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bRGB = false;
        if ( argStream.NextIsBool ( ) )
            argStream.ReadBool ( bRGB );

        if ( pVehicle ) 
        {
            CVehicleColor& color = pVehicle->GetColor ();

            if ( bRGB )
            {
                for ( uint i = 0 ; i < 4 ; i++ )
                {
                    SColor RGBColor = color.GetRGBColor ( i );
                    lua_pushnumber ( luaVM, RGBColor.R );
                    lua_pushnumber ( luaVM, RGBColor.G );
                    lua_pushnumber ( luaVM, RGBColor.B );
                }
                return 12;
            }
            else
            {
                lua_pushnumber ( luaVM, color.GetPaletteColor ( 0 ) );
                lua_pushnumber ( luaVM, color.GetPaletteColor ( 1 ) );
                lua_pushnumber ( luaVM, color.GetPaletteColor ( 2 ) );
                lua_pushnumber ( luaVM, color.GetPaletteColor ( 3 ) );
                return 4;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "vehicle", 1 );
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleModelFromName ( lua_State* luaVM )
{
    SString strName = "";
    CClientVehicle* pVehicle = NULL;
    bool bLightState = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strName );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned short usModel;
        if ( CStaticFunctionDefinitions::GetVehicleModelFromName ( strName, usModel ) )
        {
            lua_pushnumber ( luaVM, static_cast < lua_Number > ( usModel ) );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleLandingGearDown ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        // Does the vehicle have landing gears?
        if ( CClientVehicleManager::HasLandingGears ( pVehicle->GetModel () ) )
        {
            // Return whether it has landing gears or not
            bool bLandingGear = pVehicle->IsLandingGearDown ();
            lua_pushboolean ( luaVM, bLandingGear );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::GetVehicleMaxPassengers ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    unsigned short usModel = 0;
    CScriptArgReader argStream ( luaVM );

    if ( argStream.NextIsUserData () )
    {
        argStream.ReadUserData ( pVehicle );

        if ( !argStream.HasErrors () )
            usModel = pVehicle->GetModel ();
    }
    else
    {
        argStream.ReadNumber ( usModel );
    }

    if ( !argStream.HasErrors () )
    {
        if ( CClientVehicleManager::IsValidModel ( usModel ) )
        {
            unsigned int uiMaxPassengers = CClientVehicleManager::GetMaxPassengerCount ( usModel );
            if ( uiMaxPassengers != 0xFF )
            {
                lua_pushnumber ( luaVM, uiMaxPassengers );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleOccupant ( lua_State* luaVM )
{
    unsigned int uiSeat = 0;
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadIfNextIsNumber ( uiSeat, 0 );

    if ( !argStream.HasErrors ( ) )
    {
        CClientPed* pPed = pVehicle->GetOccupant ( uiSeat );
        if ( pPed )
        {
            lua_pushelement ( luaVM, pPed );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleOccupants ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        // Create a new table
        lua_newtable ( luaVM );

        // Get the maximum amount of passengers
        unsigned char ucMaxPassengers = CClientVehicleManager::GetMaxPassengerCount ( pVehicle->GetModel ( ) );

        // Make sure that if the vehicle doesn't have any seats, the function returns false
        if ( ucMaxPassengers == 255 )
        {
            lua_pushboolean ( luaVM, false );
            return 1;
        }

        // Add All Occupants
        for ( unsigned char ucSeat = 0; ucSeat <= ucMaxPassengers; ++ ucSeat )
        {
            CClientPed* pPed = pVehicle->GetOccupant ( ucSeat );
            if ( pPed )
            {
                lua_pushnumber ( luaVM, ucSeat );
                lua_pushelement ( luaVM, pPed );
                lua_settable ( luaVM, -3 );
            }
        }
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );
            
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleController ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        CClientPed* pPed = pVehicle->GetControllingPlayer ();
        if ( pPed )
        {
            lua_pushelement ( luaVM, pPed );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleSirensOn ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        // Does the vehicle have Sirens?
        if ( CClientVehicleManager::HasSirens ( pVehicle->GetModel () ) || pVehicle->DoesVehicleHaveSirens ( ) )
        {
            // Return whether it has its Sirens on or not
            bool bSirensOn = pVehicle->IsSirenOrAlarmActive ();
            lua_pushboolean ( luaVM, bSirensOn );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::GetVehicleTurnVelocity ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        CVector vecTurnVelocity;
        pVehicle->GetTurnSpeed ( vecTurnVelocity );

        lua_pushnumber ( luaVM, vecTurnVelocity.fX );
        lua_pushnumber ( luaVM, vecTurnVelocity.fY );
        lua_pushnumber ( luaVM, vecTurnVelocity.fZ );
        return 3;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleTurretPosition ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        CVector2D vecPosition;
        pVehicle->GetTurretRotation ( vecPosition.fX, vecPosition.fY );

        lua_pushnumber ( luaVM, vecPosition.fX );
        lua_pushnumber ( luaVM, vecPosition.fY );
        return 2;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsVehicleLocked ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bLocked = pVehicle->AreDoorsLocked ();
        lua_pushboolean ( luaVM, bLocked );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushnil ( luaVM );
    return 1;
}


int CLuaFunctionDefs::GetVehicleUpgradeOnSlot ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    unsigned char ucSlot = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadNumber ( ucSlot );

    if ( !argStream.HasErrors ( ) )
    {
        CVehicleUpgrades* pUpgrades = pVehicle->GetUpgrades ();
        if ( pUpgrades )
        {
            unsigned short usUpgrade = pUpgrades->GetSlotState ( ucSlot );
            lua_pushnumber ( luaVM, usUpgrade );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleUpgrades ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        CVehicleUpgrades* pUpgrades = pVehicle->GetUpgrades ();
        if ( pUpgrades )
        {
            // Create a new table
            lua_newtable ( luaVM );

            // Add all the upgrades to the table
            const SSlotStates& usSlotStates = pUpgrades->GetSlotStates ();

            unsigned int uiIndex = 0;
            unsigned char ucSlot = 0;
            for ( ; ucSlot < VEHICLE_UPGRADE_SLOTS ; ucSlot++ )
            {
                if ( usSlotStates [ucSlot] != 0 )
                {
                    lua_pushnumber ( luaVM, ++uiIndex );
                    lua_pushnumber ( luaVM, usSlotStates [ ucSlot ] );
                    lua_settable ( luaVM, -3 );
                }
            }

            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleUpgradeSlotName ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    unsigned short usNumber = 0xFF;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( usNumber );

    if ( !argStream.HasErrors ( ) )
    {
        if ( usNumber < 17 )
        {
            SString strUpgradeName;
            if ( CStaticFunctionDefinitions::GetVehicleUpgradeSlotName ( static_cast < unsigned char > ( usNumber ), strUpgradeName ) )
            {
                lua_pushstring ( luaVM, strUpgradeName );
                return 1;
            }
        }
        else if ( usNumber >= 1000 && usNumber <= 1193 )
        {
            SString strUpgradeName;
            if ( CStaticFunctionDefinitions::GetVehicleUpgradeSlotName ( usNumber, strUpgradeName ) )
            {
                lua_pushstring ( luaVM, strUpgradeName );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "slot/upgrade", 1 );
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleCompatibleUpgrades ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    unsigned char ucSlot = 0xFF;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadIfNextIsNumber ( ucSlot, 0xFF );

    if ( !argStream.HasErrors ( ) )
    {
        CVehicleUpgrades* pUpgrades = pVehicle->GetUpgrades ();
        if ( pUpgrades )
        {
            // Create a new table
            lua_newtable ( luaVM );

            unsigned int uiIndex = 0;
            for ( unsigned short usUpgrade = 1000 ; usUpgrade <= 1193 ; usUpgrade++ )
            {
                if ( pUpgrades->IsUpgradeCompatible ( usUpgrade ) )
                {
                    if ( ucSlot != 0xFF )
                    {
                        unsigned char ucUpgradeSlot;
                        if ( !pUpgrades->GetSlotFromUpgrade ( usUpgrade, ucUpgradeSlot ) )
                            continue;

                        if ( ucUpgradeSlot != ucSlot )
                            continue;
                    }

                    // Add this one to a list                        
                    lua_pushnumber ( luaVM, ++uiIndex );
                    lua_pushnumber ( luaVM, usUpgrade );
                    lua_settable ( luaVM, -3 );
                }
            }
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleWheelStates ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned char ucFrontLeft = pVehicle->GetWheelStatus ( FRONT_LEFT_WHEEL );
        unsigned char ucRearLeft = pVehicle->GetWheelStatus ( REAR_LEFT_WHEEL );
        unsigned char ucFrontRight = pVehicle->GetWheelStatus ( FRONT_RIGHT_WHEEL );
        unsigned char ucRearRight = pVehicle->GetWheelStatus ( REAR_RIGHT_WHEEL );

        lua_pushnumber ( luaVM, ucFrontLeft );
        lua_pushnumber ( luaVM, ucRearLeft );
        lua_pushnumber ( luaVM, ucFrontRight );
        lua_pushnumber ( luaVM, ucRearRight );
        return 4;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleDoorState ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    unsigned char ucDoor = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadNumber ( ucDoor );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned char ucState = pVehicle->GetDoorStatus ( ucDoor );
        lua_pushnumber ( luaVM, ucState );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleLightState ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    unsigned char ucLight = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadNumber ( ucLight );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned char ucState = pVehicle->GetLightStatus ( ucLight );
        lua_pushnumber ( luaVM, ucState );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehiclePanelState ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    unsigned char ucPanel = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadNumber ( ucPanel );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned char ucState = pVehicle->GetPanelStatus ( ucPanel );
        lua_pushnumber ( luaVM, ucState );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleOverrideLights ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned char ucLights = pVehicle->GetOverrideLights ();
        lua_pushnumber ( luaVM, ucLights );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleTowedByVehicle ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        CClientVehicle* pTowedVehicle;
        if ( pVehicle->GetVehicleType () == CLIENTVEHICLE_TRAIN )
            pTowedVehicle = pVehicle->GetNextTrainCarriage ();
        else
            pTowedVehicle = pVehicle->GetTowedVehicle ();

        if ( pTowedVehicle )
        {
            lua_pushelement ( luaVM, pTowedVehicle );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleTowingVehicle ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        CClientVehicle* pTowedByVehicle;
        if ( pVehicle->GetVehicleType () == CLIENTVEHICLE_TRAIN )
            pTowedByVehicle = pVehicle->GetPreviousTrainCarriage ();
        else
            pTowedByVehicle = pVehicle->GetTowedByVehicle ();

        if ( pTowedByVehicle )
        {
            lua_pushelement ( luaVM, pTowedByVehicle );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehiclePaintjob ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned char ucPaintjob = pVehicle->GetPaintjob ();
        lua_pushnumber ( luaVM, ucPaintjob );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehiclePlateText ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        const char* szRegPlate = pVehicle->GetRegPlate ();
        if ( szRegPlate )
        {
            lua_pushstring ( luaVM, szRegPlate );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsVehicleDamageProof ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bDamageProof;
        if ( CStaticFunctionDefinitions::IsVehicleDamageProof ( *pVehicle, bDamageProof ) )
        {
            lua_pushboolean ( luaVM, bDamageProof );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsVehicleFuelTankExplodable ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bExplodable = pVehicle->GetCanShootPetrolTank ();
        lua_pushboolean ( luaVM, bExplodable );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsVehicleFrozen ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bFrozen = pVehicle->IsFrozen ();
        lua_pushboolean ( luaVM, bFrozen );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsVehicleOnGround ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bOnGround = pVehicle->IsOnGround ();
        lua_pushboolean ( luaVM, bOnGround );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleName ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        const char* szVehicleName = CVehicleNames::GetVehicleName ( pVehicle->GetModel () );
        if ( szVehicleName )
        {
            lua_pushstring ( luaVM, szVehicleName );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleAdjustableProperty ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        if ( pVehicle->HasAdjustableProperty () )
        {
            unsigned short usAdjustableProperty = pVehicle->GetAdjustablePropertyValue ();
            lua_pushnumber ( luaVM, usAdjustableProperty );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetHelicopterRotorSpeed ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        float fSpeed;
        if ( CStaticFunctionDefinitions::GetHelicopterRotorSpeed ( *pVehicle, fSpeed ) )
        {
            lua_pushnumber ( luaVM, fSpeed );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsTrainDerailed ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bDerailed;
        if ( CStaticFunctionDefinitions::IsTrainDerailed ( *pVehicle, bDerailed ) )
        {
            lua_pushboolean ( luaVM, bDerailed );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::IsTrainDerailable ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bIsDerailable;
        if ( CStaticFunctionDefinitions::IsTrainDerailable ( *pVehicle, bIsDerailable ) )
        {
            lua_pushboolean ( luaVM, bIsDerailable );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetTrainDirection ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bDirection;
        if ( CStaticFunctionDefinitions::GetTrainDirection ( *pVehicle, bDirection ) )
        {
            lua_pushboolean ( luaVM, bDirection );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetTrainSpeed ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        float fSpeed;
        if ( CStaticFunctionDefinitions::GetTrainSpeed ( *pVehicle, fSpeed ) )
        {
            lua_pushnumber ( luaVM, fSpeed );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::IsTrainChainEngine ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors () )
    {
        bool bChainEngine;
        if ( CStaticFunctionDefinitions::IsTrainChainEngine ( *pVehicle, bChainEngine ) )
        {
            lua_pushboolean ( luaVM, bChainEngine );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleEngineState ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bState;
        if ( CStaticFunctionDefinitions::GetVehicleEngineState ( *pVehicle, bState ) )
        {
            lua_pushboolean ( luaVM, bState );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleNameFromModel ( lua_State* luaVM )
{
    unsigned short usModel = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( usModel );

    if ( !argStream.HasErrors ( ) )
    {
        SString strVehicleName;

        if ( CStaticFunctionDefinitions::GetVehicleNameFromModel ( usModel, strVehicleName ) )
        {
            lua_pushstring ( luaVM, strVehicleName );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::CreateVehicle ( lua_State* luaVM )
{
    CVector vecPosition;
    unsigned short usModel = 0;
    CVector vecRotation;
    const char* szRegPlate = NULL;
    unsigned char ucVariant = 255;
    unsigned char ucVariant2 = 255;
    SString strRegPlate = "";
    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( usModel );
    argStream.ReadVector3D ( vecPosition );
    argStream.ReadVector3D ( vecRotation, vecRotation );
    argStream.ReadString ( strRegPlate, "" );
    argStream.ReadNumber ( ucVariant, 255 );
    argStream.ReadNumber ( ucVariant2, 255 );

    if ( !argStream.HasErrors ( ) )
    {
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            CResource * pResource = pLuaMain->GetResource();
            if ( pResource )
            {
                // Create the vehicle and return its handle
                CClientVehicle* pVehicle = CStaticFunctionDefinitions::CreateVehicle ( *pResource, usModel, vecPosition, vecRotation, strRegPlate == "" ? NULL : strRegPlate.c_str(), ucVariant, ucVariant2 );
                if ( pVehicle )
                {
                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( ( CClientEntity* ) pVehicle );
                    }

                    lua_pushelement ( luaVM, pVehicle );
                    return 1;
                }
            }
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::FixVehicle ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );

    if ( !argStream.HasErrors ( ) )
    {
        if ( CStaticFunctionDefinitions::FixVehicle ( *pEntity ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::BlowVehicle ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );

    if ( !argStream.HasErrors ( ) )
    {
        if ( CStaticFunctionDefinitions::BlowVehicle ( *pEntity ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::IsVehicleBlown ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        bool bBlown;
        if ( CStaticFunctionDefinitions::IsVehicleBlown ( *pVehicle, bBlown ) ) 
        {
            lua_pushboolean ( luaVM, bBlown );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleHeadLightColor ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        SColor color;
        if ( CStaticFunctionDefinitions::GetVehicleHeadLightColor ( *pVehicle, color ) )
        {
            lua_pushnumber ( luaVM, color.R );
            lua_pushnumber ( luaVM, color.G );
            lua_pushnumber ( luaVM, color.B );
            return 3;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleCurrentGear ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) )
    {
        unsigned short currentGear;
        if ( CStaticFunctionDefinitions::GetVehicleCurrentGear ( *pVehicle, currentGear ) )
        {
            lua_pushnumber ( luaVM, currentGear );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleTurnVelocity ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    CVector vecTurnVelocity;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadVector3D ( vecTurnVelocity );

    if ( !argStream.HasErrors ( ) )
    {
        if ( CStaticFunctionDefinitions::SetVehicleTurnVelocity ( *pEntity, vecTurnVelocity ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleColor ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    CVector vecTurnVelocity;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );

    // Count up number of args after the first one
    uchar ucParams[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int i;
    for ( i = 0 ; i < 12 ; i++ )
    {
        if ( argStream.NextCouldBeNumber ( ) )
        {
            argStream.ReadNumber ( ucParams[i] );
        }
        else
            break;
    }

    if ( !argStream.HasErrors ( ) )
    {
        CVehicleColor color;

        if ( i == 4 )
        {
            // 4 args mean palette colours
            color.SetPaletteColors ( ucParams[0], ucParams[1], ucParams[2], ucParams[3] );
        }
        else
        if ( i == 3 || i == 6 || i == 9 || i == 12 )
        {
            // 3,6,9 or 12 args mean rgb colours
            color.SetRGBColors ( SColorRGBA ( ucParams[0], ucParams[1], ucParams[2], 0 ),
                                    SColorRGBA ( ucParams[3], ucParams[4], ucParams[5], 0 ),
                                    SColorRGBA ( ucParams[6], ucParams[7], ucParams[8], 0 ),
                                    SColorRGBA ( ucParams[9], ucParams[10], ucParams[11], 0 ) );
        }
        else
            argStream.SetCustomError( "Incorrect number of color arguments" );

        if ( !argStream.HasErrors ( ) )
        {
            if ( CStaticFunctionDefinitions::SetVehicleColor ( *pEntity, color ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
    }
    if ( argStream.HasErrors ( ) )
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleLandingGearDown ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    bool bLandingGearDown = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadBool ( bLandingGearDown );

    if ( !argStream.HasErrors ( ) )
    {
        if ( CClientVehicleManager::HasLandingGears ( pVehicle->GetModel () ) )
        {
            // Do it
            if ( CStaticFunctionDefinitions::SetVehicleLandingGearDown ( *pVehicle, bLandingGearDown ) )
                lua_pushboolean ( luaVM, true );
            else
                lua_pushboolean ( luaVM, false );
        }
        else
            lua_pushboolean ( luaVM, false );

        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleLocked ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    bool bLocked = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadBool ( bLocked );

    if ( !argStream.HasErrors ( ) )
    {
        if ( CStaticFunctionDefinitions::SetVehicleLocked ( *pEntity, bLocked ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleDoorsUndamageable ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    bool bDoorsUndamageable = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadBool ( bDoorsUndamageable );

    if ( !argStream.HasErrors ( ) )
    {
        if ( CStaticFunctionDefinitions::SetVehicleDoorsUndamageable ( *pEntity, bDoorsUndamageable ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleSirensOn ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    bool bSirensOn = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadBool ( bSirensOn );

    if ( !argStream.HasErrors ( ) )
    {
        if ( pEntity )
        {
            // Do it
            if ( CStaticFunctionDefinitions::SetVehicleSirensOn ( *pEntity, bSirensOn ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "element", 1 );
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::AddVehicleUpgrade ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    unsigned short usUpgrade = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( argStream.NextIsString ( ) )
        {
            SString strUpgrade = "";
            argStream.ReadString ( strUpgrade );
            if ( strUpgrade == "all" )
            {
                lua_pushboolean ( luaVM, CStaticFunctionDefinitions::AddAllVehicleUpgrades ( *pEntity ) );
                return 1;
            }
            else
                argStream.m_iIndex--;
        }
            
        argStream.ReadNumber ( usUpgrade );
        if ( argStream.HasErrors ( ) ) 
        {
            m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );
            lua_pushboolean ( luaVM, false );
            return 1;
        }            

        if ( CStaticFunctionDefinitions::AddVehicleUpgrade ( *pEntity, usUpgrade ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::RemoveVehicleUpgrade ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    unsigned short usUpgrade = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( usUpgrade );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::RemoveVehicleUpgrade ( *pEntity, usUpgrade ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleDoorState ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    unsigned char ucDoor = 0, ucState = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( ucDoor );
    argStream.ReadNumber ( ucState );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleDoorState ( *pEntity, ucDoor, ucState ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleWheelStates ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    int iFrontLeft = -1;
    int iRearLeft = -1, iFrontRight = -1, iRearRight = -1;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( iFrontLeft );
    argStream.ReadNumber ( iRearLeft, -1 );
    argStream.ReadNumber ( iFrontRight, -1 );
    argStream.ReadNumber ( iRearRight, -1 );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleWheelStates ( *pEntity, iFrontLeft, iRearLeft, iFrontRight, iRearRight ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleLightState ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    unsigned char ucLight = 0, ucState = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( ucLight );
    argStream.ReadNumber ( ucState );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleLightState ( *pEntity, ucLight, ucState ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehiclePanelState ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    unsigned char ucPanel = 0, ucState = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( ucPanel );
    argStream.ReadNumber ( ucState );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehiclePanelState ( *pEntity, ucPanel, ucState ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleOverrideLights ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    unsigned char ucLights = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( ucLights );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( ucLights <= 2 )
        {
            if ( CStaticFunctionDefinitions::SetVehicleOverrideLights ( *pEntity, ucLights ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::AttachTrailerToVehicle ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL; CClientVehicle* pTrailer = NULL; CVector vecRotationOffsetDegrees;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadUserData ( pTrailer );
    argStream.ReadVector3D ( vecRotationOffsetDegrees, CVector ( ) );
    
    if ( !argStream.HasErrors () )
    {
        if ( CStaticFunctionDefinitions::AttachTrailerToVehicle ( *pVehicle, *pTrailer, vecRotationOffsetDegrees ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::DetachTrailerFromVehicle ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CClientVehicle* pTrailer = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadUserData ( pTrailer, NULL );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::DetachTrailerFromVehicle ( *pVehicle, pTrailer ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleEngineState ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    bool bState = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadBool ( bState );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleEngineState ( *pEntity, bState ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleDirtLevel ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    float fDirtLevel = 0.0f;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( fDirtLevel );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleDirtLevel ( *pEntity, fDirtLevel ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleDamageProof ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    bool bDamageProof = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadBool ( bDamageProof );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleDamageProof ( *pEntity, bDamageProof ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehiclePaintjob ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    unsigned char ucPaintjob = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( ucPaintjob );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehiclePaintjob ( *pEntity, ucPaintjob ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleFuelTankExplodable ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    bool bExplodable = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadBool ( bExplodable );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleFuelTankExplodable ( *pEntity, bExplodable ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleFrozen ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    bool bFrozen = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadBool ( bFrozen );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleFrozen ( *pVehicle, bFrozen ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleAdjustableProperty ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    unsigned short usAdjustableProperty = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( usAdjustableProperty );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleAdjustableProperty ( *pEntity, usAdjustableProperty ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetHelicopterRotorSpeed ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    float fSpeed = 0.0f;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadNumber ( fSpeed );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetHelicopterRotorSpeed ( *pVehicle, fSpeed ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetTrainDerailed ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    bool bDerailed = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadBool ( bDerailed );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetTrainDerailed ( *pVehicle, bDerailed ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetTrainDerailable ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    bool bDerailable = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadBool ( bDerailable );

    if ( !argStream.HasErrors ( ) ) 
    {;
        if ( CStaticFunctionDefinitions::SetTrainDerailable ( *pVehicle, bDerailable ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetTrainDirection ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    bool bDirection = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadBool ( bDirection );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetTrainDirection ( *pVehicle, bDirection ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetTrainSpeed ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    float fSpeed = 0.0f;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadNumber ( fSpeed );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetTrainSpeed ( *pVehicle, fSpeed ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}



// Radio

int CLuaFunctionDefs::SetRadioChannel ( lua_State* luaVM )
{
    unsigned char ucChannel = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( ucChannel );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetRadioChannel ( ucChannel ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetRadioChannel ( lua_State* luaVM )
{
    unsigned char ucChannel = 0;
    if ( CStaticFunctionDefinitions::GetRadioChannel ( ucChannel ) )
    {
        lua_pushnumber ( luaVM, ucChannel );
        return 1;
    }

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetRadioChannelName ( lua_State* luaVM )
{
    static const SFixedArray < const char*, 13 > szRadioStations = { {
        "Radio off", "Playback FM", "K-Rose", "K-DST",
        "Bounce FM", "SF-UR", "Radio Los Santos", "Radio X", "CSR 103.9", "K-Jah West",
        "Master Sounds 98.3", "WCTR", "User Track Player" } };

    int iChannel = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( iChannel );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( iChannel >= 0 && iChannel < NUMELMS( szRadioStations ) ) 
        {
            lua_pushstring ( luaVM, szRadioStations [ iChannel ] );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleGravity ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) ) 
    {
        CVector vecGravity;
        pVehicle->GetGravity ( vecGravity );
        lua_pushnumber ( luaVM, vecGravity.fX );
        lua_pushnumber ( luaVM, vecGravity.fY );
        lua_pushnumber ( luaVM, vecGravity.fZ );
        return 3;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetVehicleGravity ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CVector vecGravity;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadVector3D ( vecGravity );

    if ( !argStream.HasErrors ( ) ) 
    {
        pVehicle->SetGravity ( vecGravity );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleHeadLightColor ( lua_State* luaVM )
{
    CClientEntity* pEntity = NULL;
    SColor color;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( color.R );
    argStream.ReadNumber ( color.G );
    argStream.ReadNumber ( color.B );
    color.A = 255;

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleHeadLightColor ( *pEntity, color ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetVehicleTurretPosition ( lua_State *luaVM )
{
    CClientVehicle* pVehicle = NULL;
    float fHorizontal = 0.0f, fVertical = 0.0f;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadNumber ( fHorizontal );
    argStream.ReadNumber ( fVertical );

    if ( !argStream.HasErrors ( ) ) 
    {
        pVehicle->SetTurretRotation ( fHorizontal, fVertical );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleHandling ( lua_State* luaVM )
{
    CClientVehicle* pVehicle = NULL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors ( ) ) 
    {
        CHandlingEntry* pEntry = pVehicle->GetHandlingData ();

        lua_newtable ( luaVM );

        lua_pushnumber ( luaVM, pEntry->GetMass() );
        lua_setfield ( luaVM, -2, "mass" );

        lua_pushnumber ( luaVM, pEntry->GetTurnMass() );
        lua_setfield ( luaVM, -2, "turnMass" );

        lua_pushnumber ( luaVM, pEntry->GetDragCoeff() );
        lua_setfield ( luaVM, -2, "dragCoeff" );

        lua_createtable ( luaVM, 3, 0 );
        CVector vecCenter = pEntry->GetCenterOfMass ();
        lua_pushnumber ( luaVM, 1 );
        lua_pushnumber ( luaVM, vecCenter.fX );
        lua_settable ( luaVM, -3 );
        lua_pushnumber ( luaVM, 2 );
        lua_pushnumber ( luaVM, vecCenter.fY );
        lua_settable ( luaVM, -3 );
        lua_pushnumber ( luaVM, 3 );
        lua_pushnumber ( luaVM, vecCenter.fZ );
        lua_settable ( luaVM, -3 );
        lua_setfield ( luaVM, -2, "centerOfMass" );

        lua_pushnumber ( luaVM, pEntry->GetPercentSubmerged() );
        lua_setfield ( luaVM, -2, "percentSubmerged" );

        lua_pushnumber ( luaVM, pEntry->GetTractionMultiplier() );
        lua_setfield ( luaVM, -2, "tractionMultiplier" );

        CHandlingEntry::eDriveType eDriveType = pEntry->GetCarDriveType ();
        if ( eDriveType == CHandlingEntry::FWD )
            lua_pushstring ( luaVM, "fwd" );
        else if ( eDriveType==CHandlingEntry::RWD )
            lua_pushstring ( luaVM, "rwd" );
        else if ( eDriveType==CHandlingEntry::FOURWHEEL )
            lua_pushstring ( luaVM, "awd" );
        else // What the ... (yeah, security)
            lua_pushnil ( luaVM );
        lua_setfield ( luaVM, -2, "driveType" );
        CHandlingEntry::eEngineType eEngineType = pEntry->GetCarEngineType ();
        if ( eEngineType == CHandlingEntry::PETROL )
            lua_pushstring ( luaVM, "petrol" );
        else if ( eEngineType == CHandlingEntry::DIESEL )
            lua_pushstring ( luaVM, "diesel" );
        else if ( eEngineType == CHandlingEntry::ELECTRIC )
            lua_pushstring ( luaVM, "electric" );
        else
            lua_pushnil ( luaVM );
        lua_setfield ( luaVM, -2, "engineType" );

        lua_pushnumber ( luaVM, pEntry->GetNumberOfGears () );
        lua_setfield ( luaVM, -2, "numberOfGears" );

        lua_pushnumber ( luaVM, pEntry->GetEngineAcceleration () );
        lua_setfield ( luaVM, -2, "engineAcceleration" );

        lua_pushnumber ( luaVM, pEntry->GetEngineInertia () );
        lua_setfield ( luaVM, -2, "engineInertia" );

        lua_pushnumber ( luaVM, pEntry->GetMaxVelocity () );
        lua_setfield ( luaVM, -2, "maxVelocity" );

        lua_pushnumber ( luaVM, pEntry->GetBrakeDeceleration () );
        lua_setfield ( luaVM, -2, "brakeDeceleration" );

        lua_pushnumber ( luaVM, pEntry->GetBrakeBias () );
        lua_setfield ( luaVM, -2, "brakeBias" );

        lua_pushboolean ( luaVM, pEntry->GetABS () );
        lua_setfield ( luaVM, -2, "ABS" );

        lua_pushnumber ( luaVM, pEntry->GetSteeringLock () );
        lua_setfield ( luaVM, -2, "steeringLock" );

        lua_pushnumber ( luaVM, pEntry->GetTractionLoss () );
        lua_setfield ( luaVM, -2, "tractionLoss" );

        lua_pushnumber ( luaVM, pEntry->GetTractionBias () );
        lua_setfield ( luaVM, -2, "tractionBias" );

        lua_pushnumber ( luaVM, pEntry->GetSuspensionForceLevel () );
        lua_setfield ( luaVM, -2, "suspensionForceLevel" );

        lua_pushnumber ( luaVM, pEntry->GetSuspensionDamping () );
        lua_setfield ( luaVM, -2, "suspensionDamping" );

        lua_pushnumber ( luaVM, pEntry->GetSuspensionHighSpeedDamping () );
        lua_setfield ( luaVM, -2, "suspensionHighSpeedDamping" );

        lua_pushnumber ( luaVM, pEntry->GetSuspensionUpperLimit () );
        lua_setfield ( luaVM, -2, "suspensionUpperLimit" );

        lua_pushnumber ( luaVM, pEntry->GetSuspensionLowerLimit () );
        lua_setfield ( luaVM, -2, "suspensionLowerLimit" );

        lua_pushnumber ( luaVM, pEntry->GetSuspensionFrontRearBias () );
        lua_setfield ( luaVM, -2, "suspensionFrontRearBias" );

        lua_pushnumber ( luaVM, pEntry->GetSuspensionAntiDiveMultiplier () );
        lua_setfield ( luaVM, -2, "suspensionAntiDiveMultiplier" );

        lua_pushnumber ( luaVM, pEntry->GetCollisionDamageMultiplier () );
        lua_setfield ( luaVM, -2, "collisionDamageMultiplier" );

        lua_pushnumber ( luaVM, pEntry->GetSeatOffsetDistance () );
        lua_setfield ( luaVM, -2, "seatOffsetDistance" );

        lua_pushnumber ( luaVM, pEntry->GetHandlingFlags () );
        lua_setfield ( luaVM, -2, "handlingFlags" );

        lua_pushnumber ( luaVM, pEntry->GetModelFlags () );
        lua_setfield ( luaVM, -2, "modelFlags" );

        lua_pushnumber ( luaVM, pEntry->GetMonetary () );
        lua_setfield ( luaVM, -2, "monetary" );

        CHandlingEntry::eLightType eHeadType = pEntry->GetHeadLight ();
        if ( eHeadType == CHandlingEntry::LONG )
            lua_pushstring ( luaVM, "long" );
        else if ( eHeadType == CHandlingEntry::SMALL )
            lua_pushstring ( luaVM, "small" );
        else if ( eHeadType == CHandlingEntry::BIG )
            lua_pushstring ( luaVM, "big" );
        else
            lua_pushnil( luaVM );
        lua_setfield ( luaVM, -2, "headLight" );

        CHandlingEntry::eLightType eTailType = pEntry->GetTailLight ();
        if ( eTailType == CHandlingEntry::LONG )
            lua_pushstring ( luaVM, "long" );
        else if ( eTailType == CHandlingEntry::SMALL )
            lua_pushstring ( luaVM, "small" );
        else if ( eTailType == CHandlingEntry::BIG )
            lua_pushstring ( luaVM, "big" );
        else
            lua_pushnil( luaVM );
        lua_setfield ( luaVM, -2, "tailLight" );

        lua_pushnumber ( luaVM, pEntry->GetAnimGroup () );
        lua_setfield ( luaVM, -2, "animGroup" );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetOriginalHandling ( lua_State* luaVM )
{
    int iType = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( iType );

    if ( !argStream.HasErrors ( ) ) 
    {
        eVehicleTypes eModel = static_cast < eVehicleTypes > ( iType );
        if ( eModel )
        {
            const CHandlingEntry* pEntry = g_pGame->GetHandlingManager()->GetOriginalHandlingData( eModel );
            if ( pEntry )
            {
                lua_newtable ( luaVM );
                lua_pushnumber ( luaVM, pEntry->GetMass() );
                lua_setfield ( luaVM, -2, "mass" );
                lua_pushnumber ( luaVM, pEntry->GetTurnMass() );
                lua_setfield ( luaVM, -2, "turnMass" );
                lua_pushnumber ( luaVM, pEntry->GetDragCoeff() );
                lua_setfield ( luaVM, -2, "dragCoeff" );
                lua_createtable ( luaVM, 3, 0 );
                CVector vecCenter = pEntry->GetCenterOfMass ();
                lua_pushnumber ( luaVM, 1 );
                lua_pushnumber ( luaVM, vecCenter.fX );
                lua_settable ( luaVM, -3 );
                lua_pushnumber ( luaVM, 2 );
                lua_pushnumber ( luaVM, vecCenter.fY );
                lua_settable ( luaVM, -3 );
                lua_pushnumber ( luaVM, 3 );
                lua_pushnumber ( luaVM, vecCenter.fZ );
                lua_settable ( luaVM, -3 );
                lua_setfield ( luaVM, -2, "centerOfMass" );
                lua_pushnumber ( luaVM, pEntry->GetPercentSubmerged() );
                lua_setfield ( luaVM, -2, "percentSubmerged" );
                lua_pushnumber ( luaVM, pEntry->GetTractionMultiplier() );
                lua_setfield ( luaVM, -2, "tractionMultiplier" );
                CHandlingEntry::eDriveType eDriveType=pEntry->GetCarDriveType();
                if (eDriveType==CHandlingEntry::FWD)
                    lua_pushstring(luaVM,"fwd");
                else if (eDriveType==CHandlingEntry::RWD)
                    lua_pushstring(luaVM,"rwd");
                else if (eDriveType==CHandlingEntry::FOURWHEEL)
                    lua_pushstring(luaVM,"awd");
                else // What the ... (yeah, security)
                    lua_pushnil ( luaVM );
                lua_setfield ( luaVM, -2, "driveType" );
                CHandlingEntry::eEngineType eEngineType=pEntry->GetCarEngineType();
                if (eEngineType==CHandlingEntry::PETROL)
                    lua_pushstring(luaVM,"petrol");
                else if (eEngineType==CHandlingEntry::DIESEL)
                    lua_pushstring(luaVM,"diesel");
                else if (eEngineType==CHandlingEntry::ELECTRIC)
                    lua_pushstring(luaVM,"electric");
                else
                    lua_pushnil ( luaVM );
                lua_setfield ( luaVM, -2, "engineType" );
                lua_pushnumber ( luaVM, pEntry->GetNumberOfGears() );
                lua_setfield ( luaVM, -2, "numberOfGears" );
                lua_pushnumber ( luaVM, pEntry->GetEngineAcceleration() );
                lua_setfield ( luaVM, -2, "engineAcceleration" );
                lua_pushnumber ( luaVM, pEntry->GetEngineInertia() );
                lua_setfield ( luaVM, -2, "engineInertia" );
                lua_pushnumber ( luaVM, pEntry->GetMaxVelocity() );
                lua_setfield ( luaVM, -2, "maxVelocity" );
                lua_pushnumber ( luaVM, pEntry->GetBrakeDeceleration() );
                lua_setfield ( luaVM, -2, "brakeDeceleration" );
                lua_pushnumber ( luaVM, pEntry->GetBrakeBias() );
                lua_setfield ( luaVM, -2, "brakeBias" );
                lua_pushboolean ( luaVM, pEntry->GetABS() );
                lua_setfield ( luaVM, -2, "ABS" );
                lua_pushnumber ( luaVM, pEntry->GetSteeringLock() );
                lua_setfield ( luaVM, -2, "steeringLock" );
                lua_pushnumber ( luaVM, pEntry->GetTractionLoss() );
                lua_setfield ( luaVM, -2, "tractionLoss" );
                lua_pushnumber ( luaVM, pEntry->GetTractionBias() );
                lua_setfield ( luaVM, -2, "tractionBias" );
                lua_pushnumber ( luaVM, pEntry->GetSuspensionForceLevel() );
                lua_setfield ( luaVM, -2, "suspensionForceLevel" );
                lua_pushnumber ( luaVM, pEntry->GetSuspensionDamping() );
                lua_setfield ( luaVM, -2, "suspensionDamping" );
                lua_pushnumber ( luaVM, pEntry->GetSuspensionHighSpeedDamping() );
                lua_setfield ( luaVM, -2, "suspensionHighSpeedDamping" );
                lua_pushnumber ( luaVM, pEntry->GetSuspensionUpperLimit() );
                lua_setfield ( luaVM, -2, "suspensionUpperLimit" );
                lua_pushnumber ( luaVM, pEntry->GetSuspensionLowerLimit() );
                lua_setfield ( luaVM, -2, "suspensionLowerLimit" );
                lua_pushnumber ( luaVM, pEntry->GetSuspensionFrontRearBias() );
                lua_setfield ( luaVM, -2, "suspensionFrontRearBias" );
                lua_pushnumber ( luaVM, pEntry->GetSuspensionAntiDiveMultiplier() );
                lua_setfield ( luaVM, -2, "suspensionAntiDiveMultiplier" );
                lua_pushnumber ( luaVM, pEntry->GetCollisionDamageMultiplier() );
                lua_setfield ( luaVM, -2, "collisionDamageMultiplier" );
                lua_pushnumber ( luaVM, pEntry->GetSeatOffsetDistance() );
                lua_setfield ( luaVM, -2, "seatOffsetDistance" );
                lua_pushnumber ( luaVM, pEntry->GetHandlingFlags() );
                lua_setfield ( luaVM, -2, "handlingFlags" );
                lua_pushnumber ( luaVM, pEntry->GetModelFlags() );
                lua_setfield ( luaVM, -2, "modelFlags" );
                lua_pushnumber ( luaVM, pEntry->GetMonetary() );
                lua_setfield ( luaVM, -2, "monetary" );
                CHandlingEntry::eLightType eHeadType=pEntry->GetHeadLight();
                if (eHeadType==CHandlingEntry::LONG)
                    lua_pushstring(luaVM,"long");
                else if (eHeadType==CHandlingEntry::SMALL)
                    lua_pushstring(luaVM,"small");
                else if (eHeadType==CHandlingEntry::BIG)
                    lua_pushstring(luaVM,"big");
                else
                    lua_pushnil( luaVM );
                lua_setfield ( luaVM, -2, "headLight" );
                CHandlingEntry::eLightType eTailType=pEntry->GetTailLight();
                if (eTailType==CHandlingEntry::LONG)
                    lua_pushstring(luaVM,"long");
                else if (eTailType==CHandlingEntry::SMALL)
                    lua_pushstring(luaVM,"small");
                else if (eTailType==CHandlingEntry::BIG)
                    lua_pushstring(luaVM,"big");
                else
                    lua_pushnil( luaVM );
                lua_setfield ( luaVM, -2, "tailLight" );
                lua_pushnumber ( luaVM, pEntry->GetAnimGroup() );
                lua_setfield ( luaVM, -2, "animGroup" );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( luaVM, "model", 1 );
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "model", 1 );
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleDoorOpenRatio ( lua_State* luaVM )
{
    CClientEntity * pEntity = NULL;
    unsigned char ucDoor = 0;
    float fRatio = 0.0f;
    unsigned long ulTime = 0UL;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( ucDoor );
    argStream.ReadNumber ( fRatio );
    argStream.ReadNumber ( ulTime, 0UL );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( CStaticFunctionDefinitions::SetVehicleDoorOpenRatio ( *pEntity, ucDoor, fRatio, ulTime ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleSirenParams( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    CClientVehicle* pVehicle = NULL;
    unsigned char ucSirenID = 0;
    SSirenInfo tSirenInfo;

    argStream.ReadUserData ( pVehicle );
    if ( !argStream.HasErrors ( ) )
    {
        tSirenInfo = pVehicle->m_tSirenBeaconInfo;// Grab the siren structure data
        lua_newtable ( luaVM );

        lua_pushstring( luaVM, "SirenCount" );
        lua_pushnumber ( luaVM, tSirenInfo.m_ucSirenCount );
        lua_settable ( luaVM, -3 ); // End of SirenCount Property

        lua_pushstring( luaVM, "SirenType" );
        lua_pushnumber ( luaVM, tSirenInfo.m_ucSirenType );
        lua_settable ( luaVM, -3 ); // End of SirenType Property

        lua_pushstring( luaVM, "Flags" );
        lua_newtable ( luaVM );

        lua_pushstring ( luaVM, "360" );
        lua_pushboolean ( luaVM, tSirenInfo.m_b360Flag );
        lua_settable ( luaVM, -3 ); // End of 360 Property

        lua_pushstring ( luaVM, "DoLOSCheck" );
        lua_pushboolean ( luaVM, tSirenInfo.m_bDoLOSCheck );
        lua_settable ( luaVM, -3 ); // End of DoLOSCheck Property

        lua_pushstring ( luaVM, "UseRandomiser" );
        lua_pushboolean ( luaVM, tSirenInfo.m_bUseRandomiser );
        lua_settable ( luaVM, -3 ); // End of UseRandomiser Property

        lua_pushstring ( luaVM, "Silent" );
        lua_pushboolean ( luaVM, tSirenInfo.m_bSirenSilent );
        lua_settable ( luaVM, -3 ); // End of Silent Property

        lua_settable ( luaVM, -3 ); // End of table

        return 1;
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleSirens( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    CClientVehicle* pVehicle = NULL;
    unsigned char ucSirenID = 0;
    SSirenInfo tSirenInfo;

    argStream.ReadUserData ( pVehicle );
    if ( !argStream.HasErrors ( ) )
    {
        tSirenInfo = pVehicle->m_tSirenBeaconInfo;// Grab the siren structure data
        lua_newtable ( luaVM );

        for ( int i = 0; i < tSirenInfo.m_ucSirenCount;i++ )
        {
            lua_pushnumber ( luaVM, i+1 );
            lua_newtable ( luaVM );

            lua_pushstring( luaVM, "Min_Alpha" );
            lua_pushnumber ( luaVM, tSirenInfo.m_tSirenInfo[i].m_dwMinSirenAlpha );
            lua_settable ( luaVM, -3 ); // End of Min_Alpha property

            lua_pushstring( luaVM, "Red" );
            lua_pushnumber ( luaVM, tSirenInfo.m_tSirenInfo[i].m_RGBBeaconColour.R );
            lua_settable ( luaVM, -3 ); // End of Red property

            lua_pushstring( luaVM, "Green" );
            lua_pushnumber ( luaVM, tSirenInfo.m_tSirenInfo[i].m_RGBBeaconColour.G );
            lua_settable ( luaVM, -3 ); // End of Green property

            lua_pushstring( luaVM, "Blue" );
            lua_pushnumber ( luaVM, tSirenInfo.m_tSirenInfo[i].m_RGBBeaconColour.B );
            lua_settable ( luaVM, -3 ); // End of Blue property

            lua_pushstring( luaVM, "Alpha" );
            lua_pushnumber ( luaVM, tSirenInfo.m_tSirenInfo[i].m_RGBBeaconColour.A );
            lua_settable ( luaVM, -3 ); // End of Alpha property

            lua_pushstring( luaVM, "x" );
            lua_pushnumber ( luaVM, tSirenInfo.m_tSirenInfo[i].m_vecSirenPositions.fX );
            lua_settable ( luaVM, -3 ); // End of X property

            lua_pushstring( luaVM, "y" );
            lua_pushnumber ( luaVM, tSirenInfo.m_tSirenInfo[i].m_vecSirenPositions.fY );
            lua_settable ( luaVM, -3 ); // End of Y property

            lua_pushstring( luaVM, "z" );
            lua_pushnumber ( luaVM, tSirenInfo.m_tSirenInfo[i].m_vecSirenPositions.fZ );
            lua_settable ( luaVM, -3 ); // End of Z property

            lua_settable ( luaVM, -3 ); // End of Table
        }

        return 1;
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetVehicleSirens ( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    CClientVehicle* pVehicle = NULL;
    unsigned char ucSirenID = 0;
    SSirenInfo tSirenInfo;

    argStream.ReadUserData ( pVehicle );
    argStream.ReadNumber ( ucSirenID );

    if ( !argStream.HasErrors () )
    {
        if ( ucSirenID > 0 && ucSirenID < 9 )
        {
            // Array indicies start at 0 so compensate here. This way all code works properly and we get nice 1-8 numbers for API
            ucSirenID--;
            argStream.ReadVector3D ( tSirenInfo.m_tSirenInfo[ ucSirenID ].m_vecSirenPositions );
            argStream.ReadNumber ( tSirenInfo.m_tSirenInfo[ ucSirenID ].m_RGBBeaconColour.R );
            argStream.ReadNumber ( tSirenInfo.m_tSirenInfo[ ucSirenID ].m_RGBBeaconColour.G );
            argStream.ReadNumber ( tSirenInfo.m_tSirenInfo[ ucSirenID ].m_RGBBeaconColour.B );
            argStream.ReadNumber ( tSirenInfo.m_tSirenInfo[ ucSirenID ].m_RGBBeaconColour.A, 255 );
            argStream.ReadNumber ( tSirenInfo.m_tSirenInfo[ ucSirenID ].m_dwMinSirenAlpha, 0 );
       
            if ( !argStream.HasErrors () )
            {
                if ( CStaticFunctionDefinitions::SetVehicleSirens ( *pVehicle, ucSirenID, tSirenInfo ) )
                {
                    lua_pushboolean ( luaVM, true );
                    return 1;
                }
            }
        }
    }

    if ( argStream.HasErrors () )
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleDoorOpenRatio ( lua_State* luaVM )
{
    CClientVehicle * pVehicle = NULL;
    unsigned char ucDoor = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadNumber ( ucDoor );

    if ( !argStream.HasErrors ( ) ) 
    {
        if ( ucDoor <= 5 )
        {
            lua_pushnumber ( luaVM, pVehicle->GetDoorOpenRatio ( ucDoor ) );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehicleComponentPosition ( lua_State* luaVM )
{
// bool setVehicleComponentPosition ( vehicle theVehicle, string theComponent, float posX, float posY, float posZ [, string base = "root"] )
    SString strComponent;
    CClientVehicle * pVehicle = NULL;
    CVector vecPosition;
    EComponentBaseType inputBase;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadString ( strComponent );
    argStream.ReadVector3D ( vecPosition );
    argStream.ReadEnumString ( inputBase, EComponentBase::ROOT );

    if ( !argStream.HasErrors() )
    {
        if ( pVehicle->SetComponentPosition ( strComponent, vecPosition, inputBase ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleComponentPosition ( lua_State* luaVM )
{
// float, float, float getVehicleComponentPosition ( vehicle theVehicle, string theComponent [, string base = "root"] )
    SString strComponent;
    CClientVehicle * pVehicle = NULL;
    EComponentBaseType outputBase;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadString ( strComponent );
    argStream.ReadEnumString ( outputBase, EComponentBase::ROOT );

    if ( !argStream.HasErrors() )
    {
        CVector vecPosition;
        if ( pVehicle->GetComponentPosition ( strComponent, vecPosition, outputBase ) )
        {
            lua_pushnumber ( luaVM, vecPosition.fX );
            lua_pushnumber ( luaVM, vecPosition.fY );
            lua_pushnumber ( luaVM, vecPosition.fZ );
            return 3;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetVehicleComponentRotation ( lua_State* luaVM )
{
//  bool setVehicleComponentRotation ( vehicle theVehicle, string theComponent, float rotX, float rotY, float rotZ [, string base = "parent"] )
    SString strComponent;
    CClientVehicle * pVehicle = NULL;
    CVector vecRotation;
    EComponentBaseType inputBase;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadString ( strComponent );
    argStream.ReadVector3D ( vecRotation );
    argStream.ReadEnumString ( inputBase, EComponentBase::PARENT );

    if ( !argStream.HasErrors() )
    {
        // Script uses degrees
        ConvertDegreesToRadians( vecRotation );
        pVehicle->SetComponentRotation ( strComponent, vecRotation, inputBase );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleComponentRotation ( lua_State* luaVM )
{
// float, float, float getVehicleComponentRotation ( vehicle theVehicle, string theComponent [, string base = "parent"]  )
    SString strComponent;
    CClientVehicle * pVehicle = NULL;
    EComponentBaseType outputBase;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadString ( strComponent );
    argStream.ReadEnumString ( outputBase, EComponentBase::PARENT );
  
    if ( !argStream.HasErrors() )
    {
        CVector vecRotation;
        if ( pVehicle->GetComponentRotation ( strComponent, vecRotation, outputBase ) )
        {
            // Script uses degrees
            ConvertRadiansToDegrees( vecRotation );
            lua_pushnumber ( luaVM, vecRotation.fX );
            lua_pushnumber ( luaVM, vecRotation.fY );
            lua_pushnumber ( luaVM, vecRotation.fZ );
            return 3;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::ResetVehicleComponentPosition ( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    SString strComponent;
    CClientVehicle * pVehicle = NULL;
    argStream.ReadUserData ( pVehicle );
    argStream.ReadString ( strComponent );
    
    if ( !argStream.HasErrors() )
    {
        if ( pVehicle->ResetComponentPosition ( strComponent ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::ResetVehicleComponentRotation ( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    SString strComponent;
    CClientVehicle * pVehicle = NULL;
    argStream.ReadUserData ( pVehicle );
    argStream.ReadString ( strComponent );
    
    if ( !argStream.HasErrors() )
    {
        if ( pVehicle->ResetComponentRotation ( strComponent ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetVehicleComponentVisible ( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    SString strComponent;
    CClientVehicle * pVehicle = NULL;
    bool bVisible = false;
    argStream.ReadUserData ( pVehicle );
    argStream.ReadString ( strComponent );
    argStream.ReadBool ( bVisible );

    if ( !argStream.HasErrors() )
    {
        if ( pVehicle->SetComponentVisible ( strComponent, bVisible ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleComponentVisible ( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    SString strComponent;
    CClientVehicle * pVehicle = NULL;
    bool bVisible = false;
    argStream.ReadUserData ( pVehicle );
    argStream.ReadString ( strComponent );
    
    if ( !argStream.HasErrors() )
    {
        if ( pVehicle->GetComponentVisible ( strComponent, bVisible ) )
        {
            lua_pushboolean ( luaVM, bVisible );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetVehicleComponents ( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    SString strComponent;
    CClientVehicle * pVehicle = NULL;

    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors() )
    {
        std::map < SString, SVehicleComponentData > ::iterator iter = pVehicle->ComponentsBegin ( );
        lua_newtable ( luaVM );
        for ( ;iter != pVehicle->ComponentsEnd ( ); iter++ )
        {
            lua_pushstring( luaVM, (*iter).first );
            lua_pushboolean( luaVM, (*iter).second.m_bVisible );
            lua_settable ( luaVM, -3 ); // End of Table
        }
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::IsVehicleNitroRecharging ( lua_State* luaVM )
{
//  bool isVehicleNitroRecharging ( vehicle theVehicle )
    CClientVehicle * pVehicle = NULL;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors() )
    {
        bool bRecharging;

        if ( CStaticFunctionDefinitions::IsVehicleNitroRecharging ( *pVehicle, bRecharging ) )
        {
            lua_pushboolean ( luaVM, bRecharging );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );
    
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::IsVehicleNitroActivated ( lua_State* luaVM )
{
//  bool isVehicleNitroActivated ( vehicle theVehicle )
    CClientVehicle * pVehicle = NULL;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors() )
    {
        bool bActivated;

        if ( CStaticFunctionDefinitions::IsVehicleNitroActivated ( *pVehicle, bActivated ) )
        {
            lua_pushboolean ( luaVM, bActivated );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );
    
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleNitroCount ( lua_State* luaVM )
{
//  int getVehicleNitroCount ( vehicle theVehicle )
    CClientVehicle * pVehicle = NULL;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors() )
    {
        char cNitroCount;

        if ( CStaticFunctionDefinitions::GetVehicleNitroCount ( *pVehicle, cNitroCount ) )
        {
            lua_pushnumber ( luaVM, cNitroCount );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );
    
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::GetVehicleNitroLevel ( lua_State* luaVM )
{
//  float getVehicleNitroLevel ( vehicle theVehicle )
    CClientVehicle * pVehicle = NULL;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );

    if ( !argStream.HasErrors() )
    {
        float fNitroLevel;

        if ( CStaticFunctionDefinitions::GetVehicleNitroLevel ( *pVehicle, fNitroLevel ) )
        {
            lua_pushnumber ( luaVM, fNitroLevel );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetVehicleNitroActivated ( lua_State* luaVM )
{
//  bool setVehicleNitroActivated ( vehicle theVehicle, bool bActivated )
    CClientEntity * pEntity = NULL; bool bActivated;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadBool ( bActivated );

    if ( !argStream.HasErrors() )
    {
        if ( CStaticFunctionDefinitions::SetVehicleNitroActivated ( *pEntity, bActivated ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetVehicleNitroCount ( lua_State* luaVM )
{
//  bool setVehicleNitroCount ( vehicle theVehicle, int iCount )
    CClientEntity * pEntity = NULL; char cCount;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( cCount );

    if ( !argStream.HasErrors() )
    {
        if ( CStaticFunctionDefinitions::SetVehicleNitroCount ( *pEntity, cCount ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetVehicleNitroLevel ( lua_State* luaVM )
{
//  bool setVehicleNitroLevel ( vehicle theVehicle, float fLevel )
    CClientEntity * pEntity = NULL; float fLevel;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadNumber ( fLevel );

    if ( !argStream.HasErrors() )
    {
        if ( CStaticFunctionDefinitions::SetVehicleNitroLevel ( *pEntity, fLevel ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetVehiclePlateText ( lua_State* luaVM )
{
//  bool setVehiclePlateText ( vehicle theVehicle, string plateText )
    CClientEntity* pEntity; SString strText;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pEntity );
    argStream.ReadString ( strText );

    if ( !argStream.HasErrors() )
    {
        if ( CStaticFunctionDefinitions::SetVehiclePlateText ( *pEntity, strText ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaFunctionDefs::SetHeliBladeCollisionsEnabled ( lua_State* luaVM )
{
//  bool setHeliBladeCollisionsEnabled ( vehicle theVehicle, bool bEnabled )
    CClientVehicle* pVehicle; 
    bool bEnabled = true;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadBool ( bEnabled );

    if ( !argStream.HasErrors() )
    {
        if ( CStaticFunctionDefinitions::SetHeliBladeCollisionsEnabled ( *pVehicle, bEnabled ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetHeliBladeCollisionsEnabled ( lua_State* luaVM )
{
//  bool getHeliBladeCollisionsEnabled ( vehicle theVehicle )
    CClientVehicle* pVehicle; 
    bool bEnabled = true;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pVehicle );
    argStream.ReadBool ( bEnabled );

    if ( !argStream.HasErrors() )
    {
        if ( CStaticFunctionDefinitions::GetHeliBladeCollisionsEnabled ( *pVehicle ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    lua_pushboolean ( luaVM, false );
    return 1;
}
