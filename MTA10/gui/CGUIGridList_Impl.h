/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        gui/CGUIGridList_Impl.h
*  PURPOSE:     Grid list widget class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Chris McArthur <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGUIGRIDLIST_IMPL_H
#define __CGUIGRIDLIST_IMPL_H

#include <gui/CGUIGridList.h>
#include "CGUIElement_Impl.h"
#include <list>

class CGUIListItem_Impl;

class CGUIGridList_Impl : public CGUIGridList, public CGUIElement_Impl
{
public:
                                        CGUIGridList_Impl       ( class CGUI_Impl* pGUI, CGUIElement* pParent = NULL, bool bFrame = true );
                                        ~CGUIGridList_Impl      ( void );

    unsigned int                        AddColumn               ( const char* szTitle, float fWidth );
	void								RemoveColumn			( unsigned int uiColumn );
	void			                    AutoSizeColumn			( unsigned int hColumn );
    void                                SetColumnWidth          ( int hColumn, float fWidth, bool bRelative = true );

	void								SetSelectionMode		( SelectionMode mode );

    void                                ForceUpdate             ( void );
    int                                 AddRow                  ( bool bFast = false );
	void								RemoveRow				( int iRow );
	int									InsertRowAfter			( int iRow );
    void                                Clear                   ( void );
    CGUIListItem*                       GetItem                 ( int iRow, int hColumn );
    char*                               GetItemText             ( int iRow, int hColumn );
    void                                SetItemText             ( int iRow, int hColumn, const char* szText, bool bNumber = false, bool bSection = false, bool bFast = false );
	void								SetItemData				( int iRow, int hColumn, void* pData );
	void*								GetItemData				( int iRow, int hColumn );

	void								SetHorizontalScrollBar	( bool bEnabled );
	void								SetVerticalScrollBar	( bool bEnabled );
	void								SetSorting				( bool bEnabled );
	void			                    SetItemImage			( int iRow, int hColumn, CGUIStaticImage* pImage );

    int                                 GetColumnIndex          ( int hColumn );
	int									GetItemColumnIndex		( CGUIListItem* pItem );
    int                                 GetItemRowIndex			( CGUIListItem* pItem );
    int                                 GetSelectedCount        ( void );
    CGUIListItem*                       GetSelectedItem         ( void );
    int                                 GetSelectedItemRow      ( void );
	int                                 GetSelectedItemColumn   ( void );
	int									GetRowCount				( void );

    bool                                IsColumnSegmentSizingEnabled     ( int hColumn );
    void                                SetColumnSegmentSizingEnabled    ( int hColumn, bool bEnabled );

    void                                Sort                    ( unsigned int uiColumn, SortDirection direction );

    void                                SetSelectedItem         ( int iRow, int hColumn, bool bReset );

    void                                SetSortColumnHandler	( GUI_CALLBACK Callback );

    void                                SetIgnoreTextSpacer     ( bool bIgnoreTextSpacer ) { m_bIgnoreTextSpacer = bIgnoreTextSpacer; };

	eCGUIType							GetType					( void ) { return CGUI_GRIDLIST; };

    #include "CGUIElement_Inc.h"

private:
    bool                                Event_OnSortColumn      ( const CEGUI::EventArgs& e );

	int				                    m_iIndex;

    unsigned int                        GetUniqueHandle         ( void );
    CGUIListItem_Impl*                  GetListItem             ( CEGUI::ListboxItem* pItem );
    unsigned int                        m_hUniqueHandle;

    std::list < CGUIListItem_Impl* >    m_Items;

    GUI_CALLBACK*                       m_pOnSortColumn;

    bool                                m_bIgnoreTextSpacer;
};

#endif
