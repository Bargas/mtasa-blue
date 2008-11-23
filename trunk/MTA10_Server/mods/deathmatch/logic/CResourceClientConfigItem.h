/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CResourceClientConfigItem.h
*  PURPOSE:     Resource client-side (XML) configuration file item class
*  DEVELOPERS:  Ed Lyons <>
*               Kevin Whiteside <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef CRESOURCECLIENTCONFIGITEM_H
#define CRESOURCECLIENTCONFIGITEM_H

#include "CGame.h"
#include "CResourceFile.h"
#include <list>

using namespace std;

class CResourceClientConfigItem : public CResourceFile
{
    
public:

                                        CResourceClientConfigItem       ( class CResource * resource, char * szShortName, char * szResourceFileName );
                                        ~CResourceClientConfigItem      ( void );

    bool                                Start                           ( void );
    bool                                Stop                            ( void );
    inline class CXMLNode *             GetRoot                         ( void ) { return m_pXMLRootNode; }

private:
    class CXMLFile*                     m_pXMLFile;
    CXMLNode*                           m_pXMLRootNode;
    char                                m_szConfigName [ MAX_PATH ];

    bool                                m_bInvalid; // it failed to load
};

#endif

