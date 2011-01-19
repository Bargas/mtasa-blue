/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/luadefs/CLuaWorldDefs.cpp
*  PURPOSE:     Lua game world function definitions class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               lil_Toady <>
*               Kevin Whiteside <>
*               Sebas Lamers <sebasdevelopment@gmx.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

void CLuaWorldDefs::LoadFunctions ( void )
{
    // Get
    CLuaCFunctions::AddFunction ( "getTime", CLuaWorldDefs::getTime );
    CLuaCFunctions::AddFunction ( "getWeather", CLuaWorldDefs::getWeather );
    CLuaCFunctions::AddFunction ( "getZoneName", CLuaWorldDefs::getZoneName );
    CLuaCFunctions::AddFunction ( "getGravity", CLuaWorldDefs::getGravity );
    CLuaCFunctions::AddFunction ( "getGameSpeed", CLuaWorldDefs::getGameSpeed );
    CLuaCFunctions::AddFunction ( "getWaveHeight", CLuaWorldDefs::getWaveHeight );
    CLuaCFunctions::AddFunction ( "getFPSLimit", CLuaWorldDefs::getFPSLimit );
    CLuaCFunctions::AddFunction ( "getMinuteDuration", CLuaWorldDefs::getMinuteDuration );
    CLuaCFunctions::AddFunction ( "isGarageOpen", CLuaWorldDefs::isGarageOpen );
    CLuaCFunctions::AddFunction ( "getTrafficLightState", CLuaWorldDefs::getTrafficLightState );
    CLuaCFunctions::AddFunction ( "areTrafficLightsLocked", CLuaWorldDefs::areTrafficLightsLocked );
    CLuaCFunctions::AddFunction ( "getJetpackMaxHeight", CLuaWorldDefs::getJetpackMaxHeight );

    // Set
    CLuaCFunctions::AddFunction ( "setTime", CLuaWorldDefs::setTime );
    CLuaCFunctions::AddFunction ( "setWeather", CLuaWorldDefs::setWeather );
    CLuaCFunctions::AddFunction ( "setWeatherBlended", CLuaWorldDefs::setWeatherBlended );
    CLuaCFunctions::AddFunction ( "setGravity", CLuaWorldDefs::setGravity );
    CLuaCFunctions::AddFunction ( "setGameSpeed", CLuaWorldDefs::setGameSpeed );
    CLuaCFunctions::AddFunction ( "setWaveHeight", CLuaWorldDefs::setWaveHeight );
    CLuaCFunctions::AddFunction ( "getSkyGradient", CLuaWorldDefs::getSkyGradient );
    CLuaCFunctions::AddFunction ( "setSkyGradient", CLuaWorldDefs::setSkyGradient );
    CLuaCFunctions::AddFunction ( "resetSkyGradient", CLuaWorldDefs::resetSkyGradient );
    CLuaCFunctions::AddFunction ( "setFPSLimit", CLuaWorldDefs::setFPSLimit );
    CLuaCFunctions::AddFunction ( "setMinuteDuration", CLuaWorldDefs::setMinuteDuration );
    CLuaCFunctions::AddFunction ( "setGarageOpen", CLuaWorldDefs::setGarageOpen );
    CLuaCFunctions::AddFunction ( "setGlitchEnabled", CLuaWorldDefs::setGlitchEnabled );
    CLuaCFunctions::AddFunction ( "isGlitchEnabled", CLuaWorldDefs::isGlitchEnabled );
    CLuaCFunctions::AddFunction ( "setCloudsEnabled", CLuaWorldDefs::setCloudsEnabled );
    CLuaCFunctions::AddFunction ( "getCloudsEnabled", CLuaWorldDefs::getCloudsEnabled );
    CLuaCFunctions::AddFunction ( "setHeatHazeEnabled", CLuaWorldDefs::setHeatHazeEnabled );
    CLuaCFunctions::AddFunction ( "getHeatHazeEnabled", CLuaWorldDefs::getHeatHazeEnabled );
    CLuaCFunctions::AddFunction ( "setTrafficLightState", CLuaWorldDefs::setTrafficLightState );
    CLuaCFunctions::AddFunction ( "setTrafficLightsLocked", CLuaWorldDefs::setTrafficLightsLocked );
    CLuaCFunctions::AddFunction ( "setJetpackMaxHeight", CLuaWorldDefs::setJetpackMaxHeight );
}


int CLuaWorldDefs::getTime ( lua_State* luaVM )
{
    // Get the time
    unsigned char ucHour, ucMinute;
    if ( CStaticFunctionDefinitions::GetTime ( ucHour, ucMinute ) )
    {
        // Return it
        lua_pushnumber ( luaVM, static_cast < lua_Number > ( ucHour ) );
        lua_pushnumber ( luaVM, static_cast < lua_Number > ( ucMinute ) );
        return 2;
    }

    // Return false
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::getWeather ( lua_State* luaVM )
{
    unsigned char ucWeather, ucWeatherBlendingTo;
    if ( CStaticFunctionDefinitions::GetWeather ( ucWeather, ucWeatherBlendingTo ) )
    {
        lua_pushnumber ( luaVM, static_cast < lua_Number > ( ucWeather ) );

        if ( ucWeatherBlendingTo != 0xFF )
            lua_pushnumber ( luaVM, static_cast < lua_Number > ( ucWeatherBlendingTo ) );
        else
            lua_pushnil ( luaVM );

        return 2;
    }

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::getZoneName ( lua_State* luaVM )
{
    int iArgument1 = lua_type ( luaVM, 1 );
    int iArgument2 = lua_type ( luaVM, 2 );
    int iArgument3 = lua_type ( luaVM, 3 );
    if ( ( iArgument1 == LUA_TSTRING || iArgument1 == LUA_TNUMBER ) &&
         ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER ) &&
         ( iArgument3 == LUA_TSTRING || iArgument3 == LUA_TNUMBER ) )
    {
        CVector vecPosition ( static_cast < float > ( lua_tonumber ( luaVM, 1 ) ),
                              static_cast < float > ( lua_tonumber ( luaVM, 2 ) ),
                              static_cast < float > ( lua_tonumber ( luaVM, 3 ) ) );

        bool bCitiesOnly = false;
        if ( lua_type ( luaVM, 4 ) == LUA_TBOOLEAN )
            bCitiesOnly = ( lua_toboolean ( luaVM, 4 ) ) ? true:false;

        char szZoneName [ 128 ];
        if ( CStaticFunctionDefinitions::GetZoneName ( vecPosition, szZoneName, 128, bCitiesOnly ) )
        {
            lua_pushstring ( luaVM, szZoneName );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getZoneName" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::getGravity ( lua_State* luaVM )
{
    float fGravity;
    if ( CStaticFunctionDefinitions::GetGravity ( fGravity ) )
    {
        lua_pushnumber ( luaVM, fGravity );
        return 1;
    }
    
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::getGameSpeed ( lua_State* luaVM )
{
    float fSpeed;
    if ( CStaticFunctionDefinitions::GetGameSpeed ( fSpeed ) )
    {
        lua_pushnumber ( luaVM, fSpeed );
        return 1;
    }
    
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::getWaveHeight ( lua_State* luaVM )
{
    float fHeight;
    if ( CStaticFunctionDefinitions::GetWaveHeight ( fHeight ) )
    {
        lua_pushnumber ( luaVM, fHeight );
        return 1;
    }
    
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::getFPSLimit ( lua_State* luaVM )
{
    unsigned short usLimit;
    if ( CStaticFunctionDefinitions::GetFPSLimit ( usLimit ) )
    {
        lua_pushnumber ( luaVM, usLimit );
        return 1;
    }
    
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::getMinuteDuration ( lua_State* luaVM )
{
    unsigned long ulDuration;
    if ( CStaticFunctionDefinitions::GetMinuteDuration ( ulDuration ) )
    {
        lua_pushnumber ( luaVM, ulDuration );
        return 1;
    }
    
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::isGarageOpen ( lua_State* luaVM )
{
    if ( lua_type( luaVM, 1 ) == LUA_TNUMBER )
    {
        unsigned char ucGarageID = static_cast < unsigned char > ( lua_tonumber( luaVM, 1 ) );
        bool bIsOpen;

        if ( CStaticFunctionDefinitions::IsGarageOpen ( ucGarageID, bIsOpen ) )
        {
            lua_pushboolean ( luaVM, bIsOpen );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType ( luaVM, "isGarageOpen" );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isGarageOpen" );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::getTrafficLightState ( lua_State* luaVM )
{
    unsigned char ucTrafficLightState;
    if ( CStaticFunctionDefinitions::GetTrafficLightState ( ucTrafficLightState ) )
    {
        lua_pushnumber ( luaVM, ucTrafficLightState );
        return 1;
    }
    
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::areTrafficLightsLocked ( lua_State* luaVM )
{
    bool bTrafficLightsLocked;
    if ( CStaticFunctionDefinitions::GetTrafficLightsLocked ( bTrafficLightsLocked ) )
    {
        lua_pushboolean ( luaVM, bTrafficLightsLocked );
        return 1;
    }

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::setTime ( lua_State* luaVM )
{
    // Verify the argument types
    int iArgument1 = lua_type ( luaVM, 1 );
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
         ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
    {
        // Set the new time
        if ( CStaticFunctionDefinitions::SetTime ( static_cast < unsigned char > ( lua_tonumber ( luaVM, 1 ) ), static_cast < unsigned char > ( lua_tonumber ( luaVM, 2 ) ) ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setTime" );

    // Return false
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::setTrafficLightState ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TNUMBER )
    {
        unsigned char ucTrafficLightState = static_cast < unsigned char > ( lua_tonumber ( luaVM, 1 ) );

        if ( CStaticFunctionDefinitions::SetTrafficLightState ( ucTrafficLightState, true ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else if ( lua_type ( luaVM, 1 ) == LUA_TSTRING )
    {
        const char* szParam1 = lua_tostring ( luaVM, 1 );
        if ( ! stricmp ( szParam1, "auto" ) )
        {
            bool bOk = CStaticFunctionDefinitions::SetTrafficLightsLocked ( false ) &&
                       CStaticFunctionDefinitions::SetTrafficLightState ( 0 );
            lua_pushboolean ( luaVM, bOk );
            return 1;
        }
        else if ( ! stricmp ( szParam1, "disabled" ) )
        {
            bool bOk = CStaticFunctionDefinitions::SetTrafficLightsLocked ( true ) &&
                       CStaticFunctionDefinitions::SetTrafficLightState ( 9 );
            lua_pushboolean ( luaVM, bOk );
            return 1;
        }
        else
        {
            if ( lua_type ( luaVM, 2 ) == LUA_TSTRING )
            {
                // Perform a conversion from two string parameters to a state number.
                const char* szColorNS = szParam1;
                const char* szColorEW = lua_tostring ( luaVM, 2 );

                unsigned char ucState;
                if ( SharedUtil::GetTrafficLightStateFromStrings ( szColorNS, szColorEW, ucState ) )
                {
                    // Change it.
                    bool bOk = CStaticFunctionDefinitions::SetTrafficLightsLocked ( true ) &&
                               CStaticFunctionDefinitions::SetTrafficLightState ( ucState );
                    lua_pushboolean ( luaVM, bOk );
                    return 1;
                }
                else
                    m_pScriptDebugging->LogBadType ( luaVM, "setTrafficLightState" );
            }
            else
                m_pScriptDebugging->LogBadType ( luaVM, "setTrafficLightState" );
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setTrafficLightState" );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::setTrafficLightsLocked ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TBOOLEAN )
    {
        bool bTrafficLightsLocked = ( ( lua_toboolean( luaVM, 1 ) == 0 ) ? false : true );
        if ( CStaticFunctionDefinitions::SetTrafficLightsLocked ( bTrafficLightsLocked ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setTrafficLightsLocked" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::setJetpackMaxHeight ( lua_State* luaVM )
{
    int iArgument1 = lua_type ( luaVM, 1 );
    if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
    {
        float fMaxHeight = static_cast < float > ( lua_tonumber ( luaVM, 1 ) );
        if ( CStaticFunctionDefinitions::SetJetpackMaxHeight ( fMaxHeight ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setJetpackMaxHeight" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::setWeather ( lua_State* luaVM )
{
    // Verify the argument types
    int iArgument1 = lua_type ( luaVM, 1 );
    if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
    {
        // Set the new time
        if ( CStaticFunctionDefinitions::SetWeather ( static_cast < unsigned char > ( lua_tonumber ( luaVM, 1 ) ) ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setWeather" );

    // Return false
    lua_pushboolean ( luaVM, false );
    return 1;        
}


int CLuaWorldDefs::setWeatherBlended ( lua_State* luaVM )
{
    // Verify the argument types
    int iArgument1 = lua_type ( luaVM, 1 );
    if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
    {
        // Set the new time
        if ( CStaticFunctionDefinitions::SetWeatherBlended ( static_cast < unsigned char > ( lua_tonumber ( luaVM, 1 ) ) ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setWeatherBlended" );

    // Return false
    lua_pushboolean ( luaVM, false );
    return 1;        
}


int CLuaWorldDefs::setGravity ( lua_State* luaVM )
{
    int iArgument1 = lua_type ( luaVM, 1 );
    if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
    {
        float fGravity = static_cast < float > ( lua_tonumber ( luaVM, 1 ) );
        if ( CStaticFunctionDefinitions::SetGravity ( fGravity ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setGravity" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::setGameSpeed ( lua_State* luaVM )
{
    int iArgument1 = lua_type ( luaVM, 1 );
    if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
    {
        float fSpeed = static_cast < float > ( lua_tonumber ( luaVM, 1 ) );
        if ( CStaticFunctionDefinitions::SetGameSpeed ( fSpeed ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setGameSpeed" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::setWaveHeight ( lua_State* luaVM )
{
    int iArgument1 = lua_type ( luaVM, 1 );
    if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
    {
        float fHeight = static_cast < float > ( lua_tonumber ( luaVM, 1 ) );
        if ( CStaticFunctionDefinitions::SetWaveHeight ( fHeight ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setWaveHeight" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::getSkyGradient ( lua_State* luaVM )
{
    unsigned char ucTopR, ucTopG, ucTopB, ucBottomR, ucBottomG, ucBottomB;
    bool bSuccess = CStaticFunctionDefinitions::GetSkyGradient ( ucTopR, ucTopG, ucTopB, ucBottomR, ucBottomG, ucBottomB );

    if ( bSuccess )
    {
        lua_pushnumber ( luaVM, ucTopR );
        lua_pushnumber ( luaVM, ucTopG );
        lua_pushnumber ( luaVM, ucTopB );
        lua_pushnumber ( luaVM, ucBottomR );
        lua_pushnumber ( luaVM, ucBottomG );
        lua_pushnumber ( luaVM, ucBottomB );
        return 6;
    }

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::setSkyGradient ( lua_State* luaVM )
{
    // Verify the argument types
    int iArgument1 = lua_type ( luaVM, 1 );
    int iArgument2 = lua_type ( luaVM, 2 );
    int iArgument3 = lua_type ( luaVM, 3 );
    int iArgument4 = lua_type ( luaVM, 4 );
    int iArgument5 = lua_type ( luaVM, 5 );
    int iArgument6 = lua_type ( luaVM, 6 );

    unsigned char ucTopRed = 0;
    unsigned char ucTopGreen = 0;
    unsigned char ucTopBlue = 0;
    if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) )
        ucTopRed = static_cast < unsigned char > ( lua_tonumber ( luaVM, 1 ) );
    if ( ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        ucTopGreen = static_cast < unsigned char > ( lua_tonumber ( luaVM, 2 ) );
    if ( ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) )
        ucTopBlue = static_cast < unsigned char > ( lua_tonumber ( luaVM, 3 ) );
    unsigned char ucBottomRed = 0;
    unsigned char ucBottomGreen = 0;
    unsigned char ucBottomBlue = 0;
    if ( ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        ucBottomRed = static_cast < unsigned char > ( lua_tonumber ( luaVM, 4 ) );
    if ( ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING ) )
        ucBottomGreen = static_cast < unsigned char > ( lua_tonumber ( luaVM, 5 ) );
    if ( ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING ) )
        ucBottomBlue = static_cast < unsigned char > ( lua_tonumber ( luaVM, 6 ) );
        // Set the new sky gradient
    if ( CStaticFunctionDefinitions::SetSkyGradient ( ucTopRed, ucTopGreen, ucTopBlue, ucBottomRed, ucBottomGreen, ucBottomBlue ) )
    {
        lua_pushboolean ( luaVM, true );
        return 1;
    }

    // Return false
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::resetSkyGradient ( lua_State* luaVM )
{
    if ( CStaticFunctionDefinitions::ResetSkyGradient () )
    {
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::setFPSLimit ( lua_State* luaVM )
{
    int iArgument1 = lua_type ( luaVM, 1 );
    if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING || iArgument1 == LUA_TBOOLEAN )
    {
        unsigned short usLimit = static_cast < unsigned short > ( lua_tonumber ( luaVM, 1 ) );
        if ( CStaticFunctionDefinitions::SetFPSLimit ( usLimit ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setFPSLimit" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaWorldDefs::setMinuteDuration ( lua_State* luaVM )
{
    int iArgument1 = lua_type ( luaVM, 1 );
    if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
    {
        unsigned long ulDuration = static_cast < unsigned long > ( lua_tonumber ( luaVM, 1 ) );
        if ( CStaticFunctionDefinitions::SetMinuteDuration ( ulDuration ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::setGarageOpen ( lua_State* luaVM )
{
    if ( lua_type( luaVM, 1 ) == LUA_TNUMBER && lua_type ( luaVM, 2 ) == LUA_TBOOLEAN )
    {
        unsigned char ucGarageID = static_cast < unsigned char > ( lua_tonumber( luaVM, 1 ) );
        bool bIsOpen = ( ( lua_toboolean( luaVM, 2 ) == 0 ) ? false : true );

        if ( CStaticFunctionDefinitions::SetGarageOpen ( ucGarageID, bIsOpen ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType ( luaVM, "setGarageOpen" );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setGarageOpen" );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::setGlitchEnabled ( lua_State* luaVM )
{
    int iArgument = lua_type ( luaVM, 1 );
    if ( iArgument == LUA_TSTRING )
    {
        iArgument = lua_type ( luaVM, 2 );
        if ( iArgument == LUA_TBOOLEAN )
        {
            std::string szGlitchName = lua_tostring ( luaVM, 1 );
            bool bEnabled = (lua_toboolean ( luaVM, 2 ) == 1);
            if ( CStaticFunctionDefinitions::SetGlitchEnabled ( szGlitchName, bEnabled ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
        {
            m_pScriptDebugging->LogBadType ( luaVM, "setGlitchEnabled" );
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setGlitchEnabled" );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::isGlitchEnabled ( lua_State* luaVM )
{
    int iArgument = lua_type ( luaVM, 1 );
    if ( iArgument == LUA_TSTRING )
    {
        std::string szGlitchName = lua_tostring ( luaVM, 1 );
        bool bEnabled;
        if ( CStaticFunctionDefinitions::IsGlitchEnabled ( szGlitchName, bEnabled ) )
        {
            lua_pushboolean ( luaVM, bEnabled );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "isGlitchEnabled" );

    lua_pushnil ( luaVM );
    return 1;
}

int CLuaWorldDefs::setCloudsEnabled ( lua_State* luaVM )
{
    int iArgument = lua_type ( luaVM, 1 );
    if ( iArgument == LUA_TBOOLEAN )
    {
        bool bEnabled = lua_toboolean ( luaVM, 1 ) ? true : false;
        if ( CStaticFunctionDefinitions::SetCloudsEnabled ( bEnabled ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setCloudsEnabled" );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::getCloudsEnabled ( lua_State* luaVM )
{
     lua_pushboolean ( luaVM, CStaticFunctionDefinitions::GetCloudsEnabled ( ) );
     return 1;
}

int CLuaWorldDefs::setHeatHazeEnabled ( lua_State* luaVM )
{
    int iArgument = lua_type ( luaVM, 1 );
    if ( iArgument == LUA_TBOOLEAN || iArgument == LUA_TNIL || iArgument == LUA_TNONE )
    {
        int iEnabled = 2;
        if ( iArgument == LUA_TBOOLEAN )
            iEnabled = lua_toboolean ( luaVM, 1 ) ? 1 : 0;
        if ( CStaticFunctionDefinitions::SetHeatHazeEnabled ( iEnabled ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setHeatHazeEnabled" );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaWorldDefs::getHeatHazeEnabled ( lua_State* luaVM )
{
    int iEnabled = CStaticFunctionDefinitions::GetHeatHazeEnabled ();
    if ( iEnabled == 2 )
        lua_pushnil ( luaVM );
    else
        lua_pushboolean ( luaVM, iEnabled ? true : false );
    return 1;
}

int CLuaWorldDefs::getJetpackMaxHeight ( lua_State* luaVM )
{
    float fMaxHeight;
    if ( CStaticFunctionDefinitions::GetJetpackMaxHeight ( fMaxHeight ) )
    {
        lua_pushnumber ( luaVM, fMaxHeight );
        return 1;
    }
    
    lua_pushboolean ( luaVM, false );
    return 1;
}
