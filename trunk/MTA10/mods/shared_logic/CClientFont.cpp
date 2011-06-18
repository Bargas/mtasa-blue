/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientFont.cpp
*  PURPOSE:     Custom font bucket
*  DEVELOPERS:  qwerty
*
*****************************************************************************/

#include <StdInc.h>


////////////////////////////////////////////////////////////////
//
// CClientFont::CClientFont
//
//
//
////////////////////////////////////////////////////////////////
CClientFont::CClientFont ( CClientManager* pManager, ElementID ID, CFontItem* pFontItem ) : ClassInit ( this ), CClientRenderElement ( pManager, ID )
{
    SetTypeName ( "font" );
    m_pRenderItem = pFontItem;
}


////////////////////////////////////////////////////////////////
//
// CClientFont::~CClientFont
//
//
//
////////////////////////////////////////////////////////////////
CClientFont::~CClientFont ( void )
{
    Unlink ();
}


////////////////////////////////////////////////////////////////
//
// CClientFont::Unlink
//
// Remove from manager lists
//
////////////////////////////////////////////////////////////////
void CClientFont::Unlink ( void )
{
    // Make sure GUI elements are not using us
    while ( m_GUIElementUserList.size () )
        (*m_GUIElementUserList.begin ())->SetFont ( "", NULL );

    CClientRenderElement::Unlink ();
}


////////////////////////////////////////////////////////////////
//
// CClientFont::GetGUIFontName
//
// Get name CEGUI uses for this custom font
//
////////////////////////////////////////////////////////////////
const SString& CClientFont::GetGUIFontName ( void )
{
    return GetFontItem ()->m_strCEGUIFontName;
}


////////////////////////////////////////////////////////////////
//
// CClientFont::GetDXFont
//
// Get DXFont for this custom font
//
////////////////////////////////////////////////////////////////
ID3DXFont* CClientFont::GetDXFont ( float fScaleX, float fScaleY )
{
    if ( fScaleX > 1.1f || fScaleY > 1.1f )
        return GetFontItem ()->m_pFntBig;
    return GetFontItem ()->m_pFntNormal;
}


////////////////////////////////////////////////////////////////
//
// CClientFont::NotifyGUIElementAttach
//
// Keep track of GUI elements using this font
//
////////////////////////////////////////////////////////////////
void CClientFont::NotifyGUIElementAttach ( CClientGUIElement* pGUIElement )
{
    assert ( !MapContains ( m_GUIElementUserList, pGUIElement ) );
    MapInsert ( m_GUIElementUserList, pGUIElement );
}


////////////////////////////////////////////////////////////////
//
// CClientFont::NotifyGUIElementDetach
//
// Keep track of GUI elements using this font
//
////////////////////////////////////////////////////////////////
void CClientFont::NotifyGUIElementDetach ( CClientGUIElement* pGUIElement )
{
    assert ( MapContains ( m_GUIElementUserList, pGUIElement ) );
    MapRemove ( m_GUIElementUserList, pGUIElement );
}
