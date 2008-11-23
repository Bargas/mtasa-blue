/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/lua/CLuaArguments.h
*  PURPOSE:     Lua argument manager class
*  DEVELOPERS:  Ed Lyons <>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CLUAARGUMENTS_H
#define __CLUAARGUMENTS_H

extern "C"
{
    #include "lua.h"
}

#include "CLuaArgument.h"
#include <vector>
#include "../common/CBitStream.h"
#include "json.h"

class CAccessControlList;
class CAccessControlListGroup;
class CAccount;
class CElement;
class CLuaTimer;
class CResource;
class CTextDisplay;
class CTextItem;

using namespace std;

class CLuaArguments
{
public:
    inline                                              CLuaArguments       ( void )                {};
                                                        CLuaArguments       ( const CLuaArguments& Arguments );
														CLuaArguments		( NetServerBitStreamInterface& bitStream );
    inline                                              ~CLuaArguments      ( void )                { DeleteArguments (); };

    const CLuaArguments&                                operator =          ( const CLuaArguments& Arguments );
	CLuaArgument*										operator []			( const unsigned int uiPosition ) const;

    void                                                ReadArgument        ( lua_State* luaVM, signed int uiIndex );
    void                                                ReadArguments       ( lua_State* luaVM, signed int uiIndexBegin = 1 );
    void                                                PushArguments       ( lua_State* luaVM ) const;
    void                                                PushArguments       ( CLuaArguments& Arguments );
    bool                                                Call                ( class CLuaMain* pLuaMain, int iLuaFunction, CLuaArguments * returnValues = NULL ) const;
	bool                                                CallGlobal          ( class CLuaMain* pLuaMain, const char* szFunction, CLuaArguments * returnValues = NULL ) const;

    void                                                ReadTable           ( lua_State* luaVM, signed int uiIndexBegin, unsigned int depth );
    void                                                PushAsTable         ( lua_State* luaVM );

    CLuaArgument*                                       PushNil             ( void );
    CLuaArgument*                                       PushBoolean         ( bool bBool );
    CLuaArgument*                                       PushNumber          ( double dNumber );
    CLuaArgument*                                       PushString          ( const char* szString );
    CLuaArgument*                                       PushUserData        ( void* pUserData );
    CLuaArgument*                                       PushElement         ( CElement* pElement );
    CLuaArgument*                                       PushACL             ( CAccessControlList* pACL );
    CLuaArgument*                                       PushACLGroup        ( CAccessControlListGroup* pACLGroup );
    CLuaArgument*                                       PushAccount         ( CAccount* pAccount );
    CLuaArgument*                                       PushResource        ( CResource* pResource );
    CLuaArgument*                                       PushTextDisplay     ( CTextDisplay* pTextDisplay );
    CLuaArgument*                                       PushTextItem        ( CTextItem* pTextItem );
    CLuaArgument*                                       PushTimer           ( CLuaTimer* pLuaTimer );

    CLuaArgument*                                       PushArgument        ( CLuaArgument& argument );
    CLuaArgument*                                       PushTable           ( CLuaArguments * table );

    void                                                DeleteArguments     ( void );

    bool                                                ReadFromBitStream   ( NetServerBitStreamInterface& bitStream );
    bool                                                ReadFromJSONString  ( const char* szJSON );
    bool                                                WriteToBitStream    ( NetServerBitStreamInterface& bitStream ) const;
    vector < char * > *                                 WriteToCharVector   ( vector < char * > * values );
    bool                                                WriteToJSONString   ( std::string& strJSON, bool bSerialize = false );
    json_object *                                       WriteTableToJSONObject ( bool bSerialize = false );
    json_object *                                       WriteToJSONArray    ( bool bSerialize );
    bool                                                ReadFromJSONObject  ( json_object * object );
    bool                                                ReadFromJSONArray   ( json_object * object );

    inline unsigned int                                 Count               ( void ) const          { return static_cast < unsigned int > ( m_Arguments.size () ); };
    inline vector < CLuaArgument* > ::const_iterator    IterBegin           ( void )                { return m_Arguments.begin (); };
    inline vector < CLuaArgument* > ::const_iterator    IterEnd             ( void )                { return m_Arguments.end (); };

private:
    vector < CLuaArgument* >                            m_Arguments;
};

#endif
