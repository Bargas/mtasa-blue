/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CScriptKeyBinds.h
*  PURPOSE:     Header for script key binds class
*  DEVELOPERS:  Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Derek Abdine <>
*               Chris McArthur <>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CScriptKeyBinds_H
#define __CScriptKeyBinds_H

#include "../../shared_logic/lua/CLuaMain.h"
#include <list>
using namespace std;

#define NUMBER_OF_KEYS 123

enum eScriptKeyBindType
{
    SCRIPT_KEY_BIND_FUNCTION = 0,
    SCRIPT_KEY_BIND_CONTROL_FUNCTION,
    SCRIPT_KEY_BIND_UNDEFINED,
};

struct SScriptBindableKey
{
    char szKey [ 20 ];
};

struct SScriptBindableGTAControl
{
    char szControl [ 25 ];
};

class CScriptKeyBind
{
public:
    inline                  CScriptKeyBind        ( void ): boundKey ( NULL ), luaMain ( NULL ), beingDeleted ( false )
    {
    }
    inline bool             IsBeingDeleted ( void ) { return beingDeleted; }
    SScriptBindableKey*     boundKey;
    CLuaMain*               luaMain;
    bool                    beingDeleted;
    virtual eScriptKeyBindType    GetType         ( void ) = 0;
};

class CScriptKeyBindWithState: public CScriptKeyBind
{
public:
    inline                  CScriptKeyBindWithState   ( void ) { bHitState = true; }
    bool                    bHitState;
};

class CScriptFunctionBind
{
public:
    inline                  CScriptFunctionBind       ( void )    { m_iLuaFunction = -1; }
    inline                  ~CScriptFunctionBind      ( void )    {}
    int						m_iLuaFunction;
    CLuaArguments           m_Arguments;
};

class CScriptKeyFunctionBind: public CScriptKeyBindWithState, public CScriptFunctionBind
{
public:
    inline eScriptKeyBindType     GetType				( void )		{ return SCRIPT_KEY_BIND_FUNCTION; }
    
};

class CScriptControlFunctionBind: public CScriptKeyBindWithState, public CScriptFunctionBind
{
public:
    inline eScriptKeyBindType     GetType              ( void )    { return SCRIPT_KEY_BIND_CONTROL_FUNCTION; }
    SScriptBindableGTAControl*    boundControl;
};

class CScriptKeyBinds
{
public:
    inline                      CScriptKeyBinds         ( void ) : m_bProcessingKey ( false )
    {
    }
                                ~CScriptKeyBinds        ( void );

    static SScriptBindableKey*        GetBindableFromKey      ( const char* szKey );
    static SScriptBindableGTAControl* GetBindableFromControl  ( const char* szControl );

    // Basic funcs
    void                        Add                     ( CScriptKeyBind* pKeyBind );
    void                        Clear                   ( eScriptKeyBindType bindType = SCRIPT_KEY_BIND_UNDEFINED );
    void                        Call                    ( CScriptKeyBind* pKeyBind );
    bool                        ProcessKey              ( const char* szKey, bool bHitState, eScriptKeyBindType bindTypeconst );

    list < CScriptKeyBind* > ::iterator IterBegin       ( void )            { return m_List.begin (); }
    list < CScriptKeyBind* > ::iterator IterEnd         ( void )            { return m_List.end (); }

    // Key-function bind funcs
    bool                        AddKeyFunction          ( const char* szKey, bool bHitState, CLuaMain* pLuaMain, int iLuaFunction, CLuaArguments& Arguments );
    bool                        AddKeyFunction          ( SScriptBindableKey* pKey, bool bHitState, CLuaMain* pLuaMain, int iLuaFunction, CLuaArguments& Arguments );
    bool                        RemoveKeyFunction       ( const char* szKey, CLuaMain* pLuaMain, bool bCheckHitState = false, bool bHitState = true, int iLuaFunction = NULL );
    bool                        RemoveKeyFunction       ( SScriptBindableKey* pKey, CLuaMain* pLuaMain, bool bCheckHitState = false, bool bHitState = true, int iLuaFunction = NULL );
    bool                        KeyFunctionExists       ( const char* szKey, CLuaMain* pLuaMain = NULL, bool bCheckHitState = false, bool bHitState = true, int iLuaFunction = NULL );
    bool                        KeyFunctionExists       ( SScriptBindableKey* pKey, CLuaMain* pLuaMain = NULL, bool bCheckHitState = false, bool bHitState = true, int iLuaFunction = NULL );

    // Control-function bind funcs
    bool                        AddControlFunction      ( const char* szControl, bool bHitState, CLuaMain* pLuaMain, int iLuaFunction, CLuaArguments& Arguments );
    bool                        AddControlFunction      ( SScriptBindableGTAControl* pControl, bool bHitState, CLuaMain* pLuaMain, int iLuaFunction, CLuaArguments& Arguments );
    bool                        RemoveControlFunction   ( const char* szControl, CLuaMain* pLuaMain, bool bCheckHitState = false, bool bHitState = true, int iLuaFunction = NULL );
    bool                        RemoveControlFunction   ( SScriptBindableGTAControl* pControl, CLuaMain* pLuaMain, bool bCheckHitState = false, bool bHitState = true, int iLuaFunction = NULL );
    bool                        ControlFunctionExists   ( const char* szControl, CLuaMain* pLuaMain = NULL, bool bCheckHitState = false, bool bHitState = true, int iLuaFunction = NULL );
    bool                        ControlFunctionExists   ( SScriptBindableGTAControl* pControl, CLuaMain* pLuaMain = NULL, bool bCheckHitState = false, bool bHitState = true, int iLuaFunction = NULL );

	void						RemoveAllKeys			( CLuaMain* pLuaMain );

    static bool                 IsMouse                 ( SScriptBindableKey* pKey );
    void                        RemoveDeletedBinds      ( void );

protected:
    list < CScriptKeyBind* >    m_List;
    bool                        m_bProcessingKey;
};

#endif