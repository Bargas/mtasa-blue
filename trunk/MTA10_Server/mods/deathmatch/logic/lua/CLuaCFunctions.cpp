/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/lua/CLuaCFunctions.cpp
*  PURPOSE:     Lua C-function extension class
*  DEVELOPERS:  Oliver Brown <>
*               Christian Myhre Lundheim <>
*               Alberto Alonso <rydencillo@gmail.com>
*               arc_
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CFastHashMap < lua_CFunction, CLuaCFunction* > CLuaCFunctions::ms_Functions;
CFastHashMap < SString, CLuaCFunction* > CLuaCFunctions::ms_FunctionsByName;
void* CLuaCFunctions::ms_pFunctionPtrLow = (void*)0xffffffff;
void* CLuaCFunctions::ms_pFunctionPtrHigh = 0;

// CFastHashMap helpers
static lua_CFunction GetEmptyMapKey ( lua_CFunction* )   { return (lua_CFunction)0x00000000; }
static lua_CFunction GetDeletedMapKey ( lua_CFunction* ) { return (lua_CFunction)0xFFFFFFFF; }

CLuaCFunction::CLuaCFunction ( const char* szName, lua_CFunction f, bool bRestricted )
{
    m_strName = szName ? szName : "";
    m_Function = f;
    m_bRestricted = bRestricted;
}

CLuaCFunctions::CLuaCFunctions ()
{
}

CLuaCFunctions::~CLuaCFunctions ()
{
    RemoveAllFunctions ();
}


CLuaCFunction* CLuaCFunctions::AddFunction ( const char* szName, lua_CFunction f, bool bRestricted )
{
    ms_pFunctionPtrLow = Min < void* > ( ms_pFunctionPtrLow, (void*)f );
    ms_pFunctionPtrHigh = Max < void* > ( ms_pFunctionPtrHigh, (void*)f );

    // Already have a function by this name?
    CLuaCFunction* pFunction = GetFunction ( szName );
    if ( pFunction )
        return pFunction;

    // Already have a function by this address?
    pFunction = GetFunction ( f );
    if ( !pFunction )
    {
        pFunction = new CLuaCFunction ( szName, f, bRestricted );
        ms_Functions [ f ] = pFunction;
    }
    ms_FunctionsByName [ szName ] = pFunction;
    return pFunction;
}


CLuaCFunction* CLuaCFunctions::GetFunction ( lua_CFunction f )
{
    // Quick cull of unknown pointer range
    if ( f < ms_pFunctionPtrLow || f > ms_pFunctionPtrHigh )
        return NULL;

    CFastHashMap < lua_CFunction, CLuaCFunction* >::iterator it;
    it = ms_Functions.find ( f );
    if ( it == ms_Functions.end () )
        return NULL;

    return it->second;
}


CLuaCFunction* CLuaCFunctions::GetFunction ( const char* szName )
{
    CFastHashMap < SString, CLuaCFunction* >::iterator it;
    it = ms_FunctionsByName.find ( szName );
    if ( it == ms_FunctionsByName.end () )
        return NULL;

    return it->second;
}


void CLuaCFunctions::RegisterFunctionsWithVM ( lua_State* luaVM )
{
    // Register all our functions to a lua VM
    CFastHashMap < SString, CLuaCFunction* >::iterator it;
    for ( it = ms_FunctionsByName.begin (); it != ms_FunctionsByName.end (); it++ )
    {
        lua_pushstring ( luaVM, it->first.c_str () );
        lua_pushcclosure ( luaVM, it->second->GetAddress (), 1 );
        lua_setglobal ( luaVM, it->first.c_str () );
    }
}


void CLuaCFunctions::RemoveAllFunctions ( void )
{
    // Delete all functions
    CFastHashMap < lua_CFunction, CLuaCFunction* >::iterator it;
    for ( it = ms_Functions.begin (); it != ms_Functions.end (); it++ )
    {
        delete it->second;
    }
    ms_Functions.clear ();
    ms_FunctionsByName.clear ();
}
