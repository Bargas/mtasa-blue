/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CClientGame.cpp
*  PURPOSE:     Client game manager
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Kent Simon <>
*               Ed Lyons <eai@opencoding.net>
*               Oliver Brown <>
*               Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               Chris McArthur <>
*               Derek Abdine <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*               Sebas Lamers <sebasdevelopment@gmx.com>
*               Cazomino05 <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include <net/SyncStructures.h>

using SharedUtil::CalcMTASAPath;
using std::list;
using std::vector;

// Hide the "conversion from 'unsigned long' to 'DWORD*' of greater size" warning
#pragma warning(disable:4312)

// Used within this file by the packet handler to grab the this pointer of CClientGame
extern CClientGame* g_pClientGame;

// Used by the Voice system
#ifdef MTA_VOICE
CVariableBuffer * CClientGame::m_pVoiceBuffer;
CRITICAL_SECTION CClientGame::m_crVoice;
#endif

bool g_bBoundsChecker = true;
#define DEFAULT_GRAVITY 0.008f
#define DEFAULT_GAME_SPEED 1.0f
#define DEFAULT_BLUR_LEVEL 36
#define DEFAULT_JETPACK_MAXHEIGHT 100
#define DEFAULT_MINUTE_DURATION 1000
#define DOUBLECLICK_TIMEOUT 330
#define DOUBLECLICK_MOVE_THRESHOLD 10.0f

CClientGame::CClientGame ( bool bLocalPlay )
{
    // Init the global var with ourself
    g_pClientGame = this;

    // Packet handler
    m_pPacketHandler = new CPacketHandler ();

    // Init
    m_bLocalPlay = bLocalPlay;
    m_bErrorStartingLocal = false;
    m_iLocalConnectAttempts = 0;
    m_fMarkerBounce = 0.0f;
    m_Status = CClientGame::STATUS_CONNECTING;
    m_ulVerifyTimeStart = 0;
    m_ulLastClickTick = 0;
    m_pLocalPlayer = NULL;
    m_LocalID = INVALID_ELEMENT_ID;
    m_szLocalNick [0] = 0;
    m_szCurrentMapName [0] = 0;
    m_bShowNametags = true;
    m_bWaitingForLocalConnect = false;
    m_bShowRadar = false;
    m_bGameLoaded = false;
    m_bTriggeredIngameAndConnected = false;
    m_bGracefulDisconnect = false;
    m_ulLastVehicleInOutTime = 0;
    m_bIsGettingOutOfVehicle = false;
    m_bIsGettingIntoVehicle = false;
    m_bIsGettingJacked = false;
    m_bIsJackingVehicle = false;
    m_VehicleInOutID = INVALID_ELEMENT_ID;
    m_pGettingJackedBy = NULL;
    m_ucVehicleInOutSeat = 0xFF;
    m_pTargetedEntity = NULL;
    m_TargetedPlayerID = INVALID_ELEMENT_ID;
    m_pDamageEntity = NULL;
    m_DamagerID = INVALID_ELEMENT_ID;
    m_ucDamageBodyPiece = 0xFF;
    m_ucDamageWeapon = 0xFF;
    m_ulDamageTime = 0;
    m_bDamageSent = true;
    m_bShowNetstat = false;
    m_bShowFPS = false;    
    m_bHudAreaNameDisabled = false;
    m_fGameSpeed = 1.0f;
    m_lMoney = 0;
    m_lastWeaponSlot = WEAPONSLOT_MAX;      // last stored weapon slot, for weapon slot syncing to server (sets to invalid value)
    ResetAmmoInClip();

    m_bNoNewVehicleTask = false;
    m_bTransferResource = false;            // flag controls whether a resource is being transferred or not
    m_bTransferInitiated = false;           // flag controls whether a transfer has been initiated (to prevent PacketResource, AUTOPATCHER_REQUEST_FILES priority issues)

    m_dwTransferStarted = 0;                // timestamp for transfer start
    m_bTransferReset = false;               // flag controls whether we have to reset the transfer counter

    m_bCursorEventsEnabled = false;
    m_bInitiallyFadedOut = true;

    m_bIsPlayingBack = false;
    m_bFirstPlaybackFrame = false;

    //Setup game glitch defaults ( false = disabled ).  Remember to update these serverside if you alter them!
    m_Glitches [ GLITCH_QUICKRELOAD ] = false;
    g_pMultiplayer->DisableQuickReload ( true );
    m_Glitches [ GLITCH_FASTFIRE ] = false;
    m_Glitches [ GLITCH_FASTMOVE ] = false;
    m_Glitches [ GLITCH_CROUCHBUG ] = false;

    // Remove Night & Thermal vision view (if enabled).
    g_pMultiplayer->SetNightVisionEnabled ( false );
    g_pMultiplayer->SetThermalVisionEnabled ( false );

    m_bCloudsEnabled = true;

    m_uiNotPulsedCounter = 0;

    #ifdef MTA_VOICE
    m_pVoice = VoiceCreate();
    // Initialize the voice module for this mod.
    if ( m_pVoice )
    {
        g_pCore->GetConsole()->Printf("Loaded voice module [%s]",
            m_pVoice->GetVersion());
        m_pVoice->Initialize(512);
        m_pVoice->RegisterSendHandler(SendVoiceData);
        m_pVoice->SetMuted(false);
        m_pVoice->Enable();
    }
    InitializeCriticalSection(&m_crVoice);
    m_pVoiceBuffer = new CVariableBuffer( 65535 );
    #endif

    // Grab the mod path
    m_strModRoot = g_pCore->GetModInstallRoot ( "deathmatch" );

    // Override CGUI's global events
    g_pCore->GetGUI ()->SetKeyDownHandler           ( INPUT_MOD, GUI_CALLBACK_KEY ( &CClientGame::OnKeyDown, this ) );
    g_pCore->GetGUI ()->SetMouseClickHandler        ( INPUT_MOD, GUI_CALLBACK_MOUSE ( &CClientGame::OnMouseClick, this ) );
    g_pCore->GetGUI ()->SetMouseDoubleClickHandler  ( INPUT_MOD, GUI_CALLBACK_MOUSE ( &CClientGame::OnMouseDoubleClick, this ) );
    g_pCore->GetGUI ()->SetMouseButtonDownHandler   ( INPUT_MOD, GUI_CALLBACK_MOUSE ( &CClientGame::OnMouseButtonDown, this ) );
    g_pCore->GetGUI ()->SetMouseButtonUpHandler     ( INPUT_MOD, GUI_CALLBACK_MOUSE ( &CClientGame::OnMouseButtonUp, this ) );
    g_pCore->GetGUI ()->SetMouseMoveHandler         ( INPUT_MOD, GUI_CALLBACK_MOUSE ( &CClientGame::OnMouseMove, this ) );
    g_pCore->GetGUI ()->SetMouseEnterHandler        ( INPUT_MOD, GUI_CALLBACK_MOUSE ( &CClientGame::OnMouseEnter, this ) );
    g_pCore->GetGUI ()->SetMouseLeaveHandler        ( INPUT_MOD, GUI_CALLBACK_MOUSE ( &CClientGame::OnMouseLeave, this ) );
    g_pCore->GetGUI ()->SetMouseWheelHandler        ( INPUT_MOD, GUI_CALLBACK_MOUSE ( &CClientGame::OnMouseWheel, this ) );
    g_pCore->GetGUI ()->SetMovedHandler             ( INPUT_MOD, GUI_CALLBACK ( &CClientGame::OnMove, this ) );
    g_pCore->GetGUI ()->SetSizedHandler             ( INPUT_MOD, GUI_CALLBACK ( &CClientGame::OnSize, this ) );
    g_pCore->GetGUI ()->SetFocusGainedHandler       ( INPUT_MOD, GUI_CALLBACK_FOCUS ( &CClientGame::OnFocusGain, this ) );
    g_pCore->GetGUI ()->SetFocusLostHandler         ( INPUT_MOD, GUI_CALLBACK_FOCUS ( &CClientGame::OnFocusLoss, this ) );
    g_pCore->GetGUI ()->SelectInputHandlers         ( INPUT_MOD );

    // Startup "entities from root" optimization for getElementsByType
    CClientEntity::StartupEntitiesFromRoot ();

    // Initialize our root entity with an invalid id, we dont know the true id until map-start
    m_pRootEntity = new CClientDummy ( NULL, INVALID_ELEMENT_ID, "root" );
    m_pRootEntity->MakeSystemEntity ();

    // Movings objects manager
    m_pMovingObjectsManager = new CMovingObjectsManager ();

    // Create the manager and grab the most important pointers
    m_pManager = new CClientManager;
    m_pCamera = m_pManager->GetCamera ();
    m_pMarkerManager = m_pManager->GetMarkerManager ();
    m_pObjectManager = m_pManager->GetObjectManager ();
    m_pPickupManager = m_pManager->GetPickupManager ();
    m_pPlayerManager = m_pManager->GetPlayerManager ();
    m_pRadarAreaManager = m_pManager->GetRadarAreaManager ();
    m_pDisplayManager = m_pManager->GetDisplayManager ();
    m_pVehicleManager = m_pManager->GetVehicleManager ();
    m_pRadarMarkerManager = m_pManager->GetRadarMarkerManager ();
    m_pPathManager = m_pManager->GetPathManager ();
    m_pTeamManager = m_pManager->GetTeamManager ();
    m_pPedManager = m_pManager->GetPedManager ();
    m_pGUIManager = m_pManager->GetGUIManager ();
    m_pResourceManager = m_pManager->GetResourceManager ();
    m_pProjectileManager = m_pManager->GetProjectileManager ();
    m_pLocalServer = NULL;

    m_pZoneNames = new CZoneNames;
    m_pScriptKeyBinds = new CScriptKeyBinds;
    m_pScriptFontLoader = new CScriptFontLoader;

    // Create our net API
    m_pNetAPI = new CNetAPI ( m_pManager );
    m_pNetworkStats = new CNetworkStats ( m_pDisplayManager );
    m_pSyncDebug = new CSyncDebug ( m_pManager );

    // Create our blended weather class
    m_pBlendedWeather = new CBlendedWeather;

    // Create our RPC class
    m_pRPCFunctions = new CRPCFunctions ( this );

    // Our management classes
    m_pUnoccupiedVehicleSync = new CUnoccupiedVehicleSync ( m_pVehicleManager );
    m_pPedSync = new CPedSync ( m_pPedManager );
    m_pObjectSync = new CObjectSync ( m_pObjectManager );
    m_pNametags = new CNametags ( m_pManager );
    m_pRadarMap = new CRadarMap ( m_pManager );

    // Set the screenshot path
    /* This is now done in CCore, to maintain a global screenshot path
    SString strScreenShotPath = SString::Printf ( "%s\\screenshots", m_szModRoot );
    g_pCore->SetScreenShotPath ( strScreenShotPath );
    */

    // Create the transfer boxes (GUI)
    m_pTransferBox = new CTransferBox ();
    m_pBigPacketTransferBox = new CTransferBox ();

    // Store the time we started on
    if ( bLocalPlay )
        m_ulTimeStart = 0;
    else
        m_ulTimeStart = CClientTime::GetTime ();

    m_dwFrameTimeSlice = 0;
    m_dwLastFrameTick = 0;
    m_llLastTransgressionTime = 0;

    // Register the message and the net packet handler
    g_pMultiplayer->SetPreWeaponFireHandler ( CClientGame::PreWeaponFire );
    g_pMultiplayer->SetPostWeaponFireHandler ( CClientGame::PostWeaponFire );
    g_pMultiplayer->SetBulletImpactHandler ( CClientGame::BulletImpact );
    g_pMultiplayer->SetExplosionHandler ( CClientExplosionManager::Hook_StaticExplosionCreation );
    g_pMultiplayer->SetBreakTowLinkHandler ( CClientGame::StaticBreakTowLinkHandler );
    g_pMultiplayer->SetDrawRadarAreasHandler ( CClientGame::StaticDrawRadarAreasHandler );
    g_pMultiplayer->SetDamageHandler ( CClientGame::StaticDamageHandler );
    g_pMultiplayer->SetFireHandler ( CClientGame::StaticFireHandler );
    g_pMultiplayer->SetProjectileStopHandler ( CClientProjectileManager::Hook_StaticProjectileAllow );
    g_pMultiplayer->SetProjectileHandler ( CClientProjectileManager::Hook_StaticProjectileCreation );
    g_pMultiplayer->SetRender3DStuffHandler ( CClientGame::StaticRender3DStuffHandler );
    g_pMultiplayer->SetChokingHandler ( CClientGame::StaticChokingHandler );
    g_pMultiplayer->SetPreWorldProcessHandler ( CClientGame::StaticPreWorldProcessHandler );
    g_pMultiplayer->SetPostWorldProcessHandler ( CClientGame::StaticPostWorldProcessHandler );
    g_pMultiplayer->SetIdleHandler ( CClientGame::StaticIdleHandler );
    g_pMultiplayer->SetAddAnimationHandler ( CClientGame::StaticAddAnimationHandler );
    g_pMultiplayer->SetBlendAnimationHandler ( CClientGame::StaticBlendAnimationHandler );
    g_pMultiplayer->SetProcessCollisionHandler ( CClientGame::StaticProcessCollisionHandler );
    m_pProjectileManager->SetInitiateHandler ( CClientGame::StaticProjectileInitiateHandler );
    g_pCore->SetMessageProcessor ( CClientGame::StaticProcessMessage );
    g_pNet->RegisterPacketHandler ( CClientGame::StaticProcessPacket );

    m_pLuaManager = new CLuaManager ( this );
    m_pScriptDebugging = new CScriptDebugging ( m_pLuaManager );

    #if defined (MTA_SCRIPT_LOGGING)
    m_pScriptDebugging->SetLogfile ( CalcMTASAPath("mta\\clientscript.log"), 3 );
    #endif

    m_pLuaManager->SetScriptDebugging ( m_pScriptDebugging );
    CStaticFunctionDefinitions::CStaticFunctionDefinitions ( m_pLuaManager, &m_Events, g_pCore, g_pGame, this, m_pManager );
    CLuaFunctionDefs::Initialize ( m_pLuaManager, m_pScriptDebugging, this );
    CLuaDefs::Initialize ( this, m_pLuaManager, m_pScriptDebugging );

    // Disable the enter/exit vehicle key button (we want to handle this button ourselves)
    g_pMultiplayer->DisableEnterExitVehicleKey ( true );

    // Disable GTA's pickup processing as we want to confirm the hits with the server
    m_pPickupManager->SetPickupProcessingDisabled ( true );

    // Key-bind for fire-key (for handling satchels and stealth-kills)
    g_pCore->GetKeyBinds ()->AddControlFunction ( "fire", CClientGame::StaticUpdateFireKey, true );

    // Init big packet progress vars
    m_bReceivingBigPacket           = false;
    m_ulBigPacketSize               = 0;
    m_ulBigPacketBytesReceivedBase  = 0;

    m_bBeingDeleted = false;

    #if defined (MTA_DEBUG) || defined (MTA_BETA)
    m_bShowSyncingInfo = false;
    #endif

    #ifdef MTA_DEBUG
    m_pShowPlayer = m_pShowPlayerTasks = NULL;
    m_bMimicLag = false;
    m_ulLastMimicLag = 0;
    m_bDoPaintballs = false;
    m_bShowInterpolation = false;
    #endif

    // Add our lua events
    AddBuiltInEvents ();

    // Init debugger class
    m_Foo.Init ( this );

    // Load some stuff from the core config
    float fScale;
    g_pCore->GetCVars ()->Get ( "text_scale", fScale );
    CClientTextDisplay::SetGlobalScale ( fScale );

    // Reset async loading script settings to default
    g_pGame->SetAsyncLoadingFromScript ( false, false );
}


CClientGame::~CClientGame ( void )
{
    m_bBeingDeleted = true;
    // Stop all explosions. Unfortunately this doesn't fix the crash
    // if a vehicle is destroyed while it explodes.
    g_pGame->GetExplosionManager ()->RemoveAllExplosions ();

    // Stop playing the continious sounds
    // if the game was loaded. This is done by
    // playing these special IDS.
    if ( m_bGameLoaded )
    {
        g_pGame->GetAudio ()->PlayFrontEndSound ( 35 );
        g_pGame->GetAudio ()->PlayFrontEndSound ( 48 );
    }

    // Reset the GUI input mode
    g_pCore->GetGUI ()->SetGUIInputMode ( INPUTMODE_ALLOW_BINDS );

    // Reset CGUI's global events
    g_pCore->GetGUI ()->ClearInputHandlers ( INPUT_MOD );

    // Destroy mimics
    #ifdef MTA_DEBUG
        list < CClientPlayer* > ::const_iterator iterMimics = m_Mimics.begin ();
        for ( ; iterMimics != m_Mimics.end (); iterMimics++ )
        {
            CClientPlayer* pPlayer = *iterMimics;
            CClientVehicle* pVehicle = pPlayer->GetOccupiedVehicle ();
            if ( pVehicle )
                delete pVehicle;

            delete pPlayer;
        }
    #endif

    // Hide the transfer box incase it is showing
    m_pTransferBox->Hide();
    m_pBigPacketTransferBox->Hide();

    #ifdef MTA_VOICE
    if ( m_pVoice )
    {
        g_pCore->GetConsole()->Print("destroy..");
        m_pVoice->Disable();
        m_pVoice->Shutdown();
        DeleteCriticalSection(&m_crVoice);
        delete m_pVoiceBuffer;
        VoiceDestroy();
    }
    #endif

    // NULL the message/net stuff
    g_pMultiplayer->SetPreContextSwitchHandler ( NULL );
    g_pMultiplayer->SetPostContextSwitchHandler ( NULL );
    g_pMultiplayer->SetPreWeaponFireHandler ( NULL );
    g_pMultiplayer->SetPostWeaponFireHandler ( NULL );
    g_pMultiplayer->SetBulletImpactHandler ( NULL );
    g_pMultiplayer->SetExplosionHandler ( NULL );
    g_pMultiplayer->SetBreakTowLinkHandler ( NULL );
    g_pMultiplayer->SetDrawRadarAreasHandler ( NULL );
    g_pMultiplayer->SetDamageHandler ( NULL );
    g_pMultiplayer->SetFireHandler ( NULL );
    g_pMultiplayer->SetProjectileStopHandler ( NULL );
    g_pMultiplayer->SetProjectileHandler ( NULL );
    g_pMultiplayer->SetRender3DStuffHandler ( NULL );
    g_pMultiplayer->SetChokingHandler ( NULL );
    g_pMultiplayer->SetPreWorldProcessHandler (  NULL );
    g_pMultiplayer->SetPostWorldProcessHandler (  NULL );
    g_pMultiplayer->SetIdleHandler ( NULL );
    g_pMultiplayer->SetAddAnimationHandler ( NULL );
    g_pMultiplayer->SetBlendAnimationHandler ( NULL );
    g_pMultiplayer->SetProcessCollisionHandler ( NULL );
    m_pProjectileManager->SetInitiateHandler ( NULL );
    g_pCore->SetMessageProcessor ( NULL );
    g_pNet->StopNetwork ();
    g_pNet->RegisterPacketHandler ( NULL );
    CKeyBindsInterface * pKeyBinds = g_pCore->GetKeyBinds ();
    pKeyBinds->RemoveAllFunctions ( CClientGame::StaticProcessClientKeyBind );
    pKeyBinds->RemoveAllControlFunctions ( CClientGame::StaticProcessClientControlBind );
    pKeyBinds->RemoveAllFunctions ( CClientGame::StaticProcessServerKeyBind );
    pKeyBinds->RemoveAllControlFunctions ( CClientGame::StaticProcessServerControlBind );
    pKeyBinds->RemoveAllControlFunctions ( CClientGame::StaticUpdateFireKey );
    pKeyBinds->SetAllControlsEnabled ( true, true, true );
    g_pCore->ForceCursorVisible ( false );
    SetCursorEventsEnabled ( false );   

    // Destroy our stuff
    delete m_pNametags;
    delete m_pSyncDebug;
    delete m_pNetworkStats;
    delete m_pNetAPI;
    delete m_pManager;
    delete m_pRPCFunctions;
    delete m_pUnoccupiedVehicleSync;
    delete m_pPedSync;
    delete m_pObjectSync;
    delete m_pBlendedWeather;
    m_pBlendedWeather = NULL;
    delete m_pMovingObjectsManager;
    delete m_pRadarMap;
    delete m_pLuaManager;

    delete m_pRootEntity;

    delete m_pZoneNames;
    delete m_pScriptKeyBinds;    
    delete m_pScriptFontLoader;    

    // Delete the scriptdebugger
    delete m_pScriptDebugging;

    // Delete the transfer boxes
    delete m_pTransferBox;
    delete m_pBigPacketTransferBox;

    if ( m_pLocalServer )
    {
        delete m_pLocalServer;
    }

    // Packet handler
    delete m_pPacketHandler;

    // NULL the global CClientGame var
    g_pClientGame = NULL;
    m_bBeingDeleted = false;
}

/*
bool CClientGame::StartGame ( void ) // for an offline game (e.g. editor)
{
    m_Status = STATUS_OFFLINE;
    g_pCore->SetOfflineMod ( true ); // hide chatbox etc
    g_pCore->SetConnected ( true ); // not sure, but its required :)
    g_pCore->HideMainMenu (); // duh

    // If the game isn't started, start it
    if ( g_pGame->GetSystemState () == 7 )
    {
        g_pGame->StartGame ();
    }
    return true;
}
*/

#include <crtdbg.h>
//#define _CRTDBG_CHECK_EVERY_16_DF   0x00100000  /* check heap every 16 heap ops */
//#define _CRTDBG_CHECK_EVERY_128_DF  0x00800000  /* check heap every 128 heap ops */
//#define _CRTDBG_CHECK_EVERY_1024_DF 0x04000000  /* check heap every 1024 heap ops */

void CClientGame::EnablePacketRecorder ( const char* szFilename )
{
    m_pManager->GetPacketRecorder ()->StartRecord ( szFilename, true );
}

void CClientGame::StartPlayback ( void )
{
//    strcpy ( m_szNick, "Playback" );

    m_bIsPlayingBack = true;
    m_bFirstPlaybackFrame = true;
    m_pManager->GetPacketRecorder ()->SetPacketHandler ( CClientGame::StaticProcessPacket );

    if ( !m_pManager->IsGameLoaded () )
    {
        g_pGame->StartGame ();
    }
}

bool CClientGame::StartGame ( const char* szNick, const char* szPassword )
{
    int dbg = _CrtSetDbgFlag ( _CRTDBG_REPORT_FLAG );
    //dbg |= _CRTDBG_ALLOC_MEM_DF;
    //dbg |= _CRTDBG_CHECK_ALWAYS_DF;
    //dbg |= _CRTDBG_DELAY_FREE_MEM_DF;
    //dbg |= _CRTDBG_LEAK_CHECK_DF;
    //_CrtSetDbgFlag(dbg);

    // Verify that the nickname is valid
    if ( !IsNickValid ( szNick ) )
    {
        g_pCore->ShowMessageBox ( "Error", "A valid nickname isn't set. Please go to Settings and set a new.", MB_BUTTON_OK | MB_ICON_ERROR );
        g_pCore->GetModManager ()->RequestUnload ();
        return false;
    }

    // Store our nickname
    strncpy ( m_szLocalNick, szNick, MAX_PLAYER_NICK_LENGTH );
    m_szLocalNick [MAX_PLAYER_NICK_LENGTH] = 0;

    // Make sure that the SA data files weren't tampered with before we connect
    // (will be verified again after having joined to prevent modifying the files
    // during loading)
    if ( !VerifySADataFiles () )
        return false;

    // Are we connected?
    if ( g_pNet->IsConnected () || m_bIsPlayingBack )
    {
        // Hide the console when connecting..
        if ( g_pCore->GetConsole ()->IsVisible () )
           g_pCore->GetConsole ()->SetVisible ( false );

        // Display the status box
        g_pCore->ShowMessageBox ( "CONNECTING", "Entering the game ...", MB_ICON_INFO );

        // Send the initial data to the server
        NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
        if ( pBitStream )
        {
            // Hash the password if neccessary
            MD5 Password;
            memset ( Password, 0, sizeof ( MD5 ) );
            if ( szPassword )
            {
                // Is it long enough?
                size_t sizePassword = strlen ( szPassword );
                if ( sizePassword > 0 )
                {
                    // Hash the password and put it in the struct
                    CMD5Hasher Hasher;
                    Hasher.Calculate ( szPassword, sizePassword, Password );
                }
            }

            // Append version information
            pBitStream->Write ( static_cast < unsigned short > ( MTA_DM_NETCODE_VERSION ) );
            pBitStream->Write ( static_cast < unsigned short > ( MTA_DM_VERSION ) );
            pBitStream->Write ( static_cast < unsigned short > ( MTA_DM_BITSTREAM_VERSION ) );

            SString strPlayerVersion ( "%d.%d.%d-%d.%05d.%d"
                                        ,MTASA_VERSION_MAJOR
                                        ,MTASA_VERSION_MINOR
                                        ,MTASA_VERSION_MAINTENANCE
                                        ,MTASA_VERSION_TYPE
                                        ,MTASA_VERSION_BUILD
                                        ,g_pNet->GetNetRev ()
                                        );
            pBitStream->WriteString ( strPlayerVersion );

            pBitStream->WriteBit ( g_pCore->IsOptionalUpdateInfoRequired ( g_pNet->GetConnectedServer() ) );

            pBitStream->Write ( static_cast < unsigned char > ( g_pGame->GetGameVersion () ) );
            
            // Append user details
            pBitStream->Write ( const_cast < char* > ( m_szLocalNick ), MAX_PLAYER_NICK_LENGTH );
            pBitStream->Write ( reinterpret_cast < char* > ( Password ), sizeof ( MD5 ) );

            // Append community information
            std::string strUser;
            g_pCore->GetCommunity ()->GetUsername ( strUser );
            pBitStream->Write ( strUser.c_str (), MAX_SERIAL_LENGTH );

            // Send the packet as joindata
            g_pNet->SendPacket ( PACKET_ID_PLAYER_JOINDATA, pBitStream );
            g_pNet->DeallocateNetBitStream ( pBitStream );

            return true;
        }
    }
    else
    {
        g_pCore->ShowMessageBox ( "Error", "Not connected; please use Quick Connect or the 'connect' command to connect to a server.", MB_BUTTON_OK | MB_ICON_ERROR );
        g_pCore->GetModManager ()->RequestUnload ();
    }

    return false;
}


void CClientGame::SetupLocalGame ( const char* szConfig )
{
    m_bWaitingForLocalConnect = true;
    if ( !m_pLocalServer )
        m_pLocalServer = new CLocalServer ( szConfig );
}


bool CClientGame::StartLocalGame ( const char* szConfig, const char* szPassword )
{
    // Verify that the nickname is valid
    std::string strNick;
    g_pCore->GetCVars()->Get ( "nick", strNick );

    if ( !IsNickValid ( strNick.c_str() ) )
    {
        g_pCore->ShowMessageBox ( "Error", "A valid nickname isn't set. Please go to Settings and set a new.", MB_BUTTON_OK | MB_ICON_ERROR );
        g_pCore->GetModManager ()->RequestUnload ();
        return false;
    }

    m_bWaitingForLocalConnect = false;

    // Gotta copy the config in case we got it from local server setup gui
    SString strTemp = szConfig;

    if ( m_pLocalServer )
    {
        delete m_pLocalServer;
        m_pLocalServer = NULL;
    }

    // Store our nickname
    strncpy ( m_szLocalNick, strNick.c_str(), MAX_PLAYER_NICK_LENGTH );
    m_szLocalNick [MAX_PLAYER_NICK_LENGTH] = 0;

    // Got a server?
    if ( m_bLocalPlay )
    {
        // Start the server locally
        if ( !m_Server.Start ( strTemp ) )
        {
            m_bWaitingForLocalConnect = true;
            m_bErrorStartingLocal = true;
            g_pCore->ShowMessageBox ( "Error", "The server is not installed", MB_ICON_ERROR | MB_BUTTON_OK );
            g_pCore->GetModManager ()->RequestUnload ();
            return false;
        }

        if ( szPassword )
            m_Server.SetPassword ( szPassword );

        // Display the status box<<<<<
        g_pCore->ShowMessageBox ( "Local Server", "Starting local server ...", MB_ICON_INFO );
    }
    else
    {
        g_pCore->GetModManager ()->RequestUnload ();
        return false;
    }

    // We're waiting for connection
    m_bWaitingForLocalConnect = true;
    return true;
}


void CClientGame::DoPulsePreFrame ( void )
{   
    #ifdef MTA_VOICE
    /*
     * Do not attempt to just 'EnterCriticalSection' here, because we _never_
     * want to block on the critical section object in the rendering loop.
     *
     * If the critical section is in use, tough.  We'll process voice some other
     * time...
     */
    if ( ( CClientGame::STATUS_JOINED == m_Status ) && ( 0 != TryEnterCriticalSection(&m_crVoice) ) )
    {
        if ( m_pVoiceBuffer->size() != 0 )
        {
            NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
            if ( pBitStream )
            {
                // TODO:  Write talking player(s).
                // TODO:  Maybe copy the buffer?  I don't like casting this from const...
                CClientPlayer* localPlayer = g_pClientGame->GetPlayerManager()->GetLocalPlayer();
                if ( localPlayer != NULL )
                {
                    pBitStream->Write ( (unsigned short)m_pVoiceBuffer->size() );
                    pBitStream->Write ( (char *)m_pVoiceBuffer->get(), m_pVoiceBuffer->size() );

                    //g_pCore->GetConsole()->Printf("Sending vox packet of size: %u...", m_pVoiceBuffer->size());

                    // Clear out the synchornized buffer.
                    m_pVoiceBuffer->clear ( );

                    // Send it and destroy the packet
                    g_pNet->SendPacket ( PACKET_ID_VOICE_DATA, pBitStream, PACKET_PRIORITY_LOW, PACKET_RELIABILITY_RELIABLE );
                    g_pNet->DeallocateNetBitStream ( pBitStream );
                }
            }
        }
        LeaveCriticalSection(&m_crVoice);
    }
    #endif
}

#ifdef MTA_VOICE
void CClientGame::SendVoiceData ( const unsigned char * pData, int len )
{
    EnterCriticalSection(&m_crVoice);
    m_pVoiceBuffer->add(pData, len);
    LeaveCriticalSection(&m_crVoice);
}
#endif


void CClientGame::DoPulsePostFrame ( void )
{
    #ifdef DEBUG_KEYSTATES
        // Get the controller state
        CControllerState cs;
        g_pGame->GetPad ()->GetCurrentControllerState ( &cs );

        SString strBuffer;
        strBuffer = SString::Printf ( "LeftShoulder1: %u\n"
                                   "LeftShoulder2: %u\n"
                                   "RightShoulder1: %u\n"
                                   "RightShoulder2: %u\n"
                                   "DPadUp: %u\n"
                                   "DPadDown: %u\n"
                                   "DPadLeft: %u\n"
                                   "DPadRight: %u\n"
                                   "Start: %u\n"
                                   "Select: %u\n"
                                   "ButtonSquare: %u\n"
                                   "ButtonTriangle: %u\n"
                                   "ButtonCross: %u\n"
                                   "ButtonCircle: %u\n"
                                   "ShockButtonL: %u\n"
                                   "ShockButtonR: %u\n"
                                   "PedWalk: %u\n",
                                   cs.LeftShoulder1,
                                   cs.LeftShoulder2,
                                   cs.RightShoulder1,
                                   cs.RightShoulder2,
                                   cs.DPadUp,
                                   cs.DPadDown,
                                   cs.DPadLeft,
                                   cs.DPadRight,
                                   cs.Start,
                                   cs.Select,
                                   cs.ButtonSquare,
                                   cs.ButtonTriangle,
                                   cs.ButtonCross,
                                   cs.ButtonCircle,
                                   cs.ShockButtonL,
                                   cs.ShockButtonR,
                                   cs.m_bPedWalk );

        g_pCore->GetGraphics ()->DrawTextTTF ( 300, 10, 1280, 800, 0xFFFFFFFF, strBuffer, 1.0f, 0 );

        strBuffer = SString::Printf ( "VehicleMouseLook: %u\n"
                                   "LeftStickX: %u\n"
                                   "LeftStickY: %u\n"
                                   "RightStickX: %u\n"
                                   "RightStickY: %u",
                                   cs.m_bVehicleMouseLook,
                                   cs.LeftStickX,
                                   cs.LeftStickY,
                                   cs.RightStickX,
                                   cs.RightStickY );

        g_pCore->GetGraphics ()->DrawTextTTF ( 300, 320, 1280, 800, 0xFFFFFFFF, strBuffer, 1.0f, 0 );
    #endif

    if ( m_pManager->IsGameLoaded () )
    {
        // Pulse the nametags before anything that changes player positions, we'll be 1 frame behind, but so is the camera
        // If nametags are enabled, pulse the nametag manager
        if ( m_bShowNametags )
        {
            m_pNametags->DoPulse ();
        }

        // If we're supposed to show netstat, draw it
        if ( m_bShowNetstat )
        {
            m_pNetworkStats->Draw ();
        }

        // Sync debug
        m_pSyncDebug->OnPulse ();

        // Also eventually draw FPS
        if ( m_bShowFPS )
        {
            DrawFPS ();
        }

        // If we're in debug mode and are supposed to show task data, do it
        #ifdef MTA_DEBUG
        if ( m_pShowPlayerTasks )
        {
            DrawTasks ( m_pShowPlayerTasks );
        }

        if ( m_pShowPlayer )
        {
            DrawPlayerDetails ( m_pShowPlayer );
        }

        std::vector < CClientPlayer* > ::const_iterator iter = m_pPlayerManager->IterBegin ();
        for ( ; iter != m_pPlayerManager->IterEnd (); ++iter )
        {
            CClientPlayer* pPlayer = *iter;
            if ( pPlayer->IsStreamedIn () && pPlayer->IsShowingWepdata () )
                DrawWeaponsyncData ( pPlayer );
        }
        #endif

        #if defined (MTA_DEBUG) || defined (MTA_BETA)
        if ( m_bShowSyncingInfo )
        {
            // Draw the header boxz
            CVector vecPosition = CVector ( 0.05f, 0.32f, 0 );
            m_pDisplayManager->DrawText2D ( "Syncing vehicles:", vecPosition, 1.0f, 0xFFFFFFFF );

            // Print each vehicle we're syncing
            CDeathmatchVehicle* pVehicle;
            list < CDeathmatchVehicle* > ::const_iterator iter = m_pUnoccupiedVehicleSync->IterBegin ();
            for ( ; iter != m_pUnoccupiedVehicleSync->IterEnd (); iter++ )
            {
                vecPosition.fY += 0.03f;
                pVehicle = *iter;

                SString strBuffer ( "ID: %u (%s)", pVehicle->GetID (), pVehicle->GetNamePointer () );

                m_pDisplayManager->DrawText2D ( strBuffer, vecPosition, 1.0f, 0xFFFFFFFF );
            }
        }
        #endif

        GetClientPerfStatManager ()->DoPulse ();
    }

    // If we are not minimized we do the pulsing here
    if ( !g_pCore->IsWindowMinimized () )
    {
        // ..if no one else is doing it
        if ( m_uiNotPulsedCounter > 1 )
            DoPulses ();
        else
            m_uiNotPulsedCounter++;
    }
}


void CClientGame::DoPulses ( void )
{
    m_BuiltCollisionMapThisFrame = false;

    if ( m_bIsPlayingBack && m_bFirstPlaybackFrame && m_pManager->IsGameLoaded () )
    {
        g_pCore->GetConsole()->Printf("First playback frame, starting");
        m_pManager->GetPacketRecorder ()->StartPlayback ( "log.rec", false );
        m_bFirstPlaybackFrame = false;
    }

    // Call debug code if debug mode
    m_Foo.DoPulse ();

    // Output stuff from our internal server eventually
    m_Server.DoPulse ();

    if ( m_pManager->IsGameLoaded () && m_Status == CClientGame::STATUS_JOINED && GetTickCount64_ () - m_llLastTransgressionTime > 60000 )
    {
        uint uiLevel = 0;
        SString strMessage;

        // Is the player a cheater?
        if ( !m_pManager->GetAntiCheat ().PerformChecks () )
        {
            uiLevel = 1;
        }
        else
        {
            strMessage = g_pNet->GetNextBuffer ();
            if ( strMessage.length () )
                uiLevel = atoi ( strMessage.SplitLeft ( ":", &strMessage ) );
        }

        // Send message to the server
        if ( uiLevel )
        {
            SString strMessageCombo  = SString( "AC #%d %s", uiLevel, strMessage.c_str () ).TrimEnd ( " " );
            m_llLastTransgressionTime = GetTickCount64_ ();
            AddReportLog ( 3100, strMessageCombo );

            // Inform the server
            NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
            pBitStream->Write ( uiLevel );
            pBitStream->WriteString ( strMessage );
            g_pNet->SendPacket ( PACKET_ID_PLAYER_TRANSGRESSION, pBitStream );
            g_pNet->DeallocateNetBitStream ( pBitStream );
        }
    }

    // Send diagnostic info
    if ( m_pManager->IsGameLoaded () && m_Status == CClientGame::STATUS_JOINED )
    {
        // Retrieve data
        SString strMessage = g_pNet->GetDiagnosticStatus ();

        // Send to the server if changed
        if ( strMessage != m_strLastDiagnosticStatus )
        {
            m_strLastDiagnosticStatus = strMessage;
            NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
            pBitStream->WriteString ( strMessage );
            g_pNet->SendPacket ( PACKET_ID_PLAYER_DIAGNOSTIC, pBitStream );
            g_pNet->DeallocateNetBitStream ( pBitStream );
        }
    }

    // Pulse the network interface
    g_pNet->DoPulse ();
    m_pManager->DoPulse ();
    m_pNetAPI->DoPulse ();
    m_pUnoccupiedVehicleSync->DoPulse ();
    m_pPedSync->DoPulse ();
    m_pObjectSync->DoPulse ();
    m_pLuaManager->DoPulse ();

    #ifdef MTA_DEBUG
    UpdateMimics ();
    #endif

    // Grab the current time
    unsigned long ulCurrentTime = CClientTime::GetTime ();

    // Waiting for a connect?
    if ( m_bWaitingForLocalConnect )
    {
        // Connected? 
        if ( g_pNet->IsConnected () )
        {
            // No longer waiting for connect
            m_bWaitingForLocalConnect = false;

            // Assume local server has the same bitstream version
            g_pNet->SetServerBitStreamVersion ( MTA_DM_BITSTREAM_VERSION );

            // Run the game normally.
            StartGame ( m_szLocalNick, m_Server.GetPassword().c_str() );
        }
        else
        {
            // Going to try connecting? Do this when the internal server has booted
            // and we haven't started the connecting.
            if ( m_Server.IsReady () &&
                m_iLocalConnectAttempts == 0 )
            {
                g_pCore->ShowMessageBox ( "Local Server", "Connecting to local server...", MB_ICON_INFO );

                // Connect
                if ( g_pNet->StartNetwork ( "localhost", 22010 ) )
                {
                    // We're waiting for connection
                    m_iLocalConnectAttempts = 1;
                    m_ulTimeStart = CClientTime::GetTime ();
                }
                else
                {
                    g_pCore->ShowMessageBox ( "Error", "Error connecting to server.", MB_BUTTON_OK | MB_ICON_ERROR );
                    g_pCore->GetModManager ()->RequestUnload ();
                    return;
                }
            }

            // Timeout?
            if ( m_ulTimeStart != 0 && CClientTime::GetTime () >= m_ulTimeStart + 5000 )
            {
                // Show timeout message and disconnect
                g_pCore->ShowMessageBox ( "Error", "Connecting to local server timed out. See console for details.", MB_BUTTON_OK | MB_ICON_ERROR );
                g_pCore->GetModManager ()->RequestUnload ();
                return;
            }
        }
    }

    // If the game is loaded ...
    if ( m_pManager->IsGameLoaded () )
    {
        // Pulse the blended weather manager
        m_pBlendedWeather->DoPulse ();

        // If we weren't ingame last frame; call the on ingame event
        if ( !m_bGameLoaded )
        {
            // Fix for gta not being focused sometimes
            SetActiveWindow ( g_pCore->GetHookedWindow () );
            SetFocus ( g_pCore->GetHookedWindow () );

            m_bGameLoaded = true;
            Event_OnIngame ();
        }

        // Check if the player is hitting the enter vehicle button
        DoVehicleInKeyCheck ();

        // Pulse some stuff
        m_pMovingObjectsManager->DoPulse ();        

        // Get rid of our deleted elements
        m_ElementDeleter.DoDeleteAll ();
        m_pLuaManager->ProcessPendingDeleteList ();

        // Get rid of deleted GUI elements
        g_pCore->GetGUI ()->CleanDeadPool ();

        // Call onClientRender LUA event
        CLuaArguments Arguments;
        m_pRootEntity->CallEvent ( "onClientRender", Arguments, false );
    }

    // Are we connecting?
    if ( m_Status == CClientGame::STATUS_CONNECTING )
    {
        if ( m_bErrorStartingLocal )
        {
            g_pCore->GetModManager ()->RequestUnload ();
            return;
        }

        // Timed out?
        if ( !m_bWaitingForLocalConnect && ulCurrentTime >= m_ulTimeStart + NET_CONNECT_TIMEOUT )
        {
            g_pCore->ShowMessageBox ( "Error", "Connection timed out", MB_BUTTON_OK | MB_ICON_ERROR );
            g_pCore->GetModManager ()->RequestUnload ();
            return;
        }
    }
    else if ( m_Status == CClientGame::STATUS_JOINED )
    {
        // Pulse DownloadFiles if we're transferring stuff
        if ( m_bTransferResource ) DownloadFiles ();
    }
    else if ( m_Status == CClientGame::STATUS_TRANSFER )
    {
        DownloadFiles ();
    }
    else if ( m_Status == CClientGame::STATUS_OFFLINE )
    {

    }

    // Not waiting for local connect?
    if ( !m_bWaitingForLocalConnect )
    {
        // Trigger the ingame and connected event
        if ( !m_bTriggeredIngameAndConnected && m_pManager->IsGameLoaded () && g_pCore->IsConnected() )
        {
            m_bTriggeredIngameAndConnected = true;
            Event_OnIngameAndConnected ();

            // Initialize the game
            g_pCore->GetGame()->Initialize();
        }

        unsigned char ucError = g_pNet->GetConnectionError ();

        // Lost connection?
        if ( !g_pNet->IsConnected () && !m_bGracefulDisconnect && !m_bIsPlayingBack )
        {
            // See if we can figure out what specifically it was
            if ( ucError == 0 )
            {
                g_pCore->ShowMessageBox ( "Error", "Connection with the server was lost", MB_BUTTON_OK | MB_ICON_ERROR );
                g_pNet->SetImmediateError ( 0 );
                g_pCore->GetModManager ()->RequestUnload ();
                return;
            }
            else
            {
                SString strError;
                switch ( ucError )
                {
                    case ID_RSA_PUBLIC_KEY_MISMATCH:
                        strError = "Disconnected: unknown protocol error.";  // encryption key mismatch
                        break;
                    case ID_REMOTE_DISCONNECTION_NOTIFICATION:
                        strError = "Disconnected: disconnected remotely.";
                        break;
                    case ID_REMOTE_CONNECTION_LOST:
                        strError = "Disconnected: connection lost remotely.";
                        break;
                    case ID_CONNECTION_BANNED:
                        strError = "Disconnected: you are banned from this server.";
                        break;
                    case ID_NO_FREE_INCOMING_CONNECTIONS:
                        strError = "Disconnected: the server is currently full.";
                        break;
                    case ID_DISCONNECTION_NOTIFICATION:
                        strError = "Disconnected: disconnected from the server.";
                        break;
                    case ID_CONNECTION_LOST:
                        strError = "Disconnected: connection to the server was lost.";
                        break;
                    case ID_INVALID_PASSWORD:
                        strError = "Disconnected: invalid password specified.";
                        break;
                    default:
                        strError = "Disconnected: connection was refused.";
                        break;
                }

                // Display an error, reset the error status and exit
                g_pCore->ShowMessageBox ( "Error", strError, MB_BUTTON_OK | MB_ICON_ERROR );
                g_pNet->SetConnectionError ( 0 );
                g_pNet->SetImmediateError ( 0 );
                g_pCore->GetModManager ()->RequestUnload ();
            }
        }

        // If we're in the verificating status
        if ( m_Status == CClientGame::STATUS_JOINING )
        {
            // Time out the verification if it takes too long
            if ( m_ulVerifyTimeStart != 0 && ulCurrentTime >= m_ulVerifyTimeStart + CLIENT_VERIFICATION_TIMEOUT )
            {
                g_pCore->ShowMessageBox ( "Error", "MTA Client verification failed!", MB_BUTTON_OK | MB_ICON_ERROR );
                g_pCore->GetModManager ()->RequestUnload ();
            }
        }
    }

    // Fire the engine ready event 10 frames after we're ingame
    static int iFrameCount = 0;

    if ( iFrameCount <= 10 && m_Status == CClientGame::STATUS_JOINED || (m_pManager->IsGameLoaded () && m_Status == CClientGame::STATUS_OFFLINE) )
    {
        ++iFrameCount;

        // Give the game 5 frames to get ready
        if ( iFrameCount == 10 )
        {
            Event_OnIngameAndReady ();
        }
    }

    // Check for radar input
    m_pRadarMap->DoPulse ();

    // Got a local player?
    if ( m_pLocalPlayer )
    {
        // Network updates
        UpdateVehicleInOut ();
        UpdatePlayerTarget ();
        UpdatePlayerWeapons ();        
        UpdateTrailers ();
        UpdateStunts ();
        // Clear last damager if more than 2 seconds old
        if ( CClientTime::GetTime () - m_ulDamageTime > 2000 )
        {
            m_DamagerID = INVALID_ELEMENT_ID;
            m_ucDamageWeapon = 0xFF;
            m_ucDamageBodyPiece = 0xFF;
        }
        DoWastedCheck ( m_DamagerID, m_ucDamageWeapon, m_ucDamageBodyPiece );
    }

    // Game hacks, restore certain variables
    // game-speed changes after spawning
    g_pGame->SetGameSpeed ( m_fGameSpeed );
    // money changes on death/getting into taxis
    g_pGame->GetPlayerInfo ()->SetPlayerMoney ( m_lMoney );
    // stop players dying from starvation
    g_pGame->GetPlayerInfo()->SetLastTimeEaten ( 0 );
    // reset weapon logs (for preventing quickreload)

    // Update streaming
    m_pManager->UpdateStreamers ();
}


void CClientGame::HandleException ( CExceptionInformation* pExceptionInformation )
{

}


void CClientGame::HandleRadioNext ( CControlFunctionBind*  )
{
    if ( g_pClientGame )
    {
        CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->GetLocalPlayer ();
        if ( pPlayer )
        {
            pPlayer->NextRadioChannel ();
        }
    }
}


void CClientGame::HandleRadioPrevious ( CControlFunctionBind*  )
{
    if ( g_pClientGame )
    {
        CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->GetLocalPlayer ();
        if ( pPlayer )
        {
            pPlayer->PreviousRadioChannel ();
        }
    }
}
bool CClientGame::IsNametagValid ( const char* szNick )
{
    // Grab the size of the nametag. Check that it's not to long or short
    size_t sizeNick = strlen ( szNick );
    if ( sizeNick < MIN_PLAYER_NAMETAG_LENGTH || sizeNick > MAX_PLAYER_NAMETAG_LENGTH )
    {
        return false;
    }

    // Check that each character is valid (Anything above 32)
    unsigned char ucTemp;
    for ( size_t i = 0; i < sizeNick; i++ )
    {
        ucTemp = szNick [i];
        if ( ucTemp < 32 )
        {
            return false;
        }
    }

    // nametag is valid, return true
    return true;
}

bool CClientGame::IsNickValid ( const char* szNick )
{
    // Grab the size of the nick. Check that it's within the player
    size_t sizeNick = strlen ( szNick );
    if ( sizeNick < MIN_PLAYER_NICK_LENGTH || sizeNick > MAX_PLAYER_NICK_LENGTH )
    {
        return false;
    }

    // Check that each character is valid (visible characters exluding space)
    unsigned char ucTemp;
    for ( size_t i = 0; i < sizeNick; i++ )
    {
        ucTemp = szNick [i];
        if ( ucTemp < 33 || ucTemp > 126 )
        {
            return false;
        }
    }

    // Nickname is valid, return true
    return true;
}

void CClientGame::ShowNetstat ( bool bShow )
{
    if ( bShow && !m_bShowNetstat )
    {
        m_pNetworkStats->Reset ();
    }
    m_bShowNetstat = bShow;
}

void CClientGame::ShowEaeg ( bool )
{
    if ( m_pLocalPlayer )
        m_pLocalPlayer->SetStat ( 0x2329, 1.0f );
}

#ifdef MTA_WEPSYNCDBG
void CClientGame::ShowWepdata ( const char* szNick )
{
    CClientPlayer* pPlayer = m_pPlayerManager->Get ( szNick );
    if ( pPlayer )
    {
        pPlayer->SetShowingWepdata ( ! pPlayer->IsShowingWepdata() );
    }
}
#endif

#ifdef MTA_DEBUG

void CClientGame::ShowWepdata ( const char* szNick )
{
    CClientPlayer* pPlayer = m_pPlayerManager->Get ( szNick );
    if ( pPlayer )
    {
        pPlayer->SetShowingWepdata ( ! pPlayer->IsShowingWepdata() );
    }
}

void CClientGame::ShowPlayer ( const char* szNick )
{
    m_pShowPlayer = m_pPlayerManager->Get ( szNick );
}


void CClientGame::ShowTasks ( const char* szNick )
{
    m_pShowPlayerTasks = m_pPlayerManager->Get ( szNick );
}


void CClientGame::SetMimic ( unsigned int uiMimicCount )
{
    // Check if we're within the max mimics boundary
    if ( uiMimicCount > MAX_MIMICS ) return;

    // Create neccessary players
    while ( m_Mimics.size () < uiMimicCount )
    {
        CClientPlayer* pPlayer = new CClientPlayer ( m_pManager, static_cast < ElementID > ( MAX_NET_PLAYERS_REAL + (int)m_Mimics.size () ) );
        pPlayer->SetNick ( "Mimic" );
        m_Mimics.push_back ( pPlayer );
    }

    // Destroy neccessary players
    while ( m_Mimics.size () > uiMimicCount )
    {
        CClientPlayer* pPlayer = m_Mimics.back ();
        CClientVehicle* pVehicle = pPlayer->GetOccupiedVehicle ();
        if ( pVehicle )
            delete pVehicle;

        delete pPlayer;
        m_Mimics.pop_back ();
    }
}

#endif


void CClientGame::DoVehicleInKeyCheck ( void )
{
    // Grab the controller state
    CControllerState cs;
    g_pGame->GetPad ()->GetCurrentControllerState ( &cs );
    static bool bButtonTriangleWasDown = false;
    if ( cs.ButtonTriangle )
    {
        if ( !bButtonTriangleWasDown )
        {
            bButtonTriangleWasDown = true;

            // Process the hit
            ProcessVehicleInOutKey ( false );
        }
    }
    else
    {
        bButtonTriangleWasDown = false;
    }
}


void CClientGame::UpdateVehicleInOut ( void )
{
    // We got told by the server to animate into a certain vehicle?
    if ( m_VehicleInOutID != INVALID_ELEMENT_ID )
    {
        // Grab the vehicle we're getting in/out of
        CDeathmatchVehicle* pInOutVehicle = static_cast < CDeathmatchVehicle* > ( m_pVehicleManager->Get ( m_VehicleInOutID ) );

        // In or out?
        if ( m_bIsGettingOutOfVehicle )
        {
            // If we aren't working on leaving the car (he's eiter finished or cancelled/failed leaving)
            if ( !m_pLocalPlayer->IsLeavingVehicle () )
            {
                // Are we outside the car?
                CClientVehicle* pVehicle = m_pLocalPlayer->GetRealOccupiedVehicle ();
                if ( !pVehicle )
                {
                    // Tell the server that we successfully left the car
                    NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
                    if ( pBitStream )
                    {
                        // Write the car id and the action id (enter complete)
                        pBitStream->WriteCompressed ( m_VehicleInOutID );
                        unsigned char ucAction = VEHICLE_NOTIFY_OUT;
                        pBitStream->WriteBits ( &ucAction, 4 );

                        // Send it and destroy the packet
                        g_pNet->SendPacket ( PACKET_ID_VEHICLE_INOUT, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
                        g_pNet->DeallocateNetBitStream ( pBitStream );
                    }

                    // Warp ourself out (so we're sure the records are correct)
                    m_pLocalPlayer->RemoveFromVehicle ();

                    /*
                    // Make it undamagable if we're not syncing it, and damagable if we're syncing it
                    if ( pInOutVehicle )
                    {
                        if ( pInOutVehicle->IsSyncing () )
                        {
                            pInOutVehicle->SetCanBeDamaged ( true );
                            pInOutVehicle->SetTyresCanBurst ( true );
                        }
                        else
                        {
                            pInOutVehicle->SetCanBeDamaged ( false );
                            pInOutVehicle->SetTyresCanBurst ( false );
                        }
                    }*/ 
                    if ( pInOutVehicle )
                    {
                        pInOutVehicle->CalcAndUpdateCanBeDamagedFlag ();
                        pInOutVehicle->CalcAndUpdateTyresCanBurstFlag ();
                    }

                    // Reset the vehicle in out stuff so we're ready for another car entry/leave.
                    // Don't allow a new entry/leave until we've gotten the notify return packet
                    g_pClientGame->ResetVehicleInOut ();
                    m_bNoNewVehicleTask = true;

#ifdef MTA_DEBUG
                            g_pCore->GetConsole ()->Printf ( "* Sent_InOut: vehicle_notify_out" );
#endif
                }
                // Are we still inside the car?
                else
                {
                    // Warp us out now to keep in sync with the server
                    m_pLocalPlayer->RemoveFromVehicle ();
                }                
            }
        }

        // Are we getting into a vehicle?
        else if ( m_bIsGettingIntoVehicle )
        {
            // If we aren't working on entering the car (he's either finished or cancelled)
            if ( !m_pLocalPlayer->IsEnteringVehicle () )
            {
                // Is he in a vehicle now?
                CClientVehicle* pVehicle = m_pLocalPlayer->GetRealOccupiedVehicle ();
                if ( pVehicle )
                {
                    // Tell the server that we successfully entered the car
                    NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
                    if ( pBitStream )
                    {
                        // Write the car id and the action id (enter complete)
                        pBitStream->WriteCompressed ( m_VehicleInOutID );
                        unsigned char ucAction;

                        if ( m_bIsJackingVehicle )
                        {
                            ucAction = static_cast < unsigned char > ( VEHICLE_NOTIFY_JACK );
#ifdef MTA_DEBUG
                            g_pCore->GetConsole ()->Printf ( "* Sent_InOut: vehicle_notify_jack" );
#endif
                        }
                        else
                        {
                            ucAction = static_cast < unsigned char > ( VEHICLE_NOTIFY_IN );
#ifdef MTA_DEBUG
                            g_pCore->GetConsole ()->Printf ( "* Sent_InOut: vehicle_notify_in" );
#endif
                        }
                        pBitStream->WriteBits ( &ucAction, 4 );

                        // Send it and destroy the packet
                        g_pNet->SendPacket ( PACKET_ID_VEHICLE_INOUT, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
                        g_pNet->DeallocateNetBitStream ( pBitStream );
                    }

                    // Warp ourself in (so we're sure the records are correct)
                    m_pLocalPlayer->WarpIntoVehicle ( pVehicle, m_ucVehicleInOutSeat );

                    /*
                    // Make it damagable
                    if ( pInOutVehicle )
                    {
                        pInOutVehicle->SetCanBeDamaged ( true );
                        pInOutVehicle->SetTyresCanBurst ( true );
                    }
                    */
                    if ( pInOutVehicle )
                    {
                        pInOutVehicle->CalcAndUpdateCanBeDamagedFlag ();
                        pInOutVehicle->CalcAndUpdateTyresCanBurstFlag ();
                    }

                }
                else
                {
                    // Tell the server that we aborted entered the car
                    NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
                    if ( pBitStream )
                    {
                        // Write the car id and the action id (enter complete)
                        pBitStream->WriteCompressed ( m_VehicleInOutID );
                        unsigned char ucAction;
                        if ( m_bIsJackingVehicle )
                        {
                            ucAction = static_cast < unsigned char > ( VEHICLE_NOTIFY_JACK_ABORT );
                            pBitStream->WriteBits ( &ucAction, 4 );

                            // Did we start jacking them?
                            bool bAlreadyStartedJacking = false;
                            CClientVehicle* pVehicle = static_cast < CClientVehicle* > ( CElementIDs::GetElement ( m_VehicleInOutID ) );
                            if ( pVehicle )
                            {
                                CClientPed* pJackedPlayer = pVehicle->GetOccupant ();
                                if ( pJackedPlayer )
                                {
                                    // Jax: have we already started to jack the other player?
                                    if ( pJackedPlayer->IsGettingJacked () )
                                    {
                                        bAlreadyStartedJacking = true;
                                    }
                                }
                                pBitStream->WriteBits ( &(m_pLocalPlayer->m_ucEnteringDoor ), 3 );
                                SDoorOpenRatioSync door;
                                door.data.fRatio = pVehicle->GetDoorOpenRatio ( m_pLocalPlayer->m_ucEnteringDoor + 2 );
                                pBitStream->Write ( &door );
                            }
                            pBitStream->WriteBit ( bAlreadyStartedJacking );

#ifdef MTA_DEBUG
                            g_pCore->GetConsole ()->Printf ( "* Sent_InOut: vehicle_notify_jack_abort" );
#endif
                        }
                        else
                        {
                            ucAction = static_cast < unsigned char > ( VEHICLE_NOTIFY_IN_ABORT );
                            pBitStream->WriteBits ( &ucAction, 4 );
                            CClientVehicle* pVehicle = static_cast < CClientVehicle* > ( CElementIDs::GetElement ( m_VehicleInOutID ) );
                            if ( pVehicle )
                            {
                                pBitStream->WriteBits ( &(m_pLocalPlayer->m_ucEnteringDoor ), 3 );
                                SDoorOpenRatioSync door;
                                door.data.fRatio = pVehicle->GetDoorOpenRatio ( m_pLocalPlayer->m_ucEnteringDoor + 2 );
                                pBitStream->Write ( &door );
                            }

#ifdef MTA_DEBUG
                            g_pCore->GetConsole ()->Printf ( "* Sent_InOut: vehicle_notify_in_abort" );
#endif
                        }

                        // Send it and destroy the packet
                        g_pNet->SendPacket ( PACKET_ID_VEHICLE_INOUT, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
                        g_pNet->DeallocateNetBitStream ( pBitStream );
                    }

                    // Warp ourself out again (so we're sure the records are correct)
                    m_pLocalPlayer->RemoveFromVehicle ();

                    /*
                    // Make it undamagable if we're not syncing it, and damagable if we're syncing it
                    if ( pInOutVehicle )
                    {
                        if ( pInOutVehicle->IsSyncing () )
                        {
                            pInOutVehicle->SetCanBeDamaged ( true );
                            pInOutVehicle->SetTyresCanBurst ( true );
                        }
                        else
                        {
                            pInOutVehicle->SetCanBeDamaged ( false );
                            pInOutVehicle->SetTyresCanBurst ( false );
                        }
                    }
                    */
                    if ( pInOutVehicle )
                    {
                        pInOutVehicle->CalcAndUpdateCanBeDamagedFlag ();
                        pInOutVehicle->CalcAndUpdateTyresCanBurstFlag ();
                    }

                }

                // Reset
                // Don't allow a new entry/leave until we've gotten the notify return packet
                ResetVehicleInOut ();
                m_bNoNewVehicleTask = true;
            }
        }
    }
    else
    {
        // If we aren't getting jacked
        if ( !m_bIsGettingJacked )
        {
            CClientVehicle* pVehicle = m_pLocalPlayer->GetRealOccupiedVehicle ();
            CClientVehicle* pOccupiedVehicle = m_pLocalPlayer->GetOccupiedVehicle ();

            // Jax: this was commented, re-comment if it was there for a reason (..and give the reason!)
            // Are we in a vehicle we aren't supposed to be in?
            if ( pVehicle && !pOccupiedVehicle )
            {
                g_pCore->GetConsole ()->Print ( "You shouldn't be in this vehicle" );
                m_pLocalPlayer->RemoveFromVehicle ();
            }            

            // Are we supposed to be in a vehicle? But aren't?
            if ( pOccupiedVehicle && !pVehicle )
            {
                // Jax: this happens when we try to warp into a streamed out vehicle, including when we use CClientVehicle::StreamInNow
                // ..maybe we need a different way to detect bike falls?

                // Tell the server
                NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
                if ( pBitStream )
                {
                    // Vehicle id
                    pBitStream->WriteCompressed ( pOccupiedVehicle->GetID () );
                    unsigned char ucAction = static_cast < unsigned char > ( VEHICLE_NOTIFY_FELL_OFF );
                    pBitStream->WriteBits ( &ucAction, 4 );

                    // Send it and destroy the packet
                    g_pNet->SendPacket ( PACKET_ID_VEHICLE_INOUT, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
                    g_pNet->DeallocateNetBitStream ( pBitStream );

                    // We're not allowed to enter any vehicle before we get a confirm
                    m_bNoNewVehicleTask = true;

                    // Remove him from the vehicle
                    m_pLocalPlayer->RemoveFromVehicle ();

                    /*
                    // Make it undamagable if we're not syncing it
                    CDeathmatchVehicle* pInOutVehicle = static_cast < CDeathmatchVehicle* > ( pOccupiedVehicle );
                    if ( pInOutVehicle )
                    {
                        if ( pInOutVehicle->IsSyncing () )
                        {
                            pInOutVehicle->SetCanBeDamaged ( true );
                            pInOutVehicle->SetTyresCanBurst ( true );
                        }
                        else
                        {
                            pInOutVehicle->SetCanBeDamaged ( false );
                            pInOutVehicle->SetTyresCanBurst ( false );
                        }
                    }
                    */

#ifdef MTA_DEBUG
                            g_pCore->GetConsole ()->Printf ( "* Sent_InOut: vehicle_notify_fell_off" );
#endif
                }
            }
        }
    }
}


void CClientGame::UpdatePlayerTarget ( void )
{
    CControllerState ControllerState;
    m_pLocalPlayer->GetControllerState ( ControllerState );
    CVector vecOrigin, vecTarget;
    m_pLocalPlayer->GetShotData ( &vecOrigin, &vecTarget );

    // Ignore the local player so we don't get hit
    m_pLocalPlayer->WorldIgnore ( true );

    // Run a process line of sight and look for an entity we target
    CEntity* pColEntity = NULL;
    CColPoint* pColPoint = NULL;
    g_pGame->GetWorld ()->ProcessLineOfSight ( &vecOrigin, &vecTarget, &pColPoint, &pColEntity );

    // Unignore the local player again
    m_pLocalPlayer->WorldIgnore ( false );

    // Kill the colpoint or we get a severe memoryleak
    if ( pColPoint ) pColPoint->Destroy ();

    if ( pColEntity != m_pTargetedGameEntity )
    {
        m_pTargetedGameEntity = pColEntity;

        CClientEntity* pNewEntity = NULL;
        if ( pColEntity )
        {
            m_pTargetedEntity = m_pManager->FindEntity ( pColEntity );
        }
        else
            m_pTargetedEntity = NULL;

        // Store the last targeted player's id
        if ( m_pTargetedEntity && m_pTargetedEntity->GetType () == CCLIENTPLAYER )
        {
            m_TargetedPlayerID = m_pTargetedEntity->GetID ();
        }
        else
            m_TargetedPlayerID = INVALID_ELEMENT_ID;

        // Send the target
        ElementID TargetID = INVALID_ELEMENT_ID;
        if ( m_pTargetedEntity )
        {
            TargetID = m_pTargetedEntity->GetID ();
        }

        CBitStream bitStream;
        bitStream.pBitStream->WriteCompressed ( TargetID );
        m_pNetAPI->RPC ( PLAYER_TARGET, bitStream.pBitStream );

        // Call our onClientPlayerTarget event
        CLuaArguments Arguments;
        if ( m_pTargetedEntity )
            Arguments.PushElement ( m_pTargetedEntity );
        else
            Arguments.PushBoolean ( false );
        m_pLocalPlayer->CallEvent ( "onClientPlayerTarget", Arguments, true );
    }
}


void CClientGame::UpdatePlayerWeapons ( void )
{
    // Check whether we changed weapon slots
    eWeaponSlot currentSlot = m_pLocalPlayer->GetCurrentWeaponSlot ();
    if ( currentSlot != m_lastWeaponSlot )
    {
        CLuaArguments Arguments;
        Arguments.PushNumber ( m_lastWeaponSlot );
        Arguments.PushNumber ( currentSlot );
        bool bCancelled = !m_pLocalPlayer->CallEvent ( "onClientPlayerWeaponSwitch", Arguments, true );

        if ( bCancelled )
        {
            // Save the current ammo in clip
            unsigned short usAmmoInClip = 0;
            CWeapon* pWeapon = m_pLocalPlayer->GetWeapon ( m_lastWeaponSlot );
            if ( pWeapon )
                usAmmoInClip = static_cast < unsigned short > ( pWeapon->GetAmmoInClip () );

            // Force it back to the old slot
            m_pLocalPlayer->SetCurrentWeaponSlot ( m_lastWeaponSlot );

            // Restore the ammo in clip that there was in that slot
            if ( usAmmoInClip > 0 )
                m_pLocalPlayer->GetWeapon ()->SetAmmoInClip ( usAmmoInClip );
        }
        else
        {
            CBitStream bitStream;
            CWeapon* pWeapon = m_pLocalPlayer->GetWeapon ();
            NetBitStreamInterface& BitStream = *(bitStream.pBitStream);
            SWeaponSlotSync slot;

            if ( pWeapon )
            {
                /* Send a packet to the server with info about the new weapon,
                   so the server stays in sync reliably */
                unsigned int uiSlot = static_cast < unsigned int > ( pWeapon->GetSlot () );
                slot.data.uiSlot = uiSlot;
                BitStream.Write ( &slot );

                if ( CWeaponNames::DoesSlotHaveAmmo ( uiSlot ) )
                {
                    SWeaponAmmoSync ammo ( pWeapon->GetType (), true, true );
                    ammo.data.usAmmoInClip = static_cast < unsigned short > ( pWeapon->GetAmmoInClip () );
                    ammo.data.usTotalAmmo = static_cast < unsigned short > ( pWeapon->GetAmmoTotal () );
                    BitStream.Write ( &ammo );
                }
            }
            else
            {
                slot.data.uiSlot = 0;
                BitStream.Write ( &slot );
            }

            m_pNetAPI->RPC ( PLAYER_WEAPON, bitStream.pBitStream );
            m_lastWeaponSlot = m_pLocalPlayer->GetCurrentWeaponSlot ();
        }
    }
}


void CClientGame::UpdateTrailers ( void )
{
    // This function is here to re-attach trailers if they fall off
    
    unsigned long ulCurrentTime = GetTickCount32 ();

    CClientVehicle * pVehicle = NULL, * pTrailer = NULL;
    CVehicle * pGameVehicle = NULL, * pGameTrailer = NULL;
    unsigned long ulIllegalTowBreakTime;
    vector < CClientVehicle* > ::const_iterator iterVehicles = m_pVehicleManager->StreamedBegin ();
    for ( ; iterVehicles != m_pVehicleManager->StreamedEnd (); iterVehicles++ )
    {
        pVehicle = *iterVehicles;
        ulIllegalTowBreakTime = pVehicle->GetIllegalTowBreakTime ();

        // Do we have an illegal break?
        if ( ulIllegalTowBreakTime != 0 )
        {
            // Has it been atleast 1 second since the break
            if ( ulCurrentTime > ( ulIllegalTowBreakTime + 1000 ) )
            {
                // Try to re-attach them
                CClientVehicle * pTowedBy = pVehicle->GetTowedByVehicle ();
                if ( pTowedBy )
                {
                    // Little hack to keep illegaly detached trailers close to their tower
                    CVector vecPosition;
                    pVehicle->GetPosition ( vecPosition );
                    pVehicle->SetPosition ( vecPosition );

                    pGameVehicle = pTowedBy->GetGameVehicle ();
                    pGameTrailer = pVehicle->GetGameVehicle ();
                    if ( pGameVehicle && pGameTrailer )
                    {
                        pGameTrailer->SetTowLink ( pGameVehicle );                        
                    }
                }

                // Reset the break time, even if we couldnt re-attach it
                pVehicle->SetIllegalTowBreakTime ( 0 );
            }
        }
    }
}


void CClientGame::UpdateFireKey ( void )
{
    if ( m_pLocalPlayer )
    {
        SBindableGTAControl * pControl = g_pCore->GetKeyBinds ()->GetBindableFromControl ( "fire" );

        // Is our 'fire' control enabled?
        if ( pControl->bEnabled )
        {          
            // ** Satchel charge detonation **
            {
                // Do we have a detonator in our hand?
                if ( m_pLocalPlayer->GetCurrentWeaponSlot () == WEAPONSLOT_TYPE_DETONATOR )
                {            
                    // Planted any satchels?
                    if ( m_pLocalPlayer->CountProjectiles ( WEAPONTYPE_REMOTE_SATCHEL_CHARGE ) > 0 )
                    {
                        // Change the state back to false so this press doesn't do anything else
                        pControl->bState = false;

                        // Tell the server we wanna destroy our satchels
                        NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
                        if ( pBitStream )
                        {
                            g_pNet->SendPacket ( PACKET_ID_DETONATE_SATCHELS, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
                            g_pNet->DeallocateNetBitStream ( pBitStream );
                        }
                    }
                    // Remove the detonator if we dont have any satchels planted
                    else
                    {
                        m_pLocalPlayer->RemoveWeapon ( WEAPONTYPE_DETONATOR );
                    }
                }
            }

            // Has our control state been cut short?
            if ( !pControl->bState ) return;

            // ** Stealth kill **
            {
                if ( m_pLocalPlayer->IsStealthAiming () )
                {
                    // Do we have a target ped?
                    CClientPed * pTargetPed = m_pLocalPlayer->GetTargetedPed ();
                    if ( pTargetPed )
                    {
                        // Do we have a target player?
                        if ( IS_PLAYER ( pTargetPed ) )
                        {
                            CClientPlayer * pTargetPlayer = static_cast < CClientPlayer * > ( pTargetPed );
                            
                            // Is the targetted player on a team
                            CClientTeam* pTeam = pTargetPlayer->GetTeam ();
                            if ( pTeam )
                            {
                                // Is this friendly-fire?
                                if ( pTargetPlayer->IsOnMyTeam ( m_pLocalPlayer ) && !pTeam->GetFriendlyFire () )
                                {
                                    // Change the state back to false so this press doesn't do anything else
                                    pControl->bState = false;
                                    return;
                                }
                            }
                        }
                        CPlayerPed * pGameTarget = static_cast < CClientPed * > ( pTargetPed )->GetGamePlayer ();
                        if ( pGameTarget )
                        {
                            // Would GTA let us stealth kill now?
                            if ( m_pLocalPlayer->GetGamePlayer ()->GetPedIntelligence ()->TestForStealthKill ( pGameTarget, false ) )
                            {
                                //Grab our local position
                                CVector vecLocalPosition;
                                m_pLocalPlayer->GetPosition(vecLocalPosition);
                                
                                //Grab the target's position
                                CVector vecTargetPosition;
                                pTargetPed->GetPosition(vecTargetPosition);

                                //Work out an angle between the players, and set this as we initiate our knife kill
                                float fAngle = AngleBetweenPoints2D ( vecLocalPosition, vecTargetPosition );
                                m_pLocalPlayer->SetCurrentRotation(fAngle);

                                // Change the state back to false so this press doesn't do anything else
                                pControl->bState = false;
                                CLuaArguments Arguments;
                                Arguments.PushElement ( pTargetPed );
                                if ( m_pLocalPlayer->CallEvent ( "onClientPlayerStealthKill", Arguments, false ) ) 
                                {
                                    // Lets request a stealth kill
                                    CBitStream bitStream;
                                    bitStream.pBitStream->WriteCompressed ( pTargetPed->GetID () );
                                    m_pNetAPI->RPC ( REQUEST_STEALTH_KILL, bitStream.pBitStream );
                                }
                                else
                                {
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void CClientGame::UpdateStunts ( void )
{
    // * Two wheeler *
    static unsigned long ulLastCarTwoWheelCounter = 0;
    static float fLastCarTwoWheelDist = 0.0f;
    unsigned long ulTemp = g_pGame->GetPlayerInfo ()->GetCarTwoWheelCounter ();
    // Did we start a stunt?
    if ( ulLastCarTwoWheelCounter == 0 && ulTemp != 0 )
    {
        // Call our stunt event
        CLuaArguments Arguments;
        Arguments.PushString ( "2wheeler" );
        m_pLocalPlayer->CallEvent ( "onClientPlayerStuntStart", Arguments, true );
    }
    // Did we finish a stunt?
    else if ( ulLastCarTwoWheelCounter != 0 && ulTemp == 0 )
    {
        float fDistance = g_pGame->GetPlayerInfo ()->GetCarTwoWheelDist ();
        
        // Call our stunt event
        CLuaArguments Arguments;
        Arguments.PushString ( "2wheeler" );
        Arguments.PushNumber ( ulLastCarTwoWheelCounter );
        Arguments.PushNumber ( fDistance );
        m_pLocalPlayer->CallEvent ( "onClientPlayerStuntFinish", Arguments, true );
    }
    ulLastCarTwoWheelCounter = ulTemp;
    fLastCarTwoWheelDist = g_pGame->GetPlayerInfo ()->GetCarTwoWheelDist ();

    // * Wheelie *
    static unsigned long ulLastBikeRearWheelCounter = 0;
    static float fLastBikeRearWheelDist = 0.0f;
    ulTemp = g_pGame->GetPlayerInfo ()->GetBikeRearWheelCounter ();
    // Did we start a stunt?
    if ( ulLastBikeRearWheelCounter == 0 && ulTemp != 0 )
    {
        // Call our stunt event
        CLuaArguments Arguments;
        Arguments.PushString ( "wheelie" );
        m_pLocalPlayer->CallEvent ( "onClientPlayerStuntStart", Arguments, true );
    }
    // Did we finish a stunt?
    else if ( ulLastBikeRearWheelCounter != 0 && ulTemp == 0 )
    {
        float fDistance = g_pGame->GetPlayerInfo ()->GetBikeRearWheelDist ();
        
        // Call our stunt event
        CLuaArguments Arguments;
        Arguments.PushString ( "wheelie" );
        Arguments.PushNumber ( ulLastBikeRearWheelCounter );
        Arguments.PushNumber ( fDistance );
        m_pLocalPlayer->CallEvent ( "onClientPlayerStuntFinish", Arguments, true );
    }
    ulLastBikeRearWheelCounter = ulTemp;
    fLastBikeRearWheelDist = g_pGame->GetPlayerInfo ()->GetBikeRearWheelDist ();

    // * Stoppie *
    static unsigned long ulLastBikeFrontWheelCounter = 0;
    static float fLastBikeFrontWheelDist = 0.0f;
    ulTemp = g_pGame->GetPlayerInfo ()->GetBikeFrontWheelCounter ();
    // Did we start a stunt?
    if ( ulLastBikeFrontWheelCounter == 0 && ulTemp != 0 )
    {
        // Call our stunt event
        CLuaArguments Arguments;
        Arguments.PushString ( "stoppie" );
        m_pLocalPlayer->CallEvent ( "onClientPlayerStuntStart", Arguments, true );
    }
    // Did we finish a stunt?
    else if ( ulLastBikeFrontWheelCounter != 0 && ulTemp == 0 )
    {
        float fDistance = g_pGame->GetPlayerInfo ()->GetBikeFrontWheelDist ();
        
        // Call our stunt event
        CLuaArguments Arguments;
        Arguments.PushString ( "stoppie" );
        Arguments.PushNumber ( ulLastBikeFrontWheelCounter );
        Arguments.PushNumber ( fDistance );
        m_pLocalPlayer->CallEvent ( "onClientPlayerStuntFinish", Arguments, true );
    }
    ulLastBikeFrontWheelCounter = ulTemp;
    fLastBikeFrontWheelDist = g_pGame->GetPlayerInfo ()->GetBikeFrontWheelDist ();
}


void CClientGame::StaticUpdateFireKey ( CControlFunctionBind * pBind )
{
    g_pClientGame->UpdateFireKey ();
}


void CClientGame::ChangeVehicleWeapon ( bool bNext )
{
    if ( m_pLocalPlayer && m_pLocalPlayer->GetRealOccupiedVehicle () )
    {        
        eWeaponSlot currentWeaponSlot = m_pLocalPlayer->GetCurrentWeaponSlot ();
        eWeaponSlot weaponSlot = currentWeaponSlot;
        CWeapon * pWeapon = NULL;
        while ( !pWeapon || pWeapon->GetType () == WEAPONTYPE_UNARMED )
        {
            if ( bNext )
            {
                if ( weaponSlot == WEAPONSLOT_TYPE_DETONATOR )
                {
                    weaponSlot = WEAPONSLOT_TYPE_UNARMED;
                    break;
                }

                weaponSlot = ( eWeaponSlot ) ( weaponSlot + 1 );

                if ( weaponSlot == currentWeaponSlot )
                    break;

                pWeapon = m_pLocalPlayer->GetWeapon ( weaponSlot );
            }
            else
            {
                if ( weaponSlot == WEAPONSLOT_TYPE_UNARMED )
                {
                    if ( weaponSlot != currentWeaponSlot )
                        break;
                    weaponSlot = WEAPONSLOT_TYPE_DETONATOR;
                }

                weaponSlot = ( eWeaponSlot ) ( weaponSlot - 1 );

                if ( weaponSlot == currentWeaponSlot )
                    break;

                pWeapon = m_pLocalPlayer->GetWeapon ( weaponSlot );
            }
        }
        if ( pWeapon || weaponSlot == WEAPONSLOT_TYPE_UNARMED )
        {            
            m_pLocalPlayer->SetCurrentWeaponSlot ( weaponSlot );
        }
    }
}


void CClientGame::ResetVehicleInOut ( void )
{
    m_ulLastVehicleInOutTime = 0;
    m_bIsGettingOutOfVehicle = false;
    m_bIsGettingIntoVehicle = false;
    m_bIsJackingVehicle = false;
    m_bIsGettingJacked = false;
    m_VehicleInOutID = INVALID_ELEMENT_ID;
    m_ucVehicleInOutSeat = 0xFF;
    m_bNoNewVehicleTask = false;
    m_pGettingJackedBy = NULL;
}


void CClientGame::SetAllDimensions ( unsigned short usDimension )
{
    m_pManager->GetMarkerStreamer ()->SetDimension ( usDimension );
    m_pManager->GetObjectStreamer ()->SetDimension ( usDimension );
    m_pManager->GetPickupStreamer ()->SetDimension ( usDimension );
    m_pManager->GetPlayerStreamer ()->SetDimension ( usDimension );
    m_pManager->GetRadarAreaManager ()->SetDimension ( usDimension );
    m_pManager->GetVehicleStreamer ()->SetDimension ( usDimension );
    m_pManager->GetRadarMarkerManager ()->SetDimension ( usDimension );
    m_pManager->GetSoundManager ()->SetDimension ( usDimension );
    m_pNametags->SetDimension ( usDimension );
}


void CClientGame::StaticProcessClientKeyBind ( CKeyFunctionBind* pBind )
{
    g_pClientGame->ProcessClientKeyBind ( pBind );
}


void CClientGame::ProcessClientKeyBind ( CKeyFunctionBind* pBind )
{
    m_pScriptKeyBinds->ProcessKey ( pBind->boundKey->szKey, pBind->bHitState, SCRIPT_KEY_BIND_FUNCTION );
}


void CClientGame::StaticProcessClientControlBind ( CControlFunctionBind * pBind )
{
    g_pClientGame->ProcessClientControlBind ( pBind );
}


void CClientGame::ProcessClientControlBind ( CControlFunctionBind* pBind )
{
    m_pScriptKeyBinds->ProcessKey ( pBind->control->szControl, pBind->bHitState, SCRIPT_KEY_BIND_CONTROL_FUNCTION );
}


void CClientGame::StaticProcessServerKeyBind ( CKeyFunctionBind* pBind )
{
    g_pClientGame->ProcessServerKeyBind ( pBind );
}


void CClientGame::ProcessServerKeyBind ( CKeyFunctionBind* pBind )
{
    const char* szName = pBind->boundKey->szKey;
    unsigned char ucNameLength = ( unsigned char ) strlen ( szName );
    CBitStream bitStream;
    bitStream.pBitStream->WriteBit ( false );
    bitStream.pBitStream->WriteBit ( pBind->bHitState );
    bitStream.pBitStream->Write ( const_cast < char * > ( szName ), ucNameLength );
    m_pNetAPI->RPC ( KEY_BIND, bitStream.pBitStream );
}


void CClientGame::StaticProcessServerControlBind ( CControlFunctionBind* pBind )
{
    g_pClientGame->ProcessServerControlBind ( pBind );
}


void CClientGame::ProcessServerControlBind ( CControlFunctionBind* pBind )
{
    const char* szName = pBind->control->szControl;
    unsigned char ucNameLength = ( unsigned char ) strlen ( szName );
    CBitStream bitStream;
    bitStream.pBitStream->WriteBit ( true );
    bitStream.pBitStream->WriteBit ( pBind->bHitState );
    bitStream.pBitStream->Write ( const_cast < char * > ( szName ), ucNameLength );
    m_pNetAPI->RPC ( KEY_BIND, bitStream.pBitStream );
}


bool CClientGame::ProcessMessageForCursorEvents ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    bool bCursorForcedVisible = g_pCore->IsCursorForcedVisible ();
    bool bMenuVisible = g_pCore->IsMenuVisible ();
    bool bConsoleVisible = g_pCore->GetConsole ()->IsVisible ();
    bool bChatInputEnabled = g_pCore->IsChatInputEnabled ();

    if ( bCursorForcedVisible )
    {
        if ( !bMenuVisible && !bConsoleVisible )
        {
            if ( m_bCursorEventsEnabled )
            {
                unsigned char ucButtonHit = 0xFF;
                switch ( uMsg )
                {
                    case WM_LBUTTONDOWN:
                        ucButtonHit = 0;
                        break;
                    case WM_LBUTTONUP:
                        ucButtonHit = 1;
                        break;
                    case WM_MBUTTONDOWN:
                        ucButtonHit = 2;
                        break;
                    case WM_MBUTTONUP:
                        ucButtonHit = 3;
                        break;
                    case WM_RBUTTONDOWN:
                        ucButtonHit = 4;
                        break;
                    case WM_RBUTTONUP:
                        ucButtonHit = 5;
                        break;
                }
                if ( ucButtonHit != 0xFF )
                {
                    int iX = LOWORD ( lParam );
                    int iY = HIWORD ( lParam );

                    CVector2D vecResolution = g_pCore->GetGUI ()->GetResolution ();

                    /*
                    // (IJs) why are these relative? it doesn't make sense
                    CVector2D vecCursorPosition ( ( ( float ) iX ) / vecResolution.fX,
                                                  ( ( float ) iY ) / vecResolution.fY );
                    */

                    CVector2D vecCursorPosition ( ( float ) iX, ( float ) iY );

                    CVector vecOrigin, vecTarget, vecScreen ( ( float )iX, ( float )iY, 300.0f );
                    g_pCore->GetGraphics ()->CalcWorldCoors ( &vecScreen, &vecTarget );

                    // Grab the camera position
                    CCamera* pCamera = g_pGame->GetCamera ();
                    CCam* pCam = pCamera->GetCam ( pCamera->GetActiveCam () );
                    CMatrix matCamera;
                    pCamera->GetMatrix ( &matCamera );
                    vecOrigin = matCamera.vPos;

                    CColPoint* pColPoint = NULL;
                    CEntity* pGameEntity = NULL;

                    // Grab the collision point/entity
                    bool bCollision = g_pGame->GetWorld ()->ProcessLineOfSight ( &vecOrigin, &vecTarget, &pColPoint, &pGameEntity );

                    CVector vecCollision;
                    ElementID CollisionEntityID = INVALID_ELEMENT_ID;
                    CClientEntity* pCollisionEntity = NULL;
                    if ( bCollision && pColPoint )
                    {
                        vecCollision = *pColPoint->GetPosition ();
                        if ( pGameEntity )
                        {
                            CClientEntity* pEntity = m_pManager->FindEntity ( pGameEntity );
                            if ( pEntity )
                            {
                                pCollisionEntity = pEntity;
                                CollisionEntityID = pEntity->GetID ();
                            }
                        }
                    }
                    else
                    {
                        vecCollision = vecTarget;
                    }

                    // Destroy the colpoint so we don't get a leak
                    if ( pColPoint )
                    {
                        pColPoint->Destroy ();
                    }

                    char* szButton = NULL;
                    char* szState = NULL;
                    switch ( ucButtonHit )
                    {
                        case 0: szButton = "left"; szState = "down";
                            break;
                        case 1: szButton = "left"; szState = "up";
                            break;
                        case 2: szButton = "middle"; szState = "down";
                            break;
                        case 3: szButton = "middle"; szState = "up";
                            break;
                        case 4: szButton = "right"; szState = "down";
                            break;
                        case 5: szButton = "right"; szState = "up";
                            break;
                    }
                    if ( szButton && szState )
                    {
                        if ( _isnan( vecCollision.fX ) ) vecCollision.fX = 0;
                        if ( _isnan( vecCollision.fY ) ) vecCollision.fY = 0;
                        if ( _isnan( vecCollision.fZ ) ) vecCollision.fZ = 0;

                        // Call the event for the client
                        CLuaArguments Arguments;
                        Arguments.PushString ( szButton );
                        Arguments.PushString ( szState );
                        Arguments.PushNumber ( vecCursorPosition.fX );
                        Arguments.PushNumber ( vecCursorPosition.fY );
                        Arguments.PushNumber ( vecCollision.fX );
                        Arguments.PushNumber ( vecCollision.fY );
                        Arguments.PushNumber ( vecCollision.fZ );
                        if ( pCollisionEntity )
                            Arguments.PushElement ( pCollisionEntity );
                        else
                            Arguments.PushBoolean ( false );
                        m_pRootEntity->CallEvent ( "onClientClick", Arguments, false );

                        // Send the button, cursor position, 3d position and the entity collided with
                        CBitStream bitStream;

                        SMouseButtonSync button;
                        button.data.ucButton = ucButtonHit;
                        bitStream.pBitStream->Write ( &button );

                        bitStream.pBitStream->WriteCompressed ( static_cast < unsigned short > ( vecCursorPosition.fX  ) );
                        bitStream.pBitStream->WriteCompressed ( static_cast < unsigned short > ( vecCursorPosition.fY  ) );

                        SPositionSync position ( false );
                        position.data.vecPosition = vecCollision;
                        bitStream.pBitStream->Write ( &position );

                        if ( CollisionEntityID != INVALID_ELEMENT_ID )
                        {
                            bitStream.pBitStream->WriteBit ( true );
                            bitStream.pBitStream->WriteCompressed ( CollisionEntityID );
                        }
                        else
                            bitStream.pBitStream->WriteBit ( false );

                        m_pNetAPI->RPC ( CURSOR_EVENT, bitStream.pBitStream );

                        if ( strcmp(szState, "down") == 0 )
                        {
                            CVector2D vecDelta = m_vecLastCursorPosition - vecCursorPosition;

                            if (    ( GetTickCount32() - m_ulLastClickTick ) < DOUBLECLICK_TIMEOUT &&
                                    vecDelta.Length() <= DOUBLECLICK_MOVE_THRESHOLD )
                            {
                                // Call the event for the client
                                CLuaArguments DoubleClickArguments;
                                DoubleClickArguments.PushString ( szButton );
                                DoubleClickArguments.PushNumber ( vecCursorPosition.fX );
                                DoubleClickArguments.PushNumber ( vecCursorPosition.fY );
                                DoubleClickArguments.PushNumber ( vecCollision.fX );
                                DoubleClickArguments.PushNumber ( vecCollision.fY );
                                DoubleClickArguments.PushNumber ( vecCollision.fZ );
                                if ( pCollisionEntity )
                                    DoubleClickArguments.PushElement ( pCollisionEntity );
                                else
                                    DoubleClickArguments.PushBoolean ( false );
                                m_pRootEntity->CallEvent ( "onClientDoubleClick", DoubleClickArguments, false );
                            }

                            m_ulLastClickTick = GetTickCount32();
                            m_vecLastCursorPosition = vecCursorPosition;
                        }

                        return true;
                    }
                }
            }
        }
    }
    switch ( uMsg )
    {
        case WM_MOUSEMOVE:
        {
            int iX = LOWORD ( lParam ), iY = HIWORD ( lParam );
            static int iPreviousX = 0, iPreviousY = 0;
            if ( iX != iPreviousX || iY != iPreviousY )
            {
                iPreviousX = iX, iPreviousY = iY;

                CVector2D vecResolution = g_pCore->GetGUI ()->GetResolution ();
                CVector2D vecCursorPosition ( ( ( float ) iX ) / vecResolution.fX,
                                                ( ( float ) iY ) / vecResolution.fY );

                CVector vecTarget, vecScreen ( ( float )iX, ( float )iY, 300.0f );
                g_pCore->GetGraphics ()->CalcWorldCoors ( &vecScreen, &vecTarget );

                // Call the onClientCursorMove event
                CLuaArguments Arguments;
                Arguments.PushNumber ( ( double ) vecCursorPosition.fX );
                Arguments.PushNumber ( ( double ) vecCursorPosition.fY );
                Arguments.PushNumber ( ( double ) iX );
                Arguments.PushNumber ( ( double ) iY );
                Arguments.PushNumber ( ( double ) vecTarget.fX );
                Arguments.PushNumber ( ( double ) vecTarget.fY );
                Arguments.PushNumber ( ( double ) vecTarget.fZ );
                m_pRootEntity->CallEvent ( "onClientCursorMove", Arguments, false );
            }
            break;
        }
    }
    return false;
}


CClientPlayer * CClientGame::GetClosestRemotePlayer ( const CVector & vecPosition, float fMaxDistance )
{
    CClientPlayer * pClosest = NULL;
    float fDistance = 0.0f, fTemp;
    CVector vecTemp;
    CClientPlayer * pPlayer;
    vector < CClientPlayer * > ::const_iterator iter = m_pPlayerManager->IterBegin ();
    for ( ; iter != m_pPlayerManager->IterEnd () ; iter++ )
    {
        pPlayer = *iter;
        if ( !pPlayer->IsLocalPlayer () )
        {            
            pPlayer->GetPosition ( vecTemp );
            fTemp = DistanceBetweenPoints3D ( vecPosition, vecTemp );
            if ( fTemp < fMaxDistance )
            {
                if ( !pClosest || fTemp < fDistance )
                {
                    pClosest = pPlayer;
                    fDistance = fTemp;
                }
            }
        }
    }
    return pClosest;
}


void CClientGame::SetGameSpeed ( float fSpeed )
{
    g_pGame->SetGameSpeed ( fSpeed );
    m_fGameSpeed = fSpeed;
}

void CClientGame::SetMinuteDuration ( unsigned long ulDelay )
{
    g_pGame->SetMinuteDuration ( ulDelay );
    m_ulMinuteDuration = ulDelay;
}

void CClientGame::SetMoney ( long lMoney )
{
    g_pGame->GetPlayerInfo ()->SetPlayerMoney ( lMoney );
    m_lMoney = lMoney;
}


void CClientGame::AddBuiltInEvents ( void )
{

    // Resource events
    m_Events.AddEvent ( "onClientResourceStart", "resource", NULL, false );
    m_Events.AddEvent ( "onClientResourceStop", "resource", NULL, false );

    // Element events
    m_Events.AddEvent ( "onClientElementDataChange", "name", NULL, false );
    m_Events.AddEvent ( "onClientElementStreamIn", "", NULL, false );
    m_Events.AddEvent ( "onClientElementStreamOut", "", NULL, false );
    m_Events.AddEvent ( "onClientElementDestroy", "", NULL, false );

    // Player events
    m_Events.AddEvent ( "onClientPlayerJoin", "", NULL, false );
    m_Events.AddEvent ( "onClientPlayerQuit", "reason", NULL, false );
    m_Events.AddEvent ( "onClientPlayerTarget", "target", NULL, false );
    m_Events.AddEvent ( "onClientPlayerSpawn", "team", NULL, false );
    m_Events.AddEvent ( "onClientPlayerChangeNick", "oldNick", NULL, false );
    m_Events.AddEvent ( "onClientPlayerVehicleEnter", "vehicle, seat", NULL, false );
    m_Events.AddEvent ( "onClientPlayerVehicleExit", "vehicle, seat", NULL, false );
    m_Events.AddEvent ( "onClientPlayerTask", "priority, slot, name", NULL, false );
    m_Events.AddEvent ( "onClientPlayerWeaponSwitch", "previous, current", NULL, false );
    m_Events.AddEvent ( "onClientPlayerStuntStart", "type", NULL, false );
    m_Events.AddEvent ( "onClientPlayerStuntFinish", "type, time, distance", NULL, false );
    m_Events.AddEvent ( "onClientPlayerRadioSwitch", "", NULL, false );
    m_Events.AddEvent ( "onClientPlayerDamage", "attacker, weapon, bodypart", NULL, false );
    m_Events.AddEvent ( "onClientPlayerWeaponFire", "weapon, ammo, ammoInClip, hitX, hitY, hitZ, hitElement", NULL, false );
    m_Events.AddEvent ( "onClientPlayerWasted", "", NULL, false );
    m_Events.AddEvent ( "onClientPlayerChoke", "", NULL, false );
    m_Events.AddEvent ( "onClientPlayerStealthKill", "target", NULL, false );

    // Ped events
    m_Events.AddEvent ( "onClientPedDamage", "attacker, weapon, bodypart", NULL, false );
    m_Events.AddEvent ( "onClientPedWeaponFire", "weapon, ammo, ammoInClip, hitX, hitY, hitZ, hitElement", NULL, false );
    m_Events.AddEvent ( "onClientPedWasted", "", NULL, false );
    m_Events.AddEvent ( "onClientPedChoke", "", NULL, false );

    // Vehicle events
    m_Events.AddEvent ( "onClientVehicleRespawn", "", NULL, false );
    m_Events.AddEvent ( "onClientVehicleEnter", "player, seat", NULL, false );
    m_Events.AddEvent ( "onClientVehicleExit", "player, seat", NULL, false );
    m_Events.AddEvent ( "onClientVehicleStartEnter", "player, seat", NULL, false );
    m_Events.AddEvent ( "onClientVehicleStartExit", "player, seat", NULL, false );
    m_Events.AddEvent ( "onClientTrailerAttach", "towedBy", NULL, false );
    m_Events.AddEvent ( "onClientTrailerDetach", "towedBy", NULL, false );
    m_Events.AddEvent ( "onClientVehicleExplode", "", NULL, false );

    // GUI events
    m_Events.AddEvent ( "onClientGUIClick", "button, state, absoluteX, absoluteY", NULL, false );
    m_Events.AddEvent ( "onClientGUIDoubleClick", "button, state, absoluteX, absoluteY", NULL, false );
    m_Events.AddEvent ( "onClientGUIMouseDown", "button, absoluteX, absoluteY", NULL, false );
    m_Events.AddEvent ( "onClientGUIMouseUp", "button, absoluteX, absoluteY", NULL, false );
    m_Events.AddEvent ( "onClientGUIScroll", "element", NULL, false );
    m_Events.AddEvent ( "onClientGUIChanged", "element", NULL, false );
    m_Events.AddEvent ( "onClientGUIAccepted", "element", NULL, false );
    //m_Events.AddEvent ( "onClientGUIClose", "element", NULL, false );
    //m_Events.AddEvent ( "onClientGUIKeyDown", "element", NULL, false );
    m_Events.AddEvent ( "onClientGUITabSwitched", "element", NULL, false );
    m_Events.AddEvent ( "onClientGUIComboBoxAccepted", "element", NULL, false );

    m_Events.AddEvent ( "onClientDoubleClick", "button, screenX, screenY, worldX, worldY, worldZ, element", NULL, false );
    m_Events.AddEvent ( "onClientMouseMove", "screenX, screenY", NULL, false );
    m_Events.AddEvent ( "onClientMouseEnter", "screenX, screenY", NULL, false );
    m_Events.AddEvent ( "onClientMouseLeave", "screenX, screenY", NULL, false );
    m_Events.AddEvent ( "onClientMouseWheel", "", NULL, false );
    m_Events.AddEvent ( "onClientGUIMove", "", NULL, false );
    m_Events.AddEvent ( "onClientGUISize", "", NULL, false );
    m_Events.AddEvent ( "onClientGUIFocus", "", NULL, false );
    m_Events.AddEvent ( "onClientGUIBlur", "", NULL, false );

    // Console events
    m_Events.AddEvent ( "onClientConsole", "text", NULL, false );

    // Chat events
    m_Events.AddEvent ( "onClientChatMessage", "test, r, g, b", NULL, false );

    // Debug events
    m_Events.AddEvent ( "onClientDebugMessage", "message, level, file, line", NULL, false );

    // Game events
    m_Events.AddEvent ( "onClientPreRender", "", NULL, false );
    m_Events.AddEvent ( "onClientRender", "", NULL, false );

    // Cursor events
    m_Events.AddEvent ( "onClientClick", "button, state, screenX, screenY, worldX, worldY, worldZ, gui_clicked", NULL, false );
    m_Events.AddEvent ( "onClientCursorMove", "relativeX, relativeX, absoluteX, absoluteY, worldX, worldY, worldZ", NULL, false );

    // Marker events
    m_Events.AddEvent ( "onClientMarkerHit", "entity, matchingDimension", NULL, false );
    m_Events.AddEvent ( "onClientMarkerLeave", "entity, matchingDimension", NULL, false );

    // Marker events
    m_Events.AddEvent ( "onClientPickupHit", "entity, matchingDimension", NULL, false );
    m_Events.AddEvent ( "onClientPickupLeave", "entity, matchingDimension", NULL, false );

    // Col-shape events
    m_Events.AddEvent ( "onClientColShapeHit", "entity, matchingDimension", NULL, false );
    m_Events.AddEvent ( "onClientColShapeLeave", "entity, matchingDimension", NULL, false );
    m_Events.AddEvent ( "onClientElementColShapeHit", "colShape, matchingDimension", NULL, false );
    m_Events.AddEvent ( "onClientElementColShapeLeave", "colShape, matchingDimension", NULL, false );

    // Explosion events
    m_Events.AddEvent ( "onClientExplosion", "x, y, z, type", NULL, false );

    // Projectile events
    m_Events.AddEvent ( "onClientProjectileCreation", "creator", NULL, false );

    // Sound events
    m_Events.AddEvent ( "onClientSoundStream", "success, length, streamName", NULL, false );
    m_Events.AddEvent ( "onClientSoundFinishedDownload", "length", NULL, false );
    m_Events.AddEvent ( "onClientSoundChangedMeta", "streamTitle", NULL, false );
}


void CClientGame::DrawFPS ( void )
{
        // Draw the background
    float fResWidth = static_cast < float > ( g_pCore->GetGraphics ()->GetViewportWidth () );
    float fResHeight = static_cast < float > ( g_pCore->GetGraphics ()->GetViewportHeight () );
    g_pGame->GetHud ()->Draw2DPolygon ( 0.75f * fResWidth, 0.22f * fResHeight,
                                        1.0f * fResWidth, 0.22f * fResHeight,
                                        0.75f * fResWidth, 0.26f * fResHeight,
                                        1.0f * fResWidth, 0.26f * fResHeight,
                                        0x78000000 );


    static char x = 0;
    static float fDisp = 0.0f;
    if ( x == 20)
    {
        x = 0;
        fDisp = g_pGame->GetFPS();
    }
    else
        x++;
    SString strBuffer ( "FrameRate: %4.2f\n", fDisp );

    // Print it
    m_pDisplayManager->DrawText2D ( strBuffer, CVector ( 0.76f, 0.23f, 0 ), 1.0f, 0xFFFFFFFF );
}

#ifdef MTA_DEBUG

void CClientGame::DrawTasks ( CClientPlayer* pPlayer )
{
    CTask* pTask = NULL;
    CTask* pSubTask = NULL;

    // Got a local player model
    if ( pPlayer )
    {
        CTaskManager *man = pPlayer->GetTaskManager ();
        if(man == NULL) return;
        if((unsigned long)man == 0xDDDDDDDD) {
            m_pDisplayManager->DrawText2D ( "HELP! MANAGER FUCKED", CVector ( 0.05f, 0.5f,0 ), 1.0f );
            return;
        }

        // Grab the current task
        SString strOutput;
        SString strSubOutput;

        pTask = man->GetTask ( TASK_PRIORITY_PHYSICAL_RESPONSE );
        strOutput += SString ( "Physical Response: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTask ( TASK_PRIORITY_EVENT_RESPONSE_TEMP );
        strOutput += SString ( "Event Response Temp: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTask ( TASK_PRIORITY_EVENT_RESPONSE_NONTEMP );
        strOutput += SString ( "Event Response Non-temp: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTask ( TASK_PRIORITY_PRIMARY );
        strOutput += SString ( "Primary: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTask ( TASK_PRIORITY_DEFAULT );
        strOutput += SString ( "Default: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTaskSecondary ( TASK_SECONDARY_ATTACK );
        strOutput += SString ( "Secondary Attack: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTaskSecondary ( TASK_SECONDARY_DUCK );
        strOutput += SString ( "Secondary Duck: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTaskSecondary ( TASK_SECONDARY_SAY );
        strOutput += SString ( "Secondary Say: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTaskSecondary ( TASK_SECONDARY_FACIAL_COMPLEX );
        strOutput += SString ( "Secondary Facial Complex: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTaskSecondary ( TASK_SECONDARY_PARTIAL_ANIM );
        strOutput += SString ( "Secondary Partial Anim: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        pTask = man->GetTaskSecondary ( TASK_SECONDARY_IK );
        strOutput += SString ( "Secondary IK: %s\n", pTask ? ( pTask->GetTaskName () ) : ( "N/A" ) );
        strSubOutput += SString ( "%s\n", pTask && pTask->GetSubTask () ? ( pTask->GetSubTask ()->GetTaskName () ) : ( "N/A" ) );

        m_pDisplayManager->DrawText2D ( strOutput, CVector ( 0.05f, 0.5f,0 ), 1.0f );
        m_pDisplayManager->DrawText2D ( strSubOutput, CVector ( 0.5f, 0.5f,0 ), 1.0f );
    }
}

int iPlayerTask = 0;
void CClientGame::DrawPlayerDetails ( CClientPlayer* pPlayer )
{
    // Get the info
    CControllerState cs;

    pPlayer->GetControllerState ( cs );

    CVector vecPosition;
    pPlayer->GetPosition ( vecPosition );

    float fRotation = pPlayer->GetCurrentRotation ();
    float fCameraRotation = pPlayer->GetCameraRotation ();
    float fHealth = pPlayer->GetHealth ();
    bool bIsDucked = pPlayer->IsDucked ();
    bool bWearingGoggles = pPlayer->IsWearingGoggles ();
    bool bInVehicle = pPlayer->GetOccupiedVehicle () != NULL;
    float fWeaponRange = 0.0f;

    unsigned char ucWeapon = 0;
    unsigned char ucWeaponState = 0;
    unsigned short usWeaponAmmo = 0;
    CWeapon* pWeapon = pPlayer->GetWeapon ( pPlayer->GetCurrentWeaponSlot () );
    if ( pWeapon )
    {
        ucWeapon = static_cast < unsigned char > ( pWeapon->GetType () );
        ucWeaponState = static_cast < unsigned char > ( pWeapon->GetState () );
        usWeaponAmmo = static_cast < unsigned short > ( pWeapon->GetAmmoInClip () );
        fWeaponRange = pWeapon->GetInfo ()->GetWeaponRange ();
    }

    float fAimX, fAimY;
    pPlayer->GetAim ( fAimX, fAimY );
    const CVector& vecAimSource = pPlayer->GetAimSource ();
    const CVector& vecAimTarget = pPlayer->GetAimTarget ();
    unsigned char ucDrivebyAim = pPlayer->GetVehicleAimAnim ();

    g_pCore->GetGraphics ()->DrawLine3D ( vecAimSource, vecAimTarget, 0x90DE1212, 1.0f );

    CTask *pPrimaryTask = pPlayer->GetCurrentPrimaryTask ();
    int iPrimaryTask = pPrimaryTask ? pPrimaryTask->GetTaskType () : -1;

    // Copy the stuff
    SString strBuffer ( "Orient:\n"
                        "Position: %f %f %f\n"
                        "Rotation/camera: %f %f\n"
                        "Health: %f\n"
                        "\n"
                        "Keys:\n"
                        "LeftShoulder1: %hi\n"
                        "RightShoulder1: %hi\n"
                        "ButtonSquare: %hi\n"
                        "ButtonCross: %hi\n"
                        "ButtonCircle: %hi\n"
                        "ShockButtonL: %hi\n"
                        "PedWalk: %hi\n"
                        "VehicleMouseLook: %hi\n"
                        "LeftStickX: %hi\n"
                        "LeftStickY: %hi\n"
                        "\n"
                        "Misc:\n"
                        "Primary task: %d\n"
                        "Ducked: %u\n"
                        "Goggles: %u\n"
                        "In vehicle: %u\n"
                        "Weapon: %u\n"
                        "Weapon state: %u\n"
                        "Weapon ammo: %u\n"
                        "Weapon range: %f\n"
                        "Aim: %f %f\n"
                        "Aim source: %f %f %f\n"
                        "Aim target: %f %f %f\n"
                        "Driveby aim: %u\n"
                        "Frozen: %u\n",
                        vecPosition.fX, vecPosition.fY, vecPosition.fZ,
                        fRotation, fCameraRotation,
                        fHealth,
                        cs.LeftShoulder1,
                        cs.RightShoulder1,
                        cs.ButtonSquare,
                        cs.ButtonCross,
                        cs.ButtonCircle,
                        cs.ShockButtonL,
                        cs.m_bPedWalk,
                        cs.m_bVehicleMouseLook,
                        cs.LeftStickX,
                        cs.LeftStickY,
                        iPrimaryTask,
                        bIsDucked,
                        bWearingGoggles,
                        bInVehicle,
                        ucWeapon,
                        ucWeaponState,
                        usWeaponAmmo,
                        fWeaponRange,
                        fAimX, fAimY,
                        vecAimSource.fX, vecAimSource.fY, vecAimSource.fZ,
                        vecAimTarget.fX, vecAimTarget.fY, vecAimTarget.fZ,
                        ucDrivebyAim,
                        pPlayer->IsFrozen()
                    );

    // Draw it
    m_pDisplayManager->DrawText2D ( strBuffer, CVector ( 0.45f, 0.05f, 0 ), 1.0f, 0xFFFFFFFF );
}

void CClientGame::DrawWeaponsyncData ( CClientPlayer* pPlayer )
{
    CWeapon* pWeapon = pPlayer->GetWeapon ( pPlayer->GetCurrentWeaponSlot () );

    if ( pWeapon )
    {
        CVector vecSource;
        CVector vecTarget;

        // red line: Draw their synced aim line
        pPlayer->GetShotData ( &vecSource, &vecTarget );
        g_pCore->GetGraphics ()->DrawLine3D ( vecSource, vecTarget, 0x90DE1212, 2.0f );

        // green line: Set muzzle as origin and perform a collision test for the target
        CColPoint* pCollision;
        CVector vecTemp;
        bool bCollision = g_pGame->GetWorld ()->ProcessLineOfSight ( &vecSource, &vecTarget, &pCollision, NULL );
        if ( pCollision )
        {
            if ( bCollision )
            {
                CVector vecBullet = *pCollision->GetPosition() - vecSource;
                vecBullet.Normalize();
                CVector vecTarget = vecSource + (vecBullet * 200);
                g_pCore->GetGraphics ()->DrawLine3D ( vecSource, vecTarget, 0x9012DE12, 0.5f );
            }
            pCollision->Destroy();
        }

        if ( m_pLocalPlayer != pPlayer )
        {
            // Draw information about their weapon state, total ammo and ammo in clip
            CVector vecScreenPosition;
            CVector vecPosition;

            pPlayer->GetPosition ( vecPosition );

            vecPosition.fZ += 1.0f;
            g_pCore->GetGraphics ()->CalcScreenCoors ( &vecPosition, &vecScreenPosition );

            char str [ 2048 ];
            int yoffset;

            yoffset = 0;
            _snprintf ( str, sizeof( str ), "Ammo in clip: %d", pWeapon->GetAmmoInClip () );
            g_pCore->GetGraphics ()->DrawText ( ( int ) vecScreenPosition.fX + 1, ( int ) vecScreenPosition.fY + 1 + yoffset, ( int ) vecScreenPosition.fX + 1, ( int ) vecScreenPosition.fY + 1 + yoffset, COLOR_ARGB ( 255, 255, 255, 255 ), str, 1.0f, 1.0f, DT_NOCLIP | DT_CENTER );
            g_pCore->GetGraphics ()->DrawText ( ( int ) vecScreenPosition.fX, ( int ) vecScreenPosition.fY + yoffset, ( int ) vecScreenPosition.fX, ( int ) vecScreenPosition.fY + yoffset, COLOR_ARGB ( 255, 0, 0, 0 ), str, 1.0f, 1.0f, DT_NOCLIP | DT_CENTER );

            yoffset = 15;
            _snprintf ( str, sizeof( str ), "State: %d", pWeapon->GetState() );
            g_pCore->GetGraphics ()->DrawText ( ( int ) vecScreenPosition.fX + 1, ( int ) vecScreenPosition.fY + 1 + yoffset, ( int ) vecScreenPosition.fX + 1, ( int ) vecScreenPosition.fY + 1 + yoffset, COLOR_ARGB ( 255, 255, 255, 255 ), str, 1.0f, 1.0f, DT_NOCLIP | DT_CENTER );
            g_pCore->GetGraphics ()->DrawText ( ( int ) vecScreenPosition.fX, ( int ) vecScreenPosition.fY + yoffset, ( int ) vecScreenPosition.fX, ( int ) vecScreenPosition.fY + yoffset, COLOR_ARGB ( 255, 0, 0, 0 ), str, 1.0f, 1.0f, DT_NOCLIP | DT_CENTER );
        }
    }
}

void CClientGame::UpdateMimics ( void )
{
    // Got a local player?
    if ( m_pLocalPlayer )
    {
        unsigned char ucWeaponType = 0;
        unsigned char ucWeaponState = 0;
        unsigned long ulWeaponAmmoInClip = 0;
        eWeaponSlot weaponSlot = WEAPONSLOT_TYPE_UNARMED;

        CWeapon* pPlayerWeapon = m_pLocalPlayer->GetWeapon ( m_pLocalPlayer->GetCurrentWeaponSlot () );
        if ( pPlayerWeapon )
        {
            ucWeaponType = static_cast < unsigned char > ( pPlayerWeapon->GetType () );
            ucWeaponState = static_cast < unsigned char > ( pPlayerWeapon->GetState () );
            weaponSlot = pPlayerWeapon->GetSlot ();
            ulWeaponAmmoInClip = pPlayerWeapon->GetAmmoInClip ();
        }

        // Simulate lag (or not)
        if ( !m_bMimicLag || CClientTime::GetTime () >= m_ulLastMimicLag + 200 ) // TICK_RATE )
        {
            m_ulLastMimicLag = CClientTime::GetTime ();

            // Grab the local data
            CControllerState Controller;
            m_pLocalPlayer->GetControllerState ( Controller );
            CVector vecPosition;
            m_pLocalPlayer->GetPosition ( vecPosition );
            float fRotation = m_pLocalPlayer->GetCurrentRotation ();
            CVector vecMoveSpeed;
            m_pLocalPlayer->GetMoveSpeed ( vecMoveSpeed );
            float fHealth = m_pLocalPlayer->GetHealth ();
            float fArmor = m_pLocalPlayer->GetArmor ();
            float fCameraRotation = g_pGame->GetCamera ()->GetCameraRotation ();
            bool bDucked = m_pLocalPlayer->IsDucked ();
            bool bWearingGoggles = m_pLocalPlayer->IsWearingGoggles ( true );
            bool bHasJetpack = m_pLocalPlayer->HasJetPack ();
            bool bChoking = m_pLocalPlayer->IsChoking ();
            bool bSunbathing = m_pLocalPlayer->IsSunbathing ();
            bool bDoingDriveby = m_pLocalPlayer->IsDoingGangDriveby ();
            bool bStealthAiming = m_pLocalPlayer->IsStealthAiming ();

            // Is the current weapon goggles (44 or 45) or a camera (43), or a detonator (40), don't apply the fire key
            if ( weaponSlot == 11 || weaponSlot == 12 || ucWeaponType == 43 )
                Controller.ButtonCircle = 0;

            CClientVehicle* pVehicle = m_pLocalPlayer->GetOccupiedVehicle ();
            unsigned int uiSeat = m_pLocalPlayer->GetOccupiedVehicleSeat ();

            CShotSyncData* pShotSync = g_pMultiplayer->GetLocalShotSyncData ();
            CVector vecOrigin, vecTarget;
            m_pLocalPlayer->GetShotData ( &vecOrigin, &vecTarget );
            float fAimX = pShotSync->m_fArmDirectionX;
            float fAimY = pShotSync->m_fArmDirectionY;
            char cVehicleAimDirection = pShotSync->m_cInVehicleAimDirection;
            bool bAkimboUp = g_pMultiplayer->GetAkimboTargetUp ();

            /*
            static CClientMarker *pOriginCorona = NULL, *pTargetCorona = NULL;
            if ( pOriginCorona == NULL )
            {
                pOriginCorona = new CClientMarker ( m_pManager, 1337, MARKER_CORONA );
                pOriginCorona->SetColor ( 0, 255, 0, 255 );
                pOriginCorona->SetSize ( 0.5f );
            }
            else
                pOriginCorona->SetPosition ( vecOrigin );

            if ( pTargetCorona == NULL )
            {
                pTargetCorona = new CClientMarker ( m_pManager, 1338, MARKER_CORONA );
                pTargetCorona->SetColor ( 255, 0, 0, 255 );
                pTargetCorona->SetSize ( 0.5f );
            }
            else
                pTargetCorona->SetPosition ( vecTarget );*/

            // Apply this to each of our mimic players
            unsigned int uiMimicIndex = 0;
            list < CClientPlayer* > ::const_iterator iterMimics = m_Mimics.begin ();
            for ( ; iterMimics != m_Mimics.end (); iterMimics++, uiMimicIndex++ )
            {
                vecPosition.fX += 4.0f;
                vecOrigin.fX += 4.0f;
                vecTarget.fX += 4.0f;

                CClientPlayer* pMimic = *iterMimics;

                pMimic->SetHealth ( fHealth );
                pMimic->LockHealth ( fHealth );
                pMimic->SetArmor ( fArmor );
                pMimic->LockArmor ( fArmor );
                pMimic->SetWearingGoggles ( bWearingGoggles );
                pMimic->SetHasJetPack ( bHasJetpack );
                pMimic->SetChoking ( bChoking );
                pMimic->SetSunbathing ( bSunbathing );
                pMimic->SetDoingGangDriveby ( bDoingDriveby );
                pMimic->SetStealthAiming ( bStealthAiming );

                Controller.ShockButtonL = 0;

                if ( m_bMimicLag )
                {
                    pMimic->SetTargetPosition ( vecPosition, TICK_RATE );
                    pMimic->SetMoveSpeed ( vecMoveSpeed );
                    pMimic->SetControllerState ( Controller );
                    pMimic->SetTargetRotation ( fRotation );
                    pMimic->SetCameraRotation ( fCameraRotation );
                    pMimic->Duck ( bDucked );
                }
                else
                {
                    pMimic->SetPosition ( vecPosition );
                    pMimic->SetMoveSpeed ( vecMoveSpeed );
                    pMimic->SetTargetRotation ( fRotation );
                    pMimic->SetCameraRotation ( fCameraRotation );
                    pMimic->SetControllerState ( Controller );
                    pMimic->Duck ( bDucked );
                }

                if ( ucWeaponType != 0 )
                {
                    if ( ucWeaponType == 44 || ucWeaponType == 45 )
                    {
                        Controller.ButtonCircle = 0;
                    }

                    if ( m_bMimicLag )
                    {
                        pMimic->SetAimInterpolated ( TICK_RATE, fAimX, fAimY, bAkimboUp, cVehicleAimDirection );
                        pMimic->SetTargetTarget ( TICK_RATE, vecOrigin, vecTarget );                                

                        pMimic->AddChangeWeapon ( TICK_RATE, weaponSlot, (unsigned char) ulWeaponAmmoInClip );
                    }
                    else
                    {
                        CWeapon* pPlayerWeapon = pMimic->GetWeapon ();
                        eWeaponType eCurrentWeapon = static_cast < eWeaponType > ( ucWeaponType );
                        if ( ( pPlayerWeapon && pPlayerWeapon->GetType () != eCurrentWeapon ) || !pPlayerWeapon )
                        {
                            pPlayerWeapon = pMimic->GiveWeapon ( eCurrentWeapon, ulWeaponAmmoInClip );
                            if ( pPlayerWeapon )
                            {
                                pPlayerWeapon->SetAsCurrentWeapon ();
                            }
                        }

                        if ( pPlayerWeapon )
                        {
                            pPlayerWeapon->SetAmmoTotal ( 9999 );
                            pPlayerWeapon->SetAmmoInClip ( ulWeaponAmmoInClip );
                            pPlayerWeapon->SetState ( static_cast < eWeaponState > ( ucWeaponState ) );
                        }
                        pMimic->SetAimInterpolated ( TICK_RATE, fAimX, fAimY, bAkimboUp, cVehicleAimDirection );
                        pMimic->SetTargetTarget ( TICK_RATE, vecOrigin, vecTarget );
                    }
                }
                else
                {
                    pMimic->SetCurrentWeaponSlot ( WEAPONSLOT_TYPE_UNARMED );
                }

                CClientVehicle* pMimicVehicle = pMimic->GetOccupiedVehicle ();
                if ( pVehicle )
                {
                    unsigned int uiModel;
                    CVector vecPosition, vecRotationDegrees;
                    CVector vecMoveSpeed, vecMoveSpeedMeters, vecTurnSpeed, vecVelocity;
                    float fHealth;

                    uiModel = pVehicle->GetModel ();
                    pVehicle->GetPosition ( vecPosition );
                    pVehicle->GetRotationDegrees ( vecRotationDegrees );
                    pVehicle->GetMoveSpeed ( vecMoveSpeed );
                    pVehicle->GetTurnSpeed ( vecTurnSpeed );
                    fHealth = pVehicle->GetHealth ();

                    if ( pMimicVehicle && pMimicVehicle->GetModel () != uiModel )
                    {
                        delete pMimicVehicle;
                        pMimicVehicle = NULL;
                    }

                    vecPosition.fX += ( ( float ) ( uiMimicIndex + 1 ) * 10.0f );

                    if ( pMimicVehicle == NULL )
                    {
                        pMimicVehicle = new CDeathmatchVehicle ( m_pManager, m_pUnoccupiedVehicleSync, INVALID_ELEMENT_ID, uiModel );
                        pMimicVehicle->SetPosition ( vecPosition );

                        unsigned short * usUpgrades = pVehicle->GetUpgrades ()->GetSlotStates ();
                        for ( unsigned char uc = 0 ; uc < VEHICLE_UPGRADE_SLOTS ; uc++ )
                        {
                            if ( usUpgrades [ uc ] )
                            {
                                pMimicVehicle->GetUpgrades ()->AddUpgrade ( usUpgrades [ uc ] );
                            }
                        }

                        m_vecLastMimicPos = vecPosition;
                    }

                    if ( m_bMimicLag )
                    {
                        pMimicVehicle->SetTargetPosition ( vecPosition, TICK_RATE );
                        pMimicVehicle->SetTargetRotation ( vecRotationDegrees, TICK_RATE );
                        pMimicVehicle->SetMoveSpeed ( vecMoveSpeed );
                        pMimicVehicle->SetTurnSpeed ( vecTurnSpeed );
                    }
                    else
                    {
                        pMimicVehicle->SetPosition ( vecPosition );
                        pMimicVehicle->SetRotationDegrees ( vecRotationDegrees );
                        pMimicVehicle->SetMoveSpeed ( vecMoveSpeed );
                        pMimicVehicle->SetTurnSpeed ( vecTurnSpeed );
                    }
                    pMimicVehicle->SetHealth ( fHealth );
                    if ( pMimic->GetOccupiedVehicle () != pMimicVehicle )
                        pMimic->WarpIntoVehicle ( pMimicVehicle, uiSeat );

                    unsigned int uiTrailerLoop = 0;
                    CClientVehicle* pTrailer = pVehicle->GetTowedVehicle ();
                    CClientVehicle* pMimicTrailer = NULL;
                    while ( pTrailer )
                    {
                        uiModel = pTrailer->GetModel ();
                        pTrailer->GetPosition ( vecPosition );
                        pTrailer->GetRotationDegrees ( vecRotationDegrees );
                        pTrailer->GetMoveSpeed ( vecMoveSpeed );
                        pTrailer->GetTurnSpeed ( vecTurnSpeed );
                        fHealth = pTrailer->GetHealth ();

                        pMimicTrailer = static_cast < CClientVehicle* > ( CElementIDs::GetElement ( static_cast < ElementID > ( 450 + uiMimicIndex + uiTrailerLoop ) ) );

                        if ( pMimicTrailer && pMimicTrailer->GetModel () != uiModel )
                        {
                            delete pMimicTrailer;
                            pMimicTrailer = NULL;
                        }

                        if ( !pMimicTrailer )
                        {
                            pMimicTrailer = new CDeathmatchVehicle ( m_pManager, m_pUnoccupiedVehicleSync, static_cast < ElementID > ( 450 + uiMimicIndex + uiTrailerLoop ), uiModel );
                            pMimicVehicle->SetTowedVehicle ( pMimicTrailer );
                        }

                        pTrailer = pTrailer->GetTowedVehicle ();
                        pMimicTrailer = pMimicTrailer->GetTowedVehicle ();
                        uiTrailerLoop++;
                    }
                    if ( pMimicTrailer )
                    {
                        if ( pMimicTrailer->GetTowedByVehicle () )
                        {
                            pMimicTrailer->GetTowedByVehicle ()->SetTowedVehicle ( NULL );
                        }
                    }
                }
                else if ( pMimicVehicle )
                {
                    pMimic->RemoveFromVehicle ();
                    delete pMimicVehicle;
                }
            }
        }
    }
}


void CClientGame::DoPaintballs ( void )
{
    if ( m_pLocalPlayer )
    {
        CVector vecOrigin, vecTarget;
        m_pLocalPlayer->GetShotData ( &vecOrigin, &vecTarget );

        CColPoint* pCollision = NULL;
        CEntity* pCollisionEntity = NULL;
        m_pLocalPlayer->WorldIgnore ( true );
        bool bCollision = g_pGame->GetWorld ()->ProcessLineOfSight ( &vecOrigin, &vecTarget, &pCollision, &pCollisionEntity );
        m_pLocalPlayer->WorldIgnore ( false );

        static list < CClientMarker * > paintBalls;
        if ( paintBalls.size () >= 5 )
        {
            CClientMarker * pCorona = paintBalls.back ();
            delete pCorona;
            paintBalls.pop_back ();
        }

        CClientMarker* pCorona = new CClientMarker ( m_pManager, INVALID_ELEMENT_ID, CClientMarker::MARKER_CORONA );
        paintBalls.push_front ( pCorona );
        pCorona->SetSize ( 0.2f );
        if ( bCollision && pCollision )
        {
            pCorona->SetPosition ( *pCollision->GetPosition () );
            pCorona->SetColor ( SColorRGBA ( 255, 0, 0, 255 ) );
        }
        else
        {
            pCorona->SetPosition ( vecTarget );
            pCorona->SetColor ( SColorRGBA ( 255, 255, 0, 255 ) );
        }

        // Destroy the colpoint
        if ( pCollision )
        {
            pCollision->Destroy ();
        }
    }
}

#endif


void CClientGame::QuitPlayer ( CClientPlayer* pPlayer, eQuitReason Reason )
{
    // Get the nick pointer and echo the quit message to the chat
    const char* szNick = pPlayer->GetNick ();
    char* szReason = "Unknown";
    switch ( Reason )
    {
        case QUIT_QUIT: szReason = "Quit"; break;
        case QUIT_KICK: szReason = "Kicked"; break;
        case QUIT_BAN: szReason = "Banned"; break;
        case QUIT_CONNECTION_DESYNC: szReason = "Bad Connection"; break;
        case QUIT_TIMEOUT: szReason = "Timed Out"; break;
    }

    // In debug, make sure we don't look at this player's details
#ifdef MTA_DEBUG
    if ( m_pShowPlayer == pPlayer )
    {
        m_pShowPlayer = NULL;
    }
#endif

    // Call our onClientPlayerQuit event
    CLuaArguments Arguments;
    Arguments.PushString ( szReason );
    pPlayer->CallEvent ( "onClientPlayerQuit", Arguments, true );

    // Detach the camera from this player if we're watching them
    m_pManager->GetCamera ()->UnreferencePlayer ( pPlayer );

    // Was this player jacking us?
    if ( m_bIsGettingJacked && m_pGettingJackedBy == pPlayer )
    {
        ResetVehicleInOut ();
        m_pLocalPlayer->RemoveFromVehicle ( false );
        m_pLocalPlayer->SetVehicleInOutState ( VEHICLE_INOUT_NONE );
    }

    // Delete the player
    delete pPlayer;
}


void CClientGame::Event_OnIngame ( void )
{
    // Unpause the game
    g_pGame->Pause ( false );

    // Disable parts of the Hud
    CHud* pHud = g_pGame->GetHud ();
    pHud->DisableHelpText ( true );
    pHud->DisableVitalStats ( true );
    pHud->DisableAreaName ( true );

    g_pMultiplayer->DeleteAndDisableGangTags ();

    // Switch off peds and traffic
    g_pGame->GetPathFind ()->SwitchRoadsOffInArea ( &CVector(-100000.0f, -100000.0f, -100000.0f), &CVector(100000.0f, 100000.0f, 100000.0f) );
    g_pGame->GetPathFind ()->SwitchPedRoadsOffInArea ( &CVector(-100000.0f, -100000.0f, -100000.0f), &CVector(100000.0f, 100000.0f, 100000.0f) );
    g_pGame->GetPathFind ()->SetPedDensity ( 0.0f );
    g_pGame->GetPathFind ()->SetVehicleDensity ( 0.0f );

    // Make sure we can access all areas
    g_pGame->GetStats()->ModifyStat ( CITIES_PASSED, 2.0 );

    // This is to prevent the 'white arrows in checkpoints' bug (#274)
    g_pGame->Get3DMarkers()->CreateMarker ( 87654, (e3DMarkerType)5, &CVector(0,0,0), 1, 0.2f, 0, 0, 0, 0 );

    // Stop us getting 4 stars if we visit the SF or LV
    //g_pGame->GetPlayerInfo()->GetWanted()->SetMaximumWantedLevel ( 0 );
    g_pGame->GetPlayerInfo()->GetWanted()->SetWantedLevel ( 0 );  

    // Reset anything from last game
    ResetMapInfo ();
    g_pGame->GetWaterManager ()->Reset ();      // Deletes all custom water elements, ResetMapInfo only reverts changes to water level

    // Create a local player for us
    m_pLocalPlayer = new CClientPlayer ( m_pManager, m_LocalID, true );
    if ( m_pLocalPlayer )
    {
        // Set our parent the root entity
        m_pLocalPlayer->SetParent ( m_pRootEntity );

        // Give the local player our nickname
        m_pLocalPlayer->SetNick ( m_szLocalNick );

        // Freeze the player at some location we won't see
        m_pLocalPlayer->SetHealth ( 100 );
        m_pLocalPlayer->SetPosition ( CVector ( 0, 0, 0 ) );
        m_pLocalPlayer->SetFrozen ( true );
        m_pLocalPlayer->ResetInterpolation ();

        // Reset him
        m_pLocalPlayer->ResetStats ();
    }
    else
    {
        RaiseFatalError ( 2 );
    }

    // Make sure we never get tired
    g_pGame->GetPlayerInfo ()->SetDoesNotGetTired ( true );

    // Tell doggy we got the game running
    WatchDogCompletedSection ( "L1" );
}


void CClientGame::Event_OnIngameAndReady ( void )
{
    // Fade in
    //m_pCamera->FadeIn ( 1.0f );
}


void CClientGame::Event_OnIngameAndConnected ( void )
{
    // Create a messagebox saying that we're verifying the client
    //g_pCore->ShowMessageBox ( "Connecting", "Verifying client ...", false );
    m_ulVerifyTimeStart = CClientTime::GetTime ();
    
    /*
    // Notify the server telling we're ingame
    NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
    if ( pBitStream )
    {
        g_pNet->SendPacket ( PACKET_ID_PLAYER_INGAME_NOTICE, pBitStream );
        g_pNet->DeallocateNetBitStream ( pBitStream );
    }*/
    m_pNetAPI->RPC ( PLAYER_INGAME_NOTICE );
}

bool CClientGame::StaticBreakTowLinkHandler ( CVehicle * pTowingVehicle )
{
    return g_pClientGame->BreakTowLinkHandler ( pTowingVehicle );
}

void CClientGame::StaticDrawRadarAreasHandler ( void )
{
    g_pClientGame->DrawRadarAreasHandler ( );
}

bool CClientGame::StaticDamageHandler ( CPed* pDamagePed, CEventDamage * pEvent )
{
    return g_pClientGame->DamageHandler ( pDamagePed, pEvent );
}

void CClientGame::StaticFireHandler ( CFire* pFire )
{
    g_pClientGame->FireHandler ( pFire );
}

void CClientGame::StaticProjectileInitiateHandler ( CClientProjectile * pProjectile )
{
    g_pClientGame->ProjectileInitiateHandler ( pProjectile );
}

void CClientGame::StaticRender3DStuffHandler ( void )
{
    g_pClientGame->Render3DStuffHandler ();
}

bool CClientGame::StaticChokingHandler ( unsigned char ucWeaponType )
{
    return g_pClientGame->ChokingHandler ( ucWeaponType );
}

void CClientGame::StaticAddAnimationHandler ( RpClump * pClump, AssocGroupId animGroup, AnimationId animID )
{
    g_pClientGame->AddAnimationHandler ( pClump, animGroup, animID );
}

void CClientGame::StaticBlendAnimationHandler ( RpClump * pClump, AssocGroupId animGroup, AnimationId animID, float fBlendDelta )
{
    g_pClientGame->BlendAnimationHandler ( pClump, animGroup, animID, fBlendDelta );
}

void CClientGame::StaticPreWorldProcessHandler ( void )
{
    g_pClientGame->PreWorldProcessHandler ();
}

void CClientGame::StaticPostWorldProcessHandler ( void )
{
    g_pClientGame->PostWorldProcessHandler ();
}


void CClientGame::StaticIdleHandler ( void )
{
    g_pClientGame->IdleHandler ();
}

bool CClientGame::StaticProcessCollisionHandler ( CEntitySAInterface* pThisInterface, CEntitySAInterface* pOtherInterface )
{
    return g_pClientGame->ProcessCollisionHandler ( pThisInterface, pOtherInterface );
}

void CClientGame::DrawRadarAreasHandler ( void )
{
    m_pRadarAreaManager->DoPulse ();
}

bool CClientGame::BreakTowLinkHandler ( CVehicle* pTowedVehicle )
{
    CClientVehicle * pVehicle = m_pVehicleManager->Get ( pTowedVehicle, false );
    if ( pVehicle )
    {
        // Check if this is a legal break
        bool bLegal = ( ( pVehicle->GetControllingPlayer () == m_pLocalPlayer ) ||
                        ( m_pUnoccupiedVehicleSync->Exists ( static_cast < CDeathmatchVehicle * > ( pVehicle ) ) ) );
        
        // Not a legal break?
        if ( !bLegal )
        {
            // Save the time it broke (used in UpdateTrailers)
            pVehicle->SetIllegalTowBreakTime ( GetTickCount32 () );
        }
    }

    // Allow it to break
    return true;
}

void CClientGame::FireHandler ( CFire * pFire )
{
    // Disable spreading fires
    pFire->SetNumGenerationsAllowed ( 0 );
}


void CClientGame::ProjectileInitiateHandler ( CClientProjectile * pProjectile )
{
    if ( pProjectile->IsLocal () )
    {
        // Did the local player create this projectile?
        if ( m_pLocalPlayer && pProjectile->GetCreator () == m_pLocalPlayer )
        {
            // Physics says our projectile should start off at our velocity
            CVector vecVelocity, vecPlayerVelocity;
            pProjectile->GetVelocity ( vecVelocity );
            m_pLocalPlayer->GetMoveSpeed ( vecPlayerVelocity );
            vecVelocity += vecPlayerVelocity;
            pProjectile->SetVelocity ( vecVelocity );
        }

        SendProjectileSync ( pProjectile );
    }

    // Renew the interior and dimension
    if ( pProjectile->GetCreator() )
    {
        pProjectile->SetInterior ( pProjectile->GetCreator()->GetInterior() );
        pProjectile->SetDimension ( pProjectile->GetCreator()->GetDimension() );
    }

    // Validate the projectile for our element tree
    pProjectile->SetParent ( m_pRootEntity );

    // Call our creation event
    CLuaArguments Arguments;
    Arguments.PushElement ( pProjectile->GetCreator () );
    pProjectile->CallEvent ( "onClientProjectileCreation", Arguments, true );
}


void CClientGame::Render3DStuffHandler ( void )
{

}


void CClientGame::PreWorldProcessHandler ( void )
{
    // If we are not minimized we do the pulsing here
    if ( !g_pCore->IsWindowMinimized () )
    {
        m_uiNotPulsedCounter = 0;
        DoPulses ();
    }
}

void CClientGame::PostWorldProcessHandler ( void )
{
    m_pManager->GetMarkerManager ()->DoPulse ();

    // Update frame time slice
    DWORD dwCurrentTick = GetTickCount32 ();
    if ( m_dwLastFrameTick )
    {
        m_dwFrameTimeSlice = dwCurrentTick - m_dwLastFrameTick;

        // Call onClientPreRender LUA event
        CLuaArguments Arguments;
        Arguments.PushNumber ( m_dwFrameTimeSlice );
        m_pRootEntity->CallEvent ( "onClientPreRender", Arguments, false );
    }
    m_dwLastFrameTick = dwCurrentTick;
}


void CClientGame::IdleHandler ( void )
{
    // If we are minimized we do the pulsing here
    if ( g_pCore->IsWindowMinimized() )
    {
        DoPulses ();
    }
}


bool CClientGame::ChokingHandler ( unsigned char ucWeaponType )
{
    if ( !m_pLocalPlayer )
        return true;
    CLuaArguments Arguments;
    Arguments.PushNumber ( ucWeaponType );
    return m_pLocalPlayer->CallEvent ( "onClientPlayerChoke", Arguments, true );
}


void CClientGame::AddAnimationHandler ( RpClump * pClump, AssocGroupId animGroup, AnimationId animID )
{
    //CClientPed * pPed = m_pPedManager->Get ( pClump, true );
}

void CClientGame::BlendAnimationHandler ( RpClump * pClump, AssocGroupId animGroup, AnimationId animID, float fBlendDelta )
{   
    //CClientPed * pPed = m_pPedManager->Get ( pClump, true );
}

bool CClientGame::ProcessCollisionHandler ( CEntitySAInterface* pThisInterface, CEntitySAInterface* pOtherInterface )
{
    if ( pThisInterface == pOtherInterface )
        return true;

    if ( !m_BuiltCollisionMapThisFrame )
    {
        // Build a map of CPhysicalSAInterface*/CClientEntity*'s that have collision disabled
        m_BuiltCollisionMapThisFrame = true;
        m_CachedCollisionMap.clear ();

        std::map < CClientEntity*, bool > ::iterator iter = m_AllDisabledCollisions.begin ();
        for ( ; iter != m_AllDisabledCollisions.end () ; iter++ )
        {
            CClientEntity* pEntity = iter->first;
            CEntity* pGameEntity = pEntity->GetGameEntity ();
            CEntitySAInterface* pInterface = pGameEntity ? pGameEntity->GetInterface () : NULL;

            if ( pInterface )
                m_CachedCollisionMap[ pInterface ] = pEntity;
        }
    }

    // Check both elements appear in the cached map before doing extra processing
    std::map < CEntitySAInterface*, CClientEntity* > ::iterator iter1 = m_CachedCollisionMap.find ( (CEntitySAInterface*)pThisInterface );
    if ( iter1 != m_CachedCollisionMap.end () )
    {
        std::map < CEntitySAInterface*, CClientEntity* > ::iterator iter2 = m_CachedCollisionMap.find ( (CEntitySAInterface*)pOtherInterface );
        if ( iter2 != m_CachedCollisionMap.end () )
        {
            // Re-get the entity pointers using a safer method
            CEntity * pGameEntity = g_pGame->GetPools ()->GetEntity ( ( DWORD* ) pThisInterface );
            CEntity * pGameColEntity = g_pGame->GetPools ()->GetEntity ( ( DWORD* ) pOtherInterface );

            if ( pGameEntity && pGameColEntity )
            {        
                CClientEntity * pEntity = m_pManager->FindEntity ( pGameEntity, true );
                CClientEntity * pColEntity = m_pManager->FindEntity ( pGameColEntity, true );

                if ( pEntity && pColEntity )
                {
                    #if MTA_DEBUG
                        CClientEntity* ppThisEntity2 = iter1->second;
                        CClientEntity* ppOtherEntity2 = iter2->second;
                        // These should match, but its not essential.
                        assert ( ppThisEntity2 == pEntity );
                        assert ( ppOtherEntity2 == pColEntity );
                    #endif
                    if ( !pEntity->IsCollidableWith ( pColEntity ) ) return false;
                }
            }
        }
    }

    return true;
}

void CClientGame::DownloadFiles ( void )
{
    m_pTransferBox->DoPulse ();

    CNetHTTPDownloadManagerInterface* pHTTP = g_pNet->GetHTTPDownloadManager ();
    if ( pHTTP )
    {
        if ( !pHTTP->ProcessQueuedFiles () )
        {
            if ( m_dwTransferStarted == 0 || m_bTransferReset )
            {
                m_dwTransferStarted = GetTickCount32 ();

                m_bTransferReset = false;
            }

            m_pTransferBox->SetInfo ( pHTTP->GetDownloadSizeNow () );
        }
        else
        {
            // Get the last error to occur in the HTTP Manager
            const char* szHTTPError = pHTTP->GetError ();

            // Was an error found?
            if ( strlen (szHTTPError) == 0 )
            {
                m_bTransferReset = true;

                Event_OnTransferComplete ();
                m_dwTransferStarted = 0;
            }
            else
            {
                // Throw the error and disconnect
                g_pCore->GetModManager ()->RequestUnload ();
                g_pCore->ShowMessageBox ( "Error", szHTTPError, MB_BUTTON_OK | MB_ICON_ERROR );
                g_pCore->GetConsole ()->Printf ( "Download error: %s", szHTTPError );
            }
        }
    }
}


//
// DamageHandler seems to be called 3 times for each bit of damage:
//
// pass 1 - preApplyDamage
//   is pass 1 if: fDamage == zero  &&  fCurrentHealth == fPreviousHealth
//      returning false stops any damage being inflicted
//      returning true applies damage and allows pass 2
// pass 2 - postApplyDamage/preApplyAnim
//   is pass 2 if: fDamage > zero  &&  fCurrentHealth != fPreviousHealth
//      returning false stops damage anim
//      returning true allows damage anim and allows pass 3
// pass 3 - postApplyAnim
//   is pass 3 if: fDamage > zero  &&  fCurrentHealth == fPreviousHealth
//      returning false ??
//      returning true ??
//
bool CClientGame::DamageHandler ( CPed* pDamagePed, CEventDamage * pEvent )
{
    // CEventDamage::AffectsPed: This is/can be called more than once for each bit of damage (and may not actually take any more health (even if we return true))
    
    // Grab some data from the event
    CEntity * pInflictor = pEvent->GetInflictingEntity ();
    eWeaponType weaponUsed = pEvent->GetWeaponUsed ();
    ePedPieceTypes hitZone = pEvent->GetPedPieceType ();
    CWeaponInfo* pWeaponInfo = g_pGame->GetWeaponInfo ( weaponUsed );
    float fDamage = pEvent->GetDamageApplied ();    

    /* Causes too much desync right now
    // Is this shotgun damage?
    if ( weaponUsed == WEAPONTYPE_SHOTGUN || weaponUsed == WEAPONTYPE_SPAS12_SHOTGUN )
    {
        // Make the ped fall down
        pEvent->MakePedFallDown ();
    } */

    // Grab the damaged ped
    CClientPed* pDamagedPed = NULL;    
    if ( pDamagePed ) pDamagedPed = m_pPedManager->Get ( dynamic_cast < CPlayerPed* > ( pDamagePed ), true, true );

    // Grab the inflictor
    CClientEntity* pInflictingEntity = NULL;
    if ( pInflictor ) pInflictingEntity = m_pManager->FindEntity ( pInflictor );

    // If the damage was caused by an explosion
    if ( weaponUsed == WEAPONTYPE_EXPLOSION )
    {
        CClientEntity * pLastExplosionCreator = m_pManager->GetExplosionManager ()->m_pLastCreator;
        
        // If we don't have an inflictor, look for the last explosion creator
        if ( !pInflictor && pLastExplosionCreator ) pInflictingEntity = pLastExplosionCreator;
        
        // Change the weapon used to whatever created the explosion
        weaponUsed = m_pManager->GetExplosionManager ()->m_LastWeaponType;
    }

    // Do we have a damaged ped?
    if ( pDamagedPed )
    {
        float fPreviousHealth = pDamagedPed->m_fHealth;
        float fCurrentHealth = pDamagedPed->GetGamePlayer ()->GetHealth ();
        float fPreviousArmor = pDamagedPed->m_fArmor;
        float fCurrentArmor = pDamagedPed->GetGamePlayer ()->GetArmor ();

        // Is the damaged ped a player?
        if ( IS_PLAYER ( pDamagedPed ) )
        {
            CClientPlayer * pDamagedPlayer = static_cast < CClientPlayer * > ( pDamagedPed );

            // Is this is a remote player?
            if ( !pDamagedPed->IsLocalPlayer () )
            {  
                // Don't allow GTA to start the choking task
                if ( weaponUsed == WEAPONTYPE_TEARGAS || weaponUsed == WEAPONTYPE_SPRAYCAN || weaponUsed == WEAPONTYPE_EXTINGUISHER )
                    return false;
            }

            // Do we have an inflicting entity?
            if ( pInflictingEntity )
            {
                // Grab the inflicting player
                CClientPlayer * pInflictingPlayer = NULL;

                switch ( pInflictingEntity->GetType () )
                {
                    case CCLIENTPLAYER:
                    {
                        pInflictingPlayer = static_cast < CClientPlayer * > ( pInflictingEntity );                        
                        break;
                    }
                    case CCLIENTVEHICLE:
                    {
                        CClientVehicle * pInflictingVehicle = static_cast < CClientVehicle * > ( pInflictingEntity );
                        if ( pInflictingVehicle && pInflictingVehicle->GetControllingPlayer () )
                        {
                            CClientPed * pPed = static_cast < CClientPed * > ( pInflictingVehicle->GetControllingPlayer() );
                            if ( pPed && pPed->GetType () == CCLIENTPLAYER )
                                pInflictingPlayer = static_cast < CClientPlayer * > ( pPed );
                        }
                        break;
                    }
                    default: break;
                }
                if ( pInflictingPlayer )
                {
                    // Is the damaged player on a team
                    CClientTeam* pTeam = pDamagedPlayer->GetTeam ();
                    if ( pTeam )
                    {
                        // Is this friendly-fire from a team-mate?
                        if ( pDamagedPlayer->IsOnMyTeam ( pInflictingPlayer ) && !pTeam->GetFriendlyFire () && pDamagedPlayer != pInflictingPlayer )
                            return false;
                    }
                }
            }
        }
        // Have we taken any damage here?
        if ( ( fPreviousHealth != fCurrentHealth || fPreviousArmor != fCurrentArmor ) && fDamage != 0.0f )
        {
            CLuaArguments Arguments;
            if ( pInflictingEntity ) Arguments.PushElement ( pInflictingEntity );
            else Arguments.PushBoolean ( false );
            Arguments.PushNumber ( static_cast < unsigned char > ( weaponUsed ) );
            Arguments.PushNumber ( static_cast < unsigned char > ( hitZone ) );
            Arguments.PushNumber ( fDamage );

            // Call our event
            if ( ( IS_PLAYER ( pDamagedPed ) && !pDamagedPed->CallEvent ( "onClientPlayerDamage", Arguments, true ) ) || ( !IS_PLAYER ( pDamagedPed ) && !pDamagedPed->CallEvent ( "onClientPedDamage", Arguments, true ) ) )
            {
                // Stop here if they cancelEvent it
                if ( pDamagedPed->IsLocalPlayer () )
                {
                    // Reget values in case they have been changed during onClientPlayerDamage event (Avoid AC#1 kick)
                    fPreviousHealth = pDamagedPed->m_fHealth;
                    fPreviousArmor = pDamagedPed->m_fArmor;
                }
                pDamagedPed->GetGamePlayer ()->SetHealth ( fPreviousHealth );
                pDamagedPed->GetGamePlayer ()->SetArmor ( fPreviousArmor );
                return false;
            }

            if ( pDamagedPed->IsLocalPlayer () )
            {
                // Reget values in case they have been changed during onClientPlayerDamage event (Avoid AC#1 kick)
                fCurrentHealth = pDamagedPed->GetGamePlayer ()->GetHealth ();
                fCurrentArmor = pDamagedPed->GetGamePlayer ()->GetArmor ();
            }

            bool bIsBeingShotWhilstAiming = ( weaponUsed >= WEAPONTYPE_PISTOL && weaponUsed <= WEAPONTYPE_MINIGUN && pDamagedPed->IsUsingGun () );

            // Check if their health or armor is locked, and if so prevent applying the damage locally
            if ( pDamagedPed->IsHealthLocked () || pDamagedPed->IsArmorLocked () )
            {
                // Restore health+armor
                pDamagedPed->GetGamePlayer ()->SetHealth ( pDamagedPed->GetHealth () );
                pDamagedPed->GetGamePlayer ()->SetArmor ( pDamagedPed->GetArmor () );

                // Don't play the animation if it's going to be a death one, or if it's going to interrupt aiming
                if ( fCurrentHealth == 0.0f || bIsBeingShotWhilstAiming ) return false;

                // Allow animation and ensure the code below is not executed if health and armor are locked (i.e. remote players)
                return true;
            }

            // Update our stored health/armor
            pDamagedPed->m_fHealth = fCurrentHealth;
            pDamagedPed->m_fArmor = fCurrentArmor;

            // Is it the local player?
            if ( pDamagedPed->IsLocalPlayer () )
            {  
                // Update our stored damage stuff
                m_ucDamageWeapon = static_cast < unsigned char > ( weaponUsed );
                m_ucDamageBodyPiece = static_cast < unsigned char > ( hitZone );
                m_pDamageEntity = pInflictingEntity;
                m_ulDamageTime = CClientTime::GetTime ();
                m_DamagerID = INVALID_ELEMENT_ID;
                if ( pInflictingEntity ) m_DamagerID = pInflictingEntity->GetID ();
                m_bDamageSent = false;
            }
            // Does this damage kill the player?
            if ( fCurrentHealth == 0.0f )
            {                
                if ( pDamagedPed->GetType () == CCLIENTPLAYER )
                {                
                    // Is the local player dying?
                    if ( pDamagedPed->IsLocalPlayer () && fPreviousHealth > 0.0f )
                    {
                        // Grab our death animation
                        pEvent->ComputeDeathAnim ( pDamagePed, true );
                        AssocGroupId animGroup = pEvent->GetAnimGroup ();
                        AnimationId animID = pEvent->GetAnimId ();

                        // Check if we're dead
                        DoWastedCheck ( m_DamagerID, m_ucDamageWeapon, m_ucDamageBodyPiece, animGroup, animID );                
                    }

                    // Allow GTA to kill us if we've fell to our death
                    if ( pDamagedPed->IsLocalPlayer () && weaponUsed == WEAPONTYPE_FALL ) return true;
                    
                    // Don't let GTA start the death task
                    return false;            
                }
                else
                {
                    if ( pDamagedPed->IsLocalEntity () && fPreviousHealth > 0.0f )
                    {
                        pDamagedPed->CallEvent ( "onClientPedWasted", Arguments, true );
                        pEvent->ComputeDeathAnim ( pDamagePed, true );
                        AssocGroupId animGroup = pEvent->GetAnimGroup ();
                        AnimationId animID = pEvent->GetAnimId ();
                        pDamagedPed->Kill ( weaponUsed, m_ucDamageBodyPiece, false, false, animGroup, animID );
                        return true;
                    }
                    if ( fPreviousHealth > 0.0f )
                    {
                        // Grab our death animation
                        pEvent->ComputeDeathAnim ( pDamagePed, true );
                        AssocGroupId animGroup = pEvent->GetAnimGroup ();
                        AnimationId animID = pEvent->GetAnimId ();
                        m_ulDamageTime = CClientTime::GetTime ();
                        m_DamagerID = INVALID_ELEMENT_ID;
                        if ( pInflictingEntity ) m_DamagerID = pInflictingEntity->GetID ();

                        // Check if we're dead
                        SendPedWastedPacket ( pDamagedPed, m_DamagerID, m_ucDamageWeapon, m_ucDamageBodyPiece, animGroup, animID );
                    }
                }
            }
            // Inhibit hit-by-gun animation for local player if required
            if ( pDamagedPed->IsLocalPlayer () && bIsBeingShotWhilstAiming ) return false;
        }
    }

    // No damage anim for fire
    if ( weaponUsed == WEAPONTYPE_FLAMETHROWER ) return false;
    
    // Allow the damage to register
    return true;
}


bool CClientGame::StaticProcessMessage ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if ( g_pClientGame )
    {
        return g_pClientGame->ProcessMessage ( hwnd, uMsg, wParam, lParam );
    }

    return false;
}

bool CClientGame::ProcessMessage ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // Is it a trainer monitor message?
#ifdef MTA_DEBUG
    #define MESSAGE_MONITOR_START       0x8500
    #define MESSAGE_MONITOR_END         0x8501
    #define MESSAGE_MONITOR_RPM         0x8502
    #define MESSAGE_MONITOR_WPM         0x8503

    switch ( uMsg )
    {
        case MESSAGE_MONITOR_START:
        {
            // Tell the console
            if ( wParam == 0 )
            {
                g_pCore->GetConsole ()->Print ( "Successfully attached to trainer for monitoring" );
            }
            else
            {
                g_pCore->GetConsole ()->Print ( "Attaching to trainer failed" );
            }

            return true;
        }

        case MESSAGE_MONITOR_END:
        {
            // Clear the histories
            m_RPMHistory.Clear ();
            m_WPMHistory.Clear ();

            // Tell the console and return
            g_pCore->GetConsole ()->Print ( "Trainer stopped" );
            return true;
        }

        case MESSAGE_MONITOR_RPM:
        {
            // Grab the offset and the size from the message
            unsigned long ulOffset = static_cast < unsigned long > ( wParam );
            unsigned int uiSize = static_cast < unsigned int > ( lParam );
            AddressInfo addressInfo;
            addressInfo.uiType = 0;
            addressInfo.ulOffset = 0;
            if ( GetAddressInfo ( ulOffset, &addressInfo ) )
            {
                // Does it exist in our history?
                if ( !m_RPMHistory.Exists ( addressInfo.ulOffset, uiSize, addressInfo.uiType ) )
                {
                    // Add it to the history
                    m_RPMHistory.Add ( addressInfo.ulOffset, uiSize, addressInfo.uiType );

                    // Tell the console
                    switch ( addressInfo.uiType )
                    {
                        case MTA_OFFSET_BASE:
                            g_pCore->GetConsole ()->Printf ( "RPM: Base offset = 0x%x - Size = %u bytes", addressInfo.ulOffset, uiSize );
                            break;
                        case MTA_OFFSET_PLAYER:
                            g_pCore->GetConsole ()->Printf ( "RPM: Player offset = 0x%x - Size = %u bytes", addressInfo.ulOffset, uiSize );
                            break;
                        case MTA_OFFSET_VEHICLE:
                            g_pCore->GetConsole ()->Printf ( "RPM: Vehicle offset = 0x%x - Size = %u bytes", addressInfo.ulOffset, uiSize );
                            break;
                    }
                }
            }

            return true;
        }

        case MESSAGE_MONITOR_WPM:
        {
            // Grab the offset and the size from the message
            unsigned long ulOffset = static_cast < unsigned long > ( wParam );
            unsigned int uiSize = static_cast < unsigned int > ( lParam );
            AddressInfo addressInfo;
            addressInfo.uiType = 0;
            addressInfo.ulOffset = 0;
            if ( g_pClientGame->GetAddressInfo ( ulOffset, &addressInfo ) )
            {
                // Does it exist in our history?
                if ( !g_pClientGame->m_WPMHistory.Exists ( addressInfo.ulOffset, uiSize, addressInfo.uiType ) )
                {
                    // Add it to the history
                    g_pClientGame->m_WPMHistory.Add ( addressInfo.ulOffset, uiSize, addressInfo.uiType );

                    // Tell the console
                    switch ( addressInfo.uiType )
                    {
                    case MTA_OFFSET_BASE:
                        g_pCore->GetConsole ()->Printf ( "WPM: Base offset = 0x%x - Size = %u bytes", addressInfo.ulOffset, uiSize );
                        break;
                    case MTA_OFFSET_PLAYER:
                        g_pCore->GetConsole ()->Printf ( "WPM: Player offset = 0x%x - Size = %u bytes", addressInfo.ulOffset, uiSize );
                        break;
                    case MTA_OFFSET_VEHICLE:
                        g_pCore->GetConsole ()->Printf ( "WPM: Vehicle offset = 0x%x - Size = %u bytes", addressInfo.ulOffset, uiSize );
                        break;
                    }
                }
            }

            return true;
        }
    }
#endif

    if ( ProcessMessageForCursorEvents ( hwnd, uMsg, wParam, lParam ) )
    {
        return true;
    }

    return false;
}


void CClientGame::ProcessVehicleInOutKey ( bool bPassenger )
{
    // Are we already sending an in/out request or not allowed to create a new in/out?
    if ( !m_bNoNewVehicleTask &&
          m_VehicleInOutID == INVALID_ELEMENT_ID &&
         !m_bIsGettingJacked &&
         !m_bIsGettingIntoVehicle &&
         !m_bIsGettingOutOfVehicle &&
         CClientTime::GetTime () >= m_ulLastVehicleInOutTime + VEHICLE_INOUT_DELAY )
    {
        // Reset the "is jacking" bit
        m_bIsJackingVehicle = false;

        // Got a local player model?
        if ( m_pLocalPlayer )
        {
            // Is he in any vehicle?
            CClientVehicle* pOccupiedVehicle = m_pLocalPlayer->GetOccupiedVehicle ();
            if ( pOccupiedVehicle )
            {
                if ( !bPassenger || m_pLocalPlayer->GetOccupiedVehicleSeat () != 0 )
                {
                    // We're about to exit a vehicle
                    // Send an in request
                    NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
                    if ( pBitStream )
                    {
                        // Write the vehicle id to it and that we're requesting to get out of it
                        pBitStream->WriteCompressed ( pOccupiedVehicle->GetID () );
                        unsigned char ucAction = static_cast < unsigned char > ( VEHICLE_REQUEST_OUT );
                        pBitStream->WriteBits ( &ucAction, 4 );

                        // Send and destroy it
                        g_pNet->SendPacket ( PACKET_ID_VEHICLE_INOUT, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
                        g_pNet->DeallocateNetBitStream ( pBitStream );

                        // We're now exiting a vehicle
                        m_bIsGettingOutOfVehicle = true;
                        m_ulLastVehicleInOutTime = CClientTime::GetTime ();

#ifdef MTA_DEBUG
                        g_pCore->GetConsole ()->Printf ( "* Sent_InOut: vehicle_request_out" );
#endif
                    }
                }
            }
            else
            {
                // We're going to enter a vehicle

                // Make sure we don't have any other primary tasks running, otherwise our 'enter-vehicle'
                // task will replace it and fuck it up!
                if ( !m_pLocalPlayer->GetCurrentPrimaryTask () )
                {
                    // Are we not holding the aim_weapon key?
                    SBindableGTAControl* pBind = g_pCore->GetKeyBinds ()->GetBindableFromControl ( "aim_weapon" );
                    if ( !pBind || !pBind->bState )
                    {
                        // Is he climbing?
                        if ( !m_pLocalPlayer->IsClimbing () )
                        {
                            // Make sure he doesn't have a jetpack
                            if ( !m_pLocalPlayer->HasJetPack () )
                            {
                                // Make sure we arent using a gun (have the gun task active) - we stop it in UpdatePlayerTasks anyway
                                if ( !m_pLocalPlayer->IsUsingGun () )
                                {
                                    // Make sure we arent running an animation
                                    if ( !m_pLocalPlayer->IsRunningAnimation () )
                                    {
                                        // Grab the closest vehicle
                                        unsigned int uiDoor = 0;
                                        CClientVehicle* pVehicle = m_pLocalPlayer->GetClosestVehicleInRange ( true, !bPassenger, bPassenger, false, &uiDoor, NULL, 20.0f );
                                        unsigned int uiSeat = uiDoor;

                                        // Make sure the door is not 0 if we're going for passenger, or 0 if we're going for driver
                                        if ( bPassenger && uiDoor == 0 ) uiSeat = 1;
                                        else if ( !bPassenger ) uiSeat = 0;

                                        if ( pVehicle && pVehicle->IsEnterable () )
                                        {
                                            // If the vehicle's a boat, make sure we're standing on it (we need a dif task to enter boats properly)
                                            if ( pVehicle->GetVehicleType () != CLIENTVEHICLE_BOAT || m_pLocalPlayer->GetContactEntity () == pVehicle )
                                            {
                                                // Send an in request
                                                NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
                                                if ( pBitStream )
                                                {
                                                    // Write the vehicle id to it and that we're requesting to get into it
                                                    pBitStream->WriteCompressed ( pVehicle->GetID () );
                                                    unsigned char ucAction = static_cast < unsigned char > ( VEHICLE_REQUEST_IN );
                                                    unsigned char ucSeat = static_cast < unsigned char > ( uiSeat );
                                                    bool bIsOnWater = pVehicle->IsOnWater ();
                                                    unsigned char ucDoor = static_cast < unsigned char > ( uiDoor );
                                                    pBitStream->WriteBits ( &ucAction, 4 );
                                                    pBitStream->WriteBits ( &ucSeat, 3 );
                                                    pBitStream->WriteBit ( bIsOnWater );
                                                    pBitStream->WriteBits ( &ucDoor, 3 );

                                                    // Send and destroy it
                                                    g_pNet->SendPacket ( PACKET_ID_VEHICLE_INOUT, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
                                                    g_pNet->DeallocateNetBitStream ( pBitStream );

                                                    // We're now entering a vehicle
                                                    m_bIsGettingIntoVehicle = true;
                                                    m_ulLastVehicleInOutTime = CClientTime::GetTime ();

#ifdef MTA_DEBUG
                                                    g_pCore->GetConsole ()->Printf ( "* Sent_InOut: vehicle_request_in" );
#endif
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Shot compensation (Jax):
// Positions the local player where he should of been on the shooting player's screen when he
// fired remotely. (the position we !reported! around the time he shot)
bool bShotCompensation = true;

// Temporary pointers for pre- and post-functions
CVector vecWeaponFirePosition, vecRemoteWeaponFirePosition;
CPlayerPed* pWeaponFirePed = NULL;

void CClientGame::PreWeaponFire ( CPlayerPed* pPlayerPed )
{
    pWeaponFirePed = pPlayerPed;

    // Got a local player model?
    CClientPed* pLocalPlayer = g_pClientGame->m_pLocalPlayer;
    if ( pLocalPlayer && pWeaponFirePed )
    {
        CClientVehicle* pVehicle = pLocalPlayer->GetRealOccupiedVehicle ();

        // Get the CClientNetPlayer class with the specified player ped
        CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->Get ( pWeaponFirePed, true );

        // Move both players to where they should be for shot compensation
        if ( pPlayer && !pPlayer->IsLocalPlayer ())
        {                   
            if ( bShotCompensation )
            {
                if ( !pVehicle || pLocalPlayer->GetOccupiedVehicleSeat() == 0 )
                {
                    // Warp back in time to where we were when this player shot (their latency)
                    
                    // We don't account for interpolation here, +250ms seems to work better
                    // ** Changed ajustment to +125ms as the position of this clients player on the firers screen
                    // has been changed. See CClientPed::UpdateTargetPosition() **
                    CVector vecPosition;
                    unsigned short usLatency = ( pPlayer->GetLatency () + 125 );
                    g_pClientGame->m_pNetAPI->GetInterpolation ( vecPosition, usLatency );
            
                    // Move the entity back
                    if ( pVehicle )
                    {
                        pVehicle->GetPosition ( vecWeaponFirePosition );
                        pVehicle->SetPosition ( vecPosition );
                    }
                    else
                    {
                        pLocalPlayer->GetPosition ( vecWeaponFirePosition );
                        pLocalPlayer->SetPosition ( vecPosition );
                    }
                }
            }
        }
    }
}


void CClientGame::PostWeaponFire ( void )
{
    // Got a local player model?
    CClientPed* pLocalPlayer = g_pClientGame->m_pLocalPlayer;
    if ( pLocalPlayer && pWeaponFirePed )
    {
        CClientPed * pPed = g_pClientGame->GetPedManager ()->Get ( pWeaponFirePed, true, true );
        if ( pPed )
        {
            if ( pPed->GetType () == CCLIENTPLAYER )
            {
                if ( bShotCompensation )
                {
                    // Restore compensated positions            
                    if ( !pPed->IsLocalPlayer () )
                    {
                        CClientVehicle* pVehicle = pLocalPlayer->GetRealOccupiedVehicle ();
                        if ( !pVehicle )
                        {
                            pLocalPlayer->SetPosition ( vecWeaponFirePosition );
                        }
                        else if ( pLocalPlayer->GetOccupiedVehicleSeat() == 0 )
                        {
                            pVehicle->SetPosition ( vecWeaponFirePosition );
                        }
                    }
                }
            }

            // Call some events
            CWeapon* pWeapon = pPed->GetWeapon ();
            if ( pWeapon )
            {
                CVector vecCollision;
                CClientEntity* pCollisionEntity = NULL;

                if ( pPed->GetBulletImpactData ( &pCollisionEntity, &vecCollision ) == false )
                {
                    CShotSyncData* pShotsyncData = pPed->m_shotSyncData;
                    CVector vecOrigin, vecTarget;
                    pPed->GetShotData ( &vecOrigin, &vecTarget );

                    CColPoint* pCollision = NULL;
                    CEntity* pCollisionGameEntity = NULL;
                    vecCollision = vecTarget;
                    bool bCollision = g_pGame->GetWorld ()->ProcessLineOfSight ( &vecOrigin, &vecTarget, &pCollision, &pCollisionGameEntity );
                    if ( bCollision && pCollision )
                        vecCollision = *pCollision->GetPosition ();

                    // Destroy the colpoint
                    if ( pCollision )
                    {
                        pCollision->Destroy ();
                    }

                    if ( pCollisionGameEntity )
                        pCollisionEntity = g_pClientGame->m_pManager->FindEntity ( pCollisionGameEntity );
                }
                else
                {
                    pPed->ClearBulletImpactData ();
                }

                // Call our lua event
                CLuaArguments Arguments;
                Arguments.PushNumber ( ( double ) pWeapon->GetType () );
                Arguments.PushNumber ( ( double ) pWeapon->GetAmmoTotal () );
                Arguments.PushNumber ( ( double ) pWeapon->GetAmmoInClip () );
                Arguments.PushNumber ( ( double ) vecCollision.fX );
                Arguments.PushNumber ( ( double ) vecCollision.fY );
                Arguments.PushNumber ( ( double ) vecCollision.fZ );
                if ( pCollisionEntity )
                    Arguments.PushElement ( pCollisionEntity );
                else
                    Arguments.PushNil ();
                if (IS_PLAYER(pPed))
                    pPed->CallEvent ( "onClientPlayerWeaponFire", Arguments, true );
                else
                    pPed->CallEvent ( "onClientPedWeaponFire", Arguments, true );
            }
            pPed->PostWeaponFire();
#ifdef MTA_DEBUG
            if ( pPed->IsLocalPlayer () && g_pClientGame->m_bDoPaintballs )
            {
                g_pClientGame->DoPaintballs ();
            }
#endif
        }        
    }
    pWeaponFirePed = NULL;
}

void CClientGame::BulletImpact ( CPed* pInitiator, CEntity* pVictim, const CVector* pStartPosition, const CVector* pEndPosition )
{
    // Got a local player model?
    CClientPed* pLocalPlayer = g_pClientGame->m_pLocalPlayer;
    if ( pLocalPlayer && pInitiator )
    {
        // Find the client ped that initiated the bullet impact
        CClientPed * pInitiatorPed = g_pClientGame->GetPedManager ()->Get ( dynamic_cast < CPlayerPed* > ( pInitiator ), true, true );

        if ( pInitiatorPed )
        {
            // Calculate the collision of the bullet
            CVector vecCollision;
            CColPoint* pCollision = NULL;
            bool bCollision = g_pGame->GetWorld ()->ProcessLineOfSight ( pStartPosition, pEndPosition, &pCollision, NULL );
            if ( bCollision && pCollision )
            {
                vecCollision = *pCollision->GetPosition ();
            }
            else
            {
                // If we don't have a collision, use the end of the ray that the bullet is tracing.
                vecCollision = *pEndPosition;
            }

            // Destroy the colpoint
            if ( pCollision )
            {
                pCollision->Destroy ();
            }

            // Find the client entity for the victim.
            CClientEntity* pClientVictim = NULL;
            if ( pVictim )
            {   
                pClientVictim = g_pClientGame->m_pManager->FindEntity ( pVictim );
            }

            // Store the data in the bullet fire initiator.
            pInitiatorPed->SetBulletImpactData ( pClientVictim, vecCollision );
        }
    }
}


bool CClientGame::StaticProcessPacket ( unsigned char ucPacketID, NetBitStreamInterface& bitStream )
{
    if ( g_pClientGame )
    {
        g_pClientGame->GetManager ()->GetPacketRecorder ()->RecordPacket ( ucPacketID, bitStream );
        return g_pClientGame->m_pPacketHandler->ProcessPacket ( ucPacketID, bitStream );
    }

    return false;
}


void CClientGame::Event_OnTransferComplete ( void )
{
    if ( m_bTransferResource )  /*** in-game transfer ***/
    {
        // Load our ("unavailable"-flagged) resources, and make them available
        m_pResourceManager->LoadUnavailableResources ( m_pRootEntity );

        // Disable m_bTransferResource (and hide the transfer box), if there are no more files in the autopatch query (to prevent "simulatenous" transfer fuck-ups)
        if ( !g_pNet->GetHTTPDownloadManager ()->IsDownloading () )
        {
            m_bTransferResource = false;
            m_pTransferBox->Hide ();
        }
    }
    else                        /*** on-join transfer ***/
    {
        // Hide the transfer box
        m_pTransferBox->Hide ();

        // Tell the core we're finished
        g_pCore->SetConnected ( true );
        g_pCore->HideMainMenu ();

        // We're now "Joining"
        m_Status = CClientGame::STATUS_JOINING;

        // If the game isn't started, start it
        if ( g_pGame->GetSystemState () == 7 )
        {
            g_pGame->StartGame ();
        }
    }
}


void CClientGame::SendExplosionSync ( const CVector& vecPosition, eExplosionType Type, CClientEntity * pOrigin )
{
    SPositionSync position ( false );
    position.data.vecPosition = vecPosition;

    // Create the bitstream
    NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
    if ( pBitStream )
    {
        // Write our origin id
        if ( pOrigin )
        {
            pBitStream->WriteBit ( true );
            pBitStream->WriteCompressed ( pOrigin->GetID () );

            // Convert position
            CVector vecTemp;
            pOrigin->GetPosition ( vecTemp );
            position.data.vecPosition -= vecTemp;
        }
        else
            pBitStream->WriteBit ( false );

        // Write the position and the type
        pBitStream->Write ( &position );

        SExplosionTypeSync explosionType;
        explosionType.data.uiType = Type;
        pBitStream->Write ( &explosionType );

        // Destroy it
        g_pNet->SendPacket ( PACKET_ID_EXPLOSION, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
        g_pNet->DeallocateNetBitStream ( pBitStream );
    }
}


void CClientGame::SendFireSync ( CFire* pFire )
{
#ifdef MTA_DEBUG
    CVector* vecPos = pFire->GetPosition ();
    if ( vecPos )
        g_pCore->GetConsole ()->Printf ( "we're sending fire: %f %f %f %f", pFire->GetStrength (), vecPos->fX, vecPos->fY, vecPos->fZ );
    else
        g_pCore->GetConsole ()->Printf ( "we're sending a fire!" );
#endif
}


void CClientGame::SendProjectileSync ( CClientProjectile * pProjectile )
{
    // Create the bitstream
    NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
    if ( pBitStream )
    {
        SPositionSync origin ( false );
        CClientEntity * pOriginSource = NULL;
        eWeaponType weaponType = pProjectile->GetWeaponType ();
        CClientEntity * pTarget = pProjectile->GetTargetEntity ();
        origin.data.vecPosition = *pProjectile->GetOrigin ();        

        // Is this a heatseaking missile with a target? sync it relative to the target
        if ( weaponType == WEAPONTYPE_ROCKET_HS && pTarget && !pTarget->IsLocalEntity () )
            pOriginSource = pTarget;

        // Write the source of the projectile, if it has
        if ( pOriginSource )
        {
            CVector vecTemp;
            pOriginSource->GetPosition ( vecTemp );
            origin.data.vecPosition -= vecTemp;

            pBitStream->WriteBit ( true );
            pBitStream->WriteCompressed ( pOriginSource->GetID () );
        }
        else
            pBitStream->WriteBit ( false );

        // Write the origin position
        pBitStream->Write ( &origin );

        // Write the creator weapon type
        SWeaponTypeSync weaponTypeSync;
        weaponTypeSync.data.ucWeaponType = static_cast < unsigned char > ( weaponType );
        pBitStream->Write ( &weaponTypeSync );

        switch ( weaponType )
        {
            case WEAPONTYPE_GRENADE:
            case WEAPONTYPE_TEARGAS:
            case WEAPONTYPE_MOLOTOV:
            case WEAPONTYPE_REMOTE_SATCHEL_CHARGE:
            {
                SFloatSync < 7, 17 > projectileForce;
                projectileForce.data.fValue = pProjectile->GetForce ();
                pBitStream->Write ( &projectileForce );

                SVelocitySync velocity;
                pProjectile->GetVelocity ( velocity.data.vecVelocity );
                pBitStream->Write ( &velocity );

                break;
            }
            case WEAPONTYPE_ROCKET:
            case WEAPONTYPE_ROCKET_HS:
            {
                if ( pTarget )
                {
                    pBitStream->WriteBit ( true );
                    pBitStream->WriteCompressed ( pTarget->GetID () );
                }
                else
                    pBitStream->WriteBit ( false );

                SVelocitySync velocity;
                pProjectile->GetVelocity ( velocity.data.vecVelocity ); 
                pBitStream->Write ( &velocity );

                SRotationRadiansSync rotation ( true );
                pProjectile->GetRotation ( rotation.data.vecRotation );
                pBitStream->Write ( &rotation );

                break;
            }
            case WEAPONTYPE_FLARE:
            case WEAPONTYPE_FREEFALL_BOMB:
                break;
        }
        g_pNet->SendPacket ( PACKET_ID_PROJECTILE, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );

        // Destroy it
        g_pNet->DeallocateNetBitStream ( pBitStream );
    }
}


void CClientGame::ResetMapInfo ( void )
{
    // Restore some game stuff
    // Keybinds
    g_pCore->GetKeyBinds ()->SetAllControlsEnabled ( true, true, true );
    
    // Radarmap
    m_pRadarMap->SetForcedState ( false );

    // Camera
    m_pCamera->FadeOut ( 0.0f, 0, 0, 0 );    
    g_pGame->GetWorld ()->SetCurrentArea ( 0 );
    m_pCamera->SetFocusToLocalPlayer ();

    // Dimension
    SetAllDimensions ( 0 );

    // Hud
    g_pGame->GetHud ()->DisableAll ( false );
    // Disable area names as they are on load until camera unfades
    g_pGame->GetHud ()->DisableAreaName ( true );
    g_pGame->GetHud ()->DisableVitalStats ( true );

    m_bHudAreaNameDisabled = false;       

    // Gravity
    g_pMultiplayer->SetLocalPlayerGravity ( DEFAULT_GRAVITY );
    g_pMultiplayer->SetGlobalGravity ( DEFAULT_GRAVITY );
    g_pGame->SetGravity ( DEFAULT_GRAVITY );

    // Gamespeed
    SetGameSpeed ( DEFAULT_GAME_SPEED );

    // Game minute duration
    SetMinuteDuration ( DEFAULT_MINUTE_DURATION );

    // Wanted-level
    g_pGame->GetPlayerInfo()->GetWanted()->SetWantedLevel ( 0 ); 

    // Money
    SetMoney ( 0 );

    // Water
    g_pGame->GetWaterManager ()->SetWaterLevel ( (CVector *)NULL, 0.0f, NULL, true );
    g_pGame->GetWaterManager ()->SetWaveLevel ( 0.0f );

    // Weather
    m_pBlendedWeather->SetWeather ( 0 );

    // Wind
    g_pMultiplayer->RestoreWindVelocity ( );

    // Far clip distance
    g_pMultiplayer->RestoreFarClipDistance ( );

    // Fog distance
    g_pMultiplayer->RestoreFogDistance ( );

    // Sun color
    g_pMultiplayer->ResetSunColor ( );

    // Sun size
    g_pMultiplayer->ResetSunSize ( );

    // Sky-gradient
    g_pMultiplayer->ResetSky ( );

    // Heat haze
    g_pMultiplayer->ResetHeatHaze ( );

    // Water-colour
    g_pMultiplayer->ResetWater ( );

    // Re-enable interior sounds
    g_pMultiplayer->SetInteriorSoundsEnabled ( true );

     // Clouds
    g_pMultiplayer->SetCloudsEnabled ( true );
    g_pClientGame->SetCloudsEnabled ( true );

    // Cheats
    g_pGame->ResetCheats ();

    // Players
    m_pPlayerManager->ResetAll ();

    // Jetpack max height
    g_pGame->GetWorld ()->SetJetpackMaxHeight ( DEFAULT_JETPACK_MAXHEIGHT );

    // Disable the change of any player stats
    g_pMultiplayer->SetLocalStatsStatic ( true );

    // Restore blur
#ifdef MTA_DEBUG
    g_pGame->SetBlurLevel ( 0 );
#else
    g_pGame->SetBlurLevel ( DEFAULT_BLUR_LEVEL );
#endif

    // Close all garages
    CGarage* pGarage = NULL;
    CGarages* pGarages = g_pCore->GetGame()->GetGarages();

    for ( unsigned char i = 0 ; (pGarage = pGarages->GetGarage( i )) != NULL ; i++ )
    {
        pGarage->SetOpen ( false );
    }

    // Player specific stuff
    if ( m_pLocalPlayer )
    {
        // Interior
        m_pLocalPlayer->SetInterior ( 0 );

        // Headless state
        m_pLocalPlayer->SetHeadless ( false );

        // Frozen state
        m_pLocalPlayer->SetFrozen ( false );

        // Voice
        short sVoiceType, sVoiceID;
        m_pLocalPlayer->GetModelInfo ()->GetVoice ( &sVoiceType, &sVoiceID );
        m_pLocalPlayer->SetVoice ( sVoiceType, sVoiceID );
    }
}

void CClientGame::SendPedWastedPacket( CClientPed* Ped, ElementID damagerID, unsigned char ucWeapon, unsigned char ucBodyPiece, AssocGroupId animGroup, AnimationId animID )
{
    if ( Ped && Ped->GetHealth () == 0.0f )
    {
        NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
        if ( pBitStream )
        {
            // Write some death info
            pBitStream->Write ( animGroup );
            pBitStream->Write ( animID );

            pBitStream->Write ( damagerID );
            pBitStream->Write ( ucWeapon );
            pBitStream->Write ( ucBodyPiece );

            // Write the position we died in
            CVector vecPosition;
            Ped->GetPosition ( vecPosition );
            pBitStream->Write ( vecPosition.fX );
            pBitStream->Write ( vecPosition.fY );
            pBitStream->Write ( vecPosition.fZ );

            // The ammo in our weapon and write the ammo total
            CWeapon* pPlayerWeapon = Ped->GetWeapon();
            unsigned short usAmmo = 0;
            if ( pPlayerWeapon ) usAmmo = static_cast < unsigned short > ( pPlayerWeapon->GetAmmoTotal () );
            pBitStream->Write ( usAmmo );

            pBitStream->Write ( Ped->GetID() );
            // Send the packet
            g_pNet->SendPacket ( PACKET_ID_PED_WASTED, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
            g_pNet->DeallocateNetBitStream ( pBitStream );
        }
    }
}

void CClientGame::DoWastedCheck ( ElementID damagerID, unsigned char ucWeapon, unsigned char ucBodyPiece, AssocGroupId animGroup, AnimationId animID )
{
    // Are we not already marked as dead? and have we run out of health?
    if ( !m_pLocalPlayer->IsDeadOnNetwork () && m_pLocalPlayer->GetHealth () == 0.0f )
    {
        // Send the "i am dead" packet
        NetBitStreamInterface* pBitStream = g_pNet->AllocateNetBitStream ();
        if ( pBitStream )
        {
            m_pLocalPlayer->SetDeadOnNetwork( true );

            // Call the onClientPlayerWasted event
            CClientEntity * pKiller = ( damagerID != INVALID_ELEMENT_ID ) ? CElementIDs::GetElement ( damagerID ) : NULL;
            CLuaArguments Arguments;
            if ( pKiller ) Arguments.PushElement ( pKiller );
            else Arguments.PushBoolean ( false );
            if ( ucWeapon != 0xFF ) Arguments.PushNumber ( ucWeapon );
            else Arguments.PushBoolean ( false );
            if ( ucBodyPiece != 0xFF ) Arguments.PushNumber ( ucBodyPiece );
            else Arguments.PushBoolean ( false );
            Arguments.PushBoolean ( false );
            m_pLocalPlayer->CallEvent ( "onClientPlayerWasted", Arguments, true );

            // Write some death info
            pBitStream->WriteCompressed ( animGroup );
            pBitStream->WriteCompressed ( animID );

            pBitStream->WriteCompressed ( damagerID );

            SWeaponTypeSync weapon;
            weapon.data.ucWeaponType = ucWeapon;
            pBitStream->Write ( &weapon );

            SBodypartSync bodyPart;
            bodyPart.data.uiBodypart = ucBodyPiece;
            pBitStream->Write ( &bodyPart );

            // Write the position we died in
            SPositionSync pos ( false );
            m_pLocalPlayer->GetPosition ( pos.data.vecPosition );
            pBitStream->Write ( &pos );

            // The ammo in our weapon and write the ammo total
            CWeapon* pPlayerWeapon = m_pLocalPlayer->GetWeapon();
            SWeaponAmmoSync ammo ( ucWeapon, true, false );
            ammo.data.usTotalAmmo = 0;
            if ( pPlayerWeapon )
                ammo.data.usTotalAmmo = static_cast < unsigned short > ( pPlayerWeapon->GetAmmoTotal () );
            pBitStream->Write ( &ammo );
            
            // Send the packet
            g_pNet->SendPacket ( PACKET_ID_PLAYER_WASTED, pBitStream, PACKET_PRIORITY_HIGH, PACKET_RELIABILITY_RELIABLE_ORDERED );
            g_pNet->DeallocateNetBitStream ( pBitStream );
        }
    }
}


bool CClientGame::OnKeyDown ( CGUIKeyEventArgs Args )
{
    return true;
}


bool CClientGame::OnMouseClick ( CGUIMouseEventArgs Args )
{
    if ( !Args.pWindow ) return false;

    char* szButton = NULL;
    char* szState = NULL;
    switch ( Args.button )
    {
    case CGUIMouse::LeftButton:
        szButton = "left";
        szState = "up";
        break;
    case CGUIMouse::MiddleButton:
        szButton = "middle";
        szState = "up";
        break;
    case CGUIMouse::RightButton:
        szButton = "right";
        szState = "up";
        break;
    }

    // Call the event handler in CCore
    bool bHandled = g_pCore->OnMouseClick ( Args );

    // Only pass on to lua if we haven't handled it yet
    if ( !bHandled && szButton ) {
        CLuaArguments Arguments;
        Arguments.PushString ( szButton );
        Arguments.PushString ( szState );
        Arguments.PushNumber ( Args.position.fX );
        Arguments.PushNumber ( Args.position.fY );

        CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pWindow );
        if ( GetGUIManager ()->Exists ( pGUIElement ) )
        {
            pGUIElement->CallEvent ( "onClientGUIClick", Arguments, true );
        }
    }

    return true;
}


bool CClientGame::OnMouseDoubleClick ( CGUIMouseEventArgs Args )
{
    if ( !Args.pWindow ) return false;

    char* szButton = NULL;
    char* szState = NULL;
    switch ( Args.button )
    {
    case CGUIMouse::LeftButton:
        szButton = "left";
        szState = "up";
        break;
    case CGUIMouse::MiddleButton:
        szButton = "middle";
        szState = "up";
        break;
    case CGUIMouse::RightButton:
        szButton = "right";
        szState = "up";
        break;
    }

    // Call the event handler in CCore
    bool bHandled = g_pCore->OnMouseDoubleClick ( Args );

    // Only pass on to lua if we haven't handled it yet
    if ( !bHandled && szButton ) {
        CLuaArguments Arguments;
        Arguments.PushString ( szButton );
        Arguments.PushString ( szState );
        Arguments.PushNumber ( Args.position.fX );
        Arguments.PushNumber ( Args.position.fY );

        CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pWindow );
        if ( GetGUIManager ()->Exists ( pGUIElement ) )
        {
            pGUIElement->CallEvent ( "onClientGUIDoubleClick", Arguments, true );
        }
    }

    return true;
}

bool CClientGame::OnMouseButtonDown ( CGUIMouseEventArgs Args )
{
    if ( !Args.pWindow )
        return false;

    const char* szButton = NULL;
    switch ( Args.button )
    {
    case CGUIMouse::LeftButton:
        szButton = "left";
        break;
    case CGUIMouse::MiddleButton:
        szButton = "middle";
        break;
    case CGUIMouse::RightButton:
        szButton = "right";
        break;
    }

    if ( szButton )
    {
        CLuaArguments Arguments;
        Arguments.PushString ( szButton );
        Arguments.PushNumber ( Args.position.fX );
        Arguments.PushNumber ( Args.position.fY );

        CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pWindow );
        if ( GetGUIManager ()->Exists ( pGUIElement ) )
        {
            pGUIElement->CallEvent ( "onClientGUIMouseDown", Arguments, true );
        }
    }

    return true;
}


bool CClientGame::OnMouseButtonUp ( CGUIMouseEventArgs Args )
{
    if ( !Args.pWindow )
        return false;

    const char* szButton = NULL;
    switch ( Args.button )
    {
    case CGUIMouse::LeftButton:
        szButton = "left";
        break;
    case CGUIMouse::MiddleButton:
        szButton = "middle";
        break;
    case CGUIMouse::RightButton:
        szButton = "right";
        break;
    }

    if ( szButton )
    {
        CLuaArguments Arguments;
        Arguments.PushString ( szButton );
        Arguments.PushNumber ( Args.position.fX );
        Arguments.PushNumber ( Args.position.fY );

        CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pWindow );
        if ( GetGUIManager ()->Exists ( pGUIElement ) )
        {
            pGUIElement->CallEvent ( "onClientGUIMouseUp", Arguments, true );
        }
    }

    return true;
}

bool CClientGame::OnMouseMove ( CGUIMouseEventArgs Args )
{
    if ( !Args.pWindow ) return false;

    CLuaArguments Arguments;
    Arguments.PushNumber ( Args.position.fX );
    Arguments.PushNumber ( Args.position.fY );

    CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pWindow );
    if ( GetGUIManager ()->Exists ( pGUIElement ) ) pGUIElement->CallEvent ( "onClientMouseMove", Arguments, true );

    return true;
}


bool CClientGame::OnMouseEnter ( CGUIMouseEventArgs Args )
{
    if ( !Args.pWindow ) return false;

    CLuaArguments Arguments;
    Arguments.PushNumber ( Args.position.fX );
    Arguments.PushNumber ( Args.position.fY );

    CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pWindow );
    if ( GetGUIManager ()->Exists ( pGUIElement ) ) pGUIElement->CallEvent ( "onClientMouseEnter", Arguments, true );

    return true;
}


bool CClientGame::OnMouseLeave ( CGUIMouseEventArgs Args )
{
    if ( !Args.pWindow ) return false;

    CLuaArguments Arguments;
    Arguments.PushNumber ( Args.position.fX );
    Arguments.PushNumber ( Args.position.fY );

    CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pWindow );
    if ( GetGUIManager ()->Exists ( pGUIElement ) ) pGUIElement->CallEvent ( "onClientMouseLeave", Arguments, true );

    return true;
}


bool CClientGame::OnMouseWheel ( CGUIMouseEventArgs Args )
{
    if ( !Args.pWindow ) return false;

    CLuaArguments Arguments;
    Arguments.PushNumber ( Args.wheelChange );

    CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pWindow );
    if ( GetGUIManager ()->Exists ( pGUIElement ) ) pGUIElement->CallEvent ( "onClientMouseWheel", Arguments, true );

    return true;
}


bool CClientGame::OnMove ( CGUIElement * pElement )
{
    if ( !pElement ) return false;

    CLuaArguments Arguments;

    CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( pElement );
    if ( pGUIElement && GetGUIManager()->Exists ( pGUIElement ) ) pGUIElement->CallEvent ( "onClientGUIMove", Arguments, true );

    return true;
}


bool CClientGame::OnSize ( CGUIElement * pElement )
{
    if ( !pElement ) return false;

    CLuaArguments Arguments;

    CClientGUIElement * pGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( pElement );
    if ( GetGUIManager ()->Exists ( pGUIElement ) ) pGUIElement->CallEvent ( "onClientGUISize", Arguments, true );

    return true;
}

bool CClientGame::OnFocusGain ( CGUIFocusEventArgs Args )
{
    if ( !Args.pActivatedWindow ) return false;

    CLuaArguments Arguments;

    CClientGUIElement * pActivatedGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pActivatedWindow );
    
    if ( Args.pDeactivatedWindow )
    {
        CClientGUIElement * pDeactivatedGUIElement = pDeactivatedGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pDeactivatedWindow );
        if ( GetGUIManager ()->Exists ( pDeactivatedGUIElement ) ) pDeactivatedGUIElement->CallEvent ( "onClientGUIBlur", Arguments, true );
    }

    if ( GetGUIManager ()->Exists ( pActivatedGUIElement ) ) pActivatedGUIElement->CallEvent ( "onClientGUIFocus", Arguments, true );

    return true;
}

bool CClientGame::OnFocusLoss ( CGUIFocusEventArgs Args )
{
    if ( !Args.pDeactivatedWindow ) return false;

    CLuaArguments Arguments;

    if ( Args.pActivatedWindow )
    {
        //pDeactivatedWindow looses focus but an other window is now gaining it so we let CClientGame::OnFocusGain trigger both events in the right order
        return true;
    }
    
    CClientGUIElement * pDeactivatedGUIElement = CGUI_GET_CCLIENTGUIELEMENT ( Args.pDeactivatedWindow );
    if ( GetGUIManager ()->Exists ( pDeactivatedGUIElement ) ) pDeactivatedGUIElement->CallEvent ( "onClientGUIBlur", Arguments, true );

    return true;
}

#ifdef MTA_DEBUG
AddressInfo * CClientGame::GetAddressInfo ( unsigned long ulOffset, AddressInfo * pAddressInfo )
{
    if ( !pAddressInfo )
    {
        return NULL;
    }

    #define SIZEOF_CPLAYERPED           1956
    #define SIZEOF_CHELI                2584 // the size of a helicopter, the largest
    #define SIZEOF_CBOAT                2024
    #define SIZEOF_CTRAIN               1708
    #define SIZEOF_CBIKE                2068
    #define SIZEOF_CBMX                 2104
    #define SIZEOF_CMONSTERTRUCK        2460
    #define SIZEOF_CQUADBIKE            2492
    #define SIZEOF_CPLANE               2564
    #define SIZEOF_CTRAILER             2548
    #define SIZEOF_CAUTOMOBILE          2440

    #define VTBL_CHELI                  0x871680
    #define VTBL_CBOAT                  0x8721A0
    #define VTBL_CTRAIN                 0x6F6030
    #define VTBL_CBIKE                  0x871360
    #define VTBL_CBMX                   0x871528
    #define VTBL_CMONSTERTRUCK          0x8717D8
    #define VTBL_CQUADBIKE              0x871AE8
    #define VTBL_CPLANE                 0x871948
    #define VTBL_CTRAILER               0x871C28
    #define VTBL_CAUTOMOBILE            0x871120

    CClientPlayer* pPlayer = NULL;
    vector < CClientPlayer* > ::const_iterator iter = m_pPlayerManager->IterBegin ();
    for ( ; iter != m_pPlayerManager->IterEnd (); iter++ )
    {
        pPlayer = *iter;

        unsigned long ulPlayerBaseAddress = pPlayer->GetGameBaseAddress ();
        if ( ulOffset >= ulPlayerBaseAddress &&
                ulOffset - ulPlayerBaseAddress <= SIZEOF_CPLAYERPED )
        {
            // its within this player
            pAddressInfo->uiType = MTA_OFFSET_PLAYER;
            pAddressInfo->ulOffset = ulOffset - ulPlayerBaseAddress;
            return pAddressInfo;
        }
    }

    CClientVehicle* pVehicle = NULL;
    vector < CClientVehicle* > ::const_iterator iterVehicle = m_pVehicleManager->IterBegin ();
    for ( ; iterVehicle != m_pVehicleManager->IterEnd (); iterVehicle++ )
    {
        // Grab the game address of the vehicle
        pVehicle = *iterVehicle;

        // It musn't be virtual, or we'll crash
        if ( !pVehicle->IsVirtual () )
        {
            unsigned long ulVehicleBaseAddress = pVehicle->GetGameBaseAddress ();

            // find the type of the vehicle and hence the size of it
            unsigned long ulSizeOfVehicle = 0;
            switch ( *(DWORD *)ulVehicleBaseAddress )
            {
                case VTBL_CHELI:
                    ulSizeOfVehicle = SIZEOF_CHELI;
                    break;
                case VTBL_CBOAT:
                    ulSizeOfVehicle = SIZEOF_CBOAT;
                    break;
                case VTBL_CTRAIN:
                    ulSizeOfVehicle = SIZEOF_CTRAIN;
                    break;
                case VTBL_CBIKE:
                    ulSizeOfVehicle = SIZEOF_CBIKE;
                    break;
                case VTBL_CBMX:
                    ulSizeOfVehicle = SIZEOF_CBMX;
                    break;
                case VTBL_CMONSTERTRUCK:
                    ulSizeOfVehicle = SIZEOF_CMONSTERTRUCK;
                    break;
                case VTBL_CQUADBIKE:
                    ulSizeOfVehicle = SIZEOF_CQUADBIKE;
                    break;
                case VTBL_CPLANE:
                    ulSizeOfVehicle = SIZEOF_CPLANE;
                    break;
                case VTBL_CTRAILER:
                    ulSizeOfVehicle = SIZEOF_CTRAILER;
                    break;
                case VTBL_CAUTOMOBILE:
                    ulSizeOfVehicle = SIZEOF_CAUTOMOBILE;
                    break;
                default:
                    ulSizeOfVehicle = 0;
                }

            if ( ulSizeOfVehicle > 0 &&
                    ulOffset >= ulVehicleBaseAddress &&
                    ulOffset - ulVehicleBaseAddress <= ulSizeOfVehicle )
            {
                // its within this vehicle
                pAddressInfo->uiType = MTA_OFFSET_VEHICLE;
                pAddressInfo->ulOffset = ulOffset - ulVehicleBaseAddress;
                return pAddressInfo;
            }
        }
    }

    // No matches
    pAddressInfo->uiType = MTA_OFFSET_BASE;
    pAddressInfo->ulOffset = ulOffset;
    return pAddressInfo;
}

#endif


//
// Display a progress dialog if a big packet is coming in
//
void CClientGame::NotifyBigPacketProgress ( unsigned long ulBytesReceived, unsigned long ulTotalSize )
{
    // Should display progress box?
    if ( ulBytesReceived >= ulTotalSize || ulTotalSize < 50000 )
    {
        if ( m_bReceivingBigPacket )
        {
            // Switch off progress box
            m_bReceivingBigPacket = false;
            m_pBigPacketTransferBox->Hide ();
        }
        return;
    }

    // Update progress box
    if ( !m_bReceivingBigPacket || m_ulBigPacketSize != ulTotalSize )
    {
        m_bReceivingBigPacket = true;
        m_ulBigPacketSize = ulTotalSize;
        m_pBigPacketTransferBox->Hide ();
        m_pBigPacketTransferBox->AddToTotalSize ( ulTotalSize );
        m_pBigPacketTransferBox->Show ();
    }

    m_pBigPacketTransferBox->DoPulse ();
    m_pBigPacketTransferBox->SetInfo ( Min ( ulTotalSize, ulBytesReceived ) );
}

bool CClientGame::SetGlitchEnabled ( unsigned char ucGlitch, bool bEnabled )
{
    if ( ucGlitch < NUM_GLITCHES && bEnabled != m_Glitches[ucGlitch] )
    {
        m_Glitches[ucGlitch] = bEnabled;
        if ( ucGlitch == GLITCH_QUICKRELOAD )
            g_pMultiplayer->DisableQuickReload ( !bEnabled );
        return true;
    }
    return false;
}

bool CClientGame::IsGlitchEnabled ( unsigned char ucGlitch )
{
    return ucGlitch < NUM_GLITCHES && m_Glitches[ucGlitch];
}

bool CClientGame::SetCloudsEnabled ( bool bEnabled )
{
   m_bCloudsEnabled = bEnabled;
   return true;
}
bool CClientGame::GetCloudsEnabled ( void )
{
    return m_bCloudsEnabled;
}

#pragma code_seg(".text")
bool CClientGame::VerifySADataFiles ( int iEnableClientChecks )
{
    int& iCheckStatus = g_pGame->GetCheckStatus ();

    if ( !g_pGame->VerifySADataFileNames () )
        iCheckStatus |= ( 1 << 11 );

    __declspec(allocate(".text")) static char szVerifyData[][32] = {
        "data/carmods.dat",     "\x6c\xbe\x84\x53\x61\xe7\x6a\xae\x35\xdd\xca\x30\x08\x67\xca\xdf",
        "data/handling.cfg",    "\x68\x68\xac\xce\xf9\x33\xf1\x85\x5e\xc2\x8c\xe1\x93\xa7\x81\x59",
        "data/melee.dat",       "\xb2\xf0\x56\x57\x98\x0e\x4a\x69\x3f\x8f\xf5\xea\xdc\xba\xd8\xf8",
        "data/object.dat",      "\x46\xa5\xe7\xdf\xf9\x00\x78\x84\x2e\x24\xd9\xde\x5e\x92\xcc\x3e",
        "data/surface.dat",     "\x9e\xb4\xe4\xe4\x74\xab\xd5\xda\x2f\x39\x61\xa5\xef\x54\x9f\x9e",
        "data/surfaud.dat",     "\xc3\x2c\x58\x6e\x8b\xa3\x57\x42\xe3\x56\xe6\x52\x56\x19\xf7\xc3",
        "data/surfinfo.dat",    "\x60\x5d\xd0\xbe\xab\xcc\xc7\x97\xce\x94\xa5\x1a\x3e\x4a\x09\xeb",
        "data/vehicles.ide",    "\xbd\xc3\xa0\xfc\xed\x24\x02\xc5\xbc\x61\x58\x57\x14\x45\x7d\x4b",
        "data/water.dat",       "\x69\x04\x00\xec\xc9\x21\x69\xd9\xea\xdd\xaa\xa9\x48\x90\x3e\xfb",
        "data/water1.dat",      "\x16\xfe\x5a\x3e\x8c\x57\xd0\x2e\xb6\x2a\x44\xa9\x6d\x8b\x9d\x39",
        "data/weapon.dat",      "\x0a\x9b\xb4\x90\x03\x68\x03\x64\xf9\xf9\x76\x8e\x9b\xce\xa9\x82",
        "anim/ped.ifp",         "\x47\x36\xB2\xC9\x0B\x00\x98\x12\x55\xF9\x50\x73\x08\xEE\x91\x74"
    };

    CMD5Hasher hasher;
    for ( int i = 0; i < NUMELMS ( szVerifyData ); i += 2 )
    {
        MD5 md5;
        if ( !hasher.Calculate ( szVerifyData[i], md5 ) ||
             memcmp ( md5, szVerifyData[i + 1], 0x10 ) )
        {
            iCheckStatus |= ( 1 << i );
        }
    }

    if ( iCheckStatus & iEnableClientChecks )
    {
        g_pCore->InitiateDataFilesFix ();
        g_pCore->GetModManager ()->RequestUnload ();
        return false;
    }

    return true;
}
