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
    m_uiScriptID = CIdArray::PopUniqueId ( this, EIdClass::TIMER );
    m_uiRepeats = 1;
    m_iLuaFunction = iLuaFunction;
    m_Arguments = Arguments;
}


CLuaTimer::~CLuaTimer ( void )
{
    CIdArray::PushUniqueId ( this, EIdClass::TIMER, m_uiScriptID );
}


void CLuaTimer::ExecuteTimer ( CLuaMain* pLuaMain )
{
    if ( VERIFY_FUNCTION ( m_iLuaFunction ) )
    {
        m_Arguments.Call ( pLuaMain, m_iLuaFunction );
    }
}


CTickCount CLuaTimer::GetTimeLeft ( void )
{
    CTickCount llCurrentTime = CTickCount::Now ();
    CTickCount llTimeLeft = m_llStartTime + m_llDelay - llCurrentTime;
    return llTimeLeft;
}
