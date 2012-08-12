/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CWeaponSA.cpp
*  PURPOSE:     Weapon class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CWeaponSA::CWeaponSA( CWeaponSAInterface * weaponInterface, CPed * ped, eWeaponSlot weaponSlot )
{
    DEBUG_TRACE("CWeaponSA::CWeaponSA( CWeaponSAInterface * weaponInterface, CPed * ped, DWORD dwSlot )");
    this->owner = ped;
    this->m_weaponSlot = weaponSlot;
    internalInterface = weaponInterface;    
}

eWeaponType CWeaponSA::GetType(  )
{
    DEBUG_TRACE("eWeaponType CWeaponSA::GetType(  )");
    return this->internalInterface->m_eWeaponType;
};

VOID CWeaponSA::SetType( eWeaponType type )
{
    DEBUG_TRACE("VOID CWeaponSA::SetType( eWeaponType type )");
    this->internalInterface->m_eWeaponType = type;
}

eWeaponState CWeaponSA::GetState(  )
{
    DEBUG_TRACE("eWeaponState CWeaponSA::GetState(  )");
    return this->internalInterface->m_eState;
}

void CWeaponSA::SetState ( eWeaponState state )
{
    DEBUG_TRACE("void CWeaponSA::SetState ( eWeaponState state )");
    this->internalInterface->m_eState = state;
}

DWORD CWeaponSA::GetAmmoInClip(  )
{
    DEBUG_TRACE("DWORD CWeaponSA::GetAmmoInClip(  )");
    return this->internalInterface->m_nAmmoInClip;
}

VOID CWeaponSA::SetAmmoInClip( DWORD dwAmmoInClip )
{
    DEBUG_TRACE("VOID CWeaponSA::SetAmmoInClip( DWORD dwAmmoInClip )");
    this->internalInterface->m_nAmmoInClip = dwAmmoInClip;
}

DWORD CWeaponSA::GetAmmoTotal(  )
{
    DEBUG_TRACE("DWORD CWeaponSA::GetAmmoTotal(  )");
    return this->internalInterface->m_nAmmoTotal;
}

VOID CWeaponSA::SetAmmoTotal( DWORD dwAmmoTotal )
{
    DEBUG_TRACE("VOID CWeaponSA::SetAmmoTotal( DWORD dwAmmoTotal )");
    this->internalInterface->m_nAmmoTotal = dwAmmoTotal;
}

CPed * CWeaponSA::GetPed()
{
    DEBUG_TRACE("CPed * CWeaponSA::GetPed()");
    return (CPed *)owner;
}

eWeaponSlot CWeaponSA::GetSlot()
{
    DEBUG_TRACE("eWeaponSlot CWeaponSA::GetSlot()");
    return m_weaponSlot;
}

VOID CWeaponSA::SetAsCurrentWeapon()
{
    DEBUG_TRACE("VOID CWeaponSA::SetAsCurrentWeapon()");
    owner->SetCurrentWeaponSlot( m_weaponSlot );
}

void CWeaponSA::Remove ()
{
    DEBUG_TRACE("void CWeaponSA::Remove ()");
    DWORD dwFunc = FUNC_Shutdown;
    DWORD dwThis = (DWORD)this->internalInterface;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    // If the removed weapon was the currently active weapon, switch to empty-handed
    if ( owner->GetCurrentWeaponSlot () == m_weaponSlot )
    {
        CWeaponInfo* pInfo = pGame->GetWeaponInfo ( this->internalInterface->m_eWeaponType );
        if ( pInfo )
        {
            int iModel = pInfo->GetModel();
            owner->RemoveWeaponModel ( iModel );
        }
        owner->SetCurrentWeaponSlot ( WEAPONSLOT_TYPE_UNARMED );
    }
}


bool CWeaponSA::FireBullet ( CEntity* pFiringEntity, const CVector& vecOrigin, const CVector& vecTarget )
{
    if ( !pFiringEntity )
        return false;

    switch ( GetType () )
    {
        case WEAPONTYPE_PISTOL:
        case WEAPONTYPE_PISTOL_SILENCED:
        case WEAPONTYPE_DESERT_EAGLE:
        case WEAPONTYPE_SHOTGUN: 
        case WEAPONTYPE_SAWNOFF_SHOTGUN:
        case WEAPONTYPE_SPAS12_SHOTGUN:
        case WEAPONTYPE_MICRO_UZI:
        case WEAPONTYPE_MP5:
        case WEAPONTYPE_AK47:
        case WEAPONTYPE_M4:
        case WEAPONTYPE_TEC9:
        case WEAPONTYPE_COUNTRYRIFLE:
        case WEAPONTYPE_SNIPERRIFLE:
        case WEAPONTYPE_MINIGUN:
        {
            // Don't hit shooter
            pGame->GetWorld ()->IgnoreEntity ( pFiringEntity );

            // Do pre shot lag compensation
            CPlayerPed* pFiringPlayerPed = dynamic_cast < CPlayerPed* > ( pFiringEntity );
            if ( pGame->m_pPreWeaponFireHandler && pFiringPlayerPed )
                pGame->m_pPreWeaponFireHandler ( pFiringPlayerPed, false );

            // Get the gun muzzle position
            float fSkill = 999.f;
            CWeaponStat* pCurrentWeaponInfo = pGame->GetWeaponStatManager ( )->GetWeaponStatsFromSkillLevel ( GetType (), fSkill );
            CVector vecGunMuzzle = *pCurrentWeaponInfo->GetFireOffset ();    
            if ( pFiringPlayerPed )
                pFiringPlayerPed->GetTransformedBonePosition ( BONE_RIGHTWRIST, &vecGunMuzzle );

            // Bullet trace
            FireInstantHit ( pFiringEntity, &vecOrigin, &vecGunMuzzle, NULL, &vecTarget, NULL, false, true );

            // Do post shot lag compensation reset & script events
            if ( pGame->m_pPostWeaponFireHandler && pFiringPlayerPed )
                pGame->m_pPostWeaponFireHandler ();

            pGame->GetWorld ()->IgnoreEntity ( NULL );

            return true;
        }

        default:
            break;

    }
    return false;
}


bool CWeaponSA::FireInstantHit ( CEntity * pFiringEntity, const CVector* pvecOrigin, const CVector* pvecMuzzle, CEntity* pTargetEntity, const CVector* pvecTarget, const CVector* pvec, bool bFlag1, bool bFlag2 )
{
    bool bReturn;
    DWORD dwEntityInterface = 0;
    if ( pFiringEntity ) dwEntityInterface = ( DWORD ) pFiringEntity->GetInterface ();
    DWORD dwTargetInterface = 0;
    if ( pTargetEntity ) dwTargetInterface = ( DWORD ) pTargetEntity->GetInterface ();
    DWORD dwThis = ( DWORD ) internalInterface;
    DWORD dwFunc = 0x073FB10;
    _asm
    {
        mov     ecx, dwThis
        push    bFlag2
        push    bFlag1
        push    pvec
        push    pvecTarget
        push    dwTargetInterface
        push    pvecMuzzle
        push    pvecOrigin
        push    dwEntityInterface
        call    dwFunc
        mov     bReturn, al
    }
    return bReturn;
}
