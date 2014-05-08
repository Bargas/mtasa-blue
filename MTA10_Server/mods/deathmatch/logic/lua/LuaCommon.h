/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/lua/LuaCommon.h
*  PURPOSE:     Lua common functions
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __LUACOMMON_H
#define __LUACOMMON_H

extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

CLuaFunctionRef         luaM_toref              ( lua_State *luaVM, int iArgument );

#define TO_ELEMENTID(x) ((ElementID) reinterpret_cast < unsigned long > (x) )

// Lua pop macros for our datatypes
class CElement*         lua_toelement           ( lua_State* luaVM, int iArgument );


// Lua push macros for our datatypes
void                    lua_pushelement         ( lua_State* luaVM, class CElement* pElement );
void                    lua_pushacl             ( lua_State* luaVM, class CAccessControlList* pACL );
void                    lua_pushaclgroup        ( lua_State* luaVM, class CAccessControlListGroup* pACL );
void                    lua_pushaccount         ( lua_State* luaVM, class CAccount* pAccount );
void                    lua_pushresource        ( lua_State* luaVM, class CResource* pResource );
void                    lua_pushtextdisplay     ( lua_State* luaVM, class CTextDisplay* pDisplay );
void                    lua_pushtextitem        ( lua_State* luaVM, class CTextItem* pItem );
void                    lua_pushtimer           ( lua_State* luaVM, class CLuaTimer* pTimer );
void                    lua_pushxmlnode         ( lua_State* luaVM, class CXMLNode* pNode );
void                    lua_pushban             ( lua_State* luaVM, class CBan* pBan );
void                    lua_pushquery           ( lua_State* luaVM, class CDbJobData* pJobData );
void                    lua_pushuserdata        ( lua_State* luaVM, void* value );
void                    lua_pushobject          ( lua_State* luaVM, const char* szClass, void* pObject );

void                    lua_pushvector          ( lua_State* luaVM, CVector& vector );
void                    lua_pushmatrix          ( lua_State* luaVM, CMatrix& matrix );

// Converts any type to string
const char*             lua_makestring          ( lua_State* luaVM, int iArgument );

// Internal use
void                    lua_initclasses         ( lua_State* luaVM );

void                    lua_newclass            ( lua_State* luaVM );
void                    lua_getclass            ( lua_State* luaVM, const char* szName );
void                    lua_registerclass       ( lua_State* luaVM, const char* szName, const char* szParent = NULL );
void                    lua_registerstaticclass ( lua_State* luaVM, const char* szName );
void                    lua_classfunction       ( lua_State* luaVM, const char* szFunction, lua_CFunction fn );
void                    lua_classfunction       ( lua_State* luaVM, const char* szFunction, const char* fn );
void                    lua_classvariable       ( lua_State* luaVM, const char* szVariable, lua_CFunction set, lua_CFunction get );
void                    lua_classvariable       ( lua_State* luaVM, const char* szVariable, const char* set, const char* get );
void                    lua_classmetamethod     ( lua_State* luaVM, const char* szName, lua_CFunction fn );

// Include the RPC functions enum
#include "net/rpc_enums.h"

enum
{
    AUDIO_FRONTEND,
    AUDIO_MISSION_PRELOAD,
    AUDIO_MISSION_PLAY
};

#endif
