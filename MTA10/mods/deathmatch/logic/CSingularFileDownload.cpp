/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CSingularFileDownloadManager.cpp
*  PURPOSE:     Singular file download manager interface
*  DEVELOPERS:  Kevin Whiteside <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CSingularFileDownload::CSingularFileDownload ( CResource* pResource, const char *szName, const char *szNameShort, SString strHTTPURL, CChecksum checksum )
{
    // Store the name
    size_t sizeName = strlen ( szName );
    m_szName = new char [ sizeName + 1 ];
    strcpy ( m_szName, szName );
    m_szName[sizeName] = '\0';

    // Store the  name (short)
    size_t sizeNameShort = strlen ( szNameShort );
    m_szNameShort = new char [ sizeNameShort + 1 ];
    strcpy ( m_szNameShort, szNameShort );
    m_szNameShort[sizeNameShort] = '\0';

    // store the resource
    m_pResource = pResource;

    // Store the provided checksum
    m_ProvidedChecksum = checksum;

    GenerateClientChecksum();

    if ( !DoesClientAndServerChecksumMatch () )
    {
        CNetHTTPDownloadManagerInterface* pHTTP = g_pCore->GetNetwork ()->GetHTTPDownloadManager ();
        pHTTP->QueueFile ( strHTTPURL.c_str(), szName, 0, NULL, 0, false, this, ProgressCallBack );
        m_bComplete = false;
    }
    else
    {
        CallFinished ();
    }

}


CSingularFileDownload::~CSingularFileDownload ( void )
{
    if ( m_szName )
    {
        delete [] m_szName;
        m_szName = 0;
    }

    if ( m_szNameShort )
    {
        delete [] m_szNameShort;
        m_szNameShort = 0;
    }
}


void CSingularFileDownload::ProgressCallBack ( double sizeJustDownloaded, double totalDownloaded, char *data, size_t dataLength, void *obj, bool complete, int error )
{
    if ( complete )
    {
        CSingularFileDownload * pFile = (CSingularFileDownload*)obj;
        CResource* pResource = pFile->GetResource();
        if ( pResource )
        {
            // Call the onFileDownloadComplete event
            CLuaArguments Arguments;
            Arguments.PushString ( pFile->GetShortName() );        // file name
            Arguments.PushBoolean ( true );     // Completed successfully?
            pResource->GetResourceEntity()->CallEvent ( "onFileDownloadComplete", Arguments, false );
        }
        pFile->SetComplete();
    }
    else if ( error )
    {
        CSingularFileDownload * pFile = (CSingularFileDownload*)obj;
        CResource* pResource = pFile->GetResource();
        if ( pResource )
        {
            // Call the onFileDownloadComplete event
            CLuaArguments Arguments;
            Arguments.PushString ( pFile->GetShortName() );        // file name
            Arguments.PushBoolean ( false );    // Completed successfully?
            pResource->GetResourceEntity()->CallEvent ( "onFileDownloadComplete", Arguments, false );
        }
        pFile->SetComplete();
    }
}


void CSingularFileDownload::CallFinished ( void )
{
    if ( m_pResource )
    {
        // Call the onFileDownloadComplete event
        CLuaArguments Arguments;
        Arguments.PushString ( GetShortName() );        // file name
        Arguments.PushBoolean ( true );     // Completed successfully?
        m_pResource->GetResourceEntity()->CallEvent ( "onFileDownloadComplete", Arguments, false );
    }
    SetComplete();
}


bool CSingularFileDownload::DoesClientAndServerChecksumMatch ( void )
{
    return ( m_LastClientChecksum.CompareWithLegacy ( m_ProvidedChecksum ) );
}

CChecksum CSingularFileDownload::GenerateClientChecksum ( void )
{
    m_LastClientChecksum = CChecksum::GenerateChecksumFromFile ( m_szName );
    return m_LastClientChecksum;
}

CChecksum CSingularFileDownload::GetLastClientChecksum ( void )
{
    return m_LastClientChecksum;
}

CChecksum CSingularFileDownload::GetProvidedChecksum ( void )
{
    return m_ProvidedChecksum;
}

void CSingularFileDownload::SetProvidedChecksum ( CChecksum providedChecksum )
{
    m_ProvidedChecksum = providedChecksum;
}
