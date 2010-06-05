/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Marker.cpp
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

int CLuaFunctionDefs::CreateMarker ( lua_State* luaVM )
{
    // Valid position arguments?
    int iArgument1 = lua_type ( luaVM, 1 );
    int iArgument2 = lua_type ( luaVM, 2 );
    int iArgument3 = lua_type ( luaVM, 3 );
    if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
        ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
        ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) )
    {
        // Grab the position
        CVector vecPosition;
        vecPosition.fX = static_cast < float > ( lua_tonumber ( luaVM, 1 ) );
        vecPosition.fY = static_cast < float > ( lua_tonumber ( luaVM, 2 ) );
        vecPosition.fZ = static_cast < float > ( lua_tonumber ( luaVM, 3 ) );

        // Other defaulted arguments
        unsigned char ucType = 0;
        float fSize = 4.0f;
        unsigned char ucRed = 0;
        unsigned char ucGreen = 0;
        unsigned char ucBlue = 255;
        unsigned char ucAlpha = 255;

        // Optional type argument
        char szDefaultType [] = "default";
        const char* szType = szDefaultType;
        int iArgument4 = lua_type ( luaVM, 4 );
        if ( iArgument4 == LUA_TSTRING )
        {
            szType = lua_tostring ( luaVM, 4 );
        }

        // Optional size argument
        int iArgument5 = lua_type ( luaVM, 5 );
        if ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING )
        {
            lua_Number nSize = lua_tonumber ( luaVM, 5 );
            if ( nSize > 0 )
            {
                fSize = static_cast < float > ( nSize );
            }
        }

        // Optional red argument
        int iArgument6 = lua_type ( luaVM, 6 );
        if ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING )
        {
            lua_Number nRed = lua_tonumber ( luaVM, 6 );
            if ( nRed >= 0 && nRed <= 255 )
            {
                ucRed = static_cast < unsigned char > ( nRed );
            }
        }

        // Optional green argument
        int iArgument7 = lua_type ( luaVM, 7 );
        if ( iArgument7 == LUA_TNUMBER || iArgument7 == LUA_TSTRING )
        {
            lua_Number nGreen = lua_tonumber ( luaVM, 7 );
            if ( nGreen >= 0 && nGreen <= 255 )
            {
                ucGreen = static_cast < unsigned char > ( nGreen );
            }
        }

        // Optional blue argument
        int iArgument8 = lua_type ( luaVM, 8 );
        if ( iArgument8 == LUA_TNUMBER || iArgument8 == LUA_TSTRING )
        {
            lua_Number nBlue = lua_tonumber ( luaVM, 8 );
            if ( nBlue >= 0 && nBlue <= 255 )
            {
                ucBlue = static_cast < unsigned char > ( nBlue );
            }
        }

        // Optional alpha argument
        int iArgument9 = lua_type ( luaVM, 9 );
        if ( iArgument9 == LUA_TNUMBER || iArgument9 == LUA_TSTRING )
        {
            lua_Number nAlpha = lua_tonumber ( luaVM, 9 );
            if ( nAlpha >= 0 && nAlpha <= 255 )
            {
                ucAlpha = static_cast < unsigned char > ( nAlpha );
            }
        }

        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            CResource* pResource = pLuaMain->GetResource();
            {
                // Create it
                CClientMarker* pMarker = CStaticFunctionDefinitions::CreateMarker ( *pResource, vecPosition, szType, fSize, ucRed, ucGreen, ucBlue, ucAlpha );
                if ( pMarker )
                {
                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( ( CClientEntity* ) pMarker );
                    }

                    lua_pushelement ( luaVM, pMarker );
                    return 1;
                }
            }
        }
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "createMarker" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetMarkerCount ( lua_State* luaVM )
{
    unsigned int uiCount = m_pMarkerManager->Count ();
    lua_pushnumber ( luaVM, static_cast < lua_Number > ( uiCount ) );
    return 1;
}


int CLuaFunctionDefs::GetMarkerType ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        CClientMarker* pMarker = lua_tomarker ( luaVM, 1 );
        if ( pMarker )
        {
            char szMarkerType [64];
            if ( CClientMarker::TypeToString ( pMarker->GetMarkerType (), szMarkerType ) )
            {
                lua_pushstring ( luaVM, szMarkerType );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getMarkerType", "marker", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getMarkerType" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetMarkerSize ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        CClientMarker* pMarker = lua_tomarker ( luaVM, 1 );
        if ( pMarker )
        {
            float fSize = pMarker->GetSize ();
            lua_pushnumber ( luaVM, static_cast < lua_Number > ( fSize ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getMarkerSize", "marker", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getMarkerSize" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetMarkerColor ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        CClientMarker* pMarker = lua_tomarker ( luaVM, 1 );
        if ( pMarker )
        {
            unsigned char ucRed;
            unsigned char ucGreen;
            unsigned char ucBlue;
            unsigned char ucAlpha;
            pMarker->GetColor ( ucRed, ucGreen, ucBlue, ucAlpha );

            lua_pushnumber ( luaVM, static_cast < lua_Number > ( ucRed ) );
            lua_pushnumber ( luaVM, static_cast < lua_Number > ( ucGreen ) );
            lua_pushnumber ( luaVM, static_cast < lua_Number > ( ucBlue ) );
            lua_pushnumber ( luaVM, static_cast < lua_Number > ( ucAlpha ) );
            return 4;
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getMarkerColor", "marker", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getMarkerColor" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetMarkerTarget ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        CClientMarker* pMarker = lua_tomarker ( luaVM, 1 );
        if ( pMarker )
        {
            CVector vecTarget;
            if ( CStaticFunctionDefinitions::GetMarkerTarget ( *pMarker, vecTarget ) )
            {
                lua_pushnumber ( luaVM, vecTarget.fX );
                lua_pushnumber ( luaVM, vecTarget.fY );
                lua_pushnumber ( luaVM, vecTarget.fZ );
                return 3;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getMarkerTarget", "marker", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getMarkerTarget" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetMarkerIcon ( lua_State* luaVM )
{
    if ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA )
    {
        CClientMarker* pMarker = lua_tomarker ( luaVM, 1 );
        if ( pMarker )
        {
            char szMarkerIcon [64];
            CClientCheckpoint* pCheckpoint = pMarker->GetCheckpoint ();
            if ( pCheckpoint )
            {
                CClientCheckpoint::IconToString ( pCheckpoint->GetIcon (), szMarkerIcon );
                lua_pushstring ( luaVM, szMarkerIcon );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "getMarkerIcon", "marker", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "getMarkerIcon" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetMarkerType ( lua_State* luaVM )
{
    int iArgument1 = lua_type ( luaVM, 1 );
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( iArgument1 == LUA_TLIGHTUSERDATA &&
        iArgument2 == LUA_TSTRING )
    {
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        const char* szType = lua_tostring ( luaVM, 2 );
        if ( pEntity )
        {
            bool bSuccess = CStaticFunctionDefinitions::SetMarkerType ( *pEntity, szType );
            lua_pushboolean ( luaVM, bSuccess );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setMarkerType", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setMarkerType" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetMarkerSize ( lua_State* luaVM )
{
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA ) &&
        ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
    {
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            if ( CStaticFunctionDefinitions::SetMarkerSize ( *pEntity, static_cast < float > ( lua_tonumber ( luaVM, 2 ) ) ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setMarkerSize", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setMarkerSize" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetMarkerColor ( lua_State* luaVM )
{
    // Verify the argument types
    int iArgument2 = lua_type ( luaVM, 2 );
    int iArgument3 = lua_type ( luaVM, 3 );
    int iArgument4 = lua_type ( luaVM, 4 );
    int iArgument5 = lua_type ( luaVM, 5 );
    if ( ( lua_type ( luaVM, 1 ) == LUA_TLIGHTUSERDATA ) &&
        ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
        ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
        ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
    {
        // Convert to numbers
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        double dRed = lua_tonumber ( luaVM, 2 );
        double dGreen = lua_tonumber ( luaVM, 3 );
        double dBlue = lua_tonumber ( luaVM, 4 );
        double dAlpha = 255;
        if ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING )
            dAlpha = lua_tonumber ( luaVM, 5 );

        // Verify the pointer
        if ( pEntity )
        {
            // Check the ranges
            if ( dRed >= 0 && dRed <= 255 &&
                dGreen >= 0 && dGreen <= 255 &&
                dBlue >= 0 && dBlue <= 255 &&
                dAlpha >= 0 && dAlpha <= 255 )
            {
                unsigned char ucRed = static_cast < unsigned char > ( dRed );
                unsigned char ucGreen = static_cast < unsigned char > ( dGreen );
                unsigned char ucBlue = static_cast < unsigned char > ( dBlue );
                unsigned char ucAlpha = static_cast < unsigned char > ( dAlpha );

                // Set the new color
                if ( CStaticFunctionDefinitions::SetMarkerColor ( *pEntity, ucRed, ucGreen, ucBlue, ucAlpha ) )
                {
                    lua_pushboolean ( luaVM, true );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setMarkerColor", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setMarkerColor" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetMarkerTarget ( lua_State* luaVM )
{
    // Verify the element argument
    int iArgument1 = lua_type ( luaVM, 1 );
    if ( iArgument1 == LUA_TLIGHTUSERDATA )
    {
        // Grab and verify the element
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        if ( pEntity )
        {
            CVector vecTarget;
            CVector* pTargetVector = NULL;

            // Valid target vector arguments?
            int iArgument2 = lua_type ( luaVM, 2 );
            int iArgument3 = lua_type ( luaVM, 3 );
            int iArgument4 = lua_type ( luaVM, 4 );
            if ( ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
                ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
                ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
            {
                // Grab the target vector
                vecTarget.fX = static_cast < float > ( lua_tonumber ( luaVM, 2 ) );
                vecTarget.fY = static_cast < float > ( lua_tonumber ( luaVM, 3 ) );
                vecTarget.fZ = static_cast < float > ( lua_tonumber ( luaVM, 4 ) );

                pTargetVector = &vecTarget;
            }
            else if ( iArgument2 != LUA_TNONE )
            {
                m_pScriptDebugging->LogBadType ( luaVM, "setMarkerTarget" );

                lua_pushboolean ( luaVM, false );
                return 1;
            }

            // Do it
            if ( CStaticFunctionDefinitions::SetMarkerTarget ( *pEntity, pTargetVector ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setMarkerTarget", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setMarkerTarget" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetMarkerIcon ( lua_State* luaVM )
{
    int iArgument1 = lua_type ( luaVM, 1 );
    int iArgument2 = lua_type ( luaVM, 2 );
    if ( iArgument1 == LUA_TLIGHTUSERDATA &&
        iArgument2 == LUA_TSTRING )
    {
        CClientEntity* pEntity = lua_toelement ( luaVM, 1 );
        const char* szIcon = lua_tostring ( luaVM, 2 );
        if ( pEntity )
        {
            if ( CStaticFunctionDefinitions::SetMarkerIcon ( *pEntity, szIcon ) )
            {
                lua_pushboolean ( luaVM, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadPointer ( luaVM, "setMarkerIcon", "element", 1 );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM, "setMarkerIcon" );

    lua_pushboolean ( luaVM, false );
    return 1;
}


