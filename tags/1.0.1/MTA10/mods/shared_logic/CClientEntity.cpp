/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientEntity.cpp
*  PURPOSE:     Base entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Chris McArthur <>
*               Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*
*****************************************************************************/

#include "StdInc.h"

using std::list;

extern CClientGame* g_pClientGame;

#define snprintf _snprintf

int CClientEntity::iCount = 0;

CClientEntity::CClientEntity ( ElementID ID )
{
    #ifdef MTA_DEBUG
        ++iCount;
    #endif

    // Init
    m_pManager = NULL;
    m_pParent = NULL;
    m_usDimension = 0;
    m_bSystemEntity = false;
    m_ucSyncTimeContext = 0;
    m_ucInterior = 0;

    // Need to generate a clientside ID?
    if ( ID == INVALID_ELEMENT_ID )
    {
        ID = CElementIDs::PopClientID ();
    }

    // Set our index in the element array
    m_ID = ID;
    CElementIDs::SetElement ( ID, this );

    m_pCustomData = new CCustomData;
    m_pEventManager = new CMapEventManager;

    m_pAttachedToEntity = NULL;

    strncpy ( m_szTypeName, "unknown", MAX_TYPENAME_LENGTH );
    m_szTypeName [MAX_TYPENAME_LENGTH] = 0;
    m_uiTypeHash = HashString ( m_szTypeName );
    if ( IsFromRoot ( m_pParent ) )
        CClientEntity::AddEntityFromRoot ( m_uiTypeHash, this );

    m_szName [0] = 0;
    m_szName [MAX_ELEMENT_NAME_LENGTH] = 0;

    m_bBeingDeleted = false;

    m_pElementGroup = NULL;
    m_pModelInfo = NULL;
}


CClientEntity::~CClientEntity ( void )
{
    #ifdef MTA_DEBUG
        --iCount;
    #endif

    // Make sure we won't get deleted later by the element deleter if we've been requested so
    if ( m_bBeingDeleted )
    {
        g_pClientGame->GetElementDeleter ()->Unreference ( this );
    }

    // Remove from parent
    ClearChildren ();
    SetParent ( NULL );

    // Reset our index in the element array
    if ( m_ID != INVALID_ELEMENT_ID )
    {
        CElementIDs::SetElement ( m_ID, NULL );

        // Got a clientside ID? Push it back on our stack so we don't run out of client id's
        if ( IsLocalEntity () )
        {
            CElementIDs::PushClientID ( m_ID );
        }
    }

    // Remove our custom data
    if ( m_pCustomData )
    {
        delete m_pCustomData;
    }

    if ( m_pAttachedToEntity )
    {
        m_pAttachedToEntity->RemoveAttachedEntity ( this );
    }

    for ( list < CClientEntity* >::iterator iter = m_AttachedEntities.begin () ; iter != m_AttachedEntities.end () ; ++iter )
    {
        CClientEntity* pAttachedEntity = *iter;
        if ( pAttachedEntity )
        {
            pAttachedEntity->m_pAttachedToEntity = NULL;
        }
    }

    RemoveAllCollisions ( true );

    if ( m_pEventManager )
    {
        delete m_pEventManager;
        m_pEventManager = NULL;
    }

    if ( m_pElementGroup )
    {
        m_pElementGroup->Remove ( this );
    }

    if ( m_OriginSourceUsers.size () > 0 )
    {
        list < CClientPed* > ::iterator iterUsers = m_OriginSourceUsers.begin ();
        for ( ; iterUsers != m_OriginSourceUsers.end () ; iterUsers++ )
        {
            CClientPed* pModel = *iterUsers;
            if ( pModel->m_interp.pTargetOriginSource == this )
            {
                pModel->m_interp.pTargetOriginSource = NULL;
                pModel->m_interp.bHadOriginSource = true;
            }
        }
        m_OriginSourceUsers.clear ();
    }

    // Unlink our contacts
    list < CClientPed * > ::iterator iterContacts = m_Contacts.begin ();
    for ( ; iterContacts != m_Contacts.end () ; iterContacts++ )
    {
        CClientPed * pModel = *iterContacts;
        if ( pModel->GetCurrentContactEntity () == this )
        {
            pModel->SetCurrentContactEntity ( NULL );
        }
    }
    m_Contacts.clear ();

    // Unlink disabled-collisions
    while ( !m_DisabledCollisions.empty () )
    {
        CClientEntity * pEntity = m_DisabledCollisions.begin ()->first;
        SetCollidableWith ( pEntity, true );
    }

    // Ensure nothing has inadvertently set a parent
    assert ( m_pParent == NULL );
}


void CClientEntity::SetTypeName ( const char* szName )
{
    CClientEntity::RemoveEntityFromRoot ( m_uiTypeHash, this );
    strncpy ( m_szTypeName, szName, MAX_TYPENAME_LENGTH );
    m_uiTypeHash = HashString ( szName );
    if ( IsFromRoot ( m_pParent ) )
        CClientEntity::AddEntityFromRoot ( m_uiTypeHash, this );
}


bool CClientEntity::CanUpdateSync ( unsigned char ucRemote )
{
    // We can update this element's sync only if the sync time
    // matches or either of them are 0 (ignore).
    return ( m_ucSyncTimeContext == ucRemote ||
             ucRemote == 0 ||
             m_ucSyncTimeContext == 0 );
}


CClientEntity* CClientEntity::SetParent ( CClientEntity* pParent )
{
    // Get into/out-of FromRoot info
    bool bOldFromRoot = CClientEntity::IsFromRoot ( m_pParent );
    bool bNewFromRoot = CClientEntity::IsFromRoot ( pParent );

    // Remove us from previous parent's children list
    if ( m_pParent )
    {
        m_pParent->m_Children.remove ( this );
    }

    // Set the new parent
    m_pParent = pParent;

    // Add us to the new parent's children list
    if ( pParent )
    {
        pParent->m_Children.push_back ( this );
    }

    // Moving out of FromRoot?
    if ( bOldFromRoot && !bNewFromRoot )
        CClientEntity::RemoveEntityFromRoot ( m_uiTypeHash, this );

    // Moving into FromRoot?
    if ( !bOldFromRoot && bNewFromRoot )
            CClientEntity::AddEntityFromRoot ( m_uiTypeHash, this );

    // Return the new parent
    return pParent;
}


CClientEntity* CClientEntity::AddChild ( CClientEntity* pChild )
{
    assert ( pChild );

    // Set us as its new parent
    pChild->SetParent ( this );

    // Return the added child
    return pChild;
}


bool CClientEntity::IsMyChild ( CClientEntity* pEntity, bool bRecursive )
{
	// Since VERIFY_ELEMENT is calling us, the pEntity argument could be NULL
	if ( pEntity == NULL ) return false;

    // Is he us?
    if ( pEntity == this )
        return true;

    // Is he our child directly?
    list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        // Return true if this is our child. If not check if he's one of our children's children if we were asked to do a recursive search.
        if ( *iter == pEntity )
        {
            return true;
        }
        else if ( bRecursive && (*iter)->IsMyChild ( pEntity, true ) )
        {
            return true;
        }
    }

    // He's not under us
    return false;
}


void CClientEntity::ClearChildren ( void )
{
    // Sanity check
    assert ( m_pParent != this );

    // Process our children - Move up to our parent
	list < CClientEntity* > cloneList = m_Children;
    list < CClientEntity* > ::const_iterator iter = cloneList.begin ();
    for ( ; iter != cloneList.end () ; ++iter )
        (*iter)->SetParent ( m_pParent );

    // This list should now be empty
    assert ( m_Children.size () == 0 );
}


void CClientEntity::SetID ( ElementID ID )
{
    // Eventually reset what we have at the old ID
    if ( m_ID != INVALID_ELEMENT_ID )
    {
        CElementIDs::SetElement ( m_ID, NULL );

        // Are we a clientside element? Push the unique ID
        if ( IsLocalEntity () )
        {
            CElementIDs::PushClientID ( m_ID );
        }
    }

    // Set our new ID and index in the array
    m_ID = ID;
    if ( ID != INVALID_ELEMENT_ID )
    {
        CElementIDs::SetElement ( ID, this );
    }
}


CLuaArgument* CClientEntity::GetCustomData ( const char* szName, bool bInheritData )
{
    assert ( szName );

    // Grab it and return a pointer to the variable
    SCustomData* pData = m_pCustomData->Get ( szName );
    if ( pData )
    {
        return &pData->Variable;
    }

    // If none, try returning parent's custom data
    if ( bInheritData && m_pParent )
    {
        return m_pParent->GetCustomData ( szName, true );
    }

    // None available
    return NULL;
}


bool CClientEntity::GetCustomDataString ( const char* szName, SString& strOut, bool bInheritData )
{
    // Grab the custom data variable
    CLuaArgument* pData = GetCustomData ( szName, bInheritData );
    if ( pData )
    {
        // Write the content depending on what type it is
        int iType = pData->GetType ();
        if ( iType == LUA_TSTRING )
        {
            strOut = pData->GetString ();
        }
        else if ( iType == LUA_TNUMBER )
        {
            strOut.Format ( "%f", pData->GetNumber () );
        }
        else if ( iType == LUA_TBOOLEAN )
        {
            strOut.Format ( "%u", pData->GetBoolean () );
        }
        else if ( iType == LUA_TNIL )
        {
            strOut = "";
        }
        else
        {
            return false;
        }

        return true;
    }

    return false;
}


bool CClientEntity::GetCustomDataInt ( const char* szName, int& iOut, bool bInheritData )
{
    // Grab the custom data variable
    CLuaArgument* pData = GetCustomData ( szName, bInheritData );
    if ( pData )
    {
        // Write the content depending on what type it is
        int iType = pData->GetType ();
        if ( iType == LUA_TSTRING )
        {
            iOut = atoi ( pData->GetString () );
        }
        else if ( iType == LUA_TNUMBER )
        {
            iOut = static_cast < int > ( pData->GetNumber () );
        }
        else if ( iType == LUA_TBOOLEAN )
        {
            if ( pData->GetBoolean () )
            {
                iOut = 1;
            }
            else
            {
                iOut = 0;
            }
        }
        else
        {
            return false;
        }

        return true;
    }

    return false;
}


bool CClientEntity::GetCustomDataFloat ( const char* szName, float& fOut, bool bInheritData )
{
    // Grab the custom data variable
    CLuaArgument* pData = GetCustomData ( szName, bInheritData );
    if ( pData )
    {
        // Write the content depending on what type it is
        int iType = pData->GetType ();
        if ( iType == LUA_TSTRING )
        {
            fOut = static_cast < float > ( atof ( pData->GetString () ) );
        }
        else if ( iType == LUA_TNUMBER )
        {
            fOut = static_cast < float > ( pData->GetNumber () );
        }
        else
        {
            return false;
        }

        return true;
    }

    return false;
}


bool CClientEntity::GetCustomDataBool ( const char* szName, bool& bOut, bool bInheritData )
{
    // Grab the custom data variable
    CLuaArgument* pData = GetCustomData ( szName, bInheritData );
    if ( pData )
    {
        // Write the content depending on what type it is
        int iType = pData->GetType ();
        if ( iType == LUA_TSTRING )
        {
            const char* szString = pData->GetString ();
            if ( strcmp ( szString, "true" ) == 0 || strcmp ( szString, "1" ) == 0 )
            {
                bOut = true;
            }
            else if ( strcmp ( szString, "false" ) == 0 || strcmp ( szString, "0" ) == 0 )
            {
                bOut = false;
            }
            else
            {
                return false;
            }
        }
        else if ( iType == LUA_TNUMBER )
        {
            int iNumber = static_cast < int > ( pData->GetNumber () );
            if ( iNumber == 1 )
            {
                bOut = true;
            }
            else if ( iNumber == 0 )
            {
                bOut = false;
            }
            else
            {
                return false;
            }
        }
        else if ( iType == LUA_TBOOLEAN )
        {
            bOut = pData->GetBoolean ();
        }
        else
        {
            return false;
        }

        return true;
    }

    return false;
}

void CClientEntity::SetCustomData ( const char* szName, const CLuaArgument& Variable, CLuaMain* pLuaMain )
{
    assert ( szName );

    // Grab the old variable
    CLuaArgument oldVariable;
    SCustomData * pData = m_pCustomData->Get ( szName );
    if ( pData )
    {
        oldVariable = pData->Variable;
    }

    // Set the new data
    m_pCustomData->Set ( szName, Variable, pLuaMain );

    // Trigger the onClientElementDataChange event on us
    CLuaArguments Arguments;
    Arguments.PushString ( szName );
    Arguments.PushArgument ( oldVariable  );
    CallEvent ( "onClientElementDataChange", Arguments, true );
}


bool CClientEntity::DeleteCustomData ( const char* szName, bool bRecursive )
{
    // Delete the custom data
    bool bReturn = m_pCustomData->Delete ( szName );

    // If recursive, delete our children's data
    if ( bRecursive )
    {
        list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
        for ( ; iter != m_Children.end (); iter++ )
        {
            // Delete it. If we deleted any, remember that we've done that so we can return true.
            if ( (*iter)->DeleteCustomData ( szName, true ) )
                bReturn = true;
        }
    }

    // Return whether we deleted some
    return bReturn;
}


void CClientEntity::DeleteAllCustomData ( CLuaMain* pLuaMain, bool bRecursive )
{
    // Delete our data
    m_pCustomData->DeleteAll ( pLuaMain );

    // If recursive, delete our children's data
    if ( bRecursive )
    {
        list < CClientEntity* > ::iterator iter = m_Children.begin ();
        for ( ; iter != m_Children.end (); iter++ )
        {
            (*iter)->DeleteAllCustomData ( pLuaMain, true );
        }
    }
}

bool CClientEntity::GetMatrix ( CMatrix& matrix ) const
{
    const CEntity* pEntity = GetGameEntity ();
    if ( pEntity )
    {
        if ( pEntity->GetMatrix ( &matrix ) ) return true;
    }

    return false;
}


bool CClientEntity::SetMatrix ( const CMatrix& matrix )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        pEntity->SetMatrix ( const_cast < CMatrix * > ( &matrix ) );
        return true;
    }

    return false;
}


void CClientEntity::GetPositionRelative ( CClientEntity * pOrigin, CVector& vecPosition ) const
{
    CVector vecOrigin;
    pOrigin->GetPosition ( vecOrigin );
    GetPosition ( vecPosition );
    vecPosition -= vecOrigin;
}

void CClientEntity::SetPositionRelative ( CClientEntity * pOrigin, const CVector& vecPosition )
{
    CVector vecOrigin;
    pOrigin->GetPosition ( vecOrigin );
    SetPosition ( vecOrigin + vecPosition );
}

bool CClientEntity::IsOutOfBounds ( void )
{
    CVector vecPosition;
    GetPosition ( vecPosition );

    return ( vecPosition.fX < -3000.0f || vecPosition.fX > 3000.0f ||
             vecPosition.fY < -3000.0f || vecPosition.fY > 3000.0f ||
             vecPosition.fZ < -3000.0f || vecPosition.fZ > 3000.0f );
}
    

void CClientEntity::AttachTo ( CClientEntity* pEntity )
{
	if ( m_pAttachedToEntity )
		m_pAttachedToEntity->RemoveAttachedEntity ( this );

	m_pAttachedToEntity = pEntity;

	if ( m_pAttachedToEntity )
		m_pAttachedToEntity->AddAttachedEntity ( this );

    InternalAttachTo ( pEntity );
}


void CClientEntity::InternalAttachTo ( CClientEntity* pEntity )
{
    CPhysical * pThis = dynamic_cast < CPhysical * > ( GetGameEntity () );
    if ( pThis )
    {
        if ( pEntity )
        {
            switch ( pEntity->GetType () )
            {
                case CCLIENTVEHICLE:
                {
                    CVehicle * pGameVehicle = static_cast < CClientVehicle* > ( pEntity )->GetGameVehicle ();
                    if ( pGameVehicle )
                    {
                        pThis->AttachEntityToEntity ( *pGameVehicle, m_vecAttachedPosition, m_vecAttachedRotation );
                    }
                    break;
                }
                case CCLIENTPED:
                case CCLIENTPLAYER:
                {
                    CPlayerPed * pGamePed = static_cast < CClientPed* > ( pEntity )->GetGamePlayer ();
                    if ( pGamePed )
                    {
                        pThis->AttachEntityToEntity ( *pGamePed, m_vecAttachedPosition, m_vecAttachedRotation );                
                    }
                    break;
                }
                case CCLIENTOBJECT:
                {
                    CObject * pGameObject = static_cast < CClientObject* > ( pEntity )->GetGameObject ();
                    if ( pGameObject )
                    {
                        pThis->AttachEntityToEntity ( *pGameObject, m_vecAttachedPosition, m_vecAttachedRotation );
                    }
                    break;
                }
                case CCLIENTPICKUP:
                {
                    CObject * pGameObject = static_cast < CClientPickup * > ( pEntity )->GetGameObject ();
                    if ( pGameObject )
                    {
                        pThis->AttachEntityToEntity ( *pGameObject, m_vecAttachedPosition, m_vecAttachedRotation );
                    }
                    break;
                }
            }
        }
        else
        {
            pThis->DetachEntityFromEntity ( 0, 0, 0, 0 );
        }
    }
}


void CClientEntity::GetAttachedOffsets ( CVector & vecPosition, CVector & vecRotation )
{
    vecPosition = m_vecAttachedPosition;
    vecRotation = m_vecAttachedRotation;
}


void CClientEntity::SetAttachedOffsets ( CVector & vecPosition, CVector & vecRotation )
{
    CPhysical * pThis = dynamic_cast < CPhysical * > ( GetGameEntity () );
    if ( pThis )
    {
        pThis->SetAttachedOffsets ( vecPosition, vecRotation );
    }
    m_vecAttachedPosition = vecPosition;
    m_vecAttachedRotation = vecRotation;
}


bool CClientEntity::AddEvent ( CLuaMain* pLuaMain, const char* szName, int iLuaFunction, bool bPropagated )
{
    return m_pEventManager->Add ( pLuaMain, szName, iLuaFunction, bPropagated );
}


bool CClientEntity::CallEvent ( const char* szName, const CLuaArguments& Arguments, bool bCallOnChildren )
{
    CEvents* pEvents = g_pClientGame->GetEvents();

    // Make sure our event-manager knows we're about to call an event
    pEvents->PreEventPulse ();

    // Call the event on our parents/us first
    CallParentEvent ( szName, Arguments, this );

    if ( bCallOnChildren )
    {
        // Call it on all our children
        CallEventNoParent ( szName, Arguments, this );
    }

    // Tell the event manager that we're done calling the event
    pEvents->PostEventPulse ();

    // Return whether it got cancelled or not
    return ( !pEvents->WasEventCancelled () );
}


void CClientEntity::CallEventNoParent ( const char* szName, const CLuaArguments& Arguments, CClientEntity* pSource )
{
    // Call it on us if this isn't the same class it was raised on
    //TODO not sure why the null check is necessary (eAi)
    if ( pSource != this && m_pEventManager != NULL )
    {
        m_pEventManager->Call ( szName, Arguments, pSource, this );
    }

    // Call it on all our children
    if ( ! m_Children.empty () )
    {
        list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
        for ( ; iter != m_Children.end (); iter++ )
        {
            (*iter)->CallEventNoParent ( szName, Arguments, pSource );
        }
    }
}


void CClientEntity::CallParentEvent ( const char* szName, const CLuaArguments& Arguments, CClientEntity* pSource )
{
    // Call the event on us
    if ( m_pEventManager )
    {
        m_pEventManager->Call ( szName, Arguments, pSource, this );
    }

    // Call parent's handler
    if ( m_pParent )
    {
        m_pParent->CallParentEvent ( szName, Arguments, pSource );
    }
}


bool CClientEntity::DeleteEvent ( CLuaMain* pLuaMain, const char* szName, int iLuaFunction )
{
    return m_pEventManager->Delete ( pLuaMain, szName, iLuaFunction );
}


void CClientEntity::DeleteEvents ( CLuaMain* pLuaMain, bool bRecursive )
{
    // Delete it from our events
    m_pEventManager->Delete ( pLuaMain );

    // Delete it from all our children's events
    if ( bRecursive )
    {
        list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
        for ( ; iter != m_Children.end (); iter++ )
        {
            (*iter)->DeleteEvents ( pLuaMain, true );
        }
    }
}


void CClientEntity::DeleteAllEvents ( void )
{
    m_pEventManager->DeleteAll ();
}

void CClientEntity::CleanUpForVM ( CLuaMain* pLuaMain, bool bRecursive )
{
    // Delete all our events and custom datas attached to that VM
    DeleteEvents ( pLuaMain, false );
    //DeleteCustomData ( pLuaMain, false ); * Removed to keep custom data global

    // If recursive, do it on our children too
    if ( bRecursive )
    {
        list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
        for ( ; iter != m_Children.end (); iter++ )
        {
            (*iter)->CleanUpForVM ( pLuaMain, true );
        }
    }
}


CClientEntity* CClientEntity::FindChild ( const char* szName, unsigned int uiIndex, bool bRecursive )
{
    assert ( szName );

    // Is it our name?
    unsigned int uiCurrentIndex = 0;
    if ( strcmp ( szName, m_szName ) == 0 )
    {
        if ( uiIndex == 0 )
        {
            return this;
        }
        else
        {
            ++uiCurrentIndex;
        }
    }

    // Find it among our children
    return FindChildIndex ( szName, uiIndex, uiCurrentIndex, bRecursive );
}


CClientEntity* CClientEntity::FindChildIndex ( const char* szName, unsigned int uiIndex, unsigned int& uiCurrentIndex, bool bRecursive )
{
    assert ( szName );

    // Look among our children
    list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        CClientEntity* pChild = *iter;
        // Name matches?
        if ( strcmp ( pChild->GetName (), szName ) == 0 )
        {
            // Does the index match? If it doesn't, increment it and keep searching
            if ( uiIndex == uiCurrentIndex )
            {
                return pChild;
            }
            else
            {
                ++uiCurrentIndex;
            }
        }

        // Tell this child to search too if recursive
        if ( bRecursive )
        {
            CClientEntity* pEntity = pChild->FindChildIndex ( szName, uiIndex, uiCurrentIndex, true );
            if ( pEntity )
            {
                return pEntity;
            }
        }
    }

    // Doesn't exist within us
    return NULL;
}


CClientEntity* CClientEntity::FindChildByType ( const char* szType, unsigned int uiIndex, bool bRecursive )
{
    assert ( szType );

    // Is it our type?
    unsigned int uiCurrentIndex = 0;
    if ( strcmp ( szType, GetTypeName () ) == 0 )
    {
        if ( uiIndex == 0 )
        {
            return this;
        }
        else
        {
            ++uiCurrentIndex;
        }
    }

    // Find it among our children
    unsigned int uiNameHash = HashString ( szType );
    return FindChildByTypeIndex ( uiNameHash, uiIndex, uiCurrentIndex, bRecursive );
}


CClientEntity* CClientEntity::FindChildByTypeIndex ( unsigned int uiTypeHash, unsigned int uiIndex, unsigned int& uiCurrentIndex, bool bRecursive )
{
    // Look among our children
    list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        // Name matches?
        if ( (*iter)->GetTypeHash() == uiTypeHash )
        {
            // Does the index match? If it doesn't, increment it and keep searching
            if ( uiIndex == uiCurrentIndex )
            {
                return *iter;
            }
            else
            {
                ++uiCurrentIndex;
            }
        }

        // Tell this child to search too if recursive
        if ( bRecursive )
        {
            CClientEntity* pEntity = (*iter)->FindChildByTypeIndex ( uiTypeHash, uiIndex, uiCurrentIndex, true );
            if ( pEntity )
            {
                return pEntity;
            }
        }
    }

    // Doesn't exist within us
    return NULL;
}


void CClientEntity::FindAllChildrenByType ( const char* szType, CLuaMain* pLuaMain, bool bStreamedIn )
{
    assert ( szType );
    assert ( pLuaMain );

    // Add all children of the given type to the table
    unsigned int uiIndex = 0;
    unsigned int uiTypeHash = HashString ( szType );

    if ( this == g_pClientGame->GetRootEntity () )
    {
        GetEntitiesFromRoot ( uiTypeHash, pLuaMain, bStreamedIn );
    }
    else
    {
        FindAllChildrenByTypeIndex ( uiTypeHash, pLuaMain, uiIndex, bStreamedIn );
    }
}


void CClientEntity::FindAllChildrenByTypeIndex ( unsigned int uiTypeHash, CLuaMain* pLuaMain, unsigned int& uiIndex, bool bStreamedIn )
{
    assert ( pLuaMain );

    lua_State* luaVM = pLuaMain->GetVirtualMachine ();

    // Our type matches?
    if ( m_uiTypeHash == uiTypeHash )
    {
        // Only streamed in elements?
        if ( !bStreamedIn || !IsStreamingCompatibleClass() || 
             reinterpret_cast < CClientStreamElement* > ( this )->IsStreamedIn() )
        {
            // Add it to the table
            lua_pushnumber ( luaVM, ++uiIndex );
            lua_pushelement ( luaVM, this );
            lua_settable ( luaVM, -3 );
        }
    }

    // Call us on the children
    list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        (*iter)->FindAllChildrenByTypeIndex ( uiTypeHash, pLuaMain, uiIndex, bStreamedIn );
    }
}


void CClientEntity::GetChildren ( CLuaMain* pLuaMain )
{
    assert ( pLuaMain );

    // Add all our children to the table on top of the given lua main's stack
    unsigned int uiIndex = 0;
    list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        // Add it to the table
        lua_State* luaVM = pLuaMain->GetVirtualMachine ();
        lua_pushnumber ( luaVM, ++uiIndex );
        lua_pushelement ( luaVM, *iter );
        lua_settable ( luaVM, -3 );
    }
}


bool CClientEntity::CollisionExists ( CClientColShape* pShape )
{
    list < CClientColShape* > ::iterator iter = m_Collisions.begin ();
    for ( ; iter != m_Collisions.end () ; iter++ )
    {
        if ( *iter == pShape )
        {
            return true;
        }
    }
    return false;
}


void CClientEntity::RemoveAllCollisions ( bool bNotify )
{
    if ( !m_Collisions.empty () )
    {
        if ( bNotify )
        {
            list < CClientColShape* > ::iterator iter = m_Collisions.begin ();
            for ( ; iter != m_Collisions.end () ; iter++ )
            {
                (*iter)->RemoveCollider ( this );
            }
        }

        m_Collisions.clear ();
    }
}


bool CClientEntity::IsEntityAttached ( CClientEntity* pEntity )
{
    list < CClientEntity* > ::iterator iter = m_AttachedEntities.begin ();
    for ( ; iter != m_AttachedEntities.end (); iter++ )
    {
        if ( *iter == pEntity )
            return true;
    }

    return false;
}


void CClientEntity::ReattachEntities ( void )
{
    // Jax: this should be called on streamIn/creation

    if ( m_pAttachedToEntity )
    {
        InternalAttachTo ( m_pAttachedToEntity );
    }

    // Reattach any entities attached to us
    list < CClientEntity* > ::iterator iter = m_AttachedEntities.begin ();
    for ( ; iter != m_AttachedEntities.end (); iter++ )
    {
        (*iter)->InternalAttachTo ( this );
    }  
}


bool CClientEntity::IsAttachable ( void )
{
    switch ( GetType () )
    {
        case CCLIENTPED:
        case CCLIENTPLAYER:
        case CCLIENTRADARMARKER:
        case CCLIENTVEHICLE:
        case CCLIENTOBJECT:
        case CCLIENTMARKER:
        case CCLIENTPICKUP:
        case CCLIENTSOUND:
        case CCLIENTCOLSHAPE:
        {
            return true;
            break;
        }
        default: break;
    }
    return false;
}


bool CClientEntity::IsAttachToable ( void )
{
    switch ( GetType () )
    {
        case CCLIENTPED:
        case CCLIENTPLAYER:
        case CCLIENTRADARMARKER:
        case CCLIENTVEHICLE:
        case CCLIENTOBJECT:
        case CCLIENTMARKER:
        case CCLIENTPICKUP:
        case CCLIENTSOUND:
        case CCLIENTCOLSHAPE:
        {
            return true;
            break;
        }
        default: break;
    }
    return false;
}


void CClientEntity::DoAttaching ( void )
{
    if ( m_pAttachedToEntity )
    {
        CMatrix matrix, returnMatrix;
        if ( !m_pAttachedToEntity->GetMatrix ( matrix ) )
            m_pAttachedToEntity->GetPosition ( matrix.vPos );

        AttachedMatrix ( matrix, returnMatrix, m_vecAttachedPosition, m_vecAttachedRotation );

        if ( !SetMatrix ( returnMatrix ) ) SetPosition ( returnMatrix.vPos );
    }
}


unsigned int CClientEntity::GetTypeID ( const char* szTypeName )
{
    if ( strcmp ( szTypeName, "dummy" ) == 0 )
        return CCLIENTDUMMY;
    else if ( strcmp ( szTypeName, "ped" ) == 0 )
        return CCLIENTPED;
    else if ( strcmp ( szTypeName, "player" ) == 0 )
        return CCLIENTPLAYER;
    else if ( strcmp ( szTypeName, "projectile" ) == 0 )
        return CCLIENTPROJECTILE;
    else if ( strcmp ( szTypeName, "vehicle" ) == 0 )
        return CCLIENTVEHICLE;
    else if ( strcmp ( szTypeName, "object" ) == 0 )
        return CCLIENTOBJECT;
    else if ( strcmp ( szTypeName, "marker" ) == 0 )
        return CCLIENTMARKER;
    else if ( strcmp ( szTypeName, "blip" ) == 0 )
        return CCLIENTRADARMARKER;
    else if ( strcmp ( szTypeName, "pickup" ) == 0 )
        return CCLIENTPICKUP;
    else if ( strcmp ( szTypeName, "radararea" ) == 0 )
        return CCLIENTRADARAREA;
 	else if ( strcmp ( szTypeName, "sound" ) == 0 )
 	    return CCLIENTSOUND;
    else
        return CCLIENTUNKNOWN;
}


void CClientEntity::DeleteClientChildren ( void )
{
	// Gather a list over children (we can't use the list as it changes)
	list < CClientEntity* > Children;
    list < CClientEntity* > ::const_iterator iterCopy = m_Children.begin ();
	for ( ; iterCopy != m_Children.end (); iterCopy++ )
	{
		Children.push_back ( *iterCopy );
	}

	// Call ourselves on each child of this to go as deep as possible and start deleting there
	list < CClientEntity* > ::const_iterator iter = Children.begin ();
	for ( ; iter != Children.end (); iter++ )
	{
		(*iter)->DeleteClientChildren ();
	}

	// We have no children at this point if we're locally created. Client elements can only be children
	// of server elements, not vice versa.
	
	// Are we a client element?
	if ( IsLocalEntity () && !IsSystemEntity () )
	{
		// Delete us
		g_pClientGame->GetElementDeleter ()->Delete ( this );
	}
}


bool CClientEntity::IsStatic ( void )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        return (pEntity->IsStatic () == TRUE);
    }
    return false;
}


void CClientEntity::SetStatic ( bool bStatic )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        pEntity->SetStatic ( bStatic );
    }
}


unsigned char CClientEntity::GetInterior ( void )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        return pEntity->GetAreaCode ();
    }
    return m_ucInterior;
}


void CClientEntity::SetInterior ( unsigned char ucInterior )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        pEntity->SetAreaCode ( ucInterior );
    }
    m_ucInterior = ucInterior;
}


bool CClientEntity::IsOnScreen ( void )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        return pEntity->IsOnScreen ();
    }
    return false;
}


RpClump * CClientEntity::GetClump ( void )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        return pEntity->GetRpClump ();
    }
    return NULL;
}


// Entities from root optimization for getElementsByType
CClientEntity::t_mapEntitiesFromRoot CClientEntity::ms_mapEntitiesFromRoot;
bool CClientEntity::ms_bEntitiesFromRootInitialized = false;

void CClientEntity::StartupEntitiesFromRoot ()
{
    if ( !ms_bEntitiesFromRootInitialized )
    {
        ms_mapEntitiesFromRoot.set_deleted_key ( (unsigned int)0x00000000 );
        ms_mapEntitiesFromRoot.set_empty_key ( (unsigned int)0xFFFFFFFF );
        ms_bEntitiesFromRootInitialized = true;
    }
}

// Returns true if top parent is root
bool CClientEntity::IsFromRoot ( CClientEntity* pEntity )
{
    if ( !pEntity )
        return false;
    if ( pEntity == g_pClientGame->GetRootEntity() )
        return true;
    return CClientEntity::IsFromRoot ( pEntity->GetParent () );
}

void CClientEntity::AddEntityFromRoot ( unsigned int uiTypeHash, CClientEntity* pEntity, bool bDebugCheck )
{
    // Check
    assert ( CClientEntity::IsFromRoot ( pEntity ) );

    // Insert into list
    std::list < CClientEntity* >& listEntities = ms_mapEntitiesFromRoot [ uiTypeHash ];
    listEntities.remove ( pEntity );
    listEntities.push_front ( pEntity );

    // Apply to child elements as well
    list < CClientEntity* > ::const_iterator iter = pEntity->IterBegin ();
    for ( ; iter != pEntity->IterEnd (); iter++ )
        CClientEntity::AddEntityFromRoot ( (*iter)->GetTypeHash (), *iter, false );

#if MTA_DEBUG
    if ( bDebugCheck )
        _CheckEntitiesFromRoot ( uiTypeHash );
#endif
}

void CClientEntity::RemoveEntityFromRoot ( unsigned int uiTypeHash, CClientEntity* pEntity )
{
    // Remove from list
    t_mapEntitiesFromRoot::iterator find = ms_mapEntitiesFromRoot.find ( uiTypeHash );
    if ( find != ms_mapEntitiesFromRoot.end () )
    {
        std::list < CClientEntity* >& listEntities = find->second;
        listEntities.remove ( pEntity );
        if ( listEntities.size () == 0 )
            ms_mapEntitiesFromRoot.erase ( find );
    }

    // Apply to child elements as well
    list < CClientEntity* > ::const_iterator iter = pEntity->IterBegin ();
    for ( ; iter != pEntity->IterEnd (); iter++ )
        CClientEntity::RemoveEntityFromRoot ( (*iter)->GetTypeHash (), *iter );
}

void CClientEntity::GetEntitiesFromRoot ( unsigned int uiTypeHash, CLuaMain* pLuaMain, bool bStreamedIn )
{
#if MTA_DEBUG
    _CheckEntitiesFromRoot ( uiTypeHash );
#endif

    t_mapEntitiesFromRoot::iterator find = ms_mapEntitiesFromRoot.find ( uiTypeHash );
    if ( find != ms_mapEntitiesFromRoot.end () )
    {
        std::list < CClientEntity* >& listEntities = find->second;
        CClientEntity* pEntity;
        lua_State* luaVM = pLuaMain->GetVirtualMachine ();
        unsigned int uiIndex = 0;

        for ( std::list < CClientEntity* >::const_reverse_iterator i = listEntities.rbegin ();
              i != listEntities.rend ();
              ++i )
        {
            pEntity = *i;

            // Only streamed in elements?
            if ( !bStreamedIn || !pEntity->IsStreamingCompatibleClass() || 
                 reinterpret_cast < CClientStreamElement* > ( pEntity )->IsStreamedIn() )
            {
                // Add it to the table
                lua_pushnumber ( luaVM, ++uiIndex );
                lua_pushelement ( luaVM, pEntity );
                lua_settable ( luaVM, -3 );
            }
        }
    }    
}


#if MTA_DEBUG

//
// Check that GetEntitiesFromRoot produces the same results as FindAllChildrenByTypeIndex on the root element
//
void CClientEntity::_CheckEntitiesFromRoot ( unsigned int uiTypeHash )
{
    std::map < CClientEntity*, int > mapResults1;
    g_pClientGame->GetRootEntity()->_FindAllChildrenByTypeIndex ( uiTypeHash, mapResults1 );

    std::map < CClientEntity*, int > mapResults2;
    _GetEntitiesFromRoot ( uiTypeHash, mapResults2 );

    std::map < CClientEntity*, int > :: const_iterator iter1 = mapResults1.begin ();
    std::map < CClientEntity*, int > :: const_iterator iter2 = mapResults2.begin ();

    for ( ; iter1 != mapResults1.end (); ++iter1 )
    {
        CClientEntity* pElement1 = iter1->first;

        if ( mapResults2.find ( pElement1 ) == mapResults2.end () )
        {
            OutputDebugString ( SString ( "Client: 0x%08x  %s is missing from GetEntitiesFromRoot list\n", pElement1, pElement1->GetTypeName () ) );
        }
    }

    for ( ; iter2 != mapResults2.end (); ++iter2 )
    {
        CClientEntity* pElement2 = iter2->first;

        if ( mapResults1.find ( pElement2 ) == mapResults1.end () )
        {
            OutputDebugString ( SString ( "Client: 0x%08x  %s is missing from FindAllChildrenByTypeIndex list\n", pElement2, pElement2->GetTypeName () ) );
        }
    }

    assert ( mapResults1 == mapResults2 );
}


void CClientEntity::_FindAllChildrenByTypeIndex ( unsigned int uiTypeHash, std::map < CClientEntity*, int >& mapResults )
{
    // Our type matches?
    if ( uiTypeHash == m_uiTypeHash )
    {
        // Add it to the table
        assert ( mapResults.find ( this ) == mapResults.end () );
        mapResults [ this ] = 1;

        assert ( this );
        ElementID ID = this->GetID ();
        assert ( ID != INVALID_ELEMENT_ID );
        assert ( this == CElementIDs::GetElement ( ID ) );
        if ( this->IsBeingDeleted () )
            OutputDebugString ( SString ( "Client: 0x%08x  %s is flagged as IsBeingDeleted() but is still in FindAllChildrenByTypeIndex\n", this, this->GetTypeName () ) );
    }

    // Call us on the children
    list < CClientEntity* > ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        (*iter)->_FindAllChildrenByTypeIndex ( uiTypeHash, mapResults );
    }
}


void CClientEntity::_GetEntitiesFromRoot ( unsigned int uiTypeHash, std::map < CClientEntity*, int >& mapResults )
{
    t_mapEntitiesFromRoot::iterator find = ms_mapEntitiesFromRoot.find ( uiTypeHash );
    if ( find != ms_mapEntitiesFromRoot.end () )
    {
        const std::list < CClientEntity* >& listEntities = find->second;
        CClientEntity* pEntity;
        unsigned int uiIndex = 0;

        for ( std::list < CClientEntity* >::const_reverse_iterator i = listEntities.rbegin ();
              i != listEntities.rend ();
              ++i )
        {
            pEntity = *i;

            assert ( pEntity );
            ElementID ID = pEntity->GetID ();
            assert ( ID != INVALID_ELEMENT_ID );
            assert ( pEntity == CElementIDs::GetElement ( ID ) );
            if ( pEntity->IsBeingDeleted () )
                OutputDebugString ( SString ( "Client: 0x%08x  %s is flagged as IsBeingDeleted() but is still in GetEntitiesFromRoot\n", pEntity, pEntity->GetTypeName () ) );

            assert ( mapResults.find ( pEntity ) == mapResults.end () );
            mapResults [ pEntity ] = 1;
        }
    }    
}

#endif


bool CClientEntity::IsCollidableWith ( CClientEntity * pEntity )
{
    return !MapContains ( m_DisabledCollisions, pEntity );
}


void CClientEntity::SetCollidableWith ( CClientEntity * pEntity, bool bCanCollide )
{
    // quit if no change
    if ( MapContains( m_DisabledCollisions, pEntity ) != bCanCollide )
        return;

    if ( bCanCollide )
    {
        MapRemove ( m_DisabledCollisions, pEntity );
        MapRemove ( g_pClientGame->m_AllDisabledCollisions, pEntity );
    }
    else
    {
        MapSet ( m_DisabledCollisions, pEntity, true );
        MapSet ( g_pClientGame->m_AllDisabledCollisions, pEntity, true );
    }
    // Set in the other entity as well
    pEntity->SetCollidableWith ( this, bCanCollide );
}
