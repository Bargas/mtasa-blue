/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CDirect3DEvents9.h
*  PURPOSE:     Header file for Direct3D 9 events class
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Derek Abdine <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CDIRECT3DEVENTS9_H
#define __CDIRECT3DEVENTS9_H

// To avoid conflict with d3d8 included by CGUI if not COMPILE_FOR_SA
#ifdef COMPILE_FOR_SA
#include <d3d9.h>
typedef IUnknown IDirect3DDevice8;
#else
#include <d3d8.h>
typedef IUnknown IDirect3DDevice9;
#endif

//#include <d3d9.h>

class CDirect3DEvents9
{
    public:

    static void __stdcall   OnDirect3DDeviceCreate  ( IDirect3DDevice9 *pDevice );
    static void __stdcall   OnDirect3DDeviceDestroy ( IDirect3DDevice9 *pDevice );
    static void __stdcall   OnPresent               ( IDirect3DDevice9 *pDevice );
    static void __stdcall   OnBeginScene            ( IDirect3DDevice9 *pDevice ); 
    static bool __stdcall   OnEndScene              ( IDirect3DDevice9 *pDevice );
    static void __stdcall   OnInvalidate            ( IDirect3DDevice9 *pDevice );
    static void __stdcall   OnRestore               ( IDirect3DDevice9 *pDevice );
};

#endif