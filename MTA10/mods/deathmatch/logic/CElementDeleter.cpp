/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CElementDeleter.cpp
*  PURPOSE:     Manager for elements to be destroyed
*  DEVELOPERS:  Jax <>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using std::list;

CElementDeleter::CElementDeleter ()
{
    // Allow unrefernces
    m_bAllowUnreference = true;
}


void CElementDeleter::Delete ( class CClientEntity* pElement )
{
	// Make sure we don't try to delete it twice
	if ( pElement && !IsBeingDeleted ( pElement ) )
	{
		// Before we do anything, fire the on-destroy event
        CLuaArguments Arguments;
        pElement->CallEvent ( "onClientElementDestroy", Arguments, true );

		// Add it to our list
		if ( !pElement->IsBeingDeleted () )
		{
			m_List.push_back ( pElement );
		}

		// Flag it as being deleted and unlink it from the tree/managers
		pElement->SetBeingDeleted ( true );
		pElement->ClearChildren ();
		pElement->SetParent ( NULL );
		pElement->Unlink ();
	}
}


void CElementDeleter::DeleteRecursive ( class CClientEntity* pElement )
{
	// Gather a list over children (we can't use the list as it changes)
	list < CClientEntity* > Children;
	list < CClientEntity* > ::const_iterator iterCopy = pElement->IterBegin ();
	for ( ; iterCopy != pElement->IterEnd (); iterCopy++ )
	{
		Children.push_back ( *iterCopy );
	}

	// Call ourselves on each child of this to go as deep as possible and start deleting there
	list < CClientEntity* > ::const_iterator iter = Children.begin ();
	for ( ; iter != Children.end (); iter++ )
	{
		DeleteRecursive ( *iter );
	}

	// At this point we're sure that this element has no more children left.
    // Add it to our list over deleting objects
    if ( !pElement->IsBeingDeleted () )
    {
        m_List.push_back ( pElement );
    }

	// Mark us as being deleted, unlink from parent and unlink from manager classes eventually
	pElement->SetBeingDeleted ( true );
	pElement->SetParent ( NULL );
	pElement->Unlink ();
}


void CElementDeleter::DoDeleteAll ( void )
{
    // Make sure elements won't call us back and screw with our list (would crash)
    m_bAllowUnreference = false;

    // Delete all the elements
    list < CClientEntity* > ::iterator iter = m_List.begin ();
    while ( iter != m_List.end () )
    {
        CClientEntity* pEntity = *iter;

        // Only destroy it if it allows that yet
        if ( pEntity->CanBeDeleted () )
        {
            // Delete the entity and put the next element in the list in the iterator
            delete pEntity;
            iter = m_List.erase ( iter );
        }
        else
        {
            ++iter;
        }
    }

    // We can now allow unrefernecs again
    m_bAllowUnreference = true;
}


bool CElementDeleter::IsBeingDeleted ( CClientEntity* pElement )
{
    // Return true if the given element is in the list
    list < CClientEntity* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        if ( pElement == *iter )
        {
            return true;
        }
    }

    return false;
}


void CElementDeleter::Unreference ( class CClientEntity* pElement )
{
    // If we allow unreferencing, remove this element from the to delete list.
    if ( m_bAllowUnreference )
    {
        m_List.remove ( pElement );
    }
}


bool CElementDeleter::CanBeDestroyed ( void )
{
    // Delete list empty? We can be destroyed
    if ( m_List.empty () )
        return true;

    // Check if there are any elements that can't be destroyed ye
    list < CClientEntity* > ::iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        // Can this element be destroyed yet?
        if ( !(*iter)->CanBeDeleted () )
        {
            // We can't be destroyed yet
            return false;
        }
    }

    // We can be destroyed
    return true;
}
