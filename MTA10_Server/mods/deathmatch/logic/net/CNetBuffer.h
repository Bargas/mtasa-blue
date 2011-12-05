/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/


// Base class for net function arguments
struct SArgs
{
                    SArgs       ( void )    { DEBUG_CREATE_COUNT( "SArgs" ); }
    virtual         ~SArgs      ( void )    { DEBUG_DESTROY_COUNT( "SArgs" ); }
    int type;
};


typedef void (*PFN_NETRESULT) ( class CNetJobData* pJobData, void* pContext );

//
// All data realating to a net job
//
class CNetJobData
{
public:
    ZERO_ON_NEW
    bool        SetCallback     ( PFN_NETRESULT pfnNetResult, void* pContext );
    bool        HasCallback     ( void );
    void        ProcessCallback ( void );

    CNetJobData                 ( void )    { DEBUG_CREATE_COUNT( "CNetJobData" ); }
    ~CNetJobData ( void )
    {
        SAFE_DELETE( pArgs );
        DEBUG_DESTROY_COUNT( "CNetJobData" );
    }

    EJobStageType   stage;
    SArgs*         pArgs;
    bool            bAutoFree;

    struct
    {
        PFN_NETRESULT       pfnNetResult;
        void*               pContext;
        bool                bSet;
        bool                bDone;
    } callback;
};


//
// Replacement net interface implementing threaded pure sync packet bouncing!
//
class CNetServerBuffer : public CNetServer
{
public:
    ZERO_ON_NEW
                                            CNetServerBuffer                ( CSimPlayerManager* pSimPlayerManager );
    virtual                                 ~CNetServerBuffer               ( void );

    // CNetServer interface
    virtual bool                            StartNetwork                    ( const char* szIP, unsigned short usServerPort, unsigned int uiAllowedPlayers );
    virtual void                            StopNetwork                     ( void );
    virtual void                            ResetNetwork                    ( void );

    virtual void                            DoPulse                         ( void );

    virtual void                            RegisterPacketHandler           ( PPACKETHANDLER pfnPacketHandler );

    virtual bool                            GetNetworkStatistics            ( NetStatistics* pDest, NetServerPlayerID& PlayerID );
    virtual const SPacketStat*              GetPacketStats                  ( void );
    virtual bool                            GetBandwidthStatistics          ( SBandwidthStatistics* pDest );

    virtual NetBitStreamInterface*          AllocateNetServerBitStream      ( unsigned short usBitStreamVersion );
    virtual void                            DeallocateNetServerBitStream    ( NetBitStreamInterface* bitStream );
    virtual bool                            SendPacket                      ( unsigned char ucPacketID, NetServerPlayerID& playerID, NetBitStreamInterface* bitStream, bool bBroadcast = false, NetServerPacketPriority packetPriority = PACKET_PRIORITY_LOW, NetServerPacketReliability packetReliability = PACKET_RELIABILITY_RELIABLE_ORDERED, NetServerPacketOrdering packetOrdering = PACKET_ORDERING_GAME );

    virtual void                            GetPlayerIP                     ( NetServerPlayerID& playerID, char strIP[22], unsigned short* usPort );

    virtual void                            AddBan                          ( const char* szIP );
    virtual void                            RemoveBan                       ( const char* szIP );
    virtual bool                            IsBanned                        ( const char* szIP );

    virtual void                            Kick                            ( NetServerPlayerID &PlayerID );

    virtual void                            SetPassword                     ( const char* szPassword );

    virtual void                            SetMaximumIncomingConnections   ( unsigned short numberAllowed );

    virtual CNetHTTPDownloadManagerInterface*   GetHTTPDownloadManager      ( void );

    virtual void                            SetClientBitStreamVersion       ( const NetServerPlayerID &PlayerID, unsigned short usBitStreamVersion );
    virtual void                            ClearClientBitStreamVersion     ( const NetServerPlayerID &PlayerID );

    virtual void                            SetChecks                       ( const std::set < SString >& disableComboACMap, const std::set < SString >& disableACMap, const std::set < SString >& enableSDMap, int iEnableClientChecks, bool bHideAC );

    virtual unsigned int                    GetPendingPacketCount           ( void );

    virtual bool                            InitServerId                    ( const char* szPath );
    virtual void                            SetEncryptionEnabled            ( bool bEncryptionEnabled );
    virtual void                            ResendModPackets                ( NetServerPlayerID& playerID );

    virtual void                            GetClientSerialAndVersion       ( NetServerPlayerID& playerID, CStaticString < 32 >& strSerial, CStaticString < 32 >& strVersion );

    //
    // Macros of doom to declare function argument structures
    //
    #define DOTYPE(func) const static int TYPE_##func = __COUNTER__;
    #define SETTYPE(func) type=TYPE_##func;

    #define DECLARE_FUNC_ARGS0(func)                                DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( ) {SETTYPE(func)} };
    #define DECLARE_FUNC_ARGS1(func,t1,n1)                          DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( t1 n1 ) : n1(n1) {SETTYPE(func)} t1 n1; };
    #define DECLARE_FUNC_ARGS2(func,t1,n1,t2,n2)                    DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( t1 n1,t2 n2 ) : n1(n1), n2(n2) {SETTYPE(func)} t1 n1; t2 n2; };
    #define DECLARE_FUNC_ARGS3(func,t1,n1,t2,n2,t3,n3)              DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( t1 n1,t2 n2,t3 n3 ) : n1(n1), n2(n2), n3(n3) {SETTYPE(func)} t1 n1; t2 n2; t3 n3; };
    #define DECLARE_FUNC_ARGS5(func,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5)  DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( t1 n1,t2 n2,t3 n3,t4 n4,t5 n5 ) : n1(n1), n2(n2), n3(n3), n4(n4), n5(n5) {SETTYPE(func)} t1 n1; t2 n2; t3 n3; t4 n4; t5 n5; };

    #define DECLARE_FUNC_ARGS0R(ret,func)                                           DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( ) {SETTYPE(func)} ret result; };
    #define DECLARE_FUNC_ARGS1R(ret,func,t1,n1)                                     DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( t1 n1 ) : n1(n1) {SETTYPE(func)} t1 n1; ret result; };
    #define DECLARE_FUNC_ARGS2R(ret,func,t1,n1,t2,n2)                               DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( t1 n1,t2 n2 ) : n1(n1), n2(n2) {SETTYPE(func)} t1 n1; t2 n2; ret result; };
    #define DECLARE_FUNC_ARGS3R(ret,func,t1,n1,t2,n2,t3,n3)                         DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( t1 n1,t2 n2,t3 n3 ) : n1(n1), n2(n2), n3(n3) {SETTYPE(func)} t1 n1; t2 n2; t3 n3; ret result; };
    #define DECLARE_FUNC_ARGS4R(ret,func,t1,n1,t2,n2,t3,n3,t4,n4)                   DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( t1 n1,t2 n2,t3 n3,t4 n4 ) : n1(n1), n2(n2), n3(n3), n4(n4) {SETTYPE(func)} t1 n1; t2 n2; t3 n3; t4 n4; ret result; };
    #define DECLARE_FUNC_ARGS7R(ret,func,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6,t7,n7) DOTYPE(func) struct S##func##Args : SArgs { S##func##Args ( t1 n1,t2 n2,t3 n3,t4 n4,t5 n5,t6 n6,t7 n7 ) : n1(n1), n2(n2), n3(n3), n4(n4), n5(n5), n6(n6), n7(n7) {SETTYPE(func)} t1 n1; t2 n2; t3 n3; t4 n4; t5 n5; t6 n6; t7 n7; ret result; };

    typedef NetServerPlayerID NetServerPlayerIDRef;

    DECLARE_FUNC_ARGS3R( bool,                  StartNetwork                    , const char*, szIP, unsigned short, usServerPort, unsigned int, uiAllowedPlayers );
    DECLARE_FUNC_ARGS0 (                        StopNetwork                     );
    DECLARE_FUNC_ARGS0 (                        ResetNetwork                    );
    DECLARE_FUNC_ARGS0 (                        DoPulse                         );
    DECLARE_FUNC_ARGS1 (                        RegisterPacketHandler           , PPACKETHANDLER, pfnPacketHandler );
    DECLARE_FUNC_ARGS2R( bool,                  GetNetworkStatistics            , NetStatistics*, pDest, NetServerPlayerIDRef, PlayerID );
    DECLARE_FUNC_ARGS0R( const SPacketStat*,    GetPacketStats                  );
    DECLARE_FUNC_ARGS1R( bool,                  GetBandwidthStatistics          , SBandwidthStatistics*, pDest );
    DECLARE_FUNC_ARGS7R( bool,                  SendPacket                      , unsigned char, ucPacketID, NetServerPlayerIDRef, playerID, NetBitStreamInterface*, bitStream, bool, bBroadcast, NetServerPacketPriority, packetPriority, NetServerPacketReliability, packetReliability, NetServerPacketOrdering, packetOrdering );
    DECLARE_FUNC_ARGS3 (                        GetPlayerIP                     , NetServerPlayerIDRef, playerID, char*, strIP, unsigned short*, usPort );
    DECLARE_FUNC_ARGS1 (                        Kick                            , NetServerPlayerIDRef, PlayerID );
    DECLARE_FUNC_ARGS1 (                        SetPassword                     , const char*, szPassword );
    DECLARE_FUNC_ARGS1 (                        SetMaximumIncomingConnections   , unsigned short, numberAllowed );
    DECLARE_FUNC_ARGS2 (                        SetClientBitStreamVersion       , const NetServerPlayerIDRef, PlayerID, unsigned short, usBitStreamVersion );
    DECLARE_FUNC_ARGS1 (                        ClearClientBitStreamVersion     , const NetServerPlayerIDRef, PlayerID );
    DECLARE_FUNC_ARGS5 (                        SetChecks                       , const std::set < SString >&, disableComboACMap, const std::set < SString >&, disableACMap, const std::set < SString >&, enableSDMap, int, iEnableClientChecks, bool, bHideAC );
    DECLARE_FUNC_ARGS0R( unsigned int,          GetPendingPacketCount           );
    DECLARE_FUNC_ARGS1R( bool,                  InitServerId                    , const char*, szPath );
    DECLARE_FUNC_ARGS1 (                        SetEncryptionEnabled            , bool, bEncryptionEnabled );
    DECLARE_FUNC_ARGS1 (                        ResendModPackets                , NetServerPlayerIDRef, playerID );
    DECLARE_FUNC_ARGS3 (                        GetClientSerialAndVersion       , NetServerPlayerIDRef, playerID, CStaticString < 32 >&, strSerial, CStaticString < 32 >&, strVersion );
    DECLARE_FUNC_ARGS4R( bool,                  ProcessPacket                   , unsigned char, ucPacketID, NetServerPlayerIDRef, Socket, NetBitStreamInterface*, BitStream, SNetExtraInfo*, pNetExtraInfo );


    // Main thread functions
    void                        StopThread                  ( void );
    CNetJobData*                AddCommand                  ( SArgs* pArgs, bool bAutoFree );
    void                        AddCommandAndFree           ( SArgs* pArgs );
    void                        AddCommandAndWait           ( SArgs* pArgs );
    void                        AddCommandAndCallback       ( SArgs* pArgs, PFN_NETRESULT pfnNetResult, void* pContext );
    bool                        PollCommand                 ( CNetJobData* pJobData, uint uiTimeout );
    CNetJobData*                GetNewJobData               ( void );
    void                        ProcessIncoming             ( void );
    void                        SetAutoPulseEnabled         ( bool bEnable );

    // Sync thread functions
    static void*                StaticThreadProc            ( void* pContext );
    void*                       ThreadProc                  ( void );
    void                        ProcessCommand              ( CNetJobData* pJobData );
    static bool                 StaticProcessPacket         ( unsigned char ucPacketID, NetServerPlayerID& Socket, NetBitStreamInterface* BitStream, SNetExtraInfo* pNetExtraInfo );
    bool                        ProcessPacket               ( unsigned char ucPacketID, NetServerPlayerID& Socket, NetBitStreamInterface* BitStream, SNetExtraInfo* pNetExtraInfo );

    // Either thread functions
    void                        UpdateQueueStats            ( void );

    // Main thread variables
    PPACKETHANDLER                      m_pfnDMPacketHandler;
    CThreadHandle*                      m_pServiceThreadHandle;
    std::set < CNetJobData* >           m_FinishedList;         // Result has been used, will be deleted next pulse

    // Sync thread variables
    CNetServer*                         m_pRealNetServer;
    CSimPlayerManager*                  m_pSimPlayerManager;
    CTickCount                          m_LastPulseTime;

    // Shared variables
    struct
    {
        bool                                        m_bTerminateThread;
        bool                                        m_bThreadTerminated;
        bool                                        m_bAutoPulse;
        std::list < CNetJobData* >                  m_OutCommandQueue;
        std::list < CNetJobData* >                  m_OutResultQueue;
        std::list < SProcessPacketArgs* >           m_InResultQueue;
        CDebugComboMutex                            m_Mutex;
        CNetBufferWatchDog*                         m_pWatchDog;
    } shared;
};