/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CKeyBinds.h
*  PURPOSE:     Server keybind manager class
*  DEVELOPERS:  Jax <>
*               Cecill Etheredge <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CKeyBinds_H
#define __CKeyBinds_H

#include "lua/LuaCommon.h"
#include "lua/CLuaArguments.h"
#include <list>

#define NUMBER_OF_KEYS 123

enum eKeyBindType
{
    KEY_BIND_FUNCTION = 0,
    KEY_BIND_CONTROL_FUNCTION,
    KEY_BIND_UNDEFINED,
};

struct SBindableKey
{
    char szKey [ 20 ];
};

struct SBindableGTAControl
{
    char szControl [ 25 ];
};

class CKeyBind
{
public:
    inline                  CKeyBind        ( void )    { boundKey = NULL; luaMain = NULL; beingDeleted = false; }
    virtual                 ~CKeyBind       ( void )    {}
    inline bool             IsBeingDeleted ( void ) { return beingDeleted; }
    
    SBindableKey*           boundKey;
    CLuaMain*               luaMain;
    bool                    beingDeleted;
    virtual eKeyBindType    GetType         ( void ) = 0;
};

class CKeyBindWithState: public CKeyBind
{
public:
    inline                  CKeyBindWithState   ( void ) { bHitState = true; }
    bool                    bHitState;

};

class CFunctionBind
{
public:
    inline                  CFunctionBind       ( void )    {}
    inline                  ~CFunctionBind      ( void )    {}
    CLuaFunctionRef         m_iLuaFunction;
    CLuaArguments           m_Arguments;
};

class CKeyFunctionBind: public CKeyBindWithState, public CFunctionBind
{
public:
    inline eKeyBindType     GetType             ( void )    { return KEY_BIND_FUNCTION; }
    
};

class CControlFunctionBind: public CKeyBindWithState, public CFunctionBind
{
public:
    inline eKeyBindType     GetType              ( void )    { return KEY_BIND_CONTROL_FUNCTION; }
    SBindableGTAControl*    boundControl;
};

class CKeyBinds
{
public:
                                CKeyBinds               ( class CPlayer* pPlayer );
                                ~CKeyBinds              ( void );

    static SBindableKey*        GetBindableFromKey      ( const char* szKey );
    static SBindableGTAControl* GetBindableFromControl  ( const char* szControl );

    // Basic funcs
    void                        Add                     ( CKeyBind* pKeyBind );
    void                        Clear                   ( eKeyBindType bindType = KEY_BIND_UNDEFINED );
    void                        Call                    ( CKeyBind* pKeyBind );
    bool                        ProcessKey              ( const char* szKey, bool bHitState, eKeyBindType bindType );

    std::list < CKeyBind* > ::iterator IterBegin        ( void )            { return m_List.begin (); }
    std::list < CKeyBind* > ::iterator IterEnd          ( void )            { return m_List.end (); }

    // Key-function bind funcs
    bool                        AddKeyFunction          ( const char* szKey, bool bHitState, CLuaMain* pLuaMain, const CLuaFunctionRef& iLuaFunction, CLuaArguments& Arguments );
    bool                        AddKeyFunction          ( SBindableKey* pKey, bool bHitState, CLuaMain* pLuaMain, const CLuaFunctionRef& iLuaFunction, CLuaArguments& Arguments );
    bool                        RemoveKeyFunction       ( const char* szKey, CLuaMain* pLuaMain, bool bCheckHitState = false, bool bHitState = true, const CLuaFunctionRef& iLuaFunction = CLuaFunctionRef () );
    bool                        KeyFunctionExists       ( const char* szKey, CLuaMain* pLuaMain = NULL, bool bCheckHitState = false, bool bHitState = true, const CLuaFunctionRef& iLuaFunction = CLuaFunctionRef () );

    // Control-function bind funcs
    bool                        AddControlFunction      ( const char* szControl, bool bHitState, CLuaMain* pLuaMain, const CLuaFunctionRef& iLuaFunction, CLuaArguments& Arguments );
    bool                        AddControlFunction      ( SBindableGTAControl* pControl, bool bHitState, CLuaMain* pLuaMain, const CLuaFunctionRef& iLuaFunction, CLuaArguments& Arguments );
    bool                        RemoveControlFunction   ( const char* szControl, CLuaMain* pLuaMain, bool bCheckHitState = false, bool bHitState = true, const CLuaFunctionRef& iLuaFunction = CLuaFunctionRef () );
    bool                        ControlFunctionExists   ( const char* szControl, CLuaMain* pLuaMain = NULL, bool bCheckHitState = false, bool bHitState = true, const CLuaFunctionRef& iLuaFunction = CLuaFunctionRef () );

    void                        RemoveAllKeys           ( CLuaMain* pLuaMain );

    static bool                 IsMouse                 ( SBindableKey* pKey );
    void                        RemoveDeletedBinds      ( void );

protected:
    bool                        Remove                  ( CKeyBind* pKeyBind );

    CPlayer*                    m_pPlayer;
    std::list < CKeyBind* >     m_List;
    bool                        m_bProcessingKey;
};

#endif
