/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        gui/CGUI_Impl.h
*  PURPOSE:     Graphical User Interface module class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CGUI_Impl;

#ifndef __CGUI_IMPL_H
#define __CGUI_IMPL_H

#include <gui/CGUI.h>
#include <list>
#include <windows.h>

#define CGUI_CHAR_SIZE 6

class CGUIElement;
class CGUIElement_Impl;
class CGUIButton;
class CGUICheckBox;
class CGUIEdit;
class CGUIEvent;
class CGUIFont;
class CGUIFont_Impl;
class CGUIGridList;
class CGUILabel;
class CGUIMemo;
class CGUIMessageBox;
class CGUIProgressBar;
class CGUIRadioButton;
class CGUIStaticImage;
class CGUIScrollBar;
class CGUIScrollPane;
class CGUIComboBox;
class CGUITexture;
class CGUIWindow;
class CGUITab;
class CGUITabPanel;
struct IDirect3DDevice9;

namespace CEGUI {
    class FontManager;
    class ImagesetManager;
    class Renderer;
    class System;
    class SchemeManager;
    class WindowManager;
    class Image;
    class EventArgs;
    class GUISheet;
    typedef GUISheet DefaultWindow;
}

class CGUI_Impl : public CGUI, public CGUITabList
{
public:
                                    CGUI_Impl                   ( IDirect3DDevice9* pDevice );
                                    ~CGUI_Impl                  ( void );

    void                            Draw                        ( void );
    void                            Invalidate                  ( void );
    void                            Restore                     ( void );

    void                            DrawMouseCursor             ( void );

    void                            ProcessMouseInput           ( CGUIMouseInput eMouseInput, unsigned long ulX = 0, unsigned long ulY = 0, CGUIMouseButton eMouseButton = NoButton );
    void                            ProcessKeyboardInput        ( unsigned long ulKey, bool bIsDown );
    void                            ProcessCharacter            ( unsigned long ulCharacter );

	//
	void							SetGUIInputEnabled			( bool bEnabled );
	bool							GetGUIInputEnabled			( void );

	//
	CGUIMessageBox*					CreateMessageBox			( const char* szTitle, const char* szMessage, unsigned int uiFlags );

	CGUIButton*						CreateButton				( CGUIElement* pParent = NULL, const char* szCaption = "" );
	CGUIButton*						CreateButton				( CGUITab* pParent = NULL, const char* szCaption = "" );

	CGUICheckBox*					CreateCheckBox				( CGUIElement* pParent = NULL, const char* szCaption = "", bool bChecked = false );
	CGUICheckBox*					CreateCheckBox				( CGUITab* pParent = NULL, const char* szCaption = "", bool bChecked = false );

	CGUIRadioButton*				CreateRadioButton			( CGUIElement* pParent = NULL, const char* szCaption = "" );
	CGUIRadioButton*				CreateRadioButton			( CGUITab* pParent = NULL, const char* szCaption = "" );

	CGUIEdit*                       CreateEdit                  ( CGUIElement* pParent = NULL, const char* szText = "" );
	CGUIEdit*                       CreateEdit                  ( CGUITab* pParent = NULL, const char* szText = "" );

    CGUIGridList*                   CreateGridList              ( CGUIElement* pParent = NULL, bool bFrame = true );
    CGUIGridList*                   CreateGridList              ( CGUITab* pParent = NULL, bool bFrame = true );

    CGUILabel*                      CreateLabel                 ( CGUIElement* pParent, const char* szCaption = "" );
	CGUILabel*                      CreateLabel                 ( CGUITab* pParent, const char* szCaption = "" );
	CGUILabel*                      CreateLabel                 ( const char* szCaption = "" );

    CGUIProgressBar*                CreateProgressBar           ( CGUIElement* pParent );
	CGUIProgressBar*                CreateProgressBar           ( CGUITab* pParent );

	CGUIMemo*                       CreateMemo                  ( CGUIElement* pParent = NULL, const char* szText = "" );
	CGUIMemo*                       CreateMemo                  ( CGUITab* pParent = NULL, const char* szText = "" );

    CGUIStaticImage*                CreateStaticImage           ( CGUIElement* pParent );
	CGUIStaticImage*                CreateStaticImage           ( CGUITab* pParent );
	CGUIStaticImage*                CreateStaticImage           ( CGUIGridList* pParent );
	CGUIStaticImage*                CreateStaticImage           ( void );

    CGUITabPanel*                   CreateTabPanel              ( CGUIElement* pParent = NULL );
	CGUITabPanel*                   CreateTabPanel              ( CGUITab* pParent = NULL );

    CGUIScrollPane*                 CreateScrollPane            ( CGUIElement* pParent = NULL );
	CGUIScrollPane*                 CreateScrollPane            ( CGUITab* pParent = NULL );

    CGUIScrollBar*                  CreateScrollBar             ( bool bHorizontal, CGUIElement* pParent = NULL );
	CGUIScrollBar*                  CreateScrollBar             ( bool bHorizontal, CGUITab* pParent = NULL );

    CGUIComboBox*					CreateComboBox				( CGUIElement* pParent = NULL, const char* szCaption = "" );
	CGUIComboBox*					CreateComboBox				( CGUIComboBox* pParent = NULL, const char* szCaption = "" );

	CGUIWindow*                     CreateWnd                   ( CGUIElement* pParent = NULL, const char* szCaption = "" );
	//

    CGUITexture*                    CreateTexture               ( void );
    CGUIFont*                       CreateFnt                   ( const char* szFontName, const char* szFontFile, unsigned int uSize = 8, unsigned int uFlags = 0, unsigned int uExtraGlyphs[] = 0, bool bAutoScale = false );

	void                            SetCursorEnabled            ( bool bEnabled );
    bool                            IsCursorEnabled             ( void );

    void                            AddChild                    ( CGUIElement_Impl* pChild );
    CEGUI::FontManager*             GetFontManager              ( void );
    CEGUI::ImagesetManager*         GetImageSetManager          ( void );
    CEGUI::Renderer*                GetRenderer                 ( void );
    CEGUI::System*                  GetGUISystem                ( void );
    CEGUI::SchemeManager*           GetSchemeManager            ( void );
    CEGUI::WindowManager*           GetWindowManager            ( void );
    void                            GetUniqueName               ( char* pBuf );

	CVector2D						GetResolution				( void );
	void							SetResolution				( float fWidth, float fHeight );

	CGUIFont*						GetDefaultFont				( void );
	CGUIFont*						GetSmallFont				( void );
	CGUIFont*						GetBoldFont					( void );
	CGUIFont*						GetClearFont				( void );
	CGUIFont*						GetSAHeaderFont				( void );
	CGUIFont*						GetSAGothicFont				( void );
	CGUIFont*						GetSansFont					( void );

	void							SetWorkingDirectory			( const char * szDir );
	inline const char*				GetWorkingDirectory			( void )	{ return const_cast < const char* > ( m_szWorkingDirectory ); }

   	const GUI_CALLBACK_KEY&			GetCharacterKeyHandler		( void )									{ return m_CharacterKeyHandler; }
	const GUI_CALLBACK_KEY&			GetKeyDownHandler			( void )									{ return m_KeyDownHandler; }
	const GUI_CALLBACK_MOUSE&		GetMouseClickHandler		( void )									{ return m_MouseClickHandler; }
	const GUI_CALLBACK_MOUSE&		GetMouseDoubleClickHandler	( void )									{ return m_MouseDoubleClickHandler; }
	const GUI_CALLBACK_MOUSE&		GetMouseMoveHandler			( void )									{ return m_MouseMoveHandler; }
	const GUI_CALLBACK_MOUSE&		GetMouseEnterHandler		( void )									{ return m_MouseEnterHandler; }
	const GUI_CALLBACK_MOUSE&		GetMouseLeaveHandler		( void )									{ return m_MouseLeaveHandler; }
	const GUI_CALLBACK_MOUSE&		GetMouseWheelHandler		( void )									{ return m_MouseWheelHandler; }
	const GUI_CALLBACK&				GetMovedHandler				( void )									{ return m_MovedHandler; }
	const GUI_CALLBACK&				GetSizedHandler				( void )									{ return m_SizedHandler; }

	void							SetCharacterKeyHandler		( void )									{ m_CharacterKeyHandler = GUI_CALLBACK_KEY (); }
	void							SetKeyDownHandler			( void )									{ m_KeyDownHandler = GUI_CALLBACK_KEY (); }
	void							SetMouseClickHandler		( void )									{ m_MouseClickHandler = GUI_CALLBACK_MOUSE (); }
	void							SetMouseDoubleClickHandler	( void )									{ m_MouseDoubleClickHandler = GUI_CALLBACK_MOUSE (); }
	void							SetMouseMoveHandler			( void )									{ m_MouseMoveHandler = GUI_CALLBACK_MOUSE (); }
	void							SetMouseEnterHandler		( void )									{ m_MouseEnterHandler = GUI_CALLBACK_MOUSE (); }
	void							SetMouseLeaveHandler		( void )									{ m_MouseLeaveHandler = GUI_CALLBACK_MOUSE (); }
	void							SetMouseWheelHandler		( void )									{ m_MouseWheelHandler = GUI_CALLBACK_MOUSE (); }
	void							SetMovedHandler				( void )									{ m_MovedHandler = GUI_CALLBACK (); }
	void							SetSizedHandler				( void )									{ m_SizedHandler = GUI_CALLBACK (); }

    void							SetCharacterKeyHandler		( const GUI_CALLBACK_KEY & Callback )       { m_CharacterKeyHandler = Callback; }
	void							SetKeyDownHandler			( const GUI_CALLBACK_KEY & Callback )		{ m_KeyDownHandler = Callback; }
	void							SetMouseClickHandler		( const GUI_CALLBACK_MOUSE & Callback )		{ m_MouseClickHandler = Callback; }
	void							SetMouseDoubleClickHandler	( const GUI_CALLBACK_MOUSE & Callback )		{ m_MouseDoubleClickHandler = Callback; }
	void							SetMouseMoveHandler			( const GUI_CALLBACK_MOUSE & Callback )		{ m_MouseMoveHandler = Callback; }
	void							SetMouseEnterHandler		( const GUI_CALLBACK_MOUSE & Callback )		{ m_MouseEnterHandler = Callback; }
	void							SetMouseLeaveHandler		( const GUI_CALLBACK_MOUSE & Callback )		{ m_MouseLeaveHandler = Callback; }
	void							SetMouseWheelHandler		( const GUI_CALLBACK_MOUSE & Callback )		{ m_MouseWheelHandler = Callback; }
	void							SetMovedHandler				( const GUI_CALLBACK & Callback )			{ m_MovedHandler = Callback; }
	void							SetSizedHandler				( const GUI_CALLBACK & Callback )			{ m_SizedHandler = Callback; }

    bool                            IsTransferBoxVisible        ( void )                                    { return m_bTransferBoxVisible; };
    void                            SetTransferBoxVisible       ( bool bVisible )                           { m_bTransferBoxVisible = bVisible; };

	bool							Event_CharacterKey			( const CEGUI::EventArgs& e );
	bool							Event_KeyDown				( const CEGUI::EventArgs& e );
	bool							Event_MouseClick			( const CEGUI::EventArgs& e );
	bool							Event_MouseDoubleClick		( const CEGUI::EventArgs& e );
	bool							Event_MouseWheel			( const CEGUI::EventArgs& e );
	bool							Event_MouseMove				( const CEGUI::EventArgs& e );
	bool							Event_MouseEnter			( const CEGUI::EventArgs& e );
	bool							Event_MouseLeave			( const CEGUI::EventArgs& e );
	bool							Event_Moved					( const CEGUI::EventArgs& e );
	bool							Event_Sized					( const CEGUI::EventArgs& e );
    bool                            Event_RedrawRequested       ( const CEGUI::EventArgs& e );

    void                            AddToRedrawQueue            ( CGUIElement* pWindow );
    void                            RemoveFromRedrawQueue       ( CGUIElement* pWindow );

    void                            CleanDeadPool               ( void );

private:
    CGUIButton*                     _CreateButton               ( CGUIElement_Impl* pParent = NULL, const char* szCaption = "" );
    CGUICheckBox*                   _CreateCheckBox             ( CGUIElement_Impl* pParent = NULL, const char* szCaption = "", bool bChecked = false );
    CGUIRadioButton*                _CreateRadioButton          ( CGUIElement_Impl* pParent = NULL, const char* szCaption = "" );
    CGUIEdit*                       _CreateEdit                 ( CGUIElement_Impl* pParent = NULL, const char* szText = "" );
    CGUIGridList*                   _CreateGridList             ( CGUIElement_Impl* pParent = NULL, bool bFrame = true );
    CGUILabel*                      _CreateLabel                ( CGUIElement_Impl* pParent = NULL, const char* szCaption = "" );
	CGUIProgressBar*				_CreateProgressBar			( CGUIElement_Impl* pParent = NULL );
    CGUIMemo*                       _CreateMemo                 ( CGUIElement_Impl* pParent = NULL, const char* szText = "" );
    CGUIStaticImage*                _CreateStaticImage          ( CGUIElement_Impl* pParent = NULL );
    CGUITabPanel*                   _CreateTabPanel             ( CGUIElement_Impl* pParent = NULL );
	CGUIScrollPane*                 _CreateScrollPane           ( CGUIElement_Impl* pParent = NULL );
	CGUIScrollBar*                  _CreateScrollBar            ( bool bHorizontal, CGUIElement_Impl* pParent = NULL );
    CGUIComboBox*                   _CreateComboBox             ( CGUIElement_Impl* pParent = NULL, const char* szCaption = "" );

    IDirect3DDevice9*               m_pDevice;

    CEGUI::Renderer*                m_pRenderer;
    CEGUI::System*                  m_pSystem;
    CEGUI::FontManager*             m_pFontManager;
    CEGUI::ImagesetManager*         m_pImageSetManager;
    CEGUI::SchemeManager*           m_pSchemeManager;
    CEGUI::WindowManager*           m_pWindowManager;

    CEGUI::DefaultWindow*           m_pTop;
    const CEGUI::Image*             m_pCursor;

    CGUIFont_Impl*                  m_pDefaultFont;
	CGUIFont_Impl*					m_pSmallFont;
	CGUIFont_Impl*					m_pBoldFont;
	CGUIFont_Impl*					m_pClearFont;
	CGUIFont_Impl*					m_pSAHeaderFont;
	CGUIFont_Impl*					m_pSAGothicFont;
	CGUIFont_Impl*					m_pSansFont;
                
    std::list < CGUIElement* >      m_RedrawQueue;

	unsigned long                   m_ulPreviousUnique;

	bool							m_bSwitchGUIInput;

	GUI_CALLBACK_KEY                m_CharacterKeyHandler;
    GUI_CALLBACK_KEY                m_KeyDownHandler;
	GUI_CALLBACK_MOUSE              m_MouseClickHandler;
	GUI_CALLBACK_MOUSE              m_MouseDoubleClickHandler;
	GUI_CALLBACK_MOUSE				m_MouseMoveHandler;
	GUI_CALLBACK_MOUSE				m_MouseEnterHandler;
	GUI_CALLBACK_MOUSE  			m_MouseLeaveHandler;
    GUI_CALLBACK_MOUSE				m_MouseWheelHandler;
	GUI_CALLBACK					m_MovedHandler;
	GUI_CALLBACK					m_SizedHandler;

	char							m_szWorkingDirectory [ MAX_PATH + 1 ];

    bool                            m_bTransferBoxVisible;
};

#endif
