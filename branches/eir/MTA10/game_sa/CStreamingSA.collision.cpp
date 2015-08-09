/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.collision.cpp
*  PURPOSE:     Collision data streaming
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

// Instantiate single copies of collision manager members.
unsigned int CColSectorManagerSA::m_loadAreaId = 0;

/*=========================================================
    RegisterCOLLibraryModel

    Arguments:
        collId - index of the COL library
        modelId - model info id to which a collision was applied
    Purpose:
        Extends the range of applicability for the given COL lib.
        This loading of collisions for said COL library can be
        limited to a range so that future loading attempts are
        boosted.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00410820
=========================================================*/
void __cdecl RegisterCOLLibraryModel( unsigned short collId, unsigned short modelId )
{
    CColFileSA *colFile = Streaming::GetCOLEnvironment().m_pool->Get( collId );

    colFile->m_range.Add( (short)modelId );
}

/*=========================================================
    ReadCOLLibraryGeneral

    Arguments:
        buf - binary string of the COL library
        size - size of the memory pointed at by buf
        collId - COL library index
    Purpose:
        Reads a COL library from memory pointed at by buf and assigns all
        collision entries to the COL library designated with collId. During
        the scan for models which need collisions the range of applicance
        is expanded. The next loading of this COL library will be boosted.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x005B5000
    Update:
        Added support for version 4 collision.
=========================================================*/
bool __cdecl ReadCOLLibraryGeneral( const char *buf, size_t size, unsigned char collId )
{
    CBaseModelInfoSAInterface *info = NULL;

    while ( size > 8 )
    {
        const ColModelFileHeader& header = *(const ColModelFileHeader*)buf;

        buf += sizeof(header);

        if ( header.checksum != '4LOC' && header.checksum != '3LOC' && header.checksum != '2LOC' && header.checksum != 'LLOC' )
            return true;

        modelId_t modelId = header.modelId;

        // Collisions may come with a cached model id.
        // Valid ids skip the need for name-checking.
        if ( modelId < MAX_MODELS )
            info = ppModelInfo[modelId];

        unsigned int hash = pGame->GetKeyGen()->GetUppercaseKey( header.name );

        if ( !info || hash != info->GetHashKey() )
            info = Streaming::GetModelByHash( hash, &modelId );

        // I am not a fan of uselessly big scopes.
        // The closer the code is to the left border, the easier it is to read for everybody.
        // Compilers do optimize goto.
        if ( !info )
            goto skip;

        // Update collision boundaries
        RegisterCOLLibraryModel( collId, modelId );

        // I do not expect collision replacements to be loaded this early.
        // The engine does preload the world collisions once. Further
        // loadings will be faster due to limitation of model scans to
        // id regions.
        assert( g_colReplacement[modelId] == NULL );

        if ( !info->IsDynamicCol() )
            goto skip;

        CColModelSAInterface *col = new CColModelSAInterface();

        switch( header.checksum )
        {
        case '4LOC':
            LoadCollisionModelVer4( buf, header.size - 0x18, col, header.name );
            break;
        case '3LOC':
            LoadCollisionModelVer3( buf, header.size - 0x18, col, header.name );
            break;
        case '2LOC':
            LoadCollisionModelVer2( buf, header.size - 0x18, col, header.name );
            break;
        default:
            LoadCollisionModel( buf, col, header.name );
            break;
        }

        // MTA extension: Put it into our global storage
        g_originalCollision[modelId] = col;

        col->m_colPoolIndex = collId;

        info->SetColModel( col, true );
        Cache_StoreCollision( modelId, col );

skip:
        size -= header.size;
        buf += header.size - (sizeof(ColModelFileHeader) - 8);
    }

    return true;
}

/*=========================================================
    ReadCOLLibraryBounds

    Arguments:
        buf - binary string of the COL library
        size - size of the memory pointed at by buf
        collId - COL library index
    Purpose:
        Reads a COL library from memory pointed at by buf and assigns all
        collision entries to the COL library designated with collId. It uses
        the cached range of applicancy to boost name-based collision loading.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00538440
=========================================================*/
bool __cdecl ReadCOLLibraryBounds( const char *buf, size_t size, unsigned char collId )
{
    CBaseModelInfoSAInterface *info = NULL;

    while ( size > 8 )
    {
        const ColModelFileHeader& header = *(const ColModelFileHeader*)buf;

        buf += sizeof(header);

        // Note: this function has version 4 support by default!
        // It never worked properly since ReadCOLLibraryGeneral rejected version 4 by default.
        // We have updated the engine by allowing version 4 in ReadCOLLibraryGeneral!
        if ( header.checksum != '4LOC' && header.checksum != '3LOC' && header.checksum != '2LOC' && header.checksum != 'LLOC' )
            return true;

        modelId_t modelId = header.modelId;

        if ( modelId < MAX_MODELS )
            info = ppModelInfo[modelId];

        unsigned int hash = pGame->GetKeyGen()->GetUppercaseKey( header.name );

        if ( !info || hash != info->GetHashKey() )
        {
            CColFileSA *colFile = Streaming::GetCOLEnvironment().m_pool->Get( collId );

            info = Streaming::GetModelInfoByName( header.name, (unsigned short)colFile->m_range.start, (unsigned short)colFile->m_range.end, &modelId );
        }

        if ( info )
        {
#ifdef _MTA_BLUE
            // Wire in a MTA team fix.
            if ( OnMY_CFileLoader_LoadCollisionFile_Mid( modelId ) )
            {
#endif //_MTA_BLUE
                CColModelSAInterface *col = NULL;
                bool isDynamic = false;

                if ( CColModelSA *colInfo = g_colReplacement[modelId] )
                {
                    // MTA extension: We store it in our data, so we can restore to it later
                    col = colInfo->GetOriginal( modelId, isDynamic );

                    if ( !col )
                    {
                        col = new CColModelSAInterface;

                        colInfo->SetOriginal( modelId, col, true );

                        isDynamic = true;
                    }
                }
                else
                {
                    isDynamic = info->IsDynamicCol();

                    // The original route
                    col = info->pColModel;

                    if ( !col )
                    {
                        col = new CColModelSAInterface;

                        info->SetCollision( col, true );

                        isDynamic = true;
                    }
                }

                if ( col && isDynamic )
                {
                    // Make sure there is no collision data attached to it anymore.
                    col->ReleaseData();

                    switch( header.checksum )
                    {
                    case '4LOC':
                        LoadCollisionModelVer4( buf, header.size - 0x18, col, header.name );
                        break;
                    case '3LOC':
                        LoadCollisionModelVer3( buf, header.size - 0x18, col, header.name );
                        break;
                    case '2LOC':
                        LoadCollisionModelVer2( buf, header.size - 0x18, col, header.name );
                        break;
                    default:
                        LoadCollisionModel( buf, col, header.name );
                        break;
                    }

                    // MTA extension: Put it into our global storage
                    g_originalCollision[modelId] = col;

                    col->m_colPoolIndex = collId;

                    // Do some procedural object logic
                    if ( info->GetModelType() == MODEL_ATOMIC )
                        ((bool (__cdecl*)( CBaseModelInfoSAInterface *info ))0x005DB650)( info );
                }
#ifdef _MTA_BLUE
            }
#endif //_MTA_BLUE
        }
        
        size -= header.size;
        buf += header.size - (sizeof(ColModelFileHeader) - 8);
    }

    return true;
}

/*=========================================================
    LoadCOLLibrary

    Arguments:
        collId - COL library index
        buf - binary string of the COL library
        size - size of the memory pointed at by buf
    Purpose:
        Loads a COL library into the container designated by collId.
        If no range has been cached to the COL library, it executes
        ReadCOLLibraryGeneral. Otherwise the loading is accelerated
        by knowing the model id bounds.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004106D0
=========================================================*/
bool __cdecl LoadCOLLibrary( unsigned char collId, const char *buf, size_t size )
{
    return Streaming::GetCOLEnvironment().LoadSector( collId, buf, size );
}

/*=========================================================
    FreeCOLLibrary

    Arguments:
        collId - COL library index
    Purpose:
        Unloads a specific COL library and deletes the collision
        data from affected models.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00410730
    Note:
        This function has been updated so it is compatible with
        the MTA model loading system. Models whose collisions were
        replaced are not affected by this function anymore.
=========================================================*/
void __cdecl FreeCOLLibrary( unsigned char collId )
{
    Streaming::GetCOLEnvironment().UnloadSector( collId );
}

/*=========================================================
    _SetCollisionLoadPosition

    Arguments:
        pos - position to request collision sectors around
    Purpose:
        Sets the position that should be used for additional
        collision file sector loading. The sectors are loaded
        during the next call to the management function.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004103A0
=========================================================*/
static void __cdecl _SetCollisionLoadPosition( const CVector& pos )
{
    Streaming::GetCOLEnvironment().SetLoadPosition( pos );
}

/*=========================================================
    _StreamCollisionSectors

    Arguments:
        pos - position to request collision sectors around
        calcVelocity - whether it should apply velocity logic to position
    Purpose:
        Loads all collision sectors that are intersecting
        the given position. Unloads all the sectors, that
        are not in streaming distance anymore.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00410860
=========================================================*/
static void __cdecl _StreamCollisionSectors( CVector pos, bool calcVelocity )
{
    Streaming::GetCOLEnvironment().StreamSectors( pos, calcVelocity );
}

/*=========================================================
    _PrioritizeCollisionSectors

    Arguments:
        pos - position to request collision sectors around
    Purpose:
        Acts as a second loader runtime. This routine prioritizes
        sectors that are close to the player. It waits till
        they are completely loaded. Otherwise the player might
        fall through the world if resources take too long to
        load.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00410AD0
=========================================================*/
static void __cdecl _PrioritizeCollisionSectors( const CVector& pos )
{
    Streaming::GetCOLEnvironment().PrioritizeLocalStreaming( pos );
}

void StreamingCOL_Init( void )
{
    // Hook stuff.
    HookInstall( 0x004103A0, (DWORD)_SetCollisionLoadPosition, 5 );
    HookInstall( 0x00410860 + 5, (DWORD)_StreamCollisionSectors, 5 );   // due to securom, we have to offset by 5
    HookInstall( 0x00410AD0, (DWORD)_PrioritizeCollisionSectors, 5 );
}

void StreamingCOL_Shutdown( void )
{
}