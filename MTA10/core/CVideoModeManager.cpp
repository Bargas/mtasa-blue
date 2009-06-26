/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CVideoModeManager.cpp
*  PURPOSE:     Video mode related operations
*  DEVELOPERS:  ccw <chris@codewave.co.uk>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include <game/CGame.h>

using std::string;

//////////////////////////////////////////////////////////
//
// Helper stuff
//

#define ZERO_ON_NEW \
    void* operator new ( size_t size )              { void* ptr = ::operator new(size); memset(ptr,0,size); return ptr; } \
    void* operator new ( size_t size, void* where ) { memset(where,0,size); return where; }



///////////////////////////////////////////////////////////////
//
// CVideoModeManager class
//
///////////////////////////////////////////////////////////////
class CVideoModeManager : public CVideoModeManagerInterface
{
public:
    ZERO_ON_NEW
                        CVideoModeManager           ( void );
                        ~CVideoModeManager          ( void );

    // CVideoModeManagerInterface methods
    virtual void        PreCreateDevice             ( D3DPRESENT_PARAMETERS* pp );
    virtual void        PostCreateDevice            ( IDirect3DDevice9* pD3DDevice, D3DPRESENT_PARAMETERS* pp );
    virtual void        PreReset                    ( D3DPRESENT_PARAMETERS* pp );
    virtual void        PostReset                   ( D3DPRESENT_PARAMETERS* pp );
    virtual void        ChangeVideoMode             ( int iNewMode );

    // CVideoModeManager methods
private:
    void                FromWindowed                ( int iNewMode );
    void                ToWindowed                  ( void );
    void                DoSetCurrentVideoMode       ( int iNewMode );

    int                 bForceFullScreenOnce;
    int                 bForceWindowed;
    int                 ForceBackBufferWidth;
    int                 ForceBackBufferHeight;
    int                 iLastMode;
    int                 iLastFullscreenMode;
    HWND                hDeviceWindow;
    CGameSettings *     gameSettings;
};


///////////////////////////////////////////////////////////////
//
// CVideoModeManager instantiation
//
///////////////////////////////////////////////////////////////
CVideoModeManagerInterface* NewVideoModeManager ( void )
{
    return new CVideoModeManager ();
}

// This is nice so there
CVideoModeManagerInterface* g_pVideoModeManager = NULL;

CVideoModeManagerInterface* GetVideoModeManager ( void )
{
    if ( !g_pVideoModeManager )
        g_pVideoModeManager = NewVideoModeManager ();
    return g_pVideoModeManager;
}


///////////////////////////////////////////////////////////////
//
// CVideoModeManager implementation
//
///////////////////////////////////////////////////////////////
CVideoModeManager::CVideoModeManager ( void )
{
    gameSettings = CCore::GetSingleton ( ).GetGame ( )->GetSettings();
    iLastMode = -1;
    iLastFullscreenMode = -1;
}


CVideoModeManager::~CVideoModeManager ( void )
{
}


///////////////////////////////////////////////////////////////
//
// CVideoModeManager::PreCreateDevice
//
// Warning - Changes here could effect stability for switching between windowed mode and alt-tabbing
//
///////////////////////////////////////////////////////////////
void CVideoModeManager::PreCreateDevice ( D3DPRESENT_PARAMETERS* pp )
{
    hDeviceWindow = pp->hDeviceWindow;

    // This block helps stability
    SetWindowLong ( hDeviceWindow, GWL_STYLE, WS_POPUP );
    int x, y;
    x = GetSystemMetrics ( SM_CXSCREEN );
    y = GetSystemMetrics ( SM_CYSCREEN );
        MoveWindow ( hDeviceWindow, 
                    (x/2)-(pp->BackBufferWidth/2), 
                    (y/2)-(pp->BackBufferHeight/2), 
                    pp->BackBufferWidth,
                    pp->BackBufferHeight,
                    TRUE );
    pp->Windowed = true;
    pp->FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

#if MTA_DEBUG
    bool bStartupWindowed = true;
#else
    bool bStartupWindowed = false;
#endif

    if ( bStartupWindowed )
    {
        bForceFullScreenOnce = false;
        bForceWindowed = true;
        ForceBackBufferWidth = pp->BackBufferWidth;
        ForceBackBufferHeight = pp->BackBufferHeight;
    }
    else
    {
        bForceFullScreenOnce = true;
        bForceWindowed = false;
        ForceBackBufferWidth = pp->BackBufferWidth;
        ForceBackBufferHeight = pp->BackBufferHeight;
    }
}
	

///////////////////////////////////////////////////////////////
//
// CVideoModeManager::PostCreateDevice
//
//
//
///////////////////////////////////////////////////////////////
void CVideoModeManager::PostCreateDevice ( IDirect3DDevice9* pD3DDevice, D3DPRESENT_PARAMETERS* pp )
{
    // This helps stability
    pD3DDevice->Reset ( pp );
}


///////////////////////////////////////////////////////////////
//
// CVideoModeManager::PreReset
//
//
//
///////////////////////////////////////////////////////////////
void CVideoModeManager::PreReset ( D3DPRESENT_PARAMETERS* pp )
{
    if ( bForceFullScreenOnce )
    {
        bForceFullScreenOnce = false;
        pp->Windowed = false;
        pp->FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    }
    else
    if ( bForceWindowed )
    {
        pp->Windowed = true;
        pp->FullScreen_RefreshRateInHz = 0;
    }

    pp->BackBufferWidth = ForceBackBufferWidth;
    pp->BackBufferHeight = ForceBackBufferHeight;
}


///////////////////////////////////////////////////////////////
//
// CVideoModeManager::PostReset
//
//
//
///////////////////////////////////////////////////////////////
void CVideoModeManager::PostReset ( D3DPRESENT_PARAMETERS* pp )
{
    if ( pp->Windowed )
    {
        // Add frame
        LONG Style = WS_VISIBLE | WS_CLIPSIBLINGS | WS_BORDER | WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX;
        SetWindowLong ( hDeviceWindow, GWL_STYLE, Style );

        LONG ExStyle = WS_EX_WINDOWEDGE;
        SetWindowLong ( hDeviceWindow, GWL_EXSTYLE, ExStyle );

        // Ensure client area of window is correct size
		RECT ClientRect = { 0, 0, pp->BackBufferWidth, pp->BackBufferHeight };
		AdjustWindowRect( &ClientRect, GetWindowLong(hDeviceWindow,GWL_STYLE), FALSE );

		int SizeX = ClientRect.right - ClientRect.left;
		int SizeY = ClientRect.bottom - ClientRect.top;

        SetWindowPos( hDeviceWindow, HWND_NOTOPMOST, 0, 0, SizeX, SizeY, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED );
	}
}


///////////////////////////////////////////////////////////////
//
// CVideoModeManager::ChangeVideoMode
//
// -1       - Fullscreen with last used fullscreen mode
// 0        - Windowed
// 1 and up - Fullscreen with specified mode
//
//
///////////////////////////////////////////////////////////////
void CVideoModeManager::ChangeVideoMode ( int iNewMode )
{
    // To windowed?
    if ( iNewMode == 0 )
    {
        if ( iLastMode != 0 )
            ToWindowed ();
        return;
    }

    // From windowed?
    if ( iLastMode == 0 )
    {
        if ( iNewMode != 0 )
            FromWindowed ( iNewMode );
        return;
    }

    // Fullscreen to fullscreen
    DoSetCurrentVideoMode ( iNewMode );
}


///////////////////////////////////////////////////////////////
//
// CVideoModeManager::FromWindowed
//
//
//
///////////////////////////////////////////////////////////////
void CVideoModeManager::FromWindowed ( int iFullscreenMode )
{
    // Determine valid target fullscreen mode
    if ( iFullscreenMode == 0 )
        return;

    if ( iFullscreenMode < 1 )
    {
        iFullscreenMode = iLastFullscreenMode;
    }

    if ( iFullscreenMode < 1 )
    {
        iFullscreenMode = gameSettings->GetCurrentVideoMode ();
    }

    if ( iFullscreenMode < 1 )
    {
        return;
    }

    // Remove window frame
    LONG Style = WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS;
    SetWindowLong ( hDeviceWindow, GWL_STYLE, Style );

    LONG ExStyle = WS_EX_TOPMOST;
    SetWindowLong ( hDeviceWindow, GWL_EXSTYLE, ExStyle );

    DoSetCurrentVideoMode ( iFullscreenMode );
}


///////////////////////////////////////////////////////////////
//
// CVideoModeManager::ToWindowed
//
//
//
///////////////////////////////////////////////////////////////
void CVideoModeManager::ToWindowed ( void )
{
    if ( iLastFullscreenMode < 1 )
    {
        iLastFullscreenMode = gameSettings->GetCurrentVideoMode ();
    }
    DoSetCurrentVideoMode ( 0 );
}


///////////////////////////////////////////////////////////////
//
// CVideoModeManager::DoSetCurrentVideoMode
//
//
//
///////////////////////////////////////////////////////////////
void CVideoModeManager::DoSetCurrentVideoMode ( int iNewMode )
{
    if ( iNewMode == -1 )
        return;

    // Remeber last set mode
    iLastMode = iNewMode;

    if ( iNewMode != 0 )
    {
        // Remember last set fullscreen mode
        iLastFullscreenMode = iNewMode;

        // Update ForceBackBuffer sizes
        VideoMode   vidModemInfo;
        gameSettings->GetVideoModeInfo ( &vidModemInfo, iNewMode );
        ForceBackBufferWidth = vidModemInfo.width;
        ForceBackBufferHeight = vidModemInfo.height;
    }

    // Turn this off now
    bForceWindowed = false;

    gameSettings->SetCurrentVideoMode ( iNewMode );
}
