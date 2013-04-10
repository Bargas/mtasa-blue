/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/logic/lua/LuaCommon.cpp
*  PURPOSE:     Lua common functions
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

// Temporary until we change these funcs:
#include "../luadefs/CLuaDefs.h"
// End of temporary

// Prevent the warning issued when doing unsigned short -> void*
#pragma warning(disable:4312)

#pragma message(__LOC__"Use RTTI/dynamic_casting here for safety?")

void lua_pushelement ( lua_State* luaVM, CClientEntity* pElement )
{
    if ( pElement )
    {
        ElementID ID = pElement->GetID ();
        if ( ID != INVALID_ELEMENT_ID )
        {
            switch ( pElement->GetType() )
            {
            case CCLIENTPLAYER:
                lua_pushuserdata ( luaVM, "Player", (void*) reinterpret_cast<unsigned int *>(ID.Value()) );
                break;
            case CCLIENTPED:
                lua_pushuserdata ( luaVM, "Ped", (void*) reinterpret_cast<unsigned int *>(ID.Value()) );
                break;
            case CCLIENTVEHICLE:
                lua_pushuserdata ( luaVM, "Element", (void*) reinterpret_cast<unsigned int *>(ID.Value()) );
                break;
            default:
                lua_pushuserdata ( luaVM, "Unknown", (void*) reinterpret_cast<unsigned int *>(ID.Value()) );
                break;
            }
            return;
        }
    }

    lua_pushnil ( luaVM );
}

CClientEntity* lua_toelement ( lua_State* luaVM, int iArgument )
{
    if ( lua_type ( luaVM, iArgument ) == LUA_TLIGHTUSERDATA )
    {
        ElementID ID = TO_ELEMENTID ( lua_touserdata ( luaVM, iArgument ) );
        CClientEntity* pEntity = CElementIDs::GetElement ( ID );
        if ( !pEntity || pEntity->IsBeingDeleted () )
            return NULL;
        return pEntity;
    }
    else if ( lua_type ( luaVM, iArgument ) == LUA_TUSERDATA )
    {
        ElementID ID = TO_ELEMENTID ( * ( ( void ** ) lua_touserdata ( luaVM, iArgument ) ) );
        CClientEntity* pEntity = CElementIDs::GetElement ( ID );
        if ( !pEntity || pEntity->IsBeingDeleted () )
            return NULL;
        return pEntity;
    }

    return NULL;
}

void lua_pushresource ( lua_State* luaVM, CResource* pResource )
{
    lua_pushuserdata ( luaVM, "Unknown",  reinterpret_cast < void* > ( pResource->GetScriptID () ) );
}

void lua_pushtimer ( lua_State* luaVM, CLuaTimer* pTimer )
{
    lua_pushuserdata ( luaVM, "Unknown",  reinterpret_cast < void* > ( pTimer->GetScriptID () ) );
}

void lua_pushxmlnode ( lua_State* luaVM, CXMLNode* pElement )
{
    unsigned long ulID = pElement->GetID ();
    lua_pushuserdata ( luaVM, "Unknown", reinterpret_cast < void* > ( ulID ) );
}

void lua_pushuserdata ( lua_State* luaVM, const char* szClass, void* value )
{
    lua_pushstring ( luaVM, "ud" );
    lua_rawget ( luaVM, LUA_REGISTRYINDEX );

    // First we want to check if we have a userdata for this already
    lua_pushlightuserdata ( luaVM, value );
    lua_rawget ( luaVM, -2 );

    if ( lua_isnil ( luaVM, -1 ) )
    {
        lua_pop ( luaVM, 1 );

        // we don't have it, create it
        * ( void ** ) lua_newuserdata ( luaVM, sizeof ( void * ) ) = value;

        // Assign the class metatable
        {
            lua_pushstring ( luaVM, "mt" );
            lua_rawget ( luaVM, LUA_REGISTRYINDEX );
            
            lua_pushstring ( luaVM, szClass );
            lua_rawget ( luaVM, -2 );

            lua_remove ( luaVM, -2 );
            lua_setmetatable ( luaVM, -2 );
        }

        // save in ud table
        lua_pushlightuserdata ( luaVM, value );
        lua_pushvalue ( luaVM, -2 );
        lua_rawset ( luaVM, -4 );
    }

    // userdata is already on the stack, just remove the table
    lua_remove ( luaVM, -2 );
}

// Just do a type check vs LUA_TNONE before calling this, or bant
const char* lua_makestring ( lua_State* luaVM, int iArgument )
{
    if ( lua_type ( luaVM, iArgument ) == LUA_TSTRING )
    {
        return lua_tostring ( luaVM, iArgument );
    }
    lua_pushvalue ( luaVM, iArgument );
    lua_getglobal ( luaVM, "tostring" );
    lua_pushvalue ( luaVM, -2 );
    lua_call ( luaVM, 1, 1 );

    const char* szString = lua_tostring ( luaVM, -1 );
    lua_pop ( luaVM, 2 );

    return szString;
}


void lua_newclass ( lua_State* luaVM )
{
    lua_newtable ( luaVM );

    lua_pushstring ( luaVM, "__class" );
    lua_newtable ( luaVM );
    lua_rawset ( luaVM, -3 );

    lua_pushstring ( luaVM, "__get" );
    lua_newtable ( luaVM );
    lua_rawset ( luaVM, -3 );

    lua_pushstring ( luaVM, "__set" );
    lua_newtable ( luaVM );
    lua_rawset ( luaVM, -3 );

    lua_pushstring ( luaVM, "__index" );
    lua_pushvalue ( luaVM, -2 );
    lua_pushcclosure ( luaVM, CLuaClassDefs::Index, 1 );
    lua_rawset ( luaVM, -3 );

    lua_pushstring ( luaVM, "__newindex" );
    lua_pushvalue ( luaVM, -2 );
    lua_pushcclosure ( luaVM, CLuaClassDefs::NewIndex, 1 );
    lua_rawset ( luaVM, -3 );
}


void lua_registerclass ( lua_State* luaVM, const char* szName, const char* szParent )
{
    if ( szParent != NULL )
    {
        lua_rawget ( luaVM, LUA_REGISTRYINDEX );
        lua_getfield ( luaVM, -1, szParent );

        assert ( lua_istable ( luaVM, -1 ) );

        lua_setfield ( luaVM, -2, "__parent" );
    }

    lua_pushstring ( luaVM, "mt" );
    lua_rawget ( luaVM, LUA_REGISTRYINDEX );

    // store in registry
    lua_pushvalue ( luaVM, -2 );
    lua_setfield ( luaVM, -2, szName );

    lua_pop ( luaVM, 1 );

    // register with environment
    lua_getfield ( luaVM, -1, "__class" );
    lua_setglobal ( luaVM, szName );

    lua_pop ( luaVM, 1 );
}

void lua_classfunction ( lua_State* luaVM, const char* szFunction, const char* szOriginal )
{
    CLuaCFunction* pFunction = CLuaCFunctions::GetFunction ( szOriginal );
    if ( pFunction )
    {
        lua_pushstring ( luaVM, "__class" );
        lua_rawget ( luaVM, -2 );

        lua_pushstring ( luaVM, szFunction );
        lua_pushstring ( luaVM, szFunction );
        lua_pushcclosure ( luaVM, pFunction->GetFunctionAddress (), 1 );
        lua_rawset ( luaVM, -3 );

        lua_pop ( luaVM, 1 );
    }
}

void lua_classvariable ( lua_State* luaVM, const char* szVariable, const char* set, const char* get )
{
    // Set
    lua_pushstring ( luaVM, "__set" );
    lua_rawget ( luaVM, -2 );

    CLuaCFunction* pSet = NULL;
    CLuaCFunction* pGet = NULL;

    if ( ( !set ) || ! ( pSet = CLuaCFunctions::GetFunction ( set ) ) )
    {
        lua_pushstring ( luaVM, szVariable );
        lua_pushstring ( luaVM, szVariable );
        lua_pushcclosure ( luaVM, CLuaClassDefs::ReadOnly, 1 );
        lua_rawset ( luaVM, -3 );
    }
    else
    {
        lua_pushstring ( luaVM, szVariable );
        lua_pushstring ( luaVM, szVariable );
        lua_pushcclosure ( luaVM, pSet->GetFunctionAddress (), 1 );
        lua_rawset ( luaVM, -3 );
    }
    lua_pop ( luaVM, 1 );

    // Get
    lua_pushstring ( luaVM, "__get" );
    lua_rawget ( luaVM, -2 );

    if ( ( !get ) || ! ( pGet = CLuaCFunctions::GetFunction ( get ) ) )
    {
        lua_pushstring ( luaVM, szVariable );
        lua_pushstring ( luaVM, szVariable );
        lua_pushcclosure ( luaVM, CLuaClassDefs::WriteOnly, 1 );
        lua_rawset ( luaVM, -3 );
    }
    else
    {
        lua_pushstring ( luaVM, szVariable );
        lua_pushstring ( luaVM, szVariable );
        lua_pushcclosure ( luaVM, pGet->GetFunctionAddress (), 1 );
        lua_rawset ( luaVM, -3 );
    }
    lua_pop ( luaVM, 1 );
}