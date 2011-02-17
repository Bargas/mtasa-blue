/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/lua/CLuaTimer.cpp
*  PURPOSE:     Lua timer class
*  DEVELOPERS:  Oliver Brown <>
*               Christian Myhre Lundheim <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CLuaTimer::CLuaTimer ( const CLuaFunctionRef& iLuaFunction, const CLuaArguments& Arguments )
{
    m_iLuaFunction = iLuaFunction;
    m_Arguments = Arguments;
    m_bBeingDeleted = false;
}


CLuaTimer::~CLuaTimer ( void )
{
}


void CLuaTimer::ExecuteTimer ( CLuaMain* pLuaMain )
{
    if ( VERIFY_FUNCTION ( m_iLuaFunction ) )
    {
        m_Arguments.Call ( pLuaMain, m_iLuaFunction );
    }
}


unsigned long CLuaTimer::GetTimeLeft ( void )
{
    unsigned long ulCurrentTime = GetTime ();
    unsigned long ulTimeLeft = m_ulStartTime + m_ulDelay - ulCurrentTime;
    return ulTimeLeft;
}
