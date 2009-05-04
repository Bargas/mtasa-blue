/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        gui/CGUIComboBox_Impl.h
*  PURPOSE:     Combobox widget class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Stanislav Bobrov <lil_Toady@hotmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using namespace google;

#define CGUICOMBOBOX_NAME "CGUI/Combobox"

CGUIComboBox_Impl::CGUIComboBox_Impl ( CGUI_Impl* pGUI, CGUIElement* pParent, const char* szCaption )
{
	m_pManager = pGUI;
    m_Items.set_deleted_key ( (CEGUI::ListboxItem *)0xFFFFFFFF );

	// Get an unique identifier for CEGUI (gah, there's gotta be an another way)
    char szUnique [CGUI_CHAR_SIZE];
    pGUI->GetUniqueName ( szUnique );

    // Create the window and set default settings
    m_pWindow = pGUI->GetWindowManager ()->createWindow ( CGUICOMBOBOX_NAME, szUnique );
    m_pWindow->setDestroyedByParent ( false );
    m_pWindow->setText ( szCaption );
	m_pWindow->setSize ( CEGUI::Absolute, CEGUI::Size ( 128.0f, 24.0f ) );
	m_pWindow->setVisible ( true );

	// Store the pointer to this CGUI element in the CEGUI element
	m_pWindow->setUserData ( reinterpret_cast < void* > ( this ) );

    AddEvents ();

    // If a parent is specified, add it to it's children list, if not, add it as a child to the pManager
    if ( pParent )
    {
        SetParent ( pParent );
    }
    else
    {
        pGUI->AddChild ( this );
		SetParent ( NULL );
    }
}


CGUIComboBox_Impl::~CGUIComboBox_Impl ( void )
{
    Clear ();
    DestroyElement ();
}


CGUIListItem* CGUIComboBox_Impl::AddItem ( const char* szText )
{
	CGUIListItem_Impl* pNewItem = new CGUIListItem_Impl ( szText, CGUIListItem_Impl::Type::TextItem, NULL );
    CEGUI::ListboxItem* pListboxItem = pNewItem->GetListItem ();
    reinterpret_cast < CEGUI::Combobox* > ( m_pWindow ) -> addItem ( pListboxItem );
    m_Items [ pNewItem->GetListItem () ] = pNewItem;
    return pNewItem;
}


CGUIListItem* CGUIComboBox_Impl::GetSelectedItem ( void )
{
    return GetListItem ( reinterpret_cast < CEGUI::Combobox* > ( m_pWindow ) -> getSelectedItem () );
}


void CGUIComboBox_Impl::Clear ( void )
{
    reinterpret_cast < CEGUI::Combobox* > ( m_pWindow ) -> getDropList () -> resetList ();

    sparse_hash_map < CEGUI::ListboxItem*, CGUIListItem_Impl* >::iterator it;
    for ( it = m_Items.begin (); it != m_Items.end (); it++ )
    {
        delete it->second;
    }

    m_Items.clear ();
}

void CGUIComboBox_Impl::SetReadOnly ( bool bReadonly )
{
    reinterpret_cast < CEGUI::Combobox* > ( m_pWindow ) -> setReadOnly ( bReadonly );
}


CGUIListItem_Impl* CGUIComboBox_Impl::GetListItem ( CEGUI::ListboxItem* pItem )
{
    sparse_hash_map < CEGUI::ListboxItem*, CGUIListItem_Impl* >::iterator it;
    it = m_Items.find ( pItem );
    if ( it == m_Items.end () )
        return NULL;

    return it->second;
}