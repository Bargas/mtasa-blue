/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Drawing.cpp
*  PURPOSE:     Lua function definitions class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Chris McArthur <>
*               Derek Abdine <>
*               Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*
*****************************************************************************/

#include "StdInc.h"

int CLuaFunctionDefs::dxDrawLine ( lua_State* luaVM )
{
//  bool dxDrawLine ( int startX, int startY, int endX, int endY, int color, [float width=1, bool postGUI=false] )
    float fStartX; float fStartY; float fEndX; float fEndY; uint ulColor; float fWidth; bool bPostGUI;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( fStartX );
    argStream.ReadNumber ( fStartY );
    argStream.ReadNumber ( fEndX );
    argStream.ReadNumber ( fEndY );
    argStream.ReadNumber ( ulColor, 0xFFFFFFFF );
    argStream.ReadNumber ( fWidth, 1 );
    argStream.ReadBool ( bPostGUI, false );

    if ( !argStream.HasErrors () )
    {
        g_pCore->GetGraphics ()->DrawLineQueued ( fStartX, fStartY, fEndX, fEndY, fWidth, ulColor, bPostGUI );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxDrawLine3D ( lua_State* luaVM )
{
// bool dxDrawLine3D ( float startX, float startY, float startZ, float endX, float endY, float endZ, int color[, int width, bool postGUI ] )
    CVector vecBegin; CVector vecEnd; uint ulColor; float fWidth; bool bPostGUI;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( vecBegin.fX );
    argStream.ReadNumber ( vecBegin.fY );
    argStream.ReadNumber ( vecBegin.fZ );
    argStream.ReadNumber ( vecEnd.fX );
    argStream.ReadNumber ( vecEnd.fY );
    argStream.ReadNumber ( vecEnd.fZ );
    argStream.ReadNumber ( ulColor, 0xFFFFFFFF );
    argStream.ReadNumber ( fWidth, 1 );
    argStream.ReadBool ( bPostGUI, false );

    if ( !argStream.HasErrors () )
    {
        g_pCore->GetGraphics ()->DrawLine3DQueued ( vecBegin, vecEnd, fWidth, ulColor, bPostGUI );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxDrawMaterialLine3D ( lua_State* luaVM )
{
// bool dxDrawMaterialLine3D ( float startX, float startY, float startZ, float endX, float endY, float endZ, element material, int width [, int color = white,
//                          float faceX, float faceY, float faceZ ] )
    CVector vecBegin; CVector vecEnd; CClientMaterial* pMaterial; float fWidth; uint ulColor;
    CVector vecFaceToward; bool bUseFaceToward = false;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( vecBegin.fX );
    argStream.ReadNumber ( vecBegin.fY );
    argStream.ReadNumber ( vecBegin.fZ );
    argStream.ReadNumber ( vecEnd.fX );
    argStream.ReadNumber ( vecEnd.fY );
    argStream.ReadNumber ( vecEnd.fZ );
    argStream.ReadUserData ( pMaterial );
    argStream.ReadNumber ( fWidth );
    argStream.ReadNumber ( ulColor, 0xFFFFFFFF );
    if ( argStream.NextCouldBeNumber () )
    {
        argStream.ReadNumber ( vecFaceToward.fX );
        argStream.ReadNumber ( vecFaceToward.fY );
        argStream.ReadNumber ( vecFaceToward.fZ );
        bUseFaceToward = true;
    }

    if ( !argStream.HasErrors () )
    {
        g_pCore->GetGraphics ()->DrawMaterialLine3DQueued ( vecBegin, vecEnd, fWidth, ulColor, pMaterial->GetMaterialItem (), 0, 0, 1, 1, true, bUseFaceToward, vecFaceToward );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxDrawMaterialSectionLine3D ( lua_State* luaVM )
{
// bool dxDrawMaterialSectionLine3D ( float startX, float startY, float startZ, float endX, float endY, float endZ, float u, float v, float usize, float vsize,
//                                  element material, int width, [ int color = white, float faceX, float faceY, float faceZ ] )
    CVector vecBegin; CVector vecEnd; float fU; float fV; float fSizeU; float fSizeV;
    CClientMaterial* pMaterial; float fWidth; uint ulColor; CVector vecFaceToward; bool bUseFaceToward = false;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( vecBegin.fX );
    argStream.ReadNumber ( vecBegin.fY );
    argStream.ReadNumber ( vecBegin.fZ );
    argStream.ReadNumber ( vecEnd.fX );
    argStream.ReadNumber ( vecEnd.fY );
    argStream.ReadNumber ( vecEnd.fZ );
    argStream.ReadNumber ( fU );
    argStream.ReadNumber ( fV );
    argStream.ReadNumber ( fSizeU );
    argStream.ReadNumber ( fSizeV );
    argStream.ReadUserData ( pMaterial );
    argStream.ReadNumber ( fWidth );
    argStream.ReadNumber ( ulColor, 0xFFFFFFFF );
    if ( argStream.NextCouldBeNumber () )
    {
        argStream.ReadNumber ( vecFaceToward.fX );
        argStream.ReadNumber ( vecFaceToward.fY );
        argStream.ReadNumber ( vecFaceToward.fZ );
        bUseFaceToward = true;
    }

    if ( !argStream.HasErrors () )
    {
        g_pCore->GetGraphics ()->DrawMaterialLine3DQueued ( vecBegin, vecEnd, fWidth, ulColor, pMaterial->GetMaterialItem (), fU, fV, fSizeU, fSizeV, false, bUseFaceToward, vecFaceToward );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxDrawText ( lua_State* luaVM )
{
//  bool dxDrawText ( string text, float left, float top [, float right=left, float bottom=top, int color=white, float scale=1, mixed font="default", 
//      string alignX="left", string alignY="top", bool clip=false, bool wordBreak=false, bool postGUI=false, bool colorCoded=false, bool subPixelPositioning=false] )
    SString strText; float fLeft; float fTop; float fRight; float fBottom; ulong ulColor; float fScaleX; float fScaleY; SString strFontName; CClientDxFont* pDxFontElement;
    eDXHorizontalAlign alignX; eDXVerticalAlign alignY; bool bClip; bool bWordBreak; bool bPostGUI; bool bColorCoded; bool bSubPixelPositioning;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strText );
    argStream.ReadNumber ( fLeft );
    argStream.ReadNumber ( fTop );
    argStream.ReadNumber ( fRight, fLeft );
    argStream.ReadNumber ( fBottom, fTop );
    argStream.ReadNumber ( ulColor, 0xFFFFFFFF );
    argStream.ReadNumber ( fScaleX, 1 );
    if ( argStream.NextIsNumber () )
        argStream.ReadNumber ( fScaleY );
    else
        fScaleY = fScaleX;
    MixedReadDxFontString ( argStream, strFontName, "default", pDxFontElement );
    argStream.ReadEnumString ( alignX, DX_ALIGN_LEFT );
    argStream.ReadEnumString ( alignY, DX_ALIGN_TOP );
    argStream.ReadBool ( bClip, false );
    argStream.ReadBool ( bWordBreak, false );
    argStream.ReadBool ( bPostGUI, false );
    argStream.ReadBool ( bColorCoded, false );
    argStream.ReadBool ( bSubPixelPositioning, false );

    if ( !argStream.HasErrors () )
    {
        // Get DX font
        ID3DXFont* pD3DXFont = CStaticFunctionDefinitions::ResolveD3DXFont ( strFontName, pDxFontElement );

        // Make format flag
        ulong ulFormat = alignX | alignY;
        //if ( ulFormat & DT_BOTTOM ) ulFormat |= DT_SINGLELINE;        MS says we should do this. Nobody tells me what to do.
        if ( bWordBreak )           ulFormat |= DT_WORDBREAK;
        if ( !bClip )               ulFormat |= DT_NOCLIP;

        CStaticFunctionDefinitions::DrawText ( fLeft, fTop, fRight, fBottom, ulColor, strText, fScaleX, fScaleY, ulFormat, pD3DXFont, bPostGUI, bColorCoded, bSubPixelPositioning );

        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxDrawRectangle ( lua_State* luaVM )
{
// bool dxDrawRectangle ( int startX, int startY, float width, float height [, int color = white, bool postGUI = false] )
    float fStartX; float fStartY; float fWidth; float fHeight; uint ulColor; bool bPostGUI;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( fStartX );
    argStream.ReadNumber ( fStartY );
    argStream.ReadNumber ( fWidth );
    argStream.ReadNumber ( fHeight );
    argStream.ReadNumber ( ulColor, 0xFFFFFFFF );
    argStream.ReadBool ( bPostGUI, false );


    if ( !argStream.HasErrors () )
    {
        g_pCore->GetGraphics ()->DrawRectQueued ( fStartX, fStartY, fWidth, fHeight, ulColor, bPostGUI );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxDrawImage ( lua_State* luaVM )
{
//  bool dxDrawImage ( float posX, float posY, float width, float height, string filepath [, float rotation = 0, float rotationCenterOffsetX = 0, 
//      float rotationCenterOffsetY = 0, int color = white, bool postGUI = false ] )
    float fPosX; float fPosY; float fWidth; float fHeight; CClientMaterial* pMaterialElement; float fRotation;
        float fRotCenOffX; float fRotCenOffY; uint ulColor; bool bPostGUI;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( fPosX );
    argStream.ReadNumber ( fPosY );
    argStream.ReadNumber ( fWidth );
    argStream.ReadNumber ( fHeight );
    MixedReadMaterialString ( argStream, pMaterialElement );
    argStream.ReadNumber ( fRotation, 0 );
    argStream.ReadNumber ( fRotCenOffX, 0 );
    argStream.ReadNumber ( fRotCenOffY, 0 );
    argStream.ReadNumber ( ulColor, 0xffffffff );
    argStream.ReadBool ( bPostGUI, false );

    if ( !argStream.HasErrors () )
    {
        if ( pMaterialElement )
        {
            g_pCore->GetGraphics ()->DrawTextureQueued ( fPosX, fPosY, fWidth, fHeight, 0, 0, 1, 1, true, pMaterialElement->GetMaterialItem (), fRotation, fRotCenOffX, fRotCenOffY, ulColor, bPostGUI );
            lua_pushboolean ( luaVM, true );
            return 1;
        }
        else
            m_pScriptDebugging->LogError ( luaVM, "dxDrawImage can't load file" );
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxDrawImageSection ( lua_State* luaVM )
{
//  bool dxDrawImageSection ( float posX, float posY, float width, float height, float u, float v, float usize, float vsize, string filepath, 
//      [ float rotation = 0, float rotationCenterOffsetX = 0, float rotationCenterOffsetY = 0, int color = white, bool postGUI = false ] )
    float fPosX; float fPosY; float fWidth; float fHeight; float fU; float fV; float fSizeU; float fSizeV; CClientMaterial* pMaterialElement;
         float fRotation; float fRotCenOffX; float fRotCenOffY; uint ulColor; bool bPostGUI;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( fPosX );
    argStream.ReadNumber ( fPosY );
    argStream.ReadNumber ( fWidth );
    argStream.ReadNumber ( fHeight );
    argStream.ReadNumber ( fU );
    argStream.ReadNumber ( fV );
    argStream.ReadNumber ( fSizeU );
    argStream.ReadNumber ( fSizeV );
    MixedReadMaterialString ( argStream, pMaterialElement );
    argStream.ReadNumber ( fRotation, 0 );
    argStream.ReadNumber ( fRotCenOffX, 0 );
    argStream.ReadNumber ( fRotCenOffY, 0 );
    argStream.ReadNumber ( ulColor, 0xffffffff );
    argStream.ReadBool ( bPostGUI, false );

    if ( !argStream.HasErrors () )
    {
        if ( pMaterialElement )
        {
            g_pCore->GetGraphics ()->DrawTextureQueued ( fPosX, fPosY, fWidth, fHeight, fU, fV, fSizeU, fSizeV, false, pMaterialElement->GetMaterialItem (), fRotation, fRotCenOffX, fRotCenOffY, ulColor, bPostGUI );
            lua_pushboolean ( luaVM, true );
            return 1;
        }
        else
            m_pScriptDebugging->LogError ( luaVM, "dxDrawImageSection can't load file" );
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxGetTextWidth ( lua_State* luaVM )
{
//  float dxGetTextWidth ( string text, [float scale=1, mixed font="default"] )
    SString strText; float fScale; SString strFontName; CClientDxFont* pDxFontElement;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strText );
    argStream.ReadNumber ( fScale, 1 );
    MixedReadDxFontString ( argStream, strFontName, "default", pDxFontElement );

    if ( !argStream.HasErrors () )
    {
        ID3DXFont* pD3DXFont = CStaticFunctionDefinitions::ResolveD3DXFont ( strFontName, pDxFontElement );

        // Retrieve the longest line's extent
        std::stringstream ssText ( strText );
        std::string sLineText;
        float fWidth = 0.0f, fLineExtent = 0.0f;

        while( std::getline ( ssText, sLineText ) )
        {
            fLineExtent = g_pCore->GetGraphics ()->GetDXTextExtent ( sLineText.c_str ( ), fScale, pD3DXFont );
            if ( fLineExtent > fWidth )
                fWidth = fLineExtent;
        }

        // Success
        lua_pushnumber ( luaVM, fWidth );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxGetFontHeight ( lua_State* luaVM )
{
//  int dxGetFontHeight ( [float scale=1, mixed font="default"] )
    float fScale; SString strFontName; CClientDxFont* pDxFontElement;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( fScale, 1 );
    MixedReadDxFontString ( argStream, strFontName, "default", pDxFontElement );

    if ( !argStream.HasErrors () )
    {
        ID3DXFont* pD3DXFont = CStaticFunctionDefinitions::ResolveD3DXFont ( strFontName, pDxFontElement );

        float fHeight = g_pCore->GetGraphics ()->GetDXFontHeight ( fScale, pD3DXFont );
        // Success
        lua_pushnumber ( luaVM, fHeight );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxCreateTexture ( lua_State* luaVM )
{
//  element dxCreateTexture( string filepath [, string textureFormat = "argb", bool mipmaps = true, string textureEdge = "wrap" ] )
//  element dxCreateTexture( string pixels [, string textureFormat = "argb", bool mipmaps = true, string textureEdge = "wrap" ] )
//  element dxCreateTexture( int width, int height [, string textureFormat = "argb", string textureEdge = "wrap", string textureType = "2d", int depth ] )
    SString strFilePath; CPixels pixels; int width; int height; ERenderFormat renderFormat; bool bMipMaps;
    ETextureAddress textureAddress; ETextureType textureType; int depth = 1;

    CScriptArgReader argStream ( luaVM );
    if ( !argStream.NextIsNumber () )
    {
        argStream.ReadCharStringRef ( pixels.externalData );
        if ( !g_pCore->GetGraphics ()->GetPixelsManager ()->IsPixels ( pixels ) )
        {
            // element dxCreateTexture( string filepath [, string textureFormat = "argb", bool mipmaps = true, string textureEdge = "wrap" ] )
            pixels = CPixels ();
            argStream = CScriptArgReader ( luaVM );
            argStream.ReadString ( strFilePath );
            argStream.ReadEnumString ( renderFormat, RFORMAT_UNKNOWN );
            argStream.ReadBool ( bMipMaps, true );
            argStream.ReadEnumString ( textureAddress, TADDRESS_WRAP );
        }
        else
        {
            // element dxCreateTexture( string pixels [, string textureFormat = "argb", bool mipmaps = true, string textureEdge = "wrap" ] )
            argStream.ReadEnumString ( renderFormat, RFORMAT_UNKNOWN );
            argStream.ReadBool ( bMipMaps, true );
            argStream.ReadEnumString ( textureAddress, TADDRESS_WRAP );
        }
    }
    else
    {
        // element dxCreateTexture( int width, int height [, string textureFormat = "argb", string textureEdge = "wrap", string textureType = "2d", int depth ] )
        argStream.ReadNumber ( width );
        argStream.ReadNumber ( height );
        argStream.ReadEnumString ( renderFormat, RFORMAT_UNKNOWN );
        if ( argStream.NextIsEnumString ( textureType ) )
        {
           // r4019 to r4037 had incorrect argument order
            m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), "invalid texture-edge type at argument 4" ) );
            textureAddress = TADDRESS_WRAP;
        }
        else
            argStream.ReadEnumString ( textureAddress, TADDRESS_WRAP );
        argStream.ReadEnumString ( textureType, TTYPE_TEXTURE );
        if ( textureType == TTYPE_VOLUMETEXTURE )
            argStream.ReadNumber ( depth );
    }

    if ( !argStream.HasErrors () )
    {
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            CResource* pParentResource = pLuaMain->GetResource ();

            if ( !strFilePath.empty () )
            {
                // From file
                CResource* pFileResource = pParentResource;
                SString strPath, strMetaPath;
                if ( CResourceManager::ParseResourcePathInput( strFilePath, pFileResource, strPath, strMetaPath ) )
                {
                    if ( FileExists ( strPath ) )
                    {
                        CClientTexture* pTexture = g_pClientGame->GetManager ()->GetRenderElementManager ()->CreateTexture ( strPath, NULL, bMipMaps, RDEFAULT, RDEFAULT, renderFormat, textureAddress );
                        if ( pTexture )
                        {
                            // Make it a child of the resource's file root ** CHECK  Should parent be pFileResource, and element added to pParentResource's ElementGroup? **
                            pTexture->SetParent ( pParentResource->GetResourceDynamicEntity () );
                        }
                        lua_pushelement ( luaVM, pTexture );
                        return 1;
                    }
                    else
                        m_pScriptDebugging->LogBadPointer ( luaVM, "file-path", 1 );
                }
                else
                    m_pScriptDebugging->LogBadPointer ( luaVM, "file-path", 1 );
            }
            else
            if ( pixels.GetSize () )
            {
                // From pixels
                CClientTexture* pTexture = g_pClientGame->GetManager ()->GetRenderElementManager ()->CreateTexture ( "", &pixels, bMipMaps, RDEFAULT, RDEFAULT, renderFormat, textureAddress );
                if ( pTexture )
                {
                    pTexture->SetParent ( pParentResource->GetResourceDynamicEntity () );
                }
                lua_pushelement ( luaVM, pTexture );
                return 1;
            }
            else
            {
                // Blank sized
                CClientTexture* pTexture = g_pClientGame->GetManager ()->GetRenderElementManager ()->CreateTexture ( "", NULL, false, width, height, renderFormat, textureAddress, textureType, depth );
                if ( pTexture )
                {
                    pTexture->SetParent ( pParentResource->GetResourceDynamicEntity () );
                }
                lua_pushelement ( luaVM, pTexture );
                return 1;
            }
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxCreateShader ( lua_State* luaVM )
{
//  element dxCreateShader( string filepath [, float priority = 0, float maxdistance = 0, bool debug = false ] )
    SString strFilePath; float fPriority; float fMaxDistance; bool bDebug;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strFilePath );
    argStream.ReadNumber ( fPriority, 0.0f );
    argStream.ReadNumber ( fMaxDistance, 0.0f );
    argStream.ReadBool ( bDebug, false );

    if ( !argStream.HasErrors () )
    {
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            CResource* pParentResource = pLuaMain->GetResource ();
            CResource* pFileResource = pParentResource;
            SString strPath, strMetaPath;
            if ( CResourceManager::ParseResourcePathInput( strFilePath, pFileResource, strPath, strMetaPath ) )
            {
                if ( FileExists ( strPath ) )
                {
                    SString strRootPath = strPath.Left ( strPath.length () - strMetaPath.length () );
                    SString strStatus;
                    CClientShader* pShader = g_pClientGame->GetManager ()->GetRenderElementManager ()->CreateShader ( strPath, strRootPath, strStatus, fPriority, fMaxDistance, bDebug );
                    if ( pShader )
                    {
                        // Make it a child of the resource's file root ** CHECK  Should parent be pFileResource, and element added to pParentResource's ElementGroup? **
                        pShader->SetParent ( pParentResource->GetResourceDynamicEntity () );
                        lua_pushelement ( luaVM, pShader );
                        lua_pushstring ( luaVM, strStatus );    // String containing name of technique being used.
                        return 2;
                    }
                    else
                    {
                        // Replace any path in the error message with our own one
                        SString strRootPathWithoutResource = strRootPath.Left ( strRootPath.TrimEnd ( "\\" ).length () - SStringX ( pFileResource->GetName () ).length () ) ;
                        strStatus = strStatus.ReplaceI ( strRootPathWithoutResource, "" );
                        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Problem @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *strStatus ) );
                    }
                }
                else
                    m_pScriptDebugging->LogCustom ( luaVM, SString ( "Missing file @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *ConformResourcePath ( strPath, true ) ) );
            }
            else
                m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad file-path @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *strFilePath ) );
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxCreateRenderTarget ( lua_State* luaVM )
{
//  element dxCreateRenderTarget( int sizeX, int sizeY [, int withAlphaChannel = false ] )
    uint uiSizeX; uint uiSizeY; bool bWithAlphaChannel;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( uiSizeX );
    argStream.ReadNumber ( uiSizeY );
    argStream.ReadBool ( bWithAlphaChannel, false );

    if ( !argStream.HasErrors () )
    {
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        CResource* pParentResource = pLuaMain ? pLuaMain->GetResource () : NULL;
        if ( pParentResource )
        {
            CClientRenderTarget* pRenderTarget = g_pClientGame->GetManager ()->GetRenderElementManager ()->CreateRenderTarget ( uiSizeX, uiSizeY, bWithAlphaChannel );
            if ( pRenderTarget )
            {
                // Make it a child of the resource's file root ** CHECK  Should parent be pFileResource, and element added to pParentResource's ElementGroup? **
                pRenderTarget->SetParent ( pParentResource->GetResourceDynamicEntity () );
            }
            lua_pushelement ( luaVM, pRenderTarget );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxCreateScreenSource ( lua_State* luaVM )
{
//  element dxCreateScreenSource( int sizeX, int sizeY )
    uint uiSizeX; uint uiSizeY;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( uiSizeX );
    argStream.ReadNumber ( uiSizeY );

    if ( !argStream.HasErrors () )
    {
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        CResource* pParentResource = pLuaMain ? pLuaMain->GetResource () : NULL;
        if ( pParentResource )
        {
            CClientScreenSource* pScreenSource = g_pClientGame->GetManager ()->GetRenderElementManager ()->CreateScreenSource ( uiSizeX, uiSizeY );
            if ( pScreenSource )
            {
                // Make it a child of the resource's file root ** CHECK  Should parent be pFileResource, and element added to pParentResource's ElementGroup? **
                pScreenSource->SetParent ( pParentResource->GetResourceDynamicEntity () );
            }
            lua_pushelement ( luaVM, pScreenSource );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxGetMaterialSize ( lua_State* luaVM )
{
//  int, int [, int] dxGetMaterialSize( element material )
    CClientMaterial* pMaterial; SString strName;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pMaterial );

    if ( !argStream.HasErrors () )
    {
        lua_pushnumber ( luaVM, pMaterial->GetMaterialItem ()->m_uiSizeX );
        lua_pushnumber ( luaVM, pMaterial->GetMaterialItem ()->m_uiSizeY );
        if ( CFileTextureItem* pTextureItem = DynamicCast < CFileTextureItem > ( pMaterial->GetMaterialItem () ) )
        {
            if ( pTextureItem->m_TextureType == TTYPE_VOLUMETEXTURE )
            {
                lua_pushnumber ( luaVM, pTextureItem->m_uiVolumeDepth );
                return 3;
            }
        }
        return 2;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxSetShaderValue ( lua_State* luaVM )
{
//  bool dxSetShaderValue( element shader, string name, mixed value )
    CClientShader* pShader; SString strName;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pShader );
    argStream.ReadString ( strName );

    if ( !argStream.HasErrors () )
    {
        // Try each mixed type in turn
        int iArgument = lua_type ( argStream.m_luaVM, argStream.m_iIndex );

        if ( iArgument == LUA_TLIGHTUSERDATA )
        {
            // Texture
            CClientTexture* pTexture;
            if ( argStream.ReadUserData ( pTexture ) )
            {
                bool bResult = pShader->GetShaderItem ()->SetValue ( strName, pTexture->GetTextureItem () );
                lua_pushboolean ( luaVM, bResult );
                return 1;
            }
        }
        else
        if ( iArgument == LUA_TBOOLEAN )
        {
            // bool
            bool bValue;
            if ( argStream.ReadBool ( bValue ) )
            {
                bool bResult = pShader->GetShaderItem ()->SetValue ( strName, bValue );
                lua_pushboolean ( luaVM, bResult );
                return 1;
            }
        }
        else
        if ( iArgument == LUA_TNUMBER || iArgument == LUA_TSTRING )
        {
            // float(s)
            float fBuffer[16];
            uint i;
            for ( i = 0 ; i < NUMELMS(fBuffer); )
            {
                fBuffer[i++] = static_cast < float > ( lua_tonumber ( argStream.m_luaVM, argStream.m_iIndex++ ) );
                iArgument = lua_type ( argStream.m_luaVM, argStream.m_iIndex );
                if ( iArgument != LUA_TNUMBER && iArgument != LUA_TSTRING )
                    break;
            }
            bool bResult = pShader->GetShaderItem ()->SetValue ( strName, fBuffer, i );
            lua_pushboolean ( luaVM, bResult );
            return 1;
        }
        else
        if ( iArgument == LUA_TTABLE )
        {
            // table (of floats)
            float fBuffer[16];
            uint i = 0;

            lua_pushnil ( luaVM );      // Loop through our table, beginning at the first key
            while ( lua_next ( luaVM, argStream.m_iIndex ) != 0 && i < NUMELMS(fBuffer) )
            {
                fBuffer[i++] = static_cast < float > ( lua_tonumber ( luaVM, -1 ) );    // Ignore the index at -2, and just read the value
                lua_pop ( luaVM, 1 );                     // Remove the item and keep the key for the next iteration
            }
            bool bResult = pShader->GetShaderItem ()->SetValue ( strName, fBuffer, i );
            lua_pushboolean ( luaVM, bResult );
            return 1;
        }
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), "Expected number, bool, table or texture at argument 3" ) );
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxSetShaderTessellation ( lua_State* luaVM )
{
//  bool dxSetShaderTessellation( element shader, int tessellationX, int tessellationY )
    CClientShader* pShader; uint uiTessellationX; uint uiTessellationY;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pShader );
    argStream.ReadNumber ( uiTessellationX );
    argStream.ReadNumber ( uiTessellationY );

    if ( !argStream.HasErrors () )
    {
        pShader->GetShaderItem ()->SetTessellation ( uiTessellationX, uiTessellationY );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxSetShaderTransform ( lua_State* luaVM )
{
//  bool dxSetShaderTransform( element shader, lots )
    CClientShader* pShader; SShaderTransform transform;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pShader );

    argStream.ReadNumber ( transform.vecRot.fX, 0 );
    argStream.ReadNumber ( transform.vecRot.fY, 0 );
    argStream.ReadNumber ( transform.vecRot.fZ, 0 );
    argStream.ReadNumber ( transform.vecRotCenOffset.fX, 0 );
    argStream.ReadNumber ( transform.vecRotCenOffset.fY, 0 );
    argStream.ReadNumber ( transform.vecRotCenOffset.fZ, 0 );
    argStream.ReadBool ( transform.bRotCenOffsetOriginIsScreen, false );
    argStream.ReadNumber ( transform.vecPersCenOffset.fX, 0 );
    argStream.ReadNumber ( transform.vecPersCenOffset.fY, 0 );
    argStream.ReadBool ( transform.bPersCenOffsetOriginIsScreen, false );

    if ( !argStream.HasErrors () )
    {
        pShader->GetShaderItem ()->SetTransform ( transform );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxSetRenderTarget ( lua_State* luaVM )
{
//  bool setRenderTaget( element renderTarget [, bool clear = false ] )
    CClientRenderTarget* pRenderTarget; bool bClear;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pRenderTarget, NULL );
    argStream.ReadBool ( bClear, false );

    if ( !argStream.HasErrors () )
    {
        bool bResult;
        if ( pRenderTarget)
            bResult = g_pCore->GetGraphics ()->GetRenderItemManager ()->SetRenderTarget ( pRenderTarget->GetRenderTargetItem (), bClear );
        else
            bResult = g_pCore->GetGraphics ()->GetRenderItemManager ()->RestoreDefaultRenderTarget ();

        if ( bResult )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
        else
            m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad usage @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), "dxSetRenderTarget can only be used inside certain events" ) );
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxUpdateScreenSource ( lua_State* luaVM )
{
//  bool dxUpdateScreenSource( element screenSource [, bool resampleNow] )
    CClientScreenSource* pScreenSource; bool bResampleNow;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadUserData ( pScreenSource );
    argStream.ReadBool ( bResampleNow, false );

    if ( !argStream.HasErrors () )
    {
        g_pCore->GetGraphics ()->GetRenderItemManager ()->UpdateScreenSource ( pScreenSource->GetScreenSourceItem (), bResampleNow );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxCreateFont ( lua_State* luaVM )
{
//  element dxCreateFont( string filepath [, int size=9, bool bold=false ] )
    SString strFilePath; int iSize; bool bBold;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strFilePath );
    argStream.ReadNumber ( iSize, 9 );
    argStream.ReadBool ( bBold, false );

    if ( !argStream.HasErrors () )
    {
        CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
        if ( pLuaMain )
        {
            CResource* pParentResource = pLuaMain->GetResource ();
            CResource* pFileResource = pParentResource;
            SString strPath;
            if ( CResourceManager::ParseResourcePathInput( strFilePath, pFileResource, strPath ) )
            {
                if ( FileExists ( strPath ) )
                {
                    CClientDxFont* pDxFont = g_pClientGame->GetManager ()->GetRenderElementManager ()->CreateDxFont ( strPath, iSize, bBold );
                    if ( pDxFont )
                    {
                        // Make it a child of the resource's file root ** CHECK  Should parent be pFileResource, and element added to pParentResource's ElementGroup? **
                        pDxFont->SetParent ( pParentResource->GetResourceDynamicEntity () );
                    }
                    lua_pushelement ( luaVM, pDxFont );
                    return 1;
                }
                else
                    m_pScriptDebugging->LogBadPointer ( luaVM, "file-path", 1 );
            }
            else
                m_pScriptDebugging->LogBadPointer ( luaVM, "file-path", 1 );
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxSetTestMode ( lua_State* luaVM )
{
//  bool dxSetTestMode( string testMode )
    eDxTestMode testMode;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadEnumString ( testMode, DX_TEST_MODE_NONE );

    if ( !argStream.HasErrors () )
    {
        g_pCore->GetGraphics ()->GetRenderItemManager ()->SetTestMode ( testMode );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxGetStatus ( lua_State* luaVM )
{
//  table dxGetStatus()

    CScriptArgReader argStream ( luaVM );

    if ( !argStream.HasErrors () )
    {
        SDxStatus dxStatus;
        g_pCore->GetGraphics ()->GetRenderItemManager ()->GetDxStatus ( dxStatus );

        lua_createtable ( luaVM, 0, 13 );

        lua_pushstring ( luaVM, "TestMode" );
        lua_pushstring ( luaVM, EnumToString ( dxStatus.testMode ) );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "VideoCardName" );
        lua_pushstring ( luaVM, dxStatus.videoCard.strName );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "VideoCardRAM" );
        lua_pushnumber ( luaVM, dxStatus.videoCard.iInstalledMemoryKB / 1024 );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "VideoCardPSVersion" );
        lua_pushstring ( luaVM, dxStatus.videoCard.strPSVersion );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "VideoMemoryFreeForMTA" );
        lua_pushnumber ( luaVM, dxStatus.videoMemoryKB.iFreeForMTA / 1024 );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "VideoMemoryUsedByFonts" );
        lua_pushnumber ( luaVM, dxStatus.videoMemoryKB.iUsedByFonts / 1024 );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "VideoMemoryUsedByTextures" );
        lua_pushnumber ( luaVM, dxStatus.videoMemoryKB.iUsedByTextures / 1024 );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "VideoMemoryUsedByRenderTargets" );
        lua_pushnumber ( luaVM, dxStatus.videoMemoryKB.iUsedByRenderTargets / 1024 );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "SettingWindowed" );
        lua_pushboolean ( luaVM, dxStatus.settings.bWindowed );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "SettingFXQuality" );
        lua_pushnumber ( luaVM, dxStatus.settings.iFXQuality );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "SettingDrawDistance" );
        lua_pushnumber ( luaVM, dxStatus.settings.iDrawDistance );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "SettingVolumetricShadows" );
        lua_pushboolean ( luaVM, dxStatus.settings.bVolumetricShadows );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "SettingStreamingVideoMemoryForGTA" );
        lua_pushnumber ( luaVM, dxStatus.settings.iStreamingMemory );
        lua_settable   ( luaVM, -3 );

        lua_pushstring ( luaVM, "AllowScreenUpload" );
        lua_pushboolean ( luaVM, dxStatus.settings.bAllowScreenUpload );
        lua_settable   ( luaVM, -3 );

        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxGetTexturePixels ( lua_State* luaVM )
{
//  string dxGetTexturePixels( [ int surfaceIndex, ] element texture [, int x, int y, int width, int height ] )
    CClientTexture* pTexture; int x; int y; int width; int height;
    int surfaceIndex = 0;

    CScriptArgReader argStream ( luaVM );
    if ( argStream.NextIsNumber () )
        argStream.ReadNumber ( surfaceIndex );
    argStream.ReadUserData ( pTexture );
    argStream.ReadNumber ( x, 0 );
    argStream.ReadNumber ( y, 0 );
    argStream.ReadNumber ( width, 0 );
    argStream.ReadNumber ( height, 0 );

    if ( !argStream.HasErrors () )
    {
        RECT rc = { x, y, x + width, y + height };
        CPixels pixels;
        if ( g_pCore->GetGraphics ()->GetPixelsManager ()->GetTexturePixels ( pTexture->GetTextureItem ()->m_pD3DTexture, pixels, height ? &rc : NULL, surfaceIndex ) )
        {
            lua_pushlstring ( luaVM, pixels.GetData (), pixels.GetSize () );
            return 1;           
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxSetTexturePixels ( lua_State* luaVM )
{
//  string dxGetTexturePixels( [ int sufaceIndex, ] element texture, string pixels [, int x, int y, int width, int height ] )
    CClientTexture* pTexture; CPixels pixels; int x; int y; int width; int height;
    int surfaceIndex = 0;

    CScriptArgReader argStream ( luaVM );
    if ( argStream.NextIsNumber () )
        argStream.ReadNumber ( surfaceIndex );
    argStream.ReadUserData ( pTexture );
    argStream.ReadCharStringRef ( pixels.externalData );
    argStream.ReadNumber ( x, 0 );
    argStream.ReadNumber ( y, 0 );
    argStream.ReadNumber ( width, 0 );
    argStream.ReadNumber ( height, 0 );

    if ( !argStream.HasErrors () )
    {
        RECT rc = { x, y, x + width, y + height };
        if ( g_pCore->GetGraphics ()->GetPixelsManager ()->SetTexturePixels ( pTexture->GetTextureItem ()->m_pD3DTexture, pixels, height ? &rc : NULL, surfaceIndex ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;           
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxGetPixelsSize ( lua_State* luaVM )
{
//  int x,y dxGetPixelsSize( string pixels )
    CPixels pixels;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadCharStringRef ( pixels.externalData );

    if ( !argStream.HasErrors () )
    {
        uint uiSizeX;
        uint uiSizeY;
        if ( g_pCore->GetGraphics ()->GetPixelsManager ()->GetPixelsSize ( pixels, uiSizeX, uiSizeY ) )
        {
            lua_pushinteger ( luaVM, uiSizeX );
            lua_pushinteger ( luaVM, uiSizeY );
            return 2;           
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxGetPixelsFormat ( lua_State* luaVM )
{
//  string dxGetPixelsFormat( string pixels )
    CPixels pixels;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadCharStringRef ( pixels.externalData );

    if ( !argStream.HasErrors () )
    {
        EPixelsFormatType format = g_pCore->GetGraphics ()->GetPixelsManager ()->GetPixelsFormat ( pixels );
        if ( format != EPixelsFormat::UNKNOWN )
        {
            lua_pushstring ( luaVM, EnumToString ( format ) );
            return 1;           
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxConvertPixels ( lua_State* luaVM )
{
//  string dxConvertPixels( string pixels, string pixelFormat [, int quality] )
    CPixels pixels; EPixelsFormatType format; int quality;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadCharStringRef ( pixels.externalData );
    argStream.ReadEnumString ( format );
    argStream.ReadNumber ( quality, 80 );

    if ( !argStream.HasErrors () )
    {
        CPixels newPixels;
        if ( g_pCore->GetGraphics ()->GetPixelsManager ()->ChangePixelsFormat ( pixels, newPixels, format, quality ) )
        {
            lua_pushlstring ( luaVM, newPixels.GetData (), newPixels.GetSize () );
            return 1;           
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxGetPixelColor ( lua_State* luaVM )
{
//  int r,g,b,a dxGetPixelColor( string pixels, int x, int y )
    CPixels pixels; int x; int y;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadCharStringRef ( pixels.externalData );
    argStream.ReadNumber ( x );
    argStream.ReadNumber ( y );

    if ( !argStream.HasErrors () )
    {
        SColor color;
        if ( g_pCore->GetGraphics ()->GetPixelsManager ()->GetPixelColor ( pixels, x, y, color ) )
        {
            lua_pushnumber ( luaVM, color.R );
            lua_pushnumber ( luaVM, color.G );
            lua_pushnumber ( luaVM, color.B );
            lua_pushnumber ( luaVM, color.A );
            return 4;           
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxSetPixelColor ( lua_State* luaVM )
{
//  bool dxSetPixelColor( string pixels, int x, int y, int r, int g, int b [, int a] )
    CPixels pixels; int x; int y; SColor color;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadCharStringRef ( pixels.externalData );
    argStream.ReadNumber ( x );
    argStream.ReadNumber ( y );
    argStream.ReadNumber ( color.R );
    argStream.ReadNumber ( color.G );
    argStream.ReadNumber ( color.B );
    argStream.ReadNumber ( color.A, 255 );

    if ( !argStream.HasErrors () )
    {
        if ( g_pCore->GetGraphics ()->GetPixelsManager ()->SetPixelColor ( pixels, x, y, color ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;           
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // error: bad arguments
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxSetBlendMode ( lua_State* luaVM )
{
//  bool dxSetBlendMode ( string blendMode )
    EBlendModeType blendMode;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadEnumString ( blendMode, EBlendMode::BLEND );

    if ( !argStream.HasErrors () )
    {
        g_pCore->GetGraphics ()->SetBlendMode ( blendMode );
        lua_pushboolean ( luaVM, true );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, SString ( "Bad argument @ '%s' [%s]", lua_tostring ( luaVM, lua_upvalueindex ( 1 ) ), *argStream.GetErrorMessage () ) );

    // Failed
    lua_pushboolean ( luaVM, false );
    return 1;
}


int CLuaFunctionDefs::dxGetBlendMode ( lua_State* luaVM )
{
//  string dxGetBlendMode ()
    EBlendModeType blendMode = g_pCore->GetGraphics ()->GetBlendMode ();
    lua_pushstring ( luaVM, EnumToString ( blendMode ) );
    return 1;
}
