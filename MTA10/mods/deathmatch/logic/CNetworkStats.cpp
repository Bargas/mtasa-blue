/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CNetworkStats.cpp
*  PURPOSE:     Network stats logger
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               Alberto Alonso <rydencillo@gmail.com>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CNetworkStats::CNetworkStats ( CClientDisplayManager* pDisplayManager )
{
    m_pDisplayManager = pDisplayManager;
    Reset ();
}

void CNetworkStats::Reset ( void )
{
    m_ulLastUpdateTime = CClientTime::GetTime ();

    unsigned int uiPacketsSent = g_pNet->GetPacketsSent ();
    unsigned int uiPacketsReceived = g_pNet->GetGoodPacketsReceived () + g_pNet->GetBadPacketsReceived ();
    unsigned int uiBitsReceived = g_pNet->GetBitsReceived ();
    unsigned int uiBitsSent = g_pNet->GetBitsSent ();

    m_uiPacketsSent = uiPacketsSent;
    m_uiPacketsReceived = uiPacketsReceived;
    m_uiBitsReceived = uiBitsReceived;
    m_uiBitsSent = uiBitsSent;

    m_uiLastPacketsSent = uiPacketsSent;
    m_uiLastPacketsReceived = uiPacketsReceived;
    m_uiLastBitsReceived = uiBitsReceived;
    m_uiLastBitsSent = uiBitsSent;
}

void CNetworkStats::Draw ( void )
{
    // Time to update?
    if ( CClientTime::GetTime () >= m_ulLastUpdateTime + 1000 )
    {
        Update();
    }

    // Draw the background
    float fResWidth = static_cast < float > ( g_pCore->GetGraphics ()->GetViewportWidth () );
    float fResHeight = static_cast < float > ( g_pCore->GetGraphics ()->GetViewportHeight () );
    g_pGame->GetHud ()->Draw2DPolygon ( 0.75f * fResWidth, 0.30f * fResHeight,
                                        1.0f * fResWidth, 0.30f * fResHeight,
                                        0.75f * fResWidth, 0.63f * fResHeight,
                                        1.0f * fResWidth, 0.63f * fResHeight,
                                        0x78000000 );

    // Grab the bytes sent/recv and datarate in the proper unit
    SString strBytesSent = GetDataUnit ( g_pNet->GetBitsSent () / 8 );
    SString strBytesRecv = GetDataUnit ( g_pNet->GetBitsReceived () / 8 );
    SString strRecvRate  = GetDataUnit ( ( m_uiBitsReceived - m_uiLastBitsReceived ) / 8 );
    SString strSendRate  = GetDataUnit ( ( m_uiBitsSent - m_uiLastBitsSent ) / 8 );

    // Populate a string to print
    SString strBuffer (
                "Ping: %u ms\n"
                "Messages in send buffer: %u\n"
                "Messages sent: %u\n"
                "Messages waiting ack: %u\n"
                "Messages resent: %u\n"
                "Acknowledges recv: %u\n"
                "Acknowledges sent: %u\n"
                "Acknowledges pending: %u\n"
                "Packets recv: %u\n"
                "Packets sent: %u\n"
                "Packet loss: %.2f%%\n"
                "Bytes recv: %s\n"
                "Bytes sent: %s\n"
                "Datarate: %s/s / %s/s\n"
                "Packet rate: %u / %u\n"
                "Compression ratio: %.3f%%\n"
                "Decompression ratio: %.3f%%",

                g_pNet->GetPing (),

                g_pNet->GetMessagesInSendBuffer (),
                g_pNet->GetMessagesSent (),
                g_pNet->GetMessagesWaitingAck (),
                g_pNet->GetMessagesResent (),

                g_pNet->GetAcknowledgesReceived (),
                g_pNet->GetAcknowledgesSent (),
                g_pNet->GetAcknowledgesPending (),

                g_pNet->GetGoodPacketsReceived () + g_pNet->GetBadPacketsReceived (),
                g_pNet->GetPacketsSent (),
                g_pNet->GetPacketLoss (),

                strBytesRecv.c_str (),
                strBytesSent.c_str (),
                strRecvRate.c_str (),
                strSendRate.c_str (),
                m_uiPacketsReceived - m_uiLastPacketsReceived,
                m_uiPacketsSent - m_uiLastPacketsSent,
                
                g_pNet->GetCompressionRatio () * 100.0f,
                g_pNet->GetDecompressionRatio () * 100.0f );

    // Print it
    m_pDisplayManager->DrawText2D ( strBuffer, CVector ( 0.76f, 0.31f, 0 ), 1.0f, 0xFFFFFFFF );
}


void CNetworkStats::Update ( void )
{
    m_ulLastUpdateTime = CClientTime::GetTime ();

    m_uiLastPacketsSent = m_uiPacketsSent;
    m_uiLastPacketsReceived = m_uiPacketsReceived;
    m_uiLastBitsReceived = m_uiBitsReceived;
    m_uiLastBitsSent = m_uiBitsSent;

    m_uiPacketsSent = g_pNet->GetPacketsSent ();
    m_uiPacketsReceived = g_pNet->GetGoodPacketsReceived () + g_pNet->GetBadPacketsReceived ();
    m_uiBitsReceived = g_pNet->GetBitsReceived ();
    m_uiBitsSent = g_pNet->GetBitsSent ();
}
