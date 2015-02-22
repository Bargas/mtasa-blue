/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientTXD.h
*  PURPOSE:     TXD manager class header

*
*****************************************************************************/

#ifndef __CCLIENTTXD_H
#define __CCLIENTTXD_H

#include "CClientEntity.h"

class CClientTXD : public CClientEntity
{
    DECLARE_CLASS( CClientTXD, CClientEntity )
public:
                                    CClientTXD              ( class CClientManager* pManager, ElementID ID );
                                    ~CClientTXD             ( void );

    void                            Unlink                  ( void ) {};
    void                            GetPosition             ( CVector& vecPosition ) const {};
    void                            SetPosition             ( const CVector& vecPosition ) {};

    eClientEntityType               GetType                 ( void ) const              { return CCLIENTTXD; }
    bool                            LoadTXD                 ( const SString& strFile, bool bFilteringEnabled, bool bIsRawData );
    bool                            Import                  ( unsigned short usModelID );
    static bool                     IsImportableModel       ( unsigned short usModelID );
    static bool                     IsTXDData               ( const SString& strData );

protected:
    void                            Restream                ( unsigned short usModel );
    bool                            LoadFileData            ( CBuffer& buffer );

    SString                         m_strFilename;
    bool                            m_bFilteringEnabled;
    bool                            m_bIsRawData;
    bool                            m_bUsingFileDataForClothes;
    CBuffer                         m_FileData;
    SReplacementTextures            m_ReplacementTextures;
};

#endif
