/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/lua/CLuaTimerManager.h
*  PURPOSE:     Lua timer manager class
*  DEVELOPERS:  Oliver Brown <>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CLuaTimerManager;

#ifndef __CLUATIMERMANAGER_H
#define __CLUATIMERMANAGER_H

#include "LuaCommon.h"
#include "CLuaTimer.h"
#include <list>

class CLuaTimerManager
{
public:
    inline                      CLuaTimerManager                ( void )                    { m_bIteratingList = false; }
    inline                      ~CLuaTimerManager               ( void )                    { RemoveAllTimers (); };

    void                        DoPulse                         ( CLuaMain* pLuaMain );

    bool                        Exists                          ( CLuaTimer* pLuaTimer );
    CLuaTimer*                  GetTimer                        ( unsigned int uiID );

    CLuaTimer*                  AddTimer                        ( lua_State* luaVM );
    void                        RemoveTimer                     ( CLuaTimer* pLuaTimer );
    void                        RemoveAllTimers                 ( void );
    unsigned long               GetTimerCount                   ( void ) const              { return m_TimerList.size (); }

    void                        ResetTimer                      ( CLuaTimer* pLuaTimer );

    void                        GetTimers                       ( unsigned long ulTime, lua_State* luaVM );

    void                        TakeOutTheTrash                 ( void );
private:
    CMappedList < CLuaTimer* >  m_TimerList;
    std::list < CLuaTimer* >    m_TrashCan;
    bool                        m_bIteratingList;
};

#endif
