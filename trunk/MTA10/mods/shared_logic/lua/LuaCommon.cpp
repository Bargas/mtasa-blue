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

// Lua push/pop macros for our datatypes
CClientRadarMarker* lua_toblip ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTRADARMARKER )
    {
        return static_cast < CClientRadarMarker* > ( pElement );
    }

    return NULL;
}


CClientColModel* lua_tocolmodel ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTCOL )
    {
        return static_cast < CClientColModel* > ( pElement );
    }

    return NULL;
}

CClientColShape* lua_tocolshape ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTCOLSHAPE )
    {
        return static_cast < CClientColShape* > ( pElement );
    }

    return NULL;
}

CScriptFile* lua_tofile ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == SCRIPTFILE )
        return static_cast < CScriptFile* > ( pElement );
    else
        return NULL;
}

CClientDFF* lua_todff ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTDFF )
    {
        return static_cast < CClientDFF* > ( pElement );
    }

    return NULL;
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

    return NULL;
}

CClientGUIElement* lua_toguielement ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTGUI )
    {
        return static_cast < CClientGUIElement* > ( pElement );
    }

    return NULL;
}

CClientMarker* lua_tomarker ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTMARKER )
    {
        return static_cast < CClientMarker* > ( pElement );
    }

    return NULL;
}

CClientObject* lua_toobject ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTOBJECT )
    {
        return static_cast < CClientObject* > ( pElement );
    }

    return NULL;
}

CClientPed* lua_toped ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement )
    {
        eClientEntityType eType = pElement->GetType ();
        if ( eType == CCLIENTPED || eType == CCLIENTPLAYER )
            return static_cast < CClientPed* > ( pElement );
    }

    return NULL;
}

CClientPickup* lua_topickup ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTPICKUP )
    {
        return static_cast < CClientPickup* > ( pElement );
    }

    return NULL;
}

CClientPlayer* lua_toplayer ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTPLAYER )
    {
        return static_cast < CClientPlayer* > ( pElement );
    }

    return NULL;
}

CClientProjectile* lua_toprojectile ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTPROJECTILE )
    {
        return static_cast < CClientProjectile* > ( pElement );
    }

    return NULL;
}

CResource* lua_toresource ( lua_State* luaVM, int iArgument )
{
    return g_pClientGame->GetResourceManager ()->GetResourceFromScriptID ( reinterpret_cast < unsigned long > ( lua_touserdata ( luaVM, iArgument ) ) );
}

CClientSound* lua_tosound ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTSOUND )
    {
        return static_cast < CClientSound* > ( pElement );
    }

    return NULL;
}

CClientTeam* lua_toteam ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTTEAM )
    {
        return static_cast < CClientTeam* > ( pElement );
    }

    return NULL;
}

CClientTXD* lua_totxd ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTTXD )
    {
        return static_cast < CClientTXD* > ( pElement );
    }

    return NULL;
}

CClientVehicle* lua_tovehicle ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTVEHICLE )
    {
        return static_cast < CClientVehicle* > ( pElement );
    }

    return NULL;
}

CClientWater* lua_towater ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && pElement->GetType () == CCLIENTWATER )
    {
        return static_cast < CClientWater* > ( pElement );
    }

    return NULL;
}

CXMLNode* lua_toxmlnode ( lua_State* luaVM, int iArgument )
{
    return g_pCore->GetXML ()->GetNodeFromID ( reinterpret_cast < unsigned long > ( lua_touserdata ( luaVM, iArgument ) ) );
}

void lua_pushelement ( lua_State* luaVM, CClientEntity* pElement )
{
    if ( pElement )
    {
        ElementID ID = pElement->GetID ();
        if ( ID != INVALID_ELEMENT_ID )
        {
            lua_pushlightuserdata ( luaVM, (void*) reinterpret_cast<unsigned int *>(ID.Value()) );
            return;
        }
    }

    lua_pushnil ( luaVM );
}

void lua_pushresource ( lua_State* luaVM, CResource* pResource )
{
    lua_pushlightuserdata ( luaVM, reinterpret_cast < void* > ( pResource->GetScriptID () ) );
}

void lua_pushtimer ( lua_State* luaVM, CLuaTimer* pTimer )
{
    lua_pushlightuserdata ( luaVM, reinterpret_cast < void* > ( pTimer->GetScriptID () ) );
}

void lua_pushxmlnode ( lua_State* luaVM, CXMLNode* pElement )
{
    unsigned long ulID = pElement->GetID ();
    lua_pushlightuserdata ( luaVM, reinterpret_cast < void* > ( ulID ) );
}

CClientRadarArea* lua_toradararea ( lua_State* luaVM, int iArgument )
{
    CClientEntity* pElement = lua_toelement ( luaVM, iArgument );
    if ( pElement && IS_RADAR_AREA ( pElement ) )
        return static_cast < CClientRadarArea* > ( pElement );
    else
        return NULL;
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
