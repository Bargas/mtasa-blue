/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CResource.h
*  PURPOSE:     Resource object class
*  DEVELOPERS:  Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Chris McArthur <>
*               Ed Lyons <eai@opencoding.net>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef CRESOURCE_H
#define CRESOURCE_H

#include "..\shared_logic\lua\CLuaManager.h"
#include "CClientEntity.h"
#include "CResourceConfigItem.h"
#include "CResourceFile.h"
#include "CElementGroup.h"
#include <list>

#define MAX_RESOURCE_NAME_LENGTH    255
#define MAX_FUNCTION_NAME_LENGTH    50
#define INVALID_COMPILED_SCRIPT_CUTOFF_DATE     "2014-08-31"

class CExportedFunction
{
private:
    SString            m_strFunctionName;
public:
    CExportedFunction ( const char* szFunctionName )
    {
        m_strFunctionName.AssignLeft ( szFunctionName, MAX_FUNCTION_NAME_LENGTH );
    };

    const char* GetFunctionName ( void ) { return m_strFunctionName; }
};

class CResource
{  

public:
                            CResource       ( unsigned short usNetID, const char* szResourceName, CClientEntity* pResourceEntity, CClientEntity* pResourceDynamicEntity, const SString& strMinServerReq, const SString& strMinClientReq, bool bEnableOOP );
                            ~CResource      ( void );

    inline unsigned short   GetNetID        ( void )                { return m_usNetID; };
    uint                    GetScriptID     ( void ) const          { return m_uiScriptID; };
    const char*             GetName         ( void )                { return m_strResourceName; };
    inline CLuaMain*        GetVM           ( void )                { return m_pLuaVM; };
    inline bool             GetActive       ( void )                { return m_bActive; };

    void                    Load            ( CClientEntity *pRootEntity );

    bool                    InDownloadQueue     ( void )            { return m_bInDownloadQueue; };
    bool                    SetInDownloadQueue  ( bool bIn )        { m_bInDownloadQueue = bIn; };

    CDownloadableResource*  QueueFile       ( CDownloadableResource::eResourceType resourceType, const char *szFileName, CChecksum serverChecksum, bool bAutoDownload = true );

    CDownloadableResource*  AddConfigFile   ( const char *szFileName, CChecksum serverChecksum );

    inline std::list < class CResourceConfigItem* >::iterator    ConfigIterBegin     ( void )        { return m_ConfigFiles.begin(); }
    inline std::list < class CResourceConfigItem* >::iterator    ConfigIterEnd       ( void )        { return m_ConfigFiles.end(); }

    CElementGroup *         GetElementGroup                 ( void )                        { return m_pDefaultElementGroup; }
    void                    AddToElementGroup               ( CClientEntity* pElement );

    void                    AddExportedFunction ( const char * szFunctionName );
    bool                    CallExportedFunction ( const char * szFunctionName, CLuaArguments& args, CLuaArguments& returns, CResource& caller );

    class CClientEntity*    GetResourceEntity ( void )              { return m_pResourceEntity; }
    void                    SetResourceEntity ( CClientEntity* pEntity )    { m_pResourceEntity = pEntity; }
    class CClientEntity*    GetResourceDynamicEntity ( void )                       { return m_pResourceDynamicEntity; }
    void                    SetResourceDynamicEntity ( CClientEntity* pEntity )     { m_pResourceDynamicEntity = pEntity; }
    SString                 GetResourceDirectoryPath ( eAccessType accessType, const SString& strMetaPath );
    class CClientEntity*    GetResourceGUIEntity ( void )                   { return m_pResourceGUIEntity; }
    void                    SetResourceGUIEntity      ( CClientEntity* pEntity )    { m_pResourceGUIEntity = pEntity; }
    inline CClientEntity*   GetResourceCOLModelRoot ( void )                           { return m_pResourceCOLRoot; };
    inline CClientEntity*   GetResourceDFFRoot ( void )                           { return m_pResourceDFFEntity; };
    inline CClientEntity*   GetResourceTXDRoot ( void )                           { return m_pResourceTXDRoot; };

    // This is to delete all the elements created in this resource that are created locally in this client
    void                    DeleteClientChildren        ( void );

    // Use this for cursor showing/hiding
    void                    ShowCursor                  ( bool bShow, bool bToggleControls = true );

    inline std::list < CExportedFunction* >::iterator    IterBeginExportedFunctions   ( void )        { return m_exportedFunctions.begin(); }
    inline std::list < CExportedFunction* >::iterator    IterEndExportedFunctions     ( void )        { return m_exportedFunctions.end(); }

    inline std::list < CResourceFile* >::iterator    IterBeginResourceFiles   ( void )        { return m_ResourceFiles.begin(); }
    inline std::list < CResourceFile* >::iterator    IterEndResourceFiles     ( void )        { return m_ResourceFiles.end(); }

    void                    SetRemainingNoClientCacheScripts    ( unsigned short usRemaining ) { m_usRemainingNoClientCacheScripts = usRemaining; }
    void                    LoadNoClientCacheScript         ( const char* chunk, unsigned int length, const SString& strFilename );
    const SString&          GetMinServerReq                 ( void ) const                  { return m_strMinServerReq; }
    const SString&          GetMinClientReq                 ( void ) const                  { return m_strMinClientReq; }
    bool                    IsOOPEnabled                    ( void )                        { return m_bOOPEnabled; }
    void                    HandleDownloadedFileTrouble     ( CResourceFile* pResourceFile, bool bCRCMismatch );

private:
    unsigned short          m_usNetID;
    uint                    m_uiScriptID;
    SString                 m_strResourceName;
    CLuaMain*               m_pLuaVM;
    CLuaManager*            m_pLuaManager;
    class CClientEntity*    m_pRootEntity;
    bool                    m_bActive;
    class CClientEntity*    m_pResourceEntity;          // no idea what this is used for anymore
    class CClientEntity*    m_pResourceDynamicEntity;   // parent of elements created by the resource
    class CClientEntity*    m_pResourceCOLRoot;
    class CClientEntity*    m_pResourceDFFEntity;
    class CClientEntity*    m_pResourceGUIEntity;
    class CClientEntity*    m_pResourceTXDRoot;
    bool                    m_bInDownloadQueue;
    unsigned short          m_usRemainingNoClientCacheScripts;
    bool                    m_bLoadAfterReceivingNoClientCacheScripts;
    SString                 m_strMinServerReq;
    SString                 m_strMinClientReq;
    bool                    m_bOOPEnabled;

    // To control cursor show/hide
    static int              m_iShowingCursor;
    bool                    m_bShowingCursor;

    SString                 m_strResourceDirectoryPath;             // stores the path to /mods/deathmatch/resources/resource_name
    SString                 m_strResourcePrivateDirectoryPath;      // stores the path to /mods/deathmatch/priv/server-id/resource_name
    SString                 m_strResourcePrivateDirectoryPathOld;   // stores the path to /mods/deathmatch/priv/old-server-id/resource_name

    std::list < class CResourceFile* >          m_ResourceFiles;
    std::list < class CResourceConfigItem* >    m_ConfigFiles;
    std::list<CExportedFunction *>              m_exportedFunctions;
    CElementGroup *                             m_pDefaultElementGroup;     // stores elements created by scripts in this resource
};

#endif
