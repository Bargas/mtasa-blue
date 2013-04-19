/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Output.cpp
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

int CLuaFunctionDefs::OutputConsole ( lua_State* luaVM )
{
    SString strText = "";
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strText );

    if ( !argStream.HasErrors ( ) )
    {
        if ( CStaticFunctionDefinitions::OutputConsole ( strText ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::OutputChatBox ( lua_State* luaVM )
{
    SString strText = "";
    unsigned char ucRed = 231;
    unsigned char ucGreen = 217;
    unsigned char ucBlue = 176;
    bool bColorCoded = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strText );
    argStream.ReadNumber ( ucRed, 231 );
    argStream.ReadNumber ( ucGreen, 217 );
    argStream.ReadNumber ( ucBlue, 176 );
    argStream.ReadBool ( bColorCoded, false );

    if ( !argStream.HasErrors ( ) )
    {
        // Output to the chatbox
        if ( CStaticFunctionDefinitions::OutputChatBox ( strText, ucRed, ucGreen, ucBlue, bColorCoded ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::SetClipboard ( lua_State* luaVM )
{
    SString strText = "";
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strText );

    if ( !argStream.HasErrors ( ) )
    {
        // set to clip board
        if ( CStaticFunctionDefinitions::SetClipboard ( strText ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::GetClipboard ( lua_State *luaVM )
{
    SString strText;
    if ( CStaticFunctionDefinitions::GetClipboard ( strText ) )
        lua_pushstring ( luaVM, strText.c_str() );
    else
        lua_pushnil ( luaVM );

    return 1;
}

int CLuaFunctionDefs::ShowChat ( lua_State* luaVM )
{
    bool bShow = false;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadBool ( bShow );

    if ( !argStream.HasErrors ( ) )
    {
        if ( CStaticFunctionDefinitions::ShowChat ( bShow ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::OutputClientDebugString ( lua_State* luaVM )
{
    SString strText = "";
    unsigned int uiLevel = 3;
    unsigned char ucRed = 235;
    unsigned char ucGreen = 221;
    unsigned char ucBlue = 178;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strText );
    argStream.ReadNumber ( uiLevel, 3 );
    argStream.ReadNumber ( ucRed, 235 );
    argStream.ReadNumber ( ucGreen, 221 );
    argStream.ReadNumber ( ucBlue, 178 );

    // check they're in range
    if ( ucRed > 255 || ucRed < 0 ) 
        m_pScriptDebugging->LogWarning ( luaVM, "Specify a red value between 0 and 255" );
    else if ( ucGreen > 255 || ucGreen < 0 )
        m_pScriptDebugging->LogWarning ( luaVM, "Specify a green value between 0 and 255" );
    else if ( ucBlue >  255 || ucBlue <  0 )
        m_pScriptDebugging->LogWarning ( luaVM, "Specify a blue value between 0 and 255" );

    // Too big level?
    if ( uiLevel > 3 )
    {
        m_pScriptDebugging->LogWarning ( luaVM, "Bad level argument sent to %s (0-3)", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ) );

        lua_pushboolean ( luaVM, false );
        return 1;
    }

    if ( !argStream.HasErrors ( ) )
    {
        // Grab our VM
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            // Valid string?
            if ( !argStream.NextIsNil ( ) )
            {
                if ( uiLevel == 1 )
                {
                    m_pScriptDebugging->LogError ( luaVM, "%s", strText );
                }
                else if ( uiLevel == 2 )
                {
                    m_pScriptDebugging->LogWarning ( luaVM, "%s", strText );
                }
                else if ( uiLevel == 3 )
                {
                    m_pScriptDebugging->LogInformation ( luaVM, "%s", strText );
                }
                else if ( uiLevel == 0 )
                {
                    m_pScriptDebugging->LogCustom ( luaVM, ucRed, ucGreen, ucBlue, "%s", strText );
                }

                // Success
                lua_pushboolean ( luaVM, true );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadType ( luaVM );
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage() );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}



