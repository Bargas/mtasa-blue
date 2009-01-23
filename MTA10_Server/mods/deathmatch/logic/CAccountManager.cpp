/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CAccountManager.cpp
*  PURPOSE:     User account manager class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <>
*               Jax <>
*               Kevin Whiteside <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CAccountManager::CAccountManager ( char* szFileName ): CXMLConfig ( szFileName )
{
    m_bRemoveFromList = true;
    m_bAutoLogin = false;
    m_ulLastTimeSaved = GetTime ();
    m_bChangedSinceSaved = false;
}


CAccountManager::~CAccountManager ( void )
{
    Save ();
    RemoveAll ();
}


void CAccountManager::DoPulse ( void )
{
    // Save it only once in a while whenever something has changed
    if ( m_bChangedSinceSaved &&
         GetTime () > m_ulLastTimeSaved + 15000 )
    {
        // Save it
        Save ();
    }
}


bool CAccountManager::Load ( const char* szFileName )
{
    if ( szFileName == NULL )
        szFileName = m_strFileName.c_str ();

    if ( szFileName && szFileName [ 0 ] )
    {
        // Delete existing XML
        if ( m_pFile )
        {
            delete m_pFile;
        }

        // Create new one
        m_pFile = g_pServerInterface->GetXML ()->CreateXML ( szFileName );
        if ( m_pFile )
        {
            if ( m_pFile->Parse () )
            {
                CXMLNode* pRootNode = m_pFile->GetRootNode ();
                if ( pRootNode )
                {
                    return Load ( pRootNode );
                }
            }
        }
    }
    return false;
}


bool CAccountManager::Load ( CXMLNode* pParent )
{
    #define ACCOUNT_VALUE_LENGTH 128

    std::string strBuffer, strName, strPassword, strLevel, strIP, strDataKey, strDataValue;

    if ( pParent )
    {
        CXMLNode* pAccountNode = NULL;
        unsigned int uiAccountNodesCount = pParent->GetSubNodeCount ();
        for ( unsigned int i = 0 ; i < uiAccountNodesCount ; i++ )
        {
            pAccountNode = pParent->GetSubNode ( i );
            if ( pAccountNode == NULL )
                continue;
            
            strBuffer = pAccountNode->GetTagName ();
            if ( strBuffer.compare ( "account" ) == 0 )
            {
                CXMLAttribute* pAttribute = pAccountNode->GetAttributes ().Find ( "name" );
                if ( pAttribute )
                {                    
                    strName = pAttribute->GetValue ();

                    pAttribute = pAccountNode->GetAttributes ().Find ( "password" );
                    if ( pAttribute )
                    {                        
                        strPassword = pAttribute->GetValue ();
                        if ( !strName.empty () && !strPassword.empty () )
                        {
                            pAttribute = pAccountNode->GetAttributes ().Find ( "ip" );
                            if ( pAttribute )
                            {
                                strIP = pAttribute->GetValue ();
								CAccount* pAccount = NULL;
								char szSerial [ 128 ] = { 0 };
								pAttribute = pAccountNode->GetAttributes ().Find ( "serial" );
								if ( pAttribute )
								{
									pAccount = new CAccount ( this, true, strName, strPassword, strIP, pAttribute->GetValue () );
								}
								else
								{
	                                pAccount = new CAccount ( this, true, strName, strPassword, strIP );
								}
                            
                                // Grab the data on this account
                                CXMLNode* pDataNode = NULL;
                                unsigned int uiDataNodesCount = pAccountNode->GetSubNodeCount ();
                                for ( unsigned int j = 0 ; j < uiDataNodesCount ; j++ )
                                {
                                    pDataNode = pAccountNode->GetSubNode ( j );
                                    if ( pDataNode == NULL )
                                        continue;

                                    strBuffer = pDataNode->GetTagName ();
                                    if ( strBuffer.compare ( "nil_data") == 0 )
                                    {
                                        CXMLAttributes* pAttributes = &(pDataNode->GetAttributes ());
                                        CXMLAttribute* pAttribute = NULL;
                                        unsigned int uiDataValuesCount = pAttributes->Count ();
                                        for ( unsigned int a = 0 ; a < uiDataValuesCount ; a++ )
                                        {
                                            pAttribute = pAttributes->Get ( a );
                                            strDataKey = pAttribute->GetName ();
                                            strDataValue = pAttribute->GetValue ();
                                            if ( strcmp ( strDataValue.c_str (), "nil" ) == 0 )
                                            {
                                                CLuaArgument Argument;
                                                pAccount->SetData ( strDataKey.c_str (), &Argument );
                                            }
                                        }
                                    }
                                    else if ( strBuffer.compare ( "boolean_data" ) == 0 )
                                    {
                                        CXMLAttributes* pAttributes = &(pDataNode->GetAttributes ());
                                        CXMLAttribute* pAttribute = NULL;
                                        unsigned int uiDataValuesCount = pAttributes->Count ();
                                        for ( unsigned int a = 0 ; a < uiDataValuesCount ; a++ )
                                        {
                                            pAttribute = pAttributes->Get ( a );
                                            strDataKey = pAttribute->GetName ();
                                            strDataValue = pAttribute->GetValue ();
                                            if ( strcmp ( strDataValue.c_str (), "true" ) == 0 )
                                            {
                                                CLuaArgument Argument ( true );
                                                pAccount->SetData ( strDataKey.c_str (), &Argument );
                                            }
                                            /* Don't bother saving/load false booleans
                                            else if ( strcmp ( strDataValue.c_str (), "false" ) == 0 )
                                            {
                                                CLuaArgument Argument ( false );
                                                pAccount->SetData ( strDataKey.c_str (), &Argument );
                                            }*/
                                        }
                                    }
                                    else if ( strBuffer.compare ( "string_data" ) == 0 )
                                    {
                                        CXMLAttributes* pAttributes = &(pDataNode->GetAttributes ());
                                        CXMLAttribute* pAttribute = NULL;
                                        unsigned int uiDataValuesCount = pAttributes->Count ();
                                        for ( unsigned int a = 0 ; a < uiDataValuesCount ; a++ )
                                        {
                                            pAttribute = pAttributes->Get ( a );
                                            strDataKey = pAttribute->GetName ();
                                            strDataValue = pAttribute->GetValue ();

                                            CLuaArgument Argument ( strDataValue.c_str () );
                                            pAccount->SetData ( strDataKey.c_str (), &Argument );
                                        }
                                    }
                                    else if ( strBuffer.compare ( "number_data" ) == 0 )
                                    {
                                        CXMLAttributes* pAttributes = &(pDataNode->GetAttributes ());
                                        CXMLAttribute* pAttribute = NULL;
                                        unsigned int uiDataValuesCount = pAttributes->Count ();
                                        for ( unsigned int a = 0 ; a < uiDataValuesCount ; a++ )
                                        {
                                            pAttribute = pAttributes->Get ( a );
                                            strDataKey = pAttribute->GetName ();
                                            strDataValue = pAttribute->GetValue ();
                                            
                                            CLuaArgument Argument ( strtod ( strDataValue.c_str (), NULL ) );
                                            pAccount->SetData ( strDataKey.c_str (), &Argument );
                                        }
                                    }
                                }
                            }
							else
							{
								CAccount* pAccount = NULL;
								char szSerial [ 128 ] = { 0 };
								pAttribute = pAccountNode->GetAttributes ().Find ( "serial" );
								if ( pAttribute )
								{
									pAccount = new CAccount ( this, true, strName, strPassword, NULL, pAttribute->GetValue () );
								}    
								else
								{
									pAccount = new CAccount ( this, true, strName, strPassword );
								}
							}
                        }
                    }
                }
            }
            else
            {
                LoadSetting ( pAccountNode );
            }
        }

        m_bChangedSinceSaved = false;
        return true;
    }

    return false;
}


bool CAccountManager::LoadSetting ( CXMLNode* pNode )
{
    if ( pNode->GetTagName ().compare ( "autologin" ) == 0 )
    {
        bool bTemp;
        if ( pNode->GetTagContent ( bTemp ) )
        {
            m_bAutoLogin = bTemp;
        }
    }
    else
        return false;

    return true;
}


bool CAccountManager::Save ( const char* szFileName )
{
    // Attempted save now
    m_bChangedSinceSaved = false;
    m_ulLastTimeSaved = GetTime ();

    if ( szFileName == NULL )
        szFileName = m_strFileName.c_str ();

    if ( szFileName == NULL || szFileName [ 0 ] == 0 )
        return false;
    
    FILE* pFile = fopen ( szFileName, "w+" );
    if ( pFile == NULL )
        return false;
    fclose ( pFile );

    // Delete existing XML
    if ( m_pFile )
    {
        delete m_pFile;
    }

    // Create the XML
    m_pFile = g_pServerInterface->GetXML ()->CreateXML ( szFileName );
    if ( !m_pFile )
        return false;

    // Get the root node. Eventually make one called accounts
    CXMLNode* pRootNode = m_pFile->GetRootNode ();
    if ( !pRootNode )
        pRootNode = m_pFile->CreateRootNode ( "accounts" );

    // Save everything into it
    if ( !Save ( pRootNode ) )
    {
        delete m_pFile;
        m_pFile = NULL;
        return false;
    }
    
    m_pFile->Write ();
    delete m_pFile;
    m_pFile = NULL;
    return true;
}


bool CAccountManager::Save ( CXMLNode* pParent )
{
    if ( pParent )
    {
        SaveSettings ( pParent );

        CXMLNode* pNode = NULL;

        list < CAccount* > ::iterator iter = m_List.begin ();
        for ( ; iter != m_List.end () ; iter++ )
        {
            if ( (*iter)->IsRegistered () )
            {
                pNode = pParent->CreateSubNode ( "account" );
                if ( pNode )
                {
                    CXMLAttribute* pAttribute = pNode->GetAttributes ().Create ( "name" );
                    if ( pAttribute )
                        pAttribute->SetValue ( (*iter)->GetName ().c_str () );

                    pAttribute = pNode->GetAttributes ().Create ( "password" );
                    if ( pAttribute )
                        pAttribute->SetValue ( (*iter)->GetPassword ().c_str () );

                    const char* szIP = (*iter)->GetIP ().c_str ();
                    if ( szIP && szIP [ 0 ] )
                    {
                        pAttribute = pNode->GetAttributes ().Create ( "ip" );
                        if ( pAttribute )
                            pAttribute->SetValue ( szIP );
                    }

					const char* szSerial = (*iter)->GetSerial ().c_str ();
                    if ( szSerial && szSerial [ 0 ] )
                    {
                        pAttribute = pNode->GetAttributes ().Create ( "serial" );
                        if ( pAttribute )
                            pAttribute->SetValue ( szSerial );
                    }

                    list < CAccountData * > ::iterator iterData = NULL;
                    // Do we have any data to save?
                    if ( (*iter)->DataCount () > 0 )
                    {
                        // Sort our data into separate lists of each type
                        list < CAccountData * > nilList, boolList, stringList, numberList;
                        list < CAccountData * > * pDataList = NULL;
                        iterData = (*iter)->DataBegin ();
                        for ( ; iterData != (*iter)->DataEnd () ; iterData++ )
                        {
                            switch ( (*iterData)->GetValue ()->GetType () )
                            {
                                case LUA_TNIL:
                                    nilList.push_back ( *iterData );
                                    break;
                                case LUA_TBOOLEAN:
                                    boolList.push_back ( *iterData );
                                    break;
                                case LUA_TSTRING:
                                    stringList.push_back ( *iterData );
                                    break;
                                case LUA_TNUMBER:
                                    numberList.push_back ( *iterData );
                                    break;
                            }
                        }
                        CXMLAttribute * pAttribute = NULL;
                        CXMLNode * pDataNode = NULL;
                        CAccountData * pData = NULL;
                        char szArgumentAsString [ 128 ];
                        // Do we have any nil data?
                        pDataList = &nilList;
                        if ( !pDataList->empty () )
                        {
                            // Create a node for this type of data
                            pDataNode = pNode->CreateSubNode ( "nil_data" );
                            if ( pDataNode )
                            {                                
                                iterData = pDataList->begin ();
                                for ( ; iterData != pDataList->end (); iterData++ )
                                {
                                    pData = *iterData;
                                    pAttribute = pDataNode->GetAttributes ().Create ( pData->GetKey ().c_str () );
                                    if ( pAttribute )
                                    {
                                        pData->GetValue ()->GetAsString ( szArgumentAsString, 128 );
                                        pAttribute->SetValue ( szArgumentAsString );
                                    }
                                }
                            }
                        }
                        // Do we have any boolean data?
                        pDataList = &boolList;
                        if ( !pDataList->empty () )
                        {
                            // Create a node for this type of data
                            pDataNode = pNode->CreateSubNode ( "boolean_data" );
                            if ( pDataNode )
                            {
                                pDataList = &boolList;
                                iterData = pDataList->begin ();
                                for ( ; iterData != pDataList->end (); iterData++ )
                                {
                                    pData = *iterData;
                                    pAttribute = pDataNode->GetAttributes ().Create ( pData->GetKey ().c_str () );
                                    if ( pAttribute )
                                    {
                                        pData->GetValue ()->GetAsString ( szArgumentAsString, 128 );
                                        pAttribute->SetValue ( szArgumentAsString );
                                    }
                                }
                            }
                        }
                        // Do we have any string data?
                        pDataList = &stringList;
                        if ( !pDataList->empty () )
                        {
                            // Create a node for this type of data
                            pDataNode = pNode->CreateSubNode ( "string_data" );
                            if ( pDataNode )
                            {
                                iterData = pDataList->begin ();
                                for ( ; iterData != pDataList->end (); iterData++ )
                                {
                                    pData = *iterData;
                                    pAttribute = pDataNode->GetAttributes ().Create ( pData->GetKey ().c_str () );
                                    if ( pAttribute )
                                    {
                                        pData->GetValue ()->GetAsString ( szArgumentAsString, 128 );
                                        pAttribute->SetValue ( szArgumentAsString );
                                    }
                                }
                            }
                        }
                        // Do we have any number data?
                        pDataList = &numberList;
                        if ( !pDataList->empty () )
                        {
                            // Create a node for this type of data
                            pDataNode = pNode->CreateSubNode ( "number_data" );
                            if ( pDataNode )
                            {
                                iterData = pDataList->begin ();
                                for ( ; iterData != pDataList->end (); iterData++ )
                                {
                                    pData = *iterData;
                                    pAttribute = pDataNode->GetAttributes ().Create ( pData->GetKey ().c_str () );
                                    if ( pAttribute )
                                    {
                                        pData->GetValue ()->GetAsString ( szArgumentAsString, 128 );
                                        pAttribute->SetValue ( szArgumentAsString );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return true;
    }
    return false;
}


bool CAccountManager::SaveSettings ( CXMLNode* pParent )
{
    SetBoolean ( pParent, "autologin", m_bAutoLogin );
    return true;
}


CAccount* CAccountManager::Get ( const char* szName, bool bRegistered )
{
    if ( szName && szName [ 0 ] )
    {
        unsigned int uiHash = HashString ( szName );
        list < CAccount* > ::iterator iter = m_List.begin ();
        for ( ; iter != m_List.end () ; iter++ )
        {
            CAccount* pAccount = *iter;
            if ( pAccount->IsRegistered () == bRegistered )
            {
                if ( pAccount->GetNameHash() == uiHash )
                {
                    return pAccount;
                }
            }
        }
    }
    return NULL;
}


CAccount* CAccountManager::Get ( const char* szName, const char* szIP )
{
    if ( szName && szName [ 0 ] && szIP && szIP [ 0 ] )
    {
        unsigned int uiHash = HashString ( szName );
        list < CAccount* > ::iterator iter = m_List.begin ();
        for ( ; iter != m_List.end () ; iter++ )
        {
            CAccount* pAccount = *iter;
            if ( pAccount->IsRegistered () )
            {
                if ( pAccount->GetNameHash() == uiHash )
                {
                    if ( pAccount->GetIP ().compare ( szIP ) == 0 )
                    {
                        return pAccount;
                    }
                }
            }
        }
    }
    return NULL;
}


bool CAccountManager::Exists ( CAccount* pAccount )
{
    list < CAccount* > ::iterator iter = m_List.begin ();
    for ( ; iter != m_List.end () ; iter++ )
    {
        if ( *iter == pAccount )
        {
            return true;
        }
    }
    return false;
}


void CAccountManager::RemoveFromList ( CAccount* pAccount )
{
    if ( m_bRemoveFromList && !m_List.empty() )
    {
        m_List.remove ( pAccount );
    }
}


void CAccountManager::RemoveAll ( void )
{
    m_bRemoveFromList = false;
    list < CAccount* > ::iterator iter = m_List.begin ();
    for ( ; iter != m_List.end () ; iter++ )
    {
        delete *iter;
    }
    m_List.clear ();
    m_bRemoveFromList = true;
}

bool CAccountManager::LogIn ( CClient* pClient, CClient* pEchoClient, const char* szNick, const char* szPassword )
{
    // Is he already logged in?
    if ( pClient->IsRegistered () )
    {
        if ( pEchoClient )
            pEchoClient->SendEcho ( "login: You are already logged in" );
        return false;
    }

    // Grab the account on his nick if any
    CAccount* pAccount = g_pGame->GetAccountManager ()->Get ( szNick );
    if ( !pAccount )
    {
        if ( pEchoClient )
            pEchoClient->SendEcho ( "login: No account with your nick" );
        return false;
    }

    if ( pAccount->GetClient () )
    {
        if ( pEchoClient )
            pEchoClient->SendEcho ( "login: Account in use" );
        return false;
    }

    // Compare the passwords
    if ( !pAccount->IsPassword ( szPassword ) )
    {
        if ( pEchoClient )
            pEchoClient->SendEcho ( "login: Bad password" );
        CLogger::LogPrintf ( "LOGIN: %s tried to log in with a bad password\n", szNick );
        return false;
    }

    // Try to log him in
    return LogIn ( pClient, pEchoClient, pAccount );
}

bool CAccountManager::LogIn ( CClient* pClient, CClient* pEchoClient, CAccount* pAccount, bool bAutoLogin )
{
    // Log him in
    CAccount* pCurrentAccount = pClient->GetAccount ();
    pClient->SetAccount ( pAccount );
    pAccount->SetClient ( pClient );

    // Set IP in account
    if ( pClient->GetClientType () == CClient::CLIENT_PLAYER )
    {
        CPlayer* pPlayer = static_cast < CPlayer* > ( pClient );

        char szIP [ 25 ];
        pPlayer->GetSourceIP ( szIP );
        pAccount->SetIP ( szIP );
    }

    // Call the onClientLogin script event
    CElement* pClientElement = NULL;
    switch ( pClient->GetClientType () )
    {
        case CClient::CLIENT_PLAYER:
        {
            CPlayer* pPlayer = static_cast < CPlayer* > ( pClient );
            pClientElement = static_cast < CElement* > ( pPlayer );
            break;
        }
        case CClient::CLIENT_CONSOLE:
        {
            CConsoleClient* pConsoleClient = static_cast < CConsoleClient* > ( pClient );
            pClientElement = static_cast < CElement* > ( pConsoleClient );
            break;
        }
    }
    if ( pClientElement )
    {
        CLuaArguments Arguments;
        Arguments.PushAccount ( pCurrentAccount );
        Arguments.PushAccount ( pAccount );
        Arguments.PushBoolean ( bAutoLogin );
        if ( !pClientElement->CallEvent ( "onPlayerLogin", Arguments ) )
        {
            // DENIED!
            pClient->SetAccount ( pCurrentAccount );
            pAccount->SetClient ( NULL );
            return false;
        }
    }

    // Tell the console
    CLogger::LogPrintf ( "LOGIN: %s successfully logged in\n", pClient->GetNick () );

    // Tell the player
    if ( pEchoClient )
    {
        if ( bAutoLogin )
            pEchoClient->SendEcho ( "auto-login: You successfully logged in" );
        else
            pEchoClient->SendEcho ( "login: You successfully logged in" );
    }
    
    // Delete the old account if it was a guest account
    if ( !pCurrentAccount->IsRegistered () )
        delete pCurrentAccount;

    return true;
}

bool CAccountManager::LogOut ( CClient* pClient, CClient* pEchoClient )
{
    // Is he logged in?
    if ( !pClient->IsRegistered () )
    {
        if ( pEchoClient )
            pEchoClient->SendEcho ( "logout: You were not logged in" );
        return false;
    }

    CAccount* pCurrentAccount = pClient->GetAccount ();
    pCurrentAccount->SetClient ( NULL );

    CAccount* pAccount = new CAccount ( g_pGame->GetAccountManager (), false, "guest" );
    pClient->SetAccount ( pAccount );

    // Call the onClientLogout event
    CElement* pClientElement = NULL;
    switch ( pClient->GetClientType () )
    {
        case CClient::CLIENT_PLAYER:
        {
            CPlayer* pPlayer = static_cast < CPlayer* > ( pClient );
            pClientElement = static_cast < CElement* > ( pPlayer );
            break;
        }
        case CClient::CLIENT_CONSOLE:
        {
            CConsoleClient* pConsoleClient = static_cast < CConsoleClient* > ( pClient );
            pClientElement = static_cast < CElement* > ( pConsoleClient );
            break;
        }
    }
    if ( pClientElement )
    {
        // Call our script event
        CLuaArguments Arguments;
        Arguments.PushAccount ( pCurrentAccount );
        Arguments.PushAccount ( pAccount );
        if ( !pClientElement->CallEvent ( "onPlayerLogout", Arguments ) )
        {
            // DENIED!
            pClient->SetAccount ( pCurrentAccount );
            pCurrentAccount->SetClient ( pClient );
            delete pAccount;
            return false;
        }
    }

    // Tell the console
    CLogger::LogPrintf ( "LOGOUT: %s logged out\n", pClient->GetNick () );

    // Tell the player
    if ( pEchoClient )
        pEchoClient->SendEcho ( "logout: You logged out" );
    
    return true;
}