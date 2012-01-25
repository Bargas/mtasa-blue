/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CPerfStat.FunctionTiming.cpp
*  PURPOSE:     Performance stats
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#define DEFAULT_THRESH_MS 1

namespace
{
    //
    // CPerfStatFunctionTiming helpers
    //
    struct STiming
    {
        uint uiNumCalls;
        float fTotalMs;
        float fPeakMs;
    };

    struct SFunctionTimingInfo
    {
        STiming now5s;
        STiming prev60s;
        int iPrevIndex;
        STiming history[12];
    };


    //
    // Keep track of a set of values over a period of time
    //
    class CValueHistory
    {
    public:
        std::map < int, CTickCount > historyMap;

        void AddValue ( int iValue )
        {
            MapSet ( historyMap, iValue, CTickCount::Now () );
        }

        void RemoveOlderThan ( int iValue )
        {
            CTickCount now = CTickCount::Now ();
            for ( std::map < int, CTickCount >::iterator iter = historyMap.begin () ; iter != historyMap.end () ; )
            {
                if ( ( now - iter->second ).ToLongLong () > iValue )
                    historyMap.erase ( iter++ );
                else
                    ++iter;
            }
        }

        int GetLowestValue ( int iDefault )
        {
            if ( historyMap.empty () )
                return iDefault;
            return historyMap.begin()->first;
        }
    };

}


///////////////////////////////////////////////////////////////
//
// CPerfStatFunctionTimingImpl
//
//
//
///////////////////////////////////////////////////////////////
class CPerfStatFunctionTimingImpl : public CPerfStatFunctionTiming
{
public:
    ZERO_ON_NEW
                                CPerfStatFunctionTimingImpl  ( void );
    virtual                     ~CPerfStatFunctionTimingImpl ( void );

    // CPerfStatModule
    virtual const SString&      GetCategoryName         ( void );
    virtual void                DoPulse                 ( void );
    virtual void                GetStats                ( CPerfStatResult* pOutResult, const std::map < SString, int >& optionMap, const SString& strFilter );

    // CPerfStatFunctionTiming
    virtual void                UpdateTiming            ( const char* szFunctionName, TIMEUS timeUs );

    // CPerfStatFunctionTimingImpl functions
    void                        SetActive               ( bool bActive );

    SString                                     m_strCategoryName;
    CElapsedTime                                m_TimeSinceLastViewed;
    bool                                        m_bIsActive;
    CValueHistory                               m_PeakUsRequiredHistory;
    TIMEUS                                      m_PeakUsThresh;

    CElapsedTime                                m_TimeSinceUpdate;
    std::map < SString, SFunctionTimingInfo >   m_TimingMap;
};


///////////////////////////////////////////////////////////////
//
// Temporary home for global object
//
//
//
///////////////////////////////////////////////////////////////
static CPerfStatFunctionTimingImpl* g_pPerfStatFunctionTimingImp = NULL;

CPerfStatFunctionTiming* CPerfStatFunctionTiming::GetSingleton ()
{
    if ( !g_pPerfStatFunctionTimingImp )
        g_pPerfStatFunctionTimingImp = new CPerfStatFunctionTimingImpl ();
    return g_pPerfStatFunctionTimingImp;
}


///////////////////////////////////////////////////////////////
//
// CPerfStatFunctionTimingImpl::CPerfStatFunctionTimingImpl
//
//
//
///////////////////////////////////////////////////////////////
CPerfStatFunctionTimingImpl::CPerfStatFunctionTimingImpl ( void )
{
    m_strCategoryName = "Function timing";
}


///////////////////////////////////////////////////////////////
//
// CPerfStatFunctionTimingImpl::CPerfStatFunctionTimingImpl
//
//
//
///////////////////////////////////////////////////////////////
CPerfStatFunctionTimingImpl::~CPerfStatFunctionTimingImpl ( void )
{
}


///////////////////////////////////////////////////////////////
//
// CPerfStatFunctionTimingImpl::GetCategoryName
//
//
//
///////////////////////////////////////////////////////////////
const SString& CPerfStatFunctionTimingImpl::GetCategoryName ( void )
{
    return m_strCategoryName;
}


///////////////////////////////////////////////////////////////
//
// CPerfStatFunctionTimingImpl::DoPulse
//
//
//
///////////////////////////////////////////////////////////////
void CPerfStatFunctionTimingImpl::DoPulse ( void )
{
    // Maybe turn off stats gathering if nobody is watching
    if ( m_bIsActive && m_TimeSinceLastViewed.Get () > 15000 )
        SetActive ( false );

    // Do nothing if not active
    if ( !m_bIsActive )
    {
        m_TimingMap.empty ();
        return;
    }

    // Check if time to cycle the stats
    if ( m_TimeSinceUpdate.Get () >= 5000 )
    {
        m_TimeSinceUpdate.Reset ();

        // For each timed function
        for ( std::map < SString, SFunctionTimingInfo >::iterator iter = m_TimingMap.begin () ; iter != m_TimingMap.end () ; )
        {
            SFunctionTimingInfo& item = iter->second;
            // Update history
            item.iPrevIndex = ( item.iPrevIndex + 1 ) % NUMELMS( item.history );
            item.history[ item.iPrevIndex ] = item.now5s;

            // Reset accumulator
            item.now5s.uiNumCalls = 0;
            item.now5s.fTotalMs = 0;
            item.now5s.fPeakMs = 0;

            // Recalculate last 60 second stats
            item.prev60s.uiNumCalls = 0;
            item.prev60s.fTotalMs = 0;
            item.prev60s.fPeakMs = 0;
            for ( uint i = 0 ; i < NUMELMS( item.history ) ; i++ )
            {
                const STiming& slot = item.history[i];
                item.prev60s.uiNumCalls += slot.uiNumCalls;
                item.prev60s.fTotalMs += slot.fTotalMs;
                item.prev60s.fPeakMs = Max ( item.prev60s.fPeakMs, slot.fPeakMs );
            }

            // Remove from map if no calls in the last 60s
            if ( item.prev60s.uiNumCalls == 0 )
                m_TimingMap.erase ( iter++ );
            else
                ++iter;
        }
    }

    //
    // Update PeakUs threshold
    //
    m_PeakUsRequiredHistory.RemoveOlderThan ( 10000 );
    m_PeakUsThresh = m_PeakUsRequiredHistory.GetLowestValue ( DEFAULT_THRESH_MS * 1000 );
}


///////////////////////////////////////////////////////////////
//
// CPerfStatFunctionTimingImpl::SetActive
//
//
//
///////////////////////////////////////////////////////////////
void CPerfStatFunctionTimingImpl::SetActive ( bool bActive )
{
    if ( bActive != m_bIsActive )
    {
        m_bIsActive = bActive;
        g_pStats->bFunctionTimingActive = m_bIsActive;
    }
}


///////////////////////////////////////////////////////////////
//
// CPerfStatFunctionTimingImpl::UpdateTiming
//
//
//
///////////////////////////////////////////////////////////////
void CPerfStatFunctionTimingImpl::UpdateTiming ( const char* szFunctionName, TIMEUS timeUs )
{
    if ( !m_bIsActive )
        return;

    // Ignore any single calls over lowest threshold from any viewer
    if ( timeUs < m_PeakUsThresh )
        return;

    float fTimeMs = timeUs * ( 1 / 1000.f );

    // Record the timing
    SFunctionTimingInfo& item = MapGet ( m_TimingMap, szFunctionName );
    item.now5s.uiNumCalls++;
    item.now5s.fTotalMs += fTimeMs;
    item.now5s.fPeakMs = Max ( item.now5s.fPeakMs, fTimeMs );
}


///////////////////////////////////////////////////////////////
//
// CPerfStatFunctionTimingImpl::GetStats
//
//
//
///////////////////////////////////////////////////////////////
void CPerfStatFunctionTimingImpl::GetStats ( CPerfStatResult* pResult, const std::map < SString, int >& optionMap, const SString& strFilter )
{
    m_TimeSinceLastViewed.Reset ();
    SetActive ( true );

    //
    // Set option flags
    //
    bool bHelp = MapContains ( optionMap, "h" );
    int iPeakMsThresh = optionMap.empty () ? 0 : atoi ( optionMap.begin ()->first );
    if ( iPeakMsThresh < 1 )
        iPeakMsThresh = DEFAULT_THRESH_MS;
    m_PeakUsRequiredHistory.AddValue ( iPeakMsThresh * 1000 );

    //
    // Process help
    //
    if ( bHelp )
    {
        pResult->AddColumn ( "Function timings help" );
        pResult->AddRow ()[0] = "Option h - This help";
        pResult->AddRow ()[0] = "1-50 - Peak Ms threshold (defaults to 1)";
        return;
    }

    //
    // Set column names
    //

    pResult->AddColumn ( " " );
    pResult->AddColumn ( "5 sec.calls" );
    pResult->AddColumn ( "5 sec.cpu total" );
    pResult->AddColumn ( "5 sec.cpu peak" );
    pResult->AddColumn ( "60 sec.calls" );
    pResult->AddColumn ( "60 sec.cpu total" );
    pResult->AddColumn ( "60 sec.cpu peak" );

    //
    // Set rows
    //

    for ( std::map < SString, SFunctionTimingInfo > :: const_iterator iter = m_TimingMap.begin () ; iter != m_TimingMap.end () ; iter++ )
    {
        const SString& strFunctionName  = iter->first;
        const SFunctionTimingInfo& item = iter->second;

        const STiming& prev5s = item.history[ item.iPrevIndex ];
        const STiming& prev60s = item.prev60s;

        bool bHas5s = prev5s.uiNumCalls > 0;
        bool bHas60s = prev60s.uiNumCalls > 0;

        if ( !bHas5s && !bHas60s )
            continue;

        // Filter peak threshold for this viewer
        if ( prev5s.fPeakMs < iPeakMsThresh && prev60s.fPeakMs < iPeakMsThresh )
            continue;

        // Apply filter
        if ( strFilter != "" && strFunctionName.find ( strFilter ) == SString::npos )
            continue;

        // Add row
        SString* row = pResult->AddRow ();
        int c = 0;
        row[c++] = strFunctionName;

        if ( !bHas5s )
        {
            row[c++] = "-";
            row[c++] = "-";
            row[c++] = "-";
        }
        else
        {
            row[c++] = SString ( "%u", prev5s.uiNumCalls );
            row[c++] = SString ( "%2.0f ms", prev5s.fTotalMs );
            row[c++] = SString ( "%2.0f ms", prev5s.fPeakMs );
        }

        row[c++] = SString ( "%u", prev60s.uiNumCalls );
        row[c++] = SString ( "%2.0f ms", prev60s.fTotalMs );
        row[c++] = SString ( "%2.0f ms", prev60s.fPeakMs );
    }
}
