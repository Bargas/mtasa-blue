/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/


class CSimPlayerManager
{
public:
    ZERO_ON_NEW

    // Main thread methods
    void            AddSimPlayer            ( CPlayer* pPlayer );
    void            RemoveSimPlayer         ( CPlayer* pPlayer );
    void            UpdateSimPlayer         ( CPlayer* pPlayer, const std::vector < CPlayer* >& simSendList );

    // Any thread methods
    void            LockSimSystem           ( void );
    void            UnlockSimSystem         ( void );

    // Sync thread methods
    bool            HandlePlayerPureSync    ( NetServerPlayerID& Socket, NetBitStreamInterface* BitStream );
    bool            HandleVehiclePureSync   ( NetServerPlayerID& Socket, NetBitStreamInterface* BitStream );
    CSimPlayer*     Get                     ( NetServerPlayerID& PlayerSocket );
    void            Broadcast               ( const CSimPacket& Packet, const std::vector < CSimPlayer* >& sendList );

protected:
    // Shared variables
    bool                                        m_bIsLocked;
    CCriticalSection                            m_CS;
    std::set < CSimPlayer* >                    m_AllSimPlayerMap;
    std::map < NetServerPlayerID, CSimPlayer* > m_SocketSimMap;
};

