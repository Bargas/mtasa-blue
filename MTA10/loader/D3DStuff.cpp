/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        D3DStuff.cpp
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "d3d9.h"
HWND CreateWindowForD3D( void );
extern HINSTANCE g_hLauncherInstance;

namespace
{
    SString GUIDToString ( const GUID& g )
    {
        return SString (
                        "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2],
                        g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7] );
    }

    SString ToString( const D3DADAPTER_IDENTIFIER9& a )
    {
        return SString(
                        "    Driver:%s\n"
                        "    Description:%s\n"
                        "    DeviceName:%s\n"
                        "    DriverVersion:0x%08x 0x%08x\n"
                        "    VendorId:0x%08x  DeviceId:0x%08x  SubSysId:0x%08x  Revision:0x%08x  WHQLLevel:0x%08x\n"
                        "    DeviceIdentifier:%s"
                        , a.Driver
                        , a.Description
                        , a.DeviceName
                        , a.DriverVersion.HighPart
                        , a.DriverVersion.LowPart
                        , a.VendorId
                        , a.DeviceId
                        , a.SubSysId
                        , a.Revision
                        , a.WHQLLevel
                        , *GUIDToString( a.DeviceIdentifier )
                    );
    }

    SString ToString( const D3DDISPLAYMODE& a )
    {
        return SString(
                        " Width:%d"
                        " Height:%d"
                        " RefreshRate:%d"
                        " Format:%d"
                        , a.Width
                        , a.Height
                        , a.RefreshRate
                        , a.Format
                    );
    }

    SString ToString( const D3DCAPS9& a )
    {
        return SString(
                        " VertexShaderVersion:0x%08x"
                        " PixelShaderVersion:0x%08x"
                        " DeclTypes:0x%03x"
                        , a.VertexShaderVersion
                        , a.PixelShaderVersion
                        , a.DeclTypes
                    );
    }

    bool bDetectedOptimus = false;
    IDirect3D9* pD3D9 = NULL;
    IDirect3DDevice9* pD3DDevice9 = NULL;
    IDirect3DVertexDeclaration9* pD3DVertexDeclarations[ 20 ] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

    // Hacky log redirection
    void _WriteDebugEvent( const SString& strText )
    {
        if ( bDetectedOptimus )
            AddReportLog( 9800, strText );
        WriteDebugEvent( strText );
    }
    #define WriteDebugEvent _WriteDebugEvent
}


//////////////////////////////////////////////////////////
//
// BeginD3DStuff
//
// Look all busy and important in case any graphic drivers are looking
//
//////////////////////////////////////////////////////////
void BeginD3DStuff( void )
{
    pD3D9 = Direct3DCreate9( D3D_SDK_VERSION );

    if ( !pD3D9 )
    {
        WriteDebugEvent( "D3DStuff - Direct3DCreate9 failed" );
        return;
    }

    WriteDebugEvent( "D3DStuff -------------------------" );
    WriteDebugEvent( SString( "D3DStuff - Direct3DCreate9: 0x%08x", pD3D9 ) );

    // Get info about each connected adapter
    uint uiNumAdapters = pD3D9->GetAdapterCount();
    WriteDebugEvent( SString( "D3DStuff - %d Adapters", uiNumAdapters ) );

    for ( uint i = 0 ; i < uiNumAdapters ; i++ )
    {
        D3DADAPTER_IDENTIFIER9 Identifier;
        D3DDISPLAYMODE DisplayMode;
        D3DCAPS9 Caps9;

        HRESULT hr1 = pD3D9->GetAdapterIdentifier( i, 0, &Identifier );
        HRESULT hr2 = pD3D9->GetAdapterDisplayMode( i, &DisplayMode );
        HRESULT hr3 = pD3D9->GetDeviceCaps( i, D3DDEVTYPE_HAL, &Caps9 );
        UINT ModeCount = pD3D9->GetAdapterModeCount( i, D3DFMT_X8R8G8B8 );
        HMONITOR hMonitor = pD3D9->GetAdapterMonitor( i );

        if ( FAILED( hr1 ) || FAILED( hr2 ) || FAILED( hr3 ) )
        {
            WriteDebugEvent( SString( "D3DStuff %d Failed GetAdapterIdentifier(%x) GetAdapterDisplayMode(%x) GetDeviceCaps(%x) ", i, hr1, hr2, hr3 ) );
            continue;
        }

        // Detect Optimus combo
        if ( uiNumAdapters == 1
            && SStringX( Identifier.Driver ).BeginsWithI( "nv" )
            && SStringX( Identifier.Description ).BeginsWithI( "Intel" )
            )
        {
            bDetectedOptimus = true;
            WriteDebugEvent( SString( "D3DStuff %d - Detected Optimus", i ) );
        }

        WriteDebugEvent( SString( "D3DStuff %d Identifier - %s", i, *ToString( Identifier ) ) );
        WriteDebugEvent( SString( "D3DStuff %d DisplayMode - %s", i, *ToString( DisplayMode ) ) );
        WriteDebugEvent( SString( "D3DStuff %d  hMonitor:0x%08x  ModeCount:%d", i, hMonitor, ModeCount ) );
        WriteDebugEvent( SString( "D3DStuff %d Caps9 - %s ", i, *ToString( Caps9 ) ) );
    }

#ifdef MTA_DEBUG
    bDetectedOptimus = true;
#endif
    bool bAltStartup = false;

    if ( bDetectedOptimus )
    {
        int iResponse = MessageBox( NULL, "NVidia Optimus detected.\n\nDo you want to try the alternate startup method?", "MTA:SA Startup", MB_YESNO );
        if ( iResponse == IDYES )
            bAltStartup = true;
    }

    SetApplicationSettingInt( "diagnostics", "optimus", bDetectedOptimus );
    SetApplicationSettingInt( "diagnostics", "optimus-alt-startup", bAltStartup );
    return;

    // Try extra stuff
    HWND hWnd = CreateWindowForD3D();

    DWORD BehaviorFlags = 0x40;
    D3DPRESENT_PARAMETERS pp;
    pp.BackBufferWidth = 800;
    pp.BackBufferHeight = 600;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.BackBufferCount = 1;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.MultiSampleQuality = 0;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = hWnd;
    pp.Windowed = true;
    pp.EnableAutoDepthStencil = true;
    pp.AutoDepthStencilFormat = D3DFMT_D24S8;
    pp.Flags = 0;
    pp.FullScreen_RefreshRateInHz = 0;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    HRESULT hr = pD3D9->CreateDevice( 0, D3DDEVTYPE_HAL, hWnd, BehaviorFlags, &pp, &pD3DDevice9 );
    if ( FAILED( hr ) )
    {
        WriteDebugEvent( SString( "D3DStuff - CreateDevice failed %x", hr ) );
    }
    else
    {
        WriteDebugEvent( "D3DStuff - CreateDevice succeeded" );

        // Try some rendering
        pD3DDevice9->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0xffff00ff, 1, 0 );
        pD3DDevice9->Present( NULL, NULL, NULL, NULL );
        pD3DDevice9->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0xff00000f, 1, 0 );
        pD3DDevice9->Present( NULL, NULL, NULL, NULL );

        // Get caps
        D3DCAPS9 Caps9;
        hr = pD3DDevice9->GetDeviceCaps( &Caps9 );
        WriteDebugEvent( SString( "D3DStuff Caps9 - %s ", *ToString( Caps9 ) ) );

        // Test caps
        struct {
            DWORD CapsType;
            BYTE  VertexType;
        } DeclTypesList[] = {
                        { D3DDTCAPS_UBYTE4,     D3DDECLTYPE_UBYTE4 },
                        { D3DDTCAPS_UBYTE4N,    D3DDECLTYPE_UBYTE4N },
                        { D3DDTCAPS_SHORT2N,    D3DDECLTYPE_SHORT2N },
                        { D3DDTCAPS_SHORT4N,    D3DDECLTYPE_SHORT4N },
                        { D3DDTCAPS_USHORT2N,   D3DDECLTYPE_USHORT2N },
                        { D3DDTCAPS_USHORT4N,   D3DDECLTYPE_USHORT4N },
                        { D3DDTCAPS_UDEC3,      D3DDECLTYPE_UDEC3 },
                        { D3DDTCAPS_DEC3N,      D3DDECLTYPE_DEC3N },
                        { D3DDTCAPS_FLOAT16_2,  D3DDECLTYPE_FLOAT16_2 },
                        { D3DDTCAPS_FLOAT16_4,  D3DDECLTYPE_FLOAT16_4 },
                    };

        // Try each vertex declaration type to see if it matches with what was advertised
        uint uiNumItems = Min( NUMELMS( DeclTypesList ), NUMELMS( pD3DVertexDeclarations ) );
        uint uiNumMatchesCaps = 0;
        for( uint i = 0 ; i < uiNumItems ; i++ )
        {
            // Try create
            D3DVERTEXELEMENT9 VertexElements[] =
            {
                { 0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
                D3DDECL_END()
            };
            VertexElements[0].Type = DeclTypesList[i].VertexType;
            hr = pD3DDevice9->CreateVertexDeclaration( VertexElements, &pD3DVertexDeclarations[i] );

            // Check against device caps
            bool bCapsSaysOk = ( Caps9.DeclTypes & DeclTypesList[i].CapsType ) ? true : false;
            bool bMatchesCaps = ( hr == D3D_OK ) == ( bCapsSaysOk == true );
            if ( bMatchesCaps )
                uiNumMatchesCaps++;
            else
                WriteDebugEvent( SString( "D3DStuff - CreateVertexDeclaration %d/%d [MISMATCH] (VertexType:%d) result: %x (Matches caps:%d)", i, uiNumItems, DeclTypesList[i].VertexType, hr, bMatchesCaps ) );
        }
        WriteDebugEvent( SString( "D3DStuff - CreateVertexDeclarations MatchesCaps:%d/%d", uiNumMatchesCaps, uiNumItems ) );
    }
}


//////////////////////////////////////////////////////////
//
// EndD3DStuff
//
// Clean up
//
//////////////////////////////////////////////////////////
void EndD3DStuff( void )
{
    for( uint i = 0 ; i < NUMELMS( pD3DVertexDeclarations ) ; i++ )
        SAFE_RELEASE( pD3DVertexDeclarations[i] );
    SAFE_RELEASE( pD3DDevice9 );
    SAFE_RELEASE( pD3D9 );
}


////////////////////////////////////////////////////////////////
//
// CreateWindowForD3D
//
//
//
////////////////////////////////////////////////////////////////
HWND CreateWindowForD3D( void )
{
    DWORD dwStyle = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    int X = 10;
    int Y = 10;
    int XL = 100;
    int YL = 100;
    HWND Parent = NULL;
    HINSTANCE hInstance = g_hLauncherInstance;

    // Register class if required
    const TCHAR* lpszClassName = "D3D window class";
    {
        WNDCLASS wcex;

        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = (WNDPROC)DefWindowProc;
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = hInstance;
        wcex.hIcon          = LoadIcon( NULL, IDI_APPLICATION );
        wcex.hCursor        = LoadCursor( NULL, IDC_ARROW );
        wcex.hbrBackground  = (HBRUSH)( COLOR_WINDOW + 1 );
        wcex.lpszMenuName   = NULL;
        wcex.lpszClassName  = lpszClassName;

        // Returns null if already registered
        RegisterClass(&wcex);
    }

    HWND hWnd = CreateWindowEx( WS_EX_APPWINDOW, lpszClassName, "D3D window", dwStyle, X, Y, XL, YL, Parent, NULL, hInstance, NULL );

    if ( !hWnd )
      return FALSE;

   ShowWindow( hWnd, SW_SHOW );
   UpdateWindow( hWnd );

   return hWnd;
}
