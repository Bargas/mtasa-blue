/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CPacketHandler.cpp
*  PURPOSE:     Packet handling and processing
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               Alberto Alonso <rydencillo@gmail.com>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

class CCore;

// TODO: Make this independant of g_pClientGame. Just moved it here to get it out of the 
//       horribly big CClientGame file.
bool CPacketHandler::ProcessPacket ( unsigned char ucPacketID, NetBitStreamInterface& bitStream, unsigned long ulTimeStamp )
{
    // Can the net api handle it?
    if ( g_pClientGame->m_pNetAPI->ProcessPacket ( ucPacketID, bitStream, ulTimeStamp ) )
    {
        return true;
    }

    // Can we handle it?
    switch ( ucPacketID )
    {
        case PACKET_ID_SERVER_CONNECTED:
            Packet_ServerConnected ( bitStream );
            return true;

        case PACKET_ID_SERVER_JOINEDGAME:
            Packet_ServerJoined ( bitStream );
            return true;

        case PACKET_ID_SERVER_DISCONNECTED:
            Packet_ServerDisconnected ( bitStream );
            return true;

        case PACKET_ID_PLAYER_LIST:
            Packet_PlayerList ( bitStream );
            return true;

        case PACKET_ID_PLAYER_QUIT:
            Packet_PlayerQuit ( bitStream );
            return true;

        case PACKET_ID_PLAYER_SPAWN:
            Packet_PlayerSpawn ( bitStream );
            return true;

        case PACKET_ID_PLAYER_WASTED:
            Packet_PlayerWasted ( bitStream );
            return true;

        case PACKET_ID_PLAYER_CHANGE_NICK:
            Packet_PlayerChangeNick ( bitStream );
            return true;

        case PACKET_ID_PLAYER_PINGS:
            Packet_PlayerPings ( bitStream );
            return true;

        case PACKET_ID_CHAT_ECHO:
            Packet_ChatEcho ( bitStream );
            return true;

        case PACKET_ID_CONSOLE_ECHO:
            Packet_ConsoleEcho ( bitStream );
            return true;

        case PACKET_ID_DEBUG_ECHO:
            Packet_DebugEcho ( bitStream );
            return true;

        case PACKET_ID_VEHICLE_SPAWN:
            Packet_VehicleSpawn ( bitStream );
            return true;

        case PACKET_ID_VEHICLE_INOUT:
            Packet_Vehicle_InOut ( bitStream );
            return true;

        case PACKET_ID_VEHICLE_DAMAGE_SYNC:
            Packet_VehicleDamageSync ( bitStream );
            return true;

        case PACKET_ID_VEHICLE_TRAILER:
            Packet_VehicleTrailer ( bitStream );
            return true;

        // Map information packet sent when you join if a map is being played
        case PACKET_ID_MAP_INFO:
            Packet_MapInfo ( bitStream );
            return true;

        // Adds a new entity of any type to the world streamer.
        case PACKET_ID_ENTITY_ADD:
            Packet_EntityAdd ( bitStream );
            return true;

        // Deletes vehicles
        case PACKET_ID_ENTITY_REMOVE:
            Packet_EntityRemove ( bitStream );
            return true;

        // Respawns/hides pickups
        case PACKET_ID_PICKUP_HIDESHOW:
            Packet_PickupHideShow ( bitStream );
            return true;

        // Pickup hit confirmations
        case PACKET_ID_PICKUP_HIT_CONFIRM:
            Packet_PickupHitConfirm ( bitStream );
            return true;

        // Functions fro LUA scripts server side
        case PACKET_ID_LUA:
            Packet_Lua ( bitStream );
            return true;

        case PACKET_ID_TEXT_ITEM:
            Packet_TextItem ( bitStream );
            break;

        case PACKET_ID_EXPLOSION:
            Packet_ExplosionSync ( bitStream );
            break;

        case PACKET_ID_FIRE:
            Packet_FireSync ( bitStream );
            break;

        case PACKET_ID_PROJECTILE:
            Packet_ProjectileSync ( bitStream );
            break;

        case PACKET_ID_PLAYER_STATS:
            Packet_PlayerStats ( bitStream );
            break;

        case PACKET_ID_PLAYER_CLOTHES:
            Packet_PlayerClothes ( bitStream );
            break;

        case PACKET_ID_LUA_EVENT:
            Packet_LuaEvent ( bitStream );
            break;

        case PACKET_ID_RESOURCE_START:
            Packet_ResourceStart ( bitStream );
            return true;

        case PACKET_ID_RESOURCE_STOP:
            Packet_ResourceStop ( bitStream );
            return true;

        case PACKET_ID_DETONATE_SATCHELS:
            Packet_DetonateSatchels ( bitStream );
            return true;

        #ifdef MTA_VOICE
        case PACKET_ID_VOICE_DATA:
            Packet_VoiceData ( bitStream );
            return true;
        #endif

        default:             break;
    }

    // See if unoccupied sync can handle it
    if ( g_pClientGame->m_pUnoccupiedVehicleSync->ProcessPacket ( ucPacketID, bitStream ) )
        return true;
    else if ( g_pClientGame->m_pPedSync->ProcessPacket ( ucPacketID, bitStream ) )
        return true;

    return false;
}


void CPacketHandler::Packet_ServerConnected ( NetBitStreamInterface& bitStream )
{
    // unsigned char [x]    - server version string

    // If we're not "connecting", we shouldn't have received this packet. Unload.
    if ( g_pClientGame->m_Status != CClientGame::STATUS_CONNECTING )
    {
        RaiseProtocolError ( 1 );
        return;
    }

    /*
     * We *REALLY* need to use the CPlayerJoinCompletePacket class here
     * instead of reading this out separately.
     */

    // Extract the version string
    char szVersionString [48];
    szVersionString[0] = '\0';

    unsigned short ucSize = 0;
    if (!bitStream.Read(ucSize))
    {
		__asm int 3;
        g_pCore->SetConnected ( false );
        return;
    }

    // Adjust the size to our buffer size
    if ( ucSize > sizeof ( szVersionString ) - 1 )
    {
        ucSize = sizeof ( szVersionString ) - 1;
    }

    // Read out the reason to a buffer
    bitStream.Read(szVersionString, ucSize);
    if (ucSize)
        szVersionString[ucSize] = '\0';

    // Run it through the character filter
    StripUnwantedCharacters ( szVersionString, ' ' );


    // Echo Connected to the chatbox
    if ( strlen ( szVersionString ) > 0 )
    {
        g_pCore->ChatPrintfColor ( "* Connected! [%s]", false, CHATCOLOR_INFO, szVersionString );
    }
    else
    {
        g_pCore->ChatEchoColor ( "* Connected!", CHATCOLOR_INFO );
    }

    //m_Status = CClientGame::STATUS_TRANSFER;

    // Tell the core we're finished
    g_pCore->SetConnected ( true );
    g_pCore->HideMainMenu ();

    // We're now "Joining"
    g_pClientGame->m_Status = CClientGame::STATUS_JOINING;

    // If the game isn't started, start it
    if ( g_pGame->GetSystemState () == 7 )
    {
        g_pGame->StartGame ();
    }
}


void CPacketHandler::Packet_ServerJoined ( NetBitStreamInterface& bitStream )
{
    // ElementID        (2)     - assigned player id
    // unsigned char    (1)     - number of players in the server (including local)
    // ElementID        (2)     - root element id
    // unsigned char    (1)     - HTTP Download Type
    // HTTP Download Type
    // 0 - Disabled     (0)
    // 1 - Port
    // unsigned short   (2)     - HTTP Download Port
    // 2 - URL
    // unsigned short   (2)     - HTTP Download URL Size
    // unsigned char    (X)     - HTTP Download URL


    // Make sure any existing messageboxes are hided
    g_pCore->RemoveMessageBox ();

    // Are we ingame?
    if ( !g_pClientGame->m_bGameLoaded )
    {
        RaiseProtocolError ( 2 );
        return;
    }

    // Are we JOINING? (if not, protocol error)
    if ( g_pClientGame->m_Status != CClientGame::STATUS_JOINING )
    {
        RaiseProtocolError ( 3 );
        return;
    }

    // Read out our local id
    bitStream.Read ( g_pClientGame->m_LocalID );
    if ( g_pClientGame->m_LocalID == INVALID_ELEMENT_ID )
    {
        RaiseProtocolError ( 4 );
        return;
    }

    // Set our player ID
    CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->GetLocalPlayer ();
    if ( !pPlayer )
    {
        RaiseProtocolError ( 5 );
        return;
    }

    pPlayer->SetID ( g_pClientGame->m_LocalID );

    // Read out number of players
    unsigned char ucNumberOfPlayers = 0;
    bitStream.Read ( ucNumberOfPlayers );

    // Can't be 0
    if ( ucNumberOfPlayers == 0 || ucNumberOfPlayers > MAX_PLAYER_COUNT )
    {
        RaiseProtocolError ( 6 );
        return;
    }

    // Read out the root element id
    ElementID RootElementID;
    bitStream.Read ( RootElementID );
    if ( RootElementID == INVALID_ELEMENT_ID )
    {
        // Raise an error
        return;
    }
    g_pClientGame->m_pRootEntity->SetID ( RootElementID );

    // HTTP Download Type
    unsigned char ucHTTPDownloadType;
    bitStream.Read ( ucHTTPDownloadType );

    g_pClientGame->m_ucHTTPDownloadType = static_cast < eHTTPDownloadType > ( ucHTTPDownloadType );
    // Depending on the HTTP Download Type, read more data
    switch ( g_pClientGame->m_ucHTTPDownloadType )
    {
        case HTTP_DOWNLOAD_DISABLED:
        {
            RaiseFatalError ( 3 );
            break;
        }
        case HTTP_DOWNLOAD_ENABLED_PORT:
        {
            bitStream.Read ( g_pClientGame->m_usHTTPDownloadPort );
            // TODO: Set m_szHTTPDownloadURL to the appropriate path based off of server ip / port
            unsigned long ulHTTPDownloadPort = g_pClientGame->m_usHTTPDownloadPort;
            snprintf ( g_pClientGame->m_szHTTPDownloadURL, MAX_HTTP_DOWNLOAD_URL, "http://%s:%d", g_pNet->GetConnectedServer(), ulHTTPDownloadPort );
            g_pClientGame->m_szHTTPDownloadURL[MAX_HTTP_DOWNLOAD_URL] = '\0';

            // We are downloading from the internal HTTP Server, therefore disable multiple downloads
            g_pCore->GetNetwork ()->GetHTTPDownloadManager ()->SetSingleDownloadOption ( true );
            break;
        }
        case HTTP_DOWNLOAD_ENABLED_URL:
        {
            unsigned short usHTTPDownloadURL;
            bitStream.Read ( usHTTPDownloadURL );

            if ( usHTTPDownloadURL > 0 && usHTTPDownloadURL <= MAX_HTTP_DOWNLOAD_URL )
            {
                bitStream.Read ( g_pClientGame->m_szHTTPDownloadURL, usHTTPDownloadURL );
            }

            g_pClientGame->m_szHTTPDownloadURL[usHTTPDownloadURL] = '\0';

            // We are downloading from a URL, therefore allow multiple downloads
            g_pCore->GetNetwork ()->GetHTTPDownloadManager ()->SetSingleDownloadOption ( false );
            break;
        }
    default:
        break;
    }

    // Make the camera black until we spawn
    // Anyone want to document wtf these values are?  Why are we putting seemingly "random"
    // numbers everywhere?  --slush.
    CVector vecPos ( -2377, -1636, 700 );    
    /*m_pCamera->SetFocus ( m_pLocalPlayer, MODE_FIXED, false );
    m_pCamera->SetFocus ( &vecPos, false );*/
    g_pClientGame->m_pCamera->ToggleCameraFixedMode ( true );
    g_pClientGame->m_pCamera->SetPosition ( vecPos );
    g_pClientGame->m_pCamera->SetTarget ( CVector ( 0, 0, 720 ) );  

    // We're now joined
    g_pClientGame->m_Status = CClientGame::STATUS_JOINED;

	// Call the onClientPlayerJoin event for ourselves
    CLuaArguments Arguments;
    g_pClientGame->m_pLocalPlayer->CallEvent ( "onClientPlayerJoin", Arguments, true );
}


void CPacketHandler::Packet_ServerDisconnected ( NetBitStreamInterface& bitStream )
{
    // unsigned char [x]    - disconnect reason

    // Any reason bytes?
    int iSize = bitStream.GetNumberOfBytesUsed ();
    if ( iSize > 0 )
    {
        // Adjust the size to our buffer size
        char szReason [NET_DISCONNECT_REASON_SIZE];
		memset ( szReason, 0, NET_DISCONNECT_REASON_SIZE );

        if ( iSize > sizeof ( szReason ) - 1 )
        {
            iSize = sizeof ( szReason ) - 1;
        }

        bitStream.Read ( szReason, iSize );

        // Run it through the character filter
        StripUnwantedCharacters ( szReason, ' ' );

        // Display the error
        g_pCore->ShowMessageBox ( "Disconnected", szReason, MB_BUTTON_OK | MB_ICON_INFO );
    }

    // Terminate the mod (disconnect first in case there were more packets after this one)
    g_pClientGame->m_bGracefulDisconnect = true;
    g_pNet->StopNetwork ();
    g_pCore->GetModManager ()->RequestUnload ();
}


void CPacketHandler::Packet_PlayerList ( NetBitStreamInterface& bitStream )
{
    // bool                  - show the "X has joined the game" messages?
    // [ following repeats <number of players joined> times ]
    // unsigned char  (1)    - assigned player id
    // unsigned char  (1)    - player nick length
    // unsigned char  (X)    - player nick (X = player nick length)
    // unsigned char  (1)    - nametag text length
    // unsigned char  (X)    - nametag text (X = nametag text length)
    // unsigned char  (3)    - nametag color
    // unsigned char  (1)    - nametag showing
    // bool                  - is he dead?
    // bool                  - spawned? (following data only if this is TRUE)
    // unsigned char  (1)    - model id
    // ElementID      (2)    - team id
    // bool                  - in a vehicle?
    // ElementID      (2)    - vehicle id (if vehicle)
    // unsigned char  (1)    - vehicle seat (if vehicle)
    // CVector        (12)   - position (if player)
    // float          (4)    - rotation (if player)
    // bool                  - has a jetpack?
    // unsigned short (2)    - dimension
    // unsigned char  (1)    - fighting style

    // The game must be loaded or the game will crash
    if ( g_pClientGame->m_Status != CClientGame::STATUS_JOINED )
    {
        RaiseProtocolError ( 7 );
        return;
    }

    // Flags for all the players
    unsigned char ucGlobalFlags = 0;
    bitStream.Read ( ucGlobalFlags );

    // Grab the flags
    bool bJustJoined = ucGlobalFlags & 0x01;

    // While there are bytes left, parse player list items
    char szNickBuffer [MAX_PLAYER_NICK_LENGTH + 1];
    while ( bitStream.GetNumberOfUnreadBits () >= 8 )
    {
        // Read out the assigned player id
        ElementID PlayerID;
        if ( !bitStream.Read ( PlayerID ) )
        {
            RaiseProtocolError ( 8 );
            return;
        }

        unsigned char ucTimeContext = 255;
        bitStream.Read ( ucTimeContext );

        // Player nick length
        unsigned char ucPlayerNickLength = 255;
        if ( !bitStream.Read ( ucPlayerNickLength ) || ucPlayerNickLength < MIN_PLAYER_NICK_LENGTH || ucPlayerNickLength > MAX_PLAYER_NICK_LENGTH )
        {
            /*
             * So what happens if someone is able to fake a packet with the nick len < MIN_PLAYER_NICK_LENGTH?
             * This is what you call a DoS...
             *
             * --slush
             */
            RaiseProtocolError ( 9 );
            return;
        }

        // Read out the nick
        bitStream.Read ( szNickBuffer, ucPlayerNickLength );
        szNickBuffer [ ucPlayerNickLength ] = '\0';

        // Strip the nick from any unwanted characters
        StripUnwantedCharacters ( szNickBuffer, '_' );

        // Player flags
        unsigned char ucFlags = 0;
        bitStream.Read ( ucFlags );

        // Decode the player flags
        bool bIsDead = ( ucFlags & 0x01 ) ? true:false;
        bool bIsSpawned = ( ucFlags & 0x02 ) ? true:false;
        bool bInVehicle = ( ucFlags & 0x04 ) ? true:false;
        bool bHasJetPack = ( ucFlags & 0x08 ) ? true:false;
        bool bNametagShowing = ( ucFlags & 0x10 ) ? true:false;
        bool bHasNametagColorOverridden = ( ucFlags & 0x20 ) ? true:false;

        // Player nametag text
        char szNametagText [MAX_PLAYER_NICK_LENGTH + 1];
        szNametagText[0] = '\0';

        unsigned char ucNametagTextLength;
        bitStream.Read ( ucNametagTextLength );
        if ( ucNametagTextLength > 0 )
        {
            bitStream.Read ( szNametagText, ucNametagTextLength );
            szNametagText [ ucNametagTextLength ] = '\0';
            StripUnwantedCharacters ( szNametagText, '_' );
        }

        // Read out the nametag override color if it's overridden
        unsigned char ucNametagR, ucNametagG, ucNametagB;
        if ( bHasNametagColorOverridden )
        {
            bitStream.Read ( ucNametagR );
            bitStream.Read ( ucNametagG );
            bitStream.Read ( ucNametagB );
        }

        // Read out the spawndata if he has spawned
        unsigned short usPlayerModelID;
        ElementID TeamID = INVALID_ELEMENT_ID;
        ElementID ID = INVALID_ELEMENT_ID;
        unsigned char ucVehicleSeat = 0xFF;
        CVector vecPosition;
        float fRotation = 0.0f;
        unsigned short usDimension = 0;
        unsigned char ucFightingStyle = 0;
        unsigned char ucAlpha = 255;
        unsigned char ucInterior = 0;
        if ( bIsSpawned )
        {
            // Read out the player model id
            bitStream.Read ( usPlayerModelID );
            if ( !CClientPlayerManager::IsValidModel ( usPlayerModelID ) )
            {
                RaiseProtocolError ( 10 );
                return;
            }

            // Team?
            bitStream.Read ( TeamID );

            // In vehicle?
            if ( bInVehicle )
            {
                // Read out the vehicle id
                bitStream.Read ( ID );
                bitStream.Read ( ucVehicleSeat );

                // Valid seat?
                if ( ucVehicleSeat == 0xFF )
                {
                    RaiseProtocolError ( 11 );
                    return;
                }
            }
            else         // On foot?
            {
                // Set the vehicle id to 0 to indicate we're on foot
                ID = INVALID_ELEMENT_ID;

                // Read out the position
                bitStream.Read ( vecPosition.fX );
                bitStream.Read ( vecPosition.fY );
                bitStream.Read ( vecPosition.fZ );

                // Read out the rotation float
                bitStream.Read ( fRotation );
            }

            bitStream.Read ( usDimension );
            bitStream.Read ( ucFightingStyle );
            bitStream.Read ( ucAlpha );
            bitStream.Read ( ucInterior );
        }

        // Create the player
        CClientPlayer* pPlayer = new CClientPlayer ( g_pClientGame->m_pManager, PlayerID );
        if ( pPlayer )
        {
            // Set its parent the root entity
            pPlayer->SetSyncTimeContext ( ucTimeContext );
            pPlayer->SetParent ( g_pClientGame->m_pRootEntity );

            // Store the nick and if he's dead
            pPlayer->SetNick ( szNickBuffer );
            pPlayer->SetDeadOnNetwork ( false );

            if ( szNametagText [ 0 ] )
                pPlayer->SetNametagText ( szNametagText );

            // Set the nametag override color if it's overridden
            if ( bHasNametagColorOverridden )
            {
                pPlayer->SetNametagOverrideColor ( ucNametagR, ucNametagG, ucNametagB );
            }

            pPlayer->SetNametagShowing ( bNametagShowing );

            CClientTeam* pTeam = NULL;
            if ( TeamID != INVALID_ELEMENT_ID )
                pTeam = g_pClientGame->m_pTeamManager->GetTeam ( TeamID );

            if ( pTeam )
                pPlayer->SetTeam ( pTeam, true );

            // If the player has spawned
            if ( bIsSpawned )
            {
                // Give him the correct skin
                pPlayer->SetModel ( usPlayerModelID );

                // Not in a vehicle?
                if ( ID == INVALID_ELEMENT_ID )
                {
                    pPlayer->SetPosition ( vecPosition );
                    pPlayer->SetCurrentRotation ( fRotation );
                    pPlayer->SetCameraRotation ( fRotation );
                    pPlayer->ResetInterpolation ();
                    pPlayer->SetHasJetPack ( bHasJetPack );
                }
                else        // In a vehicle
                {
                    // Grab the vehicle and warp him into it
                    CClientVehicle* pVehicle = g_pClientGame->m_pVehicleManager->Get ( ID );
                    if ( pVehicle )
                    {
                        pPlayer->WarpIntoVehicle ( pVehicle, ucVehicleSeat );
                    }
                }

                pPlayer->SetDimension ( usDimension );
                pPlayer->SetFightingStyle ( ( eFightingStyle ) ucFightingStyle );
                pPlayer->SetAlpha ( ucAlpha );
                pPlayer->SetInterior ( ucInterior );
            }

            // Print the join message in the chat
            if ( bJustJoined )
            {
                // Call the onClientPlayerJoin event
                CLuaArguments Arguments;
                pPlayer->CallEvent ( "onClientPlayerJoin", Arguments, true );
            }
        }
        else
        {
            RaiseFatalError ( 5 );
        }
    }
}


void CPacketHandler::Packet_PlayerQuit ( NetBitStreamInterface& bitStream )
{
    // unsigend char (1)    - player id

    if ( g_pClientGame->m_Status != CClientGame::STATUS_JOINED )
    {
        RaiseProtocolError ( 13 );
        return;
    }

    // Read out the id
    ElementID PlayerID;
    unsigned char ucReason;
    if ( bitStream.Read ( PlayerID ) &&
         bitStream.Read ( ucReason ) )
    {
        // Look up the player in the playermanager
        CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->Get ( PlayerID );
        if ( pPlayer )
        {
            // Quit him
            CClientGame::eQuitReason Reason = static_cast < CClientGame::eQuitReason > ( ucReason );
            g_pClientGame->QuitPlayer ( pPlayer, Reason );
        }
    }
    else
    {
        RaiseProtocolError ( 15 );
    }
}


void CPacketHandler::Packet_PlayerSpawn ( NetBitStreamInterface& bitStream )
{
    // unsigned char    (1)     - player id
    // bool                     - in vehicle?
    // CVector          (12)    - spawn position
    // float            (4)     - spawn rotation
    // unsigned char    (1)     - player model id
    // unsigned char    (1)     - interior
    // unsigned short   (2)     - dimension
    // unsigned short   (2)     - team id
    // unsigned char    (1)     - vehicle id (minus 400) (if vehicle)
    // unsigned char    (1)     - color 1 (if vehicle)
    // unsigned char    (1)     - color 2 (if vehicle)
    // unsigned char    (1)     - color 3 (if vehicle)
    // unsigned char    (1)     - color 4 (if vehicle)

    // Read out the player id
    ElementID PlayerID;
    bitStream.Read ( PlayerID );

    // Flags
    unsigned char ucFlags;
    bitStream.Read ( ucFlags );

    // Decode the flags
    bool bInVehicle = ucFlags & 0x01;

    // Position vector
    CVector vecPosition;
    bitStream.Read ( vecPosition.fX );
    bitStream.Read ( vecPosition.fY );
    bitStream.Read ( vecPosition.fZ );

    // Rotation short
    float fRotation;
    bitStream.Read ( fRotation );

    // Player model id
    unsigned short usPlayerModelID;
    bitStream.Read ( usPlayerModelID );
    if ( !CClientPlayerManager::IsValidModel ( usPlayerModelID ) )
    {
        RaiseProtocolError ( 16 );
        return;
    }

    // Interior
    unsigned char ucInterior = 0;
    bitStream.Read ( ucInterior );

    // Dimension
    unsigned short usDimension = 0;
    bitStream.Read ( usDimension );

    // Team id
    ElementID TeamID = INVALID_ELEMENT_ID;
    CClientTeam* pTeam = NULL;
    bitStream.Read ( TeamID );
    if ( TeamID != INVALID_ELEMENT_ID )
        pTeam = g_pClientGame->m_pTeamManager->GetTeam ( TeamID );

    // Time context
    unsigned char ucTimeContext = 0;
    bitStream.Read ( ucTimeContext ) ;

    // Read out some vehicle stuff if we spawn in a vehicle
    unsigned short usModel = 0;
    unsigned char ucColor1 = 0;
    unsigned char ucColor2 = 0;
    unsigned char ucColor3 = 0;
    unsigned char ucColor4 = 0;
    if ( bInVehicle )
    {
        // Read out the vehicle id
        unsigned char ucModel = 0xFF;
        bitStream.Read ( ucModel );
        if ( ucModel == 0xFF )
        {
            RaiseProtocolError ( 17 );
            return;
        }

        // Convert to a short. Valid id?
        usModel = ucModel + 400;
        if ( !CClientVehicleManager::IsValidModel ( usModel ) )
        {
            RaiseProtocolError ( 18 );
            return;
        }

        // Read out the colors
        bitStream.Read ( ucColor1 );
        bitStream.Read ( ucColor2 );
        bitStream.Read ( ucColor3 );
        bitStream.Read ( ucColor4 );
    }

    // Grab the player this is about
    CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->Get ( PlayerID );
    if ( pPlayer )
    {
        // Set the team we were given and update the sync time context to prevent/
        // the player sliding from his last position to the new.
        pPlayer->SetSyncTimeContext ( ucTimeContext );
        pPlayer->SetTeam ( pTeam, true );

        // He's no longer dead
        pPlayer->SetDeadOnNetwork ( false );

        // Spawn him
        pPlayer->Spawn ( vecPosition,
                         fRotation,
                         usPlayerModelID,
                         ucInterior );

        // Set his dimension
        pPlayer->SetDimension ( usDimension );

        // Is it the local player?
        if ( pPlayer->IsLocalPlayer () )
        {
            // Reset vehicle in/out stuff
            g_pClientGame->ResetVehicleInOut ();

            // Make sure all other elements are aware of what dimension we are in now.
            // So elements can be removed properly.
            g_pClientGame->SetAllDimensions ( usDimension );

            // Reset return position so we can't warp back to where we were if local player
            g_pClientGame->m_pNetAPI->ResetReturnPosition ();       
        }
        else
        {
            // Set now as the last puresync time
            pPlayer->SetLastPuresyncTime ( CClientTime::GetTime () );

            // Lock the health to full if it's not the local player
            pPlayer->LockHealth ( 100.0f );
        }

        // Call the onClientPlayerSpawn lua event
        CLuaArguments Arguments;
        if ( pTeam )
            Arguments.PushElement ( pTeam );
        else
            Arguments.PushBoolean ( false );
        pPlayer->CallEvent ( "onClientPlayerSpawn", Arguments, true );
    }
}


void CPacketHandler::Packet_PlayerWasted ( NetBitStreamInterface& bitStream )
{
    ElementID ID;
    ElementID KillerID;
    unsigned char ucWeapon;
    unsigned char ucBodyPart;
    unsigned char ucStealth;
    unsigned char ucTimeContext;
    AssocGroupId animGroup;
    AnimationId animID;
    if ( bitStream.Read ( ID ) &&
         bitStream.Read ( KillerID ) &&
         bitStream.Read ( ucWeapon ) &&
         bitStream.Read ( ucBodyPart ) &&
         bitStream.Read ( ucStealth ) &&
         bitStream.Read ( ucTimeContext ) &&
         bitStream.Read ( animGroup ) &&
         bitStream.Read ( animID ) )
    {
        // Grab the ped that was killed
        CClientPed * pPed = g_pClientGame->GetPedManager ()->Get ( ID, true );
        CClientEntity * pKiller = ( KillerID != INVALID_ELEMENT_ID ) ? CElementIDs::GetElement ( KillerID ) : NULL;
        if ( pPed )
        {
            // Mark him as dead?
            if ( IS_PLAYER ( pPed ) )
                static_cast < CClientPlayer * > ( pPed )->SetDeadOnNetwork ( true );

            // Update our sync-time context
            pPed->SetSyncTimeContext ( ucTimeContext );

            // Is this a stealth kill? and do we have a killer ped?
            if ( ucStealth == 1 && pKiller && IS_PED ( pKiller ) )
			{
                // Make our killer ped do the stealth kill animation
				CClientPed* pKillerPed = static_cast < CClientPed * > ( pKiller );
				pKillerPed->StealthKill ( pPed );				
			}
            // Kill our ped in the correct way
            pPed->Kill ( ( eWeaponType ) ucWeapon, ucBodyPart, ( ucStealth == 1 ), animGroup, animID );

            // Silently remove his satchels
            pPed->DestroySatchelCharges ( false, true );
			
            // Call the onClientPlayerWasted event
            CLuaArguments Arguments;
            if ( pKiller ) Arguments.PushElement ( pKiller );
            else Arguments.PushBoolean ( false );
            if ( ucWeapon != 0xFF ) Arguments.PushNumber ( ucWeapon );
            else Arguments.PushBoolean ( false );
            if ( ucBodyPart != 0xFF ) Arguments.PushNumber ( ucBodyPart );
            else Arguments.PushBoolean ( false );
			Arguments.PushBoolean ( ( ucStealth == 1 ) );

            pPed->CallEvent ( "onClientPedWasted", Arguments, true );
        }
    }
    else
    {
        RaiseProtocolError ( 21 );
    }
}


void CPacketHandler::Packet_PlayerChangeNick ( NetBitStreamInterface& bitStream )
{
    // ElementID        (2)     - player id
    // unsigned char    (1)     - new nick length
    // char             (x)     - new nick

    // We should be joined
    if ( g_pClientGame->m_Status != CClientGame::STATUS_JOINED )
    {
        RaiseProtocolError ( 22 );
        return;
    }

    // Read out the player ID
    ElementID PlayerID;
    if ( !bitStream.Read ( PlayerID ) )
    {
        RaiseProtocolError ( 23 );
        return;
    }

    // Check how many bytes with nick we got to read
    unsigned int uiNickLength = bitStream.GetNumberOfBytesUsed () - sizeof ( ElementID );
    if ( uiNickLength < MIN_PLAYER_NICK_LENGTH || uiNickLength > MAX_PLAYER_NICK_LENGTH )
    {
        RaiseProtocolError ( 24 );
        return;
    }

    // Read out the new nick
    char szNewNick [MAX_PLAYER_NICK_LENGTH + 1];
    memset ( szNewNick, 0, sizeof ( szNewNick ) );
    bitStream.Read ( szNewNick, uiNickLength );

    // Check that it's valid
    if ( !g_pClientGame->IsNickValid ( szNewNick ) )
    {
        RaiseProtocolError ( 25 );
        return;
    }

    // Grab the player
    CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->Get ( PlayerID );
    if ( !pPlayer )
    {
        return;
    }

    const char * szOldNick = pPlayer->GetNickPointer ();
    char * szOldNickCopy = NULL;

    if ( NULL != szOldNick )
    {
		// If the nametag wasn't changed by a script, update it
		const char* szOldNametag = pPlayer->GetNametagText ();
		if ( szOldNametag && strcmp ( szOldNametag, szOldNick ) == 0 )
			pPlayer->SetNametagText ( szNewNick );

		// Copy the old player name.
        int nOldNickLen = strlen(szOldNick);
        szOldNickCopy = new char [ nOldNickLen + 1 ];
        strncpy ( szOldNickCopy, szOldNick, nOldNickLen );
        if (nOldNickLen)
            szOldNickCopy[nOldNickLen] = 0;
    }

    /*
     * Set the new player name.
     */
	pPlayer->SetNick ( szNewNick );

    /*
     * Update the local player name in the UI
     * if necessary.
     */
    if ( pPlayer->IsLocalPlayer () )
    {
        g_pCore->GetCVars ()->Set ( "nick", std::string ( szNewNick ) );
    }

    /*
     * Call the lua function callback 'onClientPlayerChangeNick'
     * to notify client scripts.
     */
    CLuaArguments Arguments;
    if ( szOldNickCopy )
    {
        Arguments.PushString ( szOldNickCopy );
        Arguments.PushString ( szNewNick );
    }
    else
    {
        Arguments.PushBoolean ( false );
    }
    pPlayer->CallEvent ( "onClientPlayerChangeNick", Arguments, true );

    /*
     * Cleanup.
     */
    delete [] szOldNickCopy;
}


void CPacketHandler::Packet_PlayerPings ( NetBitStreamInterface& bitStream )
{
    // unsigned char    (1) - player id
    // unsigned char    (1) - player ping (only a 4th of the real ping)
    // ...

    // We should be ingame
    if ( g_pClientGame->m_Status == CClientGame::STATUS_JOINED )
    {
        // Read out each ping
        while ( bitStream.GetReadOffsetAsBits () < bitStream.GetNumberOfBitsUsed () )
        {
            // Read out the data
            ElementID PlayerID;
            unsigned short usPing;
            if ( bitStream.Read ( PlayerID ) &&
                 bitStream.Read ( usPing ) )
            {
                // Grab the player
                CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->Get ( PlayerID );
                if ( pPlayer )
                {
                    // Set the new ping
                    pPlayer->SetPing ( usPing );
                }
            }
        }
    }
}


void CPacketHandler::Packet_ChatEcho ( NetBitStreamInterface& bitStream )
{
    // unsigned char    (1)     - red
    // unsigned char    (1)     - green
    // unsigned char    (1)     - blue
    // unsigned char    (1)     - color-coded
    // unsigned char    (x)     - message

    // Read out the color
    unsigned char ucRed;
    unsigned char ucGreen;
    unsigned char ucBlue;
    unsigned char ucColorCoded;
    if ( bitStream.Read ( ucRed ) &&
         bitStream.Read ( ucGreen ) &&
         bitStream.Read ( ucBlue ) &&
         bitStream.Read ( ucColorCoded ) )
    {
        // Valid length?
        int iNumberOfBytesUsed = bitStream.GetNumberOfBytesUsed () - 4;
        if ( iNumberOfBytesUsed >= MIN_CHATECHO_LENGTH && iNumberOfBytesUsed <= MAX_CHATECHO_LENGTH )
        {
            // Read the message into a buffer
            char szMessage [MAX_CHATECHO_LENGTH + 1];
            bitStream.Read ( szMessage, iNumberOfBytesUsed );
            szMessage [iNumberOfBytesUsed] = 0;

            // Strip it for bad characters
            StripUnwantedCharacters ( szMessage, ' ' );

			// Call an event
			CLuaArguments Arguments;
			Arguments.PushString ( szMessage );
			Arguments.PushNumber ( ucRed );
			Arguments.PushNumber ( ucGreen );
			Arguments.PushNumber ( ucBlue );
			g_pClientGame->GetRootEntity()->CallEvent ( "onClientChatMessage", Arguments, false );

            // Echo it
            g_pCore->ChatEchoColor ( szMessage, ucRed, ucGreen, ucBlue, ( ucColorCoded == 1 ) );
        }
    }
}


void CPacketHandler::Packet_ConsoleEcho ( NetBitStreamInterface& bitStream )
{
    // unsigned char    (x)     - message

    // Valid length?
    int iNumberOfBytesUsed = bitStream.GetNumberOfBytesUsed ();
    if ( iNumberOfBytesUsed >= MIN_CONSOLEECHO_LENGTH && iNumberOfBytesUsed <= MAX_CONSOLEECHO_LENGTH )
    {
        // Read the message into a buffer
        char szMessage [MAX_CONSOLEECHO_LENGTH + 1];
        bitStream.Read ( szMessage, iNumberOfBytesUsed );
        szMessage [iNumberOfBytesUsed] = 0;

        // Strip it for bad characters
        StripUnwantedCharacters ( szMessage, ' ' );

        // Echo it
        g_pCore->GetConsole ()->Echo ( szMessage );
    }
}


void CPacketHandler::Packet_DebugEcho ( NetBitStreamInterface& bitStream )
{
    // unsigned char    (1)     - level
    // unsigned char    (1)     - red
    // unsigned char    (1)     - green
    // unsigned char    (1)     - blue
    // unsigned char    (x)     - message

    unsigned char ucLevel;
    unsigned char ucRed;
    unsigned char ucGreen;
    unsigned char ucBlue;

    // Read out the level
    if ( !bitStream.Read ( ucLevel ) )
        return;

    if ( ucLevel == 0 )
    {
        // Read out the color
        if ( !bitStream.Read ( ucRed ) ||
             !bitStream.Read ( ucGreen ) ||
             !bitStream.Read ( ucBlue ) )
        {
            return;
        }
    }

    // Valid length?
    int iBytesRead = ( ucLevel == 0 ) ? 4 : 1;
    int iNumberOfBytesUsed = bitStream.GetNumberOfBytesUsed () - iBytesRead;
    if ( iNumberOfBytesUsed >= MIN_DEBUGECHO_LENGTH && iNumberOfBytesUsed <= MAX_DEBUGECHO_LENGTH )
    {
        // Read the message into a buffer
        char szMessage [MAX_DEBUGECHO_LENGTH + 1];
        bitStream.Read ( szMessage, iNumberOfBytesUsed );
        szMessage [iNumberOfBytesUsed] = 0;

        // Strip it for bad characters
        StripUnwantedCharacters ( szMessage, ' ' );

        switch ( ucLevel )
        {
            case 1:
                ucRed = 255, ucGreen = 0, ucBlue = 0;
                break;
            case 2:
                ucRed = 255, ucGreen = 128, ucBlue = 0;
                break;
            case 3:
                ucRed = 0, ucGreen = 255, ucBlue = 0;
                break;
        }

        // Echo it
        g_pCore->DebugEchoColor ( szMessage, ucRed, ucGreen, ucBlue );

        // Output it to the file if need be
        std::string strFileName;
        g_pCore->GetCVars ()->Get ( "debugfile", strFileName );

        if ( !strFileName.empty () )
        {
            // get the date/time now
            struct tm *today;
            time_t ltime;
            time(&ltime);
            today = localtime(&ltime);
            char date[100];
            strftime(date, 30, "%Y-%m-%d %H:%M:%S", today);
            date[29] = '\0';

            // open the file for append access
            FILE * pFile = fopen ( strFileName.c_str (), "a" );
            if ( pFile )
            {
                // write out the data
                fprintf ( pFile, "[%s] %s\n", date, szMessage );
                fclose ( pFile );
            }
        }
    }
}


void CPacketHandler::Packet_VehicleSpawn ( NetBitStreamInterface& bitStream )
{
    // ElementID        (2)     - vehicle id
    // CVector          (12)    - position
    // CVector          (12)    - rotation in degrees
    // unsigned char    (1)     - color 1
    // unsigned char    (1)     - color 2
    // unsigned char    (1)     - color 3
    // unsigned char    (1)     - color 4
    // Repeats ...

    // While there are bytes left in the packet
    while ( bitStream.GetNumberOfUnreadBits () > 16 )
    {
        // Read out the vehicle ID
        ElementID ID = INVALID_ELEMENT_ID;
        bitStream.Read ( ID );

        // Grab it
        CClientVehicle* pVehicle = g_pClientGame->m_pVehicleManager->Get ( ID );
        if ( !pVehicle )
        {
            return;
        }

        // Read out the time context
        unsigned char ucTimeContext;
        bitStream.Read ( ucTimeContext );
        pVehicle->SetSyncTimeContext ( ucTimeContext );

        // Read out the position
        CVector vecPosition;
        bitStream.Read ( vecPosition.fX );
        bitStream.Read ( vecPosition.fY );
        bitStream.Read ( vecPosition.fZ );

        // Read out the rotation
        CVector vecRotationDegrees;
        bitStream.Read ( vecRotationDegrees.fX );
        bitStream.Read ( vecRotationDegrees.fY );
        bitStream.Read ( vecRotationDegrees.fZ );

        // Set it
        pVehicle->SetPosition ( vecPosition );
        pVehicle->SetRotationDegrees ( vecRotationDegrees );

        // Make sure its stationary
        pVehicle->SetMoveSpeed ( CVector ( 0.0f, 0.0f, 0.0f ) );
        pVehicle->SetTurnSpeed ( CVector ( 0.0f, 0.0f, 0.0f ) );

        // Read out the color
        unsigned char ucColor1 = 0;
        bitStream.Read ( ucColor1 );

        unsigned char ucColor2 = 0;
        bitStream.Read ( ucColor2 );

        unsigned char ucColor3 = 0;
        bitStream.Read ( ucColor3 );

        unsigned char ucColor4 = 0;
        bitStream.Read ( ucColor4 );

        // Set the color
        pVehicle->SetColor ( ucColor1, ucColor2, ucColor3, ucColor4 );

        // Fix its damage and reset things such as landing gears
        pVehicle->Fix ();
        pVehicle->SetLandingGearDown ( true );
        pVehicle->SetTurretRotation ( 0.0f, 0.0f );
        pVehicle->SetSirenOrAlarmActive ( false );
        pVehicle->SetAdjustablePropertyValue ( 0 );

        // Call the onClientVehicleRespawn event
        CLuaArguments Arguments;
        pVehicle->CallEvent ( "onClientVehicleRespawn", Arguments, true );
    }
}


void CPacketHandler::Packet_VehicleDamageSync ( NetBitStreamInterface& bitStream )
{
    // Read out the vehicle id and the damage sync model
    ElementID ID;
    unsigned char ucDoorStates [MAX_DOORS];
    unsigned char ucWheelStates [MAX_WHEELS];
    unsigned char ucPanelStates [MAX_PANELS];
    unsigned char ucLightStates [MAX_LIGHTS];
    if ( bitStream.Read ( ID ) &&
         bitStream.Read ( (char*) ucDoorStates, MAX_DOORS ) &&
         bitStream.Read ( (char*) ucWheelStates, MAX_WHEELS ) &&
         bitStream.Read ( (char*) ucPanelStates, MAX_PANELS ) &&
         bitStream.Read ( (char*) ucLightStates, MAX_LIGHTS ) )
    {
        // Grab the vehicle
        CDeathmatchVehicle* pVehicle = static_cast < CDeathmatchVehicle* > ( g_pClientGame->m_pVehicleManager->Get ( ID ) );
        if ( pVehicle )
        {
            for ( int i = 0 ; i < MAX_DOORS ; i++ ) pVehicle->SetDoorStatus ( i, ucDoorStates [ i ] );
            for ( int i = 0 ; i < MAX_WHEELS ; i++ ) pVehicle->SetWheelStatus ( i, ucWheelStates [ i ], false );
            for ( int i = 0 ; i < MAX_PANELS ; i++ ) pVehicle->SetPanelStatus ( i, ucPanelStates [ i ] );
            for ( int i = 0 ; i < MAX_LIGHTS ; i++ ) pVehicle->SetLightStatus ( i, ucLightStates [ i ] );

            pVehicle->ResetDamageModelSync ();
        }
    }
}


void CPacketHandler::Packet_Vehicle_InOut ( NetBitStreamInterface& bitStream )
{
    // unsigned char  (1)   - player id
    // ElementID      (2)   - vehicle id
    // unsigned char  (1)   - action   

    // Read out the player id
    ElementID PlayerID;
    if ( bitStream.Read ( PlayerID ) )
    {
        CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->Get ( PlayerID );
        if ( pPlayer )
        {
            // Set now as his last puresync time
            pPlayer->SetLastPuresyncTime ( CClientTime::GetTime () );

            // Read out the vehicle id
            ElementID ID = INVALID_ELEMENT_ID;
            bitStream.Read ( ID );
            CClientVehicle* pVehicle = g_pClientGame->m_pVehicleManager->Get ( ID );
            if ( pVehicle )
            {
                // Read out the seat id
                unsigned char ucSeat = 0xFF;
                bitStream.Read ( ucSeat );
                if ( ucSeat == 0xFF )
                {
                    RaiseProtocolError ( 28 );
                    return;
                }

                // Read out the action
                unsigned char ucAction = 0xFF;
                bitStream.Read ( ucAction );

#ifdef MTA_DEBUG
                if ( pPlayer->IsLocalPlayer () )
                {
                    char * actions [] = { "request_in_confirmed", "notify_in_return",
                                          "notify_in_abort_return", "request_out_confirmed",
                                          "notify_out_return", "notify_out_abort_return",
                                          "notify_fell_off_return", "request_jack_confirmed",
                                          "notify_jack_return", "attempt_failed" };
                    g_pCore->GetConsole ()->Printf ( "* Packet_InOut: %s", actions [ ucAction ] );
                }
#endif

                switch ( ucAction )
                {
                    case CClientGame::VEHICLE_REQUEST_IN_CONFIRMED:
                    {
                        // If it's the local player, set some stuff
                        if ( pPlayer->IsLocalPlayer () )
                        {
                            // Set the vehicle id and the seat we're about to enter
                            g_pClientGame->m_VehicleInOutID = ID;
                            g_pClientGame->m_ucVehicleInOutSeat = ucSeat;

                            /*
                            // Make it damagable
                            static_cast < CDeathmatchVehicle* > ( pVehicle )->SetCanBeDamaged ( true );
                            static_cast < CDeathmatchVehicle* > ( pVehicle )->SetTyresCanBurst ( true );
                            */
                        }

                        // Start animating him in
                        pPlayer->GetIntoVehicle ( pVehicle, ucSeat );

                        // Remember that this player is working on entering a vehicle
                        pPlayer->SetVehicleInOutState ( VEHICLE_INOUT_GETTING_IN );

                        // Call the onClientVehicleStartEnter event
                        CLuaArguments Arguments;
                        Arguments.PushElement ( pPlayer );     // player
                        Arguments.PushNumber ( ucSeat );        // seat
                        pVehicle->CallEvent ( "onClientVehicleStartEnter", Arguments, true );
                        break;
                    }

                    case CClientGame::VEHICLE_NOTIFY_IN_RETURN:
                    {
                        // Is he not getting in the vehicle yet?
                        //if ( !pPlayer->IsGettingIntoVehicle () )
                        {
                            // Warp him in
                            pPlayer->WarpIntoVehicle ( pVehicle, ucSeat );
                        }

                        // Reset vehicle in out state
                        pPlayer->SetVehicleInOutState ( VEHICLE_INOUT_NONE );

                        // If local player, we are now allowed to exit it again
                        if ( pPlayer->IsLocalPlayer () )
                            g_pClientGame->m_bNoNewVehicleTask = false;

                        // Call the onClientPlayerEnterVehicle event
                        CLuaArguments Arguments;
                        Arguments.PushElement ( pVehicle );        // vehicle
                        Arguments.PushNumber ( ucSeat );            // seat
                        pPlayer->CallEvent ( "onClientPlayerVehicleEnter", Arguments, true );

                        // Call the onClientVehicleEnter event
                        CLuaArguments Arguments2;
                        Arguments2.PushElement ( pPlayer );        // player
                        Arguments2.PushNumber ( ucSeat );           // seat
                        pVehicle->CallEvent ( "onClientVehicleEnter", Arguments2, true );
                        break;
                    }

                    case CClientGame::VEHICLE_NOTIFY_IN_ABORT_RETURN:
                    {
                        // If local player, we are now allowed to enter it again
                        if ( pPlayer->IsLocalPlayer () )
                        {
                            g_pClientGame->ResetVehicleInOut ();
                        }
                        else
                        {
                            // Was he jacking?
                            if ( pPlayer->GetVehicleInOutState () == VEHICLE_INOUT_JACKING )
                            {
                                // Grab the player model getting jacked
                                CClientPed* pJacked = pVehicle->GetOccupant ( ucSeat );

                                // If it's the local player jacking, set some stuff
                                if ( pJacked && pJacked->IsLocalPlayer () )
                                {
                                    // Set the vehicle id we're about to enter and that we're jacking
                                    g_pClientGame->ResetVehicleInOut ();
                                }
                            }
                        }


                        // Make sure he's removed from the vehicle
                        pPlayer->RemoveFromVehicle ();

                        // Reset vehicle in out state
                        pPlayer->SetVehicleInOutState ( VEHICLE_INOUT_NONE );                        
                        break;
                    }

                    case CClientGame::VEHICLE_REQUEST_OUT_CONFIRMED:
                    {
                        // If it's the local player, set some stuff
                        if ( pPlayer->IsLocalPlayer () )
                        {
                            // Set the vehicle id and the seat we're about to exit from
                            g_pClientGame->m_VehicleInOutID = ID;
                            g_pClientGame->m_ucVehicleInOutSeat = ucSeat;
                        }

                        pPlayer->GetOutOfVehicle ();

                        // Remember that this player is working on leaving a vehicle
                        pPlayer->SetVehicleInOutState ( VEHICLE_INOUT_GETTING_OUT );

                        CLuaArguments Arguments;
                        Arguments.PushElement ( pPlayer );         // player
                        Arguments.PushNumber ( ucSeat );            // seat
                        pVehicle->CallEvent ( "onClientVehicleStartExit", Arguments, true );
                        break;
                    }

                    case CClientGame::VEHICLE_NOTIFY_OUT_RETURN:
                    {
                        // If local player, we are now allowed to enter it again
                        if ( pPlayer->IsLocalPlayer () )
                            g_pClientGame->ResetVehicleInOut ();

                        // Make sure we're removed from the vehicle
                        bool bDontWarpIfGettingDraggedOut = pPlayer->IsLocalPlayer ();
                        pPlayer->RemoveFromVehicle ( bDontWarpIfGettingDraggedOut );

                        if ( ucSeat == 0 )
                            pVehicle->RemoveTargetPosition ();

                        // Reset the vehicle in out state
                        pPlayer->SetVehicleInOutState ( VEHICLE_INOUT_NONE );

                        // Call the onClientPlayerExitVehicle event
                        CLuaArguments Arguments;
                        Arguments.PushElement ( pVehicle );        // vehicle
                        Arguments.PushNumber ( ucSeat );            // seat
                        Arguments.PushBoolean ( false );            // jacker
                        pPlayer->CallEvent ( "onClientPlayerVehicleExit", Arguments, true );

                        // Call the onClientVehicleExit event
                        CLuaArguments Arguments2;
                        Arguments2.PushElement ( pPlayer );         // player
                        Arguments2.PushNumber ( ucSeat );           // seat
                        Arguments2.PushBoolean ( false );            // jacker
                        pVehicle->CallEvent ( "onClientVehicleExit", Arguments2, true );
                        break;
                    }

                    case CClientGame::VEHICLE_NOTIFY_OUT_ABORT_RETURN:
                    {
                        // If local player, we are now allowed to enter it again
                        if ( pPlayer->IsLocalPlayer () )
                            g_pClientGame->ResetVehicleInOut ();

                        // Warp into the vehicle again
                        pPlayer->WarpIntoVehicle ( pVehicle, ucSeat );

                        // Reset vehicle in out state
                        pPlayer->SetVehicleInOutState ( VEHICLE_INOUT_NONE );
                        break;
                    }

                    case CClientGame::VEHICLE_NOTIFY_FELL_OFF_RETURN:
                    {
                        // If local player, we are now allowed to enter it again
                        if ( pPlayer->IsLocalPlayer () )
                            g_pClientGame->ResetVehicleInOut ();

                        // Remove from the vehicle to be sure
                        pPlayer->RemoveFromVehicle ();

                        // Reset vehicle in out state
                        pPlayer->SetVehicleInOutState ( VEHICLE_INOUT_NONE );

                        if ( ucSeat == 0 )
                            pVehicle->RemoveTargetPosition ();
                        break;
                    }

                    case CClientGame::VEHICLE_REQUEST_JACK_CONFIRMED:
                    {
                        // Grab the player model getting jacked
                        CClientPed* pJacked = pVehicle->GetOccupant ( ucSeat );

                        // If it's the local player jacking, set some stuff
                        if ( pPlayer->IsLocalPlayer () )
                        {
                            // Set the vehicle id we're about to enter and that we're jacking
                            g_pClientGame->m_VehicleInOutID = ID;
                            g_pClientGame->m_ucVehicleInOutSeat = ucSeat;
                            g_pClientGame->m_bIsJackingVehicle = true;
                        }
                        else
                        {
                            // It's the local player getting jacked?
                            if ( pJacked && pJacked->IsLocalPlayer () )
                            {
                                g_pClientGame->m_bIsGettingJacked = true;
                                g_pClientGame->m_pGettingJackedBy = pPlayer;
                            }
                        }

                        // Remember that this player is working on leaving a vehicle
                        if ( pJacked )
                            pJacked->SetVehicleInOutState ( VEHICLE_INOUT_GETTING_JACKED );

                        // Start animating him in
                        pPlayer->GetIntoVehicle ( pVehicle, ucSeat );

                        // Remember that this player is working on leaving a vehicle
                        pPlayer->SetVehicleInOutState ( VEHICLE_INOUT_JACKING );

                        // Call the onClientVehicleStartEnter event
                        CLuaArguments Arguments;
                        Arguments.PushElement ( pPlayer );     // player
                        Arguments.PushNumber ( ucSeat );        // seat
                        pVehicle->CallEvent ( "onClientVehicleStartEnter", Arguments, true );

                        CLuaArguments Arguments2;
                        Arguments2.PushElement ( pJacked );         // player
                        Arguments2.PushNumber ( ucSeat );            // seat
                        pVehicle->CallEvent ( "onClientVehicleStartExit", Arguments2, true );
                        break;
                    }

                    case CClientGame::VEHICLE_NOTIFY_JACK_RETURN:
                    {
                        // Read out the player that's going into the vehicle
                        ElementID PlayerInside = INVALID_ELEMENT_ID;
                        bitStream.Read ( PlayerInside );
                        CClientPlayer* pInsidePlayer = g_pClientGame->m_pPlayerManager->Get ( PlayerInside );
                        if ( pInsidePlayer )
                        {
                            // And the one dumping out on the outside
                            ElementID PlayerOutside = INVALID_ELEMENT_ID;
                            bitStream.Read ( PlayerOutside );
                            CClientPlayer* pOutsidePlayer = g_pClientGame->m_pPlayerManager->Get ( PlayerOutside );
                            if ( pOutsidePlayer )
                            {
                                // If local player, we are now allowed to enter it again
                                if ( pInsidePlayer->IsLocalPlayer () ||
                                        pOutsidePlayer->IsLocalPlayer () )
                                {
                                    g_pClientGame->ResetVehicleInOut ();
                                }

                                // Warp him out
                                bool bDontWarpIfGettingDraggedOut = pOutsidePlayer->IsLocalPlayer ();
                                pOutsidePlayer->RemoveFromVehicle ( bDontWarpIfGettingDraggedOut );

								// Reset interpolation so he won't appear on the roof of the vehicle until next sync
								pOutsidePlayer->RemoveTargetPosition ();

                                // Reset vehicle in out state
                                pOutsidePlayer->SetVehicleInOutState ( VEHICLE_INOUT_NONE );

                                // Is the outside player us? Reset the jacking flags
                                if ( pOutsidePlayer->IsLocalPlayer () )
                                {
                                    g_pClientGame->m_bIsGettingJacked = false;
                                    g_pClientGame->m_bIsJackingVehicle = false;
                                }

                                // Is the inside player us? Reset the jacking flags
                                if ( pInsidePlayer->IsLocalPlayer () )
                                {
                                    g_pClientGame->m_bIsGettingJacked = false;
                                    g_pClientGame->m_bIsJackingVehicle = false;
                                }

                                // Reset vehicle in out state
                                pInsidePlayer->SetVehicleInOutState ( VEHICLE_INOUT_NONE );

                                // Is he not getting in the vehicle yet?
                                //if ( !pPlayer->IsGettingIntoVehicle () )
                                {
                                    // Warp him in
                                    pInsidePlayer->WarpIntoVehicle ( pVehicle, ucSeat );
                                }
                            }
                        }

                        break;
                    }

                    case CClientGame::VEHICLE_ATTEMPT_FAILED:
                    {
                        // Reset in/out stuff if it was the local player
                        if ( pPlayer->IsLocalPlayer () )
                        {
                            g_pClientGame->ResetVehicleInOut ();
                        }

                        // Reset vehicle in out state
                        pPlayer->SetVehicleInOutState ( VEHICLE_INOUT_NONE );

                        unsigned char ucReason;
                        bitStream.Read ( ucReason );
                            // Is the vehicle too far away?
                        if ( ucReason == 5 /*FAIL_DISTANCE*/ )
                        {
                            CVector vecPosition;
                            if ( bitStream.Read ( vecPosition.fX ) &&
                                    bitStream.Read ( vecPosition.fY ) &&
                                    bitStream.Read ( vecPosition.fZ ) )
                            {
                                // Make sure we have the right position for this vehicle
                                pVehicle->SetPosition ( vecPosition );
                            }
                        }
                        g_pCore->GetConsole ()->Printf ( "Failed to enter/exit vehicle - id: %u", ucReason );

                        break;
                    }

                    default:
                    {
                        RaiseProtocolError ( 33 );
                        break;
                    }
                }
            }
        }
    }
    else
    {
        RaiseProtocolError ( 37 );
    }
}


void CPacketHandler::Packet_VehicleTrailer ( NetBitStreamInterface& bitStream )
{
    // ElementID      (2)   - vehicle id
    // ElementID      (2)   - trailer id
    // unsigned char  (1)   - attached?
    // CVector (12)         - Position
    // CVector (12)         - Rotation in degrees
    // CVector (12)         - TurnSpeed

    ElementID ID, TrailerID;
    unsigned char ucAttached;
    CVector vecPosition, vecRotationDegrees;
    CVector vecTurnSpeed;
    if ( bitStream.Read ( ID ) &&
         bitStream.Read ( TrailerID ) &&
         bitStream.Read ( ucAttached ) &&
         bitStream.Read ( vecPosition.fX ) &&
         bitStream.Read ( vecPosition.fY ) &&
         bitStream.Read ( vecPosition.fZ ) &&
         bitStream.Read ( vecRotationDegrees.fX ) &&
         bitStream.Read ( vecRotationDegrees.fY ) &&
         bitStream.Read ( vecRotationDegrees.fZ ) &&
         bitStream.Read ( vecTurnSpeed.fX ) &&
         bitStream.Read ( vecTurnSpeed.fY ) &&
         bitStream.Read ( vecTurnSpeed.fZ ) )
    {
        CClientVehicle* pVehicle = g_pClientGame->m_pVehicleManager->Get ( ID );
        CClientVehicle* pTrailer = g_pClientGame->m_pVehicleManager->Get ( TrailerID );
        bool bAttached = ( ucAttached == 1 );
        if ( pVehicle && pTrailer )
        {
            if ( bAttached )
            {
                pVehicle->SetTowedVehicle ( pTrailer );

                // Call the onClientTrailerAttach
                CLuaArguments Arguments;
                Arguments.PushElement ( pVehicle );
                pTrailer->CallEvent ( "onClientTrailerAttach", Arguments, true );
            }
            else
            {
                pVehicle->SetTowedVehicle ( NULL );

                // Call the onClientTrailerDetach
                CLuaArguments Arguments;
                Arguments.PushElement ( pVehicle );
                pTrailer->CallEvent ( "onClientTrailerDetach", Arguments, true );
            }
        }
    }
}


void CPacketHandler::Packet_MapInfo ( NetBitStreamInterface& bitStream )
{
    // Read out the map weather
    unsigned char ucWeather, ucWeatherBlendingTo, ucBlendedWeatherHour;
    bitStream.Read ( ucWeather );
    bitStream.Read ( ucWeatherBlendingTo );
    bitStream.Read ( ucBlendedWeatherHour );

    // Is it blended?
    if ( ucBlendedWeatherHour != 0xFF )
    {
        // Go back one hour as we start the blending at that time
        if ( ucBlendedWeatherHour == 0 )
            ucBlendedWeatherHour = 23;
        else
            --ucBlendedWeatherHour;

        // Apply it blended
        g_pClientGame->m_pBlendedWeather->SetWeather ( ucWeather );
        g_pClientGame->m_pBlendedWeather->SetWeatherBlended ( ucWeatherBlendingTo, ucBlendedWeatherHour );
    }
    else
    {
        // Apply it immediately
        g_pClientGame->m_pBlendedWeather->SetWeather ( ucWeather );
    }

    // Read out the sky color
    unsigned char ucHasSkyGradient = 0;
    bitStream.Read ( ucHasSkyGradient );
    if ( ucHasSkyGradient )
    {
        unsigned char ucTopRed, ucTopGreen, ucTopBlue = 0;
        unsigned char ucBottomRed, ucBottomGreen, ucBottomBlue = 0;
        bitStream.Read ( ucTopRed );
        bitStream.Read ( ucTopGreen );
        bitStream.Read ( ucTopBlue );
        bitStream.Read ( ucBottomRed );
        bitStream.Read ( ucBottomGreen );
        bitStream.Read ( ucBottomBlue );
        g_pMultiplayer->SetSkyColor ( ucTopRed, ucTopGreen, ucTopBlue, ucBottomRed, ucBottomGreen, ucBottomBlue );
    }
    else
        g_pMultiplayer->ResetSky ();

    // Read out the map time
    unsigned char ucClockHour;
    unsigned char ucClockMinute;
    bitStream.Read ( ucClockHour );
    bitStream.Read ( ucClockMinute );

    // Set the time
    g_pGame->GetClock ()->Set ( ucClockHour, ucClockMinute );

    // Flags
    unsigned char ucFlags;
    bitStream.Read ( ucFlags );

    // Extract the flags
    g_pClientGame->m_bShowNametags = ( ucFlags & 0x01 ) ? true:false;
    g_pClientGame->m_bShowRadar = ( ucFlags & 0x02 ) ? true:false;

    // Read out the gravity
    float fGravity;
    bitStream.Read ( fGravity );
    g_pMultiplayer->SetGlobalGravity ( fGravity );

    // Read out the game speed
    float fGameSpeed;
    bitStream.Read ( fGameSpeed );
    g_pClientGame->SetGameSpeed ( fGameSpeed );

    // Read out the wave height
    float fWaveHeight = 0;
    bitStream.Read ( fWaveHeight );
    g_pMultiplayer->SetWaveLevel ( fWaveHeight );

	short sFPSLimit = 36;
	bitStream.Read ( sFPSLimit );

    unsigned int iVal;
    g_pCore->GetCVars ()->Get ( "fps_limit", iVal );

	if ( iVal > sFPSLimit )
    {
		// For some reason it needs that kind of hacky precision
		g_pGame->SetFramelimiter ( (unsigned long) ( (float)sFPSLimit * 1.333f ) );
    }
	else
    {
		g_pGame->SetFramelimiter ( (unsigned long) ( (float)iVal * 1.3f ) );
    }


    // Read out the garage door states
    CGarages* pGarages = g_pCore->GetGame()->GetGarages();
    for ( unsigned char i = 0 ; i < MAX_GARAGES ; i++ )
    {
        unsigned char ucGarageState;
        CGarage* pGarage = pGarages->GetGarage ( i );

        bitStream.Read ( ucGarageState );

        if ( pGarage )
        {
            pGarage->SetOpen ( (ucGarageState == 1) );
        }
    }
}


void CPacketHandler::Packet_EntityAdd ( NetBitStreamInterface& bitStream )
{
    // This packet contains a list over entities to add to the world.
    // There's a byte seperating the entities saying what type it is (vehicle spawn,object,weapon pickup)

    // Common:
    // ElementID            (2)     - entity id
    // unsigned char        (1)     - entity type id
    // ElementID            (2)     - parent entity id
    // unsigned char        (1)     - entity interior
    // unsigned short       (2)     - entity dimension
    // ElementID            (2)     - attached to entity id
    // ???                  (?)     - custom data

    // Objects:
    // CVector              (12)    - position
    // CVector              (12)    - rotation
    // unsigned short       (2)     - object model id

    // Pickups:
    // CVector              (12)    - position
    // unsigned char        (1)     - type
    // bool                         - visible?
    // unsigned char        (1)     - weapon type (if type is weapon)

    // Vehicles:
    // CMatrix              (48)    - matrix
    // unsigned char        (1)     - vehicle id
    // float                (4)     - health
    // unsigned char        (1)     - color 1
    // unsigned char        (1)     - color 2
    // unsigned char        (1)     - color 3
    // unsigned char        (1)     - color 4
    // unsigned char        (1)     - paintjob
    // float                (4)     - turret position x (if applies)
    // float                (4)     - turret position y (if applies)
    // unsigned short       (2)     - adjustable property (if applies)
    // bool                         - landing gear down?  (if applies)
    // bool                         - sirenes on?  (if applies)
    // unsigned char        (1)     - no. of upgrades
    // unsigned char        (1++)   - list of upgrades
    // unsigned char        (1)     - reg-plate length
    // char[]               (?)     - reg-plate
    // unsigned char        (1)     - light override
    // bool                         - can shoot petrol tank
    // bool                         - engine on
    // bool                         - locked
    // bool                         - doors damageable

    // Blips:
    // bool                         - attached to an entity?
    // -- following if attached:
    // unsigned char        (1)     - attached entity type
    // unsigned char/short  (1/2)   - attached entity id (char if player, otherwize short)
    // -- following if not attached:
    // CVector              (12)    - position
    // -- end
    // unsigned char        (1)     - icon
    // -- if icon is 0
    // unsigned char        (1)     - size
    // unsigned long        (4)     - color

    // Radar areas:
    // CVector2D            (8)     - position
    // CVector2D            (8)     - size
    // unsigned long        (4)     - color
    // bool                         - flashing?

    // Path Nodes:
    // CVector              (12)    - position
    // CVector              (12)    - rotation
    // int                  (4)     - time
    // unsigned char        (1)     - style
    // ElementID            (2)     - next-node id

    // World meshes
    // unsigned short       (2)     - name length
    // char[]               (?)     - name
    // CVector              (12)    - position
    // CVector              (12)    - rotation

    // Teams
    // unsigned short       (2)     - name length
    // char[]               (?)     - name
    // unsigned char[3]     (3)     - cols
    // unsigned char        (1)     - friendly-fire

    // Heavy variables
    CVector vecPosition;
    CVector vecRotation;
#ifdef MTA_DEBUG
    g_pCore->GetConsole()->Printf ( "Packet_EntityAdd" );
#endif
    // HACK: store new entities and link up anything depending on other entities after
    list < SEntityDependantStuff* > newEntitiesStuff;

    ElementID NumEntities = 0;
    if ( !bitStream.Read ( NumEntities ) || NumEntities == 0 )
        return;
#ifdef MTA_DEBUG
    g_pCore->GetConsole()->Printf ( "Going to add %d entities", NumEntities );
#endif
    for ( ElementID EntityIndex = 0 ; EntityIndex < NumEntities ; EntityIndex++ )
    {
#ifdef MTA_DEBUG
        g_pCore->GetConsole ()->Printf ( "  Adding entity %d", EntityIndex );
#endif
        // Read out the entity type id and the entity id
        ElementID EntityID;
        unsigned char ucEntityTypeID;
        ElementID ParentID;
        unsigned char ucInterior;
        unsigned short usDimension;
        ElementID EntityAttachedToID;
        CVector vecAttachedPosition, vecAttachedRotation;

        if ( bitStream.Read ( EntityID ) &&
             bitStream.Read ( ucEntityTypeID ) &&
             bitStream.Read ( ParentID ) &&
             bitStream.Read ( ucInterior ) &&
             bitStream.Read ( usDimension ) &&
             bitStream.Read ( EntityAttachedToID ) &&
             ( EntityAttachedToID == INVALID_ELEMENT_ID || ( bitStream.Read ( vecAttachedPosition.fX ) &&
                                                   bitStream.Read ( vecAttachedPosition.fY ) &&
                                                   bitStream.Read ( vecAttachedPosition.fZ ) &&
                                                   bitStream.Read ( vecAttachedRotation.fX ) &&
                                                   bitStream.Read ( vecAttachedRotation.fY ) &&
                                                   bitStream.Read ( vecAttachedRotation.fZ )) ) )
        {
#ifdef MTA_DEBUG
            g_pCore->GetConsole ()->Printf ( "    Retrieved common properties. Typeid %d", ucEntityTypeID );
#endif
			/*
#ifdef MTA_DEBUG
            char* names [ 17 ] = { "dummy", "player", "vehicle", "object", "marker", "blip",
                                   "pickup", "radar_area", "spawnpoint", "remoteclient", "console",
                                   "path_node", "world_mesh", "team", "ped", "colshape", "unknown" };
			g_pCore->GetConsole ()->Printf ( "* Entity-add: %s - %u - parent: %u", names [ ( ucEntityTypeID <= 16 ) ? ucEntityTypeID : 16 ], EntityID, ParentID );

            if ( FILE * file = fopen ( "entity-add.txt", "a" ) )
            {
				fprintf ( file, "entity-type: %s - id: %u - parent: %u\n", names [ ( ucEntityTypeID <= 16 ) ? ucEntityTypeID : 16 ], EntityID, ParentID );
                fclose ( file );
            }
#endif
			*/

            // Read custom data
            CCustomData* pCustomData = new CCustomData;
            unsigned short usNumData = 0;
            bitStream.Read ( usNumData );
#ifdef MTA_DEBUG
			g_pCore->GetConsole ()->Printf ( "    Adding %d custom data", usNumData );
#endif
            for ( unsigned short us = 0 ; us < usNumData ; us++ )
            {
                unsigned char ucNameLength = 0;

                // Read the custom data name length
                if ( bitStream.Read ( ucNameLength ) )
                {
                    // Was a valid custom data name length found?
                    if ( ucNameLength <= 32 )
                    {
                        // Add it only if the name is longer than 0
                        if ( ucNameLength > 0 )
                        {
                            char szName [ 33 ] = { '\0' };
                            CLuaArgument Argument;

                            bitStream.Read ( szName, ucNameLength );
                            szName[32] = '\0';
#ifdef MTA_DEBUG
							g_pCore->GetConsole ()->Printf ( "      %s", szName );
#endif

                            Argument.ReadFromBitStream ( bitStream );

                            pCustomData->Set ( szName, Argument, NULL );
                        }
                    }
                    else
                    {
                        #ifdef MTA_DEBUG
							char buf[1024] = {0};
							bitStream.Read ( buf, (ucNameLength > 1024) ? 1024 : ucNameLength );
                            // Raise a special assert, as we have to try and figure out this error.
                            assert ( 0 );
                        #endif

                        delete pCustomData;
                        pCustomData = NULL;

                        RaiseFatalError ( 6 );
                        return;
                    }
                }
                else
                {
                    #ifdef MTA_DEBUG
                        // Jax: had this with a colshape (ucNameLength=109,us=0,usNumData=4)
                        // Raise a special assert, as we have to try and figure out this error.
                        assert ( 0 );
                    #endif

                    delete pCustomData;
                    pCustomData = NULL;

                    RaiseFatalError ( 7 );
                    return;
                }
            }

            // Read out the name length
            unsigned short usNameLength;
            bitStream.Read ( usNameLength );

            // Create the name string and 0 terminate it
            char* szName = new char [ usNameLength + 1 ];
            szName [ usNameLength ] = 0;

            // Read out the name if it's longer than empty
            if ( usNameLength > 0 )
            {
                bitStream.Read ( szName, usNameLength );
            }

            // Read out the sync time context
            unsigned char ucSyncTimeContext = 0;
            bitStream.Read ( ucSyncTimeContext );

            CClientEntity* pEntity = CElementIDs::GetElement ( EntityID );
            // If we already have an entity with that ID (and aslong as this isnt just element-data for an already created player)
            if ( pEntity && pEntity->GetType () != CCLIENTPLAYER && ucEntityTypeID != CClientGame::PLAYER )
            {
#ifdef MTA_DEBUG
                // We shouldn't be replacing elements
                //assert ( 0 );
#endif

                // It shouldn't be a system
                assert ( !pEntity->IsSystemEntity () );
                if ( pEntity->IsSystemEntity () )
                {
                    RaiseFatalError ( 8 );
                    return;
                }

                // Set the id to INVALID_ELEMENT_ID so the new entity that overrides this one doesn't get deleted in CElementArray
                pEntity->SetID ( INVALID_ELEMENT_ID );

                // Clean up the overridden entity nicely
                g_pClientGame->m_ElementDeleter.Delete ( pEntity );
                pEntity = NULL;

                // And set the CElementArray entry to NULL as like it never existed
                CElementIDs::SetElement ( EntityID, NULL );
            }

            // Handle the rest depending on what entity type it is
            switch ( ucEntityTypeID )
            {
                case CClientGame::PLAYER:
                {
                    // We get entity data for players here, but player data is handled in Packet_PlayerList
                    break;
                }
                case CClientGame::OBJECT:
                {
                    unsigned short usObjectID;
                    g_pCore->GetConsole()->Printf ( "    It's an object" );
                    // Read out the position and the rotation
                    if ( bitStream.Read ( vecPosition.fX ) &&
                         bitStream.Read ( vecPosition.fY ) &&
                         bitStream.Read ( vecPosition.fZ ) &&
                         bitStream.Read ( vecRotation.fX ) &&
                         bitStream.Read ( vecRotation.fY ) &&
                         bitStream.Read ( vecRotation.fZ ) &&
                         bitStream.Read ( usObjectID ) )
                    {
                        g_pCore->GetConsole()->Printf ( "    Got model/pos/rot %d (%.2f %.2f %.2f) (%.2f %.2f %.2f)", usObjectID, vecPosition.fX, vecPosition.fY, vecPosition.fZ, vecRotation.fX, vecRotation.fY, vecRotation.fZ );
                        // Valid object id?
                        if ( !CClientObjectManager::IsValidModel ( usObjectID ) )
                        {
                            usObjectID = 1700;
                        }

                        // Create the object and put it at its position
                        CClientObject* pObject = new CDeathmatchObject ( g_pClientGame->m_pManager, g_pClientGame->m_pMovingObjectsManager, EntityID, usObjectID );
                        pEntity = pObject;
                        if ( pObject )
                        {
							g_pCore->GetConsole()->Printf ( "    Created object successfully" );
                            pObject->SetOrientation ( vecPosition, vecRotation );
                        }
                        else
                        {
                            RaiseFatalError ( 9 );
                        }
                    }

                    break;
                }

                case CClientGame::PICKUP:
                {
                    // Read out the pickup data
                    unsigned short usModel;
                    unsigned char ucFlags;
                    if ( bitStream.Read ( vecPosition.fX ) &&
                         bitStream.Read ( vecPosition.fY ) &&
                         bitStream.Read ( vecPosition.fZ ) &&
                         bitStream.Read ( usModel ) &&
                         bitStream.Read ( ucFlags ) )
                    {
                        // Create the pickup with the given position and model
                        CClientPickup* pPickup = new CClientPickup ( g_pClientGame->m_pManager, EntityID, usModel, vecPosition );
                        pEntity = pPickup;
                        if ( !pPickup )
                        {
                            RaiseFatalError ( 10 );
                            return;
                        }

                        // Set some properties
                        unsigned char ucPickupType;
                        bitStream.Read ( ucPickupType );
                        pPickup->m_ucType = ucPickupType;
                        switch ( ucPickupType )
                        {
                            case CClientPickup::ARMOR:
                            case CClientPickup::HEALTH:
                            {
                                float fAmount;
                                bitStream.Read ( fAmount );
                                pPickup->m_fAmount = fAmount;
                                break;
                            }
                            case CClientPickup::WEAPON:
                            {
                                unsigned char ucWeaponType;
                                bitStream.Read ( ucWeaponType );
                                pPickup->m_ucWeaponType = ucWeaponType;

                                unsigned short usAmmo;
                                bitStream.Read ( usAmmo );
                                pPickup->m_usAmmo = usAmmo;
                                break;
                            }
                            default: break;
                        }

                        // Set its visible status
                        pPickup->SetVisible ( ( ucFlags & 0x1 ) ? true:false );
                    }
                    else
                    {
                        RaiseProtocolError ( 38 );
                        return;
                    }

                    break;
                }

                case CClientGame::VEHICLE:
                {
                    // Read out the position
                    CVector vecPosition;
                    bitStream.Read ( vecPosition.fX );
                    bitStream.Read ( vecPosition.fY );
                    bitStream.Read ( vecPosition.fZ );

                    // Read out the rotation in degrees
                    CVector vecRotationDegrees;
                    bitStream.Read ( vecRotationDegrees.fX );
                    bitStream.Read ( vecRotationDegrees.fY );
                    bitStream.Read ( vecRotationDegrees.fZ );

                    // Read out the vehicle value as a char, then convert
                    unsigned char ucModel = 0xFF;
                    bitStream.Read ( ucModel );

                    // The server appears to subtract 400 from the vehicle id before
                    // sending it to us, as to allow the value to fit into an unsigned
                    // char.
                    //
                    // Too bad this was never documented.
                    //
                    // --slush
                    unsigned short usModel = ucModel + 400;
                    if ( !CClientVehicleManager::IsValidModel ( usModel ) )
                    {
                        RaiseProtocolError ( 39 );
                        return;
                    }

                    // Read out the health
                    float fHealth;
                    if ( !bitStream.Read ( fHealth ) )
                    {
                        RaiseProtocolError ( 40 );
                        return;
                    }

                    // Read out the color
                    unsigned char ucColor1;
                    unsigned char ucColor2;
                    unsigned char ucColor3;
                    unsigned char ucColor4;
                    unsigned char ucPaintjob;
                    if ( !bitStream.Read ( ucColor1 ) ||
                         !bitStream.Read ( ucColor2 ) ||
                         !bitStream.Read ( ucColor3 ) ||
                         !bitStream.Read ( ucColor4 ) ||
                         !bitStream.Read ( ucPaintjob ) )
                    {
                        RaiseProtocolError ( 41 );
                        return;
                    }

                    // Read out the vehicle damage sync model
                    unsigned char ucDoorStates [MAX_DOORS];
                    unsigned char ucWheelStates [MAX_WHEELS];
                    unsigned char ucPanelStates [MAX_PANELS];
                    unsigned char ucLightStates [MAX_LIGHTS];
                    if ( !bitStream.Read ( (char*) ucDoorStates, MAX_DOORS ) ||
                         !bitStream.Read ( (char*) ucWheelStates, MAX_WHEELS ) ||
                         !bitStream.Read ( (char*) ucPanelStates, MAX_PANELS ) ||
                         !bitStream.Read ( (char*) ucLightStates, MAX_LIGHTS ) )
                    {
                        RaiseProtocolError ( 42 );
                        return;
                    }

                    // Create it
                    CDeathmatchVehicle* pVehicle = new CDeathmatchVehicle ( g_pClientGame->m_pManager, g_pClientGame->m_pUnoccupiedVehicleSync, EntityID, usModel );
                    pEntity = pVehicle;
                    if ( !pVehicle )
                    {
                        RaiseFatalError ( 11 );
                        return;
                    }


                    // Set the health, color and paintjob
                    pVehicle->SetHealth ( fHealth );
                    pVehicle->SetPaintjob ( ucPaintjob );
                    pVehicle->SetColor ( ucColor1, ucColor2, ucColor3, ucColor4 );

                    // Setup our damage model
                    for ( int i = 0 ; i < MAX_DOORS ; i++ ) pVehicle->SetDoorStatus ( i, ucDoorStates [ i ] );
                    for ( int i = 0 ; i < MAX_WHEELS ; i++ ) pVehicle->SetWheelStatus ( i, ucWheelStates [ i ] );
                    for ( int i = 0 ; i < MAX_PANELS ; i++ ) pVehicle->SetPanelStatus ( i, ucPanelStates [ i ] );
                    for ( int i = 0 ; i < MAX_LIGHTS ; i++ ) pVehicle->SetLightStatus ( i, ucLightStates [ i ] );
                    pVehicle->ResetDamageModelSync ();

                    // If the vehicle has a turret, read out its position
                    if ( CClientVehicleManager::HasTurret ( usModel ) )
                    {
                        float fTurretX, fTurretY;
                        bitStream.Read ( fTurretX );
                        bitStream.Read ( fTurretY );
                        pVehicle->SetTurretRotation ( fTurretX, fTurretY );
                    }

                    // If the vehicle has an adjustable property, read out its value
                    if ( CClientVehicleManager::HasAdjustableProperty ( usModel ) )
                    {
                        unsigned short usAdjustableProperty;
                        bitStream.Read ( usAdjustableProperty );
                        pVehicle->SetAdjustablePropertyValue ( usAdjustableProperty );
                    }

                    // Read out the upgrades
                    CVehicleUpgrades* pUpgrades = pVehicle->GetUpgrades ();
                    unsigned char ucNumUpgrades;
                    bitStream.Read ( ucNumUpgrades );

                    if ( ucNumUpgrades > 0 )
                    {
                        unsigned char uc = 0;
                        for ( ; uc < ucNumUpgrades ; uc++ )
                        {
                            unsigned char ucUpgrade;
                            bitStream.Read ( ucUpgrade );

                            /*
                            * Dumb.  We're adjusting the number the server sent us,
                            * because it is apparently '1000' less than the actual number.
                            * Why this wasn't put in the CVehicleUpgrades class itself
                            * is beyond me...
                            *
                            * --slush
                            */
                            if (pUpgrades)
                                pUpgrades->AddUpgrade ( ucUpgrade + 1000 );
                        }
                    }

                    // Read out the reg plate
                    char szRegPlate [9];
                    szRegPlate [8] = 0;
                    bitStream.Read ( szRegPlate, 8 );
                    pVehicle->SetRegPlate ( szRegPlate );

                    unsigned char ucOverrideLights;
                    bitStream.Read ( ucOverrideLights );
                    pVehicle->SetOverrideLights ( ucOverrideLights );

                    // Flags
                    unsigned char ucFlags = 0;
                    bitStream.Read ( ucFlags );

                    // Extract the flag bools
                    bool bLandingGearDown = ( ucFlags & 0x01 ) ? true:false;
                    bool bSirenesActive = ( ucFlags & 0x02 ) ? true:false;
                    bool bPetrolTankWeak = ( ucFlags & 0x04 ) ? true:false;
                    bool bEngineOn = ( ucFlags & 0x08 ) ? true:false;
                    bool bLocked = ( ucFlags & 0x10 ) ? true:false;
                    bool bDoorsUndamageable = ( ucFlags & 0x20 ) ? true:false;
                    bool bDamageProof = ( ucFlags & 0x40 ) ? true:false;
                    bool bFrozen = ( ucFlags & 0x80 ) ? true:false;

                    // If the vehicle has a landing gear, set landing gear state
                    if ( CClientVehicleManager::HasLandingGears ( usModel ) )
                    {
                        pVehicle->SetLandingGearDown ( bLandingGearDown );
                    }

                    // If the vehicle has sirenes, set the sirene state
                    if ( CClientVehicleManager::HasSirens ( usModel ) )
                    {
                        pVehicle->SetSirenOrAlarmActive ( bSirenesActive );
                    }

                    // Set the general vehicle flags
                    pVehicle->SetCanShootPetrolTank ( bPetrolTankWeak );
                    pVehicle->SetEngineOn ( bEngineOn );
                    pVehicle->SetDoorsLocked ( bLocked );
                    pVehicle->SetDoorsUndamageable ( bDoorsUndamageable );
                    pVehicle->SetScriptCanBeDamaged ( !bDamageProof );
                    pVehicle->SetFrozen ( bFrozen );

                    // Read out and set alpha
                    unsigned char ucAlpha = 255;
                    bitStream.Read ( ucAlpha );
                    pVehicle->SetAlpha ( ucAlpha );	

                    // Set the matrix
                    pVehicle->SetPosition ( vecPosition );
                    pVehicle->SetRotationDegrees ( vecRotationDegrees );

                    break;
                }

                case CClientGame::MARKER:
                {
                    // Read out the common data for all kinds of markers
                    unsigned char ucType;
                    float fSize;
                    unsigned char ucRed;
                    unsigned char ucGreen;
                    unsigned char ucBlue;
                    unsigned char ucAlpha;
                    unsigned char ucFlags;

                    if ( bitStream.Read ( vecPosition.fX ) &&
                         bitStream.Read ( vecPosition.fY ) &&
                         bitStream.Read ( vecPosition.fZ ) &&
                         bitStream.Read ( ucType ) &&
                         bitStream.Read ( fSize ) &&
                         bitStream.Read ( ucRed ) &&
                         bitStream.Read ( ucGreen ) &&
                         bitStream.Read ( ucBlue ) &&
                         bitStream.Read ( ucAlpha ) &&
                         bitStream.Read ( ucFlags ) )
                    {
                        // Decode the flags
                        bool bTarget = ucFlags & 0x01;

                        // Valid type?
                        if ( ucType < CClientMarker::MARKER_INVALID )
                        {
                            // Create it
                            CClientMarker* pMarker = new CClientMarker ( g_pClientGame->m_pManager, EntityID, ucType );
                            pMarker->SetPosition ( vecPosition );
                            pMarker->SetSize ( fSize );
                            pMarker->SetColor ( ucRed, ucGreen, ucBlue, ucAlpha );

                            // Entity is this
                            pEntity = pMarker;

                            // More properties if it's a checkpoint or a ring
                            if ( ucType == CClientGame::MARKER_CHECKPOINT ||
                                 ucType == CClientGame::MARKER_RING )
                            {
                                // Grab the checkpoint
                                CClientCheckpoint* pCheckpoint = pMarker->GetCheckpoint ();

                                // Have a target?
                                if ( bTarget )
                                {
                                    // Read out the target position
                                    CVector vecTarget;
                                    if ( bitStream.Read ( vecTarget.fX ) &&
                                         bitStream.Read ( vecTarget.fY ) &&
                                         bitStream.Read ( vecTarget.fZ ) )
                                    {
                                        // Set the next position and the icon
                                        pCheckpoint->SetNextPosition ( vecTarget );
                                        pCheckpoint->SetIcon ( CClientCheckpoint::ICON_ARROW );
                                    }
                                }
                            }
                        }
                        else
                        {
                            RaiseProtocolError ( 123 );
                        }
                    }
                    else
                    {
                        RaiseProtocolError ( 44 );
                    }

                    break;
                }

                case CClientGame::BLIP:
                {         
                    // Read out the position
                    CVector vecPosition;
                    bitStream.Read ( vecPosition.fX );
                    bitStream.Read ( vecPosition.fY );
                    bitStream.Read ( vecPosition.fZ );                    

                    // Read out the ordering id
                    short sOrdering;
                    bitStream.Read ( sOrdering );

                    // Make a blip with the given ID
                    CClientRadarMarker* pBlip = new CClientRadarMarker ( g_pClientGame->m_pManager, EntityID, sOrdering );
                    pEntity = pBlip;

                    pBlip->SetPosition ( vecPosition );

                    // Read out the icon
                    unsigned char ucIcon;
                    bitStream.Read ( ucIcon );                    

                    // Set the icon if it's valid
                    if ( ucIcon <= RADAR_MARKER_LIMIT ) pBlip->SetSprite ( ucIcon );

                    unsigned char ucSize = 0, ucRed = 0, ucGreen = 0, ucBlue = 0, ucAlpha = 0;

                    // Read out size and color if there's no icon
                    if ( ucIcon == 0 )
                    {
                        // Read out the size
                        bitStream.Read ( ucSize );

                        // Read out the color
                        bitStream.Read ( ucRed );
                        bitStream.Read ( ucGreen );
                        bitStream.Read ( ucBlue );
                        bitStream.Read ( ucAlpha );

                        pBlip->SetScale ( ucSize );
                        pBlip->SetColor ( ucRed, ucGreen, ucBlue, ucAlpha );
                    }                    

                    break;
                }

                case CClientGame::RADAR_AREA:
                {
                    // Read out the radar area id, position, size and color
                    CVector2D vecPosition;
                    CVector2D vecSize;
                    unsigned long ulColor;
                    unsigned char ucFlags;
                    if ( bitStream.Read ( vecPosition.fX ) &&
                         bitStream.Read ( vecPosition.fY ) &&
                         bitStream.Read ( vecSize.fX ) &&
                         bitStream.Read ( vecSize.fY ) &&
                         bitStream.Read ( ulColor ) &&
                         bitStream.Read ( ucFlags ) )
                    {
                        // Decode the flags
                        bool bIsFlashing = ucFlags & 0x01;

                        // Create the radar area
                        CClientRadarArea* pArea = g_pClientGame->m_pRadarAreaManager->Create ( EntityID );
                        pEntity = pArea;
                        if ( pArea )
                        {
                            // Set the position, size and color
                            pArea->SetPosition ( vecPosition );
                            pArea->SetSize ( vecSize );
                            pArea->SetColor ( ulColor );
                            pArea->SetFlashing ( bIsFlashing );
                        }
                    }

                    break;
                }
                case CClientGame::PATH_NODE:
                {
                    int iTime;
                    unsigned char ucStyle;
                    ElementID NextNodeID;
                    // Read out the nodes position, rotation, time, style and next node
                    if ( bitStream.Read ( vecPosition.fX ) &&
                         bitStream.Read ( vecPosition.fY ) &&
                         bitStream.Read ( vecPosition.fZ ) &&
                         bitStream.Read ( vecRotation.fX ) &&
                         bitStream.Read ( vecRotation.fY ) &&
                         bitStream.Read ( vecRotation.fZ ) &&
                         bitStream.Read ( iTime ) &&
                         bitStream.Read ( ucStyle ) &&
                         bitStream.Read ( NextNodeID ) )
                    {
                        CClientPathNode* pNode = new CClientPathNode ( g_pClientGame->m_pManager, vecPosition, vecRotation,
                            iTime, EntityID, (CClientPathNode::ePathNodeStyle)ucStyle );
                        pEntity = pNode;
                        if ( pNode && NextNodeID != INVALID_ELEMENT_ID )
                        {
                            pNode->SetNextNodeID ( NextNodeID );
                        }
                    }

                    break;
                }

                case CClientGame::WORLD_MESH:
                    break;

                case CClientGame::TEAM:
                {
                    unsigned short usNameLength;
                    bitStream.Read ( usNameLength );
                    if (usNameLength > 255)
                    {
                        RaiseFatalError ( 12 );
                        return;
                    }

                    char* szTeamName = new char [ usNameLength + 1 ];
                    bitStream.Read ( szTeamName, usNameLength );
                    szTeamName [ usNameLength ] = 0;

                    unsigned char ucRed, ucGreen, ucBlue;
                    bitStream.Read ( ucRed );
                    bitStream.Read ( ucGreen );
                    bitStream.Read ( ucBlue );

                    CClientTeam* pTeam = new CClientTeam ( g_pClientGame->GetManager (), EntityID, szTeamName, ucRed, ucGreen, ucBlue );
                    pEntity = pTeam;

                    unsigned char ucFriendlyFire;
                    bitStream.Read ( ucFriendlyFire );
                    pTeam->SetFriendlyFire ( ( ucFriendlyFire == 1 ) );

                    delete [] szTeamName;
                    break;
                }

                case CClientGame::PED:
                {
                    // Read out position
                    CVector vecPosition;
                    bitStream.Read ( vecPosition.fX );
                    bitStream.Read ( vecPosition.fY );
                    bitStream.Read ( vecPosition.fZ );

                    // Read out the model
                    unsigned short usModel;
                    bitStream.Read ( usModel );
                    if ( !CClientPlayerManager::IsValidModel ( usModel ) )
                    {
                        RaiseProtocolError ( 51 );
                        return;
                    }

                    // Read out the rotation
                    float fRotation;
                    bitStream.Read ( fRotation );

                    // Read out the health
                    float fHealth;
                    bitStream.Read ( fHealth );

                    float fArmour;
                    bitStream.Read ( fArmour );

                    // Read out the vehicle id
                    ElementID VehicleID;
                    bitStream.Read ( VehicleID );
                    CClientVehicle * pVehicle = NULL;
                    unsigned char ucSeat = 0xFF;
                    if ( VehicleID != INVALID_ELEMENT_ID )
                    {
                        pVehicle = g_pClientGame->m_pVehicleManager->Get ( VehicleID );
                        bitStream.Read ( ucSeat );
                    }

                    // Flags
                    unsigned char ucFlags;
                    bitStream.Read ( ucFlags );
                    bool bHasJetPack = ( ucFlags & 0x1 ) > 0;
                    bool bSynced = ( ucFlags & 0x2 ) > 0;

                    CClientPed* pPed = new CClientPed ( g_pClientGame->m_pManager, usModel, EntityID );
                    pEntity = pPed;

                    pPed->SetPosition ( vecPosition );
                    pPed->SetCurrentRotation ( fRotation, true );
                    pPed->SetCameraRotation ( fRotation );
                    pPed->SetHealth ( fHealth );
                    pPed->SetArmor ( fArmour );
                    if ( bSynced )
                    {
                        pPed->LockHealth ( fHealth );
                        pPed->LockArmor ( fArmour );
                    }
                    if ( pVehicle ) pPed->WarpIntoVehicle ( pVehicle, ucSeat );
                    pPed->SetHasJetPack ( bHasJetPack );

                    break;
                }

                case CClientGame::DUMMY:
                {
                    // Type Name
                    unsigned short usTypeNameLength;
                    bitStream.Read ( usTypeNameLength );
                    char* szTypeName = new char [ usTypeNameLength + 1 ];
                    bitStream.Read ( szTypeName, usTypeNameLength );
                    szTypeName [ usTypeNameLength ] = 0;

                    CClientDummy* pDummy = new CClientDummy ( g_pClientGame->m_pManager, EntityID, szTypeName );
                    pEntity = pDummy;

                    // Position
                    CVector vecPosition;
                    bitStream.Read ( vecPosition.fX );
                    bitStream.Read ( vecPosition.fY );
                    bitStream.Read ( vecPosition.fZ );

                    if (pDummy)
                    {
                        pDummy->SetPosition ( vecPosition );
						if ( strcmp ( szTypeName, "resource" ) == 0 )
						{
							CResource* pResource = g_pClientGame->m_pResourceManager->GetResource ( szName );
							if ( pResource )
								pResource->SetResourceEntity ( pDummy ); // problem with resource starting without this entity
						}
                    }

                    delete [] szTypeName;
                    break;
                }

                case CClientGame::COLSHAPE:
                {
                    // Type
                    unsigned char ucType;
                    bitStream.Read ( ucType );

                    // Position
                    CVector vecPosition;
                    bitStream.Read ( vecPosition.fX );
                    bitStream.Read ( vecPosition.fY );
                    bitStream.Read ( vecPosition.fZ );

                    // Enabled?
                    unsigned char ucEnabled;
                    bitStream.Read ( ucEnabled );

                    // AutoCallEvent?
                    unsigned char ucAutoCallEvent;
                    bitStream.Read ( ucAutoCallEvent );

                    CClientColShape* pShape = NULL;

                    // Type-dependant stuff
                    switch ( ucType )
                    {
                        case COLSHAPE_CIRCLE:
                        {
                            float fRadius;
                            bitStream.Read ( fRadius );
                            CClientColCircle* pCircle = new CClientColCircle ( g_pClientGame->m_pManager, EntityID, vecPosition, fRadius );
                            pEntity = pShape = pCircle;
                            break;
                        }
                        case COLSHAPE_CUBOID:
                        {
                            CVector vecSize;
                            bitStream.Read ( vecSize.fX );
                            bitStream.Read ( vecSize.fY );
                            bitStream.Read ( vecSize.fZ );
                            CClientColCuboid* pCuboid = new CClientColCuboid ( g_pClientGame->m_pManager, EntityID, vecPosition, vecSize );
                            pEntity = pShape = pCuboid;
                            break;
                        }
                        case COLSHAPE_SPHERE:
                        {
                            float fRadius;
                            bitStream.Read ( fRadius );
                            CClientColSphere* pSphere = new CClientColSphere ( g_pClientGame->m_pManager, EntityID, vecPosition, fRadius );
                            pEntity = pShape = pSphere;
                            break;
                        }
                        case COLSHAPE_RECTANGLE:
                        {
                            CVector2D vecSize;
                            bitStream.Read ( vecSize.fX );
                            bitStream.Read ( vecSize.fY );
                            CClientColRectangle* pRectangle = new CClientColRectangle ( g_pClientGame->m_pManager, EntityID, vecPosition, vecSize );
                            pEntity = pShape = pRectangle;
                            break;
                        }
                        case COLSHAPE_TUBE:
                        {
                            float fRadius, fHeight;
                            bitStream.Read ( fRadius );
                            bitStream.Read ( fHeight );
                            CClientColTube* pTube = new CClientColTube ( g_pClientGame->m_pManager, EntityID, vecPosition, fRadius, fHeight );
                            pEntity = pShape = pTube;
                            break;
                        }
                        default:
                        {
                            RaiseProtocolError ( 54 );
                            break;
                        }
                    }
                    assert ( pShape );
                    if ( pShape )
                    {
                        pShape->SetEnabled ( ( ucEnabled == 1 ) );
                        pShape->SetAutoCallEvent ( ( ucAutoCallEvent == 1 ) );
                    }

                    break;
                }

                case CClientGame::HANDLING:
                {
                    // Create the handling entry
                    CClientHandling* pHandling = new CClientHandling ( g_pClientGame->m_pManager, EntityID );

                    // Read out the handling data (121 bytes :O)
                    // The following entries were skipped (not relevant to multiplayer):
                    // Monetary value (wealth of the car? who drives it in SP)
                    float fMass, fTurnMass, fDragCoeff;
                    CVector vecCenterOfMass;
                    unsigned int uiPercentSubmerged;
                    float fTractionMultiplier;
                    unsigned char ucDriveType, ucEngineType, ucNumberOfGears;
                    float fEngineAccelleration, fEngineInertia, fMaxVelocity;
                    float fBrakeDecelleration, fBrakeBias;
                    unsigned char ucABS;
                    float fSteeringLock, fTractionLoss, fTractionBias;
                    float fSuspensionForceLevel, fSuspensionDamping;
                    float fSuspensionHighSpeedDamping, fSuspensionUpperLimit, fSuspensionLowerLimit;
                    float fSuspensionFrontRearBias, fSuspensionAntidiveMultiplier;
                    float fCollisionDamageMultiplier, fSeatOffsetDistance;
                    unsigned int uiHandlingFlags, uiModelFlags;
                    unsigned char ucHeadLight, ucTailLight, ucAnimGroup;


                    // Read out the stuff we can expect to read as along
                    unsigned long ulFlagsToRead;
                    bitStream.Read ( ulFlagsToRead );

                    if ( ulFlagsToRead & 0x00000001 )
                    {
                        bitStream.Read ( fMass );
                        pHandling->SetMass ( fMass );
                    }

                    if ( ulFlagsToRead & 0x00000002 )
                    {
                        bitStream.Read ( fTurnMass );
                        pHandling->SetTurnMass ( fTurnMass );
                    }

                    if ( ulFlagsToRead & 0x00000004 )
                    {
                        bitStream.Read ( fDragCoeff );
                        pHandling->SetDragCoeff ( fDragCoeff );
                    }

                    if ( ulFlagsToRead & 0x00000008 )
                    {
                        bitStream.Read ( vecCenterOfMass.fX );
                        bitStream.Read ( vecCenterOfMass.fY );
                        bitStream.Read ( vecCenterOfMass.fZ );
                        pHandling->SetCenterOfMass ( vecCenterOfMass );
                    }

                    if ( ulFlagsToRead & 0x00000010 )
                    {
                        bitStream.Read ( uiPercentSubmerged );
                        pHandling->SetPercentSubmerged ( uiPercentSubmerged );
                    }

                    if ( ulFlagsToRead & 0x00000020 )
                    {
                        bitStream.Read ( fTractionMultiplier );
                        pHandling->SetTractionMultiplier ( fTractionMultiplier );
                    }

                    if ( ulFlagsToRead & 0x00000040 )
                    {
                        bitStream.Read ( ucDriveType );

                        if ( ucDriveType != CHandlingEntry::FOURWHEEL &&
                             ucDriveType != CHandlingEntry::RWD &&
                             ucDriveType != CHandlingEntry::FWD )
                        {
                            ucDriveType = CHandlingEntry::RWD;
                        }

                        pHandling->SetDriveType ( static_cast < CHandlingEntry::eDriveType > ( ucDriveType ) );
                    }

                    if ( ulFlagsToRead & 0x00000080 )
                    {
                        bitStream.Read ( ucEngineType );

                        if ( ucEngineType != CHandlingEntry::DIESEL &&
                             ucEngineType != CHandlingEntry::ELECTRIC &&
                             ucEngineType != CHandlingEntry::PETROL )
                        {
                            ucEngineType = CHandlingEntry::PETROL;
                        }

                        pHandling->SetEngineType ( static_cast < CHandlingEntry::eEngineType > ( ucEngineType ) );
                    }

                    if ( ulFlagsToRead & 0x00000100 )
                    {
                        bitStream.Read ( ucNumberOfGears );
                        pHandling->SetNumberOfGears ( ucNumberOfGears );
                    }

                    if ( ulFlagsToRead & 0x00000200 )
                    {
                        bitStream.Read ( fEngineAccelleration );
                        pHandling->SetEngineAccelleration ( fEngineAccelleration );
                    }

                    if ( ulFlagsToRead & 0x00000400 )
                    {
                        bitStream.Read ( fEngineInertia );
                        pHandling->SetEngineInertia ( fEngineInertia );
                    }

                    if ( ulFlagsToRead & 0x00000800 )
                    {
                        bitStream.Read ( fMaxVelocity );
                        pHandling->SetMaxVelocity ( fMaxVelocity );
                    }

                    if ( ulFlagsToRead & 0x00001000 )
                    {
                        bitStream.Read ( fBrakeDecelleration );
                        pHandling->SetBrakeDecelleration ( fBrakeDecelleration );
                    }

                    if ( ulFlagsToRead & 0x00002000 )
                    {
                        bitStream.Read ( fBrakeBias );
                        pHandling->SetBrakeBias ( fBrakeBias );
                    }

                    if ( ulFlagsToRead & 0x00004000 )
                    {
                        bitStream.Read ( ucABS );
                        pHandling->SetABS ( ucABS != 0 );
                    }

                    if ( ulFlagsToRead & 0x00008000 )
                    {
                        bitStream.Read ( fSteeringLock );
                        pHandling->SetSteeringLock ( fSteeringLock );
                    }

                    if ( ulFlagsToRead & 0x00010000 )
                    {
                        bitStream.Read ( fTractionLoss );
                        pHandling->SetTractionLoss ( fTractionLoss );
                    }
                    

                    if ( ulFlagsToRead & 0x00020000 )
                    {
                        bitStream.Read ( fTractionBias );
                        pHandling->SetTractionBias ( fTractionBias );
                    }

                    if ( ulFlagsToRead & 0x00040000 )
                    {
                        bitStream.Read ( fSuspensionForceLevel );
                        pHandling->SetSuspensionForceLevel ( fSuspensionForceLevel );
                    }

                    if ( ulFlagsToRead & 0x00080000 )
                    {
                        bitStream.Read ( fSuspensionDamping );
                        pHandling->SetSuspensionDamping ( fSuspensionDamping );
                    }

                    if ( ulFlagsToRead & 0x00100000 )
                    {
                        bitStream.Read ( fSuspensionHighSpeedDamping );
                        pHandling->SetSuspensionHighSpeedDamping ( fSuspensionHighSpeedDamping );
                    }

                    if ( ulFlagsToRead & 0x00200000 )
                    {
                        bitStream.Read ( fSuspensionUpperLimit );
                        pHandling->SetSuspensionUpperLimit ( fSuspensionUpperLimit );
                    }

                    if ( ulFlagsToRead & 0x00400000 )
                    {
                        bitStream.Read ( fSuspensionLowerLimit );
                        pHandling->SetSuspensionLowerLimit ( fSuspensionLowerLimit );
                    }

                    if ( ulFlagsToRead & 0x00800000 )
                    {
                        bitStream.Read ( fSuspensionFrontRearBias );
                        pHandling->SetSuspensionFrontRearBias ( fSuspensionFrontRearBias );
                    }

                    if ( ulFlagsToRead & 0x01000000 )
                    {
                        bitStream.Read ( fSuspensionAntidiveMultiplier );
                        pHandling->SetSuspensionAntidiveMultiplier ( fSuspensionAntidiveMultiplier );
                    }

                    if ( ulFlagsToRead & 0x02000000 )
                    {
                        bitStream.Read ( fCollisionDamageMultiplier );
                        pHandling->SetCollisionDamageMultiplier ( fCollisionDamageMultiplier );
                    }

                    if ( ulFlagsToRead & 0x04000000 )
                    {
                        bitStream.Read ( fSeatOffsetDistance );
                        pHandling->SetSeatOffsetDistance ( fSeatOffsetDistance );
                    }

                    if ( ulFlagsToRead & 0x08000000 )
                    {
                        bitStream.Read ( uiHandlingFlags );
                        pHandling->SetHandlingFlags ( uiHandlingFlags );
                    }

                    if ( ulFlagsToRead & 0x10000000 )
                    {
                        bitStream.Read ( uiModelFlags );
                        pHandling->SetModelFlags ( uiModelFlags );
                    }

                    if ( ulFlagsToRead & 0x20000000 )
                    {
                        bitStream.Read ( ucHeadLight );

                        if ( ucHeadLight > CHandlingEntry::TALL )
                            ucHeadLight = CHandlingEntry::TALL;

                        pHandling->SetHeadLight ( static_cast < CHandlingEntry::eLightType > ( ucHeadLight ) );
                    }

                    if ( ulFlagsToRead & 0x40000000 )
                    {
                        bitStream.Read ( ucTailLight );

                        if ( ucTailLight > CHandlingEntry::TALL )
                            ucTailLight = CHandlingEntry::TALL;

                        pHandling->SetTailLight ( static_cast < CHandlingEntry::eLightType > ( ucTailLight ) );
                    }

                    if ( ulFlagsToRead & 0x80000000 )
                    {
                        bitStream.Read ( ucAnimGroup );
                        pHandling->SetAnimGroup ( ucAnimGroup );
                    }
                    
                    // How many vehicles is this handling applied to?
                    unsigned char ucVehicleCount;
                    bitStream.Read ( ucVehicleCount );

                    // Read out the vehicle ID's
                    unsigned char ucID;
                    eVehicleTypes eModel;
                    for ( int i = 0; i < ucVehicleCount; i++ )
                    {
                        // Read out vehicle ID
                        bitStream.Read ( ucID );

                        // Convert it to the real ID (+400) and grab the handling for it
                        eModel = static_cast < eVehicleTypes > ( ucID + 400 );
                        pHandling->AddDefaultTo ( eModel );
                    }


                    break;
                }

                case CClientGame::SCRIPTFILE:
                {
                    break;
                }

                default:
                {
                    assert ( 0 );
                    break;
                }
            }

            if ( pEntity )
            {
                pEntity->SetName ( szName );
                pEntity->SetInterior ( ucInterior );
                pEntity->SetDimension ( usDimension );
                pEntity->SetAttachedOffsets ( vecAttachedPosition, vecAttachedRotation );
                pEntity->SetSyncTimeContext ( ucSyncTimeContext );
                pEntity->GetCustomDataPointer ()->Copy ( pCustomData );

                // Save any entity-dependant stuff for later
                SEntityDependantStuff* pStuff = new SEntityDependantStuff;
                pStuff->pEntity = pEntity;
                pStuff->Parent = ParentID;
                pStuff->AttachedToID = EntityAttachedToID;
                newEntitiesStuff.push_back ( pStuff );
            }

            delete [] szName;
            delete pCustomData;
        }
    }

    // Link our path nodes
    g_pClientGame->m_pPathManager->LinkNodes ();

    // Link the entity dependant stuff
    list < SEntityDependantStuff* > ::iterator iter = newEntitiesStuff.begin ();
    for ( ; iter != newEntitiesStuff.end () ; iter++ )
    {
        SEntityDependantStuff* pEntityStuff = *iter;
        CClientEntity* pTempEntity = pEntityStuff->pEntity;
        ElementID TempParent = pEntityStuff->Parent;
        ElementID TempAttachedToID = pEntityStuff->AttachedToID;

        if ( TempParent != INVALID_ELEMENT_ID )
        {
            CClientEntity* pParent = CElementIDs::GetElement ( TempParent );
            pTempEntity->SetParent ( pParent );
        }

        if ( TempAttachedToID != INVALID_ELEMENT_ID && pTempEntity->GetType () != CCLIENTPLAYER )
        {
            CClientEntity* pAttachedToEntity = CElementIDs::GetElement ( TempAttachedToID );
            if ( pAttachedToEntity )
            {
                pTempEntity->AttachTo ( pAttachedToEntity );
            }
        }

        delete pEntityStuff;
    }
    newEntitiesStuff.clear ();
}

void CPacketHandler::Packet_EntityRemove ( NetBitStreamInterface& bitStream )
{
    // ElementID   (2)     - entity id

    ElementID ID;
    while ( bitStream.Read ( ID ) )
    {
        CClientEntity* pEntity = CElementIDs::GetElement ( ID );
        if ( pEntity )
        {
            if ( pEntity->GetType ( ) == CCLIENTPLAYER )
            {
                RaiseProtocolError ( 45 );
                return;
            }            

            // Is this a vehicle? Make sure that if this is a player we're working on getting into
            // or out of that we reset vehicle in out stuff
            if ( g_pClientGame->m_VehicleInOutID == ID )
            {
                g_pClientGame->ResetVehicleInOut ();
            }

            // Delete its clientside children
            pEntity->DeleteClientChildren ();

            // Is this not a system entity?
            if ( !pEntity->IsSystemEntity () )
            {
                // Should be safe to delete this element...
                // We might call Delete on this element twice but the code won't delete it twice.
                g_pClientGame->m_ElementDeleter.Delete ( pEntity );
            }
        }
    }
}

void CPacketHandler::Packet_PickupHideShow ( NetBitStreamInterface& bitStream )
{
    // bool             - show it?
    // ElementID        - pickup ids (repeating)
    // unsigned char    - pickup model id

    // We must be joined
    if ( g_pClientGame->m_Status != CClientGame::STATUS_JOINED )
    {
        RaiseProtocolError ( 47 );
        return;
    }

    // Read out global flags
    unsigned char ucFlags;
    bitStream.Read ( ucFlags );

    // Extract the flag bools
    bool bShow = ucFlags & 0x01;

    // Read until the end
    while ( bitStream.GetNumberOfUnreadBits () > 10 )
    {
        // Pickup id and model
        ElementID PickupID;
        unsigned short usPickupModel;
        if ( bitStream.Read ( PickupID ) && bitStream.Read ( usPickupModel ) )
        {
            // Try to grab the pickup
            CClientPickup* pPickup = g_pClientGame->m_pPickupManager->Get ( PickupID );
            if ( pPickup )
            {
                // Show/hide it
                pPickup->SetModel ( usPickupModel );
                pPickup->SetVisible ( bShow );
            }
        }
    }
}


void CPacketHandler::Packet_PickupHitConfirm ( NetBitStreamInterface& bitStream )
{
    // ElementID        (2)     - pickup id
    // bool                     - hide it?
    // bool                     - sound?

    // We must be joined
    if ( g_pClientGame->m_Status != CClientGame::STATUS_JOINED )
    {
        RaiseProtocolError ( 49 );
        return;
    }

    // Read out the id and the flags
    ElementID PickupID;
    unsigned char ucFlags;
    if ( bitStream.Read ( PickupID ) &&
         bitStream.Read ( ucFlags ) )
    {
        // Extract the bools from the flags
        bool bHide = ( ucFlags & 0x01 ) ? true:false;
        bool bPlaySound = ( ucFlags & 0x02 ) ? true:false;

        // Grab the pickup
        CClientPickup* pPickup = g_pClientGame->m_pPickupManager->Get ( PickupID );
        if ( !pPickup )
        {
            return;
        }

        // Hide it if the packet tells us to
        pPickup->SetVisible ( bHide );

        // Server want us to play the sound?
        if ( bPlaySound )
        {
            // Play the pick up sound
            g_pGame->GetAudio ()->PlayFrontEndSound ( 40 );
        }
    }
}


void CPacketHandler::Packet_Lua ( NetBitStreamInterface& bitStream )
{
    if ( g_pClientGame->m_pRPCFunctions )
    {
        g_pClientGame->m_pRPCFunctions->ProcessPacket ( bitStream );
    }
}

void CPacketHandler::Packet_TextItem( NetBitStreamInterface& bitStream )
{
    // unsigned long    (4)     - text item id
    // bool                     - delete it? (can end here if true)

    // float            (4)     - X Position
    // float            (4)     - Y Position
    // float            (4)     - Scale
    // char             (1)     - red
    // char             (1)     - green
    // char             (1)     - blue
    // char             (1)     - alpha
    // unsigned short   (2)     - text string length
    // char[]           (*)     - text string of length specified above

    // We must be joined
    if ( g_pClientGame->m_Status != CClientGame::STATUS_JOINED )
    {
        RaiseProtocolError ( 50 );
        return;
    }

    // Read out the text ID
    unsigned long ulID;
    bitStream.Read ( ulID );

    // Flags
    unsigned char ucFlags = 0;
    bitStream.Read ( ucFlags );

    // Extract the flags
    bool bDelete = ucFlags & 0x01;

    // Something else than invalid?
    if ( ulID != 0xFFFFFFFF )
    {
        // Should we delete it?
        if ( bDelete )
        {
            // Grab it and delete it
            CClientDisplay* pDisplay = g_pClientGame->m_pDisplayManager->Get ( ulID );
            if ( pDisplay )
            {
                delete pDisplay;
            }
        }
        else
        {
            // Read out the text format
            float               fX = 0;
            float               fY = 0;
            float               fScale = 0;
            unsigned char       ucRed=0, ucGreen=0, ucBlue=0, ucAlpha=0, ucFormat=0;

            bitStream.Read ( fX );
            bitStream.Read ( fY );
            bitStream.Read ( fScale );
            bitStream.Read ( ucRed );
            bitStream.Read ( ucGreen );
            bitStream.Read ( ucBlue );
            bitStream.Read ( ucAlpha );
            bitStream.Read ( ucFormat );

            // Read out the text size
            unsigned short usTextLength = 0;
            bitStream.Read( usTextLength );
            if ( usTextLength <= 1024 )
            {
                // Read out the texgt
                char* szText = new char [ usTextLength + 1 ];
                szText [usTextLength] = 0;
                if ( usTextLength > 0 )
                {
                    bitStream.Read ( szText, usTextLength );
                }

                // Does the text not already exist? Create it
                CClientTextDisplay* pTextDisplay = NULL;
                CClientDisplay* pDisplay = g_pClientGame->m_pDisplayManager->Get ( ulID );
                if ( pDisplay && pDisplay->GetType () == DISPLAY_TEXT )
                {
                    pTextDisplay = static_cast < CClientTextDisplay* > ( pDisplay );
                }

                if ( !pTextDisplay )
                {
                    // Create it
                    pTextDisplay = new CClientTextDisplay ( g_pClientGame->m_pDisplayManager, ulID, false );
                }

                // Set the text properties
                pTextDisplay->SetCaption ( szText );
                pTextDisplay->SetPosition ( CVector ( fX, fY, 0 ) );
                pTextDisplay->SetColor ( ucRed, ucGreen, ucBlue, ucAlpha );
                pTextDisplay->SetScale ( fScale );
                pTextDisplay->SetFormat ( ( unsigned long ) ucFormat );

                delete [] szText;
            }
        }
    }
}


void CPacketHandler::Packet_ExplosionSync ( NetBitStreamInterface& bitStream )
{
    // Read out the creator, latency, position and type
    unsigned short usLatency;
    ElementID CreatorID, OriginID;
    CVector vecPosition;
    unsigned char ucType;
    if ( bitStream.Read ( CreatorID ) &&
         bitStream.Read ( usLatency ) &&
         bitStream.Read ( OriginID ) &&
         bitStream.Read ( vecPosition.fX ) &&
         bitStream.Read ( vecPosition.fY ) &&
         bitStream.Read ( vecPosition.fZ ) &&
         bitStream.Read ( ucType ) )
    {
        // Grab the creator if any (for kill credits)
        CClientPlayer* pCreator = NULL;
        if ( CreatorID != INVALID_ELEMENT_ID )
        {
            pCreator = g_pClientGame->m_pPlayerManager->Get ( CreatorID );
        }

        // Grab the true position if we have an origin source
        CClientEntity * pOrigin = NULL;
        if ( OriginID != INVALID_ELEMENT_ID )
        {
            pOrigin = CElementIDs::GetElement ( OriginID );
            if ( pOrigin )
            {
                // Is the origin a player element?
                if ( pOrigin->GetType () == CCLIENTPLAYER )
                {
                    // Is he in a vehicle?
                    CClientVehicle * pVehicle = static_cast < CClientPlayer * > ( pOrigin )->GetOccupiedVehicle ();
                    if ( pVehicle )
                    {
                        // Use this as the origin instead
                        pOrigin = pVehicle;
                    }
                }
                CVector vecTemp;
                pOrigin->GetPosition ( vecTemp );
                vecPosition += vecTemp;
            }
        }
        else
        {
            // TODO: Ping compensate
        }

        eExplosionType explosionType = ( eExplosionType ) ucType;       

        
        // Is it a vehicle explosion?
        if ( pOrigin && pOrigin->GetType () == CCLIENTVEHICLE && ( ucType == EXP_TYPE_BOAT || ucType == EXP_TYPE_CAR || ucType == EXP_TYPE_CAR_QUICK || ucType == EXP_TYPE_HELI ) )
        {            
            // Make sure the vehicle's blown
            CClientVehicle * pExplodingVehicle = static_cast < CClientVehicle * > ( pOrigin );
            pExplodingVehicle->Blow ( false );
            g_pClientGame->m_pManager->GetExplosionManager ()->Create ( EXP_TYPE_GRENADE, vecPosition, pCreator, true, -1.0f, false, WEAPONTYPE_EXPLOSION );
        }
        else
            g_pClientGame->m_pManager->GetExplosionManager ()->Create ( explosionType, vecPosition, pCreator );       

        CLuaArguments Arguments;
        Arguments.PushNumber ( vecPosition.fX );
        Arguments.PushNumber ( vecPosition.fY );
        Arguments.PushNumber ( vecPosition.fZ );
        Arguments.PushNumber ( explosionType );
        if ( pCreator ) pCreator->CallEvent ( "onClientExplosion", Arguments, true );
        else g_pClientGame->GetRootEntity ()->CallEvent ( "onClientExplosion", Arguments, false );
    }
}


void CPacketHandler::Packet_FireSync ( NetBitStreamInterface& bitStream )
{
    // Read out the creator, latency, position and size
    ElementID Creator;
    unsigned short usLatency;
    CVector vecPosition;
    float fSize;
    if ( bitStream.Read ( Creator ) &&
         bitStream.Read ( usLatency ) &&
         bitStream.Read ( vecPosition.fX ) &&
         bitStream.Read ( vecPosition.fY ) &&
         bitStream.Read ( vecPosition.fZ ) &&
         bitStream.Read ( fSize ) )
    {
        // Grab the creator if any (for kill credits)
        CClientPlayer* pCreator = NULL;
        if ( Creator != INVALID_ELEMENT_ID )
        {
            pCreator = g_pClientGame->m_pPlayerManager->Get ( Creator );
        }

        // TODO: Ping compensate
    }
}


void CPacketHandler::Packet_ProjectileSync ( NetBitStreamInterface& bitStream )
{
    // Read out the creator, latency, ...
    ElementID CreatorID;
    unsigned short usLatency;    
    ElementID OriginID;
    CVector vecOrigin;
    unsigned char ucWeaponType;
    if ( bitStream.Read ( CreatorID ) &&
         bitStream.Read ( usLatency ) &&
         bitStream.Read ( OriginID ) &&         
         bitStream.Read ( vecOrigin.fX ) &&
         bitStream.Read ( vecOrigin.fY ) &&
         bitStream.Read ( vecOrigin.fZ ) &&
         bitStream.Read ( ucWeaponType ) )
    {
        CClientEntity* pCreator = NULL;
        if ( CreatorID != INVALID_ELEMENT_ID ) pCreator = CElementIDs::GetElement ( CreatorID );
        if ( OriginID != INVALID_ELEMENT_ID )
        {
            CClientEntity* pOriginSource = CElementIDs::GetElement ( OriginID );
            if ( pOriginSource )
            {
                CVector vecTemp;
                pOriginSource->GetPosition ( vecTemp );
                vecOrigin += vecTemp;
            }
        }

        bool bCreateProjectile = false;

        eWeaponType weaponType = ( eWeaponType ) ucWeaponType;
        float fForce = 0.0f;
        CVector vecRotation, *pvecRotation = NULL;
        CVector vecVelocity, *pvecVelocity = NULL;
        CClientEntity * pTargetEntity = NULL;
        switch ( weaponType )
        {
            case WEAPONTYPE_GRENADE:
            case WEAPONTYPE_TEARGAS:
            case WEAPONTYPE_MOLOTOV:
            case WEAPONTYPE_REMOTE_SATCHEL_CHARGE:
            {
                if ( bitStream.Read ( fForce ) &&                     
                     bitStream.Read ( vecVelocity.fX ) &&
                     bitStream.Read ( vecVelocity.fY ) &&
                     bitStream.Read ( vecVelocity.fZ ) )
                {
                    pvecVelocity = &vecVelocity;
                    bCreateProjectile = true;
                }
                break;
            }
            case WEAPONTYPE_ROCKET:
            case WEAPONTYPE_ROCKET_HS:
            {
                CClientVehicle* pTargetVehicle = NULL;
                ElementID TargetID = INVALID_ELEMENT_ID;
                if ( bitStream.Read ( TargetID ) &&
                     bitStream.Read ( vecRotation.fX ) &&
                     bitStream.Read ( vecRotation.fY ) &&
                     bitStream.Read ( vecRotation.fZ ) &&
                     bitStream.Read ( vecVelocity.fX ) &&
                     bitStream.Read ( vecVelocity.fY ) &&
                     bitStream.Read ( vecVelocity.fZ ) )
                {
                    pvecRotation = &vecRotation;
                    pvecVelocity = &vecVelocity;
                    if ( TargetID != INVALID_ELEMENT_ID )
                    {
                        pTargetVehicle = g_pClientGame->m_pVehicleManager->Get ( TargetID );
                        if ( pTargetVehicle )
                        {
                            pTargetEntity = pTargetVehicle;
                        }
                    }
                    bCreateProjectile = true;
                }
                break;
            }
            case WEAPONTYPE_FLARE:
            case WEAPONTYPE_FREEFALL_BOMB:
                bCreateProjectile = true;
                break;
        }
        if ( bCreateProjectile )
        {
            if ( pCreator )
            {
                if ( pCreator->GetType () == CCLIENTPED || pCreator->GetType () == CCLIENTPLAYER )
                {
                    CClientVehicle * pVehicle = static_cast < CClientPed * > ( pCreator )->GetOccupiedVehicle ();
                    if ( pVehicle ) pCreator = pVehicle;
                }
                
                CClientProjectile * pProjectile = g_pClientGame->m_pManager->GetProjectileManager ()->Create ( pCreator, weaponType, vecOrigin, fForce, NULL, pTargetEntity );
                if ( pProjectile )
                {
                    pProjectile->Initiate ( &vecOrigin, pvecRotation, pvecVelocity, 0 );
                }              
            }
        }
    }
}


void CPacketHandler::Packet_PlayerStats ( NetBitStreamInterface& bitStream )
{
    // Read out the player and stats
    ElementID ID;
    unsigned short usNumStats;
    if ( bitStream.Read ( ID ) &&
         bitStream.Read ( usNumStats ) )
    {
        CClientPed* pPed = g_pClientGame->GetPedManager ()->Get ( ID, true );
        if ( pPed )
        {
            bool bRebuild = false;
            for ( unsigned short us = 0 ; us < usNumStats ; us++ )
            {
                unsigned short usStat;
                float fValue;
                if ( bitStream.Read ( usStat ) && bitStream.Read ( fValue ) )
                {
                    pPed->SetStat ( usStat, fValue );
                    if ( usStat == 21 || usStat == 23 )
                        bRebuild = true;
                }
            }
            if ( bRebuild )
                pPed->RebuildModel ();
        }
    }
}


void CPacketHandler::Packet_PlayerClothes ( NetBitStreamInterface& bitStream )
{
    unsigned char ucTextureLength, ucModelLength, ucType;
    unsigned short usNumClothes;
    ElementID ID;

    if ( bitStream.Read ( ID ) &&
         bitStream.Read ( usNumClothes ) )
    {
        CClientPed * pPed = g_pClientGame->GetPedManager ()->Get ( ID, true );
        if ( pPed )
        {            
            for ( unsigned short us = 0 ; us < usNumClothes ; us++ )
            {
                // Read out the texture
                bitStream.Read ( ucTextureLength );
                char* szTexture = new char [ ucTextureLength + 1 ]; szTexture [ ucTextureLength ] = 0;
                bitStream.Read ( szTexture, ucTextureLength );

                // Read out the model
                bitStream.Read ( ucModelLength );
                char* szModel = new char [ ucModelLength + 1 ]; szModel [ ucModelLength ] = 0;
                bitStream.Read ( szModel, ucModelLength );

                // Read out the type
                bitStream.Read ( ucType );
                pPed->GetClothes ()->AddClothes ( szTexture, szModel, ucType, false );

                // Clean up
                delete [] szModel;
                delete [] szTexture;
            }
            pPed->RebuildModel ( true );
        }
    }
}


void CPacketHandler::Packet_LuaEvent ( NetBitStreamInterface& bitStream )
{
    // Read out the event name length
    unsigned short usNameLength;
    if ( bitStream.Read ( usNameLength ) )
    {
        // Error?
        if ( usNameLength > (MAX_EVENT_NAME_LENGTH - 1) )
        {
            RaiseFatalError ( 13 );
            return;
        }

        // Read out the name and the entity id
        char* szName = new char [ usNameLength + 1 ];
        ElementID EntityID;
        if ( bitStream.Read ( szName, usNameLength ) && bitStream.Read ( EntityID ) )
        {
            // Null-terminate it
            szName [ usNameLength ] = 0;

            // Read out the arguments aswell
            CLuaArguments Arguments ( bitStream );

            // Grab the event. Does it exist and is it remotly triggerable?
            SEvent* pEvent = g_pClientGame->m_Events.Get ( szName );
            if ( pEvent )
            {
                if ( pEvent->bAllowRemoteTrigger )
                {
                    // Grab the element we trigger it on
                    CClientEntity* pEntity = CElementIDs::GetElement ( EntityID );
                    if ( pEntity )
                    {
                        pEntity->CallEvent ( szName, Arguments, true );
                    }
                }
                else
                    g_pClientGame->m_pScriptDebugging->LogError ( NULL, "Server triggered clientside event %s, but event is not marked as remotly triggerable", szName );
            }
            else
                g_pClientGame->m_pScriptDebugging->LogError ( NULL, "Server triggered clientside event %s, but event is not added clientside", szName );
        }

        // Delete event name again
        delete [] szName;
    }
}


void CPacketHandler::Packet_ResourceStart ( NetBitStreamInterface& bitStream )
{
    /*
    * unsigned char (1)   - resource name size
    * unsigned char (x)    - resource name
    * unsigned short (2)   - resource id
    * unsigned short (2)   - resource entity id
	* unsigned short (2)   - resource dynamic entity id
    * list of configs and scripts
    * * unsigned char (1)    - type chunk (F - File, E - Exported Function)
    * * unsigned char (1)    - file name size
    * * unsigned char (x)    - file name
    * * unsigned char (1)    - type
    * * unsigned long        - CRC
    * * double               - size
    * list of exported functions
    * * unsigned char (1)    - type chunk (F - File, E - Exported Function)
    * * unsigned char (1)    - function name size
    * * unsigned char (x)    - function name
    */

    CNetHTTPDownloadManagerInterface* pHTTP = g_pCore->GetNetwork ()->GetHTTPDownloadManager ();

    // Number of Resources to be Downloaded
    unsigned short usResourcesToBeDownloaded = 0;

    // Resource Name Size
    unsigned char ucResourceNameSize;
    bitStream.Read ( ucResourceNameSize );

    if ( ucResourceNameSize > MAX_RESOURCE_NAME_LENGTH )
    {
        RaiseFatalError ( 14 );
        return;
    }

    // Resource Name
    char* szResourceName = new char [ ucResourceNameSize + 1 ];
    bitStream.Read ( szResourceName, ucResourceNameSize );
    if (ucResourceNameSize)
       szResourceName [ ucResourceNameSize ] = NULL;

    // Resource ID
    unsigned short usResourceID;
    bitStream.Read ( usResourceID );

    // Resource Entity ID
    ElementID ResourceEntityID, ResourceDynamicEntityID;
    bitStream.Read ( ResourceEntityID );
	bitStream.Read ( ResourceDynamicEntityID );

    CResource* pResource = g_pClientGame->m_pResourceManager->Add ( usResourceID, szResourceName );
    if ( pResource )
    {
		// set the resource entity
        CClientEntity* pResourceEntity = CElementIDs::GetElement ( ResourceEntityID );
        if ( pResourceEntity )
        {
            pResource->SetResourceEntity ( pResourceEntity );
        }

		// set the resource dynamic entity
		CClientEntity* pResourceDynamicEntity = CElementIDs::GetElement ( ResourceDynamicEntityID );
		if ( pResourceDynamicEntity )
		{
			pResource->SetResourceDynamicEntity ( pResourceDynamicEntity );
		}

        // Resource Chunk Type (F = Resource File, E = Exported Function)
        unsigned char ucChunkType;
        // Resource Chunk Size
        unsigned char ucChunkSize;
        // Resource Chunk Data
        char* szChunkData;
        // Resource Chunk Sub Type
        unsigned char ucChunkSubType;
        // Resource Chunk CRC
        unsigned long ulChunkDataCRC;
        // Resource Chunk File Size
        double dChunkDataSize;

        while ( bitStream.Read ( ucChunkType ) )
        {
            switch ( ucChunkType )
            {
            case 'E': // Exported Function
                if ( bitStream.Read ( ucChunkSize ) )
                {
                    szChunkData = new char [ ucChunkSize + 1 ];
                    if ( ucChunkSize > 0 )
                    {
                        bitStream.Read ( szChunkData, ucChunkSize );
                    }
                    szChunkData [ ucChunkSize ] = NULL;

                    pResource->AddExportedFunction ( szChunkData );
                }

                break;
            case 'F': // Resource File
                if ( bitStream.Read ( ucChunkSize ) )
                {
                    szChunkData = new char [ ucChunkSize + 1 ];
                    if ( ucChunkSize > 0 )
                    {
                        bitStream.Read ( szChunkData, ucChunkSize );
                    }
                    szChunkData [ ucChunkSize ] = NULL;

                    bitStream.Read ( ucChunkSubType );
                    bitStream.Read ( ulChunkDataCRC );
                    bitStream.Read ( dChunkDataSize );

                    // Don't bother with empty files
                    if ( dChunkDataSize > 0 )
                    {
                        // Create the resource downloadable
                        CDownloadableResource* pDownloadableResource = NULL;
                        switch ( ucChunkSubType )
                        {
                            case CDownloadableResource::RESOURCE_FILE_TYPE_CLIENT_FILE:
                                pDownloadableResource = pResource->QueueFile ( CDownloadableResource::RESOURCE_FILE_TYPE_CLIENT_FILE, szChunkData, ulChunkDataCRC );

                                break;
                            case CDownloadableResource::RESOURCE_FILE_TYPE_CLIENT_SCRIPT:
                                pDownloadableResource = pResource->QueueFile ( CDownloadableResource::RESOURCE_FILE_TYPE_CLIENT_SCRIPT, szChunkData, ulChunkDataCRC );

                                break;
                            case CDownloadableResource::RESOURCE_FILE_TYPE_CLIENT_CONFIG:
                                pDownloadableResource = pResource->AddConfigFile ( szChunkData, ulChunkDataCRC );

                                break;
                            default:

                                break;
                        }

                        // Is it a valid downloadable resource?
                        if ( pDownloadableResource )
                        {
                            // Does the Client and Server CRC Match?
                            if ( !pDownloadableResource->DoesClientAndServerCRCMatch () )
                            {
                                // Make sure the directory exists
                                const char* szTempName = pDownloadableResource->GetName ();
                                if ( szTempName )
                                {
                                    // Make sure its directory exists
                                    MakeSureDirExists ( szTempName );
                                }

                                // Initialize a variable to hold the entire HTTP Download URL with the File Name
                                char szHTTPDownloadURLFull [MAX_HTTP_DOWNLOAD_URL_WITH_FILE + 1];
                                memset ( szHTTPDownloadURLFull, 0, sizeof ( szHTTPDownloadURLFull ) );

                                // Combine the HTTP Download URL, the Resource Name and the Resource File
                                _snprintf ( szHTTPDownloadURLFull, MAX_HTTP_DOWNLOAD_URL_WITH_FILE, "%s/%s/%s", g_pClientGame->m_szHTTPDownloadURL, pResource->GetName (), pDownloadableResource->GetShortName () );

                                // Delete the file that already exists
                                unlink ( pDownloadableResource->GetName () );

                                // Queue the file to be downloaded
                                pHTTP->QueueFile ( szHTTPDownloadURLFull, pDownloadableResource->GetName (), dChunkDataSize, NULL, NULL, NULL );

                                // If the file was successfully queued, increment the resources to be downloaded
                                usResourcesToBeDownloaded++;
                                g_pClientGame->m_pTransferBox->AddToTotalSize ( dChunkDataSize );
                            }						
                        }
                    }
                }

                break;
            }

            // Does the chunk data exist
            if ( szChunkData )
            {
                // Delete the chunk data
                delete [] szChunkData;
                szChunkData = NULL;
            }
        }

        // Are there any resources to be downloaded? Does it not transfer?
        if ( usResourcesToBeDownloaded > 0 ||
             g_pClientGame->m_bTransferResource )
        {
            if ( !pHTTP->IsDownloading () )
            {
                g_pClientGame->m_pTransferBox->Show ();

                pHTTP->StartDownloadingQueuedFiles ();

                g_pClientGame->m_bTransferResource = true;
            }
        }
        else
        {
            // Load the resource now
			if ( !pResource->GetActive() )
				pResource->Load ( g_pClientGame->m_pRootEntity );
        }
    }

    delete [] szResourceName;
    szResourceName = NULL;
}

void CPacketHandler::Packet_ResourceStop ( NetBitStreamInterface& bitStream )
{
    // unsigned short (2)    - resource id
    unsigned short usID;
    if ( bitStream.Read ( usID ) )
    {
        CResource* pResource = g_pClientGame->m_pResourceManager->GetResource ( usID );
        if ( pResource )
        {
            // Grab the resource entity
            CClientEntity* pResourceEntity = pResource->GetResourceEntity ();
            if ( pResourceEntity )
            {
                // Call our lua event
                CLuaArguments Arguments;
                Arguments.PushUserData ( pResource );
                pResourceEntity->CallEvent ( "onClientResourceStop", Arguments, true );
            }

			// Delete the resource
            g_pClientGame->m_pResourceManager->RemoveResource ( usID );
        }
    }
}


void CPacketHandler::Packet_DetonateSatchels ( NetBitStreamInterface& bitStream )
{
    ElementID Player;
    unsigned short usLatency;

    if ( bitStream.Read ( Player ) &&
         bitStream.Read ( usLatency ) )
    {
        // Grab the player
        if ( Player != INVALID_ELEMENT_ID )
        {
            CClientPlayer* pPlayer = g_pClientGame->m_pPlayerManager->Get ( Player );
            if ( pPlayer )
            {
                // TODO: Ping compensate
                
                pPlayer->DestroySatchelCharges ();
                if ( pPlayer->IsLocalPlayer () )
                {
                    pPlayer->RemoveWeapon ( WEAPONTYPE_DETONATOR );
                }
            }
        }
    }
}

#ifdef MTA_VOICE
/**
 * Decodes the data from an incoming voice packet and queues it on
 * the voice module, if it is loaded.
 *
 * @param bitStream The bitStream (packet) containing the data.
 */

void CPacketHandler::Packet_VoiceData ( NetBitStreamInterface& bitStream )
{
    unsigned short usPacketSize;
    if ( bitStream.Read ( usPacketSize ) )
    {
        // This should be secure enough.  We won't ever be newing 65K...
        char * pBuf = new char[usPacketSize];
        if ( bitStream.Read ( pBuf, usPacketSize ) )
        {
            // Voice may not be loaded...
            if (m_pVoice)
            {
                // Decode the sound data and queue it to be played.
                m_pVoice->DecodeAndQueueData((unsigned char *)pBuf, usPacketSize);
            }
        }
    }
}
#endif
