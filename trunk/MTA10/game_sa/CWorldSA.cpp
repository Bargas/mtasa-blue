/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CWorldSA.cpp
*  PURPOSE:     Game world/entity logic
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Sebas Lamers <sebasdevelopment@gmx.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
CWorldSA::CWorldSA ( )
{
    m_pBuildings = new std::multimap< unsigned short, SBuildingRemoval* >;
    m_pRemovedObjects = new std::map < unsigned short, unsigned short >;
    m_pDataBuildings = new std::multimap < unsigned short, sDataBuildingRemoval* >;
}

void CWorldSA::Add ( CEntity * pEntity )
{
    DEBUG_TRACE("VOID CWorldSA::Add ( CEntity * pEntity )");

    CEntitySA* pEntitySA = dynamic_cast < CEntitySA* > ( pEntity );

    if ( pEntitySA )
    {
        DWORD dwEntity = (DWORD) pEntitySA->GetInterface();
        DWORD dwFunction = FUNC_Add;
        _asm
        {
            push    dwEntity
            call    dwFunction
            add     esp, 4
        }
    }
}


void CWorldSA::Add ( CEntitySAInterface * entityInterface )
{
    DEBUG_TRACE("VOID CWorldSA::Add ( CEntitySAInterface * entityInterface )");
    DWORD dwFunction = FUNC_Add;
    _asm
    {
        push    entityInterface
        call    dwFunction
        add     esp, 4
    }
}

void CWorldSA::Remove ( CEntity * pEntity )
{
    DEBUG_TRACE("VOID CWorldSA::Remove ( CEntity * entity )");

    CEntitySA* pEntitySA = dynamic_cast < CEntitySA* > ( pEntity );

    if ( pEntitySA )
    {
        DWORD dwEntity = (DWORD)pEntitySA->GetInterface();
        DWORD dwFunction = FUNC_Remove;
        _asm
        {
            push    dwEntity
            call    dwFunction
            add     esp, 4
        }
    }
}

void CWorldSA::Remove ( CEntitySAInterface * entityInterface )
{
    DEBUG_TRACE("VOID CWorldSA::Remove ( CEntitySAInterface * entityInterface )");
    DWORD dwFunction = FUNC_Remove;
    _asm
    {
        push    entityInterface
        call    dwFunction
        add     esp, 4

    /*  mov     ecx, entityInterface
        mov     esi, [ecx]
        push    1
        call    dword ptr [esi+8]*/             
    }
}

void CWorldSA::RemoveReferencesToDeletedObject ( CEntitySAInterface * entity )
{
    DWORD dwFunc = FUNC_RemoveReferencesToDeletedObject;
    DWORD dwEntity = (DWORD)entity;
    _asm
    {
        push    dwEntity
        call    dwFunc
        add     esp, 4
    }
}

bool CWorldSA::TestLineSphere(CVector * vecStart, CVector * vecEnd, CVector * vecSphereCenter, float fSphereRadius, CColPoint ** colCollision )
{
    // THIS FUNCTION IS INCOMPLETE AND SHOULD NOT BE USED
    // Create a CColLine for us
    DWORD dwFunc = FUNC_CColLine_Constructor;
    DWORD dwCColLine[10]; // I don't know how big CColLine is, so we'll just be safe
    _asm
    {
        lea     ecx, dwCColLine
        push    vecEnd
        push    vecStart
        call    dwFunc
    }

    // Now, lets make a CColSphere
    BYTE byteColSphere[18]; // looks like its 18 bytes { vecPos, fSize, byteUnk, byteUnk, byteUnk }
    dwFunc = FUNC_CColSphere_Set;
    _asm
    {
        lea     ecx, byteColSphere
        push    255
        push    0
        push    0
        push    vecSphereCenter
        push    fSphereRadius
        call    dwFunc
    }
}


void ConvertMatrixToEulerAngles ( const CMatrix_Padded& matrixPadded, float& fX, float& fY, float& fZ )
{
    // Convert the given matrix to a padded matrix
    //CMatrix_Padded matrixPadded ( Matrix );

    // Grab its pointer and call gta's func
    const CMatrix_Padded* pMatrixPadded = &matrixPadded;
    DWORD dwFunc = FUNC_CMatrix__ConvertToEulerAngles;

    float* pfX = &fX;
    float* pfY = &fY;
    float* pfZ = &fZ;
    int iUnknown = 21;
    _asm
    {
        push    iUnknown
            push    pfZ
            push    pfY
            push    pfX
            mov     ecx, pMatrixPadded
            call    dwFunc
    }
}


bool CWorldSA::ProcessLineOfSight(const CVector * vecStart, const CVector * vecEnd, CColPoint ** colCollision, 
                                  CEntity ** CollisionEntity,
                                  const SLineOfSightFlags flags,
                                  SLineOfSightBuildingResult* pBuildingResult )
{
    DEBUG_TRACE("VOID CWorldSA::ProcessLineOfSight(CVector * vecStart, CVector * vecEnd, CColPoint * colCollision, CEntity * CollisionEntity)");
    DWORD dwPadding[100]; // stops the function missbehaving and overwriting the return address
    dwPadding [0] = 0;  // prevent the warning and eventual compiler optimizations from removing it

    CColPointSA * pColPointSA = new CColPointSA();
    CColPointSAInterface * pColPointSAInterface = pColPointSA->GetInterface();  

    //DWORD targetEntity;
    CEntitySAInterface * targetEntity = NULL;
    bool bReturn = false;

    DWORD dwFunc = FUNC_ProcessLineOfSight;
    // bool bCheckBuildings = true,                 bool bCheckVehicles = true,     bool bCheckPeds = true, 
    // bool bCheckObjects = true,                   bool bCheckDummies = true,      bool bSeeThroughStuff = false, 
    // bool bIgnoreSomeObjectsForCamera = false,    bool bShootThroughStuff = false

    _asm
    {
        push    flags.bShootThroughStuff
        push    flags.bIgnoreSomeObjectsForCamera
        push    flags.bSeeThroughStuff
        push    flags.bCheckDummies
        push    flags.bCheckObjects
        push    flags.bCheckPeds
        push    flags.bCheckVehicles
        push    flags.bCheckBuildings
        lea     eax, targetEntity
        push    eax
        push    pColPointSAInterface    
        push    vecEnd
        push    vecStart    
        call    dwFunc
        mov     bReturn, al
        add     esp, 0x30
    }

    // Building info needed?
    if ( pBuildingResult )
    {
        CPoolsSA * pPools = ((CPoolsSA *)pGame->GetPools());
        if ( pPools )
        {
            if ( targetEntity && targetEntity->nType == ENTITY_TYPE_BUILDING )
            {
                pBuildingResult->bValid = true;
                pBuildingResult->usModelID = targetEntity->m_nModelIndex;
                pBuildingResult->vecPosition = targetEntity->Placeable.m_transform.m_translate;
                if ( targetEntity->Placeable.matrix )
                {
                    CVector& vecRotation = pBuildingResult->vecRotation;
                    ConvertMatrixToEulerAngles ( *targetEntity->Placeable.matrix, vecRotation.fX, vecRotation.fY, vecRotation.fZ );
                    vecRotation = -vecRotation;
                }
            }
        }
    }


    if ( CollisionEntity )
    {
        CPoolsSA * pPools = ((CPoolsSA *)pGame->GetPools());
        if(pPools)
        {
            if(targetEntity)
            {
                switch (targetEntity->nType)
                {
                    case ENTITY_TYPE_PED:
                        *CollisionEntity = pPools->GetPed((DWORD *)targetEntity);
                        break;
                    case ENTITY_TYPE_OBJECT:
                        *CollisionEntity = pPools->GetObject((DWORD *)targetEntity);
                        break;
                    case ENTITY_TYPE_VEHICLE:
                        *CollisionEntity = pPools->GetVehicle((DWORD *)targetEntity);
                        break;
                }

                /*CEntitySA * entity = new CEntitySA();
                entity->SetInterface((CEntitySAInterface *)targetEntity);
                eEntityType EntityType = entity->GetEntityType();
                delete entity;
                switch(EntityType)
                {
                case ENTITY_TYPE_PED:
                case ENTITY_TYPE_OBJECT:
                    *CollisionEntity = pPools->GetPed((DWORD *)targetEntity);
                    if ( *CollisionEntity )
                        break;
                    *CollisionEntity = pPools->GetObject((CObjectSAInterface *)targetEntity);
                    break;
                case ENTITY_TYPE_VEHICLE:
                    *CollisionEntity = pPools->GetVehicle((CVehicleSAInterface *)targetEntity);
                    break;

                }*/
            }
        }
    }
    if ( colCollision ) *colCollision = pColPointSA;
    else pColPointSA->Destroy ();

    return bReturn;
}


void CWorldSA::IgnoreEntity(CEntity * pEntity)
{
    DEBUG_TRACE("VOID CWorldSA::IgnoreEntity(CEntity * entity)");

    CEntitySA* pEntitySA = dynamic_cast < CEntitySA* > ( pEntity );

    if ( pEntitySA )
        MemPutFast < DWORD > ( VAR_IgnoredEntity, (DWORD) pEntitySA->GetInterface () );
    else
        MemPutFast < DWORD > ( VAR_IgnoredEntity, 0 );
}

// technically this is in CTheZones
BYTE CWorldSA::GetLevelFromPosition(CVector * vecPosition)
{
    DEBUG_TRACE("BYTE CWorldSA::GetLevelFromPosition(CVector * vecPosition)");
    DWORD dwFunc = FUNC_GetLevelFromPosition;
    BYTE bReturn = 0;
    _asm
    {
        push    vecPosition
        call    dwFunc
        mov     bReturn, al
        pop     eax
    }
    return bReturn;
}

float CWorldSA::FindGroundZForPosition(float fX, float fY)
{
    DEBUG_TRACE("FLOAT CWorldSA::FindGroundZForPosition(FLOAT fX, FLOAT fY)");
    DWORD dwFunc = FUNC_FindGroundZFor3DCoord;
    FLOAT fReturn = 0;
    _asm
    {
        push    fY
        push    fX
        call    dwFunc
        fstp    fReturn
        add     esp, 8
    }
    return fReturn;
}

float CWorldSA::FindGroundZFor3DPosition(CVector * vecPosition)
{
    DEBUG_TRACE("FLOAT CWorldSA::FindGroundZFor3DPosition(CVector * vecPosition)");
    DWORD dwFunc = FUNC_FindGroundZFor3DCoord;
    FLOAT fReturn = 0;
    FLOAT fX = vecPosition->fX;
    FLOAT fY = vecPosition->fY;
    FLOAT fZ = vecPosition->fZ;
    _asm
    {
        push    0
        push    0
        push    fZ
        push    fY
        push    fX
        call    dwFunc
        fstp    fReturn
        add     esp, 0x14
    }
    return fReturn;
}

void CWorldSA::LoadMapAroundPoint(CVector * vecPosition, FLOAT fRadius)
{
    DEBUG_TRACE("VOID CWorldSA::LoadMapAroundPoint(CVector * vecPosition, FLOAT fRadius)");
    DWORD dwFunc = FUNC_CTimer_Stop;
    _asm
    {
        call    dwFunc
    }

    dwFunc = FUNC_CRenderer_RequestObjectsInDirection;
    _asm
    {
        push    32
        push    fRadius
        push    vecPosition
        call    dwFunc
        add     esp, 12
    }


    dwFunc = FUNC_CStreaming_LoadScene;
    _asm
    {
        push    vecPosition
        call    dwFunc
        add     esp, 4
    }

    dwFunc = FUNC_CTimer_Update;
    _asm
    {
        call    dwFunc
    }

}


bool CWorldSA::IsLineOfSightClear ( const CVector * vecStart, const CVector * vecEnd, const SLineOfSightFlags flags )
{
    DWORD dwFunc = FUNC_IsLineOfSightClear;
    bool bReturn = false;
    // bool bCheckBuildings = true, bool bCheckVehicles = true, bool bCheckPeds = true, 
    // bool bCheckObjects = true, bool bCheckDummies = true, bool bSeeThroughStuff = false, 
    // bool bIgnoreSomeObjectsForCamera = false

    _asm
    {
        push    flags.bIgnoreSomeObjectsForCamera
        push    flags.bSeeThroughStuff
        push    flags.bCheckDummies
        push    flags.bCheckObjects
        push    flags.bCheckPeds
        push    flags.bCheckVehicles
        push    flags.bCheckBuildings
        push    vecEnd
        push    vecStart    
        call    dwFunc
        mov     bReturn, al
        add     esp, 0x24
    }
    return bReturn;
}

bool CWorldSA::HasCollisionBeenLoaded ( CVector * vecPosition )
{
    DWORD dwFunc = FUNC_HasCollisionBeenLoaded;
    bool bRet = false;
    _asm
    {
        push    0
        push    vecPosition
        call    dwFunc
        mov     bRet, al
        add     esp, 8
    } 
    return bRet;
}

DWORD CWorldSA::GetCurrentArea ( void )
{
    return *(DWORD *)VAR_currArea;
}

void CWorldSA::SetCurrentArea ( DWORD dwArea )
{
    MemPutFast < DWORD > ( VAR_currArea, dwArea );

    DWORD dwFunc = FUNC_RemoveBuildingsNotInArea;
    _asm
    {
        push    dwArea
        call    dwFunc
        add     esp, 4
    }
}

void CWorldSA::SetJetpackMaxHeight ( float fHeight )
{
    MemPut < float > ( VAR_fJetpackMaxHeight, fHeight );
}

float CWorldSA::GetJetpackMaxHeight ( void )
{
    return *(float *)(VAR_fJetpackMaxHeight);
}

void CWorldSA::SetAircraftMaxHeight ( float fHeight )
{
    MemPut < float > ( VAR_fAircraftMaxHeight, fHeight );
}

float CWorldSA::GetAircraftMaxHeight ( void )
{
    return *(float *)( VAR_fAircraftMaxHeight );
}

void CWorldSA::RemoveBuilding ( unsigned short usModelToRemove, float fRange, float fX, float fY, float fZ)
{    
    // New building Removal

    SBuildingRemoval* pRemoval = new SBuildingRemoval();
    pRemoval->usPreviousModel = usModelToRemove;
    pRemoval->vecPos.fX = fX;
    pRemoval->vecPos.fY = fY;
    pRemoval->vecPos.fZ = fZ;
    pRemoval->fRadius = fRange;
    // Push it to the back of the removal list
    m_pBuildings->insert ( std::pair<unsigned short, SBuildingRemoval*> ( usModelToRemove, pRemoval ) );

    sDataBuildingRemoval * pFind = NULL;
    std::pair < std::multimap < unsigned short, sDataBuildingRemoval*>::iterator, std::multimap < unsigned short, sDataBuildingRemoval* >::iterator> iterators = m_pDataBuildings->equal_range ( usModelToRemove );
    std::multimap < unsigned short, sDataBuildingRemoval* > ::const_iterator iter = iterators.first;
    for ( ; iter != iterators.second; ++iter )
    {
        pFind = (*iter).second;
        if ( pFind )
        {
            if ( pFind->iCount <= 0 && pFind->m_pInterface )
            {
                // Grab distances across each axis
                float fDistanceX = fX - pFind->m_pInterface->Placeable.m_transform.m_translate.fX;
                float fDistanceY = fY - pFind->m_pInterface->Placeable.m_transform.m_translate.fY;
                float fDistanceZ = fZ - pFind->m_pInterface->Placeable.m_transform.m_translate.fZ;

                // Square root 'em
                float fDistance = sqrt ( fDistanceX * fDistanceX + fDistanceY * fDistanceY + fDistanceZ * fDistanceZ );
                if ( fDistance <= fRange && pFind->m_pInterface->bRemoveFromWorld != 1 )
                {
                    CEntitySAInterface * pInterface = pFind->m_pInterface;
                    //while ( pInterface && pInterface != NULL )
                    {
                        // Add the Data Building to the list
                        pRemoval->AddDataBuilding ( pInterface );
                        // Remove the model from the world
                        Remove ( pInterface );
                        // Get next LOD ( LOD's can have LOD's so we keep checking pInterface )
                        pInterface = pInterface->m_pLod;
                    }
                    pFind->iCount = 1;
                }
            }
        }
    }

    // if this model isn't marked as deleted mark it
    if ( IsModelRemoved ( usModelToRemove ) == false )
        (*m_pRemovedObjects) [ usModelToRemove ] = 1;
    else
        m_pRemovedObjects->find ( usModelToRemove )->second++;

    // Grab the current camera position
    CMatrix Matrix;
    pGame->GetCamera ( )->GetMatrix ( &Matrix );
    // Grab distances across each axis
    float fDistanceX = Matrix.vPos.fX - pRemoval->vecPos.fX;
    float fDistanceY = Matrix.vPos.fY - pRemoval->vecPos.fY;
    float fDistanceZ = Matrix.vPos.fZ - pRemoval->vecPos.fZ;

    // Square root 'em
    float fDistance = sqrt ( fDistanceX * fDistanceX + fDistanceY * fDistanceY + fDistanceZ * fDistanceZ );
    //  if distance is <= 300 do something
    if ( fDistance <= 300 )
    {
        // Put Flush Code here or something
    }
}

bool CWorldSA::RestoreBuilding ( unsigned short usModelToRestore, float fRange, float fX, float fY, float fZ )
{        
    bool bSuccess = false;
    // Init some variables
    SBuildingRemoval * pFind = NULL;
    std::pair < std::multimap < unsigned short, SBuildingRemoval* >::iterator, std::multimap < unsigned short, SBuildingRemoval* >::iterator> iterators = m_pBuildings->equal_range ( usModelToRestore );
    std::multimap < unsigned short, SBuildingRemoval* > ::const_iterator iter = iterators.first;
    // Loop through the buildings list
    for ( ; iter != iterators.second;  )
    {
        pFind = (*iter).second;
        // if pFind is valid and the model is the same
        if ( pFind )
        {
            // Grab distances across each axis
            float fDistanceX = fX - pFind->vecPos.fX;
            float fDistanceY = fY - pFind->vecPos.fY;
            float fDistanceZ = fZ - pFind->vecPos.fZ;

            // Square root 'em
            float fDistance = sqrt ( fDistanceX * fDistanceX + fDistanceY * fDistanceY + fDistanceZ * fDistanceZ );
            if ( fDistance <= pFind->fRadius )
            {
                // Init some variables
                CEntitySAInterface * pEntity = NULL;
                std::list < CEntitySAInterface* > ::const_iterator entityIter = pFind->pLODList->begin ( );
                if ( pFind->pLODList->empty ( ) == false ) 
                {
                    // Loop through the LOD list
                    for ( ; entityIter != pFind->pLODList->end (); )
                    {
                        // Grab the pEntity
                        pEntity = (*entityIter);
                        // if it's valid re-add it to the world.
                        if ( pEntity && pEntity != NULL && (DWORD)pEntity->vtbl == VTBL_CBUILDING )
                        {
                            Add ( pEntity );
                            pFind->pLODList->erase ( entityIter++ );
                        }
                        else
                            ++entityIter;
                    }
                }
                entityIter = pFind->pDataRemoveList->begin ( );
                if ( pFind->pDataRemoveList->empty ( ) == false )
                {
                    // Loop through the Data list
                    for ( ; entityIter != pFind->pDataRemoveList->end (); )
                    {
                        // Grab the pEntity
                        pEntity = (*entityIter);
                        // if it's valid re-add it to the world.
                        if ( pEntity && pEntity != NULL && (DWORD)pEntity->vtbl == VTBL_CBUILDING )
                        {
                            Add ( pEntity );
                            pFind->pDataRemoveList->erase ( entityIter++ );
                        }
                        else
                            ++entityIter;
                    }
                }
                // Remove the building from the list
                m_pBuildings->erase ( iter++ );
                // Success! don't return incase there are any others to delete
                bSuccess = true;
            }
            else
                iter++;
        }
        else
            iter++;
    }
    sDataBuildingRemoval * pFound = NULL;
    std::pair < std::multimap < unsigned short, sDataBuildingRemoval*>::iterator, std::multimap < unsigned short, sDataBuildingRemoval* >::iterator> dataBuildingIterators = m_pDataBuildings->equal_range ( usModelToRestore );
    std::multimap < unsigned short, sDataBuildingRemoval* > ::const_iterator iterator = dataBuildingIterators.first;
    for ( ; iterator != dataBuildingIterators.second; ++iterator )
    {
        pFound = (*iterator).second;
        if ( pFound )
        {
            // Grab distances across each axis
            float fDistanceX = fX - pFound->m_pInterface->Placeable.m_transform.m_translate.fX;
            float fDistanceY = fY - pFound->m_pInterface->Placeable.m_transform.m_translate.fY;
            float fDistanceZ = fZ - pFound->m_pInterface->Placeable.m_transform.m_translate.fZ;

            // Square root 'em
            float fDistance = sqrt ( fDistanceX * fDistanceX + fDistanceY * fDistanceY + fDistanceZ * fDistanceZ );
            if ( fDistance <= fRange )
            {
                // Fix the removed count.
                pFound->iCount--;
                if ( pFound->iCount < 0 )
                    pFound->iCount = 0;
            }
        }

    }
    return bSuccess;
}

// Check Distance to see if the model being requested is in the radius
bool CWorldSA::IsRemovedModelInRadius ( SIPLInst* pInst )
{
    // Init some variables
    SBuildingRemoval * pFind = NULL;
    std::pair < std::multimap < unsigned short, SBuildingRemoval* >::iterator, std::multimap < unsigned short, SBuildingRemoval* >::iterator> iterators = m_pBuildings->equal_range ( pInst->m_nModelIndex );
    std::multimap < unsigned short, SBuildingRemoval* > ::const_iterator iter = iterators.first;
    // Loop through the buildings list
    for ( ; iter != iterators.second; ++iter )
    {
        pFind = (*iter).second;
        // if pFind is valid and the model is the same
        if ( pFind )
        {
            // Grab the distance
            float fDistanceX = pFind->vecPos.fX - pInst->m_pPosition.fX;
            float fDistanceY = pFind->vecPos.fY - pInst->m_pPosition.fY;
            float fDistanceZ = pFind->vecPos.fZ - pInst->m_pPosition.fZ;

            float fDistance = sqrt ( fDistanceX * fDistanceX + fDistanceY * fDistanceY + fDistanceZ * fDistanceZ );
            // is it in the removal spheres radius if so return else keep looking
            if ( fDistance <=  pFind->fRadius )
            {
                return true;
            }
        }
    }
    return false;
}

// Check if a given model is replaced
bool CWorldSA::IsModelRemoved ( unsigned short usModelID )
{
    std::map<unsigned short,unsigned short>::iterator iter = m_pRemovedObjects->find ( usModelID );
    if ( iter != m_pRemovedObjects->end ( ) )
    {
        return iter->second > 0;
    }
    else
        return false;
}

// Resets deleted list
void CWorldSA::ClearRemovedBuildingLists ( )
{
    // Ensure no memory leaks by deleting items.
    SBuildingRemoval * pFind = NULL;
    std::multimap < unsigned short, SBuildingRemoval* > ::const_iterator iter = m_pBuildings->begin ( );
    for ( ; iter != m_pBuildings->end ( ); )
    {
        pFind = (*iter).second;
        if ( pFind )
        {
            // Init some variables
            CEntitySAInterface * pEntity = NULL;
            std::list < CEntitySAInterface* > ::const_iterator entityIter = pFind->pLODList->begin ( );
            if ( pFind->pLODList->empty ( ) == false )
            {
                // Loop through the LOD list
                for ( ; entityIter != pFind->pLODList->end (); ++entityIter )
                {
                    // Grab the pEntity
                    pEntity = (*entityIter);
                    // if it's valid re-add it to the world.
                    if ( pEntity && (DWORD)pEntity->vtbl == VTBL_CBUILDING )
                    {
                        Add ( pEntity );
                    }
                }
            }
            entityIter = pFind->pDataRemoveList->begin ( );
            if ( pFind->pDataRemoveList->empty ( ) == false )
            {
                // Loop through the Data list
                for ( ; entityIter != pFind->pDataRemoveList->end (); ++entityIter )
                {
                    // Grab the pEntity
                    pEntity = (*entityIter);
                    // if it's valid re-add it to the world.
                    if ( pEntity && (DWORD)pEntity->vtbl == VTBL_CBUILDING )
                    {
                        Add ( pEntity );
                    }
                }
            }
            m_pBuildings->erase ( iter++ );
        }
        else
            iter++;
    }
    sDataBuildingRemoval * pFound = NULL;   
    std::multimap < unsigned short, sDataBuildingRemoval* > ::const_iterator iterator = m_pDataBuildings->begin ( );
    for ( ; iterator != m_pDataBuildings->end ( ); ++iterator )
    {
        pFound = (*iterator).second;
        if ( pFound )
        {
            pFound->iCount = 0;
        }

    }
    // Delete old building lists
    delete m_pBuildings;
    delete m_pRemovedObjects;
    // Create new
    m_pBuildings = new std::multimap< unsigned short, SBuildingRemoval* >;
    m_pRemovedObjects = new std::map < unsigned short, unsigned short >;
}


// Resets deleted list
SBuildingRemoval* CWorldSA::GetBuildingRemoval ( CEntitySAInterface * pInterface )
{
    // Init some variables
    SBuildingRemoval * pFind = NULL;
    std::pair < std::multimap < unsigned short, SBuildingRemoval* >::iterator, std::multimap < unsigned short, SBuildingRemoval* >::iterator> iterators = m_pBuildings->equal_range ( pInterface->m_nModelIndex );
    std::multimap < unsigned short, SBuildingRemoval* > ::const_iterator iter = iterators.first;
    // Loop through the buildings list
    for ( ; iter != iterators.second; ++iter )
    {
        pFind = (*iter).second;
        // if pFind is valid and the model is the same
        if ( pFind )
        {
            // Grab the distance
            float fDistanceX = pFind->vecPos.fX - pInterface->Placeable.m_transform.m_translate.fX;
            float fDistanceY = pFind->vecPos.fY - pInterface->Placeable.m_transform.m_translate.fY;
            float fDistanceZ = pFind->vecPos.fZ - pInterface->Placeable.m_transform.m_translate.fZ;

            float fDistance = sqrt ( fDistanceX * fDistanceX + fDistanceY * fDistanceY + fDistanceZ * fDistanceZ );
            // is it in the removal spheres radius if so return else keep looking
            if ( fDistance <=  pFind->fRadius )
            {
                return pFind;
            }
        }
    }
    return NULL;
}

void CWorldSA::AddDataBuilding ( CEntitySAInterface * pInterface )
{
    sDataBuildingRemoval * pBuildingRemoval = new sDataBuildingRemoval ( pInterface, true );
    m_pDataBuildings->insert ( std::pair<unsigned short, sDataBuildingRemoval*> ( (unsigned short)pInterface->m_nModelIndex, pBuildingRemoval ) );
}

void CWorldSA::RemoveWorldBuilding ( CEntitySAInterface * pInterface )
{
    SBuildingRemoval * pFind = NULL;
    std::pair < std::multimap < unsigned short, SBuildingRemoval* >::iterator, std::multimap < unsigned short, SBuildingRemoval* >::iterator> iterators = m_pBuildings->equal_range ( pInterface->m_nModelIndex );
    std::multimap < unsigned short, SBuildingRemoval* > ::const_iterator iter = iterators.first;

    // Loop through the buildings list
    for ( ; iter != iterators.second; ++iter )
    {
        pFind = (*iter).second;
        // if pFind is valid and the model is the same
        if ( pFind )
        {
            {
                CEntitySAInterface * pEntity = NULL;
                if ( pFind->pLODList->empty ( ) == false )
                {
                    std::list < CEntitySAInterface* > ::const_iterator entityIter = pFind->pLODList->begin ( );
                    for ( ; entityIter != pFind->pLODList->end (); )
                    {
                        if ( pEntity == pInterface )
                        {
                            pFind->pLODList->erase ( entityIter++ );
                        }
                        else
                            entityIter++;
                    }
                }
                if ( pFind->pDataRemoveList->empty ( ) == false )
                {
                    std::list < CEntitySAInterface* > ::const_iterator entityIter = pFind->pDataRemoveList->begin ( );
                    // Loop through the Data list
                    for ( ; entityIter != pFind->pDataRemoveList->end (); )
                    {
                        // Grab the pEntity
                        pEntity = (*entityIter);
                        // if it's valid re-add it to the world.
                        if ( pEntity && (DWORD)pEntity->vtbl == VTBL_CBUILDING )
                        {
                            if ( pEntity == pInterface )
                            {
                                pFind->pDataRemoveList->erase ( entityIter++ );
                            }
                            else
                                entityIter++;
                        }
                    }
                }
            }
        }
    }
}