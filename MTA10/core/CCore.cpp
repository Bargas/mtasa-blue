/*****************************************************************************
*
*  PROJECT:		Multi Theft Auto v1.0
*  LICENSE:		See LICENSE in the top level directory
*  FILE:		core/CCore.cpp
*  PURPOSE:		Base core class
*  DEVELOPERS:	Cecill Etheredge <ijsf@gmx.net>
*               Chris McArthur <>
*               Christian Myhre Lundheim <>
*               Derek Abdine <>
*               Ed Lyons <eai@opencoding.net>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include <game/CGame.h>
#include <Accctrl.h>
#include <Aclapi.h>

static float fTest = 1;

extern CCore* g_pCore;
bool g_bBoundsChecker = false;
DWORD* g_Table = new DWORD[65535];
DWORD* g_TableSize = new DWORD[65535];
DWORD g_dwTable = 0;

BOOL AC_RestrictAccess( VOID )
{
    EXPLICIT_ACCESS NewAccess;
    PACL pTempDacl;
    HANDLE hProcess;
    DWORD dwFlags;
    DWORD dwErr;

    ///////////////////////////////////////////////
    // Get the HANDLE to the current process.
    hProcess = GetCurrentProcess();

    ///////////////////////////////////////////////
    // Setup which accesses we want to deny.
    dwFlags = GENERIC_WRITE|PROCESS_ALL_ACCESS|WRITE_DAC|DELETE|WRITE_OWNER|READ_CONTROL;

    ///////////////////////////////////////////////
    // Build our EXPLICIT_ACCESS structure.
    BuildExplicitAccessWithName( &NewAccess, "CURRENT_USER", dwFlags, DENY_ACCESS, NO_INHERITANCE ); 


    ///////////////////////////////////////////////
    // Create our Discretionary Access Control List.
    if ( ERROR_SUCCESS != (dwErr = SetEntriesInAcl( 1, &NewAccess, NULL, &pTempDacl )) )
    {
        #ifdef DEBUG
//        pConsole->Con_Printf("Error at SetEntriesInAcl(): %i", dwErr);
        #endif
        return FALSE;
    }

    ////////////////////////////////////////////////
    // Set the new DACL to our current process.
    if ( ERROR_SUCCESS != (dwErr = SetSecurityInfo( hProcess, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pTempDacl, NULL )) )
    {
        #ifdef DEBUG
//        pConsole->Con_Printf("Error at SetSecurityInfo(): %i", dwErr);
        #endif
        return FALSE;
    }

    ////////////////////////////////////////////////
    // Free the DACL (see msdn on SetEntriesInAcl)
    LocalFree( pTempDacl );
    CloseHandle( hProcess );

    return TRUE;
}


template<> CCore * CSingleton< CCore >::m_pSingleton = NULL;

CCore::CCore ( void )
{
    // Initialize the global pointer
    g_pCore = this;

	#if !defined(MTA_DEBUG) && !defined(MTA_ALLOW_DEBUG)
    AC_RestrictAccess ();
    #endif
    
    // Initialize critical sections
    CCriticalSection::Initialize ();

    // NULL the path buffers
    memset ( m_szInstallRoot, 0, MAX_PATH );
    memset ( m_szGTAInstallRoot, 0, MAX_PATH );

    // Open the MTA registry key
    HKEY hkey = NULL;
    DWORD dwBufferSize = MAX_PATH;
    DWORD dwType = 0;
    if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, "Software\\Multi Theft Auto: San Andreas", 0, KEY_READ, &hkey ) == ERROR_SUCCESS ) 
    {
        // Read out the MTA installpath
        if ( RegQueryValueEx ( hkey, "", NULL, &dwType, (LPBYTE)m_szInstallRoot, &dwBufferSize ) != ERROR_SUCCESS ||
             strlen ( m_szInstallRoot ) == 0 )
        {
            MessageBox ( 0, "Multi Theft Auto has not been installed properly, please reinstall.", "Error", MB_OK );
            RegCloseKey ( hkey );
            TerminateProcess ( GetCurrentProcess (), 9 );
        }

        // Read out the GTA installpath
        dwBufferSize = MAX_PATH;
        dwType = 0;
        if ( RegQueryValueEx ( hkey, "GTA:SA Path", NULL, &dwType, (LPBYTE)m_szGTAInstallRoot, &dwBufferSize ) != ERROR_SUCCESS ||
             strlen ( m_szGTAInstallRoot ) == 0 )
        {
            MessageBox ( 0, "There is no GTA path specified in the registry, please reinstall.", "Error", MB_OK );
            RegCloseKey ( hkey );
            TerminateProcess ( GetCurrentProcess (), 9 );
        }

        RegCloseKey ( hkey );
    }


    // Remove the trailing / from the installroot incase it has
    size_t sizeInstallRoot = strlen ( m_szInstallRoot );
    if ( m_szInstallRoot [ sizeInstallRoot - 1 ] == '/' ||
         m_szInstallRoot [ sizeInstallRoot - 1 ] == '\\' )
    {
        m_szInstallRoot [ sizeInstallRoot - 1 ] = 0;
    }

    // Remove the trailing / from the installroot incase it has
    size_t sizeGTAInstallRoot = strlen ( m_szGTAInstallRoot );
    if ( m_szGTAInstallRoot [ sizeGTAInstallRoot - 1 ] == '/' ||
         m_szGTAInstallRoot [ sizeGTAInstallRoot - 1 ] == '\\' )
    {
        m_szGTAInstallRoot [ sizeGTAInstallRoot - 1 ] = 0;
    }


    // Create a logger instance.
    m_pLogger                   = new CLogger ( );

    // Create interaction objects.
    m_pCommands                 = new CCommands;
    m_pConnectManager           = new CConnectManager;

    // Create the GUI manager and the graphics lib wrapper
    m_pLocalGUI                 = new CLocalGUI;
    m_pGraphics                 = new CGraphics ( m_pLocalGUI );

	// Create the mod manager
    m_pModManager               = new CModManager;

    // Create a resolution manager (for screen res)
    m_pResManager               = new CResManager;

    m_pfnMessageProcessor       = NULL;
    m_pMessageBox = NULL;

	m_bResetNeeded = false;
    m_bFirstFrame = true;
    m_bIsOfflineMod = false;
    m_bQuitOnPulse = false;
    m_bDestroyMessageBox = false;

    // Initialize time
    CClientTime::InitializeTime ();

    // Create our Direct3DData handler.
    m_pDirect3DData = new CDirect3DData;

    WriteDebugEvent ( "CCore::CCore" );

    m_pKeyBinds = new CKeyBinds ( this );

    // Create our hook objects.
    //m_pFileSystemHook           = new CFileSystemHook ( );
    m_pDirect3DHookManager      = new CDirect3DHookManager ( );
    m_pDirectInputHookManager   = new CDirectInputHookManager ( );
    m_pMessageLoopHook          = new CMessageLoopHook ( );
    m_pSetCursorPosHook         = new CSetCursorPosHook ( );
	m_pTCPManager				= new CTCPManager ( );

    // Register internal commands.
    RegisterCommands ( );

    // Setup our hooks.
    ApplyHooks ( );

	// Create the VMR9 manager
	m_pVMR9Manager = new CVideoManager ();

	// Reset the screenshot flag
	bScreenShot = false;

    m_ChatFont = CHAT_FONT_CLEAR;             
}

CCore::~CCore ( void )
{
    WriteDebugEvent ( "CCore::~CCore" );

    // Delete the mod manager
    delete m_pModManager;
    SAFE_DELETE ( m_pMessageBox );

    // Destroy early subsystems
    DestroyNetwork ();
    DestroyMultiplayer ();
    DestroyGame ();

	// Remove global events
    g_pCore->GetGUI ()->SetMouseClickHandler ( );
	g_pCore->GetGUI ()->SetMouseDoubleClickHandler ( );

    // Remove input hook
	CMessageLoopHook::GetSingleton ( ).RemoveHook ( );

    // Store core variables to cvars
    CVARS_SET ( "console_pos",                  m_pLocalGUI->GetConsole ()->GetPosition () );
    CVARS_SET ( "console_size",                 m_pLocalGUI->GetConsole ()->GetSize () );
    CVARS_SET ( "chat_font",                    m_ChatFont );
    
    CColor color;
    CChat *pChat = m_pLocalGUI->GetChat ();
    CVector2D vec;
    CVARS_SET ( "chat_lines",                   pChat->GetNumLines () );
    pChat->GetColor ( color );
    CVARS_SET ( "chat_color",                   color );
    pChat->GetInputColor ( color );
    CVARS_SET ( "chat_input_color",             color );
    pChat->GetInputPrefixColor ( color );
    CVARS_SET ( "chat_input_prefix_color",      color );
    pChat->GetScale ( vec );
    CVARS_SET ( "chat_scale",                   vec );
    CVARS_SET ( "chat_width",                   pChat->GetWidth () );
    CVARS_SET ( "chat_css_style_text",          pChat->GetCssStyleText () );
    CVARS_SET ( "chat_css_style_background",    pChat->GetCssStyleBackground () );
    CVARS_SET ( "chat_line_life",               (unsigned int)pChat->GetChatLineLife () );
    CVARS_SET ( "chat_line_fade_out",           (unsigned int)pChat->GetChatLineFadeOut () );
    CVARS_SET ( "chat_use_cegui",               pChat->GetUseCEGUI () );
    // Save cvars
    m_ClientVariables.Save ();

    delete m_pKeyBinds;

    // Delete interaction objects.
    delete m_pCommands;
    delete m_pConnectManager;
    delete m_pDirect3DData;

    // Delete hooks.
    delete m_pSetCursorPosHook;
    //delete m_pFileSystemHook;
    delete m_pDirect3DHookManager;
    delete m_pDirectInputHookManager;
    delete m_pMessageLoopHook;
	delete m_pTCPManager;

    // Delete the GUI manager    
    delete m_pLocalGUI;
    delete m_pGraphics;

	// Delete the VMR9 manager
	delete m_pVMR9Manager;

    // Delete lazy subsystems
    DestroyGUI ();
    DestroyXML ();

    // Delete the logger
    delete m_pLogger;

    // Finalize critical sections
    CCriticalSection::Finalize ();
}


eCoreVersion CCore::GetVersion ( void )
{
    return MTACORE_20;
}


CConsoleInterface* CCore::GetConsole ( void )
{
    return m_pLocalGUI->GetConsole ();
}


CCommandsInterface* CCore::GetCommands ( void )
{
    return m_pCommands;
}


CGame* CCore::GetGame ( void )
{
    return m_pGame;
}


CGraphicsInterface* CCore::GetGraphics ( void )
{
    return m_pGraphics;
}


CModManagerInterface* CCore::GetModManager ( void )
{
    return m_pModManager;
}


CMultiplayer* CCore::GetMultiplayer ( void )
{
    return m_pMultiplayer;
}


CGUI* CCore::GetGUI ( void )
{
	return m_pGUI;
}


CNet* CCore::GetNetwork ( void )
{
    return m_pNet;
}


CKeyBindsInterface* CCore::GetKeyBinds ( void )
{
    return m_pKeyBinds;
}


CLocalGUI* CCore::GetLocalGUI ( void )
{
    return m_pLocalGUI;
}


CVideoManager* CCore::GetVMR9Manager ( void )
{
	return m_pVMR9Manager;
}


void CCore::ChatEcho ( const char* szText, bool bColorCoded )
{
    CChat* pChat = m_pLocalGUI->GetChat ();
    if ( pChat )
    {
        pChat->SetTextColor ( CColor ( 255, 255, 255, 255 ) );
    }

    // Echo it to the console and chat
    m_pLocalGUI->EchoChat ( szText, bColorCoded );
    if ( bColorCoded )
    {
        char szTemp [ 512 ];
        CChatLine::RemoveColorCode ( const_cast < char* > ( szText ), szTemp, 512 );
        m_pLocalGUI->EchoConsole ( szTemp );
    }
    else
        m_pLocalGUI->EchoConsole ( szText );
}


void CCore::DebugEcho ( const char* szText )
{
    CDebugView * pDebugView = m_pLocalGUI->GetDebugView ();
    if ( pDebugView )
    {
        pDebugView->SetTextColor ( CColor ( 255, 255, 255, 255 ) );
    }

    m_pLocalGUI->EchoDebug ( szText );
}

void CCore::DebugPrintf ( const char* szFormat, ... )
{
    // Convert it to a string buffer
    char szBuffer [1024];
	va_list ap;
	va_start ( ap, szFormat );
	_vsnprintf ( szBuffer, 1024, szFormat, ap );
	va_end ( ap );

    DebugEcho ( szBuffer );
}

void CCore::SetDebugVisible ( bool bVisible )
{
    if ( m_pLocalGUI )
    {
        m_pLocalGUI->SetDebugViewVisible ( bVisible );
    }
}


bool CCore::IsDebugVisible ( void )
{
    if ( m_pLocalGUI )
        return m_pLocalGUI->IsDebugViewVisible ();
    else
        return false;
}


void CCore::DebugEchoColor ( const char* szText, unsigned char R, unsigned char G, unsigned char B )
{
    // Set the color
    CDebugView * pDebugView = m_pLocalGUI->GetDebugView ();
    if ( pDebugView )
    {
        pDebugView->SetTextColor ( CColor ( R, G, B, 255 ) );
    }

    m_pLocalGUI->EchoDebug ( szText );
}


void CCore::DebugPrintfColor ( const char* szFormat, unsigned char R, unsigned char G, unsigned char B, ... )
{
    // Set the color
    if ( szFormat )
    {
        // Convert it to a string buffer
        char szBuffer [1024];
	    va_list ap;
	    va_start ( ap, B );
	    _vsnprintf ( szBuffer, 1024, szFormat, ap );
	    va_end ( ap );

        // Echo it to the console and chat
        DebugEchoColor ( szBuffer, R, G, B );
    }
}


void CCore::DebugClear ( void )
{
    CDebugView * pDebugView = m_pLocalGUI->GetDebugView ();
    if ( pDebugView )
    {
        pDebugView->Clear();
    }
}


void CCore::ChatEchoColor ( const char* szText, unsigned char R, unsigned char G, unsigned char B, bool bColorCoded )
{
    // Set the color
    CChat* pChat = m_pLocalGUI->GetChat ();
    if ( pChat )
    {
        pChat->SetTextColor ( CColor ( R, G, B, 255 ) );
    }

    // Echo it to the console and chat
    m_pLocalGUI->EchoChat ( szText, bColorCoded );
    if ( bColorCoded )
    {
        char szTemp [ 512 ];
        CChatLine::RemoveColorCode ( const_cast < char* > ( szText ), szTemp, 512 );
        m_pLocalGUI->EchoConsole ( szTemp );
    }
    else
        m_pLocalGUI->EchoConsole ( szText );
}


void CCore::ChatPrintf ( const char* szFormat, bool bColorCoded, ... )
{
    // Convert it to a string buffer
    char szBuffer [1024];
	va_list ap;
	va_start ( ap, bColorCoded );
	_vsnprintf ( szBuffer, 1024, szFormat, ap );
	va_end ( ap );

    // Echo it to the console and chat
    ChatEcho ( szBuffer, bColorCoded );
}


void CCore::ChatPrintfColor ( const char* szFormat, bool bColorCoded, unsigned char R, unsigned char G, unsigned char B, ... )
{
    // Set the color
    if ( szFormat )
    {
        if ( m_pLocalGUI )
        {
            // Convert it to a string buffer
            char szBuffer [1024];
	        va_list ap;
	        va_start ( ap, B );
	        _vsnprintf ( szBuffer, 1024, szFormat, ap );
	        va_end ( ap );

            // Echo it to the console and chat
            ChatEchoColor ( szBuffer, R, G, B, bColorCoded );
        }
    }
}


void CCore::SetChatVisible ( bool bVisible )
{
    if ( m_pLocalGUI )
    {
        m_pLocalGUI->SetChatBoxVisible ( bVisible );
    }
}


bool CCore::IsChatVisible ( void )
{
    if ( m_pLocalGUI )
    {
        return m_pLocalGUI->IsChatBoxVisible ();
    }
    return false;
}


void CCore::TakeScreenShot ( void )
{
	bScreenShot = true;
}


void CCore::EnableChatInput ( char* szCommand, DWORD dwColor )
{
    if ( m_pLocalGUI )
    {
        if ( m_pGame->GetSystemState () == 9 /* GS_FRONTEND */ &&
            m_pModManager->GetCurrentMod () != NULL &&
            !IsOfflineMod () &&
            !m_pGame->IsAtMenu () &&
            !m_pLocalGUI->GetMainMenu ()->IsVisible () &&
            !m_pLocalGUI->GetConsole ()->IsVisible () &&
            !m_pLocalGUI->IsChatBoxInputEnabled () )
        {
            CChat* pChat = m_pLocalGUI->GetChat ();
            pChat->SetCommand ( szCommand );
            //pChat->SetInputColor ( dwColor );
            m_pLocalGUI->SetChatBoxInputEnabled ( true );
            m_pLocalGUI->SetVisibleWindows ( true );
        }
    }
}


bool CCore::IsChatInputEnabled ( void )
{
    if ( m_pLocalGUI )
    {
        return ( m_pLocalGUI->IsChatBoxInputEnabled () );
    }

    return false;
}


bool CCore::IsSettingsVisible ( void )
{
    if ( m_pLocalGUI )
    {
        return ( m_pLocalGUI->GetMainMenu ()->GetSettingsWindow ()->IsVisible () );
    }
    
    return false;
}


bool CCore::IsMenuVisible ( void )
{
    if ( m_pLocalGUI )
    {
        return ( m_pLocalGUI->GetMainMenu ()->IsVisible () );
    }

    return false;
}


bool CCore::IsCursorForcedVisible ( void )
{
    if ( m_pLocalGUI )
    {
        return ( m_pLocalGUI->IsCursorForcedVisible () );
    }

    return false;
}

/*
void CCore::SetChatFont ( eChatFont font )
{
    if ( font != m_ChatFont )
    {
        CGUIFont* pFont = m_pGUI->GetDefaultFont ();
        ID3DXFont* pDXFont = m_pGraphics->GetFont ();
        switch ( font )
        {
            case CHAT_FONT_DEFAULT:
                pFont = m_pGUI->GetDefaultFont ();
				pDXFont = m_pGraphics->GetFont ( FONT_DEFAULT );
                break;
            case CHAT_FONT_CLEAR:
                pFont = m_pGUI->GetClearFont ();
				pDXFont = m_pGraphics->GetFont ( FONT_CLEAR );
                break;
            case CHAT_FONT_BOLD:
                pFont = m_pGUI->GetBoldFont ();
                pDXFont = m_pGraphics->GetFont ( FONT_DEFAULT_BOLD );
                break;
            case CHAT_FONT_ARIAL:
				pDXFont = m_pGraphics->GetFont ( FONT_ARIAL );
                break;                
        }
        m_pLocalGUI->GetChat ()->SetFont ( pFont );
        m_pLocalGUI->GetChat ()->SetDXFont ( pDXFont );        

        m_ChatFont = font;
    }
}
*/


void CCore::ApplyConsoleSettings ( void )
{
    CVector2D vec;
    CConsole * pConsole = m_pLocalGUI->GetConsole ();

    CVARS_GET ( "console_pos", vec );
    pConsole->SetPosition ( vec );
    CVARS_GET ( "console_size", vec );
    pConsole->SetSize ( vec );
}


void CCore::ApplyChatSettings ( void )
{
    CColor col;
    CVector2D vec;
    float fval;
    bool bval;
    unsigned int uval;
    CChat *pChat = m_pLocalGUI->GetChat ();

    CVARS_GET ( "chat_color",                   col );  pChat->SetColor ( col );
    CVARS_GET ( "chat_input_color",             col );  pChat->SetInputColor ( col );
    CVARS_GET ( "chat_input_prefix_color",      col );  pChat->SetInputPrefixColor ( col );
    CVARS_GET ( "chat_input_text_color",        col );  pChat->SetInputTextColor ( col );
    CVARS_GET ( "chat_scale",                   vec );  pChat->SetScale ( vec );
    CVARS_GET ( "chat_width",                   fval ); pChat->SetWidth ( fval );
    CVARS_GET ( "chat_css_style_text",          bval ); pChat->SetCssStyleText ( bval );
    CVARS_GET ( "chat_css_style_background",    bval ); pChat->SetCssStyleBackground ( bval );
    CVARS_GET ( "chat_line_life",               uval ); pChat->SetChatLineLife ( uval );
    CVARS_GET ( "chat_line_fade_out",           uval ); pChat->SetChatLineFadeOut ( uval );
    CVARS_GET ( "chat_use_cegui",               bval ); pChat->SetUseCEGUI ( bval );
}


void CCore::ApplyGameSettings ( void )
{
    bool bval;
    CControllerConfigManager * pController = m_pGame->GetControllerConfigManager ();

    CVARS_GET ( "invert_mouse",     bval ); pController->SetMouseInverted ( bval );
    CVARS_GET ( "fly_with_mouse",   bval ); pController->SetFlyWithMouse ( bval );
}


void CCore::SetConnected ( bool bConnected )
{
    m_pLocalGUI->GetMainMenu ( )->SetIsIngame ( bConnected );
}


bool CCore::IsConnected ( void )
{
    return m_pLocalGUI->GetMainMenu ( )->GetIsIngame ();
}


void CCore::SetOfflineMod ( bool bOffline )
{
    m_bIsOfflineMod = bOffline;
}


char * CCore::GetModInstallRoot ( char * szModName, char * szBuffer, size_t bufferSize )
{
    _snprintf( szBuffer, bufferSize, "%s\\mods\\%s", GetInstallRoot(), szModName );
    return szBuffer;
}


const char* CCore::GetInstallRoot ( )
{
    return m_szInstallRoot;
}


const char* CCore::GetGTAInstallRoot ( void )
{
    return m_szGTAInstallRoot;
}


void CCore::ForceCursorVisible ( bool bVisible )
{
    m_pLocalGUI->ForceCursorVisible ( bVisible );
}


void CCore::SetMessageProcessor ( pfnProcessMessage pfnMessageProcessor )
{
    m_pfnMessageProcessor = pfnMessageProcessor;
}


void CCore::ShowMessageBox ( const char* szTitle, const char* szText, unsigned int uiFlags, GUI_CALLBACK * ResponseHandler )
{
	CFilePathTranslator		FileTranslator;
    string                  WorkingDirectory;
    char                    szCurDir [ 1024 ];

	if ( m_pMessageBox )
        delete m_pMessageBox;


    // Set the current directory to the MTA dir so we can load files using a relative path
    FileTranslator.SetCurrentWorkingDirectory ( "MTA" );
    FileTranslator.GetCurrentWorkingDirectory ( WorkingDirectory );
    GetCurrentDirectory ( sizeof ( szCurDir ), szCurDir );
    SetCurrentDirectory ( WorkingDirectory.c_str ( ) );

	// Create the message box
	m_pMessageBox = m_pGUI->CreateMessageBox ( szTitle, szText, uiFlags );
	if ( ResponseHandler ) m_pMessageBox->SetOnClickHandler ( *ResponseHandler );

    // Restore current directory
    SetCurrentDirectory ( szCurDir );

	// Make sure it doesn't auto-destroy, or we'll crash if the msgbox had buttons and the user clicks OK
    m_pMessageBox->SetAutoDestroy ( false );
}


void CCore::RemoveMessageBox ( bool bNextFrame )
{
    if ( bNextFrame )
    {
        m_bDestroyMessageBox = true;
    }
    else
    {
        if ( m_pMessageBox )
        {
            delete m_pMessageBox;
            m_pMessageBox = NULL;
        }
    }
}


HWND CCore::GetHookedWindow ( void )
{
    return CMessageLoopHook::GetSingleton ().GetHookedWindowHandle ();
}


void CCore::HideMainMenu ( void )
{
	m_pLocalGUI->GetMainMenu ()->SetVisible ( false );
    m_pLocalGUI->GetMainMenu ()->SetServerBrowserVisible ( false );
}


void CCore::ApplyHooks ( )
{ 
    // Create our hooks.
    m_pDirectInputHookManager->ApplyHook ( );
    m_pDirect3DHookManager->ApplyHook ( );
    //m_pFileSystemHook->ApplyHook ( );
    m_pSetCursorPosHook->ApplyHook ( );

    // Redirect basic files.
    //m_pFileSystemHook->RedirectFile ( "main.scm", "../../mta/gtafiles/main.scm" );
}


void CCore::SetCenterCursor ( bool bEnabled )
{
	if ( bEnabled )
		m_pSetCursorPosHook->EnableSetCursorPos ();
	else
		m_pSetCursorPosHook->DisableSetCursorPos ();
}

void CCore::CreateGame ( )
{
    // Create function pointer type and variable.
    typedef CGame* (*pfnGameInitializer) ( );
    pfnGameInitializer pfnGameInit;

    // Load approrpiate compilation-specific library.
#ifdef MTA_DEBUG
    m_GameModule.LoadModule ( "mta/game_sa_d.dll" );
# else
    m_GameModule.LoadModule ( "mta/game_sa.dll" );

#endif


    // Get client initializer function from DLL's routine.
    pfnGameInit = static_cast< pfnGameInitializer > 
    ( m_GameModule.GetFunctionPointer ( "GetGameInterface" ) );

    // If we have a valid initializer, call it.
    if ( pfnGameInit != NULL )
    {
        WriteDebugEvent ( "Game loaded." );
        m_pGame = pfnGameInit ( );

        if ( m_pGame->GetGameVersion () == VERSION_UNKNOWN )
        {
            MessageBox ( 0, "Only GTA:SA version 1.0 is supported!", "Error", MB_OK|MB_ICONEXCLAMATION );
            TerminateProcess ( GetCurrentProcess (), 0 );
        }
    }
    else
    {
        // USE CLANGUAGELOCALE HERE.
        MessageBox ( 0, "Game module could not be located!", "Error", MB_OK|MB_ICONEXCLAMATION );
        TerminateProcess ( GetCurrentProcess (), 0 );
    }
}


void CCore::CreateMultiplayer ( )
{
    // Check to see if our game has been created.
    if ( m_pGame == NULL )
    {
        // Inform user that loading failed.
        return;
    }

    // Create function pointer type and variable.
    typedef CMultiplayer* (*pfnMultiplayerInitializer) ( CGame * );
    pfnMultiplayerInitializer pfnMultiplayerInit;

    // Load approrpiate compilation-specific library.
#ifdef MTA_DEBUG
    m_MultiplayerModule.LoadModule ( "mta/multiplayer_sa_d.dll" );
# else
    m_MultiplayerModule.LoadModule ( "mta/multiplayer_sa.dll" );
#endif

    // Get client initializer function from DLL's routine.
    pfnMultiplayerInit = static_cast< pfnMultiplayerInitializer > 
    ( m_MultiplayerModule.GetFunctionPointer ( "InitMultiplayerInterface" ) );

    // If we have a valid initializer, call it.
    if ( pfnMultiplayerInit != NULL )
    {
        WriteDebugEvent ( "Multiplayer loaded." );
        m_pMultiplayer = pfnMultiplayerInit ( m_pGame );
    }
    else
    {
        // USE CLANGUAGELOCALE HERE.
        MessageBox ( 0, "Multiplayer module could not be located!", "Error", MB_OK|MB_ICONEXCLAMATION );
        TerminateProcess ( GetCurrentProcess (), 0 );
    }
}


void CCore::DeinitGUI ( void )
{

}


void CCore::InitGUI ( IUnknown* pDevice )
{
	// Initializes the GUI by calling the InitGUIInterface-function in the GUI dll

    CFilePathTranslator     FileTranslator;
    string                  WorkingDirectory;
    char                    szCurDir [ 1024 ];
	bool					bReturn = false;

    // Set the current directory.
    FileTranslator.SetCurrentWorkingDirectory ( "MTA" );
    FileTranslator.GetCurrentWorkingDirectory ( WorkingDirectory );
    GetCurrentDirectory ( sizeof ( szCurDir ), szCurDir );
    SetCurrentDirectory ( WorkingDirectory.c_str ( ) );

	typedef CGUI* (*pfnGUIInitializer) ( IDirect3DDevice9* );
	pfnGUIInitializer pfnGUIInit;

	pfnGUIInit = static_cast < pfnGUIInitializer > ( m_GUIModule.GetFunctionPointer ( "InitGUIInterface" ) );
	if ( pfnGUIInit != NULL )
	{
		IDirect3DDevice9 *dev = reinterpret_cast < IDirect3DDevice9* > ( pDevice );
		m_pGUI = pfnGUIInit ( dev );
		WriteDebugEvent ( "GUI initialized." );
	} else {
		// USE CLANGUAGELOCALE HERE.
		MessageBox ( 0, "GUI module could not be initialized!", "Error", MB_OK|MB_ICONEXCLAMATION );
		TerminateProcess ( GetCurrentProcess (), 0 );
	}

	// Set the working directory for CGUI
	m_pGUI->SetWorkingDirectory ( WorkingDirectory.c_str ( ) );

	SetCurrentDirectory ( szCurDir );

	// and set the screenshot path to this default library (screenshots shouldnt really be made outside mods)
    std::string strScreenShotPath = GetInstallRoot () + std::string ( "\\screenshots" );
    CVARS_SET ( "screenshot_path", strScreenShotPath );
}


void CCore::CreateGUI ( void )
{
	// Should only be called once, use InitGUI to init the GUI, which can be called again (after a destruction of the GUI)

    CFilePathTranslator     FileTranslator;
    string                  WorkingDirectory;
    char                    szCurDir [ 1024 ];
	bool					bReturn = false;

    // Set the current directory.
    FileTranslator.SetCurrentWorkingDirectory ( "MTA" );
    FileTranslator.GetCurrentWorkingDirectory ( WorkingDirectory );
    GetCurrentDirectory ( sizeof ( szCurDir ), szCurDir );
    SetCurrentDirectory ( WorkingDirectory.c_str ( ) );

#ifdef MTA_DEBUG
	bReturn = m_GUIModule.LoadModule ( "cgui_d.dll" );
#else
	bReturn = m_GUIModule.LoadModule ( "cgui.dll" );
#endif
	if ( !bReturn )
	{
        // USE CLANGUAGELOCALE HERE.
        MessageBox ( 0, "GUI module could not be located!", "Error", MB_OK|MB_ICONEXCLAMATION );
        TerminateProcess ( GetCurrentProcess (), 0 );
    }
	WriteDebugEvent ( "GUI loaded." );

	SetCurrentDirectory ( szCurDir );
}

void CCore::DestroyGUI ( )
{
	WriteDebugEvent ( "CCore::DestroyGUI" );
	if ( m_pGUI )
	{
        m_pGUI = NULL;
	}
	m_GUIModule.UnloadModule ();
}


void CCore::CreateNetwork ( )
{
    // Create function pointer type and variable.
    typedef CNet* (*pfnNetInitializer) ( );
    pfnNetInitializer pfnNetInit;

    CFilePathTranslator     FileTranslator;
    string                  WorkingDirectory;
    char                    szCurDir [ 1024 ];

    // Set the current directory.
    FileTranslator.SetCurrentWorkingDirectory ( "MTA" );
    FileTranslator.GetCurrentWorkingDirectory ( WorkingDirectory );
    GetCurrentDirectory ( sizeof ( szCurDir ), szCurDir );
    SetCurrentDirectory ( WorkingDirectory.c_str ( ) );

    // Load approrpiate compilation-specific library.
    m_NetModule.LoadModule ( "net.dll" );

    // Get client initializer function from DLL's routine.
    pfnNetInit = static_cast< pfnNetInitializer > 
    ( m_NetModule.GetFunctionPointer ( "InitNetInterface" ) );

    // If we have a valid initializer, call it.
    if ( pfnNetInit != NULL )
    {
        WriteDebugEvent ( "Network loaded." );
        m_pNet = pfnNetInit ( );
    }
    else
    {
        // USE CLANGUAGELOCALE HERE.
        MessageBox ( 0, "Network module could not be located!", "Error", MB_OK|MB_ICONEXCLAMATION );
        TerminateProcess ( GetCurrentProcess (), 0 );
    }

	SetCurrentDirectory ( szCurDir );
}


void CCore::CreateXML ( )
{
    // Create function pointer type and variable.
    typedef CXML* (*pfnXMLInitializer) ( );
    pfnXMLInitializer pfnXMLInit;

    CFilePathTranslator     FileTranslator;
    string                  WorkingDirectory;
    char                    szCurDir [ 1024 ];

    // Set the current directory.
    FileTranslator.SetCurrentWorkingDirectory ( "MTA" );
    FileTranslator.GetCurrentWorkingDirectory ( WorkingDirectory );
    GetCurrentDirectory ( sizeof ( szCurDir ), szCurDir );
    SetCurrentDirectory ( WorkingDirectory.c_str ( ) );

    // Load approrpiate compilation-specific library.
#ifdef MTA_DEBUG
    m_XMLModule.LoadModule ( "xmll_d.dll" );
#else
    m_XMLModule.LoadModule ( "xmll.dll" );
#endif

    // Get client initializer function from DLL's routine.
    pfnXMLInit = static_cast< pfnXMLInitializer > 
    ( m_XMLModule.GetFunctionPointer ( "InitXMLInterface" ) );

    // If we have a valid initializer, call it.
    if ( pfnXMLInit != NULL )
    {
        WriteDebugEvent ( "XML loaded." );
        m_pXML = pfnXMLInit ( );
    }
    else
    {
        // USE CLANGUAGELOCALE HERE.
        MessageBox ( 0, "XML module could not be located!", "Error", MB_OK|MB_ICONEXCLAMATION );
        TerminateProcess ( GetCurrentProcess (), 0 );
    }

	SetCurrentDirectory ( szCurDir );

    // Load XML-dependant subsystems
    m_ClientVariables.Load ( MTA_CONFIG_PATH );
}


void CCore::DestroyGame ( )
{
	WriteDebugEvent ( "CCore::DestroyGame" );

	if ( m_pGame )
	{
        m_pGame->Terminate ();
        m_pGame = NULL;
	}

	m_GameModule.UnloadModule();

}


void CCore::DestroyMultiplayer ( )
{
	WriteDebugEvent ( "CCore::DestroyMultiplayer" );

	if ( m_pMultiplayer )
	{
        m_pMultiplayer = NULL;
	}

	m_MultiplayerModule.UnloadModule();
}


void CCore::DestroyXML ( )
{
	WriteDebugEvent ( "CCore::DestroyXML" );

	if ( m_pXML )
	{
        m_pXML = NULL;
	}

	m_XMLModule.UnloadModule();
}


void CCore::DestroyNetwork ( )
{
	WriteDebugEvent ( "CCore::DestroyNetwork" );

	if ( m_pNet )
	{
        m_pNet = NULL;
	}

	m_NetModule.UnloadModule();
}


void CCore::DoPreFramePulse ( )
{
    m_pKeyBinds->DoPreFramePulse ();

    // Notify the mod manager
    m_pModManager->DoPulsePreFrame ();  
}


void CCore::DoPostFramePulse ( )
{
    if ( m_bQuitOnPulse )
        Quit ();

    if ( m_bDestroyMessageBox )
    {
        RemoveMessageBox ();
        m_bDestroyMessageBox = false;
    }

    static bool bFirstPulse = true;
    if ( bFirstPulse )
    {
        bFirstPulse = false;

        // Apply all settings
        ApplyConsoleSettings ();
        ApplyChatSettings ();
        ApplyGameSettings ();

		m_pLocalGUI->GetMainMenu ()->LoadMenuOptions ();

        m_pGUI->SetMouseClickHandler ( GUI_CALLBACK_MOUSE ( &CCore::OnMouseClick, this ) );
		m_pGUI->SetMouseDoubleClickHandler ( GUI_CALLBACK_MOUSE ( &CCore::OnMouseDoubleClick, this ) );
    }

    // This is the first frame in the menu?
    if ( m_pGame->GetSystemState () == 7 ) // GS_FRONTEND
    {
        // Wait 250 frames more than the time it took to get status 7 (fade-out time)
        static short WaitForMenu = 0;
        if ( WaitForMenu >= 250 )
        {
            if ( m_bFirstFrame )
            {
                m_bFirstFrame = false;

                // Parse commandline (TODO: Pretty hacky... Need to manage this more nicely in r2)
                char szCommandLine [512];
                strncpy ( szCommandLine, GetCommandLine (), 512 );

                // Does it begin with mta://?
                if ( strnicmp ( szCommandLine, "mtasa://", 8 ) == 0 )
                {
                    char szArguments [256];
                    szArguments [255] = 0;

                    GetConnectCommandFromURI(szCommandLine, szArguments, sizeof(szArguments));
                    // Run the connect command
                    if ( strlen( szArguments ) > 0 && !m_pCommands->Execute ( szArguments ) )
                    {
                        ShowMessageBox ( "Error", "Error executing URL", MB_BUTTON_OK | MB_ICON_ERROR );
                    }
                }
                else
                {   
                    char* szKey = strtok ( szCommandLine, " " );
                    if ( szKey )
                    {
                        // We want to load a mod?
                        if ( strcmp ( szKey, "-l" ) == 0 )
                        {
                            char* szMod = strtok ( NULL, " " );
                            char* szArguments = strtok ( NULL, "\0" );
                            
                            // Try to load the mod
                            if ( !m_pModManager->Load ( szMod, szArguments ) )
                            {
                                char szTemp [128];
                                _snprintf ( szTemp, 128, "Error running mod specified in command line ('%s')", szMod );
                                ShowMessageBox ( "Error", szTemp, MB_BUTTON_OK | MB_ICON_ERROR );
                            }
                        }
                        // We want to connect to a server?
                        else if ( strcmp ( szKey, "-c" ) == 0 )
                        {
                            char* szArguments = strtok ( NULL, "\0" );
                            
                            CCommandFuncs::Connect ( szArguments );
                        }
                    }                                                    
                }
            }
        }
        else
        {
            WaitForMenu++;
        }
    }

    m_pKeyBinds->DoPostFramePulse ();

    // Notify the mod manager and the connect manager
    m_pModManager->DoPulsePostFrame ();
    m_pConnectManager->DoPulse ();

	m_Community.DoPulse ();
}


// Called after MOD is unloaded
void CCore::OnModUnload ( )
{
	// reattach the global event
    m_pGUI->SetMouseClickHandler ( GUI_CALLBACK_MOUSE ( &CCore::OnMouseClick, this ) );
	m_pGUI->SetMouseDoubleClickHandler ( GUI_CALLBACK_MOUSE ( &CCore::OnMouseDoubleClick, this ) );
}


void CCore::RegisterCommands ( )
{
	//m_pCommands->Add ( "e", CCommandFuncs::Editor );
    //m_pCommands->Add ( "clear", CCommandFuncs::Clear );
    m_pCommands->Add ( "help",				"this help screen",					CCommandFuncs::Help );
    m_pCommands->Add ( "exit",				"exits the application",			CCommandFuncs::Exit );
    m_pCommands->Add ( "quit",				"exits the application",			CCommandFuncs::Exit );
	m_pCommands->Add ( "ver",				"shows the version",				CCommandFuncs::Ver );
    m_pCommands->Add ( "time",				"shows the time",					CCommandFuncs::Time );
	m_pCommands->Add ( "hud",				"shows the hud",					CCommandFuncs::HUD );
    m_pCommands->Add ( "binds",				"shows all the binds",				CCommandFuncs::Binds );

    m_pCommands->Add ( "vid",				"changes the video settings (id)",	CCommandFuncs::Vid );

    m_pCommands->Add ( "window",            "enter windowed mode",	            CCommandFuncs::Window );

    m_pCommands->Add ( "load",				"loads a mod (name args)",			CCommandFuncs::Load );
    m_pCommands->Add ( "unload",			"unloads a mod (name)",				CCommandFuncs::Unload );
    m_pCommands->Add ( "connect",			"connects to a server (host port nick pass)",	CCommandFuncs::Connect );
    m_pCommands->Add ( "reconnect",			"connects to a previous server",	CCommandFuncs::Reconnect );
    m_pCommands->Add ( "bind",				"binds a key (key control)",		CCommandFuncs::Bind );
    m_pCommands->Add ( "unbind",			"unbinds a key (key)",				CCommandFuncs::Unbind );
    m_pCommands->Add ( "copygtacontrols",	"copies the default gta controls",	CCommandFuncs::CopyGTAControls );
    m_pCommands->Add ( "cleardebug",		"clears the debug view",			CCommandFuncs::ClearDebug );
	m_pCommands->Add ( "screenshot",		"outputs a screenshot",				CCommandFuncs::ScreenShot );
    m_pCommands->Add ( "connectiontype",	"sets the connection type (type)",	CCommandFuncs::ConnectionType );
    m_pCommands->Add ( "saveconfig",	    "immediately saves the config",	    CCommandFuncs::SaveConfig );
    
#ifdef MTA_DEBUG
	//m_pCommands->Add ( "pools",				"read out the pool values",			CCommandFuncs::PoolRelocations );
#endif
}


bool CCore::GetResetNeeded ( )
{
	return m_bResetNeeded;
}


void CCore::SetRenderDevice ( IUnknown* pDevice )
{
	m_pRenderDevice = pDevice;
}


void CCore::SwitchRenderWindow ( HWND hWnd, HWND hWndInput )
{
    // Make GTA windowed
    m_pGame->GetSettings()->SetCurrentVideoMode(0);

	// Get the destination window rectangle
	RECT rect;
	GetWindowRect ( hWnd, &rect );

    // Size the GTA window size to the same size as the destination window rectangle
    HWND hDeviceWindow = CDirect3DData::GetSingleton ().GetDeviceWindow ();
    MoveWindow ( hDeviceWindow,
                 0,
                 0,
                 rect.right - rect.left,
                 rect.bottom - rect.top,
                 TRUE );

	// Turn the GTA window into a child window of our static render container window
    SetParent ( hDeviceWindow, hWnd );
	SetWindowLong ( hDeviceWindow, GWL_STYLE, WS_VISIBLE | WS_CHILD );
}


bool CCore::IsValidNick ( const char* szNick )
{
    // Too long or too short?
    size_t sizeNick = strlen ( szNick );
    if ( sizeNick > 0 && sizeNick <= 22 )
    {
        // Check each character
        for ( unsigned int i = 0; i < sizeNick; i++ )
        {
            // Don't allow 0x0A, 0x0D and <space>
            if ( szNick [i] == 0x0A ||
                 szNick [i] == 0x0D ||
                 szNick [i] == ' ' )
            {
                return false;
            }
        }

        return true;
    }

    return false;
}


#ifndef MTA_DEBUG
CResManager * CCore::GetResManager ( void )
{
    return m_pResManager;
}
#endif


void CCore::Quit ( bool bInstantly )
{
    if ( bInstantly )
    {
        // Destroy the client
        CModManager::GetSingleton ().Unload ();

        // Destroy ourself
        delete CCore::GetSingletonPtr ();

        // Use TerminateProcess for now as exiting the normal way crashes
        TerminateProcess ( GetCurrentProcess (), 0 );
        //PostQuitMessage ( 0 );
    }
    else
    {
        m_bQuitOnPulse = true;
    }
}


bool CCore::OnMouseClick ( CGUIMouseEventArgs Args )
{
	bool bHandled = false;

    bHandled = m_pLocalGUI->GetMainMenu ()->GetServerBrowser ()->OnMouseClick ( Args );		// CServerBrowser

	return bHandled;
}


bool CCore::OnMouseDoubleClick ( CGUIMouseEventArgs Args )
{
	bool bHandled = false;

	// Call the event handlers, where necessary
	bHandled =
		m_pLocalGUI->GetMainMenu ()->GetSettingsWindow ()->OnMouseDoubleClick ( Args ) |	// CSettings
		m_pLocalGUI->GetMainMenu ()->GetServerBrowser ()->OnMouseDoubleClick ( Args );		// CServerBrowser
	
	return bHandled;
}


const char* CCore::GetConnectCommandFromURI ( const char* szURI, char* szDest, size_t destLength )
{
    // Grab the length of the string
    size_t sizeURI = strlen ( szURI );
    
    // Parse it right to left
    char szLeft [256];
    szLeft [255] = 0;
    char* szLeftIter = szLeft + 255;

    char szRight [256];
    szRight [255] = 0;
    char* szRightIter = szRight + 255;

    const char* szIterator = szURI + sizeURI;
    bool bHitAt = false;

    for ( ; szIterator >= szURI + 8; szIterator-- )
    {
        if ( !bHitAt && *szIterator == '@' )
        {
            bHitAt = true;
        }
        else
        {
            if ( bHitAt )
            {
                if ( szLeftIter > szLeft )
                {
                    *(--szLeftIter) = *szIterator;
                }
            }
            else
            {
                if ( szRightIter > szRight )
                {
                    *(--szRightIter) = *szIterator;
                }
            }
        }
    }

    // Parse the host/port
    char szHost [64];
    char szPort [12];
    char* szHostIter = szHost;
    char* szPortIter = szPort;
    memset ( szHost, 0, sizeof(szHost) );
    memset ( szPort, 0, sizeof(szPort) );

    bool bIsInPort = false;
    size_t sizeRight = strlen ( szRightIter );
    for ( size_t i = 0; i < sizeRight; i++ )
    {
        if ( !bIsInPort && szRightIter [i] == ':' )
        {
            bIsInPort = true;
        }
        else
        {
            if ( bIsInPort )
            {
                if ( szPortIter < szPort + 11 )
                {
                    *(szPortIter++) = szRightIter [i];
                }
            }
            else
            {
                if ( szHostIter < szHost + 63 )
                {
                    *(szHostIter++) = szRightIter [i];
                }
            }
        }

    }

    // Parse the nickname / password
    char szNickname [64];
    char szPassword [64];
    char* szNicknameIter = szNickname;
    char* szPasswordIter = szPassword;
    memset ( szNickname, 0, sizeof(szNickname) );
    memset ( szPassword, 0, sizeof(szPassword) );

    bool bIsInPassword = false;
    size_t sizeLeft = strlen ( szLeftIter );
    for ( size_t i = 0; i < sizeLeft; i++ )
    {
        if ( !bIsInPassword && szLeftIter [i] == ':' )
        {
            bIsInPassword = true;
        }
        else
        {
            if ( bIsInPassword )
            {
                if ( szPasswordIter < szPassword + 63 )
                {
                    *(szPasswordIter++) = szLeftIter [i];
                }
            }
            else
            {
                if ( szNicknameIter < szNickname + 63 )
                {
                    *(szNicknameIter++) = szLeftIter [i];
                }
            }
        }

    }

    // If we got any port, convert it to an integral type
    unsigned short usPort = 22003;
    if ( strlen ( szPort ) > 0 )
    {
        usPort = static_cast < unsigned short > ( atoi ( szPort ) );
    }

    // Grab the nickname
    std::string strNick;
    if ( strlen ( szNickname ) > 0 )
    {
        strNick = szNickname;
    }
    else
    {
        CVARS_GET ( "nick", strNick );
    }

    // Generate a string with the arguments to send to the mod IF we got a host
    if ( strlen ( szHost ) > 0 )
    {
        if ( strlen ( szPassword ) > 0 )
            _snprintf ( szDest, destLength - 1, "connect %s %u %s %s", szHost, usPort, strNick.c_str (), szPassword );
        else
            _snprintf ( szDest, destLength - 1, "connect %s %u %s", szHost, usPort, strNick.c_str () );
    }
    else
    {
        szDest [ 0 ] = '\0';
    }

    return szDest;
}
