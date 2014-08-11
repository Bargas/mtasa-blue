/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CResource.cpp
*  PURPOSE:     Resource object
*  DEVELOPERS:  Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Chris McArthur <>
*               Ed Lyons <eai@opencoding.net>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#define DECLARE_PROFILER_SECTION_CResource
#include "profiler/SharedUtil.Profiler.h"
#include "CServerIdManager.h"

using namespace std;

extern CClientGame* g_pClientGame;

int CResource::m_iShowingCursor = 0;

CResource::CResource ( unsigned short usNetID, const char* szResourceName, CClientEntity* pResourceEntity, CClientEntity* pResourceDynamicEntity, const SString& strMinServerReq, const SString& strMinClientReq, bool bEnableOOP )
{
    m_uiScriptID = CIdArray::PopUniqueId ( this, EIdClass::RESOURCE );
    m_usNetID = usNetID;
    m_bActive = false;
    m_bStarting = true;
    m_bStopping = false;
    m_bInDownloadQueue = false;
    m_bShowingCursor = false;
    m_usRemainingNoClientCacheScripts = 0;
    m_bLoadAfterReceivingNoClientCacheScripts = false;
    m_strMinServerReq = strMinServerReq;
    m_strMinClientReq = strMinClientReq;

    if ( szResourceName )
        m_strResourceName.AssignLeft ( szResourceName, MAX_RESOURCE_NAME_LENGTH );

    m_pLuaManager = g_pClientGame->GetLuaManager();
    m_pRootEntity = g_pClientGame->GetRootEntity ();
    m_pDefaultElementGroup = new CElementGroup (); // for use by scripts
    m_pResourceEntity = pResourceEntity;
    m_pResourceDynamicEntity = pResourceDynamicEntity;

    // Create our GUI root element. We set its parent when we're loaded.
    // Make it a system entity so nothing but us can delete it.
    m_pResourceGUIEntity = new CClientDummy ( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "guiroot" );
    m_pResourceGUIEntity->MakeSystemEntity ();

    // Create our COL root element. We set its parent when we're loaded.
    // Make it a system entity so nothing but us can delete it.
    m_pResourceCOLRoot = new CClientDummy ( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "colmodelroot" );
    m_pResourceCOLRoot->MakeSystemEntity ();

    // Create our DFF root element. We set its parent when we're loaded.
    // Make it a system entity so nothing but us can delete it.
    m_pResourceDFFEntity = new CClientDummy ( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "dffroot" );
    m_pResourceDFFEntity->MakeSystemEntity ();

    // Create our TXD root element. We set its parent when we're loaded.
    // Make it a system entity so nothing but us can delete it.
    m_pResourceTXDRoot = new CClientDummy ( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "txdroot" );
    m_pResourceTXDRoot->MakeSystemEntity ();

    m_strResourceDirectoryPath = SString ( "%s/resources/%s", g_pClientGame->GetFileCacheRoot (), *m_strResourceName );
    m_strResourcePrivateDirectoryPath = PathJoin ( CServerIdManager::GetSingleton ( )->GetConnectionPrivateDirectory (), m_strResourceName );

    m_strResourcePrivateDirectoryPathOld = CServerIdManager::GetSingleton ()->GetConnectionPrivateDirectory ( true );
    if ( !m_strResourcePrivateDirectoryPathOld.empty () )
        m_strResourcePrivateDirectoryPathOld = PathJoin ( m_strResourcePrivateDirectoryPathOld, m_strResourceName );

    // Move this after the CreateVirtualMachine line and heads will roll
    m_bOOPEnabled = bEnableOOP;

    m_pLuaVM = m_pLuaManager->CreateVirtualMachine ( this, bEnableOOP );
    if ( m_pLuaVM )
    {
        m_pLuaVM->SetScriptName ( szResourceName );
    }
}


CResource::~CResource ( void )
{
    CIdArray::PushUniqueId ( this, EIdClass::RESOURCE, m_uiScriptID );
    // Make sure we don't force the cursor on
    ShowCursor ( false );

    // Do this before we delete our elements.
    m_pRootEntity->CleanUpForVM ( m_pLuaVM, true );
    g_pClientGame->GetElementDeleter ()->CleanUpForVM ( m_pLuaVM );
    m_pLuaManager->RemoveVirtualMachine ( m_pLuaVM );

    // Remove all keybinds on this VM
    g_pClientGame->GetScriptKeyBinds ()->RemoveAllKeys ( m_pLuaVM );
    g_pCore->GetKeyBinds()->SetAllCommandsActive ( m_strResourceName, false );

    // Destroy the txd root so all dff elements are deleted except those moved out
    g_pClientGame->GetElementDeleter ()->DeleteRecursive ( m_pResourceTXDRoot );
    m_pResourceTXDRoot = NULL;

    // Destroy the ddf root so all dff elements are deleted except those moved out
    g_pClientGame->GetElementDeleter ()->DeleteRecursive ( m_pResourceDFFEntity );
    m_pResourceDFFEntity = NULL;

    // Destroy the colmodel root so all colmodel elements are deleted except those moved out
    g_pClientGame->GetElementDeleter ()->DeleteRecursive ( m_pResourceCOLRoot );
    m_pResourceCOLRoot = NULL;

    // Destroy the gui root so all gui elements are deleted except those moved out
    g_pClientGame->GetElementDeleter ()->DeleteRecursive ( m_pResourceGUIEntity );
    m_pResourceGUIEntity = NULL;

    // Undo all changes to water
    g_pGame->GetWaterManager ()->UndoChanges ( this );

    // Destroy the element group attached directly to this resource
    if ( m_pDefaultElementGroup )
        delete m_pDefaultElementGroup;
    m_pDefaultElementGroup = NULL;

    m_pRootEntity = NULL;
    m_pResourceEntity = NULL;

    list < CResourceFile* >::iterator iter = m_ResourceFiles.begin ();
    for ( ; iter != m_ResourceFiles.end (); ++iter )
    {
        delete ( *iter );
    }
    m_ResourceFiles.clear ();

    list < CResourceConfigItem* >::iterator iterc = m_ConfigFiles.begin ();
    for ( ; iterc != m_ConfigFiles.end (); ++iterc )
    {
        delete ( *iterc );
    }
    m_ConfigFiles.clear ();

    // Delete the exported functions
    list < CExportedFunction* >::iterator iterExportedFunction = m_exportedFunctions.begin();
    for ( ; iterExportedFunction != m_exportedFunctions.end(); ++iterExportedFunction )
    {
        delete ( *iterExportedFunction );
    }
    m_exportedFunctions.clear();
}

CDownloadableResource* CResource::QueueFile ( CDownloadableResource::eResourceType resourceType, const char *szFileName, CChecksum serverChecksum, bool bAutoDownload )
{
    // Create the resource file and add it to the list
    SString strBuffer ( "%s\\resources\\%s\\%s", g_pClientGame->GetFileCacheRoot (), *m_strResourceName, szFileName );

    // Reject duplicates
    if ( g_pClientGame->GetResourceManager()->IsResourceFile( strBuffer ) )
    {
        g_pClientGame->GetScriptDebugging()->LogError( NULL, "Ignoring duplicate file in resource '%s': '%s'", *m_strResourceName, szFileName );
        return NULL;
    }

    CResourceFile* pResourceFile = new CResourceFile ( resourceType, szFileName, strBuffer, serverChecksum, bAutoDownload );
    if ( pResourceFile )
    {
        m_ResourceFiles.push_back ( pResourceFile );
    }

    return pResourceFile;
}


CDownloadableResource* CResource::AddConfigFile ( const char *szFileName, CChecksum serverChecksum )
{
    // Create the config file and add it to the list
    SString strBuffer ( "%s\\resources\\%s\\%s", g_pClientGame->GetFileCacheRoot (), *m_strResourceName, szFileName );

    // Reject duplicates
    if ( g_pClientGame->GetResourceManager()->IsResourceFile( strBuffer ) )
    {
        g_pClientGame->GetScriptDebugging()->LogError( NULL, "Ignoring duplicate file in resource '%s': '%s'", *m_strResourceName, szFileName );
        return NULL;
    }

    CResourceConfigItem* pConfig = new CResourceConfigItem ( this, szFileName, strBuffer, serverChecksum );
    if ( pConfig )
    {
        m_ConfigFiles.push_back ( pConfig );
    }

    return pConfig;
}

void CResource::AddExportedFunction ( const char *szFunctionName )
{
    m_exportedFunctions.push_back(new CExportedFunction ( szFunctionName ) );
}

bool CResource::CallExportedFunction ( const char * szFunctionName, CLuaArguments& args, CLuaArguments& returns, CResource& caller )
{
    list < CExportedFunction* > ::iterator iter =  m_exportedFunctions.begin ();
    for ( ; iter != m_exportedFunctions.end (); ++iter )
    {
        if ( strcmp ( (*iter)->GetFunctionName(), szFunctionName ) == 0 )
        {
            if ( args.CallGlobal ( m_pLuaVM, szFunctionName, &returns ) )
            {
                return true;
            }
        }
    }
    return false;
}


//
// Quick integrity check of png, dff and txd files
//
static bool CheckFileForCorruption( const SString &strPath, SString &strAppendix )
{
    const char* szExt   = strPath.c_str () + max<long>( 0, strPath.length () - 4 );
    bool bIsBad         = false;

    if ( stricmp ( szExt, ".PNG" ) == 0 )
    {
        // Open the file
        if ( FILE* pFile = fopen ( strPath.c_str (), "rb" ) )
        {
            // This is what the png header should look like
            unsigned char pGoodHeader [8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

             // Load the header
            unsigned char pBuffer [8] = { 0,0,0,0,0,0,0,0 };
            fread ( pBuffer, 1, 8, pFile );

            // Check header integrity
            if ( memcmp ( pBuffer, pGoodHeader, 8 ) )
                bIsBad = true;

            // Close the file
            fclose ( pFile );
        }
    }
    else
    if ( stricmp ( szExt, ".TXD" ) == 0 || stricmp ( szExt, ".DFF" ) == 0 )
    {
        // Open the file
        if ( FILE* pFile = fopen ( strPath.c_str (), "rb" ) )
        {
            struct {
                long id;
                long size;
                long ver;
            } header = {0,0,0};

            // Load the first header
            fread ( &header, 1, sizeof(header), pFile );
            long pos = sizeof(header);
            long validSize = header.size + pos;

            // Step through the sections
            while ( pos < validSize )
            {
                if ( fread ( &header, 1, sizeof(header), pFile ) != sizeof(header) )
                    break;
                fseek ( pFile, header.size, SEEK_CUR );
                pos += header.size + sizeof(header);
            }

            // Check integrity
            if ( pos != validSize )
                bIsBad = true;
               
            // Close the file
            fclose ( pFile );
        }
    }

    return bIsBad;
}


void CResource::Load ( CClientEntity *pRootEntity )
{
    m_pRootEntity = pRootEntity;

    if ( m_usRemainingNoClientCacheScripts > 0 )
    {
        m_bLoadAfterReceivingNoClientCacheScripts = true;
        return;
    }

    if ( m_pRootEntity )
    {
        // Set the GUI parent to the resource root entity
        m_pResourceCOLRoot->SetParent ( m_pResourceEntity );
        m_pResourceDFFEntity->SetParent ( m_pResourceEntity );
        m_pResourceGUIEntity->SetParent ( m_pResourceEntity );
        m_pResourceTXDRoot->SetParent ( m_pResourceEntity );
    }

    CLogger::LogPrintf ( "> Starting resource '%s'", *m_strResourceName );

    // Flag resource files as readable
    for ( std::list < CResourceConfigItem* >::iterator iter = m_ConfigFiles.begin ( ); iter != m_ConfigFiles.end () ; ++iter )
        (*iter)->SetDownloaded();

    for ( std::list < CResourceFile* >::iterator iter = m_ResourceFiles.begin ( ); iter != m_ResourceFiles.end () ; ++iter )
        if ( (*iter)->IsAutoDownload() )
            (*iter)->SetDownloaded();

    // Load config files
    list < CResourceConfigItem* >::iterator iterc = m_ConfigFiles.begin ();
    for ( ; iterc != m_ConfigFiles.end (); ++iterc )
    {
        if ( !(*iterc)->Start() )
        {
            CLogger::LogPrintf ( "Failed to start resource item %s in %s\n", (*iterc)->GetName(), *m_strResourceName );
        }
    }

    // Load the no cache scripts first
    for ( std::list < SNoClientCacheScript >::iterator iter = m_NoClientCacheScriptList.begin() ; iter != m_NoClientCacheScriptList.end() ; ++iter )
    {
        DECLARE_PROFILER_SECTION( OnPreLoadNoClientCacheScript )
        const SNoClientCacheScript& item = *iter;
        GetVM()->LoadScriptFromBuffer ( item.buffer.GetData(), item.buffer.GetSize(), item.strFilename );
        DECLARE_PROFILER_SECTION( OnPostLoadNoClientCacheScript )
    }
    m_NoClientCacheScriptList.clear();

    // Load the files that are queued in the list "to be loaded"
    list < CResourceFile* > ::iterator iter = m_ResourceFiles.begin ();
    for ( ; iter != m_ResourceFiles.end (); ++iter )
    {
        CResourceFile* pResourceFile = *iter;
        // Only load the resource file if it is a client script
        if ( pResourceFile->GetResourceType () == CDownloadableResource::RESOURCE_FILE_TYPE_CLIENT_SCRIPT )
        {
            // Load the file
            std::vector < char > buffer;
            FileLoad ( pResourceFile->GetName (), buffer );
            unsigned int iSize = buffer.size();

            DECLARE_PROFILER_SECTION( OnPreLoadScript )
            // Check the contents
            if ( iSize > 0 && CChecksum::GenerateChecksumFromBuffer ( &buffer.at ( 0 ), iSize ) == pResourceFile->GetServerChecksum () )
            {
                m_pLuaVM->LoadScriptFromBuffer ( &buffer.at ( 0 ), iSize, pResourceFile->GetName () );
            }
            else
            {
                HandleDownloadedFileTrouble( pResourceFile, true );
            }
            DECLARE_PROFILER_SECTION( OnPostLoadScript )
        }
        else
        if ( pResourceFile->IsAutoDownload() )
        {
            // Load the file
            std::vector < char > buffer;
            FileLoad ( pResourceFile->GetName (), buffer );
            unsigned int iSize = buffer.size();

            // Check the contents
            if ( iSize > 0 && CChecksum::GenerateChecksumFromBuffer ( &buffer.at ( 0 ), iSize ) == pResourceFile->GetServerChecksum () )
            {
                SString strError = "";
                bool bIsBad = CheckFileForCorruption ( pResourceFile->GetName ( ), strError );
                if ( bIsBad )
                {
                    HandleDownloadedFileTrouble( pResourceFile, false, strError );
                }
            }
            else
            {
                HandleDownloadedFileTrouble( pResourceFile, true, "" );
            }
        }
    }

    // Set active flag
    m_bActive = true;
    m_bStarting = false;

    // Did we get a resource root entity?
    if ( m_pResourceEntity )
    {
        // Call the Lua "onClientResourceStart" event
        CLuaArguments Arguments;
        Arguments.PushResource ( this );
        m_pResourceEntity->CallEvent ( "onClientResourceStart", Arguments, true );
    }
    else
        assert ( 0 );
}


void CResource::Stop( void )
{
    m_bStarting = false;
    m_bStopping = true;
    CLuaArguments Arguments;
    Arguments.PushResource ( this );
    m_pResourceEntity->CallEvent ( "onClientResourceStop", Arguments, true );
}


SString CResource::GetState ( void )
{
    if ( m_bStarting )
        return "starting";
    else if ( m_bStopping ) 
        return "stopping";
    else if ( m_bActive ) 
        return "running";
    else
        return "loaded";
}


void CResource::DeleteClientChildren ( void )
{
    // Run this on our resource entity if we have one
    if ( m_pResourceEntity )
        m_pResourceEntity->DeleteClientChildren ();
}


void CResource::ShowCursor ( bool bShow, bool bToggleControls )
{
    // Different cursor showing state than earlier?
    if ( bShow != m_bShowingCursor )
    {
        // Going to show the cursor?
        if ( bShow )
        {
            // Increase the cursor ref count
            m_iShowingCursor += 1;
        }
        else
        {

            // Decrease the cursor ref count
            m_iShowingCursor -= 1;
        }

        // Update our showing cursor state
        m_bShowingCursor = bShow;

        // Show cursor if more than 0 resources wanting the cursor on
        g_pCore->ForceCursorVisible ( m_iShowingCursor > 0, bToggleControls );
        g_pClientGame->SetCursorEventsEnabled ( m_iShowingCursor > 0 );
    }
}


SString CResource::GetResourceDirectoryPath ( eAccessType accessType, const SString& strMetaPath )
{
    // See if private files should be moved to a new directory
    if ( accessType == ACCESS_PRIVATE )
    {
        if ( !m_strResourcePrivateDirectoryPathOld.empty () )
        {
            SString strNewFilePath = PathJoin ( m_strResourcePrivateDirectoryPath, strMetaPath );
            SString strOldFilePath = PathJoin ( m_strResourcePrivateDirectoryPathOld, strMetaPath );

            if ( FileExists ( strOldFilePath ) )
            {
                if ( FileExists ( strNewFilePath ) )
                {
                    // If file exists in old and new, delete from old
                    OutputDebugLine ( SString ( "Deleting %s", *strOldFilePath ) );
                    FileDelete ( strOldFilePath );
                }
                else
                {
                    // If file exists in old only, move from old to new
                    OutputDebugLine ( SString ( "Moving %s to %s", *strOldFilePath, *strNewFilePath ) );
                    MakeSureDirExists ( strNewFilePath );
                    FileRename ( strOldFilePath, strNewFilePath );
                }
            }
        }
        return PathJoin ( m_strResourcePrivateDirectoryPath, strMetaPath );
    }
    return PathJoin ( m_strResourceDirectoryPath, strMetaPath );
}


void CResource::LoadNoClientCacheScript ( const char* chunk, unsigned int len, const SString& strFilename )
{
    if ( m_usRemainingNoClientCacheScripts > 0 )
    {
        --m_usRemainingNoClientCacheScripts;

        // Store for later
        m_NoClientCacheScriptList.push_back( SNoClientCacheScript() );
        SNoClientCacheScript& item = m_NoClientCacheScriptList.back();
        item.buffer = CBuffer( chunk, len );
        item.strFilename = strFilename;

        if ( m_usRemainingNoClientCacheScripts == 0 && m_bLoadAfterReceivingNoClientCacheScripts )
        {
            m_bLoadAfterReceivingNoClientCacheScripts = false;
            Load ( m_pRootEntity );
        }
    }
}


//
// Add element to the default element group 
//
void CResource::AddToElementGroup ( CClientEntity* pElement )
{
    if ( m_pDefaultElementGroup )
    {
        m_pDefaultElementGroup->Add ( pElement );
    }
}


//
// Handle when things go wrong 
//
void CResource::HandleDownloadedFileTrouble( CResourceFile* pResourceFile, bool bCRCMismatch, const SString &strAppendix )
{
    // Compose message
    SString strMessage;
    if ( bCRCMismatch )
    {
        if ( g_pClientGame->IsUsingExternalHTTPServer() )
            strMessage += "External ";
        strMessage += "HTTP server file mismatch";
    }
    else
        strMessage += "Invalid file";
    SString strFilename = ExtractFilename( PathConform( pResourceFile->GetShortName() ) );
    strMessage += SString( " (%s) %s %s", GetName(), *strFilename, *strAppendix );

    if ( !bCRCMismatch )
    {
        // For corrupt files, log to the client console
        g_pClientGame->TellServerSomethingImportant( 1000, strMessage, true );
        g_pCore->GetConsole()->Printf( "Download error: %s", *strMessage );
        return;
    }

    // If using external HTTP server, reconnect and use internal one
    if ( g_pClientGame->IsUsingExternalHTTPServer() && !g_pCore->ShouldUseInternalHTTPServer() )
    {
        g_pClientGame->TellServerSomethingImportant( 1001, strMessage, true );
        g_pCore->Reconnect( "", 0, NULL, false, true );
    }
    else
    {
        // Otherwise, log to the client console
        g_pClientGame->TellServerSomethingImportant( 1002, strMessage, true );
        g_pCore->GetConsole ()->Printf ( "Download error: %s", *strMessage );
    }
}
