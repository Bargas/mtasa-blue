/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:
*  PURPOSE:
*  DEVELOPERS:
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "CSequencer.h"

// Common command line keys
#define INSTALL_STAGE       "install_stage"
#define INSTALL_LOCATION    "install_loc"
#define HIDE_PROGRESS       "hide_prog"

typedef CBadLang < class CInstallManager > CSequencerType;

class CInstallManager
{
public:
    void            SetMTASAPathSource              ( const SString& strCommandLineIn );
    void            InitSequencer                   ( void );
    SString         Continue                        ( /*const SString& strCommandLine*/ );
    void            RestoreSequencerFromSnapshot    ( const SString& strText );
    SString         GetSequencerSnapshot            ( void );
    SString         GetLauncherPathFilename         ( void );
    bool            UpdateOptimusSymbolExport       ( void );
    SString         MaybeRenameExe                  ( const SString& strGTAPath );

protected:
    SString         _ShowCrashFailDialog            ( void );
    SString         _CheckOnRestartCommand          ( void );
    SString         _MaybeSwitchToTempExe           ( void );
    SString         _SwitchBackFromTempExe          ( void );
    SString         _InstallFiles                   ( void );
    SString         _ChangeToAdmin                  ( void );
    SString         _ShowCopyFailDialog             ( void );
    SString         _ProcessLayoutChecks            ( void );
    SString         _ProcessLangFileChecks          ( void );
    SString         _ProcessExePatchChecks          ( void );
    SString         _ProcessServiceChecks           ( void );
    SString         _ProcessAppCompatChecks         ( void );
    SString         _ChangeFromAdmin                ( void );
    SString         _InstallNewsItems               ( void );
    SString         _Quit                           ( void );

    CSequencerType*     m_pSequencer;
    SString             m_strAdminReason;
};

CInstallManager* GetInstallManager ( void );
