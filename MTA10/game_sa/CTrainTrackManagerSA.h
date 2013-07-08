/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTrainTrackManagerSA.h
*  PURPOSE:     Header file for train node manager class
*  DEVELOPERS:  Cazomino05 <Cazomino05@gmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_TRAINNODEMANAGERSA
#define __CGAMESA_TRAINNODEMANAGERSA

#include <game/CTrainTrackManager.h>
#include "CTrainTrackSA.h"

//006F6BD0  int GetTrainNodeNearPoint(float x, float y, float z, int* pTrackID) places track ID in *pTrackID and returns node ID
#define FUNC_GetTrainNodeNearPoint              0x6F6BD0
#define FUNC_CTrainNode_CTrainNode              0x6F5370
#define FUNC_CTrainNode_SetLengthFromStart      0x6F5490


#define COMP_NumberOfTracks                     0x6F6CA9

#define NUM_RAILTRACKS                          4
#define ARRAY_NumRailTrackNodes                 0xC38014            // NUM_RAILTRACKS dwords
#define ARRAY_TrackLengths                      0xC37FEC            // Track Length floats
#define ARRAY_RailTrackNodePointers             0xC38024            // NUM_RAILTRACKS pointers to arrays of SRailNode
#define MAX_TOTAL_TRACKS                        50+NUM_RAILTRACKS   // decent amount.

class CTrainTrackManagerSA : public CTrainTrackManager
{
    friend class CTrainTrackSA;
public:
                        CTrainTrackManagerSA            ( );
                        ~CTrainTrackManagerSA           ( );
    CTrainTrackSA *     GetTrainTrack                   ( unsigned char ucTrack );
    void                Initialise                      ( void );
    bool                GetOriginalRailNode             ( unsigned char ucTrack, unsigned short usNode, SRailNodeSA * pNode );
    void                ResetTracks                     ( void );
    void *              GetRailNodePointer              ( unsigned char ucTrack );
    float               GetRailLength                   ( unsigned char ucTrack );
    DWORD               GetNumberOfRailNodes            ( unsigned char ucTrack );

    bool                SetRailLength                   ( unsigned char ucTrack, float fLength );
    DWORD               SetNumberOfRailNodes            ( unsigned char ucTrack, DWORD dwTrackNodes );

    DWORD               GetRailNodePointers             ( void );

    CTrainTrackSA *     CreateTrainTrack                ( unsigned int uiNodes, unsigned char ucTrackID, bool bLinkedLastNode );


    bool                DestroyTrainTrack               ( DWORD dwTrackID );

    bool                IsValid                         ( unsigned char ucTrack );

private:
    bool                Reallocate                      ( CTrainTrackSA * pTrainTrack, unsigned int uiNodes );
    void *              AllocateDefault                 ( void );
    void                SetRailNodePointer              ( unsigned char ucTrack, void * pRailNodePointers );


    CTrainTrackSA *                                     m_pTrainTracks[MAX_TOTAL_TRACKS];
    void *                                              m_pRailNodePointers[MAX_TOTAL_TRACKS];
    float                                               m_fRailTrackLengths[MAX_TOTAL_TRACKS];
    DWORD                                               m_dwNumberOfTrackNodes[MAX_TOTAL_TRACKS];
    unsigned char                                       m_dwNumberOfTracks;
    bool                                                m_bInitialised;
    static std::map< unsigned int, SRailNodeSA >        m_OriginalTrainTrackData[4];

    // Default
    void *                                              m_pDefaultRailNodePointer;
    float                                               m_fDefaultRailTrackLengths;
    DWORD                                               m_dwDefaultNumberOfTrackNodes;
};

#endif
