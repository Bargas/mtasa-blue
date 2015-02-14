/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CWebCore.h
*  PURPOSE:     Webbrowser class
*
*****************************************************************************/

#ifndef __CWEBCORE_H
#define __CWEBCORE_H

#undef GetNextSibling
#undef GetFirstChild
#include <functional>
#include <mutex>
#include <core/CWebCoreInterface.h>
#include <cef3/include/cef_app.h>
#define MTA_BROWSERDATA_PATH "mta/cef/browserdata.xml"
#define BROWSER_LIST_UPDATE_INTERVAL (24*60*60)
#define BROWSER_UPDATE_URL "http://cef.multitheftauto.com/get.php"
#define CEF_ENABLE_SANDBOX 0
#define GetNextSibling(hwnd) GetWindow(hwnd, GW_HWNDNEXT) // Re-define the conflicting macro
#define GetFirstChild(hwnd) GetTopWindow(hwnd)

class CWebBrowserItem;
class CWebsiteRequests;
class CWebView;
struct IAudioSessionManager2;

class CWebCore : public CWebCoreInterface
{
    struct EventEntry
    {
        std::function<void(void)> callback;
        CWebView* pWebView;
    #ifdef MTA_DEBUG
        SString name;
    #endif

        EventEntry(const std::function<void(void)>& callback_, CWebView* pWebView_) : callback(callback_), pWebView(pWebView_) {}
#ifdef MTA_DEBUG
        EventEntry(const std::function<void(void)>& callback_, CWebView* pWebView_, const SString& name_) : callback(callback_), pWebView(pWebView_), name(name_) {}
#endif
    };

public:
    CWebCore();
    ~CWebCore();
    bool                Initialise          ();

    CWebViewInterface*  CreateWebView       ( unsigned int uiWidth, unsigned int uiHeight, bool bIsLocal, CWebBrowserItem* pWebBrowserRenderItem, bool bTransparent );
    void                DestroyWebView      ( CWebViewInterface* pWebViewInterface );
    void                DoPulse             ();
    CWebView*           FindWebView         ( CefRefPtr<CefBrowser> browser );

    void                AddEventToEventQueue( std::function<void(void)> func, CWebView* pWebView, const SString& name );
    void                RemoveWebViewEvents ( CWebView* pWebView );
    void                DoEventQueuePulse   ();
    
    eURLState           GetURLState         ( const SString& strURL );
    void                ResetFilter         ( bool bResetRequestsOnly = true );
    void                InitialiseWhiteAndBlacklist ( bool bAddHardcoded = true, bool bAddDynamic = true );
    void                AddAllowedPage      ( const SString& strURL, eWebFilterType filterType );
    void                AddBlockedPage      ( const SString& strURL, eWebFilterType filterType );
    void                RequestPages        ( const std::vector<SString>& pages );
    void                AllowPendingPages   ( bool bRemember );
    void                DenyPendingPages    ();

    inline bool         IsTestModeEnabled   () { return m_bTestmodeEnabled; };
    inline void         SetTestModeEnabled  ( bool bEnabled ) { m_bTestmodeEnabled = bEnabled; };

    inline CWebViewInterface* GetFocusedWebView () { return (CWebViewInterface*) m_pFocusedWebView; };
    inline void         SetFocusedWebView   ( CWebView* pWebView ) { m_pFocusedWebView = pWebView; };
    void                ProcessInputMessage ( UINT uMsg, WPARAM wParam, LPARAM lParam );
    void                ClearTextures       ();

    bool                CanLoadRemotePages  ();

    void                OnPreScreenshot     ();
    void                OnPostScreenshot    ();

    bool                InitialiseCoreAudio ();
    bool                SetGlobalAudioVolume( float fVolume );

    bool                UpdateListsFromMaster();
    bool                MakeSureXMLNodesExist();
    void                LoadListsFromXML     ( bool bWhitelist, bool bBlacklist, bool bCustomLists );
    void                WriteCustomList     ( const SString& strListName, const std::vector<SString>& customList, bool bReset = true );
    void                GetFilterEntriesByType( std::vector<std::pair<SString, bool>>& outEntries, eWebFilterType filterType, eWebFilterState state = eWebFilterState::WEBFILTER_ALL );
    static bool         StaticFetchRevisionProgress  ( double dDownloadNow, double dDownloadTotal, char* pCompletedData, size_t completedLength, void *pObj, bool bComplete, int iError );
    static bool         StaticFetchWhitelistProgress ( double dDownloadNow, double dDownloadTotal, char* pCompletedData, size_t completedLength, void *pObj, bool bComplete, int iError );
    static bool         StaticFetchBlacklistProgress ( double dDownloadNow, double dDownloadTotal, char* pCompletedData, size_t completedLength, void *pObj, bool bComplete, int iError );

private:
    typedef std::pair<bool, eWebFilterType> WebFilterPair;

    CWebsiteRequests*                       m_pRequestsGUI;
    std::list<CefRefPtr<CWebView>>          m_WebViews;
    bool                                    m_bTestmodeEnabled;
    CWebView*                               m_pFocusedWebView;
    HCURSOR                                 m_aCursors[16];

    std::list<EventEntry>                   m_EventQueue;
    std::mutex                              m_EventQueueMutex;

    CFastHashMap<SString, WebFilterPair>    m_Whitelist;
    std::vector<SString>                    m_PendingRequests;
    std::recursive_mutex                    m_FilterMutex;

    IAudioSessionManager2*                  m_pAudioSessionManager;

    CXMLFile*                               m_pXmlConfig;
    int                                     m_iWhitelistRevision;
    int                                     m_iBlacklistRevision;
};

class CCefApp : public CefApp, public CefSchemeHandlerFactory
{
public:
    virtual void OnRegisterCustomSchemes ( CefRefPtr<CefSchemeRegistrar> registrar ) override;

    // CefSchemeHandlerFactory methods
    virtual CefRefPtr<CefResourceHandler> Create ( CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request ) override;

    IMPLEMENT_REFCOUNTING(CCefApp);
};

#endif
