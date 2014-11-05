/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CSettings.h
*  PURPOSE:     Header file for in-game settings window class
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Marcus Bauer <mabako@gmail.com>
*               Florian Busse <flobu@gmx.net>
*               Sebas Lamers <sebasdevelopment@gmx.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CSettings;

#ifndef __CSETTINGS_H
#define __CSETTINGS_H

#include <core/CCoreInterface.h>
#include "CMainMenu.h"
#include "CCore.h"

#define SKINS_PATH                    "skins/*"
#define CHAT_PRESETS_PATH             "mta/chatboxpresets.xml"
#define CHAT_PRESETS_ROOT             "chatboxpresets"

//#define SHOWALLSETTINGS

enum eTabIndex
{
    UNKNOWN_TAB,
    TAB_MULTIPLAYER,
    TAB_VIDEO,
    TAB_AUDIO,
    TAB_MICROPHONE,
    TAB_BINDS,
    TAB_CONTROLS,
    TAB_INTERFACE,
    TAB_ADVANCED,
    TAB_COMMUNITY,
};

struct SKeyBindSection
{
    SKeyBindSection ( char * szTitle )
    {
        this->currentIndex = 0;
        this->szTitle = new char [ strlen ( szTitle ) + 1 ];
        this->szOriginalTitle = new char [ strlen ( szTitle ) + 1 ];
        // convert to upper case
        for ( unsigned int i = 0; i < strlen(szTitle); i++ )
        {
            if ( isalpha((uchar)szTitle[i]) )
                this->szTitle[i] = toupper(szTitle[i]);
            else
                this->szTitle[i] = szTitle[i];
        }

        this->szTitle[strlen(szTitle)] = '\0';

        strcpy ( szOriginalTitle, szTitle );

        this->rowCount = 0;

        headerItem = NULL;
    }

    ~SKeyBindSection ( )
    {
        if ( this->szTitle )
            delete[] this->szTitle;

        if ( szOriginalTitle )
            delete[] szOriginalTitle;
    }
    int currentIndex; // temporarily stores the index while the list is being created
    char* szTitle;
    char* szOriginalTitle;
    int rowCount;
    class CGUIListItem * headerItem;
};

class CColor;

namespace ChatColorTypes
{
    enum ChatColorType
    {
        CHAT_COLOR_BG = 0,
        CHAT_COLOR_TEXT,
        CHAT_COLOR_INPUT_BG,
        CHAT_COLOR_INPUT_TEXT,
        CHAT_COLOR_MAX
    };
}

using ChatColorTypes::ChatColorType;

enum
{
    FULLSCREEN_STANDARD,
    FULLSCREEN_BORDERLESS,
    FULLSCREEN_BORDERLESS_KEEP_RES,
    FULLSCREEN_BORDERLESS_STRETCHED,
};


class CSettings
{
    friend class CCore;

public:
                        CSettings               ( void );
                        ~CSettings              ( void );

    void                CreateGUI               ( void );
    void                DestroyGUI              ( void );

    bool                ProcessMessage          ( UINT uMsg, WPARAM wParam, LPARAM lParam );

    void                Update                  ( void );
    void                Initialize              ( void );

    void                SetVisible              ( bool bVisible );
    bool                IsVisible               ( void );

    void                SetIsModLoaded          ( bool bLoaded );

    void                LoadData                ( void );

    inline bool         IsCapturingKey          ( void )            { return m_bCaptureKey; }
    void                UpdateCaptureAxis       ( void );
    void                UpdateJoypadTab         ( void );

    void                UpdateAudioTab          ( void );

    void                UpdateMicrophoneTab     ( void );

    void                UpdateVideoTab          ( void );
    void                PopulateResolutionComboBox( void );

    void                AddKeyBindSection       ( char * szSectionName );
    void                RemoveKeyBindSection    ( char * szSectionName );
    void                RemoveAllKeyBindSections ( void );

    static void         OnLoginCallback         ( bool bResult, char* szError, void* obj );
    void                OnLoginStateChange      ( bool bResult );

    void                RequestNewNickname      ( void );
    void                ShowRestartQuestion     ( void );

    void                TabSkip                 ( bool bBackwards );

    bool                IsActive                ( void );

    void                SetSelectedIndex        ( unsigned int uiIndex );
protected:
    const static int    SecKeyNum = 3;     // Number of secondary keys

    // Keep these protected so we can access them in the event handlers of CClientGame
    CGUIWindow*         m_pWindow;
    CGUITabPanel*       m_pTabs;
    CGUIButton*         m_pButtonOK;
    CGUIButton*         m_pButtonCancel;
    CGUILabel*          m_pLabelNick;
    CGUIEdit*           m_pEditNick;
    CGUICheckBox*       m_pSavePasswords;
    CGUICheckBox*       m_pAutoRefreshBrowser;

    CGUILabel*          m_pVideoGeneralLabel;
    CGUILabel*          m_pVideoResolutionLabel;
    CGUIComboBox*       m_pComboResolution;
	CGUICheckBox*       m_pCheckBoxMipMapping;
    CGUICheckBox*       m_pCheckBoxWindowed;
    CGUICheckBox*       m_pCheckBoxHudMatchAspectRatio;
    CGUICheckBox*       m_pCheckBoxMinimize;
    CGUICheckBox*       m_pCheckBoxDisableAero;
    CGUICheckBox*       m_pCheckBoxDisableDriverOverrides;
    CGUILabel*          m_pMapRenderingLabel;
    CGUIComboBox*       m_pComboFxQuality;
    CGUILabel*          m_pFXQualityLabel;
    CGUIComboBox*       m_pComboAspectRatio;
    CGUILabel*          m_pAspectRatioLabel;
	CGUICheckBox*       m_pCheckBoxVolumetricShadows;
	CGUICheckBox*       m_pCheckBoxDeviceSelectionDialog;
	CGUICheckBox*       m_pCheckBoxShowUnsafeResolutions;
	CGUICheckBox*       m_pCheckBoxAllowScreenUpload;
	CGUICheckBox*       m_pCheckBoxCustomizedSAFiles;
	CGUICheckBox*       m_pCheckBoxGrass;
	CGUICheckBox*       m_pCheckBoxHeatHaze;
    CGUICheckBox*       m_pCheckBoxTyreSmokeParticles;
    CGUILabel*          m_pDrawDistanceLabel;
    CGUIScrollBar*      m_pDrawDistance;
    CGUILabel*          m_pDrawDistanceValueLabel;
    CGUILabel*          m_pBrightnessLabel;
    CGUIScrollBar*      m_pBrightness;
    CGUILabel*          m_pBrightnessValueLabel;
    CGUILabel*          m_pAnisotropicLabel;
    CGUIScrollBar*      m_pAnisotropic;
    CGUILabel*          m_pAnisotropicValueLabel;
    CGUIComboBox*       m_pComboAntiAliasing;
    CGUILabel*          m_pAntiAliasingLabel;
    CGUILabel*          m_pMapAlphaLabel;
    CGUIScrollBar*      m_pMapAlpha;
    CGUILabel*          m_pMapAlphaValueLabel;
    CGUILabel*          m_pStreamingMemoryLabel;
    CGUIScrollBar*      m_pStreamingMemory;
    CGUILabel*          m_pStreamingMemoryMinLabel;
    CGUILabel*          m_pStreamingMemoryMaxLabel;
    CGUILabel*          m_pStreamingMemoryLabelInfo;
    CGUIButton*         m_pVideoDefButton;

    CGUILabel*          m_pAdvancedSettingDescriptionLabel;
    CGUILabel*          m_pFullscreenStyleLabel;
    CGUIComboBox*       m_pFullscreenStyleCombo;
    CGUILabel*          m_pPriorityLabel;
    CGUIComboBox*       m_pPriorityCombo;
    CGUILabel*          m_pFastClothesLabel;
    CGUIComboBox*       m_pFastClothesCombo;
    CGUILabel*          m_pAudioGeneralLabel;
    CGUILabel*          m_pUserTrackGeneralLabel;
    CGUILabel*          m_pBrowserSpeedLabel;
    CGUIComboBox*       m_pBrowserSpeedCombo;
    CGUILabel*          m_pSingleDownloadLabel;
    CGUIComboBox*       m_pSingleDownloadCombo;
    CGUILabel*          m_pDebugSettingLabel;
    CGUIComboBox*       m_pDebugSettingCombo;
    CGUILabel*          m_pWin8Label;
    CGUICheckBox*       m_pWin8ColorCheckBox;
    CGUICheckBox*       m_pWin8MouseCheckBox;
    CGUILabel*          m_pUpdateBuildTypeLabel;
    CGUIComboBox*       m_pUpdateBuildTypeCombo;
    CGUIButton*         m_pButtonUpdate;
    CGUILabel*          m_pAdvancedMiscLabel;
    CGUILabel*          m_pAdvancedUpdaterLabel;

    CGUILabel*          m_pLabelRadioVolume;
    CGUILabel*          m_pLabelSFXVolume;
    CGUILabel*          m_pLabelMTAVolume;
    CGUILabel*          m_pLabelVoiceVolume;
    CGUILabel*          m_pLabelRadioVolumeValue;
    CGUILabel*          m_pLabelSFXVolumeValue;
    CGUILabel*          m_pLabelMTAVolumeValue;
    CGUILabel*          m_pLabelVoiceVolumeValue;
    CGUIScrollBar*      m_pAudioRadioVolume;
    CGUIScrollBar*      m_pAudioSFXVolume;
    CGUIScrollBar*      m_pAudioMTAVolume;
    CGUIScrollBar*      m_pAudioVoiceVolume;
    CGUICheckBox*       m_pCheckBoxAudioEqualizer;
    CGUICheckBox*       m_pCheckBoxAudioAutotune;
    CGUICheckBox*       m_pCheckBoxMuteSFX;
    CGUICheckBox*       m_pCheckBoxMuteRadio;
    CGUICheckBox*       m_pCheckBoxMuteMTA;
    CGUICheckBox*       m_pCheckBoxMuteVoice;
    CGUILabel*          m_pAudioUsertrackLabel;
    CGUICheckBox*       m_pCheckBoxUserAutoscan;
    CGUILabel*          m_pLabelUserTrackMode;
    CGUIComboBox*       m_pComboUsertrackMode;
    CGUIButton*         m_pAudioDefButton;


    CGUIButton*         m_pMicrophoneTestButton;
    CGUIButton*         m_pMicrophoneStopTestButton;
    CGUILabel*          m_pMicrophoneLabel;
    CGUILabel*          m_pMicrophoneDeviceLabel;
    CGUILabel*          m_pMicrophoneQualityLabel;
    CGUILabel*          m_pMicrophoneLocalPlaybackLabel;
    CGUICheckBox*       m_pMicrophoneLocalPlaybackCheckbox;

    CGUIStaticImage*    m_pImageRed;
    CGUIStaticImage*    m_pImageRed2;
    CGUIStaticImage*    m_pImageYellow1;
    CGUIStaticImage*    m_pImageYellow2;
    CGUIStaticImage*    m_pImageYellow3;
    CGUIStaticImage*    m_pImageGreen1;
    CGUIStaticImage*    m_pImageGreen2;
    CGUIStaticImage*    m_pImageGreen3;
    CGUIStaticImage*    m_pImageGreen4;

    CGUITexture *       m_pRedTexture;
    CGUITexture *       m_pYellowTexture;
    CGUITexture *       m_pGreenTexture;
    CGUITexture *       m_pBlankTexture;
    int                 m_iLastCase;

    CGUIComboBox*       m_pDeviceSelection;

    CGUIGridList*       m_pBindsList;
    CGUIButton*         m_pBindsDefButton;
    CGUIHandle          m_hBind, m_hPriKey, m_hSecKeys[SecKeyNum];

    CGUILabel*                  m_pJoypadName;
    CGUILabel*                  m_pJoypadUnderline;
    CGUIEdit*                   m_pEditDeadzone;
    CGUIEdit*                   m_pEditSaturation;
    std::vector < CGUILabel* >  m_pJoypadLabels;
    std::vector < CGUIButton* > m_pJoypadButtons;
    int                         m_JoypadSettingsRevision;

    CGUILabel*          m_pLabelCommunity;
    CGUILabel*          m_pLabelUser;
    CGUILabel*          m_pLabelPass;
    CGUIEdit*           m_pEditUser;
    CGUIEdit*           m_pEditPass;
    CGUIButton*         m_pButtonLogin;
    CGUIButton*         m_pButtonRegister;

    CGUILabel*          m_pControlsMouseLabel;
    CGUICheckBox*       m_pInvertMouse;
    CGUICheckBox*       m_pSteerWithMouse;
    CGUICheckBox*       m_pFlyWithMouse;
    CGUILabel*          m_pLabelMouseSensitivity;
    CGUIScrollBar*      m_pMouseSensitivity;
    CGUILabel*          m_pLabelMouseSensitivityValue;
    CGUILabel*          m_pLabelVerticalAimSensitivity;
    CGUIScrollBar*      m_pVerticalAimSensitivity;
    CGUILabel*          m_pLabelVerticalAimSensitivityValue;

    CGUILabel*          m_pControlsJoypadLabel;
    CGUIScrollPane*     m_pControlsInputTypePane;
    CGUIRadioButton*    m_pStandardControls;
    CGUIRadioButton*    m_pClassicControls;

    CGUIComboBox*       m_pInterfaceLanguageSelector;
    CGUIComboBox*       m_pInterfaceSkinSelector;
    CGUIButton*         m_pInterfaceLoadSkin;

    CGUIComboBox*       m_pChatPresets;
    CGUIButton*         m_pChatLoadPreset;

    CGUIScrollBar*      m_pChatRed          [ ChatColorTypes::CHAT_COLOR_MAX ];
    CGUIScrollBar*      m_pChatGreen        [ ChatColorTypes::CHAT_COLOR_MAX ];
    CGUIScrollBar*      m_pChatBlue         [ ChatColorTypes::CHAT_COLOR_MAX ];
    CGUIScrollBar*      m_pChatAlpha        [ ChatColorTypes::CHAT_COLOR_MAX ];

    CGUILabel*          m_pChatRedValue     [ ChatColorTypes::CHAT_COLOR_MAX ];
    CGUILabel*          m_pChatGreenValue   [ ChatColorTypes::CHAT_COLOR_MAX ];
    CGUILabel*          m_pChatBlueValue    [ ChatColorTypes::CHAT_COLOR_MAX ];
    CGUILabel*          m_pChatAlphaValue   [ ChatColorTypes::CHAT_COLOR_MAX ];

    CGUIScrollPane*     m_pPaneChatFont;
    CGUIRadioButton*    m_pRadioChatFont    [ ChatFonts::CHAT_FONT_MAX ];

    CGUIEdit*           m_pChatLines;
    CGUIEdit*           m_pChatScaleX;
    CGUIEdit*           m_pChatScaleY;
    CGUIEdit*           m_pChatWidth;

    CGUICheckBox*       m_pChatCssBackground;
    CGUICheckBox*       m_pChatCssText;
    CGUIEdit*           m_pChatLineLife;
    CGUIEdit*           m_pChatLineFadeout;

    bool                OnJoypadTextChanged     ( CGUIElement* pElement );
    bool                OnAxisSelectClick       ( CGUIElement* pElement );
    bool                OnAudioDefaultClick     ( CGUIElement* pElement );
    bool                OnMicrophoneTestClick   ( CGUIElement* pElement );
    bool                OnMicrophoneStopTestClick   ( CGUIElement* pElement );
    bool                OnControlsDefaultClick  ( CGUIElement* pElement );
    bool                OnBindsDefaultClick     ( CGUIElement* pElement );
    bool                OnVideoDefaultClick     ( CGUIElement* pElement );
    bool                OnBindsListClick        ( CGUIElement* pElement );
    bool                OnOKButtonClick         ( CGUIElement* pElement );
    bool                OnCancelButtonClick     ( CGUIElement* pElement );
    bool                OnLoginButtonClick      ( CGUIElement* pElement );
    bool                OnRegisterButtonClick   ( CGUIElement* pElement );
    bool                OnDrawDistanceChanged   ( CGUIElement* pElement );
    bool                OnBrightnessChanged     ( CGUIElement* pElement );
    bool                OnAnisotropicChanged    ( CGUIElement* pElement );
    bool                OnMapAlphaChanged       ( CGUIElement* pElement );
    bool                OnRadioVolumeChanged    ( CGUIElement* pElement );
    bool                OnSFXVolumeChanged      ( CGUIElement* pElement );
    bool                OnMTAVolumeChanged      ( CGUIElement* pElement );
    bool                OnVoiceVolumeChanged    ( CGUIElement* pElement );
    bool                OnChatRedChanged        ( CGUIElement* pElement );
    bool                OnChatGreenChanged      ( CGUIElement* pElement );
    bool                OnChatBlueChanged       ( CGUIElement* pElement );
    bool                OnChatAlphaChanged      ( CGUIElement* pElement );
    bool                OnUpdateButtonClick     ( CGUIElement* pElement );
    bool                OnMouseSensitivityChanged ( CGUIElement* pElement );
    bool                OnVerticalAimSensitivityChanged ( CGUIElement* pElement );

    bool                OnMouseDoubleClick              ( CGUIMouseEventArgs Args );

    bool                OnChatLoadPresetClick           ( CGUIElement* pElement );

    bool                OnSkinChanged                   ( CGUIElement* pElement );

    bool                OnFxQualityChanged              ( CGUIElement* pElement );
    bool                OnVolumetricShadowsClick        ( CGUIElement* pElement );
    bool                OnAllowScreenUploadClick        ( CGUIElement* pElement );
    bool                OnCustomizedSAFilesClick        ( CGUIElement* pElement );
    bool                ShowUnsafeResolutionsClick      ( CGUIElement* pElement );
    bool                OnShowAdvancedSettingDescription ( CGUIElement* pElement );
    bool                OnHideAdvancedSettingDescription ( CGUIElement* pElement );
   

    bool                OnAudioInputDeviceChanged       ( CGUIElement * pElement );
    bool                OnLocalPlaybackClick            ( CGUIElement * pElement );
    bool                OnTabChange                     ( CGUIElement * pElement );


private:
    void                ProcessKeyBinds         ( void );
    void                ProcessJoypad           ( void );

    void                SaveData                ( void );

    void                LoadSkins               ( void );

    void                LoadChatPresets         ( void );
    void                CreateChatColorTab      ( ChatColorType eType, const char* szName, CGUITabPanel* pParent );
    void                LoadChatColorFromCVar   ( ChatColorType eType, const char* szCVar );
    void                LoadChatColorFromString ( ChatColorType eType, const std::string& strColor );
    void                SaveChatColor           ( ChatColorType eType, const char* szCVar );
    CColor              GetChatColorValues      ( ChatColorType eType );
    void                SetChatColorValues      ( ChatColorType eType, CColor pColor );
    int                 GetMilliseconds         ( CGUIEdit* pEdit );
    void                SetMilliseconds         ( CGUIEdit* pEdit, int milliseconds );

    unsigned int        m_uiCaptureKey;
    bool                m_bCaptureKey;
    bool                m_bCaptureAxis;

    bool                m_bIsModLoaded;

    unsigned char       m_ucOldRadioVolume;
    unsigned char       m_ucOldSFXVolume;
    float               m_fOldMTAVolume;
    float               m_fOldVoiceVolume;

    CGUIListItem*	    m_pSelectedBind;

    DWORD               m_dwFrameCount;
    bool                m_bShownVolumetricShadowsWarning;
    bool                m_bShownAllowScreenUploadMessage;
    int                 m_iMaxAnisotropic;

    bool                m_bMuteSFX;
    bool                m_bMuteRadio;
    bool                m_bMuteMTA;
    bool                m_bMuteVoice;

    eTabIndex           m_eCurrentTab;

    std::list < SKeyBindSection *> m_pKeyBindSections;

};

#endif
