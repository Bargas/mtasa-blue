/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CWhoWas.h
*  PURPOSE:     "Who-was" nick/ip/port logging class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CWHOWAS_H
#define __CWHOWAS_H

#include "../Config.h"
#include <list>

using namespace std;

struct SWhoWasEntry
{
    char            szNick [MAX_NICK_LENGTH + 1];
    unsigned long   ulIP;
    unsigned short  usPort;
};

class CWhoWas
{
public:
    void                                            Add                 ( const char* szNick, unsigned long ulIP, unsigned short usPort );
    inline void                                     Clear               ( void )        { m_List.clear (); };

    inline unsigned int                             Count               ( void )        { return m_List.size (); };
    inline list < SWhoWasEntry > ::const_iterator   IterBegin           ( void )        { return m_List.begin (); };
    inline list < SWhoWasEntry > ::const_iterator   IterEnd             ( void )        { return m_List.end (); };

private:
    list < SWhoWasEntry >                           m_List;
};

#endif
