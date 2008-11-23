/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CMovingObjectsManager.cpp
*  PURPOSE:     Manager for moving objects
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

void CMovingObjectsManager::DoPulse ( void )
{
    // Pulse all the objects we're moving
    bool bRemoved = false;
    CDeathmatchObject* pObject;
    list < CDeathmatchObject* > ::iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        pObject = *iter;
        pObject->UpdateMovement ();

        // Remove it if it has stopped moving
        if ( !pObject->IsMoving () )
        {
			// Remove from list
            iter = m_List.erase ( iter );
        }
    }
}
