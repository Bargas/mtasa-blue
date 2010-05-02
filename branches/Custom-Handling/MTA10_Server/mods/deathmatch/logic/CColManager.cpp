/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CColManager.cpp
*  PURPOSE:     Collision entity manager class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CColManager::CColManager ( void )
{
    m_bCanRemoveFromList = true;
    m_bIteratingList = false;
}


CColManager::~CColManager ( void )
{
    DeleteAll ();
    TakeOutTheTrash ();
}


void CColManager::DoHitDetection ( const CVector& vecLastPosition, const CVector& vecNowPosition, float fRadius, CElement* pElement, CColShape * pJustThis, bool bChildren )
{
    if ( bChildren )
    {
        list < CElement* > ::const_iterator iter = pElement->IterBegin ();
        for ( ; iter != pElement->IterEnd (); iter++ )
        {
            DoHitDetection ( (*iter)->GetLastPosition(), (*iter)->GetPosition(), 0.0f, *iter, pJustThis, true );
        }
        if ( IS_COLSHAPE ( pElement ) ||
             IS_FILE ( pElement ) ||
             IS_HANDLING ( pElement ) ||
             IS_RADAR_AREA ( pElement ) ||
             IS_CONSOLE ( pElement ) ||
             IS_TEAM ( pElement ) ||
             IS_BLIP ( pElement ) ||
             IS_DUMMY ( pElement ) )
        {
            return;
        }
    }

    if ( !pJustThis )
    {
        m_bIteratingList = true;
        // Call the hit detection event on all our shapes
        for ( unsigned int i = 0 ; i < m_List.size () ; i++ )
        {
            CColShape* pShape = m_List[i];

            // Not being deleted and enabled?
            if ( !pShape->IsBeingDeleted () && pShape->IsEnabled () )
            {
                // Collided?
                if ( pShape->DoHitDetection ( vecLastPosition, vecNowPosition, fRadius ) )
                {
                    // If they havn't collided yet
                    if ( !pElement->CollisionExists ( pShape ) )
                    {
                        // Add the collision and the collider
                        pShape->AddCollider ( pElement );
                        pElement->AddCollision ( pShape );

                        // Can we call the event?
                        if ( pShape->GetAutoCallEvent () )
                        {
                            // Call the event
                            CLuaArguments Arguments;
                            Arguments.PushElement ( pElement );
                            Arguments.PushBoolean ( ( pShape->GetDimension () == pElement->GetDimension () ) );
                            pShape->CallEvent ( "onColShapeHit", Arguments );

                            CLuaArguments Arguments2;
                            Arguments2.PushElement ( pShape );
                            Arguments2.PushBoolean ( ( pShape->GetDimension () == pElement->GetDimension () ) );
                            pElement->CallEvent ( "onElementColShapeHit", Arguments2 );
                        }

                        // Run whatever callback the collision item might have attached
                        pShape->CallHitCallback ( *pElement );
                    }
                }
                else
                {
                    // If they collided before
                    if ( pElement->CollisionExists ( pShape ) )
                    {
                        // Remove the collision and the collider
                        pShape->RemoveCollider ( pElement );
                        pElement->RemoveCollision ( pShape );

                        // Can we call the event?
                        if ( pShape->GetAutoCallEvent () )
                        {
                            // Call the event
                            CLuaArguments Arguments;
                            Arguments.PushElement ( pElement );
                            Arguments.PushBoolean ( ( pShape->GetDimension () == pElement->GetDimension () ) );
                            pShape->CallEvent ( "onColShapeLeave", Arguments );

                            CLuaArguments Arguments2;
                            Arguments2.PushElement ( pShape );
                            Arguments2.PushBoolean ( ( pShape->GetDimension () == pElement->GetDimension () ) );
                            pElement->CallEvent ( "onElementColShapeLeave", Arguments2 );
                        }

                        // Run whatever callback the collision item might have attached
                        pShape->CallLeaveCallback ( *pElement );
                    }
                }
            }
        }

        m_bIteratingList = false;
        TakeOutTheTrash ();
    }
    else
    {
        if ( pJustThis->DoHitDetection ( vecLastPosition, vecNowPosition, fRadius ) )
        {
            // If they havn't collided yet
            if ( !pElement->CollisionExists ( pJustThis ) )
            {
                // Add the collision and the collider
                pJustThis->AddCollider ( pElement );
                pElement->AddCollision ( pJustThis );

                // Can we call the event?
                if ( pJustThis->GetAutoCallEvent () )
                {
                    // Call the event
                    CLuaArguments Arguments;
                    Arguments.PushElement ( pElement );
                    Arguments.PushBoolean ( ( pJustThis->GetDimension () == pElement->GetDimension () ) );
                    pJustThis->CallEvent ( "onColShapeHit", Arguments );

                    CLuaArguments Arguments2;
                    Arguments2.PushElement ( pJustThis );
                    Arguments2.PushBoolean ( ( pJustThis->GetDimension () == pElement->GetDimension () ) );
                    pElement->CallEvent ( "onElementColShapeHit", Arguments2 );
                }

                // Run whatever callback the collision item might have attached
                pJustThis->CallHitCallback ( *pElement );
            }
        }
        else
        {
            // If they collided before
            if ( pElement->CollisionExists ( pJustThis ) )
            {
                // Remove the collision and the collider
                pJustThis->RemoveCollider ( pElement );
                pElement->RemoveCollision ( pJustThis );

                // Can we call the event?
                if ( pJustThis->GetAutoCallEvent () )
                {
                    // Call the event
                    CLuaArguments Arguments;
                    Arguments.PushElement ( pElement );
                    Arguments.PushBoolean ( ( pJustThis->GetDimension () == pElement->GetDimension () ) );
                    pJustThis->CallEvent ( "onColShapeLeave", Arguments );

                    CLuaArguments Arguments2;
                    Arguments2.PushElement ( pJustThis );
                    Arguments2.PushBoolean ( ( pJustThis->GetDimension () == pElement->GetDimension () ) );
                    pElement->CallEvent ( "onElementColShapeLeave", Arguments2 );
                }

                // Run whatever callback the collision item might have attached
                pJustThis->CallLeaveCallback ( *pElement );
            }
        }
    }
}


bool CColManager::Exists ( CColShape* pShape )
{
    // Return true if it exists
    vector < CColShape* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        if ( *iter == pShape )
        {
            return true;
        }
    }

    return false;
}


void CColManager::DeleteAll ( void )
{
    // Delete all of them
    m_bCanRemoveFromList = false;
    vector < CColShape* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        delete *iter;
    }

    // Clear the list
    m_bCanRemoveFromList = true;
    m_List.clear ();
}


void CColManager::RemoveFromList ( CColShape* pShape )
{
    if ( m_bCanRemoveFromList )
    {
        // Dont wanna remove it if we're going through the list
        if ( m_bIteratingList )
        {
            m_TrashCan.push_back ( pShape );
        }
        else
        {
            ListRemove ( m_List, pShape );
        }
    }
}


void CColManager::TakeOutTheTrash ( void )
{
    vector < CColShape* > ::const_iterator iter = m_TrashCan.begin ();
    for ( ; iter != m_TrashCan.end (); iter++ )
    {
        ListRemove ( m_List, *iter );
    }

    m_TrashCan.clear ();
}
